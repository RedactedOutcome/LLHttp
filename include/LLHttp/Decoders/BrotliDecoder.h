#pragma once

#include "pch.h"
#include "../HttpData.h"

namespace LLHttp{
    class BrotliDecoder{
    public:
        BrotliDecoder()noexcept;
        ~BrotliDecoder()noexcept;

        /// @brief Resets the entire state of the decoder
        void Reset()noexcept;

        /// @brief Incrementally Parses Brotli data.
        /// @tparam Alloc 
        /// @param data 
        /// @param output 
        /// @return 
        template <typename Alloc=std::allocator<HBuffer>>
        HttpEncodingErrorCode DecodeBrotli(HBuffer&& data, std::vector<HBuffer, Alloc>& outputVector)noexcept{
            m_Input.emplace_back(std::move(data));

            constexpr int outputSize = 3200;
            uint8_t output[outputSize];
            
            for(auto it = m_Input.begin(); it != m_Input.end(); it++){
                const HBuffer& input = *it;

                const uint8_t* inputData = reinterpret_cast<const uint8_t*>(input.GetData());
                size_t availableInput = input.GetSize();

                size_t availableOut = outputSize;
                uint8_t* outputBuffer = output;
                size_t totalOut;

                while(availableInput > 0){
                    BrotliDecoderResult result = BrotliDecoderDecompressStream(m_State, &availableInput, &inputData, &availableOut, &outputBuffer, nullptr);

                    if(result == BROTLI_DECODER_RESULT_ERROR)return HttpEncodingErrorCode::FailedDecodeBrotli;

                    if(result == BROTLI_DECODER_RESULT_NEEDS_MORE_OUTPUT){
                        size_t produced = outputSize - availableOut;
                        HBuffer outputData = HBuffer(reinterpret_cast<char*>(output), produced, false, false).SubBuffer(0,-1);
                        outputVector.emplace_back(std::move(outputData));
                        outputBuffer = output;
                        availableOut = outputSize;
                        continue;
                    }
                    if(result == BROTLI_DECODER_RESULT_NEEDS_MORE_INPUT){
                        break;
                    }
                    if(result == BROTLI_DECODER_RESULT_SUCCESS){
                        it = m_Input.erase(m_Input.begin(), it+1);
                        break;
                    }
                }
                size_t produced = outputSize - availableOut;
                if(produced == 0)continue;

                HBuffer outputData = HBuffer(reinterpret_cast<char*>(output), produced, false, false).SubBuffer(0,-1);
                outputVector.emplace_back(std::move(outputData));
            }
            m_Input.clear();
            return HttpEncodingErrorCode::None;
        }
    private:
        BrotliDecoderState* m_State = nullptr;
        std::vector<HBuffer> m_Input;
    };
}