/*
	nymph_socket_listener.h	- Declares the NymphRPC Socket Listener class.
	
	Revision 0
	
	Notes:
			- 
			
	History:
	2017/06/24, Maya Posch : Initial version.
	
	(c) Nyanko.ws
*/


#pragma once
#ifndef NYMPH_SOCKET_LISTENER_H
#define NYMPH_SOCKET_LISTENER_H

#include "nymph_message.h"

#include <Poco/Runnable.h>
#include <Poco/Net/StreamSocket.h>
#include <Poco/Semaphore.h>
#include <Poco/Condition.h>

using namespace Poco;

#include <map>
#include <string>

using namespace std;

// TYPES

struct NymphSocket {
	Net::StreamSocket* socket;	// Pointer to the socket instance.
	Semaphore* semaphore;		// Signals when it's safe to delete the socket.
	void* data;					// User data.
	int handle;					// The Nymph internal socket handle.
};


struct NymphRequest {
	int handle;
	UInt64 messageId;
	Mutex mutex;
	Condition condition;
	NymphType* response;
	bool exception;
	NymphException exceptionData;
};

// ---


class NymphSocketListener : public Poco::Runnable {
	string loggerName;
	bool listen;
	NymphSocket nymphSocket;
	Net::StreamSocket* socket;
	map<UInt64, NymphRequest*> messages;
	Mutex messagesMutex;
	bool init;
	Condition* readyCond;
	Mutex* readyMutex;
	
public:
	NymphSocketListener(NymphSocket socket, Condition* cond, Mutex* mtx);
	~NymphSocketListener();
	void run();
	void stop();
	bool addMessage(NymphRequest* &request);
	bool removeMessage(UInt64 messageId);
};

#endif
