#include "LLHttp/pch.h"
#include "ParsedURL.h"
#include "LLHttp.h"

namespace LLHttp{
    constexpr std::bitset<128> GetAllowedURLChars(){
        std::bitset<128> set;

        return set;
    }
    constexpr std::bitset<128> s_AllowedURLBits = GetAllowedURLChars();
    URLParseError ParsedURL::ParseURL(const HBuffer& url) noexcept{
        //TODO: introduce url error codes
        m_Protocol = URLProtocol::Unsupported;
        size_t at = 0;

        //Imply ports
        if(url.StartsWith("http://", 7)){
            m_Protocol = URLProtocol::Http;
            m_Port = 80;
            at += 7;
        }
        else if (url.StartsWith("https://", 8) == 0){
            m_Protocol = URLProtocol::Https;
            m_Port = 443;
            at += 8;
        }else{
            //Assuming HTTP Protocol
            m_Protocol = URLProtocol::Http;
            m_Port = 80;
        }

        size_t hostStart = at;
        size_t lastLabelStart = at;
        char c;
        while(true){
            c = url.Get(at++);
            if((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'))continue;
            if(c == '.'){
                lastLabelStart = at;
                continue;
            }
            if(c == '-'){
                if(at - lastLabelStart < 1)return URLParseError::InvalidHostname;
                continue;
            }
            break;
        }
        if(c == '-')return URLParseError::InvalidHostname;
        if(c == 0x00){
            //Reached EOF. Invalid URL URL Requires path hostname/path
            return URLParseError::NeedsMoreData;
        }

        size_t hostLength = at - hostStart;
        m_Host = url.SubString(hostStart, hostLength);

        //Check if they dont want it implied
        c = url.Get(at++);
        if(c == '\0')return URLParseError::NeedsMoreData;

        if(c == ':'){
            size_t portStart = at;
            //if(!std::isdigit(url.Get(at)))return (int)HttpEncodingErrorCode::InvalidPort;
            c = url.Get(at);
            while(std::isdigit(c)){
                c = url.Get(at++);
            }

            size_t portLength = at - portStart;
            //Check for valid port size
            if(portLength > 5 || portLength < 1)return URLParseError::InvalidPort;
            ///TODO: replace with status = url.ToString(portStart, portLength, result);
            HBuffer portString = url.SubString(portStart, portLength);
            m_Port = std::atoi(portString.GetCStr());
        }

        if(c == '/'){
            size_t pathStart = at;
            c = url.Get(at);
            while(c != '\0'){
                if(LLHttp::IsValidPathCharacter(c)){
                    c = url.Get(++at);
                    continue;
                }

                std::cout << "Not valid character" << c << std::endl;
                return URLParseError::InvalidPath;
            }

            //Decrement start since start is assigned to after /
            pathStart--;
            m_Path = url.SubString(pathStart, at - pathStart);
        }

        return URLParseError::None;
    }
}