/*
 * Log file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Base/spInputOutputLog.hpp"
#include "Base/spTimer.hpp"
#include "Platform/spSoftPixelDeviceOS.hpp"

#include <iostream>

#ifdef SP_PLATFORM_IOS
#   include "Platform/iOS/spPlatformExchangeIOS.h"
#endif


namespace sp
{
namespace io
{


namespace Log
{

/*
 * Global macros
 */

#if defined(SP_PLATFORM_ANDROID)
#   define LOG_TAG          "libSoftPixelEngine"
#   define LOG_INFO(...)    __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#   define LOG_WARNING(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#   define LOG_ERROR(...)   __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#endif


/*
 * Global members
 */

ELogTimeFormats LogTimeFormat = LOGTIME_DISABLE;
s32 LogContext = LOGCONTEXT_CONSOLE;
stringc LogTabString = "  ", LogTab;
std::ofstream LogFile;
io::stringc LogFilename;
bool isLogFile = false, isLogPaused = false;
MessageCallback LogMessageCallback;


/*
 * Functions
 */

SP_EXPORT void setMessageCallback(const MessageCallback &Proc)
{
    LogMessageCallback = Proc;
}

SP_EXPORT void error(const stringc &Message, s32 Flags)
{
    message("Error: " + Message + "!", Flags | LOG_ERROR);
}

SP_EXPORT void warning(const stringc &Message, s32 Flags)
{
    message("Warning: " + Message + "!", Flags | LOG_WARNING);
}

#if defined(SP_PLATFORM_ANDROID)

SP_EXPORT void message(const stringc &Message, s32 Flags)
{
    /* Extend the message string */
    stringc FinalMessage, TimePart;
    
    if (LogTimeFormat != LOGTIME_DISABLE && (Flags & LOG_TIME))
        TimePart = getFormatedTime();
    if (!(Flags & LOG_NOTAB))
        FinalMessage += LogTab;
    
    FinalMessage = TimePart + FinalMessage + Message;
    
    /* Print the message to the default log */
    if (Flags & LOG_ERROR)
        LOG_ERROR("%s", FinalMessage.c_str());
    else if (Flags & LOG_WARNING)
        LOG_WARNING("%s", FinalMessage.c_str());
    else
        LOG_INFO("%s", FinalMessage.c_str());
    
    /* Print the message to the log file */
    if (!(Flags & LOG_NONEWLINE))
        FinalMessage += "\n";
    
    if (isLogFile && (LogContext & LOGCONTEXT_FILE))
        LogFile << FinalMessage.str();
    
    if (LogMessageCallback)
        LogMessageCallback(FinalMessage, Flags);
}

#else

