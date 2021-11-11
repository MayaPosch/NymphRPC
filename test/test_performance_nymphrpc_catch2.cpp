// test_performance_nymphrpc_catch2.cpp
//
// Objective: benchmark performance of transfer of relatively largebinary blobs.
//
// Using benchmark facility of Catch2 test framework.
// - Project: https://github.com/catchorg/Catch2
// - Single include file: https://github.com/catchorg/Catch2/releases/download/v2.13.7/catch.hpp
//

// Objectives:
// - Benchmark NymphRPC
// - Benchmark Neo-NymphRPC
// - For various Nymph data types

// NymphCastServer: NymphMessage* receiveDataMaster(int session, NymphMessage* msg, void* data) {
// NymphCastClient: 

#define CATCH_CONFIG_ENABLE_BENCHMARKING
#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "../../src/nymph.h"

#include <Poco/Condition.h>
#include <Poco/Thread.h>

#include <csignal>
#include <thread>

Poco::Condition gCon;
Poco::Mutex gMutex;

// Return a blob of given size.

std::string const & get_blob(int size)
{
	static std::map<int, std::string> blob_cache;

	// provide cached blob if present:

	if (blob_cache.find(size) != end(blob_cache))
	{
		return blob_cache[size];
	}

	// create non-existing blob and store it in the cache:
	
	std::string blob;
	blob.resize(size);

	for (auto & c : blob)
	{
		c = static_cast<unsigned char>(rand() % 256);
	}

	blob_cache[size] = std::move(blob);

	assert(blob_cache[size].size() == size);

	return blob_cache[size];
}

// Return an array filled with numbers.

NymphMessage* arrayCallbackBase(int session, NymphMessage* msg, void* data, size_t size)
{
	std::vector<NymphType*>* numbers = new std::vector<NymphType*>;
	
	for (size_t i = 0; i < size; i++) {
		NymphType* num = new NymphType(i);
		numbers->push_back(num);
	}
	
	NymphMessage* returnMsg = msg->getReplyMessage();
	returnMsg->setResultValue(new NymphType(numbers, true));
	msg->discard();

	return returnMsg;
}

NymphMessage* arrayCallback1(int session, NymphMessage* msg, void* data) { return arrayCallbackBase(session, msg, data, 1); }
NymphMessage* arrayCallback5(int session, NymphMessage* msg, void* data) { return arrayCallbackBase(session, msg, data, 5); }
NymphMessage* arrayCallback10(int session, NymphMessage* msg, void* data) { return arrayCallbackBase(session, msg, data, 10); }
NymphMessage* arrayCallback100(int session, NymphMessage* msg, void* data) { return arrayCallbackBase(session, msg, data, 100); }
NymphMessage* arrayCallback1000(int session, NymphMessage* msg, void* data) { return arrayCallbackBase(session, msg, data, 1000); }
NymphMessage* arrayCallback10000(int session, NymphMessage* msg, void* data) { return arrayCallbackBase(session, msg, data, 10000); }

NymphMessage* blobCallback(int session, NymphMessage* msg, void* data) 
{
	const uint32_t size = msg->parameters()[0]->getUint32();
	
	NymphType* result = new NymphType(const_cast<char*>(get_blob(size).data()), size);
	// NymphType* result = new NymphType(const_cast<char*>("Hello, world"), 13);

	NymphMessage* returnMsg = msg->getReplyMessage();
	returnMsg->setResultValue(result);

	return returnMsg;
}

// NymphRPC server.

void signal_handler(int signal)
{
	gCon.signal();
}

void logFunction(int level, std::string text)
{
	// std::cout << level << " - " << text << std::endl;
}

