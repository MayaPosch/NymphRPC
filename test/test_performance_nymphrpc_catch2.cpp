// test_performance_nymphrpc_catch2.cpp
//
// 14 October 2021. 
//
// Objective: benchmark performance of transfer of relatively large binary blobs.
//
// Using benchmark facility of Catch2 test framework.
// - Project: https://github.com/catchorg/Catch2
// - Single include file: https://github.com/catchorg/Catch2/releases/download/v2.13.7/catch.hpp
//

// Objectives:
// - Benchmark NymphRPC
// - Benchmark Neo-NymphRPC
// - For various Nymph data types

// Results:
// - See benchmark results at end of this file.
// - Observed improvement for blob transfer is 3 to 4 times.
// - Uint32, double and array are unchanged.

// Configuration:

#ifndef  NeoNymphRPC
# define NeoNymphRPC  1
#endif

// End configuration.

#define CATCH_CONFIG_ENABLE_BENCHMARKING
#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "../src/nymph.h"

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

// Adapt to some differences between NymphRPC implementations

#if NeoNymphRPC

uint64_t nymph_bytes(NymphType * v) { return v->bytes(); }

NymphType * new_NymphType(uint64_t v) { return new NymphType(v); }
NymphType * new_NymphType(uint32_t v) { return new NymphType(v); }
NymphType * new_NymphType(double   v) { return new NymphType(v); }

template< typename T >
NymphType * new_NymphType(std::vector<T> * v, bool b = false)
{
	return new NymphType(v, b);
}

#else // NeoNymphRPC

uint32_t nymph_bytes(NymphType * v) { return v->binarySize(); }

NymphType * new_NymphType(uint64_t v) { return new NymphUint64(v); }
NymphType * new_NymphType(uint32_t v) { return new NymphUint32(v); }
NymphType * new_NymphType(double   v) { return new NymphDouble(v); }

template< typename T >
NymphType * new_NymphType(std::vector<T> const * v, bool = false)
{
	auto array = new NymphArray();

	for( auto & elem : *v )
	{
		array->addValue(elem);	// TODO allocate new elem?

	}

	return array;
}

#endif // NeoNymphRPC

// Return an uint32 number.

NymphMessage* uint32Callback(int session, NymphMessage* msg, void* data)
{
	const uint32_t num = 42u;

	NymphMessage* returnMsg = msg->getReplyMessage();
	returnMsg->setResultValue(new_NymphType(num));

#if NeoNymphRPC
	msg->discard();
#endif

	return returnMsg;
}

// Return a double number.

NymphMessage* doubleCallback(int session, NymphMessage* msg, void* data)
{
	const double num = 3.1415;

	NymphMessage* returnMsg = msg->getReplyMessage();
	returnMsg->setResultValue(new_NymphType(num));

#if NeoNymphRPC
	msg->discard();
#endif

	return returnMsg;
}

// Return an array filled with numbers.

