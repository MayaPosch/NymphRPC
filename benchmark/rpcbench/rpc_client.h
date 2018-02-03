#include "rpc/client.h"
#include <iostream>
#include <string>
using std::string;

int main() {
  rpc::client c("localhost", 8080);

  string input, result;
  while (std::getline(std::cin, input)) {
    if (!input.empty()) {
      result = c.call("echo", input).as<string>();
      std::cout << result << std::endl;
    }
  }
}