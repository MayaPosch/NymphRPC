/*
	nymph_message.h	- Declares the NymphRPC Message class.
	
	Revision 0
	
	Notes:
			- 
			
	2017/06/24, Maya Posch : Initial version.
	(c) Nyanko.ws
*/


#pragma once
#ifndef NYMPH_MESSAGE_H
#define NYMPH_MESSAGE_H

#include "nymph_types.h"

//#include <Poco/Poco.h>

#include <vector>
#include <atomic>


enum {
	NYMPH_MESSAGE_REPLY = 0x01,		// Message is a reply.
	NYMPH_MESSAGE_EXCEPTION = 0x02,	// Message is an exception.
	NYMPH_MESSAGE_CALLBACK = 0x04	// Message is a callback.
};


struct NymphException {
	uint32_t id;
	std::string value;
};


class NymphMessage {
	std::vector<NymphType*> values;
	uint32_t command;
	uint32_t flags;
	uint32_t methodId;
	int state;
	bool corrupt;
	uint64_t messageId;
	uint64_t responseId;
	NymphException exception;
	bool hasResult;
	std::string callbackName;
	NymphType* response = 0;
	std::string loggerName;
	uint8_t* data_buffer;
	uint32_t buffer_length;
	bool responseOwned = true;
	std::atomic<uint32_t> refCount = { 0 };
	std::atomic<bool> deleted = { false };
	
public:
	NymphMessage();
	NymphMessage(uint32_t methodId);
	NymphMessage(uint8_t* binmsg, uint64_t bytes);
	~NymphMessage();
	bool addValue(NymphType* value);
	bool addValues(std::vector<NymphType*> &values);
	
	void serialize();
	uint8_t* buffer() { return data_buffer; }
	uint32_t buffer_size() { return buffer_length; }
	
	int getState() { return state; }
	bool isCorrupt() { return corrupt; }
	
	void setInReplyTo(uint64_t msgId);
	bool isCallback() { return flags & NYMPH_MESSAGE_CALLBACK; }
	uint64_t getResponseId() { return responseId; }
	uint64_t getMessageId() { return messageId; }
	void setResultValue(NymphType* value);
	NymphType* getResponse(bool take = false) { responseOwned = take; return response; }
	std::vector<NymphType*>& parameters() { return values; }
	uint32_t getMethodId() { return methodId; }
	NymphMessage* getReplyMessage();
	NymphException getException() { return exception; }
	std::string getCallbackName() { return callbackName; }
	bool isReply() { return flags & NYMPH_MESSAGE_REPLY; }
	bool isException() { return flags & NYMPH_MESSAGE_EXCEPTION; }
	bool setException(int exceptionId, std::string value);
	bool setCallback(std::string name);
	
	void addReferenceCount();
	void decrementReferenceCount();
	void discard();
};

#endif
