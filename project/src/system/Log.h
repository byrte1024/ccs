//Log.h
#pragma once

#include <raylib.h>

#include "macro.h"

/*
LOG_ALL = 0,        // Display all logs
LOG_TRACE,          // Trace logging, intended for internal use only
LOG_DEBUG,          // Debug logging, used for internal debugging, it should be disabled on release builds
LOG_INFO,           // Info logging, used for program execution info
LOG_WARNING,        // Warning logging, used on recoverable failures
LOG_ERROR,          // Error logging, used on unrecoverable failures
LOG_FATAL,          // Fatal logging, used to abort program: exit(EXIT_FAILURE)
LOG_NONE            // Disable logging
*/

#ifdef VERBOSE
#define FLogDebug(   format, ...) TraceLog(LOG_DEBUG,   "  ... - - -> [%s:%d] " EP1(format), __FILE_NAME__, __LINE__, EP(__VA_ARGS__))
#define FLogInfo(    format, ...) TraceLog(LOG_INFO,    "   ... - - -> [%s:%d] " EP1(format), __FILE_NAME__, __LINE__, EP(__VA_ARGS__))
#define FLogWarning( format, ...) TraceLog(LOG_WARNING, "??? -----> [%s:%d] " EP1(format), __FILE_NAME__, __LINE__, EP(__VA_ARGS__))
#define FLogError(   format, ...) TraceLog(LOG_ERROR,   "  !!! -----> [%s:%d] " EP1(format), __FILE_NAME__, __LINE__, EP(__VA_ARGS__))
#define FLogFatal(   format, ...) TraceLog(LOG_FATAL,   "  !!! =====> [%s:%d] " EP1(format), __FILE_NAME__, __LINE__, EP(__VA_ARGS__))
#define LogDebug(    format)      TraceLog(LOG_DEBUG,   "  ... - - -> [%s:%d] " EP1(format), __FILE_NAME__, __LINE__)
#define LogInfo(     format)      TraceLog(LOG_INFO,    "   ... - - -> [%s:%d] " EP1(format), __FILE_NAME__, __LINE__)
#define LogWarning(  format)      TraceLog(LOG_WARNING, "??? -----> [%s:%d] " EP1(format), __FILE_NAME__, __LINE__)
#define LogError(    format)      TraceLog(LOG_ERROR,   "  !!! -----> [%s:%d] " EP1(format), __FILE_NAME__, __LINE__)
#define LogFatal(    format)      TraceLog(LOG_FATAL,   "  !!! =====> [%s:%d] " EP1(format), __FILE_NAME__, __LINE__)
#else
#define FLogDebug(   format, ...) TraceLog(LOG_DEBUG,   "  ... - - -> " EP1(format), EP(__VA_ARGS__))
#define FLogInfo(    format, ...) TraceLog(LOG_INFO,    "   ... - - -> " EP1(format), EP(__VA_ARGS__))
#define FLogWarning( format, ...) TraceLog(LOG_WARNING, "??? -----> " EP1(format), EP(__VA_ARGS__))
#define FLogError(   format, ...) TraceLog(LOG_ERROR,   "  !!! -----> " EP1(format), EP(__VA_ARGS__))
#define FLogFatal(   format, ...) TraceLog(LOG_FATAL,   "  !!! =====> " EP1(format), EP(__VA_ARGS__))
#define LogDebug(    format)      TraceLog(LOG_DEBUG,   "  ... - - -> " EP1(format))
#define LogInfo(     format)      TraceLog(LOG_INFO,    "   ... - - -> " EP1(format))
#define LogWarning(  format)      TraceLog(LOG_WARNING, "??? -----> " EP1(format))
#define LogError(    format)      TraceLog(LOG_ERROR,   "  !!! -----> " EP1(format))
#define LogFatal(    format)      TraceLog(LOG_FATAL,   "  !!! =====> " EP1(format))
#endif

