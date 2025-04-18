#pragma once

#ifdef LLHTTP_USE_PCH
#include LLHTTP_PCH_DIR
#else
#include <iostream>
#include <HBuffer/HBuffer.hpp>
#include <unordered_map>
#include <cstdint>
#include <vector>
#endif

#include "Cookie.h"
#include "HttpData.h"

#ifndef HTTP_DEFAULT_HEAD_RESPONSE_TO_BUFFER_SIZE
#define HTTP_DEFAULT_HEAD_RESPONSE_TO_BUFFER_SIZE 32000
#endif
namespace LLHttp{
    class HttpResponse{
    public:
        HttpResponse();
        HttpResponse(uint16_t status);
        ~HttpResponse();

        /// @brief clears the join buffer and prepares the response to be able to read and parse body parts while keeping necessary information for head, content type, transfer encoding, ect
        void PrepareBodyRead() noexcept;

        /// @brief Starts to parse the head portion of the http response with a copy of data.
        /// @param data the data to steal and parse into the head
        /// @param finishedAt the position where the head ends. if HttpParseErrorCode != HttpParseErrorCode::None *finishedAt shall be ignored
        HttpParseErrorCode ParseHeadCopy(HBuffer&& data, uint32_t* finishedAt) noexcept;

        /// @brief Starts to parse the next body part of the response
        /// @param data the data to steal and parse into the body
        /// @param finishedAt the position where the next body ends. if HttpParseErrorCode != HttpParseErrorCode::None data will not be modified
        HttpParseErrorCode ParseNextBodyCopy(HBuffer&& data, HBuffer& output, uint32_t* finishedAt) noexcept;

        /// @brief sets the body to a copy of the strings internals excluding null terminator
        /// @param data the C string to copy.
        void SetBodyAsCopy(const char* data)noexcept;
        void SetBodyAsCopy(char* data, size_t size)noexcept;
        void SetBodyAsCopy(const HBuffer& buffer)noexcept;

        void SetBody(HBuffer&& buffer)noexcept;
        void SetBody(char* data, size_t size, bool canFree, bool canModify) noexcept;

        /// @brief Sets the body as a pointer to existing data. Will not free data
        /// @param data the string to point to
        void SetBodyReference(const char* data)noexcept;
        void SetBodyReference(char* data, size_t size)noexcept;
        /// @brief Sets body as a non owning reference to buffer. May modify if param buffer allows it
        /// @param buffer the data to reference
        void SetBodyReference(const HBuffer& buffer) noexcept;

        /// @brief appends a new buffer to the body list
        void AddBodyReference(const HBuffer& buffer) noexcept;
        /// @brief appends a new buffer to the body list
        void AddBody(const HBuffer& buffer) noexcept;
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

        void SetStatus(uint16_t status)noexcept;
        void SetStatus(HttpStatus status)noexcept;

        /// @brief Prepares the response to be converted and sent over
        /// @param overrideSize a value to override Content-Length if identity encoding. The value -1 will make have the Content-Length be the size of all the body parts combined
        void PreparePayload(size_t overrideSize = -1)noexcept;

        void Redirect(const HBuffer& location)noexcept;
        void Redirect(HBuffer&& location)noexcept;

        //Completely resets response data
        void PrepareRead()noexcept;
        void Clear()noexcept;

        /// @brief Attempts to decompress the body data depending on the Content-Encoding header. 
        /// @return returns enum of type HttpEncodingErrorCode
        int Decompress() noexcept;

        /// @brief Attempts to compress data in the bodies depending on the Content-Encoding header. 
        /// @return returns enum of type HttpEncodingErrorCode
        int Compress() noexcept;
    public:
        /// @brief returns the entire first part of the http response message just before the body
        HBuffer HeadToBuffer() const noexcept;
        std::vector<HBuffer>& GetBodyParts() const noexcept {return (std::vector<HBuffer>&)m_Body;}
        /// @brief returns a copy of a all the body parts. Body may be merged into a single part if transfer identity encoding. Else body is split up as needed depending on transfer encoding. Data inside the copy will not be decoded or encoded.
        std::vector<HBuffer> GetBodyPartsCopy() noexcept;

        /// @brief Takes a normal buffer and converts it to any encoding necessary depending on the Transfer-Encoding headers. Will not convert data to or from Content-Encoding
        /// @param input the input to make a copy of and return as a valid bodypart
        HBuffer BufferToValidBodyFormat(const HBuffer& input)noexcept;

        /// @brief Takes a normal buffer and converts it to any encoding necessary depending on the Transfer-Encoding headers. Will not convert data to or from Content-Encoding
        /// @param buffers 
        /// @param addEndChunk decides if a null chunk is appended for chunked transfer encoding.
        std::vector<HBuffer> BuffersToValidBodyFormat(std::vector<HBuffer>& buffers, bool addEndChunk=true)noexcept;
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
        
        HttpParseErrorCode ParseHead(uint32_t* finishedAt) noexcept;
        HttpParseErrorCode ParseBody(HBuffer& output, uint32_t* finishedAt) noexcept;
    public:
        std::shared_ptr<Cookie> GetCookie(const char* name) noexcept;
        
        uint16_t GetStatus() const noexcept{return m_Status;}
        HttpVersion GetVersion() const noexcept{return m_Version;}
        HttpVerb GetVerb() const noexcept{return m_Verb;}
        std::unordered_map<HBuffer, std::vector<HBuffer>, HBufferLowercaseHash, HBufferLowercaseEquals>& GetHeaders() const noexcept{return (std::unordered_map<HBuffer, std::vector<HBuffer>, HBufferLowercaseHash, HBufferLowercaseEquals>&)m_Headers;}
        std::unordered_map<HBuffer, std::shared_ptr<Cookie>, HBufferLowercaseHash, HBufferLowercaseEquals>& GetCookies() const noexcept{return (std::unordered_map<HBuffer, std::shared_ptr<Cookie>, HBufferLowercaseHash, HBufferLowercaseEquals>&)m_Cookies;}
        std::vector<HBuffer>& GetBody() const noexcept{return (std::vector<HBuffer>&)m_Body;}
    public:
        /// @brief returns the current position in the joined buffer that we are using to pase data for the current state
        int32_t GetCurrentAt() const noexcept{return m_At;}
        uint8_t GetState() const noexcept{return m_State;}
        const HBufferJoin& GetBufferJoin() const noexcept{return m_Join;}
    private:
        uint16_t m_Status = 0;
        HttpVersion m_Version = HttpVersion::Unsupported;
        HttpVerb m_Verb = HttpVerb::Unknown;
        /// TODO: Case insensitive
        /// @brief a map of headers. When accessed directly the keys are case insensitive. Only through the get functions will be using lowercase functions
        std::unordered_map<HBuffer, std::vector<HBuffer>, HBufferLowercaseHash, HBufferLowercaseEquals> m_Headers;
        std::unordered_map<HBuffer, std::shared_ptr<Cookie>, HBufferLowercaseHash, HBufferLowercaseEquals> m_Cookies;
        bool m_IsBodyCompressed=false;
        std::vector<HBuffer> m_Body;
    public:
        bool m_MidwayParsing = false;

        /// @brief State of last parse. Wether it was a success, needed data, or an error
        HttpParseErrorCode m_LastState = HttpParseErrorCode::NeedsMoreData;
        /// @brief State inside the current parse state
        uint8_t m_State=0;

        /// @brief first string is the last copy of a "read" buffer if there is one. Right is a view of the new read buffer data
        HBufferJoin m_Join;
        size_t m_At = 0;
    };
}