#pragma once

#include "Core.h"
#include "pch.h"
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