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

#ifndef SOCKET_ERROR
	#define SOCKET_ERROR -1
#endif

#include <iostream>
#include <unistd.h>
#include "Protocole.hpp"

class Socket {
public:
	// Enum s
	enum CONNECTION_TYPE {
		NONE, UDP, TCP
	};
	
	// Constructors
	Socket(const std::string& ipAdress = "localhost", const int port = 80);
	virtual ~Socket();
	
	// Methods
	virtual bool initialize(const CONNECTION_TYPE type);
	bool read(Protocole::BinMessage& msg, int idSocket = -1) const;
	bool write(Protocole::BinMessage& msg, int idSocket = -1) const;
	
	// Setters
	
	// Getters
	const std::string& getIpAdress() const;
	const int& getPort() const;
	const int& getId() const;
	
	// Enums
	enum SHUTDOWN_SOCKET {
		CLOSE_EMISSION = 0,
		CLOSE_RECEPTION = 1,
		CLOSE_ER = 2,
	};
	
	// Statics
	static const size_t BUFFER_SIZE_MAX = 1024;	
	
protected:
	// Members
	std::string _ipAdress;
	int 			_port;
	int 			_idSocket;
	CONNECTION_TYPE _type;
};

#endif