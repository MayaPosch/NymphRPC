/*
	nymph_types.cpp	- Defines the NymphRPC data types.
	
	Revision 1
	
	Notes:
			- 
			
	History:
	2017/06/24, Maya Posch : Initial version.
	2021/10/01, Maya Posch : New type system.
	
	(c) Nyanko.ws
*/


#include "nymph_types.h"
#include "nymph_utilities.h"
#include "nymph_logger.h"
#include "nymph_message.h"

#include <sstream>
#include <iostream>
#include <algorithm>
#include <cstdlib>

using namespace std;

#include <Poco/NumberFormatter.h>
#include <Poco/JSON/Stringifier.h>
#include <Poco/JSON/Parser.h>
#include <Poco/Base64Decoder.h>
#include <Poco/Exception.h>

using namespace Poco;


string NymphType::loggerName = "NymphType";


uint64_t binaryStringLength(uint64_t len) {
	// Length is:
	// * typecode (string)
	// * typecode (uint*)
	// * byte length of uint*
	uint64_t length;
	if (len <= 0xFF) { length = 3 + len; }
	else if (len <= 0xFFFF) { length = 4 + len; }
	else if (len <= 0xFFFFFFFF) { length = 6 + len; }
	else { length = 10 + len; }
	
	return length;
}


// --- CONSTRUCTORS ---
// Byte length for a type is calculated as its data size in bytes, plus the 1-byte typecode, plus
// any additional (meta) information.
NymphType::NymphType(bool v) 		{ type = NYMPH_BOOL;	length = 1;	data.boolean = v;	}
NymphType::NymphType(uint8_t v) 	{ type = NYMPH_UINT8;	length = 2;	data.uint8 = v;		}
NymphType::NymphType(int8_t v) 		{ type = NYMPH_SINT8;	length = 2;	data.int8 = v;		}
NymphType::NymphType(uint16_t v) 	{ type = NYMPH_UINT16;	length = 3;	data.uint16 = v;	}
NymphType::NymphType(int16_t v) 	{ type = NYMPH_SINT16;	length = 3; data.int16 = v;		}
NymphType::NymphType(uint32_t v) 	{ type = NYMPH_UINT32;	length = 5;	data.uint32 = v;	}
NymphType::NymphType(int32_t v) 	{ type = NYMPH_SINT32;	length = 5;	data.int32 = v;		}
NymphType::NymphType(uint64_t v) 	{ type = NYMPH_UINT64;	length = 9; data.uint64 = v;	}
NymphType::NymphType(int64_t v) 	{ type = NYMPH_SINT64;	length = 9; data.int64 = v;		}
NymphType::NymphType(float v) 		{ type = NYMPH_FLOAT;	length = 5;	data.fp32 = v;		}
NymphType::NymphType(double v) 		{ type = NYMPH_DOUBLE;	length = 9; data.fp64 = v;		}


NymphType::NymphType(char* v, uint32_t bytes, bool own) {
	type = NYMPH_STRING;
	length = binaryStringLength(bytes);
	strLength = bytes;
	data.chars = v;
	this->own = own;
	
	if (bytes == 0) { emptyString = true; }
}


NymphType::NymphType(std::string* v, bool own) {
	type = NYMPH_STRING;
	length = binaryStringLength(v->length());
	strLength = v->length();
	data.chars = v->data();
	this->own = own;
	
	if (own) 			{ string = v; }
	if (strLength == 0) { emptyString = true; }
}


NymphType::NymphType(std::vector<NymphType*>* v, bool own) {
	this->own = own;
	type = NYMPH_ARRAY;
	length = 0;
	data.vector = v;
	
	// Get length from all of the NymphTypes in the vector.
	for (int i = 0; i < v->size(); ++i) {
		length += (*v)[i]->bytes();
	}
	
	length += 10; // Add array type code & element count (uint64), plus terminator (1).
}


NymphType::NymphType(std::map<std::string, NymphPair>* v, bool own) {
	type = NYMPH_STRUCT;
	this->own = own;
	length = 0;
	data.pairs = v;
	
	// Determine byte size from the pairs.
	std::map<std::string, NymphPair>::iterator it;
	for (it = v->begin(); it != v->end(); it++) {
		length += (it->second.key)->bytes();
		length += (it->second.value)->bytes();
	}
	
	// Add typecode & terminator.
	length += 2;
	
}


