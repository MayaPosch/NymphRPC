/*
	nymph_types.h	- Defines the NymphRPC data types.
	
	Revision 1
	
	Notes:
			- 
			
	History:
	2017/06/24, Maya Posch : Initial version.
	2021/10/01, Maya Posch : New type system.
	
	(c) Nyanko.ws
*/


#pragma once
#ifndef NYMPH_TYPES_H
#define NYMPH_TYPES_H

#include <Poco/Poco.h>

#include <string>
#include <map>
#include <vector>
#include <cstdint>


class NymphMessage;


enum NymphInternalTypes {
	NYMPH_TYPE_NULL          	= 0x00,
    NYMPH_TYPE_NONE          	= 0x01,
    NYMPH_TYPE_BOOLEAN_FALSE	= 0x02,
    NYMPH_TYPE_BOOLEAN_TRUE  	= 0x03,
    NYMPH_TYPE_UINT8         	= 0x04,
	NYMPH_TYPE_SINT8			= 0x05,
    NYMPH_TYPE_UINT16        	= 0x06,
	NYMPH_TYPE_SINT16			= 0x07,
    NYMPH_TYPE_UINT32       	= 0x08,
	NYMPH_TYPE_SINT32			= 0x09,
    NYMPH_TYPE_UINT64        	= 0x0a,
	NYMPH_TYPE_SINT64			= 0x0b,
    NYMPH_TYPE_FLOAT         	= 0x0c,
    NYMPH_TYPE_DOUBLE        	= 0x0d,
    NYMPH_TYPE_ARRAY         	= 0x0e,
    NYMPH_TYPE_EMPTY_STRING  	= 0x0f,
    NYMPH_TYPE_STRING        	= 0x10,
    NYMPH_TYPE_STRUCT        	= 0x11,
    NYMPH_TYPE_VOID           	= 0x12
};


// External types.
// I.e. those seen by an application using the library.
enum NymphTypes {
	NYMPH_NULL	= 0,
	NYMPH_ARRAY,
	NYMPH_BOOL,
	NYMPH_UINT8,
	NYMPH_SINT8,
	NYMPH_UINT16,
	NYMPH_SINT16,
	NYMPH_UINT32,
	NYMPH_SINT32,
	NYMPH_UINT64,
	NYMPH_SINT64,
	NYMPH_FLOAT,
	NYMPH_DOUBLE,
	NYMPH_STRING,
	NYMPH_STRUCT,
	NYMPH_ANY
};


struct NymphPair;


class NymphType {
	NymphTypes type = NYMPH_NULL;
	union DataUnion {
		void* any;
		bool boolean;
		uint8_t uint8;
		int8_t int8;
		uint16_t uint16;
		int16_t int16;
		uint32_t uint32;
		int32_t int32;
		uint64_t uint64;
		int64_t int64;
		float fp32;
		double fp64;
		const char* chars;
		std::vector<NymphType*>* vector;
		std::map<std::string, NymphPair>* pairs;
	};
	
	DataUnion data;
	uint64_t length;			// Length of serialised value in bytes.
	uint32_t strLength;			// String length (for NYMPH_STRING).
	bool emptyString = false;	// Indicates whether a NYMPH_STRING is empty.
	bool own = false;
	std::string* string = 0;
	NymphMessage* linkedMsg = 0;
	
public:
	NymphType() { }
	NymphType(bool v);
	NymphType(uint8_t v);
	NymphType(int8_t v);
	NymphType(uint16_t v);
	NymphType(int16_t v);
	NymphType(uint32_t v);
	NymphType(int32_t v);
	NymphType(uint64_t v);
	NymphType(int64_t v);
	NymphType(float v);
	NymphType(double v);
	NymphType(char* v, uint32_t bytes, bool own = false);
	NymphType(std::string* v, bool own = false);
	NymphType(std::vector<NymphType*>* v, bool own = false);
	NymphType(std::map<std::string, NymphPair>* v, bool own = false);
	
	~NymphType();
	
	bool getBool(bool* v = 0);
	uint8_t getUint8(uint8_t* v = 0);
	int8_t getInt8(int8_t* v = 0);
	uint16_t getUint16(uint16_t* v = 0);
	int16_t getInt16(int16_t* v = 0);
	uint32_t getUint32(uint32_t* v = 0);
	int32_t getInt32(int32_t* v = 0);
	uint64_t getUint64(uint64_t* v = 0);
	int64_t getInt64(int64_t* v = 0);
	float getFloat(float* v = 0);
	double getDouble(double* v = 0);
	const char* getChar(const char* v = 0);
	std::vector<NymphType*>* getArray(std::vector<NymphType*>* v = 0);
	std::map<std::string, NymphPair>* getStruct(std::map<std::string, NymphPair>* v = 0);
	
	std::string getString();
	bool getStructValue(std::string key, NymphType* &value);
	
	void setValue(bool v);
	void setValue(uint8_t v);
	void setValue(int8_t v);
	void setValue(uint16_t v);
	void setValue(int16_t v);
	void setValue(uint32_t v);
	void setValue(int32_t v);
	void setValue(uint64_t v);
	void setValue(int64_t v);
	void setValue(float v);
	void setValue(double v);
	void setValue(char* v, uint32_t bytes, bool own = false);
	void setValue(std::string* v, bool own = false);
	void setValue(std::vector<NymphType*>* v, bool own = false);
	void setValue(std::map<std::string, NymphPair>* v, bool own = false);
	
	uint64_t bytes();
	uint32_t string_length();
	NymphTypes valuetype();
	
	void serialize(uint8_t* &index);
	
	void linkWithMessage(NymphMessage* msg);
	void triggerAddRC();
	void discard();
};


struct NymphPair {
	NymphType* key;
	NymphType* value;
};

#endif
