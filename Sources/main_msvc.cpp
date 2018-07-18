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

int main() {
	// -- Connect to server
	ManagerConnection managerConnection;
	managerConnection.initialize();
	
	std::cout << "Try to connect.." << std::endl;
	auto sock = managerConnection.connectTo(Socket::TCP, Socket::BLOCKING, "192.168.128.34", 3000);
	
	if(sock == nullptr) {
		std::cout << "Not connected." << std::endl;
		return 0;
	}
	else
		std::cout << "Connected." << std::endl;
	
	// -- Init decompresor
	tjhandle _jpegDecompressor = tjInitDecompress();
	if(_jpegDecompressor == NULL){
		std::cout << "Libjpeg-turbo not loaded" << std::endl;
		return 0;
	}
	
	// -- Start communication
	BinMessage msg;
	
	// Ask frame info		
	std::cout << "Search info" << std::endl;
	msg.set(BIN_INFO, "");
	sock->write(msg);
	
	// Answer	
	if(sock->read(msg)) {
		CmdMessage cmd(Message::To_string(msg.getData()));
		const int WIDTH 	= (int)Message::To_unsignedInt(cmd.getCommand(CMD_WIDTH).second);
		const int HEIGHT 	= (int)Message::To_unsignedInt(cmd.getCommand(CMD_HEIGHT).second);
		const int CHANNEL 	= (int)Message::To_unsignedInt(cmd.getCommand(CMD_CHANNEL).second);
		
		if(WIDTH*CHANNEL*HEIGHT > 0) {
			cv::Mat frame 			= cv::Mat::zeros(HEIGHT, WIDTH, CHANNEL == 1 ? CV_8UC1 : CV_8UC3);
			cv::Mat frameRes 		= cv::Mat::zeros(480, 640, CHANNEL == 1 ? CV_8UC1 : CV_8UC3);
			clock_t lastClock 	= clock();
			size_t nbFrames 		= 0;
		
			while(cv::waitKey(1) != 27) {
				// Ask Frame	
				msg.set(BIN_GAZO, "");
				if(!sock->write(msg)) {
					std::cout << "Server disconnected." << std::endl;
					break;
				}
				
				// Answer
				if(sock->read(msg)) {
					if(msg.getSize() == 0) {
						std::cout << "size null" << std::endl;
						break;
					}
					
					tjDecompress2(_jpegDecompressor, (const unsigned char*)msg.getData().data(), (unsigned long)msg.getSize(), frame.data, WIDTH, 0, HEIGHT, CHANNEL == 1 ? TJPF_GRAY : TJPF_BGR, TJFLAG_FASTDCT);
					if(!frame.empty()) {
						if(frame.size() != frameRes.size())
							cv::resize(frame, frameRes, frameRes.size());
						else	
							frameRes = frame;
						
						cv::imshow("Frame(res)", frameRes);
						nbFrames++;
					}
					
					// Fps compute
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
	std::cout << "Quit" << std::endl;
	msg.set(BIN_QUIT, "");
	sock->write(msg);
	
	// -- End
	tjDestroy(_jpegDecompressor);
	
	return 0;
}