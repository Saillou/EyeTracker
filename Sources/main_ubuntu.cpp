#include <iostream>
#include <fstream>
#include <cmath>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>

#include "Dk/Protocole.hpp"
#include "Dk/ManagerConnection.hpp"

#define MAXPENDING 	5
#define SOCKET_PORT 3000

using namespace Protocole;


void handleClient(int idClient, std::shared_ptr<Server> server, std::shared_ptr<cv::VideoCapture> ptrCap = nullptr) {
	std::cout << "Handle new client" << std::endl;
	BinMessage msg;
	
	// Define frame expected
	cv::Mat frameCam = cv::Mat::zeros(1000, 1000, CV_8UC3);
	size_t iFrameSend = 0;
	
	// Encodage declaration variables
	std::vector<uchar> buf;
	const std::vector<int> params{CV_IMWRITE_JPEG_QUALITY, 80};
	const std::string format = ".jpg";
	
	// -- Handle client --
	bool run = true;
	while(run) {
		// Get the frame
		std::cout << "-------" << std::endl; // <-- BEG _ mod
		clock_t clock0 = clock();
		if(ptrCap == nullptr) {
			frameCam = cv::Mat::zeros(frameCam.rows, frameCam.cols, frameCam.type());
			cv::circle(frameCam, cv::Point(frameCam.cols/2, frameCam.rows/2), 0.25*frameCam.rows*(1+std::cos(0.1*iFrameSend)), cv::Scalar(255), -1, CV_AA);
			
			if(frameCam.channels() > 1)
				cv::cvtColor(frameCam, frameCam, cv::COLOR_BGR2GRAY);
		}
		else {
			*ptrCap >> frameCam;
			cv::cvtColor(frameCam, frameCam, cv::COLOR_BGR2GRAY);
		}
		std::cout << clock() - clock0 << std::endl; // <-- END _ mod
		
		// Received
		server->read(msg, idClient);
		std::cout << clock() - clock0 << std::endl;
		
		// Answer
		if(msg.isValide()) {
			switch(msg.getAction()) {
				// Client quit
				case BIN_QUIT:
				{ 
					run = false;
				}
				break;
				
				// Frame info asked
				case BIN_INFO:
				{ 
					CmdMessage cmd;
					cmd.addCommand(CMD_HEIGHT, 	std::to_string(frameCam.rows));
					cmd.addCommand(CMD_WIDTH, 	std::to_string(frameCam.cols));
					cmd.addCommand(CMD_CHANNEL, std::to_string(frameCam.channels()));
					
					msg.set(BIN_MCMD, Message::To_string(cmd.serialize()));
					server->write(msg, idClient);
				}
				break;
				
				// Send a frame
				case BIN_GAZO: 
				{	
					try {
						std::cout << clock() - clock0 << std::endl; // <-- BEG _ mod
						cv::imencode(format, frameCam, buf, params);
						std::cout << clock() - clock0 << std::endl; // <-- End _ mod
						msg.set(BIN_GAZO, buf.size(), (const char*)buf.data());
						std::cout << clock() - clock0 << std::endl;
						server->write(msg, idClient);
						std::cout << clock() - clock0 << std::endl;
					}
					catch(...) {
						std::cout << "Exception throw" << std::endl;
						msg.clear();
						server->write(msg, idClient);
					}
					

					iFrameSend++;		
					std::cout << "total=" << clock() - clock0 << std::endl;
				}
				break;
			}

		}
	}
	server->closeSocket(idClient);
	std::cout << "Client disconnected." << std::endl;
}


int main() {
	// Try to open the cam
	std::shared_ptr<cv::VideoCapture> pCap = std::make_shared<cv::VideoCapture>(0);
	if(!pCap->isOpened())
		pCap.reset();
	
	// Create server TCP
	ManagerConnection managerConnection;
	managerConnection.initialize();
	auto server = managerConnection.createServer(Socket::TCP, SOCKET_PORT, MAXPENDING);

	// Handle client until sigint
	std::cout << "Wait for clients" << std::endl;
	while(1) {
		// Wait until client pop
		handleClient(server->waitClient(), server, pCap);
	}
	
	return 0;	
}

