#include "target_code.h"
#include <map>
#include <stdlib.h>

int get_answer(int num) {
  return 41 + num;
}

static std::map<int, std::string> blob_cache;

const std::string& get_blob(int size) {
  if (blob_cache.find(size) != end(blob_cache)) {
    return blob_cache[size];
  }

  std::string s;
  s.resize(size);
  for (auto &c : s) {
    c = static_cast<unsigned char>(rand() % 256);
  }

  blob_cache[size] = std::move(s);
  return blob_cache[size];
}

std::string rand_str(std::size_t size) {
  static const char alphanum[] = "0123456789"
                                 "AB CDEFGHIJKLMNOPQRSTUV WXYZ"
                                 "abcdefghij klmnopqrstuvwxyz ";
  std::string s;
  s.resize(size);
  for (std::size_t i = 0; i < size; ++i) {
    s[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
  }
  return s;
}

