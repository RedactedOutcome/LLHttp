#pragma once

#ifdef LLHTTP_USE_PCH
#include LLHTTP_PCH_DIR
#else
#include <HBuffer/HBuffer.hpp>
#include <unordered_map>
#include <cstdint>
#include <vector>
#endif

#include "Cookie.h"
#include "HttpData.h"

#ifndef HTTP_DEFAULT_HEAD_REQUEST_TO_BUFFER_SIZE
#define HTTP_DEFAULT_HEAD_REQUEST_TO_BUFFER_SIZE 32000
#endif

namespace LLHttp{
    class HttpRequest{
    public:
        HttpRequest();
        ~HttpRequest();

        /// @brief parses the http response and makes a copy of the body
        /// @return returns 0 if successful
        int ParseCopy(HBuffer data);

        void SetPath(const HBuffer& path) noexcept;
        void SetPath(HBuffer&& path) noexcept;

        /// @brief sets the body to a copy of the string
        /// @param data the string to copy
        void SetBodyAsCopy(const char* data) noexcept;
        void SetBodyAsCopy(char* data, size_t size) noexcept;

        void SetBody(HBuffer&& buffer)noexcept;
        void SetBody(char* data, size_t size, bool canFree, bool canModify) noexcept;

        /// @brief Assigns the body to a Non owning/modifying HBuffer with data
        void SetBodyReference(const char* data)noexcept;
        /// @brief Assigns the body to a Non owning/modifying HBuffer with data and size
        void SetBodyReference(char* data, size_t size)noexcept;
        /// @brief Assigns the body to a Non owning HBuffer with param buffers data
        void SetBodyReference(const HBuffer& buffer)noexcept;

        //void SetHeader(const char* header, const std::string& value);
        //void SetHeader(const char* header, std::string&& value);
        void SetHeader(const char* name, const char* value) noexcept;
        void SetHeader(const HBuffer& name, const char* value) noexcept;
        void SetHeader(const HBuffer& name, const HBuffer& value) noexcept;
        void SetHeader(const HBuffer& name, HBuffer&& value) noexcept;
        void SetHeader(HBuffer&& name, const char* value) noexcept;
        void SetHeader(HBuffer&& name, const HBuffer& value) noexcept;
        void SetHeader(HBuffer&& name, HBuffer&& value) noexcept;
        void RemoveHeader(const char* header) noexcept;
        void RemoveHeader(const HBuffer& header) noexcept;
        
        void SetCookie(const char* name, Cookie& cookie);
        void SetCookie(const char* name, std::shared_ptr<Cookie> cookie);

        void SetVerb(HttpVerb verb)noexcept;

        //Clears all data
        void Clear();
        void PrepareRead();
        void PreparePayload();

        /// @brief Attempts to decompress the body data depending on the Content-Encoding header. 
        /// @return returns enum of type HttpEncodingErrorCode
        int Decompress() noexcept;

        /// @brief Attempts to compress data in the bodies depending on the Content-Encoding header. 
        /// @return returns enum of type HttpEncodingErrorCode
        int Compress() noexcept;
    public:
        HBuffer HeadToBuffer() const noexcept;

        //TODO: have multiple methods to get body parts. Maybe one to try using references because we know we are going to reuse this
        //TODO: maybe have one that just force copies ect. Not sure but think about this in the future
        std::vector<HBuffer> GetBodyPartsCopy() noexcept;
    public:
        //std::string& GetHeader(const char* name);
        //std::shared_ptr<Cookie> GetCookie(const char* name);
        HBuffer& GetHeader(const char* name) noexcept;
        HBuffer& GetHeader(const HBuffer& name) noexcept;
        std::shared_ptr<Cookie> GetCookie(const char* name) noexcept;
        std::shared_ptr<Cookie> GetCookie(const HBuffer& name) noexcept;

        HBuffer& GetPath() const noexcept{return (HBuffer&)m_Path;}
        HttpVerb GetVerb() const noexcept{return m_Verb;}
        std::vector<HBuffer>& GetBody() const noexcept{return (std::vector<HBuffer>&)m_Body;}

        //const std::unordered_map<std::string, std::string> GetHeaders() const noexcept{return m_Headers;}
        std::unordered_map<HBuffer, HBuffer>& GetHeaders() const noexcept{return (std::unordered_map<HBuffer, HBuffer>&)m_Headers;}
        std::unordered_map<HBuffer, std::shared_ptr<Cookie>>& GetCookies() const noexcept{return (std::unordered_map<HBuffer, std::shared_ptr<Cookie>>&)m_Cookies;}
    private:
        int Parse() noexcept;
        HttpVersion m_Version = HttpVersion::Unsupported;
        HttpVerb m_Verb = HttpVerb::Unknown;
        HBuffer m_Path;
        std::unordered_map<HBuffer, HBuffer> m_Headers;
        std::unordered_map<HBuffer, std::shared_ptr<Cookie>> m_Cookies;
        bool m_IsBodyCompressed=false;
        std::vector<HBuffer> m_Body;
        
        std::vector<HttpContentEncoding> m_AcceptEncodings;
        uint32_t m_ParsePosition = 0;
    public:
        bool m_MidwayParsing = false;
        
        /* PARSE STATES
        0 - Hasnt attempted parse
        */

        /// @brief State of what we are doing in parsing
        int m_LastState=0;
        /// @brief State inside the current parse state
        uint8_t m_State=0;

        /// @brief first string is the last copy of a "read" buffer if there is one. Right is a view of the new read buffer data
        HBufferJoin m_Join;
        size_t m_At=0;
    };
}