// --- DESTRUCTOR ---
NymphType::~NymphType() {
	if (type == NYMPH_ARRAY) {
		if (linkedMsg) {
			linkedMsg->decrementReferenceCount();
		}
		
		if (own) {
			// Delete the std::vector & contents as we own it.
			for (int i = 0; i < data.vector->size(); i++) {
				delete (*data.vector)[i];
			}
			
			delete data.vector;
		}
	}
	else if (type == NYMPH_STRUCT) {
		if (linkedMsg) {
			linkedMsg->decrementReferenceCount();
		}
		
		if (own) {
			std::map<std::string, NymphPair>::iterator it;
			for (it = data.pairs->begin(); it != data.pairs->end(); it++) {
				delete it->second.key;
				delete it->second.value;
			}
			
			delete data.pairs;
		}
	}
	else if (type == NYMPH_STRING) {
		if (linkedMsg) {
			linkedMsg->decrementReferenceCount();
		}
		
		if (own) {
			if (string == 0) {
				delete data.chars;
			}
			else {
				delete string;
			}
		}
	}
}


// --- VALUE ---
//void* 		NymphType::value() 	{ return data.any; 		}
bool 		NymphType::getBool(bool* v)			{ if (v) { v = &data.boolean; }	return data.boolean;}
uint8_t 	NymphType::getUint8(uint8_t* v) 	{ if (v) { v = &data.uint8; } 	return data.uint8; 	}
int8_t 		NymphType::getInt8(int8_t* v) 		{ if (v) { v = &data.int8; } 	return data.int8; 	}
uint16_t 	NymphType::getUint16(uint16_t* v) 	{ if (v) { v = &data.uint16; }	return data.uint16; }
int16_t 	NymphType::getInt16(int16_t* v) 	{ if (v) { v = &data.int16; }	return data.int16; 	}
uint32_t 	NymphType::getUint32(uint32_t* v) 	{ if (v) { v = &data.uint32; }	return data.uint32; }
int32_t 	NymphType::getInt32(int32_t* v) 	{ if (v) { v = &data.int32; }	return data.int32; 	}
uint64_t 	NymphType::getUint64(uint64_t* v) 	{ if (v) { v = &data.uint64; }	return data.uint64; }
int64_t 	NymphType::getInt64(int64_t* v)  	{ if (v) { v = &data.int64; }	return data.int64; 	}
float 		NymphType::getFloat(float* v) 		{ if (v) { v = &data.fp32; }	return data.fp32; 	}
double 		NymphType::getDouble(double* v) 	{ if (v) { v = &data.fp64; }	return data.fp64; 	}
const char*	NymphType::getChar(const char* v)	{ if (v) { v = data.chars; }	return data.chars; 	}

std::vector<NymphType*>* NymphType::getArray(std::vector<NymphType*>* v) { 
	if (v) { v = data.vector; }
	return data.vector; 	
}


std::map<std::string, NymphPair>* NymphType::getStruct(std::map<std::string, NymphPair>* v) {
	if (v) { v = data.pairs; }
	return data.pairs; 	
}


std::string NymphType::getString() {
	return std::string(data.chars, strLength);
}


// --- GET STRUCT VALUE ---
bool NymphType::getStructValue(std::string key, NymphType* &value) {
	std::map<std::string, NymphPair>::iterator it = data.pairs->find(key);
	if (it == data.pairs->end()) { return false; }
	
	value = it->second.value;
	
	return true;
}


// --- SET VALUE ---
void NymphType::setValue(bool v) 		{ type = NYMPH_BOOL; 	length = 1; data.boolean = v; 	}
void NymphType::setValue(uint8_t v) 	{ type = NYMPH_UINT8;	length = 2; data.uint8 = v;		}
void NymphType::setValue(int8_t v) 		{ type = NYMPH_SINT8;	length = 2; data.int8 = v;		}
void NymphType::setValue(uint16_t v) 	{ type = NYMPH_UINT16;	length = 3;	data.uint16 = v;	}
void NymphType::setValue(int16_t v) 	{ type = NYMPH_SINT16;	length = 3;	data.int16 = v;		}
void NymphType::setValue(uint32_t v) 	{ type = NYMPH_UINT32;	length = 5; data.uint32 = v;	}
void NymphType::setValue(int32_t v) 	{ type = NYMPH_SINT32;	length = 5; data.int32 = v;		}
void NymphType::setValue(uint64_t v) 	{ type = NYMPH_UINT64;	length = 9; data.uint64 = v;	}
void NymphType::setValue(int64_t v) 	{ type = NYMPH_SINT64;	length = 9; data.int64 = v;		}
void NymphType::setValue(float v) 		{ type = NYMPH_FLOAT;	length = 5; data.fp32 = v;		}
void NymphType::setValue(double v) 		{ type = NYMPH_DOUBLE;	length = 9;	data.fp64 = v;		}


