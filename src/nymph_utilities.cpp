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
