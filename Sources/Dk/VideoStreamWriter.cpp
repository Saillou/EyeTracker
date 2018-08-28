#include "VideoStreamWriter.hpp"
#include "CvVideoCaptureProperties.hpp" // Link

using namespace Dk;
using namespace Protocole;

// Constructors
VideoStreamWriter::VideoStreamWriter(ManagerConnection& managerConnection, const int port) :
	_server(managerConnection.createServer(Socket::TCP, Socket::BLOCKING, port)),
	_jpegCompressor(tjInitCompress()),
	_buff(nullptr),				// Buffer sent
	_buffTmp(tjAlloc(10000)), 	// Random init, tj will carry on the allocation
	_bufSize(0), 				// Final size from tmp
	_bufSizeTmp(0), 			// Real size of the buffer use by tj
	_valide(false),
	_threadClients(nullptr),
	_threadCompress(nullptr),
	_format(/*Height*/0, /*Width*/0, /*Channels*/0),
	_pCam(nullptr),
	_camProp(nullptr)
{
	_valide = _server != nullptr && _jpegCompressor != NULL;
}
VideoStreamWriter::~VideoStreamWriter() {
	release();
	
	if(_bufSize > 0)
		free(_buff);
	
	if(_jpegCompressor != NULL) {
		tjFree(_buffTmp);
		tjDestroy(_jpegCompressor);	
	}
	
	if(_threadCompress != nullptr)
		delete _threadCompress;	
	
	if(_threadClients != nullptr)
		delete _threadClients;
}

// Methods
void VideoStreamWriter::handleClients() {
	std::cout << "Wait for clients";
	
	while(_atomRunning) {	
		int idClient = _server->waitClient(5);
		
		if(idClient <= 0) {			
			std::cout << ".";
			Chronometre::wait(350);
		}
		else {
			_handleClient(idClient);
		}	
		
		// Update state		
		if(_atomRunning && idClient > 0)
			std::cout << "Wait for clients";
	} 
}

void VideoStreamWriter::_handleClient(int idClient) {
	std::cout << std::endl << "Handle new client " << idClient << std::endl;
	
	// -- Handle client --
	bool clientWantToQuit = false;
	do {		
		// Deals with the socket
		BinMessage msg;
		if(!_server->read(msg, idClient)) 
			break;	// Error, better to disconnect

		// Treat client
		clientWantToQuit = _treatClient(idClient, msg);
	} while(!clientWantToQuit);
	
	_server->closeSocket(idClient);
	std::cout << "Client disconnected." << std::endl;	
}

bool VideoStreamWriter::_treatClient(const int idClient, const BinMessage& msgIn) {
	BinMessage msgOut;
	bool clientWantToQuit = false;
	
	switch(msgIn.getAction()) {
		// Client quit
		case BIN_QUIT:
			clientWantToQuit = true;
		break;
		
		// Frame info asked - change requested
		case BIN_INFO:	
		{
			// Changes are requested
			if(msgIn.getSize() > 0) 
				_changeFormat(FormatStream(CmdMessage(Message::To_string(msgIn.getData())))); // clojure style lol
			
			// Send format
			msgOut.set(BIN_MCMD, Message::To_string(_format.toCmd().serialize()));
			_server->write(msgOut, idClient);
		}
		break;
		
		// Send a frame
		case BIN_GAZO: 
			_mutexBuffer.lock();
			msgOut.set(BIN_GAZO, (size_t)_bufSize, (const char*)_buff);
			_mutexBuffer.unlock();
			
			// Write the message						
			_server->write(msgOut, idClient);
		break;
	}
	
	return clientWantToQuit;
}

void VideoStreamWriter::handleCompress() {
	while(_atomRunning) {			
		if(_atomImageUpdated) {
			// Compress
			_compressFrame(_frameToCompress);
			_atomImageUpdated = false;
		}
		
		// Update state
		Chronometre::wait(1);
	}	
}

void VideoStreamWriter::_compressFrame(const cv::Mat& frame) {	
	tjCompress2 (
		_jpegCompressor, 
		frame.data, 	// ptr to data, const uchar *
		frame.cols, 	// width
		TJPAD(frame.cols * tjPixelSize[TJPF_BGR]), // bytes per line
		frame.rows,	// height
		TJPF_BGR, 	// pixel format
		&_buffTmp, 	// ptr to buffer, unsigned char **
		&_bufSizeTmp, 
		TJSAMP_420,	// chrominace sub sampling
		80, 		// quality: 0-100
		0 			// flags: osef
	);	
	
	// Copy to displayed buffer
	_mutexBuffer.lock();
	_bufSize 	= _bufSizeTmp;
	_buff 		= reinterpret_cast<unsigned char*>(realloc(_buff, _bufSize));
	memmove(_buff, _buffTmp, _bufSize);
	_mutexBuffer.unlock();
}


