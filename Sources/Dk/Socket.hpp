#ifndef SOCKET_H
#define SOCKET_H

#ifdef __linux__ 
	#include <sys/socket.h>
	#include <sys/types.h>
	#include <netinet/in.h>	
	#include <arpa/inet.h>
	
	typedef unsigned int SOCKET_LENGTH;
#elif _WIN32
	#include <windows.h>
	
	typedef int SOCKET_LENGTH;
#endif

#ifndef USE_MSVC 
	#include <unistd.h>
#endif

#ifndef SOCKET_ERROR
	#define SOCKET_ERROR -1
#endif

#include <iostream>
#include "Protocole.hpp"

class Socket {
public:
	// Enum s
	enum CONNECTION_TYPE {
		NONE, UDP, TCP
	};
	enum CONNECTION_MODE {
		BLOCKING, NOT_BLOCKING
	};
	
	struct Accessiblity {
		bool readable;
		bool writable;
		int errorCode;
	};
	
	// Constructors
	Socket(const std::string& ipAdress = "localhost", const int port = 80);
	virtual ~Socket();
	
	// Methods
	virtual bool initialize(const CONNECTION_TYPE type, const CONNECTION_MODE mode);
	bool read(Protocole::BinMessage& msg, int idSocket = -1) const;
	bool write(Protocole::BinMessage& msg, int idSocket = -1) const;
	
	Accessiblity waitForAccess(unsigned long timeoutMs) const;
	
	// Setters
	
	// Getters
	const std::string& getIpAdress() const;
	const int& getPort() const;
	const int& getId() const;
	const CONNECTION_TYPE& getType() const;
	
	// Enums
	enum SHUTDOWN_SOCKET {
		CLOSE_EMISSION = 0,
		CLOSE_RECEPTION = 1,
		CLOSE_ER = 2,
	};
	
	// Statics
	static const size_t BUFFER_SIZE_MAX = 1024;	
	
protected:
	// Methods
	int _changeMode(const CONNECTION_MODE mode);
	
	// Members
	std::string _ipAdress;
	int 			_port;
	int 			_idSocket;
	CONNECTION_TYPE _type;
	CONNECTION_MODE _mode;
};

#endif