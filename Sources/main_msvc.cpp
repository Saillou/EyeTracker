#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>

#include <turbojpeg.h>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>

#include "Dk/Protocole.hpp"
#include "Dk/ManagerConnection.hpp"

using namespace Protocole;

int main(int argc, char* argv[]) {
	// -- Connect to server
	ManagerConnection managerConnection;
	managerConnection.initialize();
	
	std::cout << "Try to connect" << std::endl;
	auto sock = managerConnection.connectTo(Socket::TCP, "192.168.4.1", 3000);
	if(sock == nullptr)
		return 0;
	
	// -- Create variables
	tjhandle _jpegDecompressor = tjInitDecompress();
	BinMessage msg;
	
	// -- Start communication
	// Ask frame info		
	std::cout << "Search info" << std::endl;
	msg.set(BIN_INFO, "");
	sock->write(msg);
	
	// Answer	
	sock->read(msg);
	if(msg.isValide()) {
		CmdMessage cmd(Message::To_string(msg.getData()));
		
		const size_t WIDTH 		= Message::To_unsignedInt(cmd.getCommand(CMD_WIDTH).second);
		const size_t HEIGHT 		= Message::To_unsignedInt(cmd.getCommand(CMD_HEIGHT).second);
		const size_t CHANNEL 	= Message::To_unsignedInt(cmd.getCommand(CMD_CHANNEL).second);
		
		cv::Mat frame 			= cv::Mat::zeros(HEIGHT, WIDTH, CHANNEL == 1 ? CV_8UC1 : CV_8UC3);
		cv::Mat frameRes 		= cv::Mat::zeros(480, 640, CHANNEL == 1 ? CV_8UC1 : CV_8UC3);
		clock_t lastClock 	= clock();
		size_t nbFrames 		= 0;
		
		if(WIDTH*CHANNEL*HEIGHT > 0) {
			while(cv::waitKey(1) != 27) {
				
				// Ask Frame	
				msg.set(BIN_GAZO, "");
				sock->write(msg);
				
				// Answer
				sock->read(msg);
				if(msg.isValide()) {
					if(msg.getSize() == 0) {
						std::cout << "size null" << std::endl;
						continue;
					}
					try {
						if(_jpegDecompressor == NULL)
							cv::imdecode(msg.getData(), CHANNEL == 1 ? CV_LOAD_IMAGE_GRAYSCALE : CV_LOAD_IMAGE_COLOR, &frame);
						else 
							tjDecompress2(_jpegDecompressor, (const unsigned char*)msg.getData().data(), msg.getSize(), frame.data, WIDTH, 0, HEIGHT, CHANNEL == 1 ? TJPF_GRAY : TJPF_BGR, TJFLAG_FASTDCT);
						
						cv::resize(frame, frameRes, frameRes.size());
						cv::imshow("Frame(res)", frameRes);
						nbFrames++;
					}
					catch(cv::Exception& e) {
						std::cout << " -- Message >> "<< e.msg << std::endl;
						std::cout << " -- What >> " << e.what() << std::endl;
					}
					catch(...) {
						std::cout << "Exception but not rule." << std::endl;
					}
					
					clock_t thisClock = clock();
					if(thisClock - lastClock > 1000) {
						std::cout << "Fps: " << 1000.0*nbFrames/(thisClock - lastClock) << std::endl;
						lastClock = thisClock;
						nbFrames = 0;
					}
					
				}
			}
			
		}
	}
	
	// Quit
	msg.set(BIN_QUIT, "");
	sock->write(msg);
	
	// -- End
	tjDestroy(_jpegDecompressor);
	return 0;
}