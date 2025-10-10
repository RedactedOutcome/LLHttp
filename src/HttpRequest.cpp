#include "pch.h"
#include "HttpRequest.h"
#include "HttpData.h"
#include "LLHttp.h"

namespace LLHttp{
    HttpRequest::HttpRequest()noexcept{

    }
    HttpRequest::~HttpRequest()noexcept{

    }
    
    HttpRequest::HttpRequest(HttpRequest&& request)noexcept{
        m_Version = request.m_Version;
        m_Verb = request.m_Verb;
        m_State = request.m_State;
        m_LastState = request.m_LastState;
        m_Remaining = request.m_Remaining;
        m_At = request.m_At;
        m_MidwayParsing = request.m_MidwayParsing;
        m_Body = std::move(request.m_Body);
        m_Cookies = std::move(request.m_Cookies);
        m_Headers = std::move(request.m_Headers);
        m_Join = std::move(request.m_Join);
        m_Path = std::move(request.m_Path);

        request.Clear();
    }
    void HttpRequest::Clear(){
        m_Version = HttpVersion::Unsupported;
        m_LastState = HttpParseErrorCode::NeedsMoreData;
        m_State = RequestReadState::Unknown;
        m_Verb = HttpVerb::Unknown;
        m_Path.Free();
        m_Headers.clear();
        m_Cookies.clear();
        m_Body.clear();
        m_MidwayParsing = false;

        m_At = 0;
        m_Remaining = -1;
        m_Join.Free();
    }

    HttpParseErrorCode HttpRequest::ParseHead(const HBuffer& data, BodyParseInfo* info)noexcept{
        if(m_LastState != HttpParseErrorCode::NeedsMoreData)return m_LastState;

        HBuffer* buff = &m_Join.GetBuffer1();
        if(buff->GetSize() > 0){
            HBuffer& buff2 = m_Join.GetBuffer2();
            if(buff2.GetSize() > 0){
                buff->Consume(m_At, buff2);
                m_At = 0;
            }else{
                buff = &buff2;
            }
        }
        buff->Assign(data);

        HttpParseErrorCode error = ParseHead(info);
        m_LastState = error;
        
        if(error == HttpParseErrorCode::None && m_At >= m_Join.GetSize()){
            m_At = 0;
            /// @brief freeing incase data is temporary and we dont want dangling pointers
            m_Join.Free();
            return error;
        }
        if(info->m_CopyNecessary)
            buff->Assign(buff->GetCopy());
        return error;
    }
    HttpParseErrorCode HttpRequest::ParseHeadCopy(HBuffer&& data, BodyParseInfo* info)noexcept{
        if(m_LastState != HttpParseErrorCode::NeedsMoreData && m_LastState != HttpParseErrorCode::None)return m_LastState;

        HBuffer* buff = &m_Join.GetBuffer1();
        if(buff->GetSize() > 0){
            HBuffer& buff2 = m_Join.GetBuffer2();
            if(buff2.GetSize() > 0){
                buff->Consume(m_At, buff2);
                m_At = 0;
            }else{
                buff = &buff2;
            }
        }
        buff->Assign(data);

        HttpParseErrorCode error = ParseHead(info);
        m_LastState = error;
        return error;
    }
    HttpParseErrorCode HttpRequest::ParseNextBody(const HBuffer& data, HBuffer& output, BodyParseInfo* info) noexcept{
        if(m_LastState != HttpParseErrorCode::NeedsMoreData && m_LastState != HttpParseErrorCode::None)return m_LastState;
        HBuffer* buff = &m_Join.GetBuffer1();
        if(buff->GetSize() > 0){
            HBuffer& buff2 = m_Join.GetBuffer2();
            if(buff2.GetSize() > 0){
                buff->Consume(m_At, buff2);
                m_At = 0;
            }else{
                buff = &buff2;
            }
        }
        buff->Assign(data);

        HttpParseErrorCode error = ParseBody(output, info);
        m_LastState = error;

        if(error == HttpParseErrorCode::None && m_At >= m_Join.GetSize()){
            m_At = 0;
            /// @brief freeing incase data is temporary and we dont want dangling pointers
            m_Join.Free();
            return error;
        }
        if(info->m_CopyNecessary)
            buff->Assign(buff->GetCopy());        
        return error;
    }
    HttpParseErrorCode HttpRequest::ParseNextBodyCopy(HBuffer&& data, HBuffer& output, BodyParseInfo* info) noexcept{
        if(m_LastState != HttpParseErrorCode::NeedsMoreData && m_LastState != HttpParseErrorCode::None)return m_LastState;
        HBuffer* buff = &m_Join.GetBuffer1();
        if(buff->GetSize() > 0){
            HBuffer& buff2 = m_Join.GetBuffer2();
            if(buff2.GetSize() > 0){
                buff->Consume(m_At, buff2);
                m_At = 0;
            }else{
                buff = &buff2;
            }
        }
        buff->Assign(data);

        HttpParseErrorCode error = ParseBody(output, info);
        m_LastState = error;
        return error;
    }

