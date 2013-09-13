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

struct SLogState
{
    SLogState() :
        TimeFormat  (LOGTIME_DISABLE    ),
        Context     (LOGCONTEXT_CONSOLE ),
        TabString   ("  "               ),
        IsFile      (false              ),
        IsPaused    (false              )
    {
    }
    ~SLogState()
    {
    }
    
    /* Members */
    ELogTimeFormats TimeFormat;
    s32 Context;
    stringc TabString, Tab;
    std::ofstream File;
    io::stringc Filename;
    bool IsFile, IsPaused;
    MessageCallback MsgCallback;
};

static SLogState LogState;
static std::map<std::string, bool> UniqueMessages;


/*
 * Functions
 */

static bool checkUniqueMessage(const stringc &Message)
{
    /* Check if message is already in the unique list */
    std::map<std::string, bool>::iterator it = UniqueMessages.find(Message.str());
    
    if (it != UniqueMessages.end())
        return false;
    
    /* Store message in the unique list */
    UniqueMessages[Message.str()] = true;
    
    return true;
}

SP_EXPORT void setMessageCallback(const MessageCallback &Proc)
{
    LogState.MsgCallback = Proc;
}

SP_EXPORT void error(const stringc &Message, s32 Flags)
{
    message("Error: " + Message + "!", Flags | LOG_ERROR);
}

SP_EXPORT void warning(const stringc &Message, s32 Flags)
{
    message("Warning: " + Message + "!", Flags | LOG_WARNING);
}

SP_EXPORT void debug(const stringc &ProcName, const stringc &Message, s32 Flags)
{
    if (ProcName.size())
        message("Debug [ " + ProcName + " ]: " + Message + "!", Flags | LOG_DEBUG);
    else
        message("Debug: " + Message + "!", Flags | LOG_DEBUG);
}

#if defined(SP_PLATFORM_ANDROID)

SP_EXPORT void message(const stringc &Message, s32 Flags)
{
    /* Check if message is unique */
    if ((Flags & LOG_UNIQUE) != 0 && !checkUniqueMessage(Message))
        return;
    
    /* Extend the message string */
    stringc FinalMessage, TimePart;
    
    if (LogState.TimeFormat != LOGTIME_DISABLE && (Flags & LOG_TIME))
        TimePart = getFormatedTime();
    if (!(Flags & LOG_NOTAB))
        FinalMessage += LogState.Tab;
    
    FinalMessage = TimePart + FinalMessage + Message;
    
    /* Print the message to the default log */
    if (Flags & LOG_ERROR)
        LOG_ERROR("%s", FinalMessage.c_str());
    else if ( ( Flags & LOG_WARNING ) || ( Flags & LOG_DEBUG ) )
        LOG_WARNING("%s", FinalMessage.c_str());
    else
        LOG_INFO("%s", FinalMessage.c_str());
    
    /* Print the message to the log file */
    if (!(Flags & LOG_NONEWLINE))
        FinalMessage += "\n";
    
    if (LogState.IsFile && (LogContext & LOGCONTEXT_FILE))
        LogState.File << FinalMessage.str();
    
    if (LogState.MessageCallback)
        LogState.MessageCallback(FinalMessage, Flags);
}

#else

