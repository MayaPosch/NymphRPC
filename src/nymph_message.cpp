/*
	nymph_message.cpp	- Implements the NymphRPC Message class.
	
	Revision 0
	
	Notes:
			- 
			
	History:
	2017/06/24, Maya Posch : Initial version.
	
	(c) Nyanko.ws
*/

#include "nymph_message.h"
#include "nymph_utilities.h"
#include "nymph_logger.h"

#include <sstream>
#include <algorithm>
#include <cstdlib>

// debug
#include <iostream>

using namespace std;

#include <Poco/NumberFormatter.h>

using namespace Poco;


// --- CONSTRUCTOR ---
NymphMessage::NymphMessage() {
	flags = 0;
	state = 0;
	responseId = 0;
	hasResult = false;
	loggerName = "NymphMessage";
	corrupt = false;
	
	data_buffer = 0;
	buffer_length = 0;
}


NymphMessage::NymphMessage(uint32_t methodId) {
	flags = 0;
	state = 0; // no error
	responseId = 0;
	hasResult = false;
	this->methodId = methodId;
	loggerName = "NymphMessage";
	corrupt = false;
	
	data_buffer = 0;
	buffer_length = 0;
}


// Deserialises a binary Nymph message.
NymphMessage::NymphMessage(uint8_t* binmsg, uint64_t bytes) {
	flags = 0;
	state = 0; // no error
	responseId = 0;
	hasResult = false;
	corrupt = false;
	
	loggerName = "NymphMessage";
	data_buffer = binmsg;
	buffer_length = bytes;
	
	// The string we receive here is stripped of the Nymph header (0x4452474e, 'DRGN')
	// as well as the length of the message.
	// First we get the Nymph protocol version (0x00), then the command index number 
	// (<uint32>).
	uint8_t version = 0;
	methodId = 0;
	
	int index = 0;
	version = *binmsg;
	index++;
	methodId = *((uint32_t*) (binmsg + index));
	index += 4;
	
	NYMPH_LOG_DEBUG("Method ID: " + NumberFormatter::format(methodId) + ".");
	
	
	if (version != 0x00) {
		// Handle wrong version.
		NYMPH_LOG_ERROR("Wrong Nymph version: " + NumberFormatter::format(version) + ".");
		
		state = -1;
		corrupt = true;
		return;
	}
	
	// Read the message flags.
	flags = *((uint32_t*) (binmsg + index));
	index += 4;
	
	NYMPH_LOG_DEBUG("Message flags: 0x" + NumberFormatter::formatHex(flags));
	
	// Read the message ID & optionally the request message ID (if response).
	messageId = *((uint64_t*) (binmsg + index));
	index += 8;
	
	uint8_t typecode;
	if (flags & NYMPH_MESSAGE_REPLY) {
		responseId = *((uint64_t*) (binmsg + index));
		index += 8;
		
		// Read in the response
		typecode = *(binmsg + index++);
		response = new NymphType;
		NymphUtilities::parseValue(typecode, binmsg, index, *response);
		
		if (index >= bytes) {
			// Out of bounds, abort.
			NYMPH_LOG_ERROR("Message parsing index out of bounds. Abort.");
			corrupt = true;
			return;
		}
		
		if (*(binmsg + index) != NYMPH_TYPE_NONE) {
			// We didn't reach the message end. 
			// FIXME: handle this case, maybe do some pre-flight checks.
			corrupt = true;
			return;
		}
		
		response->linkWithMessage(this);
	}
	else if (flags & NYMPH_MESSAGE_EXCEPTION) {
		responseId = *((uint64_t*) (binmsg + index));
		
		// Read in the exception (integer, string).
		typecode = *(binmsg + index++);
		NymphType value;
		NymphUtilities::parseValue(typecode, binmsg, index, value);
		if (value.valuetype() == NYMPH_UINT32) {
			exception.id = value.getUint32();
		}
		
		typecode = *(binmsg + index++);
		NymphUtilities::parseValue(typecode, binmsg, index, value);
		if (value.valuetype() == NYMPH_STRING) {
			exception.value = std::string(value.getChar(), value.string_length());
		}
	}
	else if (flags & NYMPH_MESSAGE_CALLBACK) {
		// Read in the name of the callback method.
		typecode = *(binmsg + index++);
		NymphType value;
		NymphUtilities::parseValue(typecode, binmsg, index, value);
		if (value.valuetype() == NYMPH_STRING) {
			callbackName = std::string(value.getChar(), value.string_length());
		}
		
		// Read in the parameter values.
		while (index < bytes && *(binmsg + index) != NYMPH_TYPE_NONE) {		
			typecode = *(binmsg + index++);
			NymphType* val = new NymphType;
			NymphUtilities::parseValue(typecode, binmsg, index, *val);
			val->linkWithMessage(this);
			values.push_back(val);
			
			if (index >= bytes) {
				NYMPH_LOG_ERROR("Reached end of message without terminator found.");
				NYMPH_LOG_ERROR("Message is likely corrupt.");
				
				corrupt = true;
				
				break;
			}
		}
	}
	else {
		if (!(index < bytes)) {
			NYMPH_LOG_ERROR("Index is beyond message bounds. Corrupted message.");
			corrupt = true;
			return;
		}
		
		// Read in the parameter values.
		while (index < bytes && *(binmsg + index) != NYMPH_TYPE_NONE) {
			typecode = *(binmsg + index++);
			NymphType* val = new NymphType;
			NymphUtilities::parseValue(typecode, binmsg, index, *val);
			val->linkWithMessage(this);
			values.push_back(val);
			
			if (index >= bytes) {
				NYMPH_LOG_ERROR("Reached end of message without terminator found.");
				NYMPH_LOG_ERROR("Message is likely corrupt.");
				
				corrupt = true;
				
				break;
			}
		}
	}
}