SP_EXPORT void message(const stringc &Message, s32 Flags)
{
    if (Flags & LOG_MSGBOX)
    {
        #if defined(SP_PLATFORM_WINDOWS)
        if (Flags & LOG_ERROR)
            MessageBox(0, TEXT(Message.c_str()), TEXT("Error"), MB_OK | MB_ICONERROR);
        else if (Flags & LOG_WARNING)
            MessageBox(0, TEXT(Message.c_str()), TEXT("Warning"), MB_OK | MB_ICONWARNING);
        else
            MessageBox(0, TEXT(Message.c_str()), TEXT("Information"), MB_OK | MB_ICONINFORMATION);
        #elif defined(SP_PLATFORM_IOS)
        if (Flags & LOG_ERROR)
            iOS_AlertView("Error", Message.c_str());
        else if (Flags & LOG_WARNING)
            iOS_AlertView("Warning", Message.c_str());
        else
            iOS_AlertView("Message", Message.c_str());
        #endif
    }
    else if (LogContext != LOGCONTEXT_NONE)
    {
        /* Extend the message string */
        stringc FinalMessage, TimePart;
        
        if (LogTimeFormat != LOGTIME_DISABLE && (Flags & LOG_TIME))
            TimePart = getFormatedTime();
        if (!(Flags & LOG_NOTAB))
            FinalMessage += LogTab;
        
        FinalMessage += Message;
        
        if (!(Flags & LOG_NONEWLINE))
            FinalMessage += "\n";
        
        /* Print the message to the console output */
        if (LogContext & LOGCONTEXT_CONSOLE)
        {
            std::cout << TimePart.str();
            
            #if defined(SP_PLATFORM_WINDOWS)
            static HANDLE ConsoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
            
            if (Flags & LOG_ERROR)
                SetConsoleTextAttribute(ConsoleHandle, FOREGROUND_RED | FOREGROUND_INTENSITY);
            else if (Flags & LOG_WARNING)
                SetConsoleTextAttribute(ConsoleHandle, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
            #endif
            
            std::cout << FinalMessage.str();
            
            #if defined(SP_PLATFORM_WINDOWS)
            SetConsoleTextAttribute(ConsoleHandle, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
            #endif
        }
        
        /* Print the message to the log file */
        if (isLogFile && (LogContext & LOGCONTEXT_FILE))
        {
            pause(false);
            LogFile << (TimePart + FinalMessage).str();
            pause(true);
        }
        
        if (LogMessageCallback)
            LogMessageCallback(FinalMessage, Flags);
    }
}

#endif

SP_EXPORT void setTimeFormat(const ELogTimeFormats Format)
{
    LogTimeFormat = Format;
}
SP_EXPORT ELogTimeFormats getTimeFormat()
{
    return LogTimeFormat;
}

SP_EXPORT void setTabString(const stringc &TabStr)
{
    LogTabString = TabStr;
}
SP_EXPORT stringc getTabString()
{
    return LogTabString;
}

SP_EXPORT stringc getCurrentTab()
{
    return LogTab;
}

SP_EXPORT void upperTab()
{
    LogTab += LogTabString;
}
SP_EXPORT void lowerTab()
{
    const s32 Len = static_cast<s32>(LogTab.size()) - LogTabString.size();
    if (Len <= 0)
        LogTab = "";
    else
        LogTab = LogTab.left(Len);
}

SP_EXPORT void setOutputContext(const s32 Context)
{
    LogContext = Context;
}
SP_EXPORT s32 getOutputContext()
{
    return LogContext;
}

SP_EXPORT bool open(const stringc &Filename)
{
    if (isLogFile)
        close();
    
    LogContext |= LOGCONTEXT_FILE;
    
    LogFile.open(Filename.c_str(), std::ios::trunc);
    
    if (LogFile.fail())
        return false;
    
    LogFile << "SoftPixel Engine - debug log file:\n";
    LogFile << ("(generated at " + Timer::getTime() + ")\n").c_str();
    LogFile << "==================================\n\n";
    
    isLogFile   = true;
    isLogPaused = false;
    LogFilename = Filename;
    
    return true;
}
SP_EXPORT void close()
{
    if (!isLogFile)
        return;
    
    LogFile << "\n===========\n";
    LogFile << "END-OF-LINE\n";
    
    LogFile.close();
    
    isLogFile   = false;
    isLogPaused = false;
    LogFilename = "";
}

SP_EXPORT void pause(bool isPaused)
{
    if (isLogFile && isLogPaused != isPaused)
    {
        isLogPaused = isPaused;
        
        if (isPaused)
            LogFile.close();
        else
            LogFile.open(LogFilename.c_str(), std::ios::app);
    }
}

#if defined(SP_PLATFORM_WINDOWS)

SP_EXPORT void openConsole(const stringc &Title)
{
    AllocConsole();
    if (Title.str() != "")
        SetConsoleTitle(Title.c_str());
}
SP_EXPORT void closeConsole()
{
    FreeConsole();
}
SP_EXPORT void clearConsole()
{
    system("cls");
}
SP_EXPORT void pauseConsole()
{
    system("pause");
}

#elif defined(SP_PLATFORM_LINUX)

SP_EXPORT void openConsole(const stringc &Title)
{
    system("xterm");
}
SP_EXPORT void closeConsole()
{
    system("exit");
}
SP_EXPORT void clearConsole()
{
    system("clear");
}
SP_EXPORT void pauseConsole()
{
}

#endif

SP_EXPORT stringc getFormatedTime()
{
    stringc Time;
    
    switch (LogTimeFormat)
    {
        case LOGTIME_HHMMSS:
            Time = (
                stringc::number(Timer::getTime(TIME_HOUR    ), 2) + ":" +
                stringc::number(Timer::getTime(TIME_MINUTE  ), 2) + ":" +
                stringc::number(Timer::getTime(TIME_SECOND  ), 2)
            );
            break;
            
        case LOGTIME_MILLISECS:
            Time = stringc::number(Timer::millisecs(), 10, ' ');
            break;
            
        default:
            break;
    }
    
    return "[" + Time + "] ";
}

} // /namesapce Log


} // /namespace io

} // /namespace sp



// ================================================================================
