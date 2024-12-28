#include "log.h"
/* LOGGER_MODULE - you can copy this block to be used in single header / standalone files - psyorange
 *	#include <stdio.h>
 *  #include <stdarg.h>
 *  #define LOG_QUIET
 *  #define LOG_ERROR_ONLY
 * */

void logp(const char* fmt, ...){
#if !defined (LOG_QUIET) && !defined(LOG_ERROR_ONLY)
	va_list args;
	va_start(args, fmt); vfprintf(stderr,fmt, args); va_end(args);
#endif
}
void logd(const char* fmt, ...){
#if !defined (LOG_QUIET) && !defined(LOG_ERROR_ONLY)
	fprintf(stderr, "[DEBUG] ");
	va_list args;
	va_start(args, fmt); vfprintf(stderr,fmt, args); va_end(args);
	fprintf(stderr,"\n");
#endif
}
void logi(const char* fmt, ...){
#if !defined (LOG_QUIET) && !defined(LOG_ERROR_ONLY)
    fprintf(stderr, "\x1b[33m[INFO] \x1b[0m");  
	va_list args;
	va_start(args, fmt); vfprintf(stderr,fmt, args); va_end(args);
	fprintf(stderr,"\n");
#endif
}
void loge(const char* fmt, ...){
#if !defined(LOG_QUIET)
    fprintf(stderr, "\x1b[31m[ERROR] \x1b[0m");  
	va_list args;
	va_start(args, fmt); vfprintf(stderr,fmt, args); va_end(args);
	fprintf(stderr,"\n");
#endif
}
/* LOGGER_MODULE END */
