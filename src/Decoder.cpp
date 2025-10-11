#include "pch.h"
#include "Decoder.h"
#include "HttpData.h"

namespace LLHttp{
    HttpEncodingErrorCode Decoder::DecodeGZip(HBuffer& data, std::vector<HBuffer>& output){
        z_stream stream;
        memset(&stream, 0, sizeof(z_stream));
        stream.avail_in = static_cast<uInt>(data.GetSize());
        stream.next_in = reinterpret_cast<Bytef*>(data.GetData());

        if (inflateInit2(&stream, 16 + MAX_WBITS) != Z_OK) {
            return HttpEncodingErrorCode::InitializationFailure;
        }

        HBuffer out;
        const size_t capacity = 4096;

        int ret;
        do {
            out.Reserve(capacity);
            stream.avail_out = capacity;
            stream.next_out = reinterpret_cast<Bytef*>(out.GetData());

            ret = inflate(&stream, Z_NO_FLUSH);

            if (ret != Z_OK && ret != Z_STREAM_END) {
                inflateEnd(&stream);
                return HttpEncodingErrorCode::FailedDecodeGZip;
            }

            size_t bytesDecompressed = capacity - stream.avail_out;
            out.SetSize(bytesDecompressed);
            output.emplace_back(std::move(out));
        } while (ret != Z_STREAM_END);

        inflateEnd(&stream);

        return HttpEncodingErrorCode::None;
    }

    HttpEncodingErrorCode Decoder::EncodeGZip(HBuffer& input, std::vector<HBuffer>& output){
        z_stream stream;
        memset(&stream, 0, sizeof(z_stream));

        stream.avail_in = static_cast<uInt>(input.GetSize());
        stream.next_in = reinterpret_cast<Bytef*>(input.GetData());

        // Initialize zlib for GZIP encoding
        if (deflateInit2(&stream, Z_BEST_COMPRESSION, Z_DEFLATED, 16 + MAX_WBITS, 8, Z_DEFAULT_STRATEGY) != Z_OK) {
            return HttpEncodingErrorCode::InitializationFailure;
        }

        // Compress the data
        const size_t capacity = 4096;
        HBuffer out;

        int ret;
        do {
            out.Reserve(capacity);
            stream.avail_out = capacity;
            stream.next_out = reinterpret_cast<Bytef*>(out.GetData());

            ret = deflate(&stream, Z_FINISH);

            if (ret != Z_OK && ret != Z_STREAM_END) {
                deflateEnd(&stream);
                return HttpEncodingErrorCode::FailedEncodeGZip;
            }

            size_t bytesCompressed = capacity - stream.avail_out;
            out.SetSize(bytesCompressed);
            output.emplace_back(std::move(out));
            //compressedData.insert(compressedData.end(), buffer, buffer + bytesCompressed);
        } while (ret != Z_STREAM_END);

        deflateEnd(&stream);

        return HttpEncodingErrorCode::None;
    }
    HttpEncodingErrorCode Decoder::DecodeData(HttpContentEncoding encoding, HBuffer& input, std::vector<HBuffer>& output) noexcept{
        switch(encoding){
        case HttpContentEncoding::GZip:{
            return HttpEncodingErrorCode::UnsupportedContentEncoding;
        }
        case HttpContentEncoding::Identity:
            return HttpEncodingErrorCode::None;
        default:
            return HttpEncodingErrorCode::UnsupportedContentEncoding;
        }
    }


    HttpEncodingErrorCode Decoder::GetFromPercentEncoding(const HBuffer& input, HBuffer& output) noexcept{
        size_t size = input.GetSize();
        output.Reserve(size);

        for(size_t i = 0; i < size; i++){
            char c = input.At(i);

            if(c == '+'){
                output.Append(' ');
                continue;
            }

            if(c != '%'){
                if(::LLHttp::IsValidPathCharacter(c)){
                    output.Append(c);
                    continue;
                }
                return HttpEncodingErrorCode::InvalidPercentEncodingCharacter;
            }

            i++;
            if(i >= size){
                return HttpEncodingErrorCode::NeedsMoreData;
            }
            size_t number = 0;
            c = input.At(i++);

            if(i >= size){
                return HttpEncodingErrorCode::NeedsMoreData;
            }

            if((c < '0' || c > '9') && (c < 'A' || c > 'F')){
                return HttpEncodingErrorCode::IllegalPercentEncodingCharacter;
            }
            number = (c - (c < 65 ? 48 : 55)) * 16;
            c = input.At(i);
            if((c < '0' || c > '9') && (c < 'A' || c > 'F')){
                return HttpEncodingErrorCode::IllegalPercentEncodingCharacter;
            }
            number += (c - (c < 65 ? 48 : 55));

            switch(number){
            case 0x3A:
                output.Append(':');
                break;
            case 0x2F:
                output.Append('/');
                break;
            case 0x3F:
                output.Append('?');
                break;
            case 0x23:
                output.Append('#');
                break;
            case 0x5B:
                output.Append('[');
                break;
            case 0x5D:
                output.Append(']');
                break;
            case 0x40:
                output.Append('@');
                break;
            case 0x21:
                output.Append('!');
                break; 
            case 0x24:
                output.Append('$');
                break;
            case 0x26:
                output.Append('&');
                break;
            case 0x27:
                output.Append("'");
                break;
            case 0x28:
                output.Append('(');
                break;
            case 0x29:
                output.Append(')');
                break;
            case 0x2A:
                output.Append('*');
                break;
            case 0x2B:
                output.Append('+');
                break;
            case 0x2C:
                output.Append(',');
                break;
            case 0x3B:
                output.Append(';');
                break;
            case 0x3D:
                output.Append('=');
                break;
            case 0x25:
                output.Append('%');
                break;
            case 0x22:
                output.Append('"');
                break;
            case 0x20:
                output.Append(' ');
                break;
            case 0x2E:
                output.Append('.');
                break;
            default:
                //CORE_DEBUG("r 5 ({0}) {1}",number, input.SubString(i - 2, 3).GetCStr());
                return HttpEncodingErrorCode::IllegalPercentEncodingOpcode;
            }

        }

        return HttpEncodingErrorCode::None;
    }

    
    HttpEncodingErrorCode Decoder::ToPercentEncoding(const HBuffer& input, HBuffer& output) noexcept {
        size_t size = input.GetSize();
        output.Reserve(size);

        for(size_t i = 0; i < size; i++){
            char c = input.At(i);

            switch(c){
                case ':':
                    output.Append("%3A");
                    break;
                case '/':
                    output.Append("%2F");
                    break;
                case '#':
                    output.Append("%23");
                    break;
                case '?':
                    output.Append("%3F");
                    break;
                case '[':
                    output.Append("%5B");
                    break;
                case ']':
                    output.Append("%5D");
                    break;
                case '@':
                    output.Append("%40");
                    break;
                case '!':
                    output.Append("%21");
                    break;
                case '$':
                    output.Append("%24");
                    break;
                case '&':
                    output.Append("%26");
                    break;
                case '\'':;
                    output.Append("%27");
                    break;
                case '(':
                    output.Append("%28");
                    break;
                case ')':
                    output.Append("%29");
                    break;
                case '*':
                    output.Append("%2A");
                    break;
                case '+':
                    output.Append("%2B");
                    break;
                case ',':
                    output.Append("%2C");
                    break;
                case ';':
                    output.Append("%3B");
                    break;
                case '=':
                    output.Append("%3D");
                    break;
                case '%':
                    output.Append("%25");
                    break;
                case '"':
                    output.Append("%22");
                    break;
                case ' ':
                    output.Append("%20");
                    break;
                case '.':
                    output.Append("%2E");
                    break;
                default:{
                    if(::LLHttp::IsValidPathCharacter(c)){
                        output.Append(c);
                        break;
                    }   
                    return HttpEncodingErrorCode::InvalidPercentEncodingCharacter;
                }
                }
        }

        return HttpEncodingErrorCode::None;
    }

