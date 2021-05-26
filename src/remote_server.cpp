/*
	remote_server.cpp	- implementation for the Remote Server class.
	
	Revision 0
	
	Features:
			- 
	
	Notes:
			- This class declares the main class to be used by NymphRPC clients.
			
	History:
	2017/06/24, Maya Posch : Initial version.
*/


#include <Poco/Net/NetException.h>
#include <Poco/NumberFormatter.h>

using namespace Poco;

#include "remote_server.h"
#include "nymph_utilities.h"

using namespace std;

#include "dispatcher.h"


// Static initialisations
uint32_t NymphRemoteServer::lastHandle = 0;
long NymphRemoteServer::timeout = 3000;
string NymphRemoteServer::loggerName = "NymphRemoteServer";
uint32_t NymphRemoteServer::nextMethodId = 0;
std::map<uint32_t, NymphServerInstance*> NymphRemoteServer::instances;
Poco::Mutex NymphRemoteServer::instancesMutex;


// --- NYMPH SERVER INSTANCE ---
//--- CONSTRUCTOR ---
NymphServerInstance::NymphServerInstance(uint32_t handle, Poco::Net::StreamSocket* socket, uint32_t timeout) 
	: handle(handle), socket(socket), timeout(timeout) { 
	socketSemaphore = new Poco::Semaphore(0, 1);
	
	// Register built-in synchronisation method ('nymphsync').
	vector<NymphTypes> parameters;
	NymphMethod syncFunction("nymphsync", parameters, NYMPH_STRING);
	addMethod("nymphsync", syncFunction);
}


// --- DESTRUCTOR ---
NymphServerInstance::~NymphServerInstance() { }
	

// --- SET HANDLE ---
void NymphServerInstance::setHandle(uint32_t handle) {
	this->handle = handle;
}


// --- HANDLE ---
uint32_t NymphServerInstance::getHandle() { return handle; }


// --- SEMAPHORE ---
Poco::Semaphore* NymphServerInstance::semaphore() { return socketSemaphore; }


// --- SYNC ---
// Synchronises the function list on the client with that of the server.
// Automatically called once upon connecting to a new Nymph server instance.
bool NymphServerInstance::sync(std::string &result) {
	// Send a message to the server with function ID 0 ('sync'), then wait for
	// the response.
	NYMPH_LOG_DEBUG("Sync: calling remote server...");
	vector<NymphType*> values;
	NymphType* retval = 0;
	//if (!callMethodId(handle, 0, values, retval, result)) {
	if (!callMethod("nymphsync", values, retval, result)) {
		NYMPH_LOG_DEBUG("Sync: failed to call remote sync method.");
		return false;
	}
	
	// Parse results.
	// We should have received a NymphString instance containing the 
	// serialised methods.
	if (!retval || retval->type() != NYMPH_STRING) { 
		NYMPH_LOG_ERROR("Invalid return value from remote for sync.");
		return false; 
	}
	
	NYMPH_LOG_DEBUG("Received sync response.");
	
	string binmsg = ((NymphString*) retval)->getValue();
	if (binmsg.length() < 11) { return false; }
	UInt32 index = 0;
	string signature = binmsg.substr(0, 7);
	index += 7;
	UInt32 methodCount = *((UInt32*) &binmsg[index]);
	index += 4;
	
	NYMPH_LOG_DEBUG("Received " + NumberFormatter::format(methodCount) + " methods.");
	
	if (signature != "METHODS") {
		NYMPH_LOG_DEBUG("Sync: METHODS signature wasn't found.");
		return false; 
	}
	
	if (methodCount == 0) {
		NYMPH_LOG_DEBUG("Sync: method count was zero.");
		return false; 
	}
	
	NYMPH_LOG_DEBUG("Parsing methods...");
	
	// Parse the methods. The IDs are expected to start at 0 and
	// increment without gaps.
	//UInt32 nextMethodId = 0;
	for (UInt32 i = 0; i < methodCount; ++i) {
		signature = binmsg.substr(index, 6);
		index += 6;
		UInt32 methodId = *((UInt32*) &binmsg[index]);
		index += 4;
		
		NYMPH_LOG_DEBUG("Validating method...");
		
		if (signature != "METHOD") {
			NYMPH_LOG_DEBUG("Sync: METHOD signature wasn't found.");
			return false; 
		}
		
		/* if (methodId != nextMethodId) {
			NYMPH_LOG_DEBUG("Sync: methodId doesn't match expected ID.");
			return false; 
		} */
		
		//++nextMethodId;
		
		UInt8 l = *((UInt8*) &binmsg[index++]);
		string methodName = binmsg.substr(index, l);
		index += l;
		
		NYMPH_LOG_DEBUG("Synchronising method: " + methodName);
		
		vector<NymphTypes> parameters;
		l = *((UInt8*) &binmsg[index++]);
		for (UInt8 i = 0; i < l; ++i) {
			UInt8 t = *((UInt8*) &binmsg[index++]);
			parameters.push_back((NymphTypes) t);
		}
		
		UInt8 t = *((UInt8*) &binmsg[index++]);
		
		// Skip the 'sync' method, as we already have it registered.
		if (methodId == 0) {
			NYMPH_LOG_DEBUG("Skipping sync method...");
			continue; 
		}
		
		NymphMethod method(methodName, parameters, (NymphTypes) t);
		addMethod(methodName, method);
	}
	
	return true;
}

	
// --- ADD METHOD ---
bool NymphServerInstance::addMethod(std::string name, NymphMethod method) {
	// Add the method to the map for the specified handle.
	methodsMutex.lock();
	method.setId(nextMethodId++);
	std::pair<map<string, NymphMethod>::iterator, bool> newPair;
	newPair = methods.insert(pair<string, NymphMethod>(name, method));
	
	// TODO: check whether a new method was inserted or just overwritten.
	// Create a reference to the method instance in the methods map here.
	methodIds.insert(pair<UInt32, NymphMethod*>(method.getId(), &(newPair.first->second)));
	methodsMutex.unlock();
	
	return true;
}