void NymphType::setValue(char* v, uint32_t bytes, bool own) {
	type = NYMPH_STRING;
	length = binaryStringLength(bytes);
	strLength = bytes;
	data.chars = v;
	this->own = own;
	
	if (bytes == 0) { emptyString = true; }
}


void NymphType::setValue(std::string* v, bool own) {
	type = NYMPH_STRING;
	length = binaryStringLength(v->length());
	strLength = v->length();
	data.chars = v->data();
	this->own = own;
	
	if (own) 			{ string = v; }
	if (strLength == 0) { emptyString = true; }
}


void NymphType::setValue(std::vector<NymphType*>* v, bool own) {
	this->own = own;
	type = NYMPH_ARRAY;
	length = 0;
	data.vector = v;
	
	// Get length from all of the NymphTypes in the vector.
	for (int i = 0; i < v->size(); ++i) {
		length += (*v)[i]->bytes();
	}
	
	length += 10; // Add array type code & element count (uint64), plus terminator (1).
}


void NymphType::setValue(std::map<std::string, NymphPair>* v, bool own) {
	type = NYMPH_STRUCT;
	this->own = own;
	length = 0;
	data.pairs = v;
	
	// Determine byte size from the pairs.
	std::map<std::string, NymphPair>::iterator it;
	for (it = v->begin(); it != v->end(); it++) {
		length += it->second.key->bytes();
		length += it->second.value->bytes();
	}
	
	// Add typecode & terminator.
	length += 2;
}


