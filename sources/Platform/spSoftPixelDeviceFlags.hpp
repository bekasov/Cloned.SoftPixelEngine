/*
 * SoftPixelDevice header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_SOFTPIXELDEVICE_FLAGS_H__
#define __SP_SOFTPIXELDEVICE_FLAGS_H__


#include "Base/spStandard.hpp"


namespace sp
{


/*
 * Macros
 */

static const s32 DEF_SCRIPT_OBJECTCOUNT = 100;
static const s32 DEF_MULTISAMPLE        = 2;
static const s32 DEF_COLORDEPTH         = 32;
static const f32 DEF_FRAMERATE          = 70.0f;


/*
 * Enumerations
 */

//! Direct3D feature levels.
enum ED3DFeatureLevels
{
    DIRECT3D_FEATURE_LEVEL_DEFAULT, //!< Default Direct3D 11 feature level. This will use highest available feature level.
    DIRECT3D_FEATURE_LEVEL_9_1,     //!< Direct3D 9.1 will be used with shader model 2.
    DIRECT3D_FEATURE_LEVEL_9_2,     //!< Direct3D 9.2 will be used with shader model 2.
    DIRECT3D_FEATURE_LEVEL_9_3,     //!< Direct3D 9.3 will be used with shader model 3.
    DIRECT3D_FEATURE_LEVEL_10_0,    //!< Direct3D 10.0 will be used with shader model 4.
    DIRECT3D_FEATURE_LEVEL_10_1,    //!< Direct3D 10.1 will be used with shader model 4.
    DIRECT3D_FEATURE_LEVEL_11_0,    //!< Direct3D 11.0 will be used with shader model 5.
};


/*
 * Structures
 */

/**
Version number structure which contains four unsigned integers: Major, Minor, Revision and Build number.
\since Version 3.3
*/
struct SVersionNumber
{
    SVersionNumber(u32 Maj = 1, u32 Min = 0, u32 Rev = 0, u32 Bld = 0) :
        Major   (Maj),
        Minor   (Min),
        Revision(Rev),
        Build   (Bld)
    {
    }
    SVersionNumber(const SVersionNumber &Other) :
        Major   (Other.Major    ),
        Minor   (Other.Minor    ),
        Revision(Other.Revision ),
        Build   (Other.Build    )
    {
    }
    ~SVersionNumber()
    {
    }
    
    /* === Operators === */
    
    /**
    Returns the specified number.
    \param[in] Index Specifies which number is to be returned.
    0 -> Major, 1 -> Minor, 2 -> Revision, 3 -> Build.
    \return Specified version number or zero if the input parameter is not in the range [0 .. 3].
    */
    inline u32 operator [] (u32 Index) const
    {
        switch (Index)
        {
            case 0: return Major;
            case 1: return Minor;
            case 2: return Revision;
            case 3: return Build;
        }
        return 0;
    }
    
    inline bool operator == (const SVersionNumber &Other) const
    {
        return
            Major       == Other.Major &&
            Minor       == Other.Minor &&
            Revision    == Other.Revision &&
            Build       == Other.Build;
    }
    
    inline bool operator > (const SVersionNumber &Other) const
    {
        for (u32 i = 0; i < 4; ++i)
        {
            if ((*this)[i] > Other[i])
                return true;
            if ((*this)[i] < Other[i])
                return false;
        }
        return false;
    }
    
    inline bool operator >= (const SVersionNumber &Other) const
    {
        return *this > Other || *this == Other;
    }
    
    inline bool operator < (const SVersionNumber &Other) const
    {
        for (u32 i = 0; i < 4; ++i)
        {
            if ((*this)[i] > Other[i])
                return false;
            if ((*this)[i] < Other[i])
                return true;
        }
        return false;
    }
    
    inline bool operator <= (const SVersionNumber &Other) const
    {
        return *this < Other || *this == Other;
    }
    
    /* === Functions === */
    
    /**
    Returns the simplified version number as a string, i.e. only major and minor version (e.g. "3.2").
    \param[in] MinorTenSpot Specifies whether the minor version can have a ten-spot (e.g. false -> "3.2"; true -> "3.02").
    */
    inline io::stringc getStringSimple(bool MinorTenSpot = false) const
    {
        return io::stringc(Major) + "." + (MinorTenSpot ? io::stringc::number(Minor, 2) : io::stringc(Minor));
    }
    
    /**
    Returns the version number in detail as a string (e.g. "3.2, Rev. 0, Build. 3920").
    \param[in] MinorTenSpot Specifies whether the minor version can have a ten-spot
    (e.g. false -> "3.2, Rev. 0, Build. 3920"; true -> "3.02, Rev. 0, Build. 3920").
    */
    inline io::stringc getStringDetailed(bool MinorTenSpot = false) const
    {
        return getStringSimple(MinorTenSpot) + ", Rev. " + io::stringc(Revision) + ", Build. " + io::stringc(Build);
    }
    
    //! Returns true if this version number is valid. If all numbers are zero the return value is false.
    inline bool valid() const
    {
        return (Major | Minor | Revision | Build) != 0;
    }
    
    /* === Members === */
    
    u32 Major;      //!< Major version number. Should start with 1. By default 1.
    u32 Minor;      //!< Minor version number. Should start with 0. By default 0.
    u32 Revision;   //!< Revision number. Commonly used for bug-fixes. By default 0.
    u32 Build;      //!< Build number or rather project compilation counter. By default 0.
};


