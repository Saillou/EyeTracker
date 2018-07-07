#include <iostream>
#include <fstream>
#include <cmath>

#include <mutex>
#include <atomic>
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

std::atomic<bool> G_record(true);
std::atomic<bool> G_client(true);
std::mutex G_frameMutex;

int handleRecord(const cv::String pathVideo ,std::shared_ptr<cv::Mat> pFrame) {
	const int FPS = 30;
	const double Ts = 1000.0/FPS; // ms.
	
	G_frameMutex.lock();
	cv::VideoWriter video(pathVideo, CV_FOURCC('M', 'P', '4', '2'), FPS, cv::Size(pFrame->cols, pFrame->rows), true);
	G_frameMutex.unlock();
	
	clock_t clockT = clock();
	cv::Mat frame;
	
	while(G_record) {
		if((clock() - clockT) / 1000 >= Ts) {
			clockT = clock();
			
			G_frameMutex.lock();
			frame = pFrame->clone();
			G_frameMutex.unlock();
			
			video << frame;
		}
	}
	
	video.release();
	return 0;
}

int handleClient(std::shared_ptr<Server> server, std::shared_ptr<cv::Mat> pFrame) {
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
	do {	
		// Received
		server->read(msg, idClient);
		
		// Answer
		if(!msg.isValide()) {
			
			continue;
		}
		
		switch(msg.getAction()) {
			// Client quit
			case BIN_QUIT:
				G_client = false;
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
					if(_jpegCompressor == NULL) {
						G_frameMutex.lock();
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
		} // getAction()
	} while(G_client);
	
	server->closeSocket(idClient);
	std::cout << "Client disconnected." << std::endl;
	
	tjFree(buff);
	tjDestroy(_jpegCompressor);
	
	return 0;
}


int main() {
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
	std::thread threadClient(handleClient, server, pFrameResized);
	std::thread threadRecord(handleRecord, "Record_.avi", pFrameResized);
	
	while(cv::waitKey(1) != 27) {
		// Acquire the frame
		*pCap >> frameCam;
		
		// Check validity
		if(frameCam.empty())
			continue;
		
		// Adapt size
		G_frameMutex.lock();
		cv::resize(frameCam, *pFrameResized, pFrameResized->size());
		G_frameMutex.unlock();
		
		if (G_frameMutex.try_lock()) {
			cv::imshow("frame", *pFrameResized);
			G_frameMutex.unlock();
		}
	}
	
	// Finish record
	G_record = false;
	threadRecord.join();
	
	// Finish client
	G_client = false;
	threadClient.join();
	
	return 0;	
}

