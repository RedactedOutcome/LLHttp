#pragma once

#if __cplusplus < 201703L
#error "LLHttp Requires a c++ version of atleast c++17"
#endif

#ifdef LLHTTP_USING_PCH
#include LLHTTP_PCH_DIR
#else

#include <iostream>
#include <vector>
#include <memory>

#include <HBuffer/HBufferJoin.hpp>

#endif