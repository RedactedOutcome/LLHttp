#pragma once

#include "Core.h"
#ifdef LLHTTP_USE_PCH
#include LLHTTP_PCH_DIR
#else
#include <string>
#include <unordered_map>
#include <memory>
#endif

namespace LLHttp{
    class Cookie{
    public:
        Cookie();
        ~Cookie();

        void SetValue(const char* value);
        void SetValue(std::string value);
        void SetValue(std::string& value);
        void SetValue(std::string&& value);

        void SetHeader(const char* name, const char* value);
        void SetHeader(const char* name, std::string value);
        void SetHeader(const char* name, std::string& value);
        void SetHeader(const char* name, std::string&& value);

        std::string& GetHeader(const char* name);
    public:
        const std::unordered_map<const char*, std::string>& GetHeaderMap() const noexcept{return m_Headers;}
        const std::string& GetValue() const noexcept{return m_Value;}
    private:
        std::unordered_map<const char*, std::string> m_Headers;
        std::string m_Value = "";
    };

    #define CreateCookie(...) std::make_shared<Cookie>(__VA_ARGS__)
}