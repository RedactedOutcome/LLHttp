#pragma once

#include "pch.h"

namespace LLHttp{
    class BrotliDecoder{
    public:
        BrotliDecoder()noexcept;
        ~BrotliDecoder()noexcept;

    private:
        BrotliDecoderState* m_State = nullptr;
    };
}