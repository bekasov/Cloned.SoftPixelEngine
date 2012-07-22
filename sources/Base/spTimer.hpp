/*
 * Timer header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_IO_TIMER_H__
#define __SP_IO_TIMER_H__


#include "Base/spStandard.hpp"
#include "Base/spInputOutputString.hpp"


namespace sp
{
namespace io
{


//! Types of time.
enum ETimeTypes
{
    TIME_SECOND = 0,
    TIME_MINUTE,
    TIME_HOUR,
    TIME_DAY,
    TIME_MONTH,
    TIME_YEAR
};

struct SFrequenceQuery;


/**
The Timer class can be used as stop watch or just to have simple time access.
\since Version 3.2
*/
class SP_EXPORT Timer
{
    
    public:
        
        Timer(u64 Duration = 0);
        Timer(bool UseFrequenceQuery);
        virtual ~Timer();
        
        /* === Functions === */
        
        //! Starts the stop watch with the given duration.
        void start(u64 Duration);
        //! Stops the stop watch.
        void stop();
        //! Pauses the stop watch.
        void pause(bool isPaused = true);
        //! Resets the stop watch and uses the same duration as before.
        void reset();
        
        //! Returns true if the timer has finished.
        bool finish();
        
        /**
        Returns the elapsed microseconds since the last reset or creation time.
        \note This can only be used if this timer was created with a frequence query.
        \see resetClockCounter
        */
        u64 getElapsedMicroseconds();
        
        /**
        Resets the clock counter which is used get the elapsed microseconds.
        \see getElapsedMicroseconds
        */
        void resetClockCounter();
        
        /**
        Returns the statistic count of frames per seconds.
        \note This can only be used if this timer was created with a frequence query.
        Otherwise the function will always return 60.
        \see getElapsedMicroseconds
        */
        f64 getFPS();
        
        //! Returns true if the timer was paused.
        inline bool paused() const
        {
            return TimeOut_ != 0;
        }
        
        /* === Static functions === */
        
        /**
        Returns the count of elapsed microseconds since your computer started.
        \note This is actually only supported for Linux systems. For Windows you can use "getElapsedMicroseconds".
        \see getElapsedMicroseconds
        */
        static u64 microsecs();
        
        //! Returns the count of elapsed milliseconds since your computer started.
        static u64 millisecs();
        
        //! Returns the count of elapsed seconds since your computer started.
        static u64 secs();
        
        //! Returns the specified time value.
        static u32 getTime(const ETimeTypes Type);
        
        //! Returns current time as a string in the form "DD/MM/YYYY HH:MM:SS" (e.g. "03/01/2008 15:23:46").
        static io::stringc getTime();
        
        /**
        Returns the statistic count of frames per seconds.
        \param UpdateFrameRate: Specifies the count of frames which need to elapse before the new
        FPS value will be computed (e.g. 5 is a good value).
        \return Frames per second as floating point (statistic value).
        \note Call this function only once every frame. But also note that this version of getting the FPS
        is unprecise. There is a better "getFPS" member function which usese the elapsed microseconds.
        */
        static f32 getFPS(u32 UpdateFrameRate);
        
        /**
        Returns the count of frame since the elapsed second.
        \param Duration: Specifies the duration after which the elapsed frames will be updated.
        \note Call this function only once in each frame!
        */
        static u32 getElapsedFrames(u64 Duration = 1000);
        
        //! Waits for the specified time.
        static void sleep(u32 Milliseconds);
        
        //! Waits for 1 millisecond to give other processes time to run. This can be used in multi-threading.
        static inline void yield()
        {
            sleep(1);
        }
        
    protected:
        
        /* === Members === */
        
        u64 StartTime_, EndTime_, TimeOut_, Duration_;
        
    private:
        
        /* === Functions === */
        
        void createFrequenceQuery();
        
        /* === Members === */
        
        SFrequenceQuery* FreqQuery_;
        
};


} // /namespace io

} // /namespace sp


#endif



// ================================================================================
