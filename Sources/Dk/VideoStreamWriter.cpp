#include "VideoStreamWriter.hpp"

using namespace Dk;
using namespace Protocole;

// Constructors
VideoStreamWriter::VideoStreamWriter(ManagerConnection& managerConnection, const int port) :
	_server(managerConnection.createServer(Socket::TCP, Socket::BLOCKING, port)),
	_jpegCompressor(tjInitCompress()),
	_buff(tjAlloc(10000)), // Random init, tj will carry on the allocation
	_bufSize(0), 				// Real size of the buffer use by tj
	_valide(false),
	_running(false),
	_frameToPublish(cv::Mat()),
	_threadClients(nullptr)
{
	_valide = _server != nullptr && _jpegCompressor != NULL;
}
VideoStreamWriter::~VideoStreamWriter() {
	release();
	
	if(_jpegCompressor != NULL) {
		tjFree(_buff);
		tjDestroy(_jpegCompressor);	
	}
	
	if(_threadClients != nullptr)
		delete _threadClients;
}

// Methods
void VideoStreamWriter::handleClients() {
	std::cout << "Wait for clients";

	_mutexRun.lock();
	auto retRun = _running;
	_mutexRun.unlock();
	
	while(retRun) {	
		int idClient = _server->waitClient(5);
		
		if(idClient <= 0) {			
			std::cout << ".";
			Chronometre::wait(350);
		}
		else {
			_handleClient(idClient);
		}	
		
		// Update state
		_mutexRun.lock();
		retRun = _running;
		_mutexRun.unlock();
		
		if(retRun && idClient > 0)
			std::cout << "Wait for clients";
	} 
}

void VideoStreamWriter::_handleClient(int idClient) {
	std::cout << std::endl << "Handle new client " << idClient << std::endl;
	BinMessage msg;
	
	// -- Handle client --
	std::vector<std::shared_ptr<Server::ThreadWrite>> threadsRunning;

	bool clientQuitted = false;
	while(!clientQuitted) {
		// Get the frame
		cv::Mat frame;
		
		_mutexFrame.lock();
		if(!_frameToPublish.empty())		
			frame = _frameToPublish.clone();
		_mutexFrame.unlock();
		
		if(frame.empty())
			continue;
		
		// Deals with the socket
		if(!_server->read(msg, idClient)) // Error, better to break
			break;
		
		switch(msg.getAction()) {
			// Client quit
			case BIN_QUIT:
				clientQuitted = true;
			break;
			
			// Frame info asked
			case BIN_INFO:
			{ 
				CmdMessage cmd;
				cmd.addCommand(CMD_HEIGHT, 	std::to_string(frame.rows));
				cmd.addCommand(CMD_WIDTH, 	std::to_string(frame.cols));
				cmd.addCommand(CMD_CHANNEL,	std::to_string(frame.channels()));
				
				msg.set(BIN_MCMD, Message::To_string(cmd.serialize()));
				_server->write(msg, idClient);
			}
			break;
			
			// Send a frame
			case BIN_GAZO: 
				tjCompress2 (
					_jpegCompressor, 
					frame.data, 	// ptr to data, const uchar *
					frame.cols, 	// width
					TJPAD(frame.cols * tjPixelSize[TJPF_BGR]), // bytes per line
					frame.rows,	// height
					TJPF_BGR, 		// pixel format
					&_buff, 		// ptr to buffer, unsigned char **
					&_bufSize, 	// ptr to buffer size, unsigned long *
					TJSAMP_420,	// chrominace sub sampling
					80, 				// quality, int
					0 					// flags
				);
				msg.set(BIN_GAZO, (size_t)_bufSize, (const char*)_buff);
				
				// Write the message						
				threadsRunning.push_back(std::make_shared<Server::ThreadWrite>(_server, msg, idClient));
				//_server->write(msg, idClient);

			break;
		} // Switch action
		
		// Delete threads that are finished (<=> inactive)
		threadsRunning.erase(
			std::remove_if(threadsRunning.begin(), threadsRunning.end(), [](const std::shared_ptr<Server::ThreadWrite>& tw) {
				return !tw->isActive();
		}), threadsRunning.end());

	} // clientQuitted or error occured
	
	_server->closeSocket(idClient);
	
	threadsRunning.clear();
	std::cout << "Client disconnected." << std::endl;	
}

bool VideoStreamWriter::startBroadcast(const cv::Mat& frameInit) {
	if(!_valide)
		return false;
	
	_mutexRun.lock();
	_running = true;
	_mutexRun.unlock();
	
	if(_threadClients == nullptr)
		_threadClients = new std::thread(&VideoStreamWriter::handleClients, this);
	
	return true;
}
void VideoStreamWriter::update(const cv::Mat& newFrame) {
	_mutexFrame.lock();
	_frameToPublish = newFrame.clone();
	_mutexFrame.unlock();
}
void VideoStreamWriter::release() {
	_mutexRun.lock();
	_running = false;
	_mutexRun.unlock();
	
	_valide = false;
	if(_threadClients != nullptr)
		if(_threadClients->joinable())
			_threadClients->join();
		
}

// Getters
bool VideoStreamWriter::isValide() const {
	return _valide;
}
