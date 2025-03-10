#include "LLHttp/pch.h"
#include "LLHttp/LLHttp.h"

namespace LLHttp{
    std::bitset<128> LLHttp::s_PathAllowedCharacters;

    void LLHttp::Init() noexcept{
        for(size_t c = 'a'; c <= 'z'; c++)s_PathAllowedCharacters.set(c);
        for(size_t c = 'A'; c <= 'Z'; c++)s_PathAllowedCharacters.set(c);
        for(size_t c = '0'; c <= '9'; c++)s_PathAllowedCharacters.set(c);
        for(size_t c = '&'; c <= ','; c++)s_PathAllowedCharacters.set(c);
        for(size_t c = ':'; c <= '='; c++)s_PathAllowedCharacters.set(c);
        s_PathAllowedCharacters.set('!');
        s_PathAllowedCharacters.set('!');
        s_PathAllowedCharacters.set('#');
        s_PathAllowedCharacters.set('$');
        s_PathAllowedCharacters.set('/');
        s_PathAllowedCharacters.set('?');
        s_PathAllowedCharacters.set('@');
        s_PathAllowedCharacters.set('[');
        s_PathAllowedCharacters.set(']');
    }
    bool LLHttp::IsValidPathCharacter(char c){
        return s_PathAllowedCharacters.test(c);
    }
}