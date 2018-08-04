#include "VideoStreamWriter.hpp"
#include "CvVideoCaptureProperties.hpp"

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
	_format(/*Height*/0, /*Width*/0, /*Channels*/0)
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
		clientWantToQuit = _treatClient(idClient, msg.getAction());
	} while(!clientWantToQuit);
	
	_server->closeSocket(idClient);
	std::cout << "Client disconnected." << std::endl;	
}

bool VideoStreamWriter::_treatClient(const int idClient, const size_t action) {
	BinMessage msg;
	bool clientWantToQuit = false;
	
	switch(action) {
		// Client quit
		case BIN_QUIT:
			clientWantToQuit = true;
		break;
		
		// Frame info asked
		case BIN_INFO:
		{ 
			CmdMessage cmd;
			cmd.addCommand(CMD_HEIGHT, 	std::to_string(_format.height));
			cmd.addCommand(CMD_WIDTH, 	std::to_string(_format.width));
			cmd.addCommand(CMD_CHANNEL,	std::to_string(_format.channels));
			
			msg.set(BIN_MCMD, Message::To_string(cmd.serialize()));
			_server->write(msg, idClient);
		}
		break;
		
		// Send a frame
		case BIN_GAZO: 
			_mutexBuffer.lock();
			msg.set(BIN_GAZO, (size_t)_bufSize, (const char*)_buff);
			_mutexBuffer.unlock();
			
			// Write the message						
			_server->write(msg, idClient);
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
		TJPF_BGR, 		// pixel format
		&_buffTmp, 	// ptr to buffer, unsigned char **
		&_bufSizeTmp, 
		TJSAMP_420,	// chrominace sub sampling
		80, 				// quality, int
		0 					// flags
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
	CvProperties::CaptureProperties camProp(_pCam, Dk::CvProperties::Camera);
	
	_format.width 	 = static_cast<int>(camProp.get(cv::CAP_PROP_FRAME_WIDTH).value.manual);
	_format.height 	 = static_cast<int>(camProp.get(cv::CAP_PROP_FRAME_HEIGHT).value.manual);
	_format.channels = 3;
	
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
const cv::Mat VideoStreamWriter::update() {
	cv::Mat newFrame;
	
	// Read camera
	if(!_pCam || !_pCam->isOpened())
		return newFrame;
	
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
