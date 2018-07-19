#include <Windows.h>
#include <iostream>
#include <fstream>
#include <cmath>
#include <thread>
#include <atomic>
#include <turbojpeg.h>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>

#include "Dk/Protocole.hpp"
#include "Dk/ManagerConnection.hpp"
#include "Dk/Chronometre.hpp"

#define MULTITHREAD 0

std::atomic<bool> G_HANDLING(true);
std::mutex G_mutexFrame;

using namespace Protocole;

void handleClient(std::shared_ptr<Server> server, std::shared_ptr<cv::Mat> pFrame) {
	const int QUALITY 	= 80;
	
	// Encodage TURBO-JPG
	tjhandle _jpegCompressor = tjInitCompress();
	if(_jpegCompressor == NULL) {
		std::cout << "Libjpeg-turbo not loaded" << std::endl;
		return;
	}
	
	unsigned char* buff 		= tjAlloc(10000); // Random init, tj will carry on the allocation
	unsigned long bufSize 	= 0;
		
	// ---------------------------- 
	// Wait for client
	std::cout << "Wait for clients";
	while(G_HANDLING) {
		std::cout << ".";
		
		int idClient = server->waitClient(5);
		if(idClient <= 0) { 
			Chronometre::wait(350);
			continue;
		}
		
		std::cout << std::endl << "Handle new client " << idClient << std::endl;
		BinMessage msg;
		
		// -- Handle client --
#ifdef MULTITHREAD
		std::vector<std::shared_ptr<Server::ThreadWrite>> threadsRunning;
#endif
		bool run = true;
		
		while(run) {
			// Get the frame
			G_mutexFrame.lock();
			if(pFrame == nullptr || pFrame->empty())
				continue;
			
			cv::Mat frame = pFrame->clone();
			G_mutexFrame.unlock();
			
			// Need an answer ?
			if(server->read(msg, idClient)) {
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
						cmd.addCommand(CMD_HEIGHT, 	std::to_string(frame.rows));
						cmd.addCommand(CMD_WIDTH, 	std::to_string(frame.cols));
						cmd.addCommand(CMD_CHANNEL,	std::to_string(frame.channels()));
						
						msg.set(BIN_MCMD, Message::To_string(cmd.serialize()));
						server->write(msg, idClient);
					}
					break;
					
					// Send a frame
					case BIN_GAZO: 
					{	
						tjCompress2 (
							_jpegCompressor, 
							frame.data, 	// ptr to data, const uchar *
							frame.cols, 	// width
							TJPAD(frame.cols * tjPixelSize[TJPF_BGR]), // bytes per line
							frame.rows,	// height
							TJPF_BGR, 		// pixel format
							&buff, 			// ptr to buffer, unsigned char **
							&bufSize, 		// ptr to buffer size, unsigned long *
							TJSAMP_420,	// chrominace sub sampling
							QUALITY, 		// quality, int
							0 					// flags
						);
						msg.set(BIN_GAZO, (size_t)bufSize, (const char*)buff);
						
						// Write the message						
#ifdef MULTITHREAD
						threadsRunning.push_back(std::make_shared<Server::ThreadWrite>(server, msg, idClient));
#else
						server->write(msg, idClient);
#endif
					}
					break;
				} // Switch action
				
				// Delete threads that are finished (<=> inactive)
#ifdef MULTITHREAD
				threadsRunning.erase(
					std::remove_if(threadsRunning.begin(), threadsRunning.end(), [](const std::shared_ptr<Server::ThreadWrite>& tw) {
						return !tw->isActive();
				}), threadsRunning.end());
#endif

			} // Msg.valide
			else 
				break;

		} // run
		
		server->closeSocket(idClient);
#ifdef MULTITHREAD
		threadsRunning.clear();
#endif

		std::cout << "Client disconnected." << std::endl;
		if(G_HANDLING)
			std::cout << "Wait for clients";
	} // G_HANDLING
	
	tjFree(buff);
	tjDestroy(_jpegCompressor);
}


int main() {
	// Open the cam
	std::shared_ptr<cv::Mat> pFrame = std::make_shared<cv::Mat>(480, 640, CV_8UC3, cv::Scalar::all(0));
	std::shared_ptr<cv::VideoCapture> pCap = std::make_shared<cv::VideoCapture>(0);
	
	if(!pCap->isOpened())
		return 0;
	
	// Create server TCP
	const int MAXPENDING = 5;
	const unsigned short SOCKET_PORT = 3000;
	
	ManagerConnection managerConnection;
	managerConnection.initialize();
	auto server = managerConnection.createServer(Socket::TCP, Socket::BLOCKING, SOCKET_PORT, MAXPENDING);

	// Handle one client
	std::thread handleThread(handleClient, server, pFrame);
	
	bool run = true;
	Chronometre chrono;		
	
	while((GetKeyState(VK_SPACE) & 0x8000) == 0) {
		// Update frame
		G_mutexFrame.lock();
		*pCap >> *pFrame;
		G_mutexFrame.unlock();
		
		// Display info
		if(chrono.elapsed_ms() >= 1000) { 			
			chrono.beg();
			int clockMs = (int)chrono.clock_ms();
			int mn 	= clockMs/60/1000;
			int sec	= clockMs/1000%60;
			std::cout << std::endl << (mn < 10 ? "0" : "") << mn << "'" << (sec < 10 ? "0" : "") << sec << std::endl;
		}
	}
	
	// Wait to finish with the client
	G_HANDLING = false;
	handleThread.join();
	
	return 0;	
}

