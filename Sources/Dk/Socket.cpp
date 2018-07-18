#include "Socket.hpp"

// Constructors
Socket::Socket(const std::string& ipAdress, const int port) :
	_ipAdress(ipAdress),
	_port(port),
	_idSocket(-1),
	_type(NONE)
{
	// Nothing else to do
}

Socket::~Socket() {
	if(_idSocket > 0) {
		shutdown(_idSocket, CLOSE_ER); // No emission or reception
#ifndef USE_MSVC 
		close(_idSocket);
#else
	_idSocket = -1;
#endif
	}
}

// Methods
bool Socket::initialize(const CONNECTION_TYPE type, const CONNECTION_MODE mode)	{
	// Type define id
	_type = type;	
	_mode = mode;
	
	if(_type == NONE) 
		return false;
	else	if(_type == TCP)
		_idSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	else	if(_type == UDP) 
		_idSocket = socket(PF_INET, SOCK_DGRAM, 0);
	else {
		_type = NONE;
		std::cout << "[Socket] Type not recognized." << std::endl;
		return false;
	}
	
	// Create echo
	struct sockaddr_in clientEcho;
	memset(&clientEcho, 0, sizeof(clientEcho));
	
	clientEcho.sin_addr.s_addr	= inet_addr(_ipAdress.c_str());
	clientEcho.sin_port				= htons(_port);	
	clientEcho.sin_family		 	= AF_INET;
	
	if(_mode == NOT_BLOCKING)
		_changeMode(mode);
	
	// Try to connect	
	if(connect(_idSocket, (struct sockaddr *)&clientEcho, sizeof(clientEcho)) == SOCKET_ERROR) {
		bool criticError = true;
		
		#ifdef _WIN32
			int error = WSAGetLastError();
			
			switch(error) {
				case WSAEWOULDBLOCK: // Only triggered during not_blocking operations
					std::cout << "Connecting..." << std::endl;
					
					{ // Wait to be connected and check writable
						auto info = waitForAccess(5);
						criticError = info.errorCode <= 0 || !info.writable;
					}
				break;
				
				case WSAEISCONN:
					std::cout << "Socket is already connected." << std::endl;
					criticError = false;
				break;
				
				case WSAECONNREFUSED:
					std::cout << "Connection refused. " << std::endl;
				break;
				
				default:
					std::cout << "Error not treated: " << error << std::endl;
				break;
			}
		#endif
		
		if(criticError) {
			std::cout << "Could not reach server." << std::endl;
			return false;
		}
	}
	
	return true;
}
bool Socket::read(Protocole::BinMessage& msg, int idSocket) const {
	// Check
	if(_idSocket <= 0 || _type == NONE) {
		std::cout << "Socket not connected." << std::endl;
		return false;
	}
	
	if(idSocket <= 0)
		idSocket = _idSocket;
	
	if(_type == TCP || _type == UDP) {
		// -- Read message --
		int received 			= -1;
		size_t messageSize = 0;
		size_t messageCode = 0;
		char* buffer 			=	nullptr;
		
		// Message size
		received = -1;
		buffer 		= (char*)realloc(buffer, Protocole::BinMessage::SIZE_SIZE * sizeof(char));
		
		if((received = recv(idSocket, buffer, Protocole::BinMessage::SIZE_SIZE, 0)) == (int)Protocole::BinMessage::SIZE_SIZE) {
			messageSize = Protocole::BinMessage::Read_256(buffer, Protocole::BinMessage::SIZE_SIZE);
		}
		else // Error size
			return false;

		// Message code
		received = -1;
		buffer = (char*)realloc(buffer, Protocole::BinMessage::SIZE_CODE * sizeof(char));
		
		if((received = recv(idSocket, buffer, Protocole::BinMessage::SIZE_CODE, 0)) == (int)Protocole::BinMessage::SIZE_CODE) {
			messageCode = Protocole::BinMessage::Read_256(buffer, Protocole::BinMessage::SIZE_CODE);
		}
		else // Error size
			return false;
		
		// Message data
		size_t already_read = 0;
		received 	= -1;
		buffer 	= (char*)realloc(buffer, messageSize * sizeof(char));
		
		while(already_read < messageSize) {
			int still = messageSize - already_read;
			if((received = recv(idSocket, buffer + already_read, still > BUFFER_SIZE_MAX ? BUFFER_SIZE_MAX : still, 0)) > 0) 
				already_read += received;
			else 
				break;
		}

		msg.set(messageCode, messageSize, buffer);
		free(buffer);
		
		
		// Result
		return msg.isValide();
	}
	
	return false;
}
bool Socket::write(Protocole::BinMessage& msg, int idSocket) const {
	// Check
	if(_idSocket <= 0 || _type == NONE) {
		std::cout << "Socket not connected." << std::endl;
		return false;
	}
	
	if(idSocket <= 0)
		idSocket = _idSocket;
	
	if(_type == TCP || _type == UDP) {
		auto message = msg.serialize();
		int sended = (int)message.size();
		
		return (send(idSocket, message.data(), sended, 0) == sended);
	}
	
	return false;
}
	
Socket::Accessiblity Socket::waitForAccess(unsigned long timeoutMs) const {
	Socket::Accessiblity access{false, false, 0};
	
	// Wait using select
	const timeval timeout {
		/* timeout.tv_sec = */(long)(timeoutMs / (long)1e3),
		/* timeout.tv_usec = */(long)(timeoutMs * (long)1e3) % (long)1e6
	};
	fd_set bkRead, bkWrite, bkErr;
	FD_ZERO(&bkRead);
	FD_ZERO(&bkWrite);
	FD_ZERO(&bkErr);
	
	FD_SET(_idSocket, &bkRead);
	FD_SET(_idSocket, &bkWrite);
	FD_SET(_idSocket, &bkErr);
	
	access.errorCode = select(0, &bkRead, &bkWrite, &bkErr, &timeout);
	if(access.errorCode > 0) { // No errors
		access.writable = FD_ISSET(_idSocket, &bkWrite);
		access.readable = FD_ISSET(_idSocket, &bkRead);
	}
	
	return access;
}

// Setters


// Getters
const std::string& Socket::getIpAdress() const {
	return _ipAdress;
}
const int& Socket::getPort() const {
	return _port;
}
const int& Socket::getId() const {
	return _idSocket;
}
const Socket::CONNECTION_TYPE& Socket::getType() const {
	return _type;
}


int Socket::_changeMode(const CONNECTION_MODE mode) {
#ifdef __linux__ 
	// Use the standard POSIX 
	int oldFlags = fcntl(_idSocket, F_GETFL, 0);
	int flags = (mode == NOT_BLOCKING) ? oldFlags | O_NONBLOCK : oldFlags & ~O_NONBLOCK;
	return fcntl(_idSocket, F_SETFL, flags);
	
#elif _WIN32
	unsigned long ul = (mode == NOT_BLOCKING) ? 1 : 0; // Parameter for FIONBIO
	return ioctlsocket(_idSocket, FIONBIO, &ul);
#endif
}

