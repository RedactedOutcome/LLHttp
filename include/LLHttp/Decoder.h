#pragma once

#include "Core.h"
#include "pch.h"

#include "LLHttp.h"

namespace LLHttp{
    struct AcceptEncoding{
        HttpContentEncoding m_Encoding = HttpContentEncoding::Unsupported;
        float m_Priority = 1.0f;
    };
    class Decoder{
    public:
        /// @return type of HttpEncodingErrorCode
        static HttpEncodingErrorCode DecodeGZip(HBuffer& input, std::vector<HBuffer>& output);
        /// @return type of HttpEncodingErrorCode
        static HttpEncodingErrorCode EncodeGZip(HBuffer& input, std::vector<HBuffer>& output);

        static HttpEncodingErrorCode DecodeBrotli(const HBuffer& input, std::vector<HBuffer>& output)noexcept;
        
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

        static HttpParseErrorCode GetEncodingFromString(const HBuffer& input, HttpContentEncoding& output)noexcept;
        /// @brief Takes in a encoding or list of content-encodings and returns the order in which to decode it.
        /// @param input the Content-Encoding header value string. eg Content-Encoding:identity. ContentEncoding: gzip, br.
        /// @param output the order of encodings to decode with
        /// @return 
        static HttpParseErrorCode GetDecodingOrder(const HBuffer& input, std::vector<HttpContentEncoding>& output)noexcept;

        /// @brief Takes in a encoding or list of content-encodings and returns the order in which to encode it.
        /// @param input the Accept-Encoding header value string
        /// @param output the order of encodings to encode with
        /// @return 
        static HttpParseErrorCode GetEncodingOrder(const HBuffer& input, std::vector<AcceptEncoding>& output)noexcept;
    };
}