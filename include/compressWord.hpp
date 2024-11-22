#pragma once
#include <array>
#include <string>
#include "constants.hpp"

using namespace std;

typedef  array<unsigned int, MAX_WORD_LEN/6> WordCompressed;
WordCompressed compressWord(string&);

