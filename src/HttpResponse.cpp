#include "pch.h"
#include "HttpResponse.h"
#include "Decoder.h"
#include "LLHttp.h"

namespace LLHttp{
    HttpResponse::HttpResponse(){
        //memset(&m_Stream, 0, sizeof(z_stream));
    }
    HttpResponse::HttpResponse(uint16_t status){
        //memset(&m_Stream, 0, sizeof(z_stream));
    }
    HttpResponse::~HttpResponse(){
        //m_Body.Free();
    }
    
    void HttpResponse::PrepareBodyRead() noexcept{
        m_State = ResponseReadState::DetectBodyType;
        m_At = 0;
        m_Join.Free();
        m_LastState = HttpParseErrorCode::NeedsMoreData;
    }

    void HttpResponse::Clear()noexcept{
        m_Headers.clear();
        m_Cookies.clear();
        m_Body.clear();
        m_IsBodyCompressed = false;
        m_Join.Free();
        m_At = 0;
        m_Remaining = -1;
        m_LastState = HttpParseErrorCode::NeedsMoreData;
        m_State = ResponseReadState::Unknown;
        
        m_Version = HttpVersion::Unsupported;
    }
    HttpParseErrorCode HttpResponse::ParseHead(const HBuffer& data, BodyParseInfo* info) noexcept{
        if(m_LastState != HttpParseErrorCode::NeedsMoreData)return m_LastState;

        HBuffer* buff = &m_Join.GetBuffer1();
        buff->Consume(m_At, m_Join.GetBuffer2());
        if(buff->GetSize() > 0)
            buff = &m_Join.GetBuffer2();
        buff->Assign(data);
        /// TODO: fix potential bugs with reassigning m_At
        m_At = 0;
        HttpParseErrorCode error = ParseHead(info);
        m_LastState = error;

        if(error == HttpParseErrorCode::None && m_At >= m_Join.GetSize()){
            m_At = 0;
            /// @brief freeing incase data is temporary and we dont want dangling pointers
            buff->Free();
            return error;
        }
        if(info->m_CopyNecessary)buff->Assign(buff->GetCopy());
        return error;
    }
    
    HttpParseErrorCode HttpResponse::ParseHeadCopy(HBuffer&& data, BodyParseInfo* info) noexcept{
        if(m_LastState != HttpParseErrorCode::NeedsMoreData && m_LastState != HttpParseErrorCode::None)return m_LastState;

        HBuffer* buff = &m_Join.GetBuffer1();
        buff->Consume(m_At, m_Join.GetBuffer2());
        if(buff->GetSize() > 0)
            buff = &m_Join.GetBuffer2();
        buff->Assign(std::move(data));
        /// TODO: fix potential bugs with reassigning m_At
        m_At = 0;
        HttpParseErrorCode error = ParseHead(info);
        m_LastState = error;
        return error;
    }

    HttpParseErrorCode HttpResponse::ParseNextBody(const HBuffer& data, HBuffer& output, BodyParseInfo* info) noexcept{
        if(m_LastState != HttpParseErrorCode::NeedsMoreData && m_LastState != HttpParseErrorCode::None)return m_LastState;
        HBuffer* buff = &m_Join.GetBuffer1();
        buff->Consume(m_At, m_Join.GetBuffer2());
        if(buff->GetSize() > 0)
            buff = &m_Join.GetBuffer2();
        //std::cout << "Buff siz e" << buff->GetSize()<<std::endl;
        buff->Assign(std::move(data));
        /// TODO: fix potential bugs with reassigning m_At
        m_At = 0;

        HttpParseErrorCode error = ParseBodyTo(output, info);
        m_LastState = error;
        if((error == HttpParseErrorCode::None || error == HttpParseErrorCode::NoMoreBodies) && m_At >= m_Join.GetSize()){
            /// @brief freeing incase data is temporary and we dont want dangling pointers
            m_At = 0;
            buff->Free();
            return error;
        }
        if(info->m_CopyNecessary)buff->Assign(buff->GetCopy());
        return error;
    }

    HttpParseErrorCode HttpResponse::ParseNextBodyCopy(HBuffer&& data, HBuffer& output, BodyParseInfo* info) noexcept{
        if(m_LastState != HttpParseErrorCode::NeedsMoreData && m_LastState != HttpParseErrorCode::None)return m_LastState;
        HBuffer* buff = &m_Join.GetBuffer1();
        buff->Consume(m_At, m_Join.GetBuffer2());
        if(buff->GetSize() > 0)
            buff = &m_Join.GetBuffer2();
        buff->Assign(std::move(data));
        /// TODO: fix potential bugs with reassigning m_At
        m_At = 0;

        HttpParseErrorCode error = ParseBodyTo(output, info);
        m_LastState = error;
        return error;
    }