    void Decoder::ConvertToChunkedEncoding(const HBuffer& input, HBuffer& output)noexcept{
        size_t partSize = input.GetSize();

        HBuffer string;
        string.Reserve(5);

        size_t size = partSize;
        while(size > 0){
            char digit = size % 16;
            string.AppendString(digit >= 10 ? (55 + digit) : (digit + '0'));
            size/=16;
        }

        string.Reverse();
        output.SetSize(0);

        output.Append(string.GetData(), string.GetSize());
        output.Append('\r');
        output.Append('\n');
        output.Append(input.GetData(), partSize);

        output.Append('\r');
        output.Append('\n');
        output = std::move(output);
    }

    static HttpParseErrorCode Decoder::GetEncodingFromString(const HBuffer& input, HttpContentEncoding& output)noexcept{
        HBuffer encoding = input.SubPointer(lastAt, at - lastAt);
        if(encoding.Get(0) == ' ')encoding = encoding.SubPointer(1, -1);

        if(encoding == "identity"){      
            output = HttpContentEncoding::Identity;
            return HttpParseErrorCode::None;
        }
        else if(encoding == "gzip"){     
            output = HttpContentEncoding::GZip;
            return HttpParseErrorCode::None;
        }
        else if(encoding == "deflate"){  
            output = HttpContentEncoding::Deflate;
            return HttpParseErrorCode::None;
        }
        else if(encoding == "br"){       
            output = HttpContentEncoding::Brotli;
            return HttpParseErrorCode::None;
        }
        else if(encoding == "zstd"){     
            output = HttpContentEncoding::ZStd;
            return HttpParseErrorCode::None;
        }
        else if(encoding == "dcb"){      
            output = HttpContentEncoding::DCB;
            return HttpParseErrorCode::None;
        }
        else if(encoding == "dcz"){
            output = HttpContentEncoding::DCZ;
            return HttpParseErrorCode::None;
        }
        return HttpParseErrorCode::UnsupportedContentEncoding;
    }
    static HttpParseErrorCode Decoder::GetDecodingOrder(const HBuffer& input, std::vector<HttpContentEncoding>& output)noexcept{
        std::vector<HttpContentEncoding> encodings;
        encodings.reserve(3);

        size_t lastAt = 0;
        size_t i;
        for(i = 0; i < input.GetSize(); i++){
            char c = input.At(i);
            if(c == ','){
                HBuffer buffer = input.SubPointer(lastAt, at - lastAt);
                if(buffer.Get(0) == ' ')buffer = buffer.SubPointer(1, -1);

                HttpContentEncoding encoding;
                HttpParseErrorCode errorCode = GetEncodingFromString(buffer, encoding);
                if(errorCode != HttpParseErrorCode::None)return errorCode;
                encodings.emplace_back(encoding);
                lastAt = i + 1;
            }
        }
        if(lastAt < i){
            HBuffer buffer = input.SubPointer(lastAt, at - lastAt);
            if(buffer.Get(0) == ' ')buffer = buffer.SubPointer(1, -1);

            HttpContentEncoding encoding;
            HttpParseErrorCode errorCode = GetEncodingFromString(buffer, encoding);
            if(errorCode != HttpParseErrorCode::None)return errorCode;
            encodings.emplace_back(encoding);
        }
        
        output.reserve(encodings);
        for(size_t i = encodings.size(); i > 0; --i){
            output.emplace_back(encodings[i - 1]);
        }
    }
}