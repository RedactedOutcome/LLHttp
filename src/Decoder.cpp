#include "Decoder.h"
#include "HttpData.h"

int Decoder::DecodeGZip(HBuffer& data, std::vector<HBuffer>& output){
    z_stream stream;
    memset(&stream, 0, sizeof(z_stream));
    stream.avail_in = static_cast<uInt>(data.GetSize());
    stream.next_in = reinterpret_cast<Bytef*>(data.GetData());

    if (inflateInit2(&stream, 16 + MAX_WBITS) != Z_OK) {
        CORE_ERROR("Failed to initialize zlib for GZIP decoding");
        return -1;
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