    HttpParseErrorCode HttpResponse::ParseHead(BodyParseInfo* info)noexcept{
        switch(m_Version){
            case HttpVersion::HTTP1_0:
            case HttpVersion::HTTP1_1:
            switch(m_State){
            case ResponseReadState::HeadersAndCookies:
                //Get Headers
                while(true){
                    std::cout << "Starting with"<< m_Join.SubString(m_At, 15).GetCStr()<<std::endl;
                                
                    /// @brief check for double line end to stop the head phase
                    int status = m_Join.StrXCmp(m_At, "\r\n");
                    if(status == 0)
                        break;
                    if(status == -1){
                        return HttpParseErrorCode::NeedsMoreData;
                    }

                    size_t startAt = m_At;
                    while(true){
                        char c = m_Join.Get(m_At);
                        if(c == ':')break;
                        if(!::LLHttp::IsValidHeaderNameCharacter(c)){
                            if(m_At >= m_Join.GetSize()){
                                m_At = startAt;
                                return HttpParseErrorCode::NeedsMoreData;
                            }
                            return HttpParseErrorCode::InvalidHeaderName;
                        }
                        m_At++;
                    }

                    size_t headerLength = m_At - startAt;
                    
                    char c = m_Join.Get(++m_At);
                    if(c != ' '){
                        m_At = startAt;
                        if(m_At >= m_Join.GetSize())return HttpParseErrorCode::NeedsMoreData;
                        return HttpParseErrorCode::InvalidHeaderSplit;
                    }

                    size_t headerValueStart = ++m_At;
                    while(true){
                        char c = m_Join.Get(m_At);
                        if(!::LLHttp::IsValidHeaderValueCharacter(c)){
                            int status = m_Join.StrXCmp(m_At, "\r\n");
                            if(status == 0)
                                break;
                            if(status == -1){
                                m_At = startAt;
                                std::cout<<"needsmore"<<std::endl;
                                std::cout << "Start at is "<< m_Join.SubString(startAt, 15).GetCStr()<<std::endl;
                                return HttpParseErrorCode::NeedsMoreData;
                            }
                            return HttpParseErrorCode::InvalidHeaderValue;
                        }
                        m_At++;
                    }
                    //Last Value
                    HBuffer headerName = m_Join.SubString(startAt, headerLength);
                    HBuffer headerValue = m_Join.SubString(headerValueStart, m_At - headerValueStart);

                    HBufferLowercaseEquals equals;
                    std::cout << "Setting " << headerName.GetCStr() << ": "<< headerValue.GetCStr()<<std::endl;
                    if(!equals(headerName, "Set-Cookie")){
                        m_Headers.insert(std::make_pair(std::move(headerName), std::move(headerValue)));
                    }else{
                        /// TODO: Set cookies map with key
                    }
                    
                    //Jump past \r\n
                    m_At+=2;
                    if(m_Join.StartsWith(m_At, "\r\n", 2)){
                        //Check for body start
                        m_At+=2;
                        break;
                    }
                }
                m_State = ResponseReadState::DetectBodyType;
                return HttpParseErrorCode::None;
            default:
                return HttpParseErrorCode::UnsupportedHttpProtocol;
            }
        }
        //Get Protocol
        if(m_Join.GetSize() < 15)return HttpParseErrorCode::NeedsMoreData;
        bool http1_0 = m_Join.StartsWith("HTTP/1.0", 8);
        bool http1_1 = m_Join.StartsWith("HTTP/1.1", 8);
        if(http1_1 || http1_0){
            if(m_Join.Get(8)!=' ')return HttpParseErrorCode::InvalidHttpResponse;
            
            for(uint8_t i = 0; i < 3; i++){
                char c = m_Join.Get(i + 9);
                if(c == '\0'){
                    return HttpParseErrorCode::NeedsMoreData;
                }
                if(std::isdigit(c) == false)return HttpParseErrorCode::UnsupportedHttpProtocol;
            }

            m_Status = std::stoi(m_Join.SubString(9, 3).GetCStr());

            if(m_Join.Get(12) != ' ')return HttpParseErrorCode::UnsupportedHttpProtocol;
            m_At = 13;
            while(!m_Join.StartsWith(m_At, "\r\n")){
                if(m_Join.Get(m_At) == '\0')return HttpParseErrorCode::NeedsMoreData;
                m_At++;
            }
            m_At+=2;
            m_State = ResponseReadState::HeadersAndCookies;
            m_Version = http1_1 ? HttpVersion::HTTP1_1 : HttpVersion::HTTP1_0;
            return ParseHead(info);
        }

        return HttpParseErrorCode::UnsupportedHttpProtocol;
    }
    
