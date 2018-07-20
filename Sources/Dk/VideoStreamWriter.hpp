#ifndef VIDEO_STREAM_WRITER_HPP
#define VIDEO_STREAM_WRITER_HPP

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
	class VideoStreamWriter {
	public:
		// Constructors
		VideoStreamWriter(ManagerConnection& managerConnection, const int port);
		~VideoStreamWriter();
		
		// Methods
		bool startBroadcast(const cv::Mat& frameInit);
		void update(const cv::Mat& newFrame);
		void release();
		
		// Thread launch methods
		void handleClients();
		void handleCompress();
		
		// Getters
		bool isValide() const;
		
	private:
		// Methods
		void _handleClient(int idClient);
		bool _treatClient(int idClient, const size_t action);
		void _compressFrame(const cv::Mat& frame);
		
		// Members
		std::shared_ptr<Server> _server;
		tjhandle _jpegCompressor;
		unsigned char* _buff;
		unsigned char* _buffTmp;
		unsigned long _bufSize;
		unsigned long _bufSizeTmp;
		bool _valide;
		
		std::atomic<bool> _atomRunning{false};
		std::atomic<bool> _atomImageUpdated{false};
		
		std::mutex _mutexBuffer;
		std::thread* _threadClients;
		std::thread* _threadCompress;
		
		int _widthFrame;
		int _heightFrame;
		int _channelFrame;
		cv::Mat _frameToCompress;
	};
}

#endif