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


//! Memroy size (or rather dimension) types.
enum EMemorySizeTypes
{
    MEMORYSIZE_BYTE = 0,    //!< Byte.
    MEMORYSIZE_KB,          //!< Kilo byte (KB).
    MEMORYSIZE_MB,          //!< Mega byte (MB).
    MEMORYSIZE_GB,          //!< Giga byte (GB).
    MEMORYSIZE_TB,          //!< Tera byte (TB).
};


class SP_EXPORT OSInformator
{
    
    public:
        
        OSInformator();
        ~OSInformator();
        
        /* === Functions === */
        
        //! Returns the compiler version whereby the engine has been compiled.
        stringc getCompilerVersion() const;
        
        /**
        \return Information about the engine compilation. This string shows all the compilation options
        which has been set at the engine's compilation time. Among others which video drivers has been compiled.
        e.g. in GCC version Direct3D11 will not be available.
        */
        stringc getCompilationInfo() const;
        
        bool setClipboardText(const stringc &Text);
        stringc getClipboardText() const;
        
        //! Returns the speed of the CPU in MHz.
        u32 getProcessorSpeed() const;
        //! Returns the count of physical and virtual processors.
        u32 getProcessorCount() const;
        
        //! \todo Not yet implemented!
        void getDiskSpace(const stringc &PartitionName, u32 &Total, u32 &Free) const;
        /**
        Returns the total and free virutal memory.
        \param[out] Total Specifies the total virtual memory.
        \param[out] Free Specifies the free virtual memory.
        \param[in] SizeType Specifies in which size (or rather dimension) the output is to be returned.
        */
        void getVirtualMemory(u64 &Total, u64 &Free, const EMemorySizeTypes SizeType = MEMORYSIZE_MB) const;
        
        /* === Inline functions === */
        
        //! Returns the operating system version (e.g. "Microsoft Windows 7 Professional (Build xyz)").
        inline const stringc& getOSVersion() const
        {
            return OSVersion_;
        }
        
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
