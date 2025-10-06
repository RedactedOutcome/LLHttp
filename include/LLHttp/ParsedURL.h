#pragma once

#include "Core.h"

#ifdef LLHTTP_USE_PCH
#include LLHTTP_PCH_DIR
#else
#include <HBuffer/HBuffer.hpp>
#endif

#include "HttpData.h"

namespace LLHttp{
    class ParsedURL{
    public:
        HBuffer m_Host = "";
        HBuffer m_Path = "";
        uint16_t m_Port = 0;
        URLProtocol m_Protocol = URLProtocol::Http;
        
        /// @param url the URL to parse
        /// @return returns 0 if success
        URLParseError ParseURL(const HBuffer& url) noexcept;
    };
}