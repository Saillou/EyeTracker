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
		close(_idSocket);
	}
}

// Methods
bool Socket::initialize(const CONNECTION_TYPE type)	{
	// Type define id
	_type = type;	
	
	if(_type == NONE) {
		std::cout << "Please, set a type for socket." << std::endl;
		return false;
	}
	else	if(_type == TCP) {
		_idSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
		
		// Create echo
		struct sockaddr_in clientEcho;
		memset(&clientEcho, 0, sizeof(clientEcho));
		
		clientEcho.sin_addr.s_addr	= inet_addr(_ipAdress.c_str());
		clientEcho.sin_port				= htons(_port);	
		clientEcho.sin_family		 	= AF_INET;
		
		// Try connection
		if(connect(_idSocket, (struct sockaddr *)&clientEcho, sizeof(clientEcho)) == SOCKET_ERROR) {
			std::cout << "Could not reach server." << std::endl;
			return false;
		}
	}
	else	if(_type == UDP) {
		_idSocket = socket(PF_INET, SOCK_DGRAM, 0);
	}
	else {
		_type = NONE;
		std::cout << "Type not recognized for socket." << std::endl;
		return false;
	}
	
	return true;
}
bool Socket::read(Protocole::BinMessage& msg, int idSocket) const {
	try {
		// Check
		if(_idSocket <= 0 || _type == NONE) {
			std::cout << "Socket not connected." << std::endl;
			return false;
		}
		
		if(idSocket <= 0)
			idSocket = _idSocket;
		
		if(_type ==TCP) {
			// -- Read message --
			int received 			= -1;
			size_t messageSize = 0;
			size_t messageCode = 0;
			char* buffer 			=	nullptr;
			
			// Message size
			received 	= -1;
			buffer 	= (char*)realloc(buffer, Protocole::BinMessage::SIZE_SIZE * sizeof(char));
			
			if((received = recv(idSocket, buffer, Protocole::BinMessage::SIZE_SIZE, 0)) == (int)Protocole::BinMessage::SIZE_SIZE) {
				messageSize = Protocole::BinMessage::Read_256(buffer, Protocole::BinMessage::SIZE_SIZE);
			}

			// Message code
			received 	= -1;
			buffer 	= (char*)realloc(buffer, Protocole::BinMessage::SIZE_CODE * sizeof(char));
			
			if((received = recv(idSocket, buffer, Protocole::BinMessage::SIZE_CODE, 0)) == (int)Protocole::BinMessage::SIZE_CODE) {
				messageCode = Protocole::BinMessage::Read_256(buffer, Protocole::BinMessage::SIZE_CODE);
			}
			
			// Message data
			size_t already_read = 0;
			received 	= -1;
			buffer 	= (char*)realloc(buffer, messageSize * sizeof(char));
			
			while(already_read < messageSize) {
				if((received = recv(idSocket, buffer + already_read, BUFFER_SIZE_MAX, 0)) > 0) 
					already_read += received;
				else 
					break;
			}
			msg.set(messageCode, messageSize, buffer);
			
			free(buffer);
			
			// Result
			return msg.isValide();
		}
		else {
			return false;
		}
	}
	catch(...) {
		std::cout << "Read error" << std::endl;
		return false;
	}
}
bool Socket::write(Protocole::BinMessage& msg, int idSocket) const {
	try {
		// Check
		if(_idSocket <= 0 || _type == NONE) {
			std::cout << "Socket not connected." << std::endl;
			return false;
		}
		
		if(idSocket <= 0)
			idSocket = _idSocket;
		
		if(_type == TCP) {
			auto message = msg.serialize();
			int sended = (int)message.size();
			
			return (send(idSocket, message.data(), sended, 0) == sended);
		}
		else if(_type == UDP) {
			return false;
		}
		
		return false;
	}
	catch(...) {
		std::cout << "Write error" << std::endl;
		return false;
	}
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