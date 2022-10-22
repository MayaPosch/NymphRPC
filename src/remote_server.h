/*
	remote_server.h	- header file for the Remote Server class.
	
	Revision 0
	
	Notes:
			- This class declares the main class to be used by NymphRPC clients.
			
	2017/06/24, Maya Posch	: Initial version.	
	(c) Nyanko.ws
*/


#ifndef REMOTE_SERVER_H
#define REMOTE_SERVER_H


#include <vector>
#include <string>
#include <map>

#ifdef HOST_FREERTOS
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#else
//#include <Poco/Semaphore.h>
#endif

#ifdef LWIP_SOCKET
#include <lwip/sockets.h>
#include <lwip/sys.h>
#elif defined NPOCO
#include <npoco/net/SocketAddress.h>
#include <npoco/net/StreamSocket.h>
#include <npoco/Semaphore.h>
#else
#include <Poco/Net/SocketAddress.h>
#include <Poco/Net/StreamSocket.h>
#include <Poco/Semaphore.h>
#endif

#include "nymph_method.h"
#include "nymph_listener.h"
#include "nymph_logger.h"


class NymphServerInstance {
	std::string loggerName = "NymphServerInstance";
	uint32_t handle;
/* #ifdef LWIP_SOCKET
	int socket;
#else */
	Poco::Net::StreamSocket* socket = 0;
//#endif
	Poco::Semaphore* socketSemaphore = 0;
	uint32_t nextMethodId = 0;
	std::map<std::string, NymphMethod> methods;
	std::map<uint32_t, NymphMethod*> methodIds;
#ifdef HOST_FREERTOS
	//
#else
	Poco::Mutex methodsMutex;
#endif
	uint32_t timeout;
	
public:
#ifdef HOST_FREERTOS
	//
#else
	NymphServerInstance(uint32_t handle, Poco::Net::StreamSocket* socket, uint32_t timeout = 3000);
#endif
	~NymphServerInstance();
	
	void setHandle(uint32_t handle);
	uint32_t getHandle();
#ifdef HOST_FREERTOS
	//
#else
	Poco::Semaphore* semaphore();
#endif
	bool sync(std::string &result);
	bool addMethod(std::string name, NymphMethod method);
	bool removeMethod(std::string name);
	bool disconnect(std::string& result);
	bool callMethod(std::string name, std::vector<NymphType*> &values, 
										NymphType* &returnvalue, std::string &result);
	bool callMethodId(uint32_t id, std::vector<NymphType*> &values, NymphType* &returnvalue, std::string &result);
};


class NymphRemoteServer {
	static std::map<uint32_t, NymphServerInstance*> instances;
#ifdef HOST_FREERTOS
	//
#else
	static Poco::Mutex instancesMutex;
#endif
	static uint32_t lastHandle;
	static long timeout;
	static std::string loggerName;
	static uint32_t nextMethodId;
	
public:
	static bool init(logFnc logger, int level = NYMPH_LOG_LEVEL_TRACE, long timeout = 3000);
	static void setLogger(logFnc logger, int level);
	static bool shutdown();
	static bool connect(std::string host, int port, uint32_t &handle, void* data, std::string &result);
	static bool connect(std::string url, uint32_t &handle, void* data, std::string &result);
//#ifndef LWIP_SOCKET
	static bool connect(Poco::Net::SocketAddress sa, uint32_t &handle, void* data, std::string &result);
//#endif
	static bool disconnect(uint32_t handle, std::string &result);
	static bool callMethod(uint32_t handle, std::string name, std::vector<NymphType*> &values, 
										NymphType* &returnvalue, std::string &result);
	static bool callMethodId(uint32_t handle, uint32_t id, std::vector<NymphType*> &values, NymphType* &returnvalue, std::string &result);
	static bool removeMethod(uint32_t handle, std::string name);
	
	static bool registerCallback(std::string name, NymphCallbackMethod method, void* data);
	static bool removeCallback(std::string name);
};

#endif
