#pragma once

#ifndef HTTP_DEFAULT_HEAD_REQUEST_TO_BUFFER_SIZE
#define HTTP_DEFAULT_HEAD_REQUEST_TO_BUFFER_SIZE 32000
#endif

#ifndef HTTP_DEFAULT_HEAD_RESPONSE_TO_BUFFER_SIZE
#define HTTP_DEFAULT_HEAD_RESPONSE_TO_BUFFER_SIZE 32000
#endif

/*
#if defined(_MSC_VER) && (_MSC_VER >= 1910)
    // MSVC 2017 or later (1910 or greater)
    #if __cplusplus < 201703L
        #error "LLHttp requires at least C++17"
    #endif
#else
    #if __cplusplus < 201703L
    #error "LLHttp requires atleast c++17"
#endif
#endif
*/

struct BodyParseInfo{
    /// @brief a bool that is determined on whether the body parts for identity is any data sent until the stream is ended
    /// @brief Http specification says that if content-length isnt specified any data passed the head phase is the body until the stream is ended
    /// @brief Set by response parser and should only be used in a response parsing context.
    /// TODO: might just remove and replace with error code NeedsMoreData
    bool m_IdentityEndsByStream=false;
    uint32_t m_FinishedAt=0;
    bool m_ValidBody=false; 
    bool m_CopyNecessary=true;
};