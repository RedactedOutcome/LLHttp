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
        s_LLHttpPathAllowedCharacters.set('/');
        s_LLHttpPathAllowedCharacters.set('?');
        s_LLHttpPathAllowedCharacters.set('@');
        s_LLHttpPathAllowedCharacters.set('[');
        s_LLHttpPathAllowedCharacters.set(']');
        s_LLHttpPathAllowedCharacters.set('.');
        s_LLHttpPathAllowedCharacters.set('-');
    }
    bool IsValidPathCharacter(char c){
        return s_LLHttpPathAllowedCharacters.test(c);
    }
}