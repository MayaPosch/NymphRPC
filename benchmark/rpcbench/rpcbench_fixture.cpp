#include "hayai/hayai.hpp"
#include "hayai/hayai_main.hpp"

#include "rpc_client.h"

class RPCClientFixture : public ::hayai::Fixture {
public:
    virtual void SetUp() {
        this->rpcclient = new RPCClientClass();
    }

    virtual void TearDown() {
        delete this->rpcclient;
    }

    RPCClientClass* rpcclient;
};

BENCHMARK_F(RPCClientFixture, GetAnswer, 10, 100) {
    rpcclient->get_answer();
}

BENCHMARK_F(RPCClientFixture, GetStruct, 10, 100) {
    rpcclient->get_struct();
}

BENCHMARK_F(RPCClientFixture, GetBlob, 10, 100) {
	// TODO: generate values for the function with multiplier 2
	// in the range of a uint32.
    rpcclient->get_blob(1024);
}

int main(int argc, char* argv[]) {
    // Set up the main runner.
    ::hayai::MainRunner runner;

    // Parse the arguments.
    int result = runner.ParseArgs(argc, argv);
    if (result) {
        return result;
	}

    // Execute based on the selected mode.
    return runner.Run();
}
