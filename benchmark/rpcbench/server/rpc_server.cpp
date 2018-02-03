#include "rpc/server.h"
#include <string>
#include <vector>

using std::string;
using std::vector;

#include "target_code.h"

constexpr std::size_t min_size = 1 << 10;
constexpr std::size_t max_size = 16 << 10 << 10;
constexpr std::size_t multiplier = 2;


int main() {	
	// Init blob cache.
	for (std::size_t s = min_size; s <= max_size; s *= multiplier) {
		get_blob(s);
	}
	
	rpc::server srv(8080);

	srv.bind("get_answer", []() {
		return 42;
	});


	srv.bind("get_struct", []() {
		vector<string> tmp;
		return tmp;
	});


	srv.bind("get_blob", [](int const& s) {
		return get_blob(s);
	});

	srv.run();
	return 0;
}