/**
Renderer profile flags. This is currently only used for the OpenGL render system.
It can be used to specify which OpenGL profile and version is to be used for context creation.
By default the OpenGL "Compatibility Profile" is used.
\since Version 3.3
*/
struct SRendererProfileFlags
{
    SRendererProfileFlags(
        bool IsExtProfile = false, bool IsGLCoreProfile = false,
        const SVersionNumber &ExGLVersion = SVersionNumber(0, 0, 0, 0)) :
        UseExtProfile   (IsExtProfile                   ),
        UseGLCoreProfile(IsGLCoreProfile                ),
        GLVersion       (ExGLVersion                    ),
        D3DFeatureLevel (DIRECT3D_FEATURE_LEVEL_DEFAULT )
    {
    }
    SRendererProfileFlags(
        const ED3DFeatureLevels ExDXFeatureLevel) :
        UseExtProfile   (false              ),
        UseGLCoreProfile(false              ),
        GLVersion       (0, 0, 0, 0         ),
        D3DFeatureLevel (ExDXFeatureLevel   )
    {
    }
    ~SRendererProfileFlags()
    {
    }
    
    /* === Members === */
    //! Specifies whether an extended renderer profile is to be used. By default false.
    bool UseExtProfile;
    
    /**
    Specifies whether the OpenGL "Core Profile" or the "Compatibility Profile" is to be used.
    This requires "UseExtProfile" to be true. By default false.
    */
    bool UseGLCoreProfile;
    /**
    Specifies the OpenGL renderer version. If "UseGLCoreProfile" is false, this member will be ignored.
    Only the major and minor version will be used to select an OpenGL profile. By default the latest supported OpenGL is used.
    \see SVersionNumber
    */
    SVersionNumber GLVersion;
    
    /**
    Specifies the Direct3D 11 renderer feature level. This can be used for shader
    backwards compatibility. By default DIRECT3D_FEATURE_LEVEL_DEFAULT.
    \see ED3DFeatureLevels
    */
    ED3DFeatureLevels D3DFeatureLevel;
};


/**
Anti-Aliasing flags.
\since Version 3.3
*/
struct SAntiAliasingFlags
{
    SAntiAliasingFlags(bool IsEnabled = false, s32 AAMultiSamples = DEF_MULTISAMPLE) :
        Enabled     (IsEnabled      ),
        MultiSamples(AAMultiSamples )
    {
    }
    ~SAntiAliasingFlags()
    {
    }
    
    /* === Members === */
    bool Enabled;       //!< Enables or disables the anti-aliasing flag.
    u32 MultiSamples;   //!< Specifies the number of multi-samples. By default 2.
};


/**
Graphics device window flags.
\since Version 3.3
*/
struct SDeviceWindowFlags
{
    SDeviceWindowFlags(bool IsVisible = true, bool IsDropFileAccept = false, bool IsResizable = false) :
        Visible         (IsVisible          ),
        DropFileAccept  (IsDropFileAccept   ),
        Resizable       (IsResizable        )
    {
    }
    ~SDeviceWindowFlags()
    {
    }
    
    /* === Members === */
    bool Visible;           //!< Specifies whether the window is visible or hidden at startup. By default visible.
    bool DropFileAccept;    //!< Specifies whether the window accepts droped files or not. By default disabled.
    bool Resizable;         //!< Specifies whether the window is resizable or fixed sized. By default fixed.
};


/**
Vertical-synchronisation flags.
\since Version 3.3
*/
struct SVSyncFlags
{
    SVSyncFlags(bool IsEnabled = true, u32 VSRefreshRate = 60, u32 VSInterval = 1) :
        Enabled     (IsEnabled      ),
        RefreshRate (VSRefreshRate  ),
        Interval    (VSInterval     )
    {
    }
    ~SVSyncFlags()
    {
    }
    
    /* === Inline functions === */
    //! Returns the synchronisation interval (which can be 1, 2, 3 or 4) or zero if vsync is disabled.
    inline u32 getInterval() const
    {
        return Enabled ? Interval : 0;
    }
    
    /* === Members === */
    bool Enabled;       //!< Specifies whether vertical-synchronisation is to be enabled or disabled. By default enabled.
    u32 RefreshRate;    //!< Specifies the refresh rate (in Hz). By default 60.
    u32 Interval;       //!< Specifies the synchronisation interval. Can be 1, 2, 3 or 4. By default 1. If VSync is disabled the interval is automatically zero.
};


/**
Graphics device flags structure. Stores information for further context creation such as Anti-Aliasing and window settings.
Since version 3.3 it also stores information which OpenGL profile it to be used.
*/
struct SDeviceFlags
{
    SDeviceFlags(
        bool ResizAble = false, bool Vsync = true, bool AntiAlias = false,
        s32 AntiAliasSamples = DEF_MULTISAMPLE, bool DropFileAccept = false, bool WindowVisible = true) :
        VSync       (Vsync                                      ),
        Window      (WindowVisible, DropFileAccept, ResizAble   ),
        AntiAliasing(AntiAlias, AntiAliasSamples                )
    {
    }
    ~SDeviceFlags()
    {
    }
    
    /* === Members === */
    SVSyncFlags             VSync;
	SDeviceWindowFlags      Window;
    SAntiAliasingFlags      AntiAliasing;
    SRendererProfileFlags   RendererProfile;
};

static const SDeviceFlags DEVICEFLAG_HQ = SDeviceFlags(false, true, true, 8);


} // /namespace sp


#endif



// ================================================================================
