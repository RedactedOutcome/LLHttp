#pragma once

#include "Core.h"
#include "pch.h"
#include "HttpData.h"

namespace LLHttp{
    /// @brief A basic cookie representation that that contains everything but a name for the cookie. Used as a value in a dictionary primarily.
    class Cookie{
    public:
        Cookie()noexcept;
        Cookie(const Cookie& cookie)noexcept;
        Cookie(Cookie&& cookie)noexcept;
        Cookie(const HBuffer& data)noexcept;
        Cookie(HBuffer&& data)noexcept;
        ~Cookie()noexcept;
        
        /// @brief The data of the cookie, value, headers, etc. If we were to have a cookie "testcookie=test; Expires=67" then data is "test; Expires=67"
        template<typename... Args>
        void SetData(Args&&... args)noexcept{
            m_Data = HBuffer(std::forward<Args>(args)...);
            ParseData();
        }
        /// @brief Sets the value of the cookie
        template<typename... Args>
        void SetValue(Args&&... args)noexcept{
            m_Value = HBuffer(std::forward<Args>(args)...);
        }

        /// @brief Sets the value of the cookie and evaluates the entire cookie string
        template<typename... Args>
        void SetValueAndEvaluate(Args&&... args)noexcept{
            m_Value = HBuffer(std::forward<Args>(args)...);
            EvaluateData();
        }

        template <typename... Args>
        void SetHeader(const HBuffer& header, Args&&... args)noexcept{
            m_Headers[header] = HBuffer(std::forward<Args>(args)...);
        }
        template <typename... Args>
        void SetHeader(HBuffer&& header, Args&&... args)noexcept{
            m_Headers[std::move(header)] = HBuffer(std::forward<Args>(args)...);
        }
        template <typename... Args>
        void SetHeader(const char* header, Args&&... args)noexcept{
            m_Headers[header] = HBuffer(std::forward<Args>(args)...);
        }

        /// @brief Get any header/setting inside the cookie
        /// @param name 
        /// @return 
        HBuffer& GetHeader(const char* name) noexcept;

        /// @brief Get any header/setting inside the cookie
        /// @param name 
        /// @return 
        HBuffer& GetHeader(const HBuffer& name) noexcept;
    public:
        /// @brief Evaluates the m_Data and deduces the headers and values off of it
        HttpParseErrorCode ParseData()noexcept;

        /// @brief Reevaluates the m_Data string.
        void EvaluateData()noexcept;
    public:
        const std::unordered_map<HBuffer, HBuffer>& GetHeaderMap() const noexcept{return m_Headers;}

        /// @brief Get Value without needing to modify
        HBuffer& GetData() const noexcept{return (HBuffer&)m_Data;}
        HBuffer& GetValue() const noexcept{return (HBuffer&)m_Value;}
    private:
        std::unordered_map<HBuffer, HBuffer> m_Headers;
        HBuffer m_Data;
        HBuffer m_Value;
    };
}