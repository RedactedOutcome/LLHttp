#include "Cookie.h"

Cookie::Cookie(){}
Cookie::~Cookie(){
}

void Cookie::SetValue(const char* value){
    m_Value = value;
}
void Cookie::SetValue(std::string value){
    m_Value = value;
}
void Cookie::SetValue(std::string& value){
    m_Value = value;
}
void Cookie::SetValue(std::string&& value){
    m_Value = value;
}
std::string& Cookie::GetHeader(const char* name){
    return m_Headers[name];
}

void Cookie::SetHeader(const char* name, const char* value){
    m_Headers[name] = value;
}
void Cookie::SetHeader(const char* name, std::string value){
    m_Headers[name] = value;
}
void Cookie::SetHeader(const char* name, std::string& value){
     m_Headers[name] = value;
}
void Cookie::SetHeader(const char* name, std::string&& value){
     m_Headers[name] = value;
}