#pragma once

#include "Core.h"
#include "pch.h"
namespace LLHttp{
    /// @brief A basic cookie representation that that contains everything but a name for the cookie. Used as a value in a dictionary primarily.
    class Cookie{
    public:
        Cookie();
        ~Cookie();

        /// @brief The value of the cookie
        void SetValue(const char* value) noexcept;
        void SetValue(const HBuffer& value) noexcept;
        void SetValue(HBuffer&& value) noexcept;

        /// @brief Set any headers in the cookie
        /// @param name 
        /// @param value
        void SetHeader(const char* name, const char* value) noexcept;
        void SetHeader(const char* name, const HBuffer& value) noexcept;
        void SetHeader(const char* name, HBuffer&& value) noexcept;
        void SetHeader(const HBuffer& name, const HBuffer& value) noexcept;
        void SetHeader(HBuffer&& name, HBuffer&& value) noexcept;

        /// @brief Get any header/setting inside the cookie
        /// @param name 
        /// @return 
        HBuffer& GetHeader(const char* name) noexcept;

        /// @brief Get any header/setting inside the cookie
        /// @param name 
        /// @return 
        HBuffer& GetHeader(const HBuffer& name) noexcept;
    public:
        const std::unordered_map<HBuffer, HBuffer>& GetHeaderMap() const noexcept{return m_Headers;}
        /// @brief Get Value without needing to modify
        const HBuffer& GetValue() const noexcept{return m_Value;}

        /// @brief get the value and/or modify it
        HBuffer& GetOwningValue()const noexcept{return (HBuffer&)m_Value;}
    private:
        std::unordered_map<HBuffer, HBuffer> m_Headers;
        HBuffer m_Value;
    };
}