// --- DECONSTRUCTOR ---
// Delete all values stored in this message since we have taken ownership.
NymphMessage::~NymphMessage() {
	if (data_buffer && buffer_length > 0) {
		delete[] data_buffer;
	}
	
	for (int i = 0; i < values.size(); i++) {
		delete values[i];
	}
	
	if (responseOwned && response) {
		delete response;
	}
	
	values.clear();
}


// --- ADD VALUE ---
// Adds a new value to this message. Message takes ownership of value.
bool NymphMessage::addValue(NymphType* value) {
	values.push_back(value);
	
	// Add the binary size of the new value to the total count.
	buffer_length += value->bytes();
	
	return true;
}


// --- ADD VALUES ---
// Overwrite the internal values with the provided values.
bool NymphMessage::addValues(std::vector<NymphType*> &values) {
	this->values = values;
	
	// Reset and add the binary size of the new values to the total count.
	buffer_length = 0;
	for (int i = 0; i < values.size(); i++) {
		buffer_length += values[i]->bytes();
	}
	
	return true;
}


// --- SERIALIZE ---
// Serialise the message's data and update the internal message data buffer.
void NymphMessage::serialize() {
	uint8_t nymphNone = NYMPH_TYPE_NONE;
	
	NYMPH_LOG_DEBUG("Serialising message with flags: 0x" + NumberFormatter::formatHex(flags));
	
	// Header section.
	// * uint32		Signature => 'DRGN'
	// * uint32		Length rest of message
	// * uint8		Protocol version
	// * uint32		Method ID
	// * uint32		Flags
	// * uint64		Message ID
	//
	// Reply message:
	// * <header>
	// * uint64		ReplyTo ID
	// * ?			Serialised reply.
	// * uint8		None
	//
	// Exception message:
	// * <header>
	// * uint64		ReplyTo ID
	// * uint32		Exception ID
	// * 
	// * uint8		None
	//
	// Callback message:
	// * <header>
	// * uint8		String typecode
	// * uint8-64	String length
	// * ?			Callback name
	// * ?			Serialised values.
	// * uint8		None
	//
	// Regular message:
	// * <header>
	// * ?			Serialised values.
	// * uint8		None
	uint32_t signature = 0x4452474e; // 'DRGN'
	
	// Message length is always:
	// * 1 byte from the header (version).
	// * 4 bytes method ID.
	// * 4 bytes message flags.
	// * 8 bytes message ID.
	// * 1 byte from the terminating typecode (Nymph None).
	// ===
	// 18 bytes + other values size.
	// 
	// For a response message, add another 8 bytes to the length. (incl. exceptions).
	// For a callback message, add 1 byte + callback name length.
	//length = (uint32_t) (content.length() + 18);
	uint32_t message_length = 18 + buffer_length;
	if (flags & NYMPH_MESSAGE_REPLY) { message_length += 8; }
	if (flags & NYMPH_MESSAGE_EXCEPTION) { message_length += 8; }
	else if (flags & NYMPH_MESSAGE_CALLBACK) {
		NymphType cbn(&callbackName);
		message_length +=  cbn.bytes();
	}
	
	NYMPH_LOG_DEBUG("Message with length: " + Poco::NumberFormatter::format(message_length));
	
	buffer_length = message_length + 8; // Add the size of the signature & version fields.
	data_buffer = new uint8_t[buffer_length];
	uint8_t* buf = data_buffer; // pointer to beginning.
	
	uint8_t version = 0x00;
	
	// If the first message in a series, add a new messageId.
	if (!(flags & NYMPH_MESSAGE_REPLY)) { messageId = NymphUtilities::getMessageId(); }
	
	// Write header into buffer.
	// FIXME: On a big-endian system all integers will be in the wrong byte order.
	// TODO: add endianness-check. Currently assume LE.
	*((uint32_t*) buf) = signature;
	buf += 4;
	*((uint32_t*) buf) = message_length;
	buf += 4;
	*buf = version;
	buf++;
	*((uint32_t*) buf) = methodId;
	buf += 4;
	*((uint32_t*) buf) = flags;
	buf += 4;
	*((uint64_t*) buf) = messageId;
	buf += 8;
	
	
	// Message types.
	if (flags & NYMPH_MESSAGE_REPLY) {
		*((uint64_t*) buf) = responseId;
		buf += 8;
		response->serialize(buf);
	}
	else if (flags & NYMPH_MESSAGE_EXCEPTION) {
		*((uint64_t*) buf) = responseId;
		buf += 8;
		
		*((uint32_t*) buf) = exception.id;
		buf += 4;
		
		NymphType exstr(&exception.value);
		exstr.serialize(buf);
	}
	else if (flags & NYMPH_MESSAGE_CALLBACK) {
		NymphType cbn(&callbackName);
		cbn.serialize(buf);
		
		unsigned int valueLen = values.size();
		for (unsigned int i = 0; i < valueLen; ++i) {
			values[i]->serialize(buf);
		}
	}
	else {
		unsigned int valueLen = values.size();
		for (unsigned int i = 0; i < valueLen; ++i) {
			values[i]->serialize(buf);
		}
	}
	
	*buf = nymphNone;
}


