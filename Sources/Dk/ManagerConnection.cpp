#include "ManagerConnection.hpp"

ManagerConnection::ManagerConnection() : _initialized(false) {
}
ManagerConnection::~ManagerConnection() {
#ifdef _WIN32
	WSACleanup();
#endif
}

// Methods
bool ManagerConnection::initialize() {
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


std::shared_ptr<Server> ManagerConnection::createServer(const Socket::CONNECTION_TYPE type, const Socket::CONNECTION_MODE mode, const int port, const int pending) {
	// Context ok ?
	if(!_initialized) {
		std::cout << "Manager not initialized." << std::endl;
		return nullptr;
	}	
	
	// Initialize the server
	auto serv = std::make_shared<Server>(port, pending);
	if(!serv->initialize(type, mode))
		serv.reset();
	
	return serv;
}
std::shared_ptr<Socket> ManagerConnection::connectTo(const Socket::CONNECTION_TYPE type, const Socket::CONNECTION_MODE mode, const std::string& ipAdress, const int port) {
	// Context ok ?
	if(!_initialized) {
		std::cout << "Manager not initialized." << std::endl;
		return nullptr;
	}
	
	// Initialize the socket
	auto sock = std::make_shared<Socket>(ipAdress, port);
	if(!sock->initialize(type, mode))
		sock.reset();
	
	return sock;
}

// Getters
bool ManagerConnection::isInitialized() const {
	return _initialized;
}

std::vector<ManagerConnection::IpAdress> ManagerConnection::snif(const IpAdress& ipBeg, const IpAdress& ipEnd, int stopAfterNb) {	
	// Determine delta port
	int pBeg = ipBeg.getPort();
	int pEnd = ipBeg.getPort();
	
	if(pBeg > pEnd) {
		pBeg += pEnd;
		pEnd = pBeg - pEnd;
		pBeg = pBeg - pEnd;
	}
	
	// Determine delta target
	size_t sBeg = ipBeg.toNumber();
	size_t sEnd = ipEnd.toNumber();
	
	if(sBeg > sEnd) {
		size_t tmp = sBeg;
		sBeg = sEnd;
		sEnd = tmp;
	}
	
	// Iterate through all ip
	std::vector<IpAdress> ipAvailable;
	
	for(int port = pBeg; port <= pEnd; port++) {
		for(size_t target = sBeg; target <= sEnd; target++) {
			IpAdress ipTested(target, port);
			if(!ipTested.isValide()) 
				continue;

			if(connectTo(Socket::TCP, Socket::NOT_BLOCKING, ipTested.toString(), port) != nullptr) {
				ipAvailable.push_back(ipTested);

				if(stopAfterNb > 0 && (int)ipAvailable.size() >= stopAfterNb)
					break;
			}
			
		}
	}
	
	return ipAvailable;
}

// --------- Nested class --------- //
// Constructors
ManagerConnection::IpAdress::IpAdress(const std::string& ipAndPort)  :
	_target({0, 0, 0, 0}),
	_port(0),
	_valide(false)
{
	size_t posPort = ipAndPort.find(':');	
	if(posPort != std::string::npos && posPort > 0) {
		std::string strTarget = ipAndPort.substr(0, posPort);
		std::string strPort = ipAndPort.substr(posPort+1);

		_valide = _targetFromString(strTarget) && _portFromString(strPort);
	}
}
ManagerConnection::IpAdress::IpAdress(const std::string& strTarget, int port)  :
	_target({0,0,0,0}),
	_port(port),
	_valide(false)
{
	_valide = (bool)(_targetFromString(strTarget) && (_port > 0 && _port < 1e4));
}
ManagerConnection::IpAdress::IpAdress(char c1, char c2, char c3, char c4, int port) :
	_target({c1, c2, c3, c4}),
	_port(port),
	_valide(false)
{
	_valide = (bool)(_port > 0 && _port < 1e4);
}
ManagerConnection::IpAdress::IpAdress(size_t target, int port) :
	_target(Protocole::BinMessage::Write_256(target, 4)),
	_port(port),
	_valide(false)
{
	_valide = (bool)(_port > 0 && _port < 1e4);
}

// Methods
std::string ManagerConnection::IpAdress::toString() const {
	std::stringstream ss;
	for(int i = 0; i < 4; i++)
		ss << (int)(unsigned char)_target[i] << (i < 3 ? "." : "");
	
	return ss.str();
}
std::string ManagerConnection::IpAdress::toFullString() const {
	std::stringstream ss;
	ss << toString() << ":" << _port;
	return ss.str();
}
size_t ManagerConnection::IpAdress::toNumber() const {
	return Protocole::BinMessage::Read_256(_target);
}

bool ManagerConnection::IpAdress::_targetFromString(const std::string& path) {
    std::istringstream flow(path);
    std::string s;    
	int i = 0;
    while (getline(flow, s, '.')) {
        int nb = (int)Protocole::Message::To_unsignedInt(s);
		
		// Check
		if(i > 4 || nb > 255 || nb < 0)
			return false;		
		else 
			_target[i] = (char)nb;
		i++;
    }
	
	return true;
}
bool ManagerConnection::IpAdress::_portFromString(const std::string& port) {
	_port = (int)Protocole::Message::To_unsignedInt(port);
	return _port > 0 && _port < 1e4;
}

// Getters
int ManagerConnection::IpAdress::getPort() const {
	return _port;
}
const std::vector<char>& ManagerConnection::IpAdress::getTarget() const {
	return _target;
}
bool ManagerConnection::IpAdress::isValide() const {
	return _valide;
}
		
