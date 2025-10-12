#pragma once

#include "pch.h"
#include "../HttpData.h"

namespace LLHttp{
    class BrotliEncoder{
    public:
        BrotliEncoder()noexcept;
        ~BrotliEncoder()noexcept;

        void Reset()noexcept;
        
        HttpEncodingErrorCode EncodeBrotli(HBuffer&& input, HBuffer& output)noexcept{
            m_Input.emplace_back(std::move(data));

            constexpr int outputSize = 3200;
            uint8_t output[outputSize];

            auto lastChunk = m_Input.end() - 1;
            for(auto it = m_Input.begin(); it != m_Input.end(); it++){
                const HBuffer& input = *it;

                const uint8_t* inputData = reinterpret_cast<const uint8_t*>(input.GetData());
                size_t availableInput = input.GetSize();

                size_t availableOut = outputSize;
                uint8_t* outputBuffer = output;
                size_t totalOut;

                while(availableInput > 0){
                    BrotliEncoderOperation op = (it == lastChunk) ? BROTLI_OPERATION_FINISH : BROTLI_OPERATION_PROCESS;
                    if(BrotliEncoderCompressStream(m_State, op, BROTLI_OPERATION_PROCESS, &availableInput, &inputData, &availableOut, &outputBuffer, nullptr) == BROTLI_FALSE){
                        BrotliEncoderDestroyInstance(m_State);
                        m_State = nullptr;
                    }
                    size_t produced = outputSize - availableOut;
                    HBuffer outputData = HBuffer(reinterpret_cast<char*>(output), produced, false, false).SubBuffer(0,-1);
                    outputVector.emplace_back(std::move(outputData));
                    outputBuffer = output;
                    availableOut = outputSize;
                }
                /*
                size_t produced = outputSize - availableOut;
                if(produced == 0)continue;

                HBuffer outputData = HBuffer(reinterpret_cast<char*>(output), produced, false, false).SubBuffer(0,-1);
                outputVector.emplace_back(std::move(outputData));
                */
            }
            m_Input.clear();
            return HttpEncodingErrorCode::None;
        }
    private:
        BrotliEncoderState* m_State = nullptr;
        std::vector<HBuffer> m_Input;
    };
}