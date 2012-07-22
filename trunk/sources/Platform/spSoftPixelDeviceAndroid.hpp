/*
 * SoftPixel Device Android header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_SOFTPIXELDEVICE_ANDROID_H__
#define __SP_SOFTPIXELDEVICE_ANDROID_H__


#include "Base/spStandard.hpp"

#if defined(SP_PLATFORM_ANDROID)


#include "Platform/spSoftPixelDevice.hpp"
#include "Platform/Android/android_native_app_glue.h"

#include <jni.h>
#include <android/log.h>
#include <android/sensor.h>


namespace sp
{


typedef void (*PFNSAVESTATECALLBACKPROC)(void* &SaveState, size_t &SaveStateSize);

//! SoftPixelDevice class for the Google Android platform.
class SP_EXPORT SoftPixelDeviceAndroid : public SoftPixelDevice
{
    
    public:
        
        SoftPixelDeviceAndroid(
            android_app* App, const video::ERenderSystems RendererType, const io::stringc &Title, bool isFullscreen
        );
        ~SoftPixelDeviceAndroid();
        
        /* Functions */
        
        void processEvent(int32_t Cmd);
        
        bool updateEvent();
        void deleteDevice();
        
        /* Static functions */
        
        static void setSaveStateReadCallback(PFNSAVESTATECALLBACKPROC Callback);
        static void setSaveStateWriteCallback(PFNSAVESTATECALLBACKPROC Callback);
        
        /* Inline functions */
        
        inline android_app* getApp() const
        {
            return App_;
        }
        
    private:
        
        /* Functions */
        
        bool openGraphicsScreen();
        
        void startActivity();
        void stopActivity();
        
        bool updateNextEvent();
        void processSensorEvent(s32 Ident);
        
        /* Members */
        
        android_app* App_;
        
        ASensorManager* SensorManager_;
        ASensorEventQueue* SensorEventQueue_;
        
        const ASensor* GyroscopeSensor_;
        const ASensor* AccelerometerSensor_;
        const ASensor* LightSensor_;
        
        bool isActive_;
        bool isInitWindow_;
        bool isTermWindow_;
        
        static PFNSAVESTATECALLBACKPROC SaveStateRead_, SaveStateWrite_;
        
};


} // /namespace sp


#endif

#endif



// ================================================================================
