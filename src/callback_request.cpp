/*
	callback_request.cpp - implementation of the Request class.
	
	Revision 0
	
	Notes:
			- 
			
	2016/11/19, Maya Posch
	(c) Nyanko.ws
*/


#include "callback_request.h"
#include "nymph_listener.h"
#include "nymph_logger.h"


// --- SET MESSAGE ---
void CallbackRequest::setMessage(uint32_t session, NymphMessage* msg, void* data) { 
	this->session = session;
	this->msg = msg;
	this->data = data; 
}


// --- PROCESS ---
void CallbackRequest::process() {
	if (!NymphListener::callCallback(session, msg, data)) {
		//NYMPH_LOG_ERROR("Calling callback failed. Skipping message.");
		delete msg;
	}
	
	//NYMPH_LOG_INFORMATION("Calling callback succeeded.");
}


// --- FINISH ---
void CallbackRequest::finish() {
	// Call own destructor.
	delete this;
}
