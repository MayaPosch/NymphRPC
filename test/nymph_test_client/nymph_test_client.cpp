/*
	nymph_test_client.cpp - Test client application using the NymphRPC library.
	
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


void logFunction(int level, std::string logStr) {
	std::cout << level << " - " << logStr << std::endl;
}


// Callback to register with the server. 
// This callback will be called once by the server and then discarded. This is
// useful for one-off events, but can also be used for callbacks during the 
// life-time of the client.
void callbackFunction(uint32_t session, NymphMessage* msg, void* data) {
	std::cout << "Client callback function called.\n";
	
	// Remove the callback.
	NymphRemoteServer::removeCallback("helloCallbackFunction");
	
	msg->discard();
}


int main() {
	// Initialise the remote client instance.
	long timeout = 5000; // 5 seconds.
	NymphRemoteServer::init(logFunction, NYMPH_LOG_LEVEL_TRACE, timeout);
	
	// Connect to the remote server.
	uint32_t handle;
	std::string result;
	if (!NymphRemoteServer::connect("localhost", 4004, handle, 0, result)) {
		std::cout << "Connecting to remote server failed: " << result << std::endl;
		NymphRemoteServer::disconnect(handle, result);
		NymphRemoteServer::shutdown();
		return 1;
	}
	
	// Send message and wait for response.
	std::vector<NymphType*> values;
	std::string hello = "Hello World!";
	values.push_back(new NymphType(&hello));
	NymphType* returnValue = 0;
	if (!NymphRemoteServer::callMethod(handle, "helloFunction", values, returnValue, result)) {
		std::cout << "Error calling remote method: " << result << std::endl;
		NymphRemoteServer::disconnect(handle, result);
		NymphRemoteServer::shutdown();
		return 1;
	}
	
	std::string response(returnValue->getChar(), returnValue->string_length());
	
	std::cout << "Response string: " << response << std::endl;
	
	delete returnValue;
	
	// Register callback and send message with its ID to the server. Then wait
	// for the callback to be called.
	NymphRemoteServer::registerCallback("callbackFunction", callbackFunction, 0);
	values.clear();
	std::string cbStr = "callbackFunction";
	values.push_back(new NymphType(&cbStr));
	returnValue = 0;
	if (!NymphRemoteServer::callMethod(handle, "helloCallbackFunction", values, returnValue, result)) {
		std::cout << "Error calling remote method: " << result << std::endl;
		NymphRemoteServer::disconnect(handle, result);
		NymphRemoteServer::shutdown();
		return 1;
	}
	
	if (!returnValue->getBool()) {
		std::cout << "Remote method returned false. " << result << std::endl;
		NymphRemoteServer::disconnect(handle, result);
		NymphRemoteServer::shutdown();
		return 1;
	}
	
	delete returnValue;
	
	// Request array with integers.
	returnValue = 0;
	values.clear();
	if (!NymphRemoteServer::callMethod(handle, "arrayFunction", values, returnValue, result)) {
		std::cout << "Error calling remote method: " << result << std::endl;
		NymphRemoteServer::disconnect(handle, result);
		NymphRemoteServer::shutdown();
		return 1;
	}
	
	// Print out values in vector.
	std::vector<NymphType*>* numbers = returnValue->getArray();
	std::cout << "Got numbers: ";
	for (int i = 0; i < numbers->size(); i++) {
		std::cout << i << ":" << (uint16_t) (*numbers)[i]->getUint8() << " ";
	}
	
	std::cout << "." << std::endl;
	
	delete returnValue;
	
	// Request struct with all basic types (integers, floats).
	returnValue = 0;
	if (!NymphRemoteServer::callMethod(handle, "structFunction", values, returnValue, result)) {
		std::cout << "Error calling remote method: " << result << std::endl;
		NymphRemoteServer::disconnect(handle, result);
		NymphRemoteServer::shutdown();
		return 1;
	}
	
	// Print out the values.
	std::map<std::string, NymphPair>* pairs = returnValue->getStruct();
	std::cout << "Pairs: " << std::endl;
	NymphPair ref = (*pairs)["Boolean"];
	std::cout << "Boolean: " << ref.value->getBool() << std::endl;
	ref = (*pairs)["Uint8"];
	std::cout << "Uint8: " << (uint16_t) ref.value->getUint8() << std::endl;
	ref = (*pairs)["Int8"];
	std::cout << "Int8: " << (int16_t) ref.value->getInt8() << std::endl;
	ref = (*pairs)["Uint16"];
	std::cout << "Uint16: " << ref.value->getUint16() << std::endl;
	ref = (*pairs)["Int16"];
	std::cout << "Int16: " << ref.value->getInt16() << std::endl;
	ref = (*pairs)["Uint32"];
	std::cout << "Uint32: " << ref.value->getUint32() << std::endl;
	ref = (*pairs)["Int32"];
	std::cout << "Int32: " << ref.value->getInt32() << std::endl;
	ref = (*pairs)["Uint64"];
	std::cout << "Uint64: " << ref.value->getUint64() << std::endl;
	ref = (*pairs)["Int64"];
	std::cout << "Int64: " << ref.value->getInt64() << std::endl;
	ref = (*pairs)["Float"];
	std::cout << "Float: " << ref.value->getFloat() << std::endl;
	ref = (*pairs)["Double"];
	std::cout << "Double: " << ref.value->getDouble() << std::endl;
	
	delete returnValue;
	
	std::cout << "Test completed." << std::endl;
	
	std::cout << "Shutting down client...\n";
	
	// Shutdown.
	NymphRemoteServer::disconnect(handle, result);
	NymphRemoteServer::shutdown();
	return 0;
}