void setup_server()
{
	std::cout << "*** Initialising server..." << std::endl;

	long timeout = 5000; // 5 seconds.
	NymphRemoteClient::init(logFunction, NYMPH_LOG_LEVEL_TRACE, timeout);
	
	// Register methods to expose to the clients.

	std::cout << "*** Registering methods...\n";

	{
		std::vector<NymphTypes> parameters;

		NymphMethod arrayFunction1    ("arrayFunction1"    , parameters, NYMPH_ARRAY, arrayCallback1    );
		NymphMethod arrayFunction5    ("arrayFunction5"    , parameters, NYMPH_ARRAY, arrayCallback5    );
		NymphMethod arrayFunction10   ("arrayFunction10"   , parameters, NYMPH_ARRAY, arrayCallback10   );
		NymphMethod arrayFunction100  ("arrayFunction100"  , parameters, NYMPH_ARRAY, arrayCallback100  );
		NymphMethod arrayFunction1000 ("arrayFunction1000" , parameters, NYMPH_ARRAY, arrayCallback1000 );
		NymphMethod arrayFunction10000("arrayFunction10000", parameters, NYMPH_ARRAY, arrayCallback10000);
		
		NymphRemoteClient::registerMethod("arrayFunction1"    , arrayFunction1    );
		NymphRemoteClient::registerMethod("arrayFunction5"    , arrayFunction5    );
		NymphRemoteClient::registerMethod("arrayFunction10"   , arrayFunction10   );
		NymphRemoteClient::registerMethod("arrayFunction100"  , arrayFunction100  );
		NymphRemoteClient::registerMethod("arrayFunction1000" , arrayFunction1000 );
		NymphRemoteClient::registerMethod("arrayFunction10000", arrayFunction10000);
	}

	// Receive data chunks.
	{
		// uint8 receiveDataMaster(blob data, bool done, sint64)
		std::vector<NymphTypes> parameters( {NYMPH_UINT32} );

		NymphMethod blobFunction("blobFunction", parameters, NYMPH_STRING, blobCallback);
		NymphRemoteClient::registerMethod("blobFunction", blobFunction);
	}

	// Install signal handler to terminate the server.
	signal(SIGINT, signal_handler);
	
	// Start server on port 4004.
	NymphRemoteClient::start(4004);
	
	// Loop until the SIGINT signal has been received.
	gMutex.lock();
	gCon.wait(gMutex);
	
	// Clean-up
	NymphRemoteClient::shutdown();
	
	// Wait before exiting, giving threads time to exit.
	Poco::Thread::sleep(2000); // 2 seconds.
}

// Method call to benchmark arrays:

size_t call(uint32_t handle, char const * fun)
{
	// Request array with integers.

	NymphType* returnValue = 0;
	std::string result;
	std::vector<NymphType*> values;

	if (!NymphRemoteServer::callMethod(handle, fun, values, returnValue, result))
	{
		std::cout << "*** Error calling remote method: '" << fun << "': "<< result << std::endl;
		NymphRemoteServer::disconnect(handle, result);
		NymphRemoteServer::shutdown();
		exit(1);
	}
	
	// Print out values in blob or vector.
#if 0
	std::vector<NymphType*>* numbers = returnValue->getArray();
	std::cout << "*** Got numbers: ";
	for (int i = 0; i < numbers->size(); i++)
	{
		std::cout << i << ":" << (uint16_t) (*numbers)[i]->getUint8() << " ";
	}
	
	std::cout << "." << std::endl;
#endif

	const auto bytes = returnValue->bytes();

	delete returnValue;

	return bytes;
}

// Method call to benchmark blobs:

size_t call(uint32_t handle, char const * fun, uint32_t size)
{
	// Request blob with integers.

	NymphType* returnValue = 0;
	std::string result;
	std::vector<NymphType*> values({ new NymphType(size) });

	if (!NymphRemoteServer::callMethod(handle, fun, values, returnValue, result)) {
		std::cout << "*** Error calling remote method: '" << fun << "': "<< result << std::endl;
		NymphRemoteServer::disconnect(handle, result);
		NymphRemoteServer::shutdown();
		exit(1);
	}
	
	// if (!returnValue) { return; }
	
	// if (returnValue->type() != NYMPH_STRING) {
	// 	cout << "Return value wasn't a string. Type: " << returnValue->type() << endl;
	// 	NymphRemoteServer::disconnect(handle, result);
	// 	NymphRemoteServer::shutdown();
	// 	return;
	// }

	const auto bytes = returnValue->bytes();

	delete returnValue;

	return bytes;
}

void init_blob_cache()
{
	for (auto size : {1, 10, 100, 1000, 10000, 100000, 200000, 500000, 1000000} )
	{
		get_blob(size);
	}
}

std::thread start_server()
{
	std::cout << "*** Starting server...\n";

	init_blob_cache();

	std::thread server(setup_server);

	// Allow server to start, wait 200 ms:
	Poco::Thread::sleep(200);

	return server;
}

uint32_t connect()
{
	std::cout << "*** Connecting to server...\n";

	long timeout = 5000; // 5 seconds.
	NymphRemoteServer::init(logFunction, NYMPH_LOG_LEVEL_TRACE, timeout);
	
	// Connect to the remote server.
	uint32_t handle;
	std::string result;
	if (!NymphRemoteServer::connect("localhost", 4004, handle, 0, result))
	{
		std::cout << "*** Connecting to remote server failed: " << result << std::endl;
		NymphRemoteServer::disconnect(handle, result);
		NymphRemoteServer::shutdown();
		exit(1);
	}

	return handle;
}

