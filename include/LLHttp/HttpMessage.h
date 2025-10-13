#pragma once

#include "HttpData.h"
#include "Cookie.h"

namespace LLHttp{
    enum class MessageReadState : uint8_t{
        /// @brief Unknown is the state of detecting the current Http protocol.
        DetectHttpVersion,
        HeadersAndCookies,
        DetectBodyType,
        IdentityBody,
        ChunkedBody,
        GZipBody,
        CompressBody,
        DeflateBody,
        Finished,
        Unknown
    };

    struct ParseInfo{
        bool m_CopyNecessary=true;
    };

    template<typename BodyAlloc=std::allocator<HBuffer>>
    class HttpMessage{
    public:
        HttpMessage()noexcept{}
        virtual ~HttpMessage()noexcept{}

        using HeaderMapType = std::unordered_map<HBuffer, HBuffer, HBufferLowercaseHash, HBufferLowercaseEquals>;
        using CookieMapType = std::unordered_map<HBuffer, Cookie>;

        void ResetBase()noexcept{
            m_Headers.clear();
            m_Cookies.clear();
            m_Body.clear();

            m_Version = HttpVersion::HTTP1_1;
            m_At = 0;
            m_Join.Free();
            m_ReadState = MessageReadState::DetectHttpVersion;
            m_LastState = HttpParseErrorCode::NeedsMoreData;
        }
    public:
        void SetVersion(HttpVersion version)noexcept{
            m_Version = version;
        }

        template<typename... T>
        void SetBody(T&&... t)noexcept{
            m_Body.clear();
            m_Body.emplace_back(std::forward<T>(t)...);
        }

        /// @brief Sets the stream id for parsing 
        /// @param id a 31 bit http stream id. the last bit is assumed 0
        void SetStreamId(uint32_t id)noexcept{
            m_StreamId = id;
        }
    public:
        template<typename... T>
        void SetHeader(const HBuffer& name, T&&... args)noexcept{
            m_Headers[name] = HBuffer(std::forward<T>(args)...);
        }
        template<typename... T>
        void SetHeader(HBuffer&& name, T&&... args)noexcept{
            m_Headers[std::move(name)] = HBuffer(std::forward<T>(args)...);
        }

        template<typename... T>
        void SetCookie(const HBuffer& name, T&&... args)noexcept{
            m_Cookies[name] = Cookie(std::forward<T>(args)...);
        }
        template<typename... T>
        void SetCookie(HBuffer&& name, T&&... args)noexcept{
            m_Cookies[std::move(name)] = Cookie(std::forward<T>(args)...);
        }
    public:
        HttpVersion GetVersion()const noexcept{return m_Version;}
        MessageReadState GetReadState()const noexcept{return m_ReadState;}
        HttpParseErrorCode GetLastState()const noexcept{return m_LastState;}

        const std::vector<HBuffer, BodyAlloc>& GetBody()const noexcept{return m_Body;}
        const HeaderMapType& GetHeaders()const noexcept{return m_Headers;}
        const CookieMapType& GetCookies()const noexcept{return m_Cookies;}

        std::vector<HBuffer, BodyAlloc>& GetBodyRef()noexcept{return (std::vector<HBuffer, BodyAlloc>&)m_Body;}
        HeaderMapType& GetHeadersRef()noexcept{return (HeaderMapType&)m_Headers;}
        CookieMapType& GetCookiesRef()noexcept{return (CookieMapType&)m_Cookies;}
    public: /// PARSING
        const HBufferJoin& GetJoin()const noexcept{return m_Join;}
        size_t GetParseAt()const noexcept{return m_At;}
        size_t GetRemaining()const noexcept{return m_Remaining;}
        HBufferJoin& GetJoinRef() noexcept{return m_Join;}
    protected:
        HttpVersion m_Version = HttpVersion::HTTP1_1;
        std::vector<HBuffer, BodyAlloc> m_Body;
        HeaderMapType m_Headers;
        std::unordered_map<HBuffer, Cookie> m_Cookies;
    protected:
        /// @brief stream id for http 2 responses. Defaults to an invalid value
        uint32_t m_StreamId=(1<<31);
        MessageReadState m_ReadState = MessageReadState::DetectHttpVersion;
        HttpParseErrorCode m_LastState = HttpParseErrorCode::NeedsMoreData;
        HBufferJoin m_Join;
        /// @brief where inside we are parsing data
        size_t m_At=0;
        /// @brief used for parsing http bodys
        size_t m_Remaining=-1;
    };
}