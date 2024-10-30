#pragma once

#ifndef TABSPLSLOG_FUNC_INFO
#error "TABSPLSLOG_FUNC_INFO is not defined, log statements won't contain the relevant function name/info."
#endif

namespace TabsPlsLog {
void Init();
void Debug(const char* fileName, int lineno, const char* function, const char* category, const char* fmt, ...);
void Warning(const char* fileName, int lineno, const char* function, const char* category, const char* fmt, ...);
void Error(const char* fileName, int lineno, const char* function, const char* category, const char* fmt, ...);
} // namespace TabsPlsLog

// Log something that is supposed to be helpful for developers to fix the problem
#define TabsPlsLog_Debug(...) (TabsPlsLog::Debug(__FILE__, __LINE__, TABSPLSLOG_FUNC_INFO, nullptr, __VA_ARGS__))
#define TabsPlsLog_DebugCategory(category, ...)                                                                        \
    (TabsPlsLog::Debug(__FILE__, __LINE__, TABSPLSLOG_FUNC_INFO, category, __VA_ARGS__))

// Log something that might be useful to the user to solve their problem
#define TabsPlsLog_Warning(...) (TabsPlsLog::Warning(__FILE__, __LINE__, TABSPLSLOG_FUNC_INFO, nullptr, __VA_ARGS__))
#define TabsPlsLog_WarningCategory(category, ...)                                                                      \
    (TabsPlsLog::Warning(__FILE__, __LINE__, TABSPLSLOG_FUNC_INFO, category, __VA_ARGS__))

// Report an error, program execution can still go on
#define TabsPlsLog_Error(...) (TabsPlsLog::Error(__FILE__, __LINE__, TABSPLSLOG_FUNC_INFO, nullptr, __VA_ARGS__))
#define TabsPlsLog_ErrorCategory(category, ...)                                                                        \
    (TabsPlsLog::Error(__FILE__, __LINE__, TABSPLSLOG_FUNC_INFO, category, __VA_ARGS__))
