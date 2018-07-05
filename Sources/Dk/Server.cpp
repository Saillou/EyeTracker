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
bool Server::initialize()	{
	// Define id
	_idSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	struct sockaddr_in  serverEcho;
	memset(&serverEcho, 0, sizeof(serverEcho));
	
	serverEcho.sin_addr.s_addr	= htonl(INADDR_ANY);
	serverEcho.sin_port				= htons(_port);	
	serverEcho.sin_family		 	= AF_INET;
	
	// Try bind
	if(bind(_idSocket, (struct sockaddr*)&serverEcho, sizeof(serverEcho)) == SOCKET_ERROR) {
		std::cout << "Could not bind adress." << std::endl;
		return false;
	}
	
	// Listen
	if(listen(_idSocket, _maxPending) == SOCKET_ERROR) {
		std::cout << "Could not listen adress." << std::endl;
		return false;
	}
	
	return true;
}

int Server::waitClient() {
	if(_idSocket <= 0){
		std::cout << "Server not initialized" << std::endl;
		return -1;
	}
	
	int clientId = -1;
	struct sockaddr_in clientEcho;
	SOCKET_LENGTH len = sizeof(clientEcho);
		
	if((clientId = accept(_idSocket, (struct sockaddr*)&clientEcho, &len)) == SOCKET_ERROR) {
		std::cout << "Failed to accept the client" << std::endl;
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