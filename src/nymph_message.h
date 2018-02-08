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

#include <Poco/Poco.h>

using namespace Poco;

#include <vector>

using namespace std;


enum {
	NYMPH_MESSAGE_REPLY = 0x01,		// Message is a reply.
	NYMPH_MESSAGE_EXCEPTION = 0x02,	// Message is an exception.
	NYMPH_MESSAGE_CALLBACK = 0x04	// Message is a callback.
};


struct NymphException {
	UInt32 id;
	string value;
};


class NymphMessage {
	vector<NymphType*> values;
	UInt32 command;
	UInt32 flags;
	UInt32 methodId;
	int state;
	UInt64 messageId;
	UInt64 responseId;
	NymphException exception;
	bool hasResult;
	bool responseOwned;
	string callbackName;
	NymphType* response;
	string loggerName;
	
public:
	NymphMessage();
	NymphMessage(UInt32 methodId);
	NymphMessage(string binmsg);
	~NymphMessage();
	bool addValue(NymphType* value);
	bool finish(string &output);
	int getState() { return state; }
	void setInReplyTo(UInt64 msgId);
	bool isCallback() { return flags & NYMPH_MESSAGE_CALLBACK; }
	UInt64 getResponseId() { return responseId; }
	UInt64 getMessageId() { return messageId; }
	void setResultValue(NymphType* value);
	NymphType* getResponse() { return response; responseOwned = false; }
	vector<NymphType*> parameters() { return values; }
	UInt32 getMethodId() { return methodId; }
	NymphMessage* getReplyMessage();
	NymphException getException() { return exception; }
	string getCallbackName() { return callbackName; }
	bool isReply() { return flags & NYMPH_MESSAGE_REPLY; }
	bool isException() { return flags & NYMPH_MESSAGE_EXCEPTION; }
	bool setException(int exceptionId, string value);
	bool setCallback(string name);
};

#endif
