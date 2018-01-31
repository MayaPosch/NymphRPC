#include "hayai/hayai.hpp"
#include "hayai/hayai_main.hpp"

#include "nymphclientclass.h"

class NymphClientFixture : public ::hayai::Fixture {
public:
    virtual void SetUp() {
        this->nymphclient = new NymphClientClass();
    }

    virtual void TearDown() {
        delete this->nymphclient;
    }

    NymphClientClass* nymphclient;
};

BENCHMARK_F(NymphClientFixture, GetAnswer, 10, 100) {
    nymphclient->get_answer();
}

BENCHMARK_F(NymphClientFixture, GetStruct, 10, 100) {
    nymphclient->get_struct();
}

BENCHMARK_F(NymphClientFixture, GetBlob, 10, 100) {
	// TODO: generate values for the function with multiplier 2
	// in the range of a uint32.
    nymphclient->get_blob(1024);
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
