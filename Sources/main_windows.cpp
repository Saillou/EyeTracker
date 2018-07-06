#include <iostream>
#include <opencv2\core.hpp>
#include <opencv2\highgui.hpp>
#include <opencv2\imgproc.hpp>

#include "Dk/Protocole.hpp"
#include "Dk/ManagerConnection.hpp"

using namespace Protocole;

int main() {
	// -- Connect to server
	ManagerConnection managerConnection;
	managerConnection.initialize();
	
	auto sock = managerConnection.connectTo(Socket::UDP, "192.168.128.46", 3000);
	if(sock == nullptr)
		return 0;
	
	// -- Create empty message
	BinMessage msg;
	
	// -- Start communication
	// Ask frame info		
	msg.set(BIN_INFO, "");
	sock->write(msg);
	
	// Answer	
	sock->read(msg);
	if(msg.isValide()) {
		CmdMessage cmd(Message::To_string(msg.getData()));
		
		const size_t WIDTH 		= Message::To_unsignedInt(cmd.getCommand(CMD_WIDTH).second);
		const size_t HEIGHT 	= Message::To_unsignedInt(cmd.getCommand(CMD_HEIGHT).second);
		const size_t CHANNEL 	= Message::To_unsignedInt(cmd.getCommand(CMD_CHANNEL).second);
		
		cv::Mat frame 			= cv::Mat::zeros(HEIGHT, WIDTH, CHANNEL == 1 ? CV_8UC1 : CV_8UC3);
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
					try {
						cv::imdecode(msg.getData(), CHANNEL == 1 ? CV_LOAD_IMAGE_GRAYSCALE : CV_LOAD_IMAGE_COLOR, &frame);
						cv::imshow("frame", frame);
						nbFrames++;
					}
					catch(cv::Exception& e) {
						std::cout << " -- Message >> "<< e.msg << std::endl;
						std::cout << " -- What >> " << e.what() << std::endl;
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
	return 0;
}