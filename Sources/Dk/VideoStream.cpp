#include "VideoStream.hpp"

using namespace Dk;
using namespace Protocole;

// Constructors
VideoStream::VideoStream(ManagerConnection& managerConnection, const ManagerConnection::IpAdress& ipServer) :
	_ipServer(ipServer),
	_sock(managerConnection.connectTo(Socket::TCP, Socket::BLOCKING, ipServer.toString(), ipServer.getPort())),
	_jpegDecompressor(tjInitDecompress()),
	_frame(cv::Mat()),
	_valide(false),
	_threadRun(nullptr)
{
	// Check intialization
	if(_sock == nullptr) 
		std::cout << "Not connected." << std::endl;
	
	if(_jpegDecompressor == NULL)
		std::cout << "Libjpeg-turbo not loaded" << std::endl;
	
	_valide = _sock != nullptr && _jpegDecompressor != NULL;
}
VideoStream::~VideoStream() {
	if(_atomState != STOPPED)
		release();
	
	if(_jpegDecompressor != NULL)
		tjDestroy(_jpegDecompressor);
	
	if(_threadRun != nullptr)
		delete _threadRun;
}

// Methods
void VideoStream::run() {
	Chronometre chronoLag;
	Chronometre chronoUpdate;

	int nbFrame = 0;
	double totalLag = 0;
	
	while(_atomState != STOPPED) {		
		chronoLag.beg();
		if(_atomState == PLAYING) {
			if(!_askFrame())
				break;
			else 
				nbFrame++;
		}
		chronoLag.end();
		totalLag += chronoLag.ms();
		
		if(chronoUpdate.elapsed_ms() > 1000) { // Update info
			double fps = 1000.0 * nbFrame / chronoUpdate.elapsed_ms();
			double lag = nbFrame > 0 ? totalLag / nbFrame : 0.0;
			
			_atomFps = fps;			
			_atomLag = lag;
			
			chronoUpdate.beg();
			nbFrame 	= 0;
			totalLag 	= 0;
		}
		
		// Wait 1ms
		cv::waitKey(1);
	}
}

bool VideoStream::initFormat() {
	if(!_valide)
		return false;
	
	// Ask frame info	 and read answer
	BinMessage msg;
	msg.set(BIN_INFO, "");

	if(_sock->write(msg) && _sock->read(msg)) {
		CmdMessage cmd(Message::To_string(msg.getData()));
		const int WIDTH 	= (int)Message::To_unsignedInt(cmd.getCommand(CMD_WIDTH).second);
		const int HEIGHT 	= (int)Message::To_unsignedInt(cmd.getCommand(CMD_HEIGHT).second);
		const int CHANNEL 	= (int)Message::To_unsignedInt(cmd.getCommand(CMD_CHANNEL).second);
		
		if(WIDTH*CHANNEL*HEIGHT > 0) {
			_frame = cv::Mat::zeros(HEIGHT, WIDTH, CHANNEL == 1 ? CV_8UC1 : CV_8UC3);
			return true;
		}
	}
	
	return false;
}

bool VideoStream::play() {
	if(_threadRun == nullptr)
		_threadRun = new std::thread(&VideoStream::run, this);
	
	_atomState = PLAYING;
	
	return _valide;
}
bool VideoStream::pause() {
	_atomState = PAUSED;

	return _valide;
}
bool VideoStream::stop() {
	return release();
}

bool VideoStream::_askFrame() {
	if(!_valide || _frame.size().area() == 0)
		return false;
	
	// Ask Frame	
	BinMessage msg;
	msg.set(BIN_GAZO, "");
	
	if(!_sock->write(msg)) 
		return false;
	
	// Answer
	if(_sock->read(msg)) {
		if(msg.getSize() == 0) 
			return false;
		
		tjDecompress2(_jpegDecompressor, (const unsigned char*)msg.getData().data(), (unsigned long)msg.getSize(), _frame.data, _frame.cols, 0, _frame.rows, _frame.channels() == 1 ? TJPF_GRAY : TJPF_BGR, TJFLAG_FASTDCT);
		if(!_frame.empty()) {
			cv::imshow("Frame", _frame);
			return true; // Emit
		}
	}
	
	return false;
}

bool VideoStream::release() {	
	// Quit
	_atomState = STOPPED;
	_valide = false;
	
	if(_threadRun != nullptr)
		if(_threadRun->joinable())
			_threadRun->join();
	
	if(_sock != nullptr) {
		BinMessage msg;
		msg.set(BIN_QUIT, "");
		return _sock->write(msg);
	}
	return true;
}

// Getters
RunningState VideoStream::getState() const {
	return _atomState;
}
double VideoStream::getFpsRate() const {
	return _atomFps;	
}
double VideoStream::getLag() const {
	return _atomLag;	
}
bool VideoStream::isValide() const {
	return _valide;
}