const Protocole::FormatStream& VideoStreamWriter::startBroadcast(std::shared_ptr<cv::VideoCapture> pCam) {	
	// Members initialized and not already working
	if(!_valide || _pCam)
		return _format; // Empty, can be evaluated to false
	_pCam.swap(pCam);
	
	// Read camera format
	_camProp = std::make_shared<CvProperties::CaptureProperties>(_pCam, Dk::CvProperties::Camera);
	
	_format.width 	 	= static_cast<int>(_camProp->get(cv::CAP_PROP_FRAME_WIDTH).value);
	_format.height 	 	= static_cast<int>(_camProp->get(cv::CAP_PROP_FRAME_HEIGHT).value);
	_format.channels 	= 3;
	_format.fps 		= static_cast<int>(_camProp->get(cv::CAP_PROP_FPS).value);
	_format.hue 		= _camProp->get(cv::CAP_PROP_HUE).value;
	_format.saturation 	= _camProp->get(cv::CAP_PROP_SATURATION).value;
	_format.brightness 	= _camProp->get(cv::CAP_PROP_BRIGHTNESS).value;
	_format.contrast 	= _camProp->get(cv::CAP_PROP_CONTRAST).value;
	_format.exposure 	= _camProp->get(cv::CAP_PROP_EXPOSURE).value;
	_format.autoExposure = _camProp->get(cv::CAP_PROP_AUTO_EXPOSURE).value;
	
	// Init buffer - create a first (black) frame
	cv::Mat frameInit = cv::Mat::zeros(_format.height, _format.width, _format.channels == 1 ? CV_8UC1 : CV_8UC3);
	
	_compressFrame(frameInit);
	_atomRunning = true;
	
	if(_threadClients == nullptr)
		_threadClients = new std::thread(&VideoStreamWriter::handleClients, this);	
	
	if(_threadCompress == nullptr)
		_threadCompress = new std::thread(&VideoStreamWriter::handleCompress, this);
	
	return _format;
}

bool VideoStreamWriter::_changeFormat(const Protocole::FormatStream& format) {
	// Members initialized and working
	if(!_valide || !_pCam || !_camProp)
		return false;
		
	// Cannot change frame parameters
	if(format.height != _format.height || format.width != _format.width || format.channels != _format.channels)
		return false;
	
	// Change camera properties	
	if(_format.hue != format.hue)
		if(_camProp->set(cv::CAP_PROP_HUE, format.hue))
			_format.hue = format.hue;
			
	if(_format.saturation != format.saturation)
		if(_camProp->set(cv::CAP_PROP_SATURATION, format.saturation))
			_format.saturation = format.saturation;
			
	if(_format.brightness != format.brightness)
		if(_camProp->set(cv::CAP_PROP_BRIGHTNESS, format.brightness))
			_format.brightness = format.brightness;
			
	if(_format.contrast	!= format.contrast)
		if(_camProp->set(cv::CAP_PROP_CONTRAST, format.contrast))
			_format.contrast = format.contrast;
			
	if(_format.exposure	!= format.exposure)
		if(_camProp->set(cv::CAP_PROP_EXPOSURE, format.exposure))
			_format.exposure = format.exposure;
		
	if(_format.autoExposure	!= format.autoExposure)
		if(_camProp->set(cv::CAP_PROP_AUTO_EXPOSURE, format.autoExposure))
			_format.autoExposure = format.autoExposure;
	
	return true;
}

const cv::Mat VideoStreamWriter::update() {
	cv::Mat newFrame;
	
	// Read camera
	if(!_pCam || !_pCam->isOpened())
		return newFrame;
	_pCam->grab();
	_pCam->retrieve(newFrame);
	
	// Should be same format as the inital one
	if(newFrame.cols != _format.width || newFrame.rows != _format.height || newFrame.channels() != _format.channels)
		return newFrame;
	
	// Everything ok ? Send to compression pipe
	_frameToCompress 	= newFrame.clone();
	_atomImageUpdated 	= true;	
	
	return newFrame;
}
void VideoStreamWriter::release() {
	_atomRunning = false;	
	_valide = false;
	
	if(_threadCompress != nullptr)
		if(_threadCompress->joinable())
			_threadCompress->join();
	
	if(_threadClients != nullptr)
		if(_threadClients->joinable())
			_threadClients->join();
		
	if(_pCam && _pCam->isOpened())
		_pCam->release();		
}

// Getters
bool VideoStreamWriter::isValide() const {
	return _valide;
}
