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

#ifdef NPOCO
#include <npoco/Condition.h>
#include <npoco/Thread.h>
#else
#include <Poco/Condition.h>
#include <Poco/Thread.h>
#endif


Poco::Condition gCon;
Poco::Mutex gMutex;


void signal_handler(int signal) {
	gCon.signal();
}


// --- LOG FUNCTION ---
void logFunction(int level, std::string logStr) {
	std::cout << level << " - " << logStr << std::endl;
}


// --- HELLO ---
// Callback for helloFunction.
NymphMessage* hello(int session, NymphMessage* msg, void* data) {
	std::cout << "Received message for session: " << session << ", msg ID: " << msg->getMessageId() << "\n";
	
	NymphType* nt = msg->parameters()[0];
	std::string* echoStr = new std::string(nt->getChar(), nt->string_length());
	std::cout << "Message string: " << *echoStr << "\n";
	
	NymphMessage* returnMsg = msg->getReplyMessage();
	NymphType* world = new NymphType(echoStr, true);
	returnMsg->setResultValue(world);
	msg->discard();
	return returnMsg;
}


// --- HELLO CALLBACK ---
// Callback for helloCallback. Called to register a client-side callback which 
// we will call right afterwards.
NymphMessage* helloCallback(int session, NymphMessage* msg, void* data) {
	std::cout << "Received message for session: " << session << ", msg ID: " << msg->getMessageId() << "\n";
	
	NymphType* nt = msg->parameters()[0];
	std::string* echoStr = new std::string(nt->getChar(), nt->string_length());
	std::cout << "Client callback method name: " << *echoStr << "\n";
	
	// Register and call the callback method ('callbackFunction') on the client.
	std::vector<NymphTypes> parameters;
	parameters.push_back(NYMPH_STRING);
	NymphMethod remoteMethod(*echoStr, parameters, NYMPH_NULL);
	remoteMethod.enableCallback();
	NymphRemoteClient::registerCallback(*echoStr, remoteMethod);
	
	std::vector<NymphType*> values;
	values.push_back(new NymphType(echoStr, true));
	std::string result;
	NymphMessage* returnMsg = msg->getReplyMessage();
	NymphType* world = new NymphType(true);
	if (!NymphRemoteClient::callCallback(session, *echoStr, values, result)) {
		std::cerr << "Calling callback failed: " << result << std::endl;
		world->setValue(false);
	}
	
	returnMsg->setResultValue(world);
	msg->discard();
	return returnMsg;
}


// --- ARRAY CALLBACK ---
// Returns an array filled with numbers.
NymphMessage* arrayCallback(int session, NymphMessage* msg, void* data) {
	std::vector<NymphType*>* numbers = new std::vector<NymphType*>;
	
	for (uint8_t i = 0; i < 10; i++) {
		NymphType* num = new NymphType(i);
		numbers->push_back(num);
	}
	
	NymphMessage* returnMsg = msg->getReplyMessage();
	returnMsg->setResultValue(new NymphType(numbers, true));
	msg->discard();
	return returnMsg;
}