    HttpParseErrorCode HttpResponse::ParseBodyTo(HBuffer& output, BodyParseInfo* info)noexcept{
        switch(m_Version){
            case HttpVersion::HTTP1_0:
            case HttpVersion::HTTP1_1:
            switch(m_State){
            case ResponseReadState::DetectBodyType:{
                //Detect Transfer Mode
                HBuffer& transferEncoding = GetHeader("Transfer-Encoding");
                //Rest wont be evaluated since after the first it will just jump to true
                if(!transferEncoding || transferEncoding == "" || transferEncoding == "identity"){
                    m_State = ResponseReadState::IdentityBody;
                }
                else if(transferEncoding == "chunked"){
                    m_State = ResponseReadState::ChunkedBody;
                }
                else if(transferEncoding == "gzip" || transferEncoding == "x-gzip"){
                    m_State = ResponseReadState::GZipBody;
                }
                else if(transferEncoding == "compress"){
                    m_State = ResponseReadState::CompressBody;
                }
                else if(transferEncoding == "deflate"){
                    m_State = ResponseReadState::DeflateBody;
                }
                else{
                    return HttpParseErrorCode::UnsupportedTransferEncoding;
                }
                return ParseBodyTo(output, info);
            }
            case ResponseReadState::IdentityBody:{
                if(m_Remaining != -1){
                    /// Remaining has a valid value
                    size_t remainingSize = m_Join.GetSize() - m_At;
                    if(m_Remaining < 1)
                        return HttpParseErrorCode::NoMoreBodies;
                    info->m_ValidBody = true;
                    if(remainingSize < m_Remaining){
                        m_Remaining -= remainingSize;
                        output = m_Join.SubString(m_At, remainingSize);
                        m_At+=remainingSize;
                        return HttpParseErrorCode::NeedsMoreData;
                    }

                    output = m_Join.SubString(m_At, m_Remaining);
                    m_At+=m_Remaining;
                    m_Remaining = -1;
                    m_State = ResponseReadState::Finished;
                    return HttpParseErrorCode::None;
                }
                //Get Body from no encoding with Content-Length
                HBuffer& contentLength = GetHeader("Content-Length");
                if(contentLength == ""){
                    /// @brief If content length is empty with identity encoding then the body parts end when the connection ends
                    output = std::move(m_Join.SubString(m_At, -1));
                    m_At = m_Join.GetSize();
                    info->m_IdentityEndsByStream = true;
                    return HttpParseErrorCode::NoMoreBodies;
                }
                size_t contentLengthValue = std::atoi(contentLength.GetCStr());

                if(contentLengthValue < 1){
                    return HttpParseErrorCode::NoMoreBodies;
                }
                size_t remainingSize = m_Join.GetSize() - m_At;
                if(remainingSize < contentLengthValue){
                    m_Remaining = contentLengthValue - remainingSize;
                    output = m_Join.SubString(m_At, remainingSize);
                    m_At+=m_Remaining;
                    return HttpParseErrorCode::NeedsMoreData;
                }
                /// TODO: Check for encoding and decode
                //Gots all the body data we need
                output = std::move(m_Join.SubString(m_At, contentLengthValue));
                m_State = ResponseReadState::Finished;
                return HttpParseErrorCode::None;
            }
            case ResponseReadState::ChunkedBody:{
                /// @brief used if success parsed in remaining we dont want it to go to waste
                bool shouldReturn = false;
                if(m_Remaining > 0 && m_Remaining != -1){
                    /// @brief getting rest of chunk data
                    size_t remaining = m_Join.GetSize() - m_At;
                    if(remaining <= m_Remaining){
                        output = m_Join.SubBuffer(m_At, remaining);
                        m_Remaining-=remaining;
                        m_At+=remaining;
                        return HttpParseErrorCode::NeedsMoreData;
                    }
                    shouldReturn = true;
                    output = m_Join.SubBuffer(m_At, m_Remaining);
                    m_At+=m_Remaining;
                    m_Remaining=0;
                }
                if(m_Remaining == 0){
                    /// @brief just getting end of chunk

                    int status = m_Join.StrXCmp(m_At, "\r\n");
                    if(status == 1)
                        return HttpParseErrorCode::InvalidChunkEnd;
                    if(status == -1)
                        return HttpParseErrorCode::NeedsMoreData;
                    m_At+=2;
                    /// @brief m_Metadata is treated as size of chunk
                    if(reinterpret_cast<uint64_t>(m_Metadata) == 0){
                        return HttpParseErrorCode::NoMoreBodies;
                    }
                    m_Remaining = -1;
                    if(shouldReturn)return HttpParseErrorCode::None;
                }
                //Transfer Chunked Encoding
                size_t before = m_At;
                int status;
                std::cout << m_Join.SubString(m_At, 15).GetCStr()<<std::endl;
                /*
                int status = m_Join.StrXCmp(m_At, "\r\n");
                if(status == 1)
                    return HttpParseErrorCode::InvalidChunkStart;
                if(status == -1)
                    return HttpParseErrorCode::NeedsMoreData;
                std::cout<<"done"<<std::endl;
                m_At+=2;
                */
                size_t bytes = 0;
                while(true){
                    char c = m_Join.Get(m_At);
                    char real;
                    if(c >= '0' && c <= '9')
                        real = c - '0';
                    else if(c >= 'A' && c <= 'F')
                        real = c - ('A' - 10);
                    else if(c >= 'a' && c <= 'f')
                        real = c - ('a' - 10);
                    else
                        break;
                    bytes <<=4;
                    bytes+=real;
                    m_At++;
                }
                m_Metadata = reinterpret_cast<void*>(bytes);
                status = m_Join.StrXCmp(m_At, "\r\n");
                if(status == 1)return HttpParseErrorCode::InvalidChunkStart;
                if(status == -1){
                    m_At = before;
                    return HttpParseErrorCode::NeedsMoreData;
                }
                m_At+=2;
                size_t fillSize = m_Join.GetSize() - m_At;
                m_Remaining = bytes - std::min(bytes, fillSize);
                if(fillSize <= bytes){
                    output = m_Join.SubBuffer(m_At, fillSize);
                    m_At+=fillSize;
                    return HttpParseErrorCode::NeedsMoreData;
                }
                output = m_Join.SubBuffer(m_At, bytes);
                m_At+=bytes;

                status = m_Join.StrXCmp(m_At, "\r\n");
                if(status == 1)return HttpParseErrorCode::InvalidChunkEnd;
                if(status == -1)return HttpParseErrorCode::NeedsMoreData;
                m_At+=2;

                if(bytes == 0){
                    m_State = ResponseReadState::Finished;
                    return HttpParseErrorCode::NoMoreBodies;
                }
                m_Remaining = -1;
                return HttpParseErrorCode::None;
            }
            case ResponseReadState::GZipBody:{
                //GZIP
                //CORE_ERROR("Getting GZIP Transfer encoding");
                
            }
            case ResponseReadState::CompressBody:{
                //Compress
                //CORE_ERROR("Getting Compress Transfer encoding");
            }
            case ResponseReadState::DeflateBody:{
                //Deflate
                //CORE_ERROR("Getting Deflate Transfer encoding");
            }
            case ResponseReadState::Finished:{
                return HttpParseErrorCode::NoMoreBodies;
            }
            default:
                return HttpParseErrorCode::UnsupportedHttpProtocol;
            }
        }
        return HttpParseErrorCode::UnsupportedHttpProtocol;
    }
    void HttpResponse::RemoveHeader(const char* header)noexcept{
        m_Headers.erase(header);
    }
    void HttpResponse::RemoveHeader(const HBuffer& header)noexcept{
        m_Headers.erase(header);
    }
    HBuffer& HttpResponse::GetHeader(const char* name) noexcept{
        return m_Headers[name];
    }
    HBuffer& HttpResponse::GetHeader(const HBuffer& name) noexcept{
        return m_Headers[name];
    }
    HBuffer& HttpResponse::GetHeader(HBuffer&& name) noexcept{
        return m_Headers[std::move(name)];
    }
    void HttpResponse::SetBodyAsCopy(const char* data)noexcept{
        size_t strLen = strlen(data);
        char* bodyData = new char[strLen];
        for(size_t i = 0; i < strLen; i++)
        bodyData[i] = data[i];
        m_Body.clear();
        m_Body.emplace_back(const_cast<char*>(bodyData), strLen, true, true);
    }
    
