#pragma once
#include "HttpMessage.h"

namespace LLHttp{
    template<typename T>
    class FutureRequest : public HttpMessage<T>{

    }
    enum class RequestReadState : uint8_t{
        Unknown=0,
        HeadersAndCookies,
        DetectBodyType,
        IdentityBody,
        ChunkedBody,
        EndOfBodies,
        Finished
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
        /// @brief Copies necessary unparsed data. Lets say we pass a read buffer whos data is temporary and will most likely be invalidated after the scope. We will copy any necessary remaining data.
        /// @brief but if we do infact own any data inside the buffer join. We will do our best to avoid reallocations.
        void CopyNecessary()noexcept;
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

        template<typename... Args>
        void SetHeader(const char* name, Args&&... args)noexcept{
            m_Headers[name] = HBuffer(std::forward<Args>(args)...);
        }

        template<typename... Args>
        void SetHeader(const HBuffer& name, Args&&... args)noexcept{
            m_Headers[name] = HBuffer(std::forward<Args>(args)...);
        }

        template<typename... Args>
        void SetHeader(HBuffer&& name, Args&&... args)noexcept{
            m_Headers[std::move(name)] = HBuffer(std::forward<Args>(args)...);
        }

        void RemoveHeader(const char* header) noexcept;
        void RemoveHeader(const HBuffer& header) noexcept;
        void RemoveCookie(const char* cookie) noexcept;
        void RemoveCookie(const HBuffer& cookie) noexcept;
                
        template<typename... Args>
        void SetCookie(const char* name, Args&&... args)noexcept{
            m_Cookies[name] = Cookie(std::forward<Args>(args)...);
        }
        template<typename... Args>
        void SetCookie(const HBuffer& name, Args&&... args)noexcept{
            m_Cookies[name] = Cookie(std::forward<Args>(args)...);
        }
        template<typename... Args>
        void SetCookie(HBuffer&& name, Args&&... args)noexcept{
            m_Cookies[std::move(name)] = Cookie(std::forward<Args>(args)...);
        }

        void SetVersion(HttpVersion version)noexcept;
        void SetVerb(HttpVerb verb)noexcept;

        void SetReadState(RequestReadState state)noexcept;
    public:
        //Clears all data
        void Clear();
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
        HttpParseErrorCode HeadToBuffer(HBuffer& output) const noexcept;

        std::vector<HBuffer> GetBodyPartsCopy() const noexcept;

        HttpEncodingErrorCode GetFormattedBodyPartsCopy(std::vector<HBuffer>& output) noexcept;
    public:
        HBuffer& GetHeader(const char* name) noexcept;
        HBuffer& GetHeader(const HBuffer& name) noexcept;
        HBuffer& GetHeader(HBuffer&& name) noexcept;
    public:
        Cookie& GetCookie(const char* name) noexcept;
        Cookie& GetCookie(const HBuffer& name) noexcept;

        HBuffer& GetPath() const noexcept{return (HBuffer&)m_Path;}
        HttpVerb GetVerb() const noexcept{return m_Verb;}
        std::vector<HBuffer>& GetBody() const noexcept{return (std::vector<HBuffer>&)m_Body;}

        //const std::unordered_map<std::string, std::string> GetHeaders() const noexcept{return m_Headers;}
        std::unordered_map<HBuffer, HBuffer, HBufferLowercaseHash, HBufferLowercaseEquals>& GetHeaders() const noexcept{return (std::unordered_map<HBuffer, HBuffer, HBufferLowercaseHash, HBufferLowercaseEquals>&)m_Headers;}
        std::unordered_map<HBuffer, Cookie>& GetCookies() const noexcept{return (std::unordered_map<HBuffer, Cookie>&)m_Cookies;}
    public:
        HttpVersion GetVersion()const noexcept{return m_Version;}
        RequestReadState GetReadState()const noexcept{return m_State;}
        HBufferJoin& GetJoin()const noexcept{return (HBufferJoin&)m_Join;}
        size_t GetAt() const noexcept{return m_At;}
        size_t GetRemaining()const noexcept{return m_Remaining;}
    private:
        HttpVersion m_Version = HttpVersion::Unsupported;
        HttpVerb m_Verb = HttpVerb::Unknown;
        HBuffer m_Path;
        std::unordered_map<HBuffer, HBuffer, HBufferLowercaseHash, HBufferLowercaseEquals> m_Headers;
        std::unordered_map<HBuffer, Cookie> m_Cookies;
        bool m_IsBodyEncoded=false;
        std::vector<HBuffer> m_Body;
    public:
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
        size_t m_Remaining = -1;
    };
}