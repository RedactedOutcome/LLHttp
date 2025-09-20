#pragma once

#ifdef LLHTTP_USE_PCH
#include LLHTTP_PCH_DIR
#else
#include <HBuffer/HBuffer.hpp>
#include <HBuffer/HBufferExtras.hpp>
#include <unordered_map>
#include <cstdint>
#include <vector>
#endif

#include "Cookie.h"
#include "HttpData.h"

namespace LLHttp{
    enum class RequestReadState : uint8_t{
        Unknown=0,
        HeadersAndCookies,
        DetectBodyType,
        IdentityBody,
        ChunkedBody,
        EndOfBodies
    };
    class HttpRequest{
    public:
        HttpRequest()noexcept;
        ~HttpRequest()noexcept;
        HttpRequest(const HttpRequest&)noexcept=delete;
        HttpRequest(HttpRequest&& request)noexcept;

        /// @brief Starts to parse the head portion of the http request with a buffer of data. If this data does not finish all the data then we copy it
        /// @param data the data to steal and parse into the head
        /// @param finishedAt the position where the head ends. if HttpParseErrorCode != HttpParseErrorCode::None *finishedAt shall be ignored
        HttpParseErrorCode ParseHead(const HBuffer& data, BodyParseInfo* info) noexcept;

        /// @brief Starts to parse the head portion of the http request with a copy of data.
        /// @param data the data to steal and parse into the head
        /// @param finishedAt the position where the head ends. if HttpParseErrorCode != HttpParseErrorCode::None *finishedAt shall be ignored
        HttpParseErrorCode ParseHeadCopy(HBuffer&& data, BodyParseInfo* info) noexcept;

        /// @brief Starts to parse the next body part of the request with a buffer. if data is still needed then we copy it
        /// @param data the data to steal and parse into the body
        /// @param finishedAt the position where the next body ends. if HttpParseErrorCode != HttpParseErrorCode::None data will not be modified
        HttpParseErrorCode ParseNextBody(const HBuffer& data, HBuffer& output, BodyParseInfo* info) noexcept;

        /// @brief Starts to parse the next body part of the request
        /// @param data the data to steal and parse into the body
        /// @param finishedAt the position where the next body ends. if HttpParseErrorCode != HttpParseErrorCode::None data will not be modified
        HttpParseErrorCode ParseNextBodyCopy(HBuffer&& data, HBuffer& output, BodyParseInfo* info) noexcept;
    
        HttpParseErrorCode ParseHead(BodyParseInfo* info) noexcept;
        HttpParseErrorCode ParseBody(HBuffer& output, BodyParseInfo* parseInfo) noexcept;
    public:
        void SetPath(const HBuffer& path) noexcept;
        void SetPath(HBuffer&& path) noexcept;

        /// @brief copies the null terminated string passed
        void SetBodyAsCopy(const char* data) noexcept;
        /// @brief copies the string passed
        void SetBodyAsCopy(char* data, size_t size) noexcept;
        /// @brief copies the buffer
        void SetBodyAsCopy(const HBuffer& buffer)noexcept;

        void SetBody(HBuffer&& buffer)noexcept;
        
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
        HttpRequest& operator=(HttpRequest&& right)noexcept;
    public:
        HBuffer HeadToBuffer() const noexcept;

        //TODO: have multiple methods to get body parts. Maybe one to try using references because we know we are going to reuse this
        //TODO: maybe have one that just force copies ect. Not sure but think about this in the future
        std::vector<HBuffer> GetBodyPartsCopy() noexcept;
    public:
        HBuffer& GetHeader(const char* name) noexcept;
        HBuffer& GetHeader(const HBuffer& name) noexcept;
        HBuffer& GetHeader(HBuffer&& name) noexcept;
    public:
        std::shared_ptr<Cookie> GetCookie(const char* name) noexcept;
        std::shared_ptr<Cookie> GetCookie(const HBuffer& name) noexcept;

        HBuffer& GetPath() const noexcept{return (HBuffer&)m_Path;}
        HttpVerb GetVerb() const noexcept{return m_Verb;}
        std::vector<HBuffer>& GetBody() const noexcept{return (std::vector<HBuffer>&)m_Body;}

        //const std::unordered_map<std::string, std::string> GetHeaders() const noexcept{return m_Headers;}
        std::unordered_map<HBuffer, HBuffer, HBufferLowercaseHash, HBufferLowercaseEquals>& GetHeaders() const noexcept{return (std::unordered_map<HBuffer, HBuffer, HBufferLowercaseHash, HBufferLowercaseEquals>&)m_Headers;}
        std::unordered_map<HBuffer, std::shared_ptr<Cookie>>& GetCookies() const noexcept{return (std::unordered_map<HBuffer, std::shared_ptr<Cookie>>&)m_Cookies;}
    private:
        HttpVersion m_Version = HttpVersion::Unsupported;
        HttpVerb m_Verb = HttpVerb::Unknown;
        HBuffer m_Path;
        std::unordered_map<HBuffer, HBuffer, HBufferLowercaseHash, HBufferLowercaseEquals> m_Headers;
        std::unordered_map<HBuffer, std::shared_ptr<Cookie>> m_Cookies;
        bool m_IsBodyEncoded=false;
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
        RequestReadState m_State=RequestReadState::Unknown;

        /// @brief first string is the last copy of a "read" buffer if there is one. Right is a view of the new read buffer data
        HBufferJoin m_Join;
        size_t m_At=0;
    };
}