#pragma once

#include "Core.h"
#include "HttpData.h"
#include "Cookie.h"
#include "Decoder.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "ParsedURL.h"
#include "WebSocketPacket.h"

namespace LLHttp{
    /// @brief Initializes the library to be able to quickly parse http data
    static void InitLLHttp() noexcept;

    /// @brief returns if a character is valid in a url resource path
    extern bool IsValidPathCharacter(char c);

    static std::bitset<128> s_LLHttpPathAllowedCharacters;
}