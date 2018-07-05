#ifndef SERVER_H
#define SERVER_H

#include "Socket.hpp"

class Server : public Socket {
public:
	// Constructors
	Server(const int port = 80, const size_t maxPending = 10);
	~Server();
	
	// Methods
	bool initialize() override;
	int waitClient();
	void closeSocket(int& idSocket);
	
private:
	// Members
	size_t _maxPending;
};

#endif