// --- PARSE VALUE ---
bool NymphType::parseValue(uint8_t typecode, uint8_t* binmsg, int &index) {
	switch (typecode) {
        case NYMPH_TYPE_NULL:
			NYMPH_LOG_DEBUG("NYMPH_TYPE_NONE");
			break;
        case NYMPH_TYPE_BOOLEAN_FALSE:
			NYMPH_LOG_DEBUG("NYMPH_TYPE_BOOLEAN_FALSE");
			type = NYMPH_BOOL;
			length = 1; 
			data.boolean = false;
			break;
        case NYMPH_TYPE_BOOLEAN_TRUE:
			NYMPH_LOG_DEBUG("NYMPH_TYPE_BOOLEAN_TRUE");
			type = NYMPH_BOOL;
			length = 1; 
			data.boolean = true;
			break;
		case NYMPH_TYPE_FLOAT: {
			NYMPH_LOG_DEBUG("NYMPH_TYPE_FLOAT");
			type = NYMPH_FLOAT;	
			length = 5; 
			memcpy(&(data.fp32), (binmsg + index), 4);
			index += 4;
			break;
		}
		case NYMPH_TYPE_DOUBLE:
			NYMPH_LOG_DEBUG("NYMPH_TYPE_DOUBLE");
			type = NYMPH_DOUBLE;	
			length = 9;	
			memcpy(&(data.fp64), (binmsg + index), 8);
			index += 8;
			break;
        case NYMPH_TYPE_UINT8:
			NYMPH_LOG_DEBUG("NYMPH_TYPE_UINT8");
			type = NYMPH_UINT8;	
			length = 2; 
			memcpy(&(data.uint8), (binmsg + index), 1);
			index++;
            break;
		case NYMPH_TYPE_SINT8:
			NYMPH_LOG_DEBUG("NYMPH_TYPE_SINT8");
			type = NYMPH_SINT8;	
			length = 2; 
			memcpy(&(data.int8), (binmsg + index), 1);
			index++;
			break;
		case NYMPH_TYPE_UINT16:
			NYMPH_LOG_DEBUG("NYMPH_TYPE_UINT16");
			type = NYMPH_UINT16;	
			length = 3;	
			memcpy(&(data.uint16), (binmsg + index), 2);
			index += 2;
			break;
		case NYMPH_TYPE_SINT16:
			NYMPH_LOG_DEBUG("NYMPH_TYPE_SINT16");
			type = NYMPH_SINT16;	
			length = 3;	
			memcpy(&(data.int16), (binmsg + index), 2);
			index += 2;
			break;
		case NYMPH_TYPE_UINT32:
			NYMPH_LOG_DEBUG("NYMPH_TYPE_UINT32");
			type = NYMPH_UINT32;	
			length = 5; 
			memcpy(&(data.uint32), (binmsg + index), 4);
			index += 4;
			break;
		case NYMPH_TYPE_SINT32:
			NYMPH_LOG_DEBUG("NYMPH_TYPE_SINT32");
			type = NYMPH_SINT32;	
			length = 5; 
			memcpy(&(data.int32), (binmsg + index), 4);
			index += 4;
			break;
		case NYMPH_TYPE_UINT64:
			NYMPH_LOG_DEBUG("NYMPH_TYPE_UINT64");
			type = NYMPH_UINT64;	
			length = 9; 
			memcpy(&(data.uint64), (binmsg + index), 8);
			index += 8;
			break;
		case NYMPH_TYPE_SINT64:
			NYMPH_LOG_DEBUG("NYMPH_TYPE_SINT64");
			type = NYMPH_SINT64;	
			length = 9; 
			memcpy(&(data.int64), (binmsg + index), 8);
			index += 8;
			break;
        case NYMPH_TYPE_EMPTY_STRING:
			NYMPH_LOG_DEBUG("NYMPH_TYPE_EMPTY_STRING");
			type = NYMPH_STRING;
			length = binaryStringLength(0);
			strLength = 0;
			data.chars = 0;
			own = false;
			emptyString = true;
			break;
        case NYMPH_TYPE_STRING: {
			NYMPH_LOG_DEBUG("NYMPH_TYPE_STRING");
			uint8_t tc = *(binmsg + index);
			index++;
			
			uint64_t l = 0;
			switch (tc) {
				 case NYMPH_TYPE_UINT8: {
					l = *(binmsg + index);
					NYMPH_LOG_DEBUG("String uint8 length: " + NumberFormatter::format(l));
					index++;
				 }
					break;
				case NYMPH_TYPE_UINT16: {
					uint16_t t;
					memcpy(&t, (binmsg + index), 2);
					l = t;
					NYMPH_LOG_DEBUG("String uint16 length: " + NumberFormatter::format(l));
					index += 2;
				}
					break;
				case NYMPH_TYPE_UINT32: {
					uint32_t t;
					memcpy(&t, (binmsg + index), 4);
					l = t;
					NYMPH_LOG_DEBUG("String uint32 length: " + NumberFormatter::format(l));
					index += 4;
				}
					break;
				case NYMPH_TYPE_UINT64: {
					memcpy(&l, (binmsg + index), 8);
					NYMPH_LOG_DEBUG("String uint64 length: " + NumberFormatter::format(l));
					index += 8;
				}
					break;
				default:
					NYMPH_LOG_ERROR("Not a valid integer type for string length.");
					return false;
			}
			
			type = NYMPH_STRING;
			length = binaryStringLength(l);
			strLength = l;
			data.chars = (char*) (binmsg + index);
			own = false;
			if (l == 0) { emptyString = true; }
			index += l;
			
			//NYMPH_LOG_DEBUG("String value: " + value + ".");
            break;
		}
        case NYMPH_TYPE_ARRAY: {
			NYMPH_LOG_DEBUG("NYMPH_TYPE_ARRAY");
			std::string loggerName = "NymphTypes";
			uint64_t numElements = *((uint64_t*) (binmsg + index));
			index += 8;
			
			NYMPH_LOG_DEBUG("Array size: " + NumberFormatter::format(numElements) + " elements.");
			
			// Create a vector and read the individual NymphTypes into it.
			std::vector<NymphType*>* vec = new std::vector<NymphType*>();
			vec->reserve(numElements);
			
			// Parse the elements.
			uint8_t tc = 0;
			for (uint64_t i = 0; i < numElements; ++i) {
				NYMPH_LOG_TRACE("Parsing array index " + NumberFormatter::format(i) + " of " + NumberFormatter::format(numElements) + " elements - Index: " + NumberFormatter::format(index) + ".");
				tc = *(binmsg + index++);
				NymphType* elVal = new NymphType;
				elVal->parseValue(tc, binmsg, index);
				length += elVal->bytes();
				vec->push_back(elVal);
			}
			
			tc = *(binmsg + index++);
			if (tc != NYMPH_TYPE_NONE) {
				NYMPH_LOG_ERROR("Array terminator was not found where expected.");
			}
			
			own = true;
			type = NYMPH_ARRAY;
			data.vector = vec;
	
			length += 10; // Add array type code & element count (uint64), plus terminator (1).
			
            break;
		}
		case NYMPH_TYPE_STRUCT: {
			NYMPH_LOG_DEBUG("NYMPH_TYPE_STRUCT");
			
			std::string loggerName = "NymphTypes";
			
			std::map<std::string, NymphPair>* pairs = new std::map<std::string, NymphPair>();
	
			// Read pairs until NONE type has been found.
			// FIXME: check that we're not running out of bytes to read.
			while (*(binmsg + index) != NYMPH_TYPE_NONE) {
				if (*(binmsg + index) != NYMPH_TYPE_STRING) { return false; }
				uint8_t tc = *(binmsg + index++);
				NymphPair p;
				p.key = new NymphType;
				p.value = new NymphType;
				if (!p.key->parseValue(tc, binmsg, index)) { return false; }
				tc = *(binmsg + index++);
				if (!p.value->parseValue(tc, binmsg, index)) { return false; }
				
				length += p.key->bytes();
				length += p.value->bytes();
				
				pairs->insert(std::pair<std::string, NymphPair>(std::string(p.key->getChar(), p.key->string_length()), p));
			}
			
			// Skip the terminator.
			index++;
			
			//value.setValue(pairs, true);
			own = true;
			type = NYMPH_STRUCT;
			data.pairs = pairs;
	
			// Add typecode & terminator.
			length += 2;
			
			break;
		}
        default:
			NYMPH_LOG_DEBUG("Default case. And nothing happened.");
    }
	
	return true;
}




