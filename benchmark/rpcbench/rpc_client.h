

#pragma once
#ifndef RPCCLIENT_H
#define RPCCLIENT_H

#include "rpc/client.h"
#include <iostream>
#include <string>
using std::string;


class RPCClient {
	rpc::client* server;
	
public:
	RPCClient() {
		server = new rpc::client("localhost", 8080);
	}
	
	~RPCClient() { delete server; }
	
	void get_answer() {
		server->call("get_answer");
	}
	
	void get_struct() {
		server->call("get_struct");
	}
	
	void get_blob(uint32_t i) {
		int string = server->call("get_blob", i).as<int>();
	}
};

#endif


// int main() {
//   rpc::client c("localhost", 8080);
// 
//   string input, result;
//   while (std::getline(std::cin, input)) {
//     if (!input.empty()) {
//       result = c.call("echo", input).as<string>();
//       std::cout << result << std::endl;
//     }
//   }
// }
