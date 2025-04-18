#include "LLHttp/pch.h"
#include "Decoder.h"
#include "HttpData.h"

namespace LLHttp{
    int Decoder::DecodeGZip(HBuffer& data, std::vector<HBuffer>& output){
        z_stream stream;
        memset(&stream, 0, sizeof(z_stream));
        stream.avail_in = static_cast<uInt>(data.GetSize());
        stream.next_in = reinterpret_cast<Bytef*>(data.GetData());

        if (inflateInit2(&stream, 16 + MAX_WBITS) != Z_OK) {
            //CORE_ERROR("Failed to initialize zlib for GZIP decoding");
            return (int)HttpEncodingErrorCode::InitializationFailure;
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
                return (int)HttpParseErrorCode::FailedDecodeGZip;
            }

            size_t bytesDecompressed = capacity - stream.avail_out;
            out.SetSize(bytesDecompressed);
            output.emplace_back(std::move(out));
        } while (ret != Z_STREAM_END);

        inflateEnd(&stream);

        return 0;
    }

    int Decoder::EncodeGZip(HBuffer& input, std::vector<HBuffer>& output){
        z_stream stream;
        memset(&stream, 0, sizeof(z_stream));

        stream.avail_in = static_cast<uInt>(input.GetSize());
        stream.next_in = reinterpret_cast<Bytef*>(input.GetData());

        // Initialize zlib for GZIP encoding
        if (deflateInit2(&stream, Z_BEST_COMPRESSION, Z_DEFLATED, 16 + MAX_WBITS, 8, Z_DEFAULT_STRATEGY) != Z_OK) {
            return -1;
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
                return (int)HttpParseErrorCode::FailedEncodeGZip;
            }

            size_t bytesCompressed = capacity - stream.avail_out;
            out.SetSize(bytesCompressed);
            output.emplace_back(std::move(out));
            //compressedData.insert(compressedData.end(), buffer, buffer + bytesCompressed);
        } while (ret != Z_STREAM_END);

        deflateEnd(&stream);

        return 0;
    }
    int Decoder::DecodeData(int encoding, HBuffer& input, std::vector<HBuffer>& output) noexcept{
        switch(encoding){
        case (int)HttpContentEncoding::GZip:{
            return (int)HttpEncodingErrorCode::UnsupportedContentEncoding;
        }
        case (int)HttpContentEncoding::Identity:
            return (int)HttpEncodingErrorCode::Success;
        default:
            return (int)HttpEncodingErrorCode::UnsupportedContentEncoding;
        }
    }


    int Decoder::GetFromPercentEncoding(const HBuffer& input, HBuffer& output) noexcept{
        size_t size = input.GetSize();
        output.Reserve(size);

        for(size_t i = 0; i < size; i++){
            char c = input.At(i);

            if((c >= 'a' && c<= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '-' || c == '_' || c == '.'){
                output.Append(c);
                continue;
            }

            if(c == '+'){
                output.Append(' ');
                continue;
            }

            if(c != '%'){
                return (int)HttpEncodingErrorCode::IllegalPercentEncodingDelimiter;
            }

            i++;
            if(i >= size){
                return (int)HttpEncodingErrorCode::NeedsMoreData;
            }
            size_t number = 0;
            c = input.At(i++);

            if(i >= size){
                return (int)HttpEncodingErrorCode::NeedsMoreData;
            }

            if((c < '0' || c > '9') && (c < 'A' || c > 'F')){
                return (int)HttpEncodingErrorCode::IllegalPercentEncodingCharacter;
            }
            number = (c - (c < 65 ? 48 : 55)) * 16;
            c = input.At(i);
            if((c < '0' || c > '9') && (c < 'A' || c > 'F')){
                return (int)HttpEncodingErrorCode::IllegalPercentEncodingCharacter;
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
                return (int)HttpEncodingErrorCode::IllegalPercentEncodingOpcode;
            }

        }

        return (int)HttpEncodingErrorCode::Success;
    }

    
    int Decoder::ToPercentEncoding(const HBuffer& input, HBuffer& output) noexcept {
        size_t size = input.GetSize();
        output.Reserve(size);

        for(size_t i = 0; i < size; i++){
            char c = input.At(i);

            if((c >= 'a' && c<= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '-' || c == '_' || c == '.'){
                output.Append(c);
                continue;
            }
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
                default:
                    return (int)HttpEncodingErrorCode::InvalidPercentEncodingCharacter;
                }
        }

        return (int)HttpEncodingErrorCode::Success;
    }
    
#pragma region ConvertingBuffers
    HBuffer Decoder::ConvertToChunkedEncoding(const HBuffer& input) noexcept{
        HBuffer output;
        size_t partSize = input.GetSize();

        HBuffer string;
        string.Reserve(5);
        string.SetSize(0);

        size_t size = partSize;
        std::cout << "Size " << size<<std::endl;
        do{
            char digit = size % 16;
            char c = digit >= 10 ? (55 + digit) : (digit + '0');
            string.AppendString(c);
            size/=16;
        }while(size > 0);
        string.Reverse();
        std::cout << "Part size : " << partSize << " hex : " << string.GetCStr()<<std::endl;

        output.Reserve(partSize + 6 + input.GetSize());

        output.Append(string);
        output.Append("\r\n", 2);
        output.Append(input.GetData(), partSize);
        output.Append("\r\n", 2);
        return output;
    }
#pragma endregion
}