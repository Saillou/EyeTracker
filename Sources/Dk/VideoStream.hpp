#ifndef VIDEO_STREAM_HPP
#define VIDEO_STREAM_HPP

#include <iostream>
#include <thread>
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
		RunningState getState() const;
		double getFpsRate() const;
		double getLag() const;
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
		std::atomic<RunningState> _atomState{NOT_DEFINED};
		std::atomic<double> _atomFps{0.0};
		std::atomic<double> _atomLag{0.0};
		
		std::thread* _threadRun;
	};
}

#endif