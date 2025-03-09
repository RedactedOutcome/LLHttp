#pragma once

#include "Core.h"

#ifdef LLHTTP_USE_PCH
#include LLHTTP_PCH_DIR
#else
#include <HBuffer/HBuffer.hpp>
#include <vector>
#endif

namespace LLHttp{
    class Decoder{
    public:
        /// @return type of HttpEncodingErrorCode
        static int DecodeGZip(HBuffer& input, std::vector<HBuffer>& output);
        /// @return type of HttpEncodingErrorCode
        static int EncodeGZip(HBuffer& input, std::vector<HBuffer>& output);

        /// @brief returns a new buffer that is the decoded form of a percent encoded url. Only contains a subset of percent encoding without control characters.
        /// @return returns a buffer with a size 0 if error or if input is empty.
        static HBuffer GetFromPercentEncoding(const HBuffer& input);

        /// @brief will decode input depending on the encoding and push to an output buffer. If identity encoding then we return;
        /// @param encoding enum type of HttpContentEncoding
        /// @param input the input that gets decoded
        /// @param output the output buffer that we append the decoded data in if any to decode
        /// @return type of HttpEncodingErrorCode
        static int DecodeData(int encoding, HBuffer& input, std::vector<HBuffer>& output) noexcept;
    };
}