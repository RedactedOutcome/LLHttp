#pragma once

#include "Core.h"

#ifdef LLHTTP_USE_PCH
#include LLHTTP_PCH_DIR
#else
#include "LLHttp/pch.h"
#endif

#include "LLHttp.h"
namespace LLHttp{
    class Decoder{
    public:
        /// @return type of HttpEncodingErrorCode
        static HttpEncodingErrorCode DecodeGZip(HBuffer& input, std::vector<HBuffer>& output);
        /// @return type of HttpEncodingErrorCode
        static HttpEncodingErrorCode EncodeGZip(HBuffer& input, std::vector<HBuffer>& output);

        /// @brief returns a new buffer that is the decoded form of a percent encoded url. Only contains a subset of percent encoding without control characters.
        /// @return type of HttpEncodingErrorCode
        static HttpEncodingErrorCode GetFromPercentEncoding(const HBuffer& input, HBuffer& output) noexcept;

        /// @brief returns a new buffer that is the encoded form of a url. Only contains a subset of percent encoding without control characters.
        /// @return type of HttpEncodingErrorCode
        static HttpEncodingErrorCode ToPercentEncoding(const HBuffer& input, HBuffer& output) noexcept;

        /// @brief will decode input depending on the encoding and push to an output buffer. If identity encoding then we return;
        /// @param encoding enum type of HttpContentEncoding
        /// @param input the input that gets decoded
        /// @param output the output buffer that we append the decoded data in if any to decode
        /// @return type of HttpEncodingErrorCode
        static HttpEncodingErrorCode DecodeData(HttpContentEncoding encoding, HBuffer& input, std::vector<HBuffer>& output) noexcept;

        static void ConvertToChunkedEncoding(const HBuffer&, HBuffer& output)noexcept;

    };
}