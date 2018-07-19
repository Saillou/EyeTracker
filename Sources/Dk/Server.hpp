#ifndef SERVER_H
#define SERVER_H

#include "Socket.hpp"

#include <memory>
#include <thread>
#include <mutex>

class Server : public Socket {
public:
	// Constructors
	Server(const unsigned short port = 80, const int maxPending = 10);
	~Server();
	
	// Methods
	bool initialize(const CONNECTION_TYPE type, const CONNECTION_MODE mode) override;
	int waitClient(long ms = -1);
	void closeSocket(int& idSocket);
	void closeAll();
	
protected:
	// Methods
	
	// Members
	int _maxPending;
	std::vector<int> _idScketsConnected;
	
// -------- Nested helper --------
public:
	class ThreadWrite {
	public:
		// Constructors
		ThreadWrite(std::shared_ptr<Server> server, const Protocole::BinMessage& msg, const int idClient);
		~ThreadWrite();
		
		// Methods
		void write(std::shared_ptr<Server> server, const Protocole::BinMessage& msg, const int idClient);
		bool isActive();

	private: 
		// Members
		volatile bool _active; // should be better to be an atomic, but good luck with that.. (No copy, no move .. how to use with vector ?) So, we use with a mutex.
		std::thread* _ptrThread;
		std::mutex _mutexActive;
	};
};

#endif