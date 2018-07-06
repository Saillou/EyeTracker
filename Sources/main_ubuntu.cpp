#include <iostream>
#include <fstream>
#include <cmath>

#include <turbojpeg.h>

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
	const int QUALITY = 80;
	
	// Encodage TURBO-JPG
	tjhandle _jpegCompressor 	= tjInitCompress();
	unsigned char* buff 		= tjAlloc(25000);
	unsigned long bufSize 		= 0;
	
	// Encodage OPENCV
	std::vector<uchar> buf;
	const std::vector<int> params{CV_IMWRITE_JPEG_QUALITY, QUALITY};
	const std::string format = ".jpg";
		
	// ---------------------------- 
					
	std::cout << "Handle new client" << std::endl;
	BinMessage msg;
	
	// Define frame expected
	cv::Mat frameCam 		= cv::Mat::zeros(480, 640, CV_8UC3);
	const cv::Point center 	= cv::Point(frameCam.cols/2, frameCam.rows/2);
	const int diameterMax 	= 0.25*frameCam.rows;
	size_t iFrameSend = 0;
	
	// -- Handle client --
	bool run = true;
	while(run) {
		// Get the frame
		if(ptrCap == nullptr) {
			frameCam = cv::Mat::zeros(frameCam.rows, frameCam.cols, CV_8UC1);
			cv::circle(frameCam, center, diameterMax*(1+std::cos(0.1*iFrameSend)), cv::Scalar(255), -1);
		}
		else {
			*ptrCap >> frameCam;
			cv::cvtColor(frameCam, frameCam, cv::COLOR_BGR2GRAY);
		}
		
		// Received
		server->read(msg, idClient);
		
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
						if(_jpegCompressor == NULL) {
							cv::imencode(
								format, 	// Extension, std::string
								frameCam, 	// Data in, cv::Mat
								buf, 		// Data out, vector<char>
								params		// Jpeg copmression, vector<int>
							);
							msg.set(BIN_GAZO, buf.size(), (const char*)buf.data());
						}
						else {							
							tjCompress2(
								_jpegCompressor, 
								frameCam.data, 	// ptr to data, const uchar *
								frameCam.cols, 	// width
								TJPAD(frameCam.cols * tjPixelSize[TJPF_GRAY]), // bytes per line
								frameCam.rows,	// height
								TJPF_GRAY, 		// pixel format
								&buff, 			// ptr to buffer, unsigned char **
								&bufSize, 		// ptr to buffer size, unsigned long *
								TJSAMP_GRAY,	// chrominace sub sampling
								QUALITY, 		// quality, int
								0 				// flags
							);
							msg.set(BIN_GAZO, (size_t)bufSize, (const char*)buff);
						}
						server->write(msg, idClient);
					}
					catch(...) {
						std::cout << "Exception throw" << std::endl;
						msg.clear();
						server->write(msg, idClient);
					}
					
					iFrameSend++;
				}
				break;
			}

		}
	}
	
	server->closeSocket(idClient);
	std::cout << "Client disconnected." << std::endl;
	
	tjFree(buff);
	tjDestroy(_jpegCompressor);
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

