#include "ManagerConnection.hpp"

ManagerConnection::ManagerConnection() : _initialized(false) {
}
ManagerConnection::~ManagerConnection() {
#ifdef _WIN32
	WSACleanup();
#endif
}

// Methods
bool ManagerConnection::initialize(const CONNECTION_TYPE type) {
	// Avoid startup again
	if(_initialized)
		return _initialized;
	
#ifdef __linux__ 
	_initialized = true;
#elif _WIN32
	// Initialize "Winsock dll" version 2.2
	WSADATA wsaData;
	WORD wVersionRequested = MAKEWORD(2, 2);

	int error = -1;
	if ((error = WSAStartup(wVersionRequested, &wsaData)) != 0)
		std::cout << "WSAStartup failed with error: " << error << "." << std::endl;
	else if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
		std::cout << "Could not find a usable version of Winsock.dll." << std::endl;
	else 
		_initialized = true;
#else
	std::cout << "Not supported OS. - Only windows and ubuntu." << std::endl;
#endif

	return _initialized;
}


std::shared_ptr<Server> ManagerConnection::createServer(const int port, const int pending) {
	// Context ok ?
	if(!_initialized) {
		std::cout << "Manager not initialized." << std::endl;
		return nullptr;
	}	
	
	// Initialize the server
	auto serv = std::make_shared<Server>(port, pending);
	if(!serv->initialize())
		serv.reset();
	
	return serv;
}
std::shared_ptr<Socket> ManagerConnection::connectTo(const std::string& ipAdress, const int port) {
	// Context ok ?
	if(!_initialized) {
		std::cout << "Manager not initialized." << std::endl;
		return nullptr;
	}
	
	// Initialize the socket
	auto sock = std::make_shared<Socket>(ipAdress, port);
	if(!sock->initialize())
		sock.reset();
	
	return sock;
}

// Getters
bool ManagerConnection::isInitialized() const {
	return _initialized;
}
