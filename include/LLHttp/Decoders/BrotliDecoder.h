#pragma once

#include "pch.h"

namespace LLHttp{
    class BrotliDecoder{
    public:
        BrotliDecoder()noexcept;
        ~BrotliDecoder()noexcept;
        
    private:
        BortliDecoderState* m_State = nullptr;
    };
}