    HttpParseErrorCode HttpRequest::ParseHead(BodyParseInfo* info)noexcept{
        switch(m_Version){
        case HttpVersion::HTTP0_9:
        case HttpVersion::HTTP1_0:
        case HttpVersion::HTTP1_1:{
            /// TODO : headers/cookies
            switch(m_State){
            case (RequestReadState::HeadersAndCookies):{
                //Headers
                while(true){
                    size_t wasAt =m_At;
                    size_t headerEnd =m_At;
                    size_t valueStart=m_At;
                    size_t valueEnd =m_At;

                    //HeaderName
                    while(true){
                        /// @brief check for double line end to stop the head phase
                        int status = m_Join.StrXCmp(headerEnd, "\r\n");
                        if(status == 0)
                            return HttpParseErrorCode::InvalidHeaderName;
                        if(status == -1){
                            return HttpParseErrorCode::NeedsMoreData;
                        }
                        
                        char c = m_Join.Get(headerEnd);
                        if(c == ':')break;
                        if(!std::isdigit(c) && !std::isalpha(c) && c!= '-' && c!='_'){
                            //CORE_DEBUG("BREAKING {0} {1}", (size_t)c, m_Join.SubString(headerEnd, 5).GetCStr());
                            return HttpParseErrorCode::InvalidHeaderName;
                        }
                        headerEnd++;
                    }

                    //Check for valid end
                    char spaceChar = m_Join.Get(headerEnd + 1);
                    if(spaceChar != ' '){
                        if(spaceChar == '\0'){
                            return HttpParseErrorCode::NeedsMoreData;
                        }
                        return HttpParseErrorCode::InvalidHeaderSplit;
                    }

                    valueStart = headerEnd + 2;
                    valueEnd = valueStart;

                    //HeaderValue
                    while(true){
                        int status = m_Join.StrXCmp(valueEnd, "\r\n");
                        if(status == 0)
                            break;
                        if(status == -1){
                            return HttpParseErrorCode::NeedsMoreData;
                        }
                        char c= m_Join.Get(m_At);
                        /// TODO: make table
                        //if(c != '*' && c != '+' && c != '\'' && c!= ' ' && c != '"' && c != ';' && c!= ',' && c!= '&' && c != '=' && c != '?' && c != ':' && c != '/' && c != '-' && c != '_' && c != '.' && c != '~' && c != '%' && !std::isalpha(c) && !std::isdigit(c)){
                        if(!::LLHttp::IsValidHeaderValueCharacter(c)){
                            return HttpParseErrorCode::InvalidHeaderValue;
                        }
                        valueEnd++;
                    }
                    size_t headerSize = headerEnd - wasAt;
                    size_t valueLength = valueEnd - valueStart;

                    HBuffer headerNameBuffer = m_Join.SubString(wasAt, headerSize);

                    HBufferLowercaseEquals equals;
                    if(!equals(headerNameBuffer, "Set-Cookie")){
                        SetHeader(std::move(headerNameBuffer), m_Join.SubString(valueStart, valueLength));
                    }else{
                        HBuffer value = m_Join.SubPointer(valueStart, valueLength);
                        std::vector<HBuffer> parts = value.SubPointerSplitByDelimiter('=', 1);
                        if(parts.size() < 2){
                            return HttpParseErrorCode::InvalidCookie;
                        }

                        HBuffer cookieName = parts[0].SubString();
                        HBuffer cookieValue = parts[1].SubString(0,-1);
                        Cookie cookie(std::move(cookieValue));
                        m_Cookies.insert(std::make_pair(std::move(cookieName), std::move(cookie)));
                    }
                    m_At = valueEnd + 2;
                    
                    if(m_Join.StartsWith(m_At, "\r\n")){
                        m_At+=2;
                        break;
                    }
                }
                /// Checking if body is encoded.
                HBuffer& encoding = GetHeader("Content-Encoding");
                m_IsBodyEncoded = encoding != "" && encoding != "identity";
                /// TODO: might have a seperate variable for body encoding for ease of access
                m_State = RequestReadState::DetectBodyType;
                return HttpParseErrorCode::None;
            }
            default:{
                return HttpParseErrorCode::InvalidState;
            }
            }
            break;
        }
        default:
            /// @brief Detect Http Version
            /// TODO: Support http 2.2/3
            if(m_Join.StartsWith(m_At, "GET ", 4)){
                m_Verb = HttpVerb::Get;
                m_At+=4;
            }
            else if(m_Join.StartsWith(m_At, "POST ", 5)){
                m_Verb = HttpVerb::Post;
                m_At += 5;
            }
            else if(m_Join.StartsWith(m_At, "CONNECT ", 8)){
                m_Verb = HttpVerb::Connect;
                m_At += 9;
            }
            else if(m_Join.StartsWith(m_At, "DELETE ", 7)){
                m_Verb = HttpVerb::Delete;
                m_At += 7;
            }
            else if(m_Join.StartsWith(m_At, "PUT ", 4)){
                m_Verb = HttpVerb::Put;
                m_At += 4;
            }
            else if(m_Join.StartsWith(m_At, "TRACE ", 6)){
                m_Verb = HttpVerb::Trace;
                m_At += 6;
            }
            else if(m_Join.StartsWith(m_At, "PATCH ", 6)){
                m_Verb = HttpVerb::Patch;
                m_At += 6;
            }
            else if(m_Join.StartsWith(m_At, "OPTIONS ", 8)){
                m_Verb = HttpVerb::Options;
                m_At += 8;
            }
            else if(m_Join.StartsWith(m_At, "HEAD ", 5)){
                m_Verb = HttpVerb::Head;
                m_At += 5;
            }
            else{
                /// TODO: Check for different HTTP Protocol
                m_Verb = HttpVerb::Unknown;
                return HttpParseErrorCode::UnsupportedHttpProtocol;
            }

            /// HTTP 0.9 or HTTP 1.X
            bool valid = false;
            size_t i;
            
            for(i = m_At; i < m_Join.GetSize(); i++){
                if(m_Join.At(i) == ' '){
                    valid = true;
                    break;
                }
            }

            if(!valid)
                return HttpParseErrorCode::NeedsMoreData;
            m_Path = m_Join.SubString(m_At, i - m_At);
            m_At = i + 1;

            /// Specific Version Parsing
            if(m_Join.StartsWith(m_At, "HTTP/0.9\r\n", 10)){
                m_Version = HttpVersion::HTTP0_9;
            }
            else if(m_Join.StartsWith(m_At, "HTTP/1.0\r\n", 10)){
                m_Version = HttpVersion::HTTP1_0;
            }
            else if(m_Join.StartsWith(m_At, "HTTP/1.1\r\n", 10)){
                m_Version = HttpVersion::HTTP1_1;
            }
            else{
                return HttpParseErrorCode::UnsupportedHttpProtocol;
            }
            m_At += 10;
            m_State = RequestReadState::HeadersAndCookies;
            return ParseHead(info);
        }
    }
    HttpParseErrorCode HttpRequest::ParseBody(HBuffer& output, BodyParseInfo* info)noexcept{
        switch(m_State){
            case RequestReadState::DetectBodyType:{
                //Get ransfer encoding
                HBuffer& transferEncoding = GetHeader("Transfer-Encoding");
                if(!transferEncoding || transferEncoding == ""){
                    if(GetHeader("Content-Length")){
                        m_State = RequestReadState::IdentityBody;
                        return ParseBody(output, info);
                    }
                    m_State = RequestReadState::EndOfBodies;
                }
                else if(transferEncoding == "identity"){
                    m_State = RequestReadState::IdentityBody;
                }else if(transferEncoding == "chunked"){
                    m_State = RequestReadState::ChunkedBody;
                }else{
                    return HttpParseErrorCode::UnsupportedTransferEncoding;
                }
                return ParseBody(output, info);
            }
            case RequestReadState::IdentityBody:{//Get the body from no transfer encoding
                if(m_Remaining != -1){
                    /// Remaining has a valid value

                    size_t fillSize = m_Join.GetSize() - m_At;
                    if(m_Remaining < 1)
                        return HttpParseErrorCode::NoMoreBodies;
                    info->m_ValidBody = true;
                    if(fillSize < m_Remaining){
                        m_Remaining -= fillSize;
                        output = m_Join.SubString(m_At, fillSize);
                        m_At+=fillSize;
                        //std::cout<<"H" << __LINE__<<std::endl;
                        return HttpParseErrorCode::NeedsMoreData;
                    }

                    output = m_Join.SubString(m_At, m_Remaining);
                    m_At+=m_Remaining;
                    m_Remaining = 0;
                    m_State = RequestReadState::Finished;
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
                if(m_Join.GetSize() - m_At < contentLengthValue){
                    size_t fillSize = std::min(contentLengthValue, m_Join.GetSize() - m_At);
                    m_Remaining = contentLengthValue - fillSize;
                    output = std::move(m_Join.SubString(m_At, fillSize));
                    info->m_ValidBody;
                    return HttpParseErrorCode::NeedsMoreData;
                }

                /// TODO: Check for encoding and decode
                //Gots all the body data we need
                output = std::move(m_Join.SubString(m_At, contentLengthValue));
                m_State = RequestReadState::Finished;
                return HttpParseErrorCode::None;
            }
            case RequestReadState::ChunkedBody:{//Get body from chunked transfer encoding
                size_t before = m_At;
                uint8_t state = 0;

                while(true){
                    char current = m_Join.Get(m_At++);
                    if(current == '\r'){
                        state++;
                        continue;
                    }
                    if(current == '\n'){
                        state++;
                        if(state == 2)break;
                    }
                    if(current == '\0'){
                        m_At = before;
                        return HttpParseErrorCode::NeedsMoreData;
                    }
                    state = 0;
                }

                size_t bytes = 0;
                for(size_t i = before; i < m_At - 2; i++){
                    uint8_t c = m_Join.At(i);
                    //uint8_t num = 0;
                    if(c >= '0' && c <= '9')
                        c -= '0';
                    else if(c >= 'A' && c <= 'F')
                        c-= 55;
                    else if(c >= 'a' && c <= 'f')
                        c-=87;
                    else{
                        //INVALID CHARACTER;
                        return HttpParseErrorCode::InvalidChunkStart;
                    }
                    bytes <<= 4;
                    bytes+= c;
                }
                
                if(m_At + bytes + 2 < m_Join.GetSize())return HttpParseErrorCode::NeedsMoreData;
                m_Body.emplace_back(std::move(m_Join.SubBuffer(m_At, bytes)));
                m_At += bytes;
                if(m_Join.StartsWith("\r\n") == false)return HttpParseErrorCode::InvalidChunkEnd;
                if(bytes <= 0){
                    m_State = RequestReadState::EndOfBodies;
                    return HttpParseErrorCode::NoMoreBodies;
                }
                return ParseBody(output, info);
            }
            
            case RequestReadState::EndOfBodies:
            case RequestReadState::Finished:{
                return HttpParseErrorCode::NoMoreBodies;
            }
        }
        return HttpParseErrorCode::InvalidState;
    }
    void HttpRequest::SetBodyAsCopy(const char* data)noexcept{
        size_t strLen = strlen(data);
        char* bodyData = new char[strLen];
        //m_Body.Assign(bodyData, strLen, true, true);
        m_Body.clear();
        m_Body.emplace_back(bodyData, strLen, true, true);

        for(size_t i = 0; i < strLen; i++)
            bodyData[i] = data[i];
    }

    void HttpRequest::SetBodyAsCopy(char* data, size_t size)noexcept{
        char* bodyData = new char[size];
        //m_Body.Assign(bodyData, size, true, true);
        m_Body.clear();
        m_Body.emplace_back(bodyData, size, true, true);

        for(size_t i = 0; i < size; i++)
            bodyData[i] = data[i];
    }

    void HttpRequest::SetBodyAsCopy(const HBuffer& buffer)noexcept{
        m_Body.clear();
        m_Body.emplace_back(buffer.GetCopy());
    }

    void HttpRequest::SetBody(HBuffer&& buffer)noexcept{
        //m_Body.Assign(std::move(buffer));
        m_Body.clear();
        m_Body.emplace_back(std::move(buffer));
    }
    void HttpRequest::SetBodyReference(const char* data)noexcept{
        //m_Body.Assign((char*)data, strlen(data), false, false);
        m_Body.clear();
        m_Body.emplace_back((char*)data, strlen(data), false, false);
    }
    void HttpRequest::SetBodyReference(char* data, size_t size)noexcept{
        //m_Body.Assign(data, size, false, false);
        m_Body.clear();
        m_Body.emplace_back(data, size, false, false);
    }
    void HttpRequest::SetBodyReference(const HBuffer& buffer)noexcept{
        //m_Body.Assign(buffer);
        m_Body.clear();
        m_Body.emplace_back(buffer);
    }

    void HttpRequest::AddBodyReference(const HBuffer& path)noexcept{
        m_Body.emplace_back(path);
    }

    void HttpRequest::AddBody(HBuffer&& path)noexcept{
        m_Body.emplace_back(path);
    }
    void HttpRequest::SetPath(const HBuffer& path) noexcept{
        m_Path.Assign(path);
    }
    void HttpRequest::SetPath(HBuffer&& path) noexcept{
        m_Path.Assign(std::move(path));
    }

    void HttpRequest::RemoveHeader(const char* header)noexcept{
        m_Headers.erase(header);
    }
    void HttpRequest::RemoveHeader(const HBuffer& header)noexcept{
        m_Headers.erase(header);
    }
    void HttpRequest::RemoveCookie(const char* cookie)noexcept{
        m_Cookies.erase(cookie);
    }
    void HttpRequest::RemoveCookie(const HBuffer& cookie)noexcept{
        m_Cookies.erase(cookie);
    }
    HBuffer& HttpRequest::GetHeader(const char* name) noexcept{
        return m_Headers[name];
    }
    HBuffer& HttpRequest::GetHeader(const HBuffer& name) noexcept{
        return m_Headers[name];
    }
    HBuffer& HttpRequest::GetHeader(HBuffer&& name) noexcept{
        return m_Headers[std::move(name)];
    }

    void HttpRequest::SetVersion(HttpVersion version)noexcept{
        m_Version = version;
    }
    void HttpRequest::SetVerb(HttpVerb verb)noexcept{
        m_Verb = verb;
    }
    void HttpRequest::PreparePayload(){
        HBuffer& transferEncoding = GetHeader("Transfer-Encoding");

        if(transferEncoding == "chunked"){
            RemoveHeader(HBuffer("Content-Length", 14, false, false));
            return;
        }
        size_t totalSize = 0;
        for(size_t i = 0; i < m_Body.size(); i++){
            totalSize += m_Body[i].GetSize();
        }
        if(totalSize < 1){
            RemoveHeader(HBuffer("Content-Length", 14, false, false));
            return;
        }
        SetHeader("Content-Length", HBuffer::ToString(totalSize));
    }
    HttpParseErrorCode HttpRequest::HeadToBuffer(HBuffer& output) const noexcept{
        switch(m_Version){
        case HttpVersion::HTTP0_9:
        case HttpVersion::HTTP1_0:
        case HttpVersion::HTTP1_1:{
            HBuffer buffer;
            buffer.Reserve(HTTP_DEFAULT_HEAD_REQUEST_TO_BUFFER_SIZE);

            /// TODO: Fix potential overflow later in function.
            switch(m_Verb){
                case HttpVerb::Get:{
                    buffer.Append("GET ", 4);
                    break;
                }
                case HttpVerb::Patch:{
                    buffer.Append("PATCH ", 6);
                    break;
                }
                case HttpVerb::Post:{
                    buffer.Append("POST ", 5);
                    break;
                }
                case HttpVerb::Head:{
                    buffer.Append("HEAD ", 5);
                    break;
                }
                case HttpVerb::Delete:{
                    buffer.Append("DELETE ", 7);
                    break;
                }
                case HttpVerb::Connect:{
                    buffer.Append("CONNECT ", 8);
                    break;
                }
                case HttpVerb::Trace:{
                    buffer.Append("TRACE ", 6);
                    break;
                }
                case HttpVerb::Options:{
                    buffer.Append("OPTIONS ", 8);
                    break;
                }
                default:{
                    return HttpParseErrorCode::UnsupportedHttpVerb;
                }
            }
            if(m_Path.GetSize() > 0)
                buffer.Append(m_Path);
            else
                buffer.Append('/');
            buffer.Append(" HTTP/1.1\r\n", 11);
            
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
                const HBuffer cookieName = pair.first;
                const Cookie& cookie = pair.second;
                const HBuffer& data = cookie.GetData();

                if(cookieName.GetSize() < 1 || data.GetSize() < 1)continue;
                buffer.Append("Set-Cookie: ")
                buffer.Append(cookieName);
                buffer.Append('=');
                buffer.Append(data);
                buffer.Append("\r\n", 2);
            }
               
            buffer.Append("\r\n", 2);
            output = std::move(buffer);
            return HttpParseErrorCode::None;
        }
        default:{
            const char* version = "Unidentified";
            switch(m_Version){
            case HttpVersion::HTTP0_9:
                version = "0.9";
                break;
            case HttpVersion::HTTP1_0:
                version = "1.0";
                break;
            case HttpVersion::HTTP1_1:
                version = "1.1";
                break;
            case HttpVersion::HTTP2_0:
                version = "2";
                break;
            case HttpVersion::HTTP3_0:
                version = "3";
                break;
            }
            /// TODO: log
            return HttpParseErrorCode::UnsupportedHttpProtocol;
        }
        }
    }
    std::vector<HBuffer> HttpRequest::GetBodyPartsCopy()const noexcept{
        std::vector<HBuffer> bodyParts;
        bodyParts.reserve(m_Body.size());
        for(size_t i = 0; i < m_Body.size(); i++)
            bodyParts.emplace_back(m_Body[i].GetCopy());
        return bodyParts;
    }
    HttpEncodingErrorCode HttpRequest::GetFormattedBodyPartsCopy(std::vector<HBuffer>& output)noexcept{
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

    HttpEncodingErrorCode HttpRequest::Decompress() noexcept{
        /// TODO: copy from response
        return HttpEncodingErrorCode::None;
    }

    HttpEncodingErrorCode HttpRequest::Compress() noexcept{
        return HttpEncodingErrorCode::None;
    }
    
    HttpRequest& HttpRequest::operator=(HttpRequest&& request)noexcept{
        m_Version = request.m_Version;
        m_Verb = request.m_Verb;
        m_State = request.m_State;
        m_LastState = request.m_LastState;
        m_At = request.m_At;
        m_MidwayParsing = request.m_MidwayParsing;
        m_Body = std::move(request.m_Body);
        m_Cookies = std::move(request.m_Cookies);
        m_Headers = std::move(request.m_Headers);
        m_Join = std::move(request.m_Join);
        m_Path = std::move(request.m_Path);

        request.Clear();
        return *this;
    }
    void HttpRequest::CopyNecessary()noexcept{
        HBuffer& vec1 = m_Join.GetBuffer1();
        HBuffer& vec2 = m_Join.GetBuffer2();

        size_t vec1Size = vec1.GetSize();
        size_t vec2Size = vec1.GetSize();

        bool ownVec1 = vec1.CanFree();
        bool ownVec2 = vec2.CanFree();

        if(m_At >= m_Join.GetSize()){
            /// @brief no need to copy anything
            m_Join.Free();
            m_At = 0;
            return;
        }
        if(ownVec1 && ownVec2){
            /// @brief no need to copy since we own the data
            // Very rare case with servers
            return;
        }

        if(m_At >= vec1Size){
            /// Only worrying about second buffer atp
            if(ownVec2){
                /// @brief we own the second buffer
                vec1 = std::move(vec2);
                m_At -= vec1Size;
                //vec1 = std::move(vec2.SubPointer(m_At - vec1Size, -1));
                //vec2.Free()
                return;
            }

            vec1.Copy(vec2.SubPointer(m_At - vec1Size, -1));
            vec2.Free();
            m_At = 0;
            return;
        }

        if(ownVec1){
            vec1.Copy(vec1.SubPointer(m_At, -1));
            vec1.Append(vec2);
            vec2.Free();
            m_At = 0;
            return;
        }
        size_t newSize = (vec1Size - m_At) + vec2Size;
        HBuffer buff;
        buff.Reserve(newSize);
        buff.Append(vec1.SubPointer(m_At, -1));
        buff.Append(vec2);
        vec1 = std::move(buff);
        vec2.Free();
        m_At = 0;
    }
}