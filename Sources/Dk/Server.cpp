#include "Server.hpp"

// Constructors
Server::Server(const int port, const size_t maxPending) : 
	Socket("", port), 
	_maxPending(maxPending) 
{
	// Nothing else to do
}

Server::~Server() {
}

// Methods
bool Server::initialize(const CONNECTION_TYPE type)	{
	_type = type;
	
	// Type define id
	if(_type == NONE) {
		std::cout << "Please, set a type for server." << std::endl;
		return false;
	}
	else	if(_type == TCP) {
		_idSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	}
	else	if(_type == UDP) {
		_idSocket = socket(PF_INET, SOCK_DGRAM, 0);
	}
	else {
		_type = NONE;
		std::cout << "Type not recognized for server." << std::endl;
		return false;
	}
	
	// Create echo
	struct sockaddr_in  serverEcho;
	memset(&serverEcho, 0, sizeof(serverEcho));
	
	serverEcho.sin_family		 	= AF_INET;
	serverEcho.sin_addr.s_addr	= htonl(INADDR_ANY);
	serverEcho.sin_port				= htons(_port);	
	
	// Try bind
	if(bind(_idSocket, (struct sockaddr*)&serverEcho, sizeof(serverEcho)) == SOCKET_ERROR) {
		std::cout << "Could not bind adress." << std::endl;
		return false;
	}
	
	// Listen
	if(_type == TCP) {
		if(listen(_idSocket, _maxPending) == SOCKET_ERROR) {
			std::cout << "Could not listen adress." << std::endl;
			return false;
		}
	}
	
	return true;
}

int Server::waitClient() {
	int clientId = -1;
	
	if(_idSocket <= 0 || _type == NONE){
		std::cout << "Server not initialized" << std::endl;
	}
	else if(_type == TCP) {
		struct sockaddr_in clientEcho;
		SOCKET_LENGTH len = sizeof(clientEcho);
			
		if((clientId = accept(_idSocket, (struct sockaddr*)&clientEcho, &len)) == SOCKET_ERROR) {
			std::cout << "Failed to accept the client" << std::endl;
		}
	}
	else if(_type == UDP) {
		// What to do?
		// Nothing to wait ?
		// Read a message connect ? 
	}
	
	return clientId;
}

void Server::closeSocket(int& idSocket) {
	if(idSocket > 0) {
		shutdown(idSocket, CLOSE_ER); // No emission or reception
		close(idSocket);
	}
	idSocket = 0;
}