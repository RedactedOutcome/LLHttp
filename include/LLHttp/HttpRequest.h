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

        /// @brief Starts to parse the head portion of the http request with a copy of data.
        /// @param data the data to steal and parse into the head
        /// @param finishedAt the position where the head ends. if HttpParseErrorCode != HttpParseErrorCode::None *finishedAt shall be ignored
        HttpParseErrorCode ParseHeadCopy(HBuffer&& data, uint32_t* finishedAt) noexcept;

        /// @brief Starts to parse the next body part of the request
        /// @param data the data to steal and parse into the body
        /// @param finishedAt the position where the next body ends. if HttpParseErrorCode != HttpParseErrorCode::None data will not be modified
        HttpParseErrorCode ParseNextBodyCopy(HBuffer&& data, HBuffer& output, uint32_t* finishedAt) noexcept;
    
        HttpParseErrorCode ParseHead(uint32_t* finishedAt) noexcept;
        HttpParseErrorCode ParseBody(HBuffer& output, uint32_t* finishedAt) noexcept;
    public:
        void SetPath(const HBuffer& path) noexcept;
        void SetPath(HBuffer&& path) noexcept;

        /// @brief sets the body to a copy of the string
        /// @param data the string to copy
        void SetBodyAsCopy(const char* data) noexcept;
        void SetBodyAsCopy(char* data, size_t size) noexcept;
        void SetBodyAsCopy(const HBuffer& buffer)noexcept;

        void SetBody(HBuffer&& buffer)noexcept;
        void SetBody(char* data, size_t size, bool canFree, bool canModify) noexcept;

        /// @brief Assigns the body to a Non owning/modifying HBuffer with data
        void SetBodyReference(const char* data)noexcept;
        /// @brief Assigns the body to a Non owning/modifying HBuffer with data and size
        void SetBodyReference(char* data, size_t size)noexcept;
        /// @brief Assigns the body to a Non owning HBuffer with param buffers data
        void SetBodyReference(const HBuffer& buffer)noexcept;

        /// @brief Appends a new buffer to the body list
        void AddBodyReference(const HBuffer& buffer)noexcept;
        /// @brief Appends a new buffer to the body list
        void AddBody(HBuffer&& buffer)noexcept;
        
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
        HttpEncodingErrorCode Decompress() noexcept;

        /// @brief Attempts to compress data in the bodies depending on the Content-Encoding header. 
        /// @return returns enum of type HttpEncodingErrorCode
        HttpEncodingErrorCode Compress() noexcept;
    public:
        HBuffer HeadToBuffer() const noexcept;

        //TODO: have multiple methods to get body parts. Maybe one to try using references because we know we are going to reuse this
        //TODO: maybe have one that just force copies ect. Not sure but think about this in the future
        std::vector<HBuffer> GetBodyPartsCopy() noexcept;
    public:
        std::vector<HBuffer>& GetHeaderValues(const char* name) noexcept;
        std::vector<HBuffer>& GetHeaderValues(const HBuffer& name) noexcept;
        /// @brief Returns the first value inside a header if any
        /// @return returns nullptr if no values else first value
        HBuffer* GetHeader(const char* name) noexcept;

        /// @brief Returns the first value inside a header if any
        /// @return returns nullptr if no values else first value
        HBuffer* GetHeader(const HBuffer& name) noexcept;

        /// @brief Returns the last value inside a header if any
        /// @return returns nullptr if no values else first value
        HBuffer* GetHeaderLastValue(const char* name) noexcept;
        
        /// @brief Returns the last value inside a header if any
        /// @return returns nullptr if no values else first value
        HBuffer* GetHeaderLastValue(const HBuffer& name) noexcept;
    public:
        std::shared_ptr<Cookie> GetCookie(const char* name) noexcept;
        std::shared_ptr<Cookie> GetCookie(const HBuffer& name) noexcept;

        HBuffer& GetPath() const noexcept{return (HBuffer&)m_Path;}
        HttpVerb GetVerb() const noexcept{return m_Verb;}
        std::vector<HBuffer>& GetBody() const noexcept{return (std::vector<HBuffer>&)m_Body;}

        //const std::unordered_map<std::string, std::string> GetHeaders() const noexcept{return m_Headers;}
        std::unordered_map<HBuffer, std::vector<HBuffer>>& GetHeaders() const noexcept{return (std::unordered_map<HBuffer, std::vector<HBuffer>>&)m_Headers;}
        std::unordered_map<HBuffer, std::shared_ptr<Cookie>>& GetCookies() const noexcept{return (std::unordered_map<HBuffer, std::shared_ptr<Cookie>>&)m_Cookies;}
    private:
        int Parse() noexcept;
        HttpVersion m_Version = HttpVersion::Unsupported;
        HttpVerb m_Verb = HttpVerb::Unknown;
        HBuffer m_Path;
        std::unordered_map<HBuffer, std::vector<HBuffer>> m_Headers;
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
        HttpParseErrorCode m_LastState=HttpParseErrorCode::NeedsMoreData;
        /// @brief State inside the current parse state
        uint8_t m_State=0;

        /// @brief first string is the last copy of a "read" buffer if there is one. Right is a view of the new read buffer data
        HBufferJoin m_Join;
        size_t m_At=0;
    };
}