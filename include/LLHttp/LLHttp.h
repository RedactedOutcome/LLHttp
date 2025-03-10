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
    class LLHttp{
    public:
        /// @brief Initializes the library to be able to quickly parse http data
        static void Init() noexcept;

        /// @brief returns if a character is valid in a url resource path
        static bool IsValidPathCharacter(char c);
    private:
        static std::bitset<128> s_PathAllowedCharacters;
    };
}