NymphMessage* arrayCallbackBase(int session, NymphMessage* msg, void* data, size_t size)
{
	std::vector<NymphType*>* numbers = new std::vector<NymphType*>;

	for (size_t i = 0; i < size; i++) {
		NymphType* num = new_NymphType(i);
		numbers->push_back(num);
	}

	NymphMessage* returnMsg = msg->getReplyMessage();
	returnMsg->setResultValue(new_NymphType(numbers, true));

#if NeoNymphRPC
	msg->discard();
#endif

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
#if NeoNymphRPC
	const uint32_t size = msg->parameters()[0]->getUint32();

	NymphType* result = new NymphType(const_cast<char*>(get_blob(size).data()), size);
	// NymphType* result = new NymphType(const_cast<char*>("Hello, world"), 13);
#else
	const uint32_t size = dynamic_cast<NymphUint32*>(msg->parameters()[0])->getValue();

	NymphString* result = new NymphString(get_blob(size));
#endif

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

	// Receive Arrays.
	{
		std::vector<NymphTypes> parameters;

#if NeoNymphRPC
		NymphRemoteClient::registerMethod("uint32Function"    , NymphMethod("uint32Function"    , parameters, NYMPH_UINT32, uint32Callback   ));
		NymphRemoteClient::registerMethod("doubleFunction"    , NymphMethod("doubleFunction"    , parameters, NYMPH_DOUBLE, doubleCallback   ));

		NymphRemoteClient::registerMethod("arrayFunction1"    , NymphMethod("arrayFunction1"    , parameters, NYMPH_ARRAY, arrayCallback1    ));
		NymphRemoteClient::registerMethod("arrayFunction5"    , NymphMethod("arrayFunction5"    , parameters, NYMPH_ARRAY, arrayCallback5    ));
		NymphRemoteClient::registerMethod("arrayFunction10"   , NymphMethod("arrayFunction10"   , parameters, NYMPH_ARRAY, arrayCallback10   ));
		NymphRemoteClient::registerMethod("arrayFunction100"  , NymphMethod("arrayFunction100"  , parameters, NYMPH_ARRAY, arrayCallback100  ));
		NymphRemoteClient::registerMethod("arrayFunction1000" , NymphMethod("arrayFunction1000" , parameters, NYMPH_ARRAY, arrayCallback1000 ));
		NymphRemoteClient::registerMethod("arrayFunction10000", NymphMethod("arrayFunction10000", parameters, NYMPH_ARRAY, arrayCallback10000));
#else
		NymphMethod uint32Function("uint32Function"        , parameters, NYMPH_UINT32);
		NymphMethod doubleFunction("doubleFunction"        , parameters, NYMPH_UINT32);

		NymphMethod arrayFunction1("arrayFunction1"        , parameters, NYMPH_ARRAY);
		NymphMethod arrayFunction5("arrayFunction5"        , parameters, NYMPH_ARRAY);
		NymphMethod arrayFunction10("arrayFunction10"      , parameters, NYMPH_ARRAY);
		NymphMethod arrayFunction100("arrayFunction100"    , parameters, NYMPH_ARRAY);
		NymphMethod arrayFunction1000("arrayFunction1000"  , parameters, NYMPH_ARRAY);
		NymphMethod arrayFunction10000("arrayFunction10000", parameters, NYMPH_ARRAY);

		uint32Function.setCallback(uint32Callback);
		doubleFunction.setCallback(doubleCallback);

		arrayFunction1.setCallback(arrayCallback1);
		arrayFunction5.setCallback(arrayCallback5);
		arrayFunction10.setCallback(arrayCallback10);
		arrayFunction100.setCallback(arrayCallback100);
		arrayFunction1000.setCallback(arrayCallback1000);
		arrayFunction10000.setCallback(arrayCallback10000);

		NymphRemoteClient::registerMethod("uint32Function"    , uint32Function );
		NymphRemoteClient::registerMethod("doubleFunction"    , doubleFunction );

		NymphRemoteClient::registerMethod("arrayFunction1"    , arrayFunction1 );
		NymphRemoteClient::registerMethod("arrayFunction5"    , arrayFunction5 );
		NymphRemoteClient::registerMethod("arrayFunction10"   , arrayFunction10 );
		NymphRemoteClient::registerMethod("arrayFunction100"  , arrayFunction100 );
		NymphRemoteClient::registerMethod("arrayFunction1000" , arrayFunction1000 );
		NymphRemoteClient::registerMethod("arrayFunction10000", arrayFunction10000 );
#endif
	}

	// Receive data chunks.
	{
		// uint8 receiveDataMaster(blob data, bool done, sint64)
		std::vector<NymphTypes> parameters( {NYMPH_UINT32} );
#if NeoNymphRPC
		NymphRemoteClient::registerMethod("blobFunction", NymphMethod("blobFunction", parameters, NYMPH_STRING, blobCallback));
#else
		NymphMethod getBlobFunction("blobFunction", parameters, NYMPH_STRING);
		getBlobFunction.setCallback(blobCallback);
		NymphRemoteClient::registerMethod("blobFunction", getBlobFunction);
#endif
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

	const auto bytes = nymph_bytes(returnValue);

	delete returnValue;

	return bytes;
}

// Method call to benchmark blobs:

size_t call(uint32_t handle, char const * fun, uint32_t size)
{
	// Request blob with integers.

	NymphType* returnValue = 0;
	std::string result;

	std::vector<NymphType*> values({ new_NymphType(size) });

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

	const auto bytes = nymph_bytes(returnValue);

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

	BENCHMARK("uint32"      ) { return call(handle, "uint32Function" ); };
	BENCHMARK("double"      ) { return call(handle, "doubleFunction" ); };

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

//================================================================================
// Results:


//********************************************************************************
// Neo NymphRPC: Example output (VS2019, -O2):

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
// uint32                                          20             1    122.193 us
// double                                          20             1    140.368 us
// array     1:                                    20             1    173.963 us
// array     5:                                    20             1    189.888 us
// array    10:                                    20             1    220.653 us
// array   100:                                    20             1    573.168 us
// array  1000:                                    20             1    3.33472 ms
// array 10000:                                    20             1    31.8041 ms
// blob       1:                                   20             1    181.433 us
// blob      10:                                   20             1    194.048 us
// blob     100:                                   20             1    153.998 us
// blob    1000:                                   20             1    174.073 us
// blob   10000:                                   20             1    166.228 us
// blob  100000:                                   20             1    240.223 us
// blob  200000:                                   20             1    343.233 us
// blob  500000:                                   20             1    716.233 us
// blob 1000000:                                   20             1     2.0748 ms Stopped workers.
//
// ===============================================================================
// test cases: 1 | 1 passed
// assertions: - none -


//********************************************************************************
// NymphRPC Antiqua: Example output (VS2019, -O2):

// *** Starting server...
// *** Initialising server...
// Dispatcher: Setting max pool size to 10 workers.
// *** Registering methods...
// *** Connecting to server...
// Dispatcher: Setting max pool size to 10 workers.

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// test_performance_nymphrpc_catch2.exe is a Catch v2.13.7 host application.
// Run with -? for options

// -------------------------------------------------------------------------------
// NymphRPC
// -------------------------------------------------------------------------------
// ..\test_performance_nymphrpc_catch2.cpp(327)
// ...............................................................................

// benchmark name                            samples    iterations          mean
// -------------------------------------------------------------------------------
// uint32                                          20             1    178.387 us
// double                                          20             1    138.282 us
// array     1:                                    20             1    197.452 us
// array     5:                                    20             1    198.407 us
// array    10:                                    20             1    204.417 us
// array   100:                                    20             1    512.027 us
// array  1000:                                    20             1    3.08481 ms
// array 10000:                                    20             1    32.8876 ms
// blob       1:                                   20             1    188.677 us
// blob      10:                                   20             1    141.712 us
// blob     100:                                   20             1    174.832 us
// blob    1000:                                   20             1    133.617 us
// blob   10000:                                   20             1    211.097 us
// blob  100000:                                   20             1    362.747 us
// blob  200000:                                   20             1    1.35672 ms
// blob  500000:                                   20             1    3.37874 ms
// blob 1000000:                                   20             1    8.19277 ms Stopped workers.
//
// ===============================================================================
// test cases: 1 | 1 passed
// assertions: - none -
