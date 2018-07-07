#include <iostream>
#include <fstream>
#include <cmath>
#include <thread>

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

void handleClient(std::shared_ptr<Server> server, std::shared_ptr<cv::VideoCapture> ptrCap = nullptr) {
	const int QUALITY = 80;
	const bool GRAY 	= false;
	const bool RESIZE = true;
	
	// Encodage TURBO-JPG
	tjhandle _jpegCompressor 	= tjInitCompress();
	unsigned char* buff 			= tjAlloc(10000); // Random init, tj will carry on the allocation
	unsigned long bufSize 		= 0;
	
	// Encodage OPENCV
	std::vector<uchar> buf;
	const std::vector<int> params{CV_IMWRITE_JPEG_QUALITY, QUALITY};
	const std::string format = ".jpg";
		
	// ---------------------------- 
	// Wait for client
	std::cout << "Wait for clients" << std::endl;
	int idClient = server->waitClient();
	
	std::cout << "Handle new client" << std::endl;
	BinMessage msg;
	
	// Define frame expected
	cv::Mat frameCam 				= cv::Mat::zeros(480, 640, CV_8UC3);
	cv::Mat frameCamResized	= RESIZE ? cv::Mat::zeros(240, 320, GRAY ? CV_8UC1 : CV_8UC3) : cv::Mat::zeros(frameCam.rows, frameCam.cols, frameCam.type());
	
	// Animation parameters
	const cv::Point center 		= cv::Point(frameCam.cols/2, frameCam.rows/2);
	const int diameterMax 		= frameCam.rows/4;
	size_t iFrameSend 			= 0;
	
	// -- Handle client --
	bool run = true;
	while(run) {
		// Get the frame [If no videoCapture => generate an animation]
		if(ptrCap == nullptr) {
			frameCam = cv::Mat::zeros(frameCam.rows, frameCam.cols, GRAY ? CV_8UC1 : CV_8UC3);
			cv::circle(frameCam, center, diameterMax*(1+std::cos(0.1*iFrameSend)), cv::Scalar(255), -1);
		}
		else {
			*ptrCap >> frameCam;
			if(GRAY && frameCam.channels() == 3)
				cv::cvtColor(frameCam, frameCam, cv::COLOR_BGR2GRAY);
		}
		
		// Check validity
		if(frameCam.empty())
			continue;
		
		// Adapt size
		if(RESIZE)
			cv::resize(frameCam, frameCamResized, frameCamResized.size());
		else
			frameCamResized = frameCam;
		
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
					cmd.addCommand(CMD_HEIGHT, 	std::to_string(frameCamResized.rows));
					cmd.addCommand(CMD_WIDTH, 	std::to_string(frameCamResized.cols));
					cmd.addCommand(CMD_CHANNEL,	std::to_string(frameCamResized.channels()));
					
					msg.set(BIN_MCMD, Message::To_string(cmd.serialize()));
					server->write(msg, idClient);
				}
				break;
				
				// Send a frame
				case BIN_GAZO: 
				{	
					try {
						// Compress to jpg with turbojpeg or opencv
						if(_jpegCompressor == NULL) {
							cv::imencode(
								format, 		// Extension, std::string
								frameCamResized,
								buf, 			// Data out, vector<char>
								params		// Jpeg copmression, vector<int>
							);
							msg.set(BIN_GAZO, buf.size(), (const char*)buf.data());
						}
						else {	
							const int TJ_FORMAT = GRAY ? TJPF_GRAY : TJPF_BGR;
							const int TJ_SUBSAMP = GRAY ? TJSAMP_GRAY : TJSAMP_420;
							
							tjCompress2(
								_jpegCompressor, 
								frameCamResized.data, 	// ptr to data, const uchar *
								frameCamResized.cols, 	// width
								TJPAD(frameCamResized.cols * tjPixelSize[TJ_FORMAT]), // bytes per line
								frameCamResized.rows,	// height
								TJ_FORMAT, 		// pixel format
								&buff, 			// ptr to buffer, unsigned char **
								&bufSize, 		// ptr to buffer size, unsigned long *
								TJ_SUBSAMP,		// chrominace sub sampling
								QUALITY, 		// quality, int
								0 					// flags
							);
							msg.set(BIN_GAZO, (size_t)bufSize, (const char*)buff);
						}
						
						// Write a message, even if encodage failed (it will be null then).
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

		} // Msg.valide()
	} // run
	
	server->closeSocket(idClient);
	std::cout << "Client disconnected." << std::endl;
	
	tjFree(buff);
	tjDestroy(_jpegCompressor);
}


int main() {
	// cv::Mat frameCam;
	// cv::Mat frameCamResized(240, 320, CV_8UC3);
	
	// cv::VideoCapture camera(0);
	// cv::VideoWriter video("Record.avi", CV_FOURCC('M', 'P', '4', '2'), 30, cv::Size(frameCamResized.cols, frameCamResized.rows), true);
	
	// if(!video.isOpened() || !camera.isOpened())
		// return -1;
	
	// clock_t lastClock = clock();
	// size_t nbFrames 	= 0;
	
	// while(cv::waitKey(1) != 27) {
		// camera >> frameCam;
		// if(frameCam.empty())
			// continue;
		
		// cv::resize(frameCam, frameCamResized, frameCamResized.size());
		
		
		// video << frameCamResized;
		// cv::imshow("Frame", frameCamResized);
		// nbFrames++;
		
		// clock_t thisClock = clock();
		// if(thisClock - lastClock > 1000) {
			// std::cout << "Fps: " << 1000000.0*nbFrames/(thisClock - lastClock) << std::endl;
			// lastClock = thisClock;
			// nbFrames = 0;
		// }
	// }
	
	// return 0;
	
	
	// Try to open the cam
	std::shared_ptr<cv::VideoCapture> pCap = std::make_shared<cv::VideoCapture>(0);
	if(!pCap->isOpened())
		pCap.reset();
	
	// Create server TCP
	ManagerConnection managerConnection;
	managerConnection.initialize();
	auto server = managerConnection.createServer(Socket::TCP, SOCKET_PORT, MAXPENDING);

	while(1) {
		// Handle new clients coming
		try {
			std::thread handleThread(handleClient, server, pCap);
		}
		catch(...) {
			std::cout << "Thread failed" << std::endl;
		}
		std::cout << "." << std::endl;
	}
	
	return 0;	
}