SP_EXPORT void message(const stringc &Message, s32 Flags)
{
    /* Check if message is unique */
    if ((Flags & LOG_UNIQUE) != 0 && !checkUniqueMessage(Message))
        return;
    
    if (Flags & LOG_MSGBOX)
    {
        
        #if defined(SP_PLATFORM_WINDOWS)
        
        /* Open Win32 message box */
        if (Flags & LOG_ERROR)
            MessageBox(0, TEXT(Message.c_str()), TEXT("Error"), MB_OK | MB_ICONERROR);
        else if ( ( Flags & LOG_WARNING ) || ( Flags & LOG_DEBUG ) )
            MessageBox(0, TEXT(Message.c_str()), TEXT("Warning"), MB_OK | MB_ICONWARNING);
        else
            MessageBox(0, TEXT(Message.c_str()), TEXT("Information"), MB_OK | MB_ICONINFORMATION);
        
        #elif defined(SP_PLATFORM_IOS)
        
        /* Open iOS alert view */
        if (Flags & LOG_ERROR)
            iOS_AlertView("Error", Message.c_str());
        else if (Flags & LOG_WARNING)
            iOS_AlertView("Warning", Message.c_str());
        else
            iOS_AlertView("Message", Message.c_str());
        
        #endif
    }
    else if (LogState.Context != LOGCONTEXT_NONE)
    {
        /* Extend the message string */
        stringc FinalMessage, TimePart;
        
        if (LogState.TimeFormat != LOGTIME_DISABLE && (Flags & LOG_TIME))
            TimePart = getFormatedTime();
        if (!(Flags & LOG_NOTAB))
            FinalMessage += LogState.Tab;
        
        FinalMessage += Message;
        
        if (!(Flags & LOG_NONEWLINE))
            FinalMessage += "\n";
        
        /* Print the message to the console output */
        if (LogState.Context & LOGCONTEXT_CONSOLE)
        {
            std::cout << TimePart.str();
            
            #if defined(SP_PLATFORM_WINDOWS)
            static HANDLE ConsoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
            
            if (Flags & LOG_ERROR)
                SetConsoleTextAttribute(ConsoleHandle, FOREGROUND_RED | FOREGROUND_INTENSITY);
            else if (Flags & LOG_WARNING)
                SetConsoleTextAttribute(ConsoleHandle, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
            else if (Flags & LOG_DEBUG)
                SetConsoleTextAttribute(ConsoleHandle, FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
            #endif
            
            std::cout << FinalMessage.str();
            
            #if defined(SP_PLATFORM_WINDOWS)
            SetConsoleTextAttribute(ConsoleHandle, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
            #endif
        }
        
        /* Print the message to the log file */
        if (LogState.IsFile && (LogState.Context & LOGCONTEXT_FILE))
        {
            pause(false);
            LogState.File << (TimePart + FinalMessage).str();
            pause(true);
        }
        
        if (LogState.MsgCallback)
            LogState.MsgCallback(FinalMessage, Flags);
    }
}

#endif

SP_EXPORT void setTimeFormat(const ELogTimeFormats Format)
{
    LogState.TimeFormat = Format;
}
SP_EXPORT ELogTimeFormats getTimeFormat()
{
    return LogState.TimeFormat;
}

SP_EXPORT void setTabString(const stringc &TabStr)
{
    LogState.TabString = TabStr;
}
SP_EXPORT stringc getTabString()
{
    return LogState.TabString;
}

SP_EXPORT stringc getCurrentTab()
{
    return LogState.Tab;
}

SP_EXPORT void upperTab()
{
    LogState.Tab += LogState.TabString;
}
SP_EXPORT void lowerTab()
{
    const s32 Len = static_cast<s32>(LogState.Tab.size()) - LogState.TabString.size();
    if (Len <= 0)
        LogState.Tab = "";
    else
        LogState.Tab = LogState.Tab.left(Len);
}

SP_EXPORT void setOutputContext(const s32 Context)
{
    LogState.Context = Context;
}
SP_EXPORT s32 getOutputContext()
{
    return LogState.Context;
}

SP_EXPORT bool open(const stringc &Filename)
{
    if (LogState.IsFile)
        close();
    
    LogState.Context |= LOGCONTEXT_FILE;
    
    LogState.File.open(Filename.c_str(), std::ios::trunc);
    
    if (LogState.File.fail())
        return false;
    
    LogState.File << "SoftPixel Engine - debug log file:\n";
    LogState.File << ("(generated at " + Timer::getTime() + ")\n").c_str();
    LogState.File << "==================================\n\n";
    
    LogState.IsFile     = true;
    LogState.IsPaused   = false;
    LogState.Filename   = Filename;
    
    return true;
}
SP_EXPORT void close()
{
    if (!LogState.IsFile)
        return;
    
    LogState.File << "\n===========\n";
    LogState.File << "END-OF-LINE\n";
    
    LogState.File.close();
    
    LogState.IsFile     = false;
    LogState.IsPaused   = false;
    LogState.Filename   = "";
}

SP_EXPORT void pause(bool isPaused)
{
    if (LogState.IsFile && LogState.IsPaused != isPaused)
    {
        LogState.IsPaused = isPaused;
        
        if (isPaused)
            LogState.File.close();
        else
            LogState.File.open(LogState.Filename.c_str(), std::ios::app);
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
    
    switch (LogState.TimeFormat)
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