    void HttpResponse::SetBodyAsCopy(char* data, size_t size)noexcept{
        char* bodyData = new char[size];
        for(size_t i = 0; i < size; i++)
        bodyData[i] = data[i];
        m_Body.clear();
        m_Body.emplace_back(const_cast<char*>(bodyData), size, true, true);
    }
    void HttpResponse::SetBodyAsCopy(const HBuffer& buffer)noexcept{
        m_Body.clear();
        m_Body.emplace_back(buffer.GetCopy());
    }
    void HttpResponse::SetBody(HBuffer&& buffer)noexcept{
        //m_Body.Assign(std::move(buffer));
        m_Body.clear();
        m_Body.emplace_back(std::move(buffer));
    }
    void HttpResponse::SetBodyReference(const char* data)noexcept{
        //m_Body.Assign((char*)data, strlen(data), false, false);
        m_Body.clear();
        m_Body.emplace_back(const_cast<char*>(data), strlen(data), false, false);
    }
    void HttpResponse::SetBodyReference(char* data, size_t size)noexcept{
        //m_Body.Assign(data, size, false, false);
        
        m_Body.clear();
        m_Body.emplace_back(data, size, false, false);
    }
    void HttpResponse::SetBodyReference(const HBuffer& buffer)noexcept{
        //m_Body.Assign(buffer);
        m_Body.clear();
        m_Body.emplace_back(buffer);
    }
    
