#include "hayai/hayai.hpp"
#include "hayai/hayai_main.hpp"

#include "rpc_client.h"

class RPCClientFixture : public ::hayai::Fixture {
public:
    virtual void SetUp() {
        this->rpcclient = new RPCClient();
    }

    virtual void TearDown() {
        delete this->rpcclient;
    }

    RPCClient* rpcclient;
};

BENCHMARK_F(RPCClientFixture, GetAnswer, 10, 100) { rpcclient->get_answer(); }
BENCHMARK_F(RPCClientFixture, GetStruct, 10, 100) { rpcclient->get_struct(); }
BENCHMARK_F(RPCClientFixture, GetBlob_1024, 10, 100) { rpcclient->get_blob(1024); }
BENCHMARK_F(RPCClientFixture, GetBlob_2048, 10, 100) { rpcclient->get_blob(2048); }
BENCHMARK_F(RPCClientFixture, GetBlob_4096, 10, 100) { rpcclient->get_blob(4096); }
BENCHMARK_F(RPCClientFixture, GetBlob_8192, 10, 100) { rpcclient->get_blob(8192); }
BENCHMARK_F(RPCClientFixture, GetBlob_16k, 10, 100) { rpcclient->get_blob(16384); }
BENCHMARK_F(RPCClientFixture, GetBlob_32k, 10, 100) { rpcclient->get_blob(32768); }
BENCHMARK_F(RPCClientFixture, GetBlob_65k, 10, 100) { rpcclient->get_blob(65536); }
BENCHMARK_F(RPCClientFixture, GetBlob_131k, 10, 100) { rpcclient->get_blob(131072); }
BENCHMARK_F(RPCClientFixture, GetBlob_262k, 10, 100) { rpcclient->get_blob(262144); }
BENCHMARK_F(RPCClientFixture, GetBlob_524k, 10, 100) { rpcclient->get_blob(524288); }
BENCHMARK_F(RPCClientFixture, GetBlob_1M, 10, 100) { rpcclient->get_blob(1048576); }
BENCHMARK_F(RPCClientFixture, GetBlob_2M, 10, 100) { rpcclient->get_blob(2097152); }
BENCHMARK_F(RPCClientFixture, GetBlob_4M, 10, 100) { rpcclient->get_blob(4194304); }
BENCHMARK_F(RPCClientFixture, GetBlob_8M, 10, 100) { rpcclient->get_blob(8388608); }
BENCHMARK_F(RPCClientFixture, GetBlob_16M, 10, 100) { rpcclient->get_blob(16777216); }

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
