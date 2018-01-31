// Benchmark test for NymphRPC
// 2018, Maya Posch

#include "hayai/hayai.hpp"
#include "hayai/hayai_main.hpp"

#include "nymphclientclass.h"

BENCHMARK(ToString, IntConversion100, 10, 100) {
    IntToStringConversionTest(TEST_NUM_COUNT100);
}

BENCHMARK(ToString, DoubleConversion100, 10, 100) {
    DoubleToStringConversionTest(TEST_NUM_COUNT100);
}

int main() {
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
