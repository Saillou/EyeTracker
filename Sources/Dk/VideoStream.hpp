#ifndef VIDEO_STREAM_HPP
#define VIDEO_STREAM_HPP

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

#include "Protocole.hpp"
#include "ManagerConnection.hpp"
#include "Chronometre.hpp"

namespace Dk {
	// --- Some enumerations --- 
	enum RunningState {
		NOT_DEFINED, STOPPED, PAUSED, PLAYING
	};
	
	class VideoStream {
	public:
		// Constructors
		VideoStream(ManagerConnection& managerConnection, const ManagerConnection::IpAdress& ipServer);
		~VideoStream();
	
		// Methods
		bool initFormat();
		bool release();
		
		bool play();
		bool pause();
		bool stop();
		
		void run();
		
		// Getter
		RunningState getState();
		double getFpsRate();
		double getLag();
		bool isValide() const;
		
	private:
		// Methods
		bool _askFrame();
		
		// Members
		ManagerConnection::IpAdress _ipServer;
		std::shared_ptr<Socket> 		_sock;
		tjhandle 								_jpegDecompressor;
		
		cv::Mat _frame;
		
		bool 	_valide;
		
		// Thread use
		RunningState _state;
		std::mutex 	_mutexState;
		
		double _fps;
		std::mutex _mutexFps;
		
		double _lag;
		std::mutex _mutexLag;
		
		std::thread* _threadRun;
	};
}

#endif