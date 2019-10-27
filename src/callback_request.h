/*
	callback_request.h - header file for the CallbackRequest class.
	
	Revision 0
	
	Notes:
			- 
			
	2016/11/19, Maya Posch
	(c) Nyanko.ws
*/


#ifndef REQUEST_H
#define REQUEST_H


#include "abstract_request.h"
#include "nymph_message.h"


#include <string>
#include <atomic>
#include <condition_variable>
#include <mutex>


class CallbackRequest : public AbstractRequest {
	uint32_t session;
	NymphMessage* msg;
	void* data;
	std::string loggerName;
	
public:
	CallbackRequest() { loggerName = "CallbackRequest"; }
	void setMessage(uint32_t session, NymphMessage* msg, void* data);
	void process();
	void finish();
};

#endif