// --- STRUCT CALLBACK ---
// Returns a struct filled with data.
NymphMessage* structCallback(int session, NymphMessage* msg, void* data) {
	std::map<std::string, NymphPair>* pairs = new std::map<std::string, NymphPair>;
	
	NymphPair pair;
	std::string* key = new std::string("Boolean");
	pair.key = new NymphType(key, true);
	pair.value = new NymphType(true);
	pairs->insert(std::pair<std::string, NymphPair>(*key, pair));
	
	key = new std::string("Uint8");
	pair.key = new NymphType(key, true);
	uint8_t uint8 = 255;
	pair.value = new NymphType(uint8);
	pairs->insert(std::pair<std::string, NymphPair>(*key, pair));
	
	key = new std::string("Int8");
	pair.key = new NymphType(key, true);
	int8_t int8 = -127;
	pair.value = new NymphType(int8);
	pairs->insert(std::pair<std::string, NymphPair>(*key, pair));
	
	key = new std::string("Uint16");
	pair.key = new NymphType(key, true);
	uint16_t uint16 = 65535;
	pair.value = new NymphType(uint16);
	pairs->insert(std::pair<std::string, NymphPair>(*key, pair));
	
	key = new std::string("Int16");
	pair.key = new NymphType(key, true);
	int16_t int16 = -32767;
	pair.value = new NymphType(int16);
	pairs->insert(std::pair<std::string, NymphPair>(*key, pair));
	
	key = new std::string("Uint32");
	pair.key = new NymphType(key, true);
	uint32_t uint32 = 4294967295;
	pair.value = new NymphType(uint32);
	pairs->insert(std::pair<std::string, NymphPair>(*key, pair));
	
	key = new std::string("Int32");
	pair.key = new NymphType(key, true);
	int32_t int32 = -2147483648;
	pair.value = new NymphType(int32);
	pairs->insert(std::pair<std::string, NymphPair>(*key, pair));
	
	key = new std::string("Uint64");
	pair.key = new NymphType(key, true);
	uint64_t uint64 = 18446744073709551615ull;
	pair.value = new NymphType(uint64);
	pairs->insert(std::pair<std::string, NymphPair>(*key, pair));
	
	key = new std::string("Int64");
	pair.key = new NymphType(key, true);
	int64_t int64 = -9223372036854775807;
	pair.value = new NymphType(int64);
	pairs->insert(std::pair<std::string, NymphPair>(*key, pair));
	
	key = new std::string("Float");
	pair.key = new NymphType(key, true);
	float fp32 = 32767.1234;
	pair.value = new NymphType(fp32);
	pairs->insert(std::pair<std::string, NymphPair>(*key, pair));
	
	key = new std::string("Double");
	pair.key = new NymphType(key, true);
	double fp64 = 3276732767.12341234;
	pair.value = new NymphType(fp64);
	pairs->insert(std::pair<std::string, NymphPair>(*key, pair));
	
	NymphMessage* returnMsg = msg->getReplyMessage();
	returnMsg->setResultValue(new NymphType(pairs, true));
	msg->discard();
	return returnMsg;
}


int main() {
	// Initialise the server instance.
	std::cout << "Initialising server..." << std::endl;
	long timeout = 5000; // 5 seconds.
	NymphRemoteClient::init(logFunction, NYMPH_LOG_LEVEL_TRACE, timeout);
	
	// Register methods to expose to the clients.
	std::cout << "Registering methods...\n";
	std::vector<NymphTypes> parameters;
	parameters.push_back(NYMPH_STRING);
	NymphMethod helloFunction("helloFunction", parameters, NYMPH_STRING, hello);
	NymphRemoteClient::registerMethod("helloFunction", helloFunction);
	
	NymphMethod helloCallbackFunction("helloCallbackFunction", parameters, NYMPH_BOOL, helloCallback);
	NymphRemoteClient::registerMethod("helloCallbackFunction", helloCallbackFunction);
	
	parameters.clear();
	
	NymphMethod arrayFunction("arrayFunction", parameters, NYMPH_ARRAY, arrayCallback);
	NymphRemoteClient::registerMethod("arrayFunction", arrayFunction);
	
	NymphMethod structFunction("structFunction", parameters, NYMPH_STRUCT, structCallback);
	NymphRemoteClient::registerMethod("structFunction", structFunction);
	
	
	// Install signal handler to terminate the server.
	signal(SIGINT, signal_handler);
	
	// Start server on port 4004.
	std::cout << "Starting server..." << std::endl;
	NymphRemoteClient::start(4004);
	
	// Loop until the SIGINT signal has been received.
	std::cout << "Waiting for clients..." << std::endl;
	gMutex.lock();
	gCon.wait(gMutex);
	
	// Clean-up
	NymphRemoteClient::shutdown();
	
	// Wait before exiting, giving threads time to exit.
	Poco::Thread::sleep(2000); // 2 seconds.
	
	return 0;
}
