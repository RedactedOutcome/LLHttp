#include "LLHttp/pch.h"
#include "LLHttp/Cookie.h"

    namespace LLHttp{
    Cookie::Cookie(){}
    Cookie::~Cookie(){
    }

    void Cookie::SetValue(const char* value) noexcept{
        m_Value = value;
    }

    void Cookie::SetValue(const HBuffer& value) noexcept{
        m_Value = value;
    }

    void Cookie::SetValue(HBuffer&& value) noexcept{
        m_Value = std::move(value);
    }
    
    HBuffer& Cookie::GetHeader(const char* name) noexcept{
        return m_Headers[name];
    }

    HBuffer& Cookie::GetHeader(const HBuffer& name) noexcept{
        return m_Headers[name];
    }

    void Cookie::SetHeader(const char* name, const char* value) noexcept{
        m_Headers[name] = value;
    }
    void Cookie::SetHeader(const char* name, const HBuffer& value) noexcept{
        m_Headers[name] = value;
    }
    void Cookie::SetHeader(const char* name, HBuffer&& value) noexcept{
        m_Headers[name] = std::move(value);
    }
    void Cookie::SetHeader(const HBuffer& name, const HBuffer& value) noexcept{
        m_Headers[name] = value;
    }
    void Cookie::SetHeader(HBuffer&& name, HBuffer&& value) noexcept{
        m_Headers[std::move(name)] = std::move(value);
    }
}