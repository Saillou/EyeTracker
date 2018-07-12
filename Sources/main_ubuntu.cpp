#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <ctime>

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

using namespace Protocole;

std::atomic<bool> G_record(true);
std::atomic<bool> G_client(true);
std::mutex G_frameMutex;

int handleRecord(const cv::String pathVideo ,std::shared_ptr<cv::Mat> pFrame) {
	const int FPS = 30;
	const double Ts = 1000.0/FPS; // ms.
	
	G_frameMutex.lock();
	cv::VideoWriter video(pathVideo, cv::VideoWriter::fourcc('M', 'P', '4', '2'), FPS, cv::Size(pFrame->cols, pFrame->rows), true);
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
	unsigned char* buff 		= tjAlloc(10000); // Random init, tj will carry on the allocation
	unsigned long bufSize 		= 0;
	
	// Encodage OPENCV
	std::vector<uchar> buf;
	const std::vector<int> params{cv::IMWRITE_JPEG_QUALITY, QUALITY};
	const std::string format = ".jpg";
		
	// ---------------------------- 
	while(G_client) {
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
			if(!msg.isValide()) {
				continue;
			}
			
			switch(msg.getAction()) {
				// Client quit
				case BIN_QUIT:
					run = false;
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
		} while(run);
		
		server->closeSocket(idClient);
		std::cout << "Client disconnected." << std::endl;
	}
	
	tjFree(buff);
	tjDestroy(_jpegCompressor);
	
	return 0;
}

std::string _dateToString() {
	time_t rawtime;
	time(&rawtime);
	
	struct tm* timeInfo;
	timeInfo = localtime(&rawtime);
	
	auto __int2paddedStr = [](const int _int, const size_t pad) {
		std::string intstr = std::to_string(_int);
		if(intstr.size() >= pad)
			return intstr;
		else
			return std::string(pad - intstr.size() , '0') + intstr;
	};
	
	
	std::stringstream ss;
	ss << __int2paddedStr(timeInfo->tm_year + 1900, 4);
	ss << __int2paddedStr(timeInfo->tm_mon + 1, 2);
	ss << __int2paddedStr(timeInfo->tm_mday,	2);
	ss << __int2paddedStr(timeInfo->tm_hour,	2);
	ss << __int2paddedStr(timeInfo->tm_min,	2);
	ss << __int2paddedStr(timeInfo->tm_sec, 	2);
	
	return ss.str();
}


int main(int argc, char* argv[]) {
	// Config
	const std::string baseInput 	= "0";
	const std::string baseOutput 	= "/home/pi/prog/EyeTracker/Web/Recordings/";
	const std::string basePort	= "3000";
	
	// Parameters final
	std::string input 	= baseInput;
	std::string output 	= baseOutput;
	std::string port 		= basePort;
	
	// Interpreat command line
	for(int i = 1; i < argc; i++) {
		// -- Read --
		if(strlen(argv[i]) < 2 || argv[i][0] != '-') 
			continue;
		
		std::string str(argv[i]);
		size_t posM 	= str.find(':');
		bool withValue = (posM != std::string::npos);
		
		std::string command 	= std::string(str, 1, posM - (int)withValue); // Cut commande before ':'
		std::string value 	= withValue ? std::string(str, posM + 1) : "true";
		
		// -- Usage --
		if(command == "in") input 		= value;
		if(command == "out") output 	= value;
		if(command == "port") port 	= value;
	}
	
	// Create server TCP
	ManagerConnection managerConnection;
	managerConnection.initialize();
	auto server = managerConnection.createServer(Socket::TCP, Message::To_unsignedInt(port), 1);
	
	// Create frames
	cv::Mat frameCam(480, 640, CV_8UC3, cv::Scalar::all(0));
	std::shared_ptr<cv::Mat> pFrameResized = std::make_shared<cv::Mat>(240, 320, CV_8UC3, cv::Scalar::all(0));
	
	// Try to open the cam
	std::shared_ptr<cv::VideoCapture> pCap;
	if(input.size() == 1)
		pCap = std::make_shared<cv::VideoCapture>((int)(input[0]-'0'));
	else
		pCap = std::make_shared<cv::VideoCapture>(input);
	
	if(pCap == nullptr || !pCap->isOpened())
		return -1;

	// Handle one client
	std::thread threadClient(handleClient, server, pFrameResized);
	std::thread threadRecord(handleRecord, output + _dateToString() + ".avi", pFrameResized);
	
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

