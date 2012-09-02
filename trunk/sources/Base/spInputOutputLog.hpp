/*
 * Log header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_INPUTOUTPUT_LOG_H__
#define __SP_INPUTOUTPUT_LOG_H__


#include "Base/spStandard.hpp"
#include "Base/spInputOutputString.hpp"
#include "Base/spDimension.hpp"
#include "Base/spMaterialColor.hpp"

#include <fstream>
#include <string.h>
#include <boost/function.hpp>

#if defined(SP_PLATFORM_WINDOWS)
#   include <io.h>
#elif defined(SP_PLATFORM_LINUX)
#   include <cstdio>
#elif defined(SP_PLATFORM_ANDROID)
#   include <android/log.h>
#endif


namespace sp
{
namespace io
{


//! Log message time information formats.
enum ELogTimeFormats
{
    LOGTIME_DISABLE,    //!< No time is printed to any log message.
    LOGTIME_HHMMSS,     //!< Current time is printed to every log message in the format [HH:MM:SS.] (e.g. "[18:30:12]").
    LOGTIME_MILLISECS,  //!< Current time is printed to every log message in the format [M] where M are the milliseconds since program start (e.g. "[139758]").
};

//! Flags for printed log messages.
enum ELogMessageFlags
{
    LOG_MSGBOX      = 0x01, //!< Use a message box.
    LOG_TIME        = 0x02, //!< Print time (e.g. "[18:30:12] ...").
    LOG_WARNING     = 0x04, //!< Yellow color like a warning message.
    LOG_ERROR       = 0x08, //!< Red color like an error message.
    LOG_DEBUG       = 0x10, //!< Pink color like a debug message.
    LOG_NONEWLINE   = 0x20, //!< No new line will be printed.
    LOG_NOTAB       = 0x40, //!< No tab string is printed.
};

//! Log output contexts for runtime debugging.
enum ELogOutputContexts
{
    LOGCONTEXT_NONE     = 0x00, //!< No log information will be printed.
    LOGCONTEXT_CONSOLE  = 0x01, //!< Log information will be printed in the console.
    LOGCONTEXT_FILE     = 0x02, //!< Log information will be printed in the debug file.
};


#if defined(SP_PLATFORM_ANDROID)
static stringc DEF_DEBUGLOG_FILE = "/mnt/sdcard/spDebugLog.txt";
#else
static stringc DEF_DEBUGLOG_FILE = "spDebugLog.txt";
#endif


//! Log class has been changed to an own namespace. The syntax remains the same but now you can use "using namespace io::Log;".
namespace Log
{

typedef boost::function<void (const io::stringc &Message, s32 Flags)> MessageCallback;

SP_EXPORT void setMessageCallback(const MessageCallback &Proc);

/**
Prints an error message in red into the current output context.
\param Message: Message which is to be printed.
\param Flags: Message flags which can be a combination of the ELogMessageFlags constants.
*/
SP_EXPORT void error(const stringc &Message, s32 Flags = LOG_TIME);

//! Prints a warning message in yellow.
SP_EXPORT void warning(const stringc &Message, s32 Flags = LOG_TIME);

//! Prints a debug message in pink.
SP_EXPORT void debug(const stringc &ProcName, const stringc &Message = "Invalid arguments", s32 Flags = LOG_TIME);

//! Prints a standard message.
SP_EXPORT void message(const stringc &Message, s32 Flags = LOG_TIME);

/**
Sets the time format.
\param Format: Specifies the new time format for the log output.
By default LOGTIME_DISABLE.
*/
SP_EXPORT void setTimeFormat(const ELogTimeFormats Format);
SP_EXPORT ELogTimeFormats getTimeFormat();

//! Sets the string which is to be added when "upperTab" is called.
SP_EXPORT void setTabString(const stringc &TabStr);
SP_EXPORT stringc getTabString();

//! Returns the current string of the whole tab.
SP_EXPORT stringc getCurrentTab();

//! Adds the tab string to the current tab. \see setTabString
SP_EXPORT void upperTab();
//! Removes the tab string from the current tab. \see setTabString
SP_EXPORT void lowerTab();

/**
Sets the output contexts.
\see ELogOutputContexts
*/
SP_EXPORT void setOutputContext(const s32 Context);
SP_EXPORT s32 getOutputContext();

/**
Opens the debug log file. This also sets the debug log file as an additional output context automatically.
\param Filename: Filename for the debug log file. If the file already exists it will be overwritten!
*/
SP_EXPORT bool open(const stringc &Filename = DEF_DEBUGLOG_FILE);
SP_EXPORT void close();

//! Opens or closes the output stream temporarily.
SP_EXPORT void pause(bool isPaused);

SP_EXPORT void openConsole(const stringc &Title = "");
SP_EXPORT void closeConsole();
SP_EXPORT void clearConsole();
SP_EXPORT void pauseConsole();

SP_EXPORT stringc getFormatedTime();

} // /namespace Log


} // /namespace io

} // /namespace sp


#endif



// ================================================================================
