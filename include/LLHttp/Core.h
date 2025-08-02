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