    void HttpResponse::AddBodyReference(const HBuffer& buffer)noexcept{
        m_Body.emplace_back(buffer);
    }
    
    void HttpResponse::AddBody(const HBuffer& buffer)noexcept{
        m_Body.emplace_back(std::move(buffer));
    }
    void HttpResponse::AddBody(HBuffer&& buffer)noexcept{
        m_Body.emplace_back(std::move(buffer));
    }
    void HttpResponse::SetStatus(uint16_t status)noexcept{
        m_Status = status;
    }
    void HttpResponse::SetStatus(HttpStatus status)noexcept{
        m_Status = (uint16_t)status;
    }
    
    void HttpResponse::Redirect(const HBuffer& location) noexcept{
        m_Status = (uint16_t)HttpStatus::MovedPermanently;
        SetHeader("Location", location);
    }
    void HttpResponse::Redirect(HBuffer&& location) noexcept{
        m_Status = (uint16_t)HttpStatus::MovedPermanently;
        SetHeader("Location", std::move(location));
    }
    void HttpResponse::SetVersion(HttpVersion version)noexcept{
        m_Version = version;
    }
    void HttpResponse::PreparePayload(size_t preferedLength)noexcept{
        /// TODO: handle multiple content encodings
        HBuffer& transferEncoding = GetHeader("Transfer-Encoding");

        if(transferEncoding == "chunked"){
            RemoveHeader("Content-Length");
            return;
        }
        if(transferEncoding != "" && transferEncoding != "identity"){
            
        }
        //if(strcmp(transferEncodingString, "") != 0 && strcmp(transferEncodingString, "identity") != 0){
            //std::cout << "Failed to prepare payload for HttpResponse. Invalid Transfer-Encoding: " << transferEncodingString << std::endl;
            //CORE_WARN("Failed to prepare payload for HttpResponse. Invalid Transfer-Encoding: {0}", transferEncodingString);
            //return;
        //}
        size_t totalSize = preferedLength;
        if(totalSize != -2){
            if(totalSize == -1){
                totalSize = 0;
                for(size_t i = 0; i < m_Body.size(); i++){
                    totalSize += m_Body[i].GetSize();
                }
            }
            if(totalSize < 1){
                RemoveHeader("Content-Length");
                return;
            }
        }
        
        SetHeader("Content-Length", HBuffer::ToString(totalSize));
    }
    HttpParseErrorCode HttpResponse::HeadToBuffer(HBuffer& output) const noexcept{
        /// TODO: different versions
        switch(m_Version){
        case HttpVersion::HTTP0_9:
        case HttpVersion::HTTP1_0:
        case HttpVersion::HTTP1_1:{
            HBuffer buffer;
            buffer.Reserve(HTTP_DEFAULT_HEAD_RESPONSE_TO_BUFFER_SIZE);
        
            buffer.Append("HTTP/1.1 ");
            HBuffer statusBuff;
            statusBuff.Assign(HBuffer::ToString((size_t)m_Status));
            buffer.Append(statusBuff);
            buffer.Append(' ');
        
            const char* statusInfo;
            switch(m_Status){
            case 101:
                statusInfo = "Switching Protocols";
                break;
            case 102:
                statusInfo = "Processing";
                break;
            case 103:
                statusInfo = "Early Hints";
                break;
            case 200:
                statusInfo = "OK";
                break;
            case 201:
                statusInfo = "Created";
                break;
            case 202:
                statusInfo = "Accepted";
                break;
            case 203:
                statusInfo = "Non-Authoritative Information";
                break;
            case 204:
                statusInfo = "No Content";
                break;
            case 205:
                statusInfo = "Reset Content";
                break;
            case 206:
                statusInfo = "Partial Content";
                break;
            case 207:
                statusInfo = "Multi-Status";
                break;
            case 208:
                statusInfo = "Already Reported";
                break;
            case 226:
                statusInfo = "IM Used";
                break;
            case 300:
                statusInfo = "Multiple Choices";
                break;
            case 301:
                statusInfo = "Moved Permanently";
                break;
            case 302:
                statusInfo = "Found";
                break;
            case 303:
                statusInfo = "See Other";
                break;
            case 304:
                statusInfo = "Not Modified";
                break;
            case 305:
                statusInfo = "Use Proxy";
                break;
            case 306:
                statusInfo = "Unused";
                break;
            case 307:
                statusInfo = "Temporary Redirect";
                break;
            case 308:
                statusInfo = "Permanent Redirect";
                break;
            case 400:
                statusInfo = "Bad Request";
                break;
            case 401:
                statusInfo = "Unauthorized";
                break;
            case 402:
                statusInfo = "Payment Required";
                break;
            case 403:
                statusInfo = "Forbidden";
                break;
            case 404:
                statusInfo = "Not Found";
                break;
            case 405:
                statusInfo = "Method Not Allowed";
                break;
            case 406:
                statusInfo = "Not Acceptable";
                break;
            case 407:
                statusInfo = "Proxy Authentication Required";
                break;
            case 408:
                statusInfo = "Request Timeout";
                break;
            case 409:
                statusInfo = "Conflict";
                break;
            case 410:
                statusInfo = "Gone";
                break;
            case 411:
                statusInfo = "Length Required";
                break;
            case 412:
                statusInfo = "Precondition Failed";
                break;
            case 413:
                statusInfo = "Content Too Large";
                break;
            case 414:
                statusInfo = "URI Too Long";
                break;
            case 415:
                statusInfo = "Unsupported Media Type";
                break;
            case 416:
                statusInfo = "Range Not Satisfiable";
                break;
            case 417:
                statusInfo = "Expectation Failed";
                break;
            case 418:
                statusInfo = "I'm a teapot";
                break;
            case 421:
                statusInfo = "Misdirected Request";
                break;
            case 422:
                statusInfo = "Unprocessable Content";
                break;
            case 423:
                statusInfo = "Locked";
                break;
            case 424:
                statusInfo = "Failed Dependency";
                break;
            case 425:
                statusInfo = "Too Early";
                break;
            case 426:
                statusInfo = "Upgrade Required";
                break;
            case 428:
                statusInfo = "Precondition Required";
                break;
            case 429:
                statusInfo = "Too Many Request";
                break;
            case 431:
                statusInfo = "Request Header Fields Too Large";
                break;
            case 451:
                statusInfo = "Unavailable For Legal Reasons";
                break;
            case 500:
                statusInfo = "InternalServerError";
                break;
            case 501:
                statusInfo = "Not Implemented";
                break;
            case 502:
                statusInfo = "Bad Gateway";
                break;
            case 503:
                statusInfo = "Service Unavailable";
                break;
            case 504:
                statusInfo = "Gateway Timeout";
                break;
            case 505:
                statusInfo = "HTTP Version Not Supported";
                break;
            case 506:
                statusInfo = "Variant Also Negotiates";
                break;
            case 507:
                statusInfo = "Insufficient Storage";
                break;
            case 508:
                statusInfo = "Loop Detected";
                break;
            case 510:
                statusInfo = "Not Extended";
                break;
            case 511:
                statusInfo = "Network Authentication Required";
                break;
            default:
                //CORE_DEBUG("WRITING NOT IMPLEMENTED {0}", m_Status);
                statusInfo = "Not Implemented";
            }
        
            buffer.Append(statusInfo);
            buffer.Append("\r\n", 2);
        
            //Headers
            for (const auto &myPair : m_Headers) {
                const HBuffer& headerName = myPair.first;
                const HBuffer& headerValue = myPair.second;
                if(headerName.GetSize() < 1 || headerValue.GetSize() < 1)continue;
        
                buffer.Append(headerName);
                buffer.Append(": ", 2);
                buffer.Append(headerValue);
                buffer.Append("\r\n", 2);
            }
        
            //Cookies
            for (const auto &pair : m_Cookies) {
                const HBuffer& cookieName = pair.first;
                const Cookie& cookie = pair.second;
                if(cookieName.GetSize() < 1 || !cookie.GetValue())continue;
                buffer.Append(pair.first.GetCStr());
                buffer.Append("= ", 2);
                buffer.Append(pair.second.GetValue());
                buffer.Append("\r\n", 2);
            }
            buffer.Append("\r\n", 2);
            output = std::move(buffer);
            return HttpParseErrorCode::None;
        }
        default:{
            return HttpParseErrorCode::UnsupportedHttpProtocol;
        }
        }
    }

