#ifndef MANAGER_CONNECTION_H
#define MANAGER_CONNECTION_H

#ifdef _WIN32
	#include <windows.h>
#endif

#include <iostream>
#include <vector>
#include <memory>

#include "Socket.hpp"
#include "Server.hpp"

class ManagerConnection {
public:	
	// Constructors
	ManagerConnection();
	~ManagerConnection();
	
	// Methods
	bool initialize();
	std::shared_ptr<Server> createServer(const Socket::CONNECTION_TYPE type, const int port = 80, const int pending = 10);
	std::shared_ptr<Socket> connectTo(const Socket::CONNECTION_TYPE type, const std::string& ipAdress = "localhost", const int port = 80);
	
	// Getters
	bool isInitialized() const;
	
private:
	// Members
	bool _initialized;
};

#endif