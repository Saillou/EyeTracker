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
	_state(RunningState::NOT_DEFINED),
	_mutexState(std::mutex()),
	_fps(0.0),
	_mutexFps(std::mutex()),
	_lag(0.0),
	_mutexLag(std::mutex()),
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
	_mutexState.lock();
	auto retState = _state;
	_mutexState.unlock();
	
	if(retState != STOPPED)
		release();
	
	if(_jpegDecompressor != NULL)
		tjDestroy(_jpegDecompressor);
}

// Methods
void VideoStream::run() {
	_mutexState.lock();
	auto retState = _state;
	_mutexState.unlock();
	
	Chronometre chrono;
	clock_t c0Info = clock();
	int nbFrame = 0;
	double totalLag = 0;
	
	while(retState != STOPPED) {
		// Get state update
		_mutexState.lock();
		retState = _state;
		_mutexState.unlock();
		
		chrono.beg();
		
		if(retState == PLAYING) {
			if(!_askFrame())
				break;
			else 
				nbFrame++;
		}
		
		chrono.end();
		totalLag += chrono.ms();
		
		clock_t nowC = clock();
		
		if(nowC - c0Info >= 1000) { // Update info
			double fps = 1000.0*nbFrame / (nowC - c0Info);
			double lag = nbFrame > 0 ? totalLag / nbFrame : 0.0;
			
			_mutexFps.lock();
			_fps = fps;
			_mutexFps.unlock();			
			
			_mutexLag.lock();
			_lag = lag;
			_mutexLag.unlock();
			
			c0Info 		= nowC;
			nbFrame 	= 0;
			totalLag 	= 0;
		}
		
		// Wait 10ms
		cv::waitKey(10);
	}
}

bool VideoStream::initFormat() {
	if(!_valide)
		return false;
	
	// Ask frame info		
	BinMessage msg;
	msg.set(BIN_INFO, "");
	_sock->write(msg);
	
	// Answer	
	if(_sock->read(msg)) {
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
	
	_mutexState.lock();
	_state = PLAYING;
	_mutexState.unlock();
	
	return _valide;
}
bool VideoStream::pause() {
	_mutexState.lock();
	_state = PAUSED;
	_mutexState.unlock();

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
	_mutexState.lock();
	_state = STOPPED;
	_mutexState.unlock();

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
RunningState VideoStream::getState() {
	_mutexState.lock();
	auto res = _state;
	_mutexState.unlock();
	
	return res;
}
double VideoStream::getFpsRate() {
	_mutexFps.lock();
	auto res = _fps;
	_mutexFps.unlock();
	
	return res;	
}
double VideoStream::getLag() {
	_mutexLag.lock();
	auto res = _lag;
	_mutexLag.unlock();
	
	return _lag;	
}
bool VideoStream::isValide() const {
	return _valide;
}