    std::vector<HBuffer> HttpResponse::GetBodyPartsCopy() const noexcept{
        std::vector<HBuffer> buffer;
        buffer.resize(m_Body.size());
        for(size_t i = 0; i < m_Body.size(); i++){
            buffer[i] = m_Body[i].GetCopy();
        }
        return buffer;
    }

    HttpEncodingErrorCode HttpResponse::GetFormattedBodyPartsCopy(std::vector<HBuffer>& output)noexcept{
        HBuffer& transferEncoding = GetHeader("Transfer-Encoding");
        output.reserve(m_Body.size());

        if(!transferEncoding || transferEncoding == "" || transferEncoding == "identity"){
            for(size_t i = 0; i < m_Body.size(); i++){
                output.emplace_back(m_Body[i].GetCopy());
            }

            return HttpEncodingErrorCode::None;
        }else if(transferEncoding == "chunked"){
            for(size_t i = 0; i < m_Body.size(); i++){
                const HBuffer& input = m_Body[i];

                size_t partSize = input.GetSize();
                HBuffer newPart;
                newPart.Reserve(partSize + 5);

                HBuffer string;
                string.Reserve(5);
                
                size_t size = partSize;
                do{
                    char digit = size % 16;
                    string.AppendString(digit >= 10 ? (55 + digit) : (digit + '0'));
                    size/=16;
                }while(size > 0);

                string.Reverse();

                newPart.Reserve(partSize + 6);

                newPart.Append(string.GetData(), string.GetSize());
                newPart.Append('\r');
                newPart.Append('\n');
                newPart.Append(input.GetData(), partSize);

                newPart.Append('\r');
                newPart.Append('\n');
                output.emplace_back(std::move(newPart));
            }

            return HttpEncodingErrorCode::None;
        }
        return HttpEncodingErrorCode::UnsupportedContentEncoding;
    }

