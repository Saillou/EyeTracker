#include <iostream>
#include <fstream>
#include <cmath>

#include <mutex>
#include <thread>

#include <turbojpeg.h>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>

#include "Dk/Protocole.hpp"
#include "Dk/ManagerConnection.hpp"

const int MAXPENDING 	= 1;
const int SOCKET_PORT 	= 3000;

using namespace Protocole;

std::mutex G_frameMutex;

void handleClient(std::shared_ptr<Server> server, std::shared_ptr<cv::Mat> pFrame) {
	// Params encoding
	const int QUALITY = 80;
	
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
	
	// -- Handle client --
	bool run = true;
	do {	
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
					G_frameMutex.lock();
					cmd.addCommand(CMD_HEIGHT, 	std::to_string(pFrame->rows));
					cmd.addCommand(CMD_WIDTH, 	std::to_string(pFrame->cols));
					cmd.addCommand(CMD_CHANNEL,	std::to_string(pFrame->channels()));
					G_frameMutex.unlock();

					msg.set(BIN_MCMD, Message::To_string(cmd.serialize()));
					server->write(msg, idClient);
				}
				break;
				
				// Send a frame
				case BIN_GAZO: 
				{	
					try {
						// Compress to jpg with turbojpeg or opencv
						G_frameMutex.lock();
						if(_jpegCompressor == NULL) {
							cv::imencode(
								format, 		// Extension, std::string
								*pFrame,
								buf, 			// Data out, vector<char>
								params		// Jpeg copmression, vector<int>
							);
							G_frameMutex.unlock();
							msg.set(BIN_GAZO, buf.size(), (const char*)buf.data());
						}
						else {	
							G_frameMutex.lock();
							tjCompress2(
								_jpegCompressor, 
								pFrame->data, 	// ptr to data, const uchar *
								pFrame->cols, 	// width
								TJPAD(pFrame->cols * tjPixelSize[TJPF_BGR]), // bytes per line
								pFrame->rows,	// height
								TJPF_BGR, 		// pixel format
								&buff, 			// ptr to buffer, unsigned char **
								&bufSize, 		// ptr to buffer size, unsigned long *
								TJSAMP_420,		// chrominace sub sampling
								QUALITY, 		// quality, int
								0 					// flags
							);
							G_frameMutex.unlock();
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
				}
				break;
			}
		} // Msg.valide()
	} while(run);
	
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
	
	// Create server TCP
	ManagerConnection managerConnection;
	managerConnection.initialize();
	auto server = managerConnection.createServer(Socket::TCP, SOCKET_PORT, MAXPENDING);
	
	// Create frames
	cv::Mat frameCam(480, 640, CV_8UC3, cv::Scalar::all(0));
	std::shared_ptr<cv::Mat> pFrameResized = std::make_shared<cv::Mat>(240, 320, CV_8UC3, cv::Scalar::all(0));
	
	// Try to open the cam
	std::shared_ptr<cv::VideoCapture> pCap = std::make_shared<cv::VideoCapture>(0);
	if(pCap == nullptr || !pCap->isOpened())
		return -1;

	// Handle one client
	std::thread handleThread(handleClient, server, pFrameResized);
	
	while(cv::waitKey(1) != 27) {
		// Acquire the frame
		*pCap >> frameCam;
		
		// Check validity
		if(frameCam.empty())
			continue;
		
		// Adapt size
		G_frameMutex.lock();
		cv::resize(frameCam, *pFrameResized, pFrameResized->size());
		cv::imshow("frame", *pFrameResized);
		G_frameMutex.unlock();
	}
	
	// Wait to finish with the client
	// handleThread.join();
	
	return 0;	
}

