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
		// TODO: Lack of (full) IPv6-support makes bind() necessary. Check this is sufficient.
		//ss.bind6(port, true, false); // Port, SO_REUSEADDR, IPv6-only.
#ifndef NPOCO
		try {
#endif
			ss.bind(port, true, true); // Port, SO_REUSEADDR, SO_REUSEPORT.
#ifndef NPOCO
		}
		catch (...) {
			// Exception while calling bind(). Give it a retry for 2 minutes before giving up.
			NYMPH_LOG_ERROR("Exception in bind. Retrying for 120 seconds...");
			uint32_t countdown = 120; // seconds.
			bool success = true;
			while (1) {
				// Wait 5 seconds.
				Thread::sleep(5000); // milliseconds.
				success = true;
				try {
					ss.bind(port, true, true);
				}
				catch (...) {
					NYMPH_LOG_ERROR("Exception in bind.");
					success = false;
				}
				
				if (success) {
					NYMPH_LOG_INFORMATION("Connected to port after retrying.");
					break;
				}
				
				countdown -= 5;
				if (countdown < 5) {
					NYMPH_LOG_ERROR("Error starting TCP server, abort.");
					return false;
				}
			}
		}
#endif
		
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
	ss.close();
	running = false;
	delete server;
	
	NYMPH_LOG_INFORMATION("Stopped NymphServer.");
	
	return true;
}