// --- GET METHOD ---
bool NymphServerInstance::callMethod(std::string name, std::vector<NymphType*> &values, 
										NymphType* &returnvalue, std::string &result) {	
	NYMPH_LOG_DEBUG("Called method: " + name);
	
	// Get the method.
	methodsMutex.lock();
	map<string, NymphMethod>::iterator mit;
	mit = methods.find(name);
	if (mit == methods.end()) {
		result = "Specified method name was not found.";
		methodsMutex.unlock();
		
		// Delete the values in the values vector since we own them.
		std::vector<NymphType*>::iterator it;
		for (it = values.begin(); it != values.end(); ++it) { delete (*it); }
		return false;
	}
	
	// Add NymphRequest to listener.
	NymphRequest* request = new NymphRequest;
	request->response = 0;
	request->exception = false;
	request->handle = handle;
	request->mutex.lock();
	
	// Call the method instance. Ownership of the values vector is transferred
	// to this instance.
	bool ret = mit->second.call(socket, request, values, result);
	methodsMutex.unlock();
	
	if (!ret) { return false; }
	
	// Wait for the message response, else return time-out error.
	// We use tryWait() since it's exception-free.
	if (!request->condition.tryWait(request->mutex, timeout)) {
		// Handle timeout of the message.
		result = "Method call for " + name + " timed out while waiting for response.";
		request->mutex.unlock();
		NymphListener::removeMessage(handle, request->messageId);
		return false;
	}
	
	request->mutex.unlock();
	
	// Remove message from listener since we're done with it.
	NymphListener::removeMessage(handle, request->messageId);
	
	// Check for an exception.
	if (request->exception) {
		NYMPH_LOG_DEBUG("Exception found: " + request->exceptionData.value);
		
		result = to_string(request->exceptionData.id) + " - " + request->exceptionData.value;
		returnvalue = 0;
	}
	else {
		// Set output result. This is a singular NymphType value.
		returnvalue = request->response;
	}
	
	delete request;
	
	return true;
}


// --- CALL METHOD ID ---
bool NymphServerInstance::callMethodId(uint32_t id, std::vector<NymphType*> &values, NymphType* &returnvalue, std::string &result) {
	NYMPH_LOG_DEBUG("Called method ID: " + NumberFormatter::format(id));
	
	// Get the method.
	methodsMutex.lock();
	map<uint32_t, NymphMethod*>::iterator mit;
	mit = methodIds.find(id);
	if (mit == methodIds.end()) {
		result = "Specified method name was not found.";
		methodsMutex.unlock();
		return false;
	}
	
	// Add NymphRequest to listener.
	NymphRequest* request = new NymphRequest;
	request->response = 0;
	request->handle = handle;
	request->mutex.lock();
	
	bool ret = mit->second->call(socket, request, values, result);
	methodsMutex.unlock();
	
	if (!ret) { return false; }
	
	// Wait for the message response, else return time-out error.
	// We use tryWait() since it's exception-free.
	if (!request->condition.tryWait(request->mutex, timeout)) {
		// Handle timeout of the message.
		result = "Method call for ID " + NumberFormatter::format(id) + " timed out while waiting for response.";
		request->mutex.unlock();
		NymphListener::removeMessage(handle, request->messageId);
		return false;
	}
	
	request->mutex.unlock();
	
	// Remove message from listener since we're done with it.
	NymphListener::removeMessage(handle, request->messageId);
	
	// Check for an exception.
	if (request->exception) {
		result = to_string(request->exceptionData.id) + " - " + request->exceptionData.value;
		returnvalue = 0;
	}
	else {
		// Set output result. This is a singular NymphType value.
		returnvalue = request->response;
	}
	
	delete request;
	
	return true;
}


