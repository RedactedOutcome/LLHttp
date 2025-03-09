#include "LLHttp/pch.h"
#include "Decoder.h"
#include "HttpData.h"

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


HBuffer Decoder::GetFromPercentEncoding(const HBuffer& input){
    HBuffer output;
    size_t size = input.GetSize();
    output.Reserve(size);

    for(size_t i = 0; i < size; i++){
        char c = input.At(i);

        if((c >= 'a' && c<= 'z') || (c >= 'A' && c <= 'Z') || c == '-' || c == '_'){
            output.Append(c);
            continue;
        }

        if(c == '+'){
            output.Append(' ');
            continue;
        }

        if(c != '%'){
            output.Free();
            return output;
        }

        i++;
        if(i >= size){
            output.Free();
            return output;
        }
        size_t number = 0;
        c = input.At(i++);

        if(i >= size){
            output.Free();
            return output;
        }

        if((c < '0' || c > '9') && (c < 'A' || c > 'F')){
            //CORE_DEBUG("r 3 ({0})", (size_t)c);
            output.Free();
            return output;
        }
        number = (c - (c < 65 ? 48 : 55)) * 16;
        c = input.At(i);
        if((c < '0' || c > '9') && (c < 'A' || c > 'F')){
            output.Free();
            return output;
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
            output.Append('#');
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
        case 0x20:
            output.Append(' ');
            break;
        case 0x2E:
            output.Append('.');
            break;
        default:
            //CORE_DEBUG("r 5 ({0}) {1}",number, input.SubString(i - 2, 3).GetCStr());
            output.Free();
            return output;
        }

    }

    return output;
}