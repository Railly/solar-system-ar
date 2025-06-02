#pragma once
#include <cstdio>
#include <ctime>

#ifndef LOG_LEVEL
#define LOG_LEVEL 2            // 0-none 1-error 2-info 3-debug
#endif

inline const char* _ts() {
    static char buf[20];
    std::time_t t = std::time(nullptr);
    std::strftime(buf, sizeof(buf), "%H:%M:%S", std::localtime(&t));
    return buf;
}

#define LOG_ERR(fmt, ...)   do{ if (LOG_LEVEL>=1) std::fprintf(stderr,"[%s][ERR] " fmt "\n", _ts(), ##__VA_ARGS__);}while(0)
#define LOG_INF(fmt, ...)   do{ if (LOG_LEVEL>=2) std::printf ("[%s][INF] " fmt "\n", _ts(), ##__VA_ARGS__);}while(0)
#define LOG_DBG(fmt, ...)   do{ if (LOG_LEVEL>=3) std::printf ("[%s][DBG] " fmt "\n", _ts(), ##__VA_ARGS__);}while(0) 