    HttpEncodingErrorCode HttpResponse::BufferCopyToValidBodyPartFormat(const HBuffer& input, HBuffer& output) noexcept{ 
        HBuffer& transferEncoding = GetHeader("Transfer-Encoding");

        if(!transferEncoding || transferEncoding == "" || transferEncoding == "identity"){
            output = input.CreateCopy();
            return HttpEncodingErrorCode::None;
        }
        else if(transferEncoding == "chunked"){
            size_t partSize = input.GetSize();

            HBuffer string;
            string.Reserve(5);

            size_t size = partSize;
            do{
                char digit = size % 16;
                string.AppendString(digit >= 10 ? (55 + digit) : (digit + '0'));
                size/=16;
            }while(size > 0);

            string.Reverse();

            output.Reserve(partSize + 6);

            output.Append(string.GetData(), string.GetSize());
            output.Append('\r');
            output.Append('\n');
            output.Append(input.GetData(), partSize);
            
            output.Append('\r');
            output.Append('\n');
            return HttpEncodingErrorCode::None;
        }

        return HttpEncodingErrorCode::UnsupportedContentEncoding;
    }
    HttpEncodingErrorCode HttpResponse::BufferToValidBodyPartFormat(const HBuffer& input, HBuffer& output) noexcept{ 
        HBuffer& transferEncoding = GetHeader("Transfer-Encoding");

        if(!transferEncoding || transferEncoding == "" || transferEncoding == "identity"){
            output = input.GetCopy();
            return HttpEncodingErrorCode::None;
        }
        else if(transferEncoding == "chunked"){
            size_t partSize = input.GetSize();

            HBuffer string;
            string.Reserve(5);

            size_t size = partSize;
            do{
                char digit = size % 16;
                string.AppendString(digit >= 10 ? (55 + digit) : (digit + '0'));
                size/=16;
            }while(size > 0);

            string.Reverse();

            output.Reserve(partSize + 6);

            output.Append(string.GetData(), string.GetSize());
            output.Append('\r');
            output.Append('\n');
            output.Append(input.GetData(), partSize);

            output.Append('\r');
            output.Append('\n');
            return HttpEncodingErrorCode::None;
        }

        return HttpEncodingErrorCode::UnsupportedContentEncoding;
    }
    HttpEncodingErrorCode HttpResponse::BufferToValidBodyPartFormat(HBuffer&& input, HBuffer& output) noexcept{ 
        HBuffer& transferEncoding = GetHeader("Transfer-Encoding");

        if(!transferEncoding || transferEncoding == "" || transferEncoding == "identity"){
            output = std::move(input);
            return HttpEncodingErrorCode::None;
        }
        else if(transferEncoding == "chunked"){
            size_t partSize = input.GetSize();

            HBuffer string;
            string.Reserve(5);

            size_t size = partSize;
            do{
                char digit = size % 16;
                string.AppendString(digit >= 10 ? (55 + digit) : (digit + '0'));
                size/=16;
            }while(size > 0);

            string.Reverse();

            output.Reserve(partSize + 6);

            output.Append(string.GetData(), string.GetSize());
            output.Append('\r');
            output.Append('\n');
            output.Append(input.GetData(), partSize);

            output.Append('\r');
            output.Append('\n');
            return HttpEncodingErrorCode::None;
        }

        return HttpEncodingErrorCode::UnsupportedContentEncoding;
    }

