#include "pch.h"
#include "LLHttp/Cookie.h"

namespace LLHttp{
    Cookie::Cookie()noexcept{}
    Cookie::Cookie(const HBuffer& data)noexcept : m_Data(data){
        ParseData();
    }
    Cookie(const Cookie& cookie)noexcept{
        m_Data = cookie.m_Data.SubString(0,-1);
        ParseData();  
    }
    Cookie(Cookie&& cookie)noexcept{
        m_Data = std::move(cookie.m_Data);
        m_Value = std::move(cookie.m_Value);
    }
    Cookie::~Cookie()noexcept{

    }

    void Cookie::SetValue(const char* value, bool reevaluateData) noexcept{
        m_Value = value;
    }

    void Cookie::SetValue(const HBuffer& value, bool reevaluateData) noexcept{
        m_Value = value;
    }

    void Cookie::SetValue(HBuffer&& value, ) noexcept{
        m_Value = std::move(value);
    }
    
    HBuffer& Cookie::GetHeader(const char* name) noexcept{
        return m_Headers[name];
    }

    HBuffer& Cookie::GetHeader(const HBuffer& name) noexcept{
        return m_Headers[name];
    }

    HttpParseErrorCode Cookie::ParseData()noexcept{
        m_Headers.clear();
        m_Value = "";

        std::vector<HBuffer> splits = m_Data.SubPointerSplitByDelimiter(';');
        if(splits.size() < 1){
            return HttpParseErrorCode::InvalidCookie;
        }

        m_Value = splits[0];
        for(size_t i = 1; i < splits.size(); i++){
            /// header:value
            std::cout<<"Header data : "<< splits[i].SubString(0,-1).GetCStr()<<std::endl;
        }
        return HttpParseErrorCode::None;
    }

    void Cookie::EvaluateData()noexcept{
        m_Data.SetSize(0);
        m_Data.Append(m_Value);
        m_Data.Append("; ", 2);
        for(auto it : m_Headers){
            const HBuffer name = it.first;
            const HBuffer value = it.second;
            if(name.GetSize() < 1 || value.GetSize() < 1)continue;

            m_Data.Append(name);
            m_Data.Append('=');
            m_Data.Apend(value);
            m_Data.Append("; ", 2);
        }
    }
}