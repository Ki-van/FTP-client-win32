#pragma once
#include <winuser.h>
#include <debugapi.h>

inline void logA(const char* format, ...)
{
    char buf[1024];
    wvsprintfA(buf, format, ((char*)&format) + sizeof(void*)%1024);
    OutputDebugStringA(buf);
}

#define DEBUG_LOG_A(format, ...) logA( \
        "(#" BOOST_PP_STRINGIZE( __LINE__ ) ") "__FUNCTION__ " : " \
        format, __VA_ARGS__)