    HttpEncodingErrorCode HttpResponse::Decompress() noexcept{
        HBuffer& contentEncoding = GetHeader("Content-Encoding");

        if(!contentEncoding)return HttpEncodingErrorCode::None;
        std::vector<HttpContentEncoding> encodings;
        size_t at = 0;

        while(at < contentEncoding.GetSize()){
            bool valid = false;
            if(contentEncoding.StartsWith(at, "identity", 8)){
                encodings.push_back(HttpContentEncoding::Identity);
                at+=8;
                valid = true;
            }
            else if(contentEncoding.StartsWith(at, "br", 2)){
                encodings.push_back(HttpContentEncoding::Brotli);
                at+=2;
                valid = true;
            }
            else if(contentEncoding.StartsWith(at, "gzip", 4)){
                encodings.push_back(HttpContentEncoding::GZip);
                at+=4;
                valid = true;
            }
            else if(contentEncoding.StartsWith(at, "compress", 8)){
                encodings.push_back(HttpContentEncoding::Compress);
                at+=8;
                valid = true;
            }

            if(!valid) return HttpEncodingErrorCode::UnsupportedContentEncoding;
        }
        std::vector<HBuffer> newBodies;
        if(encodings.size() == 1)
            if(encodings[0] == HttpContentEncoding::Identity)return HttpEncodingErrorCode::None;
        
        for(int i = encodings.size(); i > 0; --i){
            HttpContentEncoding encoding = encodings[i];
            //newBodies.emplace_back(HBuffer)
            /// TODO: call a function to decode data and append to newBodies
            for(size_t j = 0; j < m_Body.size(); j++){
                /// TODO: Catch unsupported encodings
                Decoder::DecodeData(encoding, m_Body[i], newBodies);
            }
        }
        
        /// TODO: might just clear the body and use the move constructor to reassign m_Body to newBodies
        for(size_t i = 0; i < newBodies.size(); i++)
            m_Body[i].Assign(std::move(newBodies[i]));
        
        return HttpEncodingErrorCode::None;
    }

    HttpEncodingErrorCode HttpResponse::Compress() noexcept{
        return HttpEncodingErrorCode::None;
    }

    void HttpResponse::CopyNecessary()noexcept{
        HBuffer& vec1 = m_Join.GetBuffer1();
        HBuffer& vec2 = m_Join.GetBuffer2();

        size_t vec1Size = vec1.GetSize();
        size_t vec2Size = vec1.GetSize();

        bool ownVec1 = vec1.CanFree();
        bool ownVec2 = vec2.CanFree();
        std::cout<<"Owns are " << ownVec1 << " " << ownVec2<<std::endl;

        if(ownVec1 && ownVec2){
            /// @brief no need to copy since we own the data
            // Very rare case with servers
            return;
        }

        if(m_At >= vec1Size){
            /// Only worrying about second buffer atp
            if(ownVec2){
                /// @brief we own the second buffer
                m_At -= vec1Size;
                vec1 = std::move(vec2);
                return;
            }

            vec1 = vec2.SubString(m_At - vec1Size, -1);
            vec2.Free();
            m_At = 0;
            return;
        }

        if(ownVec1){
        }
    }
}