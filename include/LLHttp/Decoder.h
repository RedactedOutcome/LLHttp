#pragma once

#include "Core.h"

class Decoder{
public:
    static int DecodeGZip(HBuffer& input, std::vector<HBuffer>& output);
    static int EncodeGZip(HBuffer& input, std::vector<HBuffer>& output);

    /// @brief will decode input depending on the encoding and push to an output buffer. If identity encoding then we return;
    /// @param encoding enum type of HttpContentEncoding
    /// @param input the input that gets decoded
    /// @param output the output buffer that we append the decoded data in if any to decode
    /// @return type of HttpEncodingErrorCode
    static int DecodeData(int encoding, HBuffer& input, std::vector<HBuffer>& output) noexcept;
};