#ifndef LOG_H
#define LOG_H
#include <stdarg.h>
void logp(const char* fmt, ...);
void logd(const char* fmt, ...);
void logi(const char* fmt, ...);
void logw(const char* fmt, ...);
void loge(const char* fmt, ...);
#endif 
