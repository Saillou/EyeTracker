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
	namespace CvProperties {
		class CaptureProperties; // forward
	}
	
	class VideoStreamWriter {
	public:
		// Constructors
		VideoStreamWriter(ManagerConnection& managerConnection, const int port);
		~VideoStreamWriter();
		
		// Methods
		const Protocole::FormatStream& startBroadcast(std::shared_ptr<cv::VideoCapture> pCam);
		const cv::Mat update();
		void release();
		
		// Thread launch methods
		void handleClients();
		void handleCompress();
		
		// Getters
		bool isValide() const;
		
	private:
		// Methods
		void _handleClient(int idClient);
		bool _treatClient(int idClient, const Protocole::BinMessage& msg);
		void _compressFrame(const cv::Mat& frame);
		bool _changeFormat(const Protocole::FormatStream& newFormat);
		
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
		
		Protocole::FormatStream _format;
		cv::Mat _frameToCompress;
		std::shared_ptr<cv::VideoCapture> _pCam;
		std::shared_ptr<CvProperties::CaptureProperties> _camProp;
	};
}

#endif