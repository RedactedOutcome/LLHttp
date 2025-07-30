#include "LLHttp/pch.h"
#include "HttpRequest.h"
#include "HttpData.h"

namespace LLHttp{
    HttpRequest::HttpRequest(){

    }
    HttpRequest::~HttpRequest(){
        m_Join.Free();
        //m_Body.Free();
        m_Body.clear();
    }
    void HttpRequest::PrepareRead(){
        m_Version = HttpVersion::Unsupported;
        m_LastState = HttpParseErrorCode::NeedsMoreData;
        m_State = RequestReadState::Unknown;
        m_At = 0;
        m_MidwayParsing = false;
        m_Body.clear();
        m_Join.Free();
        m_Path.Free();
        m_Headers.clear();
        m_Cookies.clear();
    }

    void HttpRequest::Clear(){
        m_Version = HttpVersion::Unsupported;
        m_Verb = HttpVerb::Unknown;
        m_Path.Free();
        m_Headers.clear();
        m_Cookies.clear();
        //m_Body.Free();
        m_Body.clear();
        m_MidwayParsing = false;
    }

    HttpParseErrorCode HttpRequest::ParseHeadCopy(HBuffer&& data, uint32_t* finishedAt)noexcept{
        HBuffer* buff = &m_Join.GetBuffer1();
        buff->Consume(m_At, m_Join.GetBuffer2());
        if(buff->GetSize() > 0)
            buff = &m_Join.GetBuffer2();
        buff->Assign(std::move(data));
        /// TODO: fix potential bugs with reassigning m_At
        if(m_LastState != HttpParseErrorCode::NeedsMoreData)return m_LastState;
        m_At = 0;
        HttpParseErrorCode error = ParseHead(finishedAt);
        m_LastState = error;
        *finishedAt = m_At;
        return error;
    }
    HttpParseErrorCode HttpRequest::ParseNextBodyCopy(HBuffer&& data, HBuffer& output, uint32_t* finishedAt) noexcept{
        HBuffer* buff = &m_Join.GetBuffer1();
        buff->Consume(m_At, m_Join.GetBuffer2());
        if(buff->GetSize() > 0)
            buff = &m_Join.GetBuffer2();
        buff->Assign(std::move(data));
        /// TODO: fix potential bugs with reassigning m_At
        if(m_LastState != HttpParseErrorCode::NeedsMoreData)return m_LastState;
        m_At = 0;

        HttpParseErrorCode error = ParseBody(output, finishedAt);
        m_LastState = error;
        *finishedAt = m_At;
        return error;
    }

