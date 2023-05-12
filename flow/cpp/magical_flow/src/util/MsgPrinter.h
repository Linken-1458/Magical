/**
 * @file MsgPrinter.h
 * @brief Used to manage printing message into the log or on the screen
 * @author Keren Zhu
 * @date 06/14/2019
 */


#ifndef ZKUTIL_MSGPRINTER_H_
#define ZKUTIL_MSGPRINTER_H_

#include <cstdio> // FILE *, stderr
#include <string>
#include <ctime>
#include <cstdarg>
#include "global/namespace.h"

PROJECT_NAMESPACE_BEGIN


/// Enum type for message printing      消息打印的枚举类型
enum class MsgType 
{
    INF,
    WRN,
    ERR,
    DBG
};

/// Function converting enum type to std::string        将枚举类型转换为std:：string的函数 
std::string msgTypeToStr(MsgType msgType);

/// Message printing class      消息打印类
class MsgPrinter 
{
    public:
        static void startTimer() { _startTime = std::time(nullptr); } // Cache start time       缓存开始时间 
        static void screenOn()   { _screenOutStream = stderr; }       // Turn on screen printing    打开屏幕展示
        static void screenOff()  { _screenOutStream = nullptr; }      // Turn off screen printing   关闭屏幕展示

        static void openLogFile(const std::string &file);
        static void closeLogFile();
        static void inf(const char *rawFormat, ...);
        static void wrn(const char *rawFormat, ...);
        static void err(const char *rawFormat, ...);
        static void dbg(const char *rawFormat, ...);

    private:
        static void print(MsgType msgType, const char *rawFormat, va_list args);

    private:
        static std::time_t   _startTime;
        static FILE *        _screenOutStream;  // Out stream for screen printing   屏幕打印
        static FILE *        _logOutStream;     // Out stream for log printing      日志打印
        static std::string   _logFileName;      // Current log file name            当前日志文件名 
};

PROJECT_NAMESPACE_END

#endif // SHROUTER_MSGPRINTER_H_
 
