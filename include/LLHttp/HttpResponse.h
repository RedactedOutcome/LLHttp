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

#ifndef HTTP_DEFAULT_HEAD_RESPONSE_TO_BUFFER_SIZE
#define HTTP_DEFAULT_HEAD_RESPONSE_TO_BUFFER_SIZE 32000
#endif

    namespace LLHttp{
    class HttpResponse{
    public:
        HttpResponse();
        HttpResponse(uint16_t status);
        ~HttpResponse();

        /// @brief parses the http response and makes a copy of the body
        int ParseCopy(HBuffer data);

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
        void PreparePayload()noexcept;

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

        HBuffer& GetHeader(const char* name) noexcept;
        HBuffer& GetHeader(const HBuffer& name) noexcept;
        std::shared_ptr<Cookie> GetCookie(const char* name) noexcept;
        
        uint16_t GetStatus() const noexcept{return m_Status;}
        HttpVersion GetVersion() const noexcept{return m_Version;}
        HttpVerb GetVerb() const noexcept{return m_Verb;}
        //const std::unordered_map<std::string, std::string>& GetHeaders() const noexcept{return m_Headers;}
        std::unordered_map<HBuffer, HBuffer>& GetHeaders() const noexcept{return (std::unordered_map<HBuffer, HBuffer>&)m_Headers;}
        std::unordered_map<HBuffer, std::shared_ptr<Cookie>>& GetCookies() const noexcept{return (std::unordered_map<HBuffer, std::shared_ptr<Cookie>>&)m_Cookies;}
        //const std::unordered_map<std::string, std::shared_ptr<Cookie>>& GetCookies() const noexcept{return m_Cookies;}
        std::vector<HBuffer>& GetBody() const noexcept{return (std::vector<HBuffer>&)m_Body;}
    private:
        int Parse() noexcept;
    private:
        uint16_t m_Status = 0;
        HttpVersion m_Version = HttpVersion::Unsupported;
        HttpVerb m_Verb = HttpVerb::Unknown;
        //std::unordered_map<std::string, std::string> m_Headers;
        //std::unordered_map<std::string, std::shared_ptr<Cookie>> m_Cookies;
        std::unordered_map<HBuffer, HBuffer> m_Headers;
        std::unordered_map<HBuffer, std::shared_ptr<Cookie>> m_Cookies;
        //std::unordered_map<HBuffer, HBuffer> m_NewHeaders;
        bool m_IsBodyCompressed=false;
        //z_stream m_Stream = {};
        //HBuffer m_Body;
        std::vector<HBuffer> m_Body;
    private:
        uint32_t m_ParsePosition = 0;
    public:
        bool m_MidwayParsing = false;

        /// @brief State of last parse. Wether it was a success, needed data, or an error
        int m_LastState=0;
        /// @brief State inside the current parse state
        uint8_t m_State=0;

        /// @brief first string is the last copy of a "read" buffer if there is one. Right is a view of the new read buffer data
        HBufferJoin m_Join;
        int32_t m_At = 0;
    };
}