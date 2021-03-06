#ifndef VIDEO_STREAM_HPP
#define VIDEO_STREAM_HPP

#include <iostream>
#include <thread>
#include <atomic>
#include <mutex>

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
		const Protocole::FormatStream& initFormat();
		bool release();
		
		bool play();
		bool pause();
		bool stop();
		
		void run();
		
		// Setters
		bool setFormat(const Protocole::FormatStream& format);
		
		// Getters
		RunningState getState() const;
		double getFpsRate() const;
		double getLag() const;
		bool isValide() const;
		
		const Protocole::FormatStream& getFormat() const;
		const cv::Mat getFrame();
		
	private:
		// Methods
		bool _askFrame();
		
		// Members
		ManagerConnection::IpAdress _ipServer;
		std::shared_ptr<Socket> 	_sock;
		tjhandle 					_jpegDecompressor;
		
		cv::Mat _frame;
		cv::Mat _frameCpy;
		Protocole::FormatStream _format;
		
		bool _valide;
		
		// Thread share
		std::atomic<RunningState> _atomState{NOT_DEFINED};
		std::atomic<double> _atomFps{0.0};
		std::atomic<double> _atomLag{0.0};
		
		std::thread* _threadRun;
		std::mutex _mutexSock;
		std::mutex _mutexFrameCpy;
	};
}

#endif