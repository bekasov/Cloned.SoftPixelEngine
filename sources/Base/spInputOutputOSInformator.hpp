/*
 * OperatingSystem informator header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_INPUTOUTPUT_OSINFORMATOR_H__
#define __SP_INPUTOUTPUT_OSINFORMATOR_H__


#include "Base/spStandard.hpp"
#include "Base/spInputOutputString.hpp"


namespace sp
{
namespace io
{


enum EByteTypes
{
    SIZE_BYTE = 0,
    SIZE_KB,
    SIZE_MB,
    SIZE_GB
};


class SP_EXPORT OSInformator
{
    
    public:
        
        OSInformator();
        ~OSInformator();
        
        /* === Functions === */
        
        //! \return Operating system version on which the engine is currently running.
        inline stringc getOSVersion() const
        {
            return OSVersion_;
        }
        
        //! \return Compiler version whereby the engine has been compiled.
        stringc getCompilerVersion() const;
        
        /**
        \return Information about the engine compilation. This string shows all the compilation options
        which has been set at the engine's compilation time. Among others which video drivers has been compiled.
        e.g. in GCC version Direct3D11 will not be available.
        */
        stringc getCompilationInfo() const;
        
        bool setClipboardText(const stringc &Text);
        stringc getClipboardText() const;
        
        //! \return Speed of the CPU in MHz.
        u32 getProcessorSpeed() const; // (in MHz)
        
        void getDiskSpace(stringc PartitionName, u32 &Total, u32 &Free) const;
        void getVirtualMemory(u64 &Total, u64 &Free, s32 SizeType = SIZE_MB) const;
        
    private:
        
        /* === Functions === */
        
        stringc allocOSVersion();
        
        /* === Members === */
        
        stringc OSVersion_;
        
};


} // /namespace io

} // /namespace sp


#endif








// ================================================================================
