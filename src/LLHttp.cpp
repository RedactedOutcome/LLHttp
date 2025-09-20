#ifdef LLHTTP_USE_PCH
#include LLHTTP_PCH_DIR
#else
#include "LLHttp/pch.h"
#endif
#include "LLHttp/LLHttp.h"

namespace LLHttp{
    void InitLLHttp() noexcept{
        for(size_t c = 'a'; c <= 'z'; c++)s_LLHttpPathAllowedCharacters.set(c);
        for(size_t c = 'A'; c <= 'Z'; c++)s_LLHttpPathAllowedCharacters.set(c);
        for(size_t c = '0'; c <= '9'; c++)s_LLHttpPathAllowedCharacters.set(c);
        for(size_t c = '&'; c <= ','; c++)s_LLHttpPathAllowedCharacters.set(c);
        for(size_t c = ':'; c <= '='; c++)s_LLHttpPathAllowedCharacters.set(c);
        s_LLHttpPathAllowedCharacters.set('!');
        s_LLHttpPathAllowedCharacters.set('!');
        s_LLHttpPathAllowedCharacters.set('#');
        s_LLHttpPathAllowedCharacters.set('$');
        s_LLHttpPathAllowedCharacters.set('%');
        s_LLHttpPathAllowedCharacters.set('/');
        s_LLHttpPathAllowedCharacters.set('?');
        s_LLHttpPathAllowedCharacters.set('@');
        s_LLHttpPathAllowedCharacters.set('[');
        s_LLHttpPathAllowedCharacters.set(']');
        s_LLHttpPathAllowedCharacters.set('.');
        s_LLHttpPathAllowedCharacters.set('-');
        s_LLHttpPathAllowedCharacters.set('_');

        for(size_t i = 0x21; i <= 0x7E; i++){
            s_LLHttpHeaderValueAllowedCharacters.set(i);
        }
        s_LLHttpHeaderValueAllowedCharacters.set(' ');
    }
    bool IsValidPathCharacter(char c)noexcept{
        return s_LLHttpPathAllowedCharacters.test(c);
    }

    bool IsValidHeaderValueCharacter(char c)noexcept{
        return s_LLHttpHeaderValueAllowedCharacters.test(c);
    }
}