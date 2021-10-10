/*
	nymph_session.cpp	- implementation of the NymphRPC Session class.
	
	Revision 0
	
	Notes:
			- This class implements the session class to be used by Nymph servers.
			
	History:
	2017/06/24, Maya Posch : Initial version.
	
	(c) Nyanko.ws
*/


#include "nymph_session.h"
#include "nymph_server.h"
#include "nymph_message.h"
#include "remote_client.h"

#include <Poco/NumberFormatter.h>

using namespace Poco;

using namespace std;


// Static initialisations.
int NymphSession::lastSessionHandle = 0;
Mutex NymphSession::handleMutex;


// --- CONSTRUCTOR ---
NymphSession::NymphSession(const Net::StreamSocket& socket) 
							: Net::TCPServerConnection(socket) {
	loggerName = "NymphSession";
	
	// TODO: send a list of the method signatures to the new client.
	// Get the serialised list from the RemoteClient class and send it.
}


// --- RUN ---
// Reads in and validates incoming Nymph requests, then tries to call the 
// registered callback for any known method.
void NymphSession::run() {
	Net::StreamSocket& socket = this->socket();
	
	// Set a handle for this session instance.
	handleMutex.lock();
	handle = lastSessionHandle++;
	handleMutex.unlock();
	
	// Add this client to the list of sessions.
	NymphRemoteClient::addSession(handle, this);
	
	Timespan timeout(1, 0); // 1 second timeout
	char headerBuff[8];
	while (NymphServer::running) {
		if (socket.poll(timeout, Net::Socket::SELECT_READ)) {
			// Attempt to receive the entire message.
			// First validate the header (0x4452474e), then read the uint32
			// following it. This contains the data length (LE format).
			int received = socket.receiveBytes((void*) &headerBuff, 8);
			if (received == 0) {
				// Remote disconnnected. Socket should be discarded.
				NYMPH_LOG_INFORMATION("Received remote disconnected notice. Terminating listener thread.");
				break;
			}
			else if (received < 8) {
				// TODO: try to wait for more bytes.
				NYMPH_LOG_WARNING("Received <8 bytes: " + Poco::NumberFormatter::format(received));
				
				continue;
			}
			
			UInt32 signature = *((UInt32*) &headerBuff[0]);
			if (signature != 0x4452474e) { // 'DRGN' ASCII in LE format.
				// TODO: handle invalid header.
				NYMPH_LOG_ERROR("Invalid header: 0x" + NumberFormatter::formatHex(signature));
				
				continue;
			}
			
			UInt32 length = 0;
			length = *((UInt32*) &headerBuff[4]);
			
			NYMPH_LOG_DEBUG("Message length: " + NumberFormatter::format(length) + " bytes.");
			
			uint8_t* buff = new uint8_t[length];
			
			// Read the entire message into a string. This is then used to
			// construct an NymphMessage instance.
			received = socket.receiveBytes((void*) buff, length);
			if (received != length) {
				// Handle incomplete message.
				NYMPH_LOG_WARNING("Incomplete message: " + NumberFormatter::format(received) + " of " + NumberFormatter::format(length));
				
				// Loop until the rest of the message has been received.
				// TODO: Set a maximum number of loops/timeout? Reset when 
				// receiving data, timeout when poll times out N times?
				int buffIdx = received;
				int unread = length - received;
				while (1) {
					if (socket.poll(timeout, Net::Socket::SELECT_READ)) {
						received = socket.receiveBytes((void*) (buff + buffIdx), unread);
						if (received == 0) {
							// Remote disconnnected. Socket should be discarded.
							NYMPH_LOG_INFORMATION("Received remote disconnected notice. Terminating listener thread.");
							delete[] buff;
							break;
						}
						else if (received != unread) {
							unread -= received;
							buffIdx += received;
							NYMPH_LOG_WARNING("Incomplete message: " + NumberFormatter::format(unread) + "/" + NumberFormatter::format(length) + " unread.");
							continue;
						}
						
						// Full message was read. Continue with processing.
						break;
					} // if
				} //while
			}
			else { 
				NYMPH_LOG_DEBUG("Read " + NumberFormatter::format(received) + " bytes.");
			}
			
			// Parse the string into an NymphMessage instance.
			// Buffer ownership is transferred to the message.
			NymphMessage* msg = new NymphMessage(buff, length);
			
			// Check for good state on message.
			if (msg->isCorrupt()) {
				// Handle corrupted message.
				NYMPH_LOG_WARNING("Corrupted message. Discarding it.");
				delete msg;
				continue;
			}
			
			if (msg->getState() != 0) {
				// Error during the parsing of the message. Abort.
				NYMPH_LOG_ERROR("Failed to parse the binary message. Skipping...");
				delete msg;
				continue;
			}
			
			// The message ID is now used to find the appropriate callback to call.
			Int64 msgId = msg->getMessageId();
			NYMPH_LOG_DEBUG("Calling method callback for message ID: " + NumberFormatter::format(msgId));
			UInt32 id = msg->getMethodId();
			NymphMessage* response = 0;
			if (!NymphRemoteClient::callMethodCallback(handle, id, msg, response)) {
				NYMPH_LOG_ERROR("Calling callback for message " + NumberFormatter::format(msgId) + " failed. Skipping message.");
				delete msg;
				continue;
			}
			
			if (!response) {
				NYMPH_LOG_ERROR("Calling callback failed: no response returned.");
				delete msg;
				continue;
			}
			
			NYMPH_LOG_INFORMATION("Calling method callback succeeded. Sending response.");
			
			// Prepare the response.
			response->serialize();
			
			// Send the message.
			try {
				int ret = socket.sendBytes(((const void*) response->buffer()), 
														response->buffer_size());
				if (ret != response->buffer_size()) {
					// Handle error.
					NYMPH_LOG_ERROR("Failed to send message.");
					delete response;
					continue;
				}
				
				NYMPH_LOG_DEBUG("Sent " + NumberFormatter::format(ret) + " bytes.");
			}
			catch (Poco::Exception &e) {
				NYMPH_LOG_ERROR("Failed to send message: " + e.message());
				delete response;
				continue;
			}
			
			delete response;
		} // if
	} // while
	
	// Remove this session from the list.
	NymphRemoteClient::removeSession(handle);
}


// --- SEND ---
// Send data on the socket instance.
bool NymphSession::send(uint8_t* msg, uint32_t length, std::string &result) {
	Net::StreamSocket& socket = this->socket();
	
	// Send the message.
	try {
		int ret = socket.sendBytes((const void*) msg, length);
		if (ret != length) {
			// Handle error.
			result = "Failed to send message.";		
			return false;
		}
		
		NYMPH_LOG_DEBUG("Sent " + NumberFormatter::format(ret) + " bytes.");
	}
	catch (Poco::Exception &e) {
		result = "Failed to send message: " + e.message();
		return false;
	}
	
	return true;
}