TEST_CASE("NymphRPC")
{
	// Steps:
	// - initialize and start server.
	// - connect to server.
	// - for different transfer sizes:
	//   - benchmark the actual transfer
	// - stop the server

	auto server = start_server();

	auto handle = connect();

	BENCHMARK("array     1:") { return call(handle, "arrayFunction1"    ); };
	BENCHMARK("array     5:") { return call(handle, "arrayFunction5"    ); };
	BENCHMARK("array    10:") { return call(handle, "arrayFunction10"   ); };
	BENCHMARK("array   100:") { return call(handle, "arrayFunction100"  ); };
	BENCHMARK("array  1000:") { return call(handle, "arrayFunction1000" ); };
	BENCHMARK("array 10000:") { return call(handle, "arrayFunction10000"); };

	BENCHMARK("blob       1:") { return call(handle, "blobFunction", 1); };
	BENCHMARK("blob      10:") { return call(handle, "blobFunction", 10); };
	BENCHMARK("blob     100:") { return call(handle, "blobFunction", 100); };
	BENCHMARK("blob    1000:") { return call(handle, "blobFunction", 1000); };
	BENCHMARK("blob   10000:") { return call(handle, "blobFunction", 10000); };
	BENCHMARK("blob  100000:") { return call(handle, "blobFunction", 100000); };
	BENCHMARK("blob  200000:") { return call(handle, "blobFunction", 200000); };
	BENCHMARK("blob  500000:") { return call(handle, "blobFunction", 500000); };
	BENCHMARK("blob 1000000:") { return call(handle, "blobFunction", 1000000); };

	// Stop the server:

	gCon.signal();
	server.join();
}

// TODO Create Makefile

// cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
// cmake --build build --config Release


// g++ -std=c++17 -Wall -Wextra -O2 -o bin/test_performance_nymphrpc_catch2 -I../.  test_performance_nymphrpc_catch2.cpp -pthread -lPocoNet && bin/test_performance_nymphrpc_catch2 --benchmark-no-analysis
// g++ -std=c++17 -Wall -Wextra -g3 -Og -o bin/test_performance_nymphrpc_catch2 -I../. test_performance_nymphrpc_catch2.cpp -pthread && bin/test_performance_nymphrpc_catch2 --benchmark-no-analysis
// g++ -std=c++17 -Wall -Wextra -g3 -O0 -o bin/test_performance_nymphrpc_catch2 -I../. test_performance_nymphrpc_catch2.cpp -pthread && bin/test_performance_nymphrpc_catch2 --benchmark-no-analysis

// Note: Poco in D:/Libraries/Poco is 64-bit, use x64Native Tools command prompt for VC2019
// cl -nologo -std:c++latest -EHsc -MD -W4 -O2 -Fetest_performance_nymphrpc_catch2.exe -I../. -ID:/Libraries/Poco/include test_performance_nymphrpc_catch2.cpp ../../src/callback_request.cpp ../../src/dispatcher.cpp ../../src/nymph_listener.cpp ../../src/nymph_logger.cpp ../../src/nymph_message.cpp ../../src/nymph_method.cpp ../../src/nymph_server.cpp ../../src/nymph_session.cpp ../../src/nymph_socket_listener.cpp ../../src/nymph_types.cpp ../../src/nymph_utilities.cpp ../../src/remote_client.cpp ../../src/remote_server.cpp ../../src/worker.cpp -link -libpath:D:/Libraries/Poco/lib  & .\test_performance_nymphrpc_catch2.exe --benchmark-no-analysis --benchmark-samples 20

// Example output (VS2019, -O2):

// *** Starting server...
// *** Initialising server...
// Dispatcher: Setting max pool size to 10 workers.
// *** Registering methods...
// *** Connecting to server...
// Dispatcher: Setting max pool size to 10 workers.
//
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// test_performance_nymphrpc_catch2.exe is a Catch v2.13.7 host application.
// Run with -? for options
//
// -------------------------------------------------------------------------------
// NymphRPC
// -------------------------------------------------------------------------------
// test_performance_nymphrpc_catch2.cpp(181)
// ...............................................................................
//
// benchmark name                            samples    iterations          mean
// -------------------------------------------------------------------------------
// array     1:                                    20             1    181.307 us
// array     5:                                    20             1    229.297 us
// array    10:                                    20             1    249.117 us
// array   100:                                    20             1     1.1406 ms
// array  1000:                                    20             1    4.49935 ms
// array 10000:                                    20             1    34.0406 ms
// blob       1:                                   20             1    145.792 us
// blob      10:                                   20             1    177.897 us
// blob     100:                                   20             1    185.917 us
// blob    1000:                                   20             1    184.402 us
// blob   10000:                                   20             1    207.152 us
// blob  100000:                                   20             1    274.147 us
// blob  200000:                                   20             1    653.972 us
// blob  500000:                                   20             1    672.072 us
// blob 1000000:                                   20             1     2.7826 ms Stopped workers.
//
//
// ===============================================================================
// test cases: 1 | 1 passed
// assertions: - none -
