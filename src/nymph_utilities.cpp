/*
	nymph_utilities.cpp	- Implements the NymphRPC Utilities class.
	
	Revision 0
	
	Notes:
			- 
			
	History:
	2017/06/24, Maya Posch : Initial version.
	
	(c) Nyanko.ws
*/


#include "nymph_utilities.h"
#include "nymph_logger.h"

#include <cstring>
#include <cstdlib>
#include <iostream>

using namespace std;

#include <Poco/JSON/Parser.h>
#include <Poco/JSON/Array.h>
#include <Poco/JSON/Object.h>
#include <Poco/NumberFormatter.h>

using namespace Poco;


// Static initialisations.
Int64 NymphUtilities::messageId = 1;
Mutex NymphUtilities::idMutex;
string NymphUtilities::loggerName = "NymphUtilities";


// --- GET MESSAGE ID ---
Int64 NymphUtilities::getMessageId() {
	Int64 temp;
	idMutex.lock();
	temp = messageId++;
	idMutex.unlock();
	return temp;
}


// --- PARSE VALUE ---
// Parses the value section of a message's key/value pair. Determines the value
// type and uses the appropriate NymphType to parse the value.
//bool NymphUtilities::parseValue(UInt8 typecode, string* binmsg, int &index, NymphType* &value) {
bool NymphUtilities::parseValue(UInt8 typecode, uint8_t* binmsg, int &index, NymphType &value) {
	//NYMPH_LOG_DEBUG("parseValue called with typecode: " + NumberFormatter::format(typecode) + ".");
	
	switch (typecode) {
        case NYMPH_TYPE_NULL:
			NYMPH_LOG_DEBUG("NYMPH_TYPE_NONE");
			//value = 0;
			break;
        case NYMPH_TYPE_BOOLEAN_FALSE:
			NYMPH_LOG_DEBUG("NYMPH_TYPE_BOOLEAN_FALSE");
			value.setValue(false);
			break;
        case NYMPH_TYPE_BOOLEAN_TRUE:
			NYMPH_LOG_DEBUG("NYMPH_TYPE_BOOLEAN_TRUE");
			value.setValue(true);
			break;
		case NYMPH_TYPE_FLOAT: {
			NYMPH_LOG_DEBUG("NYMPH_TYPE_FLOAT");
			value.setValue(*((float*) (binmsg + index)));
			index += 4;
			break;
		}
		case NYMPH_TYPE_DOUBLE:
			NYMPH_LOG_DEBUG("NYMPH_TYPE_DOUBLE");
			value.setValue(*((double*) (binmsg + index)));
			index += 8;
			break;
        case NYMPH_TYPE_UINT8:
			NYMPH_LOG_DEBUG("NYMPH_TYPE_UINT8");
			value.setValue(*((binmsg + index)));
			index++;
            break;
		case NYMPH_TYPE_SINT8:
			NYMPH_LOG_DEBUG("NYMPH_TYPE_SINT8");
			value.setValue(*((int8_t*) (binmsg + index)));
			index++;
			break;
		case NYMPH_TYPE_UINT16:
			NYMPH_LOG_DEBUG("NYMPH_TYPE_UINT16");
			value.setValue(*((uint16_t*) (binmsg + index)));
			index += 2;
			break;
		case NYMPH_TYPE_SINT16:
			NYMPH_LOG_DEBUG("NYMPH_TYPE_SINT16");
			value.setValue(*((int16_t*) (binmsg + index)));
			index += 2;
			break;
		case NYMPH_TYPE_UINT32:
			NYMPH_LOG_DEBUG("NYMPH_TYPE_UINT32");
			value.setValue(*((uint32_t*) (binmsg + index)));
			index += 4;
			break;
		case NYMPH_TYPE_SINT32:
			NYMPH_LOG_DEBUG("NYMPH_TYPE_SINT32");
			value.setValue(*((int32_t*) (binmsg + index)));
			index += 4;
			break;
		case NYMPH_TYPE_UINT64:
			NYMPH_LOG_DEBUG("NYMPH_TYPE_UINT64");
			value.setValue(*((uint64_t*) (binmsg + index)));
			index += 8;
			break;
		case NYMPH_TYPE_SINT64:
			NYMPH_LOG_DEBUG("NYMPH_TYPE_SINT64");
			value.setValue(*((int64_t*) (binmsg + index)));
			index += 8;
			break;
        case NYMPH_TYPE_EMPTY_STRING:
			NYMPH_LOG_DEBUG("NYMPH_TYPE_EMPTY_STRING");
			value.setValue((char*) 0, 0);
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
					l = *((uint16_t*) (binmsg + index));
					NYMPH_LOG_DEBUG("String uint16 length: " + NumberFormatter::format(l));
					index += 2;
				}
					break;
				case NYMPH_TYPE_UINT32: {
					l = *((uint32_t*) (binmsg + index));
					NYMPH_LOG_DEBUG("String uint32 length: " + NumberFormatter::format(l));
					index += 4;
				}
					break;
				case NYMPH_TYPE_UINT64: {
					l = *((uint64_t*) (binmsg + index));
					NYMPH_LOG_DEBUG("String uint64 length: " + NumberFormatter::format(l));
					index += 8;
				}
					break;
				default:
					NYMPH_LOG_ERROR("Not a valid integer type for string length.");
					return false;
			}
			
			value.setValue((char*) (binmsg + index), l);
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
				NymphUtilities::parseValue(tc, binmsg, index, *elVal);
				vec->push_back(elVal);
			}
			
			tc = *(binmsg + index++);
			if (tc != NYMPH_TYPE_NONE) {
				NYMPH_LOG_ERROR("Array terminator was not found where expected.");
			}
			
			value.setValue(vec, true);
			
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
				if (!NymphUtilities::parseValue(tc, binmsg, index, *(p.key))) { return false; }
				tc = *(binmsg + index++);
				if (!NymphUtilities::parseValue(tc, binmsg, index, *(p.value))) { return false; }
				
				pairs->insert(std::pair<std::string, NymphPair>(std::string(p.key->getChar(), p.key->string_length()), p));
			}
			
			// Skip the terminator.
			index++;
			
			value.setValue(pairs, true);
			
			break;
		}
        default:
			NYMPH_LOG_DEBUG("Default case. And nothing happened.");
    }
	
	return true;
}
