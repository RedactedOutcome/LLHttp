#pragma once


#include <string>
#include <unordered_map>
#include <memory>
#include <vector>
#include <bitset>

#include <HBuffer/HBuffer.hpp>
#include <HBuffer/HBufferJoin.hpp>

#pragma region Encoding Libraries
#include <brotli/decode.h>
#include <brotli/encode.h>

#include <zlib.h>
#pragma endregion