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

#include <Poco/Semaphore.h>
#include <Poco/Net/SocketAddress.h>
#include <Poco/Net/StreamSocket.h>

#include "nymph_method.h"
#include "nymph_listener.h"
#include "nymph_logger.h"


class NymphServerInstance {
	std::string loggerName = "NymphServerInstance";
	Poco::Net::StreamSocket* socket = 0;
	Poco::Semaphore* socketSemaphore = 0;
	uint32_t nextMethodId = 0;
	std::map<std::string, NymphMethod> methods;
	std::map<uint32_t, NymphMethod*> methodIds;
	Poco::Mutex methodsMutex;
	uint32_t handle;
	uint32_t timeout;
	
public:
	NymphServerInstance(uint32_t handle, Poco::Net::StreamSocket* socket, uint32_t timeout = 3000);
	~NymphServerInstance();
	
	void setHandle(uint32_t handle);
	uint32_t getHandle();
	Poco::Semaphore* semaphore();
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
	static Poco::Mutex instancesMutex;
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
	static bool connect(Poco::Net::SocketAddress sa, uint32_t &handle, void* data, std::string &result);
	static bool disconnect(uint32_t handle, std::string &result);
	static bool callMethod(uint32_t handle, std::string name, std::vector<NymphType*> &values, 
										NymphType* &returnvalue, std::string &result);
	static bool callMethodId(uint32_t handle, uint32_t id, std::vector<NymphType*> &values, NymphType* &returnvalue, std::string &result);
	static bool removeMethod(uint32_t handle, std::string name);
	
	static bool registerCallback(std::string name, NymphCallbackMethod method, void* data);
	static bool removeCallback(std::string name);
};

#endif
