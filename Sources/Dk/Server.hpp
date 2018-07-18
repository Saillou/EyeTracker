#ifndef SERVER_H
#define SERVER_H

#include "Socket.hpp"

class Server : public Socket {
public:
	// Constructors
	Server(const int port = 80, const size_t maxPending = 10);
	~Server();
	
	// Methods
	bool initialize(const CONNECTION_TYPE type, const CONNECTION_MODE mode) override;
	int waitClient(long ms = -1);
	void closeSocket(int& idSocket);
	void closeAll();
	
private:
	// Members
	size_t _maxPending;
	std::vector<int> _idScketsConnected;
};

#endif