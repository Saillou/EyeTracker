#ifndef VIDEO_STREAM_WRITER_HPP
#define VIDEO_STREAM_WRITER_HPP

#include <iostream>
#include <thread>
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
	class VideoStreamWriter {
	public:
		// Constructors
		VideoStreamWriter(ManagerConnection& managerConnection, const int port);
		~VideoStreamWriter();
		
		// Methods
		bool startBroadcast(const cv::Mat& frameInit);
		void update(const cv::Mat& newFrame);
		void release();
		
		void handleClients();
		
		// Getters
		bool isValide() const;
		
	private:
		// Methods
		void _handleClient(int idClient);
		
		// Members
		std::shared_ptr<Server> _server;
		tjhandle _jpegCompressor;
		unsigned char* _buff;
		unsigned long _bufSize;
		bool _valide;
		
		std::mutex _mutexRun;
		volatile bool _running;
		
		std::mutex _mutexFrame;
		cv::Mat _frameToPublish;
		
		std::thread* _threadClients;
	};
}

#endif