// --- REMOVE METHOD ---
bool NymphServerInstance::removeMethod(std::string name) {
	methodsMutex.lock();
	map<string, NymphMethod>::iterator it;
	it = methods.find(name);
	if (it != methods.end()) {
		methods.erase(it);
	}
	
	// TODO: remove from methods ID map as well.
	
	methodsMutex.unlock();
	
	return true;
}


// --- DISCONNECT ---
bool NymphServerInstance::disconnect(std::string& result) {
	// Shutdown socket. Set the semaphore once done to signal that the socket's 
	// listener thread that it's safe to delete the socket.
	bool res = true;
	try {
		socket->shutdown();
		socket->close();
	}
	catch (Poco::Net::NetException &ex) {
		result = "Net exception: " + ex.displayText();
		res = false;
	}
	catch (Poco::TimeoutException &ex) {
		result = "Connect timed out: " + ex.displayText();
		res = false;
	}
	catch (...) {
		result = "Caught unknown exception.";
		res = false;
	}
	
	socketSemaphore->set();
	
	// Remove socket from listener.
	NymphListener::removeConnection(handle);
	
	return res;
}
// ---


// --- INIT ---
// Initialise the runtime and sets the logger function to be used by the Nymph 
// Logger class, along with the desired maximum log level:
// NYMPH_LOG_LEVEL_FATAL = 1,
// NYMPH_LOG_LEVEL_CRITICAL,
// NYMPH_LOG_LEVEL_ERROR,
// NYMPH_LOG_LEVEL_WARNING,
// NYMPH_LOG_LEVEL_NOTICE,
// NYMPH_LOG_LEVEL_INFO,
// NYMPH_LOG_LEVEL_DEBUG,
// NYMPH_LOG_LEVEL_TRACE
bool NymphRemoteServer::init(logFnc logger, int level, long timeout) {
	NymphRemoteServer::timeout = timeout;
	setLogger(logger, level);
	
	// Start the dispatcher runtime.
	Dispatcher::init(10); // 10 worker threads.
	
	return true;
}


// --- SET LOGGER ---
// Sets the logger function to be used by the Nymph Logger class, along with the
// desired maximum log level:
// NYMPH_LOG_LEVEL_FATAL = 1,
// NYMPH_LOG_LEVEL_CRITICAL,
// NYMPH_LOG_LEVEL_ERROR,
// NYMPH_LOG_LEVEL_WARNING,
// NYMPH_LOG_LEVEL_NOTICE,
// NYMPH_LOG_LEVEL_INFO,
// NYMPH_LOG_LEVEL_DEBUG,
// NYMPH_LOG_LEVEL_TRACE
void NymphRemoteServer::setLogger(logFnc logger, int level) {
	NymphLogger::setLoggerFunction(logger);
	NymphLogger::setLogLevel((Poco::Message::Priority) level);
}


// --- SHUTDOWN ---
// Shutdown the runtime. Close any open connections and clean up resources.
bool NymphRemoteServer::shutdown() {
	map<uint32_t, NymphServerInstance*>::iterator it;
	for (it = instances.begin(); it != instances.end(); ++it) {
		// Remove socket from listener.
		NymphListener::removeConnection(it->first);
		
		// Disconnect.
		std::string result;
		it->second->disconnect(result);
	}
	
	// Clear instances map.
	instances.clear();
	
	NymphListener::stop();
	
	return true;
}

 
// --- CONNECT ---
// Create a new connection with the remote Nymph server and return a handle for
// the connection.
bool NymphRemoteServer::connect(string host, int port, uint32_t &handle, void* data, 
															string &result) {
	Poco::Net::SocketAddress sa(host, port);
	return connect(sa, handle, data, result);
}


bool NymphRemoteServer::connect(string url, uint32_t &handle, void* data, 
															string &result) {
	Poco::Net::SocketAddress sa(url);
	return connect(sa, handle, data, result);
}


