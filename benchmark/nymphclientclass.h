// NymphClientClass - Simple NymphRPC client implementation for benchmarking.

#pragma once
#ifndef NYMPHCLIENTCLASS_H
#define NYMPHCLIENTCLASS_H

#include "../src/nymph.h"

class NymphClientClass {
	string result;
	int handle;
	
public:
	NymphClientClass();
	~NymphClientClass();
	
	void get_answer();
	void get_struct();
	void get_blob(uint32_t i);
};

#endif
