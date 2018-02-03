#pragma once

#ifndef TARGET_CODE_H_MNJDLMEV
#define TARGET_CODE_H_MNJDLMEV

#include <stdlib.h>
#include <string>
#include <vector>

//#include <rpc/server.h>


int get_answer(int num);
const std::string& get_blob(int size);

std::string rand_str(std::size_t length);

static constexpr std::size_t item_count = 34;


#endif /* end of include guard: TARGET_CODE_H_MNJDLMEV */