bool NymphRemoteServer::connect(Poco::Net::SocketAddress sa, uint32_t &handle, 
												void* data, string &result) {
	Poco::Net::StreamSocket* socket;
	try {
		socket = new Poco::Net::StreamSocket(sa);
	}
	catch (Poco::Net::ConnectionRefusedException &ex) {
		// Handle connection error.
		result = "Unable to connect: " + ex.displayText();
		return false;
	}
	catch (Poco::InvalidArgumentException &ex) {
		result = "Invalid argument: " + ex.displayText();
		return false;
	}
	catch (Poco::Net::InvalidSocketException &ex) {
		result = "Invalid socket exception: " + ex.displayText();
		return false;
	}
	catch (Poco::Net::NetException &ex) {
		result = "Net exception: " + ex.displayText();
		return false;
	}
	catch (Poco::TimeoutException &ex) {
		result = "Connect timed out: " + ex.displayText();
		return false;
	}
	
	// Create new NymphServerInstance instance for this connection.
	// Add it to the instances map.
	instancesMutex.lock();
	NymphServerInstance* si = new NymphServerInstance(lastHandle, socket);
	instances.insert(std::pair<uint32_t, NymphServerInstance*>(lastHandle, si));
	instancesMutex.unlock();
	
	NymphSocket ns;
	ns.socket = socket;
	ns.semaphore = si->semaphore();
	ns.data = data;
	ns.handle = lastHandle;
	NymphListener::addConnection(lastHandle, ns);
	handle = lastHandle++;
	
	NYMPH_LOG_DEBUG("Added new connection with handle: " + NumberFormatter::format(handle));
	
	// Fetch remote method signatures from the server.
	if (!si->sync(result)) {
		return false;
	}

	return true;
}


// --- DISCONNECT ---
bool NymphRemoteServer::disconnect(uint32_t handle, string &result) {
	map<uint32_t, NymphServerInstance*>::iterator it;
	instancesMutex.lock();
	it = instances.find(handle);
	if (it == instances.end()) { 
		result = "Provided handle " + NumberFormatter::format(handle) + " was not found.";
		instancesMutex.unlock();
		return false; 
	}
	
	// TODO: try/catch.
	// Tell server instance to disconnect.
	if (!it->second->disconnect(result)) {
		instancesMutex.unlock();
		return false;
	}
	
	// Delete instance.
	delete it->second;
	instances.erase(it);
	
	// Remove socket from listener.
	NymphListener::removeConnection(handle);
	
	instancesMutex.unlock();
	
	NYMPH_LOG_DEBUG("Removed connection with handle: " + NumberFormatter::format(handle));
	
	return true;
}


// --- CALL METHOD ---
bool NymphRemoteServer::callMethod(uint32_t handle, string name, vector<NymphType*> &values,
										NymphType* &returnvalue, string &result) {
	map<uint32_t, NymphServerInstance*>::iterator it;
	instancesMutex.lock();
	it = instances.find(handle);
	if (it == instances.end()) { 
		result = "Provided handle " + NumberFormatter::format(handle) + " was not found.";
		instancesMutex.unlock();
		return false; 
	}
	
	// TODO: try/catch.
	// Tell server instance to disconnect.
	if (!it->second->callMethod(name, values, returnvalue, result)) {
		instancesMutex.unlock();
		return false;
	}
	
	instancesMutex.unlock();
	
	return true;
}


// --- CALL METHOD ID ---
bool NymphRemoteServer::callMethodId(uint32_t handle, uint32_t id, vector<NymphType*> &values, NymphType* &returnvalue, string &result) {
	std::map<uint32_t, NymphServerInstance*>::iterator it;
	instancesMutex.lock();
	it = instances.find(handle);
	if (it == instances.end()) { 
		result = "Provided handle " + NumberFormatter::format(handle) + " was not found.";
		instancesMutex.unlock();
		return false; 
	}
	
	// TODO: try/catch.
	// Tell server instance to disconnect.
	if (!it->second->callMethodId(id, values, returnvalue, result)) {
		instancesMutex.unlock();
		return false;
	}
	
	instancesMutex.unlock();
		
	return true;
}


// --- REMOVE METHOD ---
bool NymphRemoteServer::removeMethod(uint32_t handle, string name) {
	map<uint32_t, NymphServerInstance*>::iterator it;
	instancesMutex.lock();
	it = instances.find(handle);
	if (it == instances.end()) { 
		//result = "Provided handle " + NumberFormatter::format(handle) + " was not found.";
		instancesMutex.unlock();
		return false; 
	}
	
	// TODO: try/catch.
	// Tell server instance to disconnect.
	if (!it->second->removeMethod(name)) {
		instancesMutex.unlock();
		return false;
	}
	
	instancesMutex.unlock();
	
	return true;
}


// --- REGISTER CALLBACK ---
bool NymphRemoteServer::registerCallback(string name, 
											NymphCallbackMethod method, 
											void* data) {
	NymphCallback cb;
	cb.name = name;
	cb.method = method;
	cb.data = data;
	
	NymphListener::addCallback(cb);
	
	return true;
}


// --- REMOVE CALLBACK ---
bool NymphRemoteServer::removeCallback(string name) {
	NymphListener::removeCallback(name);
	
	return true;
}