    HttpParseErrorCode HttpRequest::ParseHead(uint32_t* finishedAt)noexcept{
        switch(m_Version){
        case HttpVersion::HTTP0_9:
        case HttpVersion::HTTP1_0:
        case HttpVersion::HTTP1_1:{
            /// TODO : headers/cookies
            switch(m_State){
            case ((uint8_t)RequestReadState::HeadersAndCookies):
                //Headers
                while(true){
                    size_t wasAt =m_At;
                    size_t headerEnd =m_At;
                    size_t valueStart=m_At;
                    size_t valueEnd =m_At;

                    //HeaderName
                    while(true){
                        char c = m_Join.Get(headerEnd);
                        if(c == '\0'){
                            m_At = wasAt;
                            return HttpParseErrorCode::NeedsMoreData;
                        }
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
                            m_At = wasAt;
                            return HttpParseErrorCode::NeedsMoreData;
                        }
                        return HttpParseErrorCode::InvalidHeaderSplit;
                    }

                    valueStart = headerEnd + 2;
                    valueEnd = valueStart;

                    //HeaderValue
                    while(true){
                        char c = m_Join.Get(valueEnd);
                        if(c == '\0'){
                            m_At = wasAt;
                            return HttpParseErrorCode::NeedsMoreData;
                        }
                        if(m_Join.StartsWith(valueEnd, "\r\n"))break;
                        if((c < 0x21 || c > 0x7E) && c != ' '){
                            return HttpParseErrorCode::InvalidHeaderValue;
                        }
                        valueEnd++;
                    }

                    size_t headerSize = headerEnd - m_At;
                    char* headerName = new char[headerSize + 1];
                    m_Join.MemcpyTo(headerName, wasAt, headerSize);
                    headerName[headerSize] = '\0';

                    size_t valueLength = valueEnd - valueStart;
                    char* headerValue = new char[valueLength + 1];
                    m_Join.MemcpyTo(headerValue, valueStart, valueLength);
                    headerValue[valueLength] = '\0';

                    if(strcmp(headerName, "Set-Cookie") != 0){
                        SetHeader(std::move(HBuffer(headerName, headerSize, true, true)), HBuffer(headerValue, valueLength, true, true));
                    }else{
                        delete headerName;
                        delete headerValue;
                    }
                    //CORE_DEBUG("Setting header {0}:{1}", headerName, headerValue);
                    //delete headerValue;
                    //delete headerName;
                    m_At = valueEnd + 2;
                    
                    if(m_Join.StartsWith(m_At, "\r\n")){
                        m_At+=2;
                        break;
                    }
                }
                m_State = RequestReadState::DetectBodyType;
                return HttpParseErrorCode::None;
            default:{
                return HttpParseErrorCode::InvalidState;
                break;
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
            m_State = RequestReadState::Body;
            return ParseHead(finishedAt);
        }
    }
    HttpParseErrorCode HttpRequest::ParseBody(HBuffer& output, uint32_t* finishedAt)noexcept{
        switch(m_State){
            case RequestReadState::DetectBodyType:{
                //Get ransfer encoding
                HBuffer* transferEncoding = GetHeader("Transfer-Encoding");
                if(transferEncoding == nullptr || *transferEncoding == "" || *transferEncoding == "identity"){
                    m_State = RequestReadState::IdentityBody;
                }else if(*transferEncoding == "chunked"){
                    m_State = RequestReadState::ChunkedBody;
                }else{
                    return HttpParseErrorCode::UnsupportedTransferEncoding;
                }
                return ParseBody(output, finishedAt);
            }
            case RequestReadState::IdentityBody:{//Get the body from no transfer encoding
                HBuffer* contentLength = GetHeader("Content-Length");

                if(contentLength == nullptr)return HttpParseErrorCode::None;

                size_t size = atoi(contentLength->GetCStr());
                if(m_Join.GetSize() - m_At < size)return HttpParseErrorCode::NeedsMoreData;

                m_Body.emplace_back(std::move(m_Join.SubString(m_At, size)));
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
                        return HttpParseErrorCode::InvalidChunkSize;
                    }
                    bytes <<= 4;
                    bytes+= c;
                }
                
                if(m_At + bytes + 2 < m_Join.GetSize())return HttpParseErrorCode::NeedsMoreData;
                m_Body.emplace_back(std::move(m_Join.SubBuffer(m_At, bytes)));
                m_At += bytes;
                if(m_Join.StartsWith("\r\n") == false)return HttpParseErrorCode::InvalidChunkEnd;
                return ParseBody(output, finishedAt);
            }
        }
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
    void HttpRequest::SetBody(char* data, size_t size, bool canFree, bool canModify) noexcept{
        //m_Body.Assign(data, size, canFree, canModify);
        m_Body.clear();
        m_Body.emplace_back(data, size, canFree, canModify);
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

    void HttpRequest::SetHeader(const char* name, const char* value) noexcept{
        //m_Headers[name].Assign(value, false, false);
        std::vector<HBuffer>& values = m_Headers[name];
        values.clear();
        values.emplace_back(value);
    }
    void HttpRequest::SetHeader(const HBuffer& name, const char* value) noexcept{
        std::vector<HBuffer>& values = m_Headers[name];
        values.clear();
        values.emplace_back(value);
    }
    void HttpRequest::SetHeader(const HBuffer& name, const HBuffer& value) noexcept{
        std::vector<HBuffer>& values = m_Headers[name];
        values.clear();
        values.emplace_back(value);
    }
    void HttpRequest::SetHeader(const HBuffer& name, HBuffer&& value) noexcept{
        std::vector<HBuffer>& values = m_Headers[name];
        values.clear();
        values.emplace_back(std::move(value));
    }
    void HttpRequest::SetHeader(HBuffer&& name, const char* value) noexcept{
        std::vector<HBuffer>& values = m_Headers[std::move(name)];
        values.clear();
        values.emplace_back(value);
    }
    void HttpRequest::SetHeader(HBuffer&& name, const HBuffer& value) noexcept{
        std::vector<HBuffer>& values = m_Headers[std::move(name)];
        values.clear();
        values.emplace_back(value);
    }
    void HttpRequest::SetHeader(HBuffer&& name, HBuffer&& value) noexcept{
        std::vector<HBuffer>& values = m_Headers[std::move(name)];
        values.clear();
        values.emplace_back(std::move(value));
    }
    void HttpRequest::RemoveHeader(const char* header)noexcept{
        m_Headers.erase(header);
    }
    void HttpRequest::RemoveHeader(const HBuffer& header)noexcept{
        m_Headers.erase(header);
    }

    void HttpRequest::SetCookie(const char* name, Cookie& cookie){
        m_Cookies[name] = std::shared_ptr<Cookie>(&cookie);
    }
    void HttpRequest::SetCookie(const char* name, std::shared_ptr<Cookie> cookie){
        m_Cookies[name] = cookie;
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
    HBuffer* HttpRequest::GetHeader(const char* name) noexcept{
        std::vector<HBuffer>& value = m_Headers[name];
        if(value.size() < 1)return nullptr;
        return &value[0];
    }
    HBuffer* HttpRequest::GetHeader(const HBuffer& name) noexcept{
        std::vector<HBuffer>& value = m_Headers[name];
        if(value.size() < 1)return nullptr;
        return &value[0];
    }
    std::shared_ptr<Cookie> HttpRequest::GetCookie(const char* name) noexcept{
        return m_Cookies[name];
    }
    std::shared_ptr<Cookie> HttpRequest::GetCookie(const HBuffer& name) noexcept{
        return m_Cookies[name];
    }

    void HttpRequest::SetVerb(HttpVerb verb)noexcept{
        m_Verb = verb;
    }
    void HttpRequest::PreparePayload(){
        HBuffer* transferEncoding = GetHeader("Transfer-Encoding");

        if(transferEncoding != nullptr && *transferEncoding == "chunked"){
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
        SetHeader("Content-Length", std::move(HBuffer::ToString(totalSize)));
    }
    HBuffer HttpRequest::HeadToBuffer() const noexcept{
        HBuffer buffer;
        buffer.Reserve(HTTP_DEFAULT_HEAD_REQUEST_TO_BUFFER_SIZE);

        /// TODO: Fix potential overflow later in function.
        switch(m_Version){
        case HttpVersion::HTTP1_0:
        case HttpVersion::HTTP1_1:{
            switch(m_Verb){
            case (int)HttpVerb::Get:{
                buffer.Append("GET ", 4);
                break;
            }
            case (int)HttpVerb::Patch:{
                buffer.Append("PATCH ", 6);
                break;
            }
            case (int)HttpVerb::Post:{
                buffer.Append("POST ", 5);
                break;
            }
            case (int)HttpVerb::Head:{
                buffer.Append("HEAD ", 5);
                break;
            }
            case (int)HttpVerb::Delete:{
                buffer.Append("DELETE ", 7);
                break;
            }
            case (int)HttpVerb::Connect:{
                buffer.Append("CONNECT ", 8);
                break;
            }
            case (int)HttpVerb::Trace:{
                buffer.Append("TRACE ",6 );
                break;
            }
            case (int)HttpVerb::Options:{
                buffer.Append("OPTIONS ", 8);
                break;
            }
            default:{
                return buffer;
            }
            }
            if(m_Path.GetSize() > 0)
                buffer.Append(m_Path);
            else
                buffer.Append('/');

            buffer.Append(" HTTP/1.1\r\n", 11);

            //Headers
            for (const auto &myPair : m_Headers) {
                if(myPair.first.GetSize() < 1 || myPair.second.size() < 1)continue;
                buffer.Append(myPair.first.GetCStr());
                buffer.Append(": ", 2);

                const std::vector<HBuffer>& headerValues = myPair.second;
                for(size_t i = 0; i < headerValues.size(); i++){
                    buffer.Append(headerValues[i].GetCStr());
                    buffer.Append("\r\n", 2);
                }
            }

            //Cookies
            for (const auto &pair : m_Cookies) {
                if(pair.first.GetSize() < 1 || pair.second == nullptr)continue;
                buffer.Append(pair.first.GetCStr());
                buffer.Append("= ", 2);
                buffer.Append(pair.second->GetValue());
                buffer.Append("\r\n", 2);
            }
            buffer.Append("\r\n", 2);
            break;
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
            //CORE_ERROR("Http Request doesnt support converting head of type Http{0} to buffer yet.", version);
            buffer.Free();
            return buffer;
        }
        }
        return buffer;
    }

    std::vector<HBuffer> HttpRequest::GetBodyPartsCopy() noexcept{
        std::vector<HBuffer> bodyParts;
        
        HBuffer* transferEncoding = GetHeader("Transfer-Encoding");
        
        if(!transferEncoding || *transferEncoding == "" || *transferEncoding == "identity"){
            for(size_t i = 0; i < m_Body.size(); i++){
                HBuffer part;
                part.Copy(m_Body[i]);
                bodyParts.emplace_back(std::move(part));
            }
        }else if(*transferEncoding == "chunked"){
            for(size_t i = 0; i < m_Body.size(); i++){
                const HBuffer& bodyPart = m_Body[i];

                size_t bodySize = bodyPart.GetSize();
                
                HBuffer string;
                string.Reserve(5);
                
                size_t size = bodySize;
                while(size > 0){
                    char digit = size % 16;
                    string.AppendString(digit >= 10 ? 55 + digit : digit + '0');
                    size/=16;
                }
                
                string.Reverse();

                HBuffer buffer;
                buffer.Reserve(bodySize + 6);

                buffer.Append(string.GetData(), string.GetSize());
                buffer.Append('\r');
                buffer.Append('\n');
                buffer.Append(bodyPart.GetData(), bodySize);

                buffer.Append('\r');
                buffer.Append('\n');
                bodyParts.emplace_back(std::move(buffer));
            }
            bodyParts.emplace_back("0\r\n\r\n", 5, false, false);
        }else{
            //CORE_ERROR("Failed to get body parts copy from unsupported transfer Encoding {0}", transferEncoding.GetCStr());
        }

        //HBuffer = operator creates a copy
        //HBuffer bodyPart = m_Body;
        //bodyParts.emplace_back(std::move(bodyPart));
        return std::move(bodyParts);
    }

    HttpEncodingErrorCode HttpRequest::Decompress() noexcept{
        /// TODO: copy from response
        return HttpEncodingErrorCode::None;
    }

    HttpEncodingErrorCode HttpRequest::Compress() noexcept{
        return HttpEncodingErrorCode::None;
    }
}