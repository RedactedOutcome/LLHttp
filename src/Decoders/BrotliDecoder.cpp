#include "pch.h"

namespace LLHttp{
    BrotliDecoder::BrotliDecoder()noexcept:
    m_State(BrotliDecoderCreateInstance(nullptr, nullptr, nullptr)){

    }
    BrotliDecoder::~BrotliDecoder()noexcept{
        BrotliDecoderDestroyInstance(m_State);
    }

    void BrotliDecoder::Reset()noexcept{
        m_Input.clear();
        BrotliDecoderReset(m_State);
    }
}