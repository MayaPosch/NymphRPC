//

#include "nymphclientclass.h"


// --- LOG FUNCTION ---
void logFunction(int level, string logStr) {
	cout << level << " - " << logStr << endl;
}


// --- CONSTRUCTOR ---
NymphClientClass::NymphClientClass() {
	// Initialise the remote client instance.
	long timeout = 5000; // 5 seconds.
	NymphRemoteServer::init(logFunction, NYMPH_LOG_LEVEL_TRACE, timeout);
	
	// Connect to the remote server.
	if (!NymphRemoteServer::connect("localhost", 4004, handle, 0, result)) {
		cout << "Connecting to remote server failed: " << result << endl;
		NymphRemoteServer::disconnect(handle, result);
		NymphRemoteServer::shutdown();
	}
}


// --- DECONSTRUCTOR ---
NymphClientClass::~NymphClientClass() {
	// Shutdown.
	NymphRemoteServer::disconnect(handle, result);
	NymphRemoteServer::shutdown();
}


// --- GET ANSWER ---
// * uint32 get_answer();
void NymphClientClass::get_answer() {
	// Send message and wait for response.
	vector<NymphType*> values;
	NymphType* returnValue = 0;
	if (!NymphRemoteServer::callMethod(handle, "get_answer", values, returnValue, result)) {
		cout << "Error calling remote method: " << result << endl;
		NymphRemoteServer::disconnect(handle, result);
		NymphRemoteServer::shutdown();
		return;
	}
	
	if (!returnValue) { return; }
	
	if (returnValue->type() != NYMPH_UINT32) {
		cout << "Return value wasn't a string. Type: " << returnValue->type() << endl;
		NymphRemoteServer::disconnect(handle, result);
		NymphRemoteServer::shutdown();
		return;
	}
	
	delete returnValue;
}

// --- GET STRUCT ---
// * struct get_struct();
void NymphClientClass::get_struct() {
	// Send message and wait for response.
	vector<NymphType*> values;
	NymphType* returnValue = 0;
	if (!NymphRemoteServer::callMethod(handle, "get_struct", values, returnValue, result)) {
		cout << "Error calling remote method: " << result << endl;
		NymphRemoteServer::disconnect(handle, result);
		NymphRemoteServer::shutdown();
		return;
	}
	
	if (!returnValue) { return; }
	
	if (returnValue->type() != NYMPH_ARRAY) {
		cout << "Return value wasn't a struct. Type: " << returnValue->type() << endl;
		NymphRemoteServer::disconnect(handle, result);
		NymphRemoteServer::shutdown();
		return;
	}
	
	delete returnValue;
}
	

// --- GET BLOB ---
// * string get_blob(uint32);
void NymphClientClass::get_blob(uint32_t i) {
	// Send message and wait for response.
	vector<NymphType*> values;
	NymphType* returnValue = 0;
	values.push_back(new NymphUint32(i));
	if (!NymphRemoteServer::callMethod(handle, "get_blob", values, returnValue, result)) {
		cout << "Error calling remote method: " << result << endl;
		NymphRemoteServer::disconnect(handle, result);
		NymphRemoteServer::shutdown();
		return;
	}
	
	if (!returnValue) { return; }
	
	if (returnValue->type() != NYMPH_STRING) {
		cout << "Return value wasn't a string. Type: " << returnValue->type() << endl;
		NymphRemoteServer::disconnect(handle, result);
		NymphRemoteServer::shutdown();
		return;
	}
	
	delete returnValue;
}