// --- SET IN REPLY TO ---
// Set the message ID we're responding to with this message. Set the 'reply' bitflag, too.
void NymphMessage::setInReplyTo(uint64_t msgId) {
	responseId = msgId;
	messageId = msgId + 1;
	
	
	NYMPH_LOG_DEBUG("New message flags: 0x" + NumberFormatter::formatHex(flags));
}


// --- SET RESULT VALUE ---
// Sets the result value for a response message.
void NymphMessage::setResultValue(NymphType* value) {
	flags |= NYMPH_MESSAGE_REPLY;
	response = value;
	buffer_length = response->bytes();
}


// --- GET REPLY MESSAGE ---
// Returns a new Nymph message instance, prefilled with just the 'responseId' and
// 'messageId' values.
NymphMessage* NymphMessage::getReplyMessage() {
	NymphMessage* msg = new NymphMessage(methodId);
	msg->setInReplyTo(messageId);
	return msg;
}


// --- SET EXCEPTION ---
// Set the value for an exception to be sent with the message.
// This is generally used by the receiving side to indicate an issue with the
// message itself, or a service being accessed.
// The message instance takes ownership of the value.
bool NymphMessage::setException(int exceptionId, string value) {
	flags |= NYMPH_MESSAGE_EXCEPTION;
	exception.id = exceptionId;
	exception.value = value;
	
	return true;
}


// --- SET CALLBACK ---
// Enable the status to that of a callback message (server->client).
bool NymphMessage::setCallback(std::string name) {
	flags |= NYMPH_MESSAGE_CALLBACK;
	callbackName = name;
	return true;
}


// --- ADD REFERENCE COUNT ---
void NymphMessage::addReferenceCount() {
	refCount++;
	NYMPH_LOG_DEBUG("[" + Poco::NumberFormatter::format(messageId) + "] Holding " + Poco::NumberFormatter::format(refCount) + " references. (+1)");
}


// --- DECREMENT REFERENCE COUNT ---
void NymphMessage::decrementReferenceCount() {
	if (deleted) { return; }
	
	refCount--;
	
	NYMPH_LOG_DEBUG("[" + Poco::NumberFormatter::format(messageId) + "] Holding " + Poco::NumberFormatter::format(refCount) + " references. (-1)");
	
	if (refCount == 0) {
		delete this;
	}
}


// --- DISCARD ---
void NymphMessage::discard() {
	// The message will no longer be used by the handling function. Check that we can safely delete.
	// This depends on whether a response or a value is currently being used.
	
	deleted = true;
	
	delete this;
}
