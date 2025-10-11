#include "pch.h"
#include "Encoders/BrotliEncoder.h"

namespace LLHttp{
    BrotliEncoder::BrotliEncoder()noexcept:
        m_State(BrotliEncoderCreateInstance(nullptr,nullptr,nullptr)){}
    BrotliEncoder::~BrotliEncoder()noexcept{
        BrotliEncoderDestroyInstance(m_State);
    }

    void BrotliEncoder::Reset()noexcept{
        if(m_State)BrotliEncoderDestroyInstance(m_State);
        m_State = BrotliEncoderCreateInstance(nullptr,nullptr,nullptr);
    }

    
}