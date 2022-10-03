/*
	nymph_server.cpp	- implementation for the NymphRPC Server class.
	
	Revision 0
	
	Notes:
			- This class implements the server class to be used by Nymph servers.
			
	History:
	2017/06/24, Maya Posch : Initial version.
	
	(c) Nyanko.ws
*/


#include "nymph_server.h"
#include "nymph_logger.h"
#include "nymph_session.h"

#ifndef NPOCO
#include <Poco/Net/NetException.h>
#endif

using namespace Poco;

using namespace std;


// Static initialisations.
string NymphServer::loggerName = "NymphServer";
Poco::Net::ServerSocket NymphServer::ss;
Net::TCPServer* NymphServer::server;
std::atomic<bool> NymphServer::running;


// --- START ---
bool NymphServer::start(int port) {
#ifndef NPOCO
	try {
#endif
		// Create a server socket that listens on all interfaces, IPv4 and IPv6.
		// Assign it to the new TCPServer.
		ss.bind6(port, true, false); // Port, SO_REUSEADDR, IPv6-only.
		ss.listen();
		server = new Net::TCPServer(new Net::TCPServerConnectionFactoryImpl<NymphSession>(),
									ss);
		server->start();
#ifndef NPOCO
	}
	catch (Net::NetException& e) {
		NYMPH_LOG_ERROR("Error starting TCP server: " + e.message());
		return false;
	}
#endif
	
	running = true;
	return true;
}


// --- STOP ---
bool NymphServer::stop() {
	server->stop();
	running = false;
	delete server;
	
	NYMPH_LOG_INFORMATION("Stopped NymphServer.");
	
	return true;
}
