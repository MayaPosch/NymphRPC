/*
	nymph_test_server.cpp - Test server application using the NymphRPC library.
	
	Revision 0
	
	Features:
				- 
				
	Notes:
				-
				
	2017/06/24, Maya Posch	: Initial version.
	(c) Nyanko.ws
*/


#include "../../src/nymph.h"


#include <iostream>
#include <vector>
#include <csignal>

using namespace std;


#include <Poco/Condition.h>
#include <Poco/Thread.h>

using namespace Poco;


#include "target_code.h"

constexpr std::size_t min_size = 1 << 10;
constexpr std::size_t max_size = 16 << 10 << 10;
constexpr std::size_t multiplier = 2;


Condition gCon;
Mutex gMutex;
//volatile sig_atomic_t gSignalTrigger = 0;


void signal_handler(int signal) {
	//gSignalTrigger = 1;
	gCon.signal();
}


// --- LOG FUNCTION ---
void logFunction(int level, string logStr) {
	cout << level << " - " << logStr << endl;
}


// Callback for get_answer() method.
NymphMessage* get_answer_cb(int session, NymphMessage* msg, void* data) {
	//cout << "Received message for session: " << session << ", msg ID: " << msg->getMessageId() << "\n";
	
	NymphMessage* returnMsg = msg->getReplyMessage();
	NymphUint32* response = new NymphUint32(42);
	returnMsg->setResultValue(response);
	return returnMsg;
}


// Callback for get_blob() method.
NymphMessage* get_blob_cb(int session, NymphMessage* msg, void* data) {
	//cout << "Received message for session: " << session << ", msg ID: " << msg->getMessageId() << "\n";
	
	uint32_t num = ((NymphUint32*) msg->parameters()[0])->getValue();
	//cout << "Message string: " << echoStr << "\n";
	
	NymphMessage* returnMsg = msg->getReplyMessage();
	NymphString* world = new NymphString(get_blob(num));
	returnMsg->setResultValue(world);
	return returnMsg;
}


// Callback for get_struct() method.
NymphMessage* get_struct_cb(int session, NymphMessage* msg, void* data) {
	//cout << "Received message for session: " << session << ", msg ID: " << msg->getMessageId() << "\n";
		
	NymphMessage* returnMsg = msg->getReplyMessage();
	NymphStruct* str = new NymphStruct();
	// TODO: add struct values.
	returnMsg->setResultValue(str);
	return returnMsg;
}


int main() {	
	// Init blob cache.
	for (std::size_t s = min_size; s <= max_size; s *= multiplier) {
		get_blob(s);
	}
	
	// Initialise the server instance.
	cout << "Initialising server...\n";
	long timeout = 5000; // 5 seconds.
	NymphRemoteClient::init(logFunction, NYMPH_LOG_LEVEL_WARNING, timeout);
	
	// Register methods to expose to the clients.
	cout << "Registering methods...\n";
	vector<NymphTypes> parameters;
	NymphMethod getAnswerFunction("get_answer", parameters, NYMPH_UINT32);
	getAnswerFunction.setCallback(get_answer_cb);
	NymphRemoteClient::registerMethod("get_answer", getAnswerFunction);
	
	NymphMethod getStructFunction("get_struct", parameters, NYMPH_STRUCT);
	getStructFunction.setCallback(get_struct_cb);
	NymphRemoteClient::registerMethod("get_struct", getStructFunction);
	
	parameters.push_back(NYMPH_UINT32);
	NymphMethod getBlobFunction("get_blob", parameters, NYMPH_STRING);
	getBlobFunction.setCallback(get_blob_cb);
	NymphRemoteClient::registerMethod("get_blob", getBlobFunction);
	
	// Install signal handler to terminate the server.
	signal(SIGINT, signal_handler);
	
	// Start server on port 4004.
	NymphRemoteClient::start(4004);
	
	// Loop until the SIGINT signal has been received.
	//while (gSignalTrigger == 0) { }
	gMutex.lock();
	gCon.wait(gMutex);
	
	// Clean-up
	NymphRemoteClient::shutdown();
	
	// Wait before exiting, giving threads time to exit.
	Thread::sleep(2000); // 2 seconds.
	
	return 0;
}