// --- BYTES ---
// Return the number of bytes in the current value (serialised length).
uint64_t NymphType::bytes() {
	return length;
}


// --- STRING LENGTH ---
// Returns the length of a string (if NYMPH_STRING type or equivalent).
uint32_t NymphType::string_length() {
	return strLength;
}


// --- VALUE TYPE ---
// Return the stored type.
NymphTypes NymphType::valuetype() {
	return type;
}


// --- SERIALIZE ---
void NymphType::serialize(uint8_t* &index) {
	if (type == NYMPH_ANY) {
		// ?
	}
	else if (type == NYMPH_ARRAY) {
		uint8_t typecode = NYMPH_TYPE_ARRAY;
		*index = typecode;
		index++;
		
		uint64_t valueCount = (uint64_t) data.vector->size();
		memcpy(index, &valueCount, 8);
		index += 8;
		
		vector<NymphType*>::iterator it;
		for (it = data.vector->begin(); it != data.vector->end(); ++it) {
			(*it)->serialize(index);
		}
		
		typecode = NYMPH_TYPE_NONE;
		*index = typecode;
		index++;
	}
	else if (type == NYMPH_BOOL) {
		int8_t typecode = NYMPH_TYPE_BOOLEAN_FALSE;
		if (data.boolean) { typecode = NYMPH_TYPE_BOOLEAN_TRUE; }
		
		*index = typecode;
		index++;
	}
	else if (type == NYMPH_UINT8) {
		uint8_t typecode = NYMPH_TYPE_UINT8;
		*index = typecode;
		index++;
	
		*index = data.uint8;
		index++;
	}
	else if (type == NYMPH_SINT8) {
		uint8_t typecode = NYMPH_TYPE_SINT8;
		*index = typecode;
		index++;
	
		*index = data.int8;
		index++;
	}
	else if (type == NYMPH_UINT16) {
		uint8_t typecode = NYMPH_TYPE_UINT16;
		*index = typecode;
		index++;
	
		memcpy(index, &(data.uint16), 2);
		index += 2;
	}
	else if (type == NYMPH_SINT16) {
		uint8_t typecode = NYMPH_TYPE_SINT16;
		*index = typecode;
		index++;
	
		memcpy(index, &(data.int16), 2);
		index += 2;
	}
	else if (type == NYMPH_UINT32) {
		uint8_t typecode = NYMPH_TYPE_UINT32;
		*index = typecode;
		index++;
	
		memcpy(index, &(data.uint32), 4);
		index += 4;
	}
	else if (type == NYMPH_SINT32) {
		uint8_t typecode = NYMPH_TYPE_SINT32;
		*index = typecode;
		index++;
	
		memcpy(index, &(data.int32), 4);
		index += 4;
	}
	else if (type == NYMPH_UINT64) {
		uint8_t typecode = NYMPH_TYPE_UINT64;
		*index = typecode;
		index++;
	
		memcpy(index, &(data.uint64), 8);
		index += 8;
	}
	else if (type == NYMPH_SINT64) {
		uint8_t typecode = NYMPH_TYPE_SINT64;
		*index = typecode;
		index++;
	
		memcpy(index, &(data.int64), 8);
		index += 8;
	}
	else if (type == NYMPH_FLOAT) {
		uint8_t typecode = NYMPH_TYPE_FLOAT;
		*index = typecode;
		index++;
		
		memcpy(index, &(data.fp32), 4);
		index += 4;
	}
	else if (type == NYMPH_DOUBLE) {
		uint8_t typecode = NYMPH_TYPE_DOUBLE;
		*index = typecode;
		index++;
		
		memcpy(index, &(data.fp64), 8);
		index += 8;
	}
	else if (type == NYMPH_STRING) {
		uint8_t typecode;
		if (emptyString) {
			typecode = NYMPH_TYPE_EMPTY_STRING;
			*index = typecode;
			index++;
		}
		else {
			typecode = NYMPH_TYPE_STRING;
			*index = typecode;
			index++;
			
			if (strLength <= 0xFF) {
				typecode = NYMPH_TYPE_UINT8;
				*index = typecode;
				index++;
				
				uint8_t l = strLength;
				*index = l;
				index++;
			}
			else if (strLength <= 0xFFFF) {
				typecode = NYMPH_TYPE_UINT16;
				*index = typecode;
				index++;
				
				uint16_t l = strLength;
				memcpy(index, &(l), 2);
				index += 2;
			}
			else if (strLength <= 0xFFFFFFFF) {
				typecode = NYMPH_TYPE_UINT32;
				*index = typecode;
				index++;
				
				uint32_t l = strLength;
				memcpy(index, &(l), 4);
				index += 4;
			}
			else {
				typecode = NYMPH_TYPE_UINT64;
				*index = typecode;
				index++;
				
				uint64_t l = strLength;
				memcpy(index, &(l), 8);
				index += 8;
			}
		}
		
		memcpy(index, (uint8_t*) data.chars, strLength);
		index += strLength;
	}
	else if (type == NYMPH_STRUCT) {
		uint8_t typecode = NYMPH_TYPE_STRUCT;
		*index = typecode;
		index++;
		
		std::map<std::string, NymphPair>::iterator it;
		for (it = data.pairs->begin(); it != data.pairs->end(); it++) {
			it->second.key->serialize(index);
			it->second.value->serialize(index);
		}
		
		typecode = NYMPH_TYPE_NONE;
		*index = typecode;
		index++;
	}
	else {
		// World ends.
	}
}


// --- LINK WITH MESSAGE ---
// Link this NymphType with a specific NymphMessage instance. This will 
void NymphType::linkWithMessage(NymphMessage* msg) {
	linkedMsg = msg;
	
	triggerAddRC();
}
	
	
// --- TRIGGER ADD RC ---
void NymphType::triggerAddRC() {
	if (!linkedMsg) { return; }
	
	if (type == NYMPH_ARRAY) {
		linkedMsg->addReferenceCount();
	}
	else if (type == NYMPH_STRUCT) {
		linkedMsg->addReferenceCount();
	}
	else if (type == NYMPH_STRING) {
		linkedMsg->addReferenceCount();
	}
}
	
	
// --- DISCARD ---
// Triggers clean-up routines.
void NymphType::discard() {
	if (!linkedMsg) { return; }
	
	if (type == NYMPH_ARRAY) {
		linkedMsg->decrementReferenceCount();
	}
	else if (type == NYMPH_STRUCT) {
		linkedMsg->decrementReferenceCount();
	}
	else if (type == NYMPH_STRING) {
		linkedMsg->decrementReferenceCount();
	}
}
