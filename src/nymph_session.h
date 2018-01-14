/*
	nymph_session.h	- header file for the NymphRPC Session class.
	
	Revision 0
	
	Notes:
			- This class declares the session class to be used by Nymph servers.
			
	History:
	2017/06/24, Maya Posch : Initial version.
	
	(c) Nyanko.ws
*/


#pragma once
#ifndef NYMPH_SESSION_H
#define NYMPH_SESSION_H

#include <string>

using namespace std;

#include <Poco/Net/TCPServerConnection.h>
#include <Poco/Mutex.h>

using namespace Poco;


class NymphSession : public Net::TCPServerConnection {
	string loggerName;
	int handle;
	static int lastSessionHandle;
	static Mutex handleMutex;
	
public:
	NymphSession(const Net::StreamSocket& socket);
	void run();
	bool send(string& msg, string &result);
};

#endif
