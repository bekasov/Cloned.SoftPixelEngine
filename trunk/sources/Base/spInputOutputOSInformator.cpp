/*
 * OperatingSystem informator file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Base/spInputOutputOSInformator.hpp"

#include <iostream>

#if defined(SP_PLATFORM_WINDOWS)
#   include <windows.h>
#elif defined(SP_PLATFORM_LINUX) || defined(SP_PLATFORM_ANDROID)
#   include <sys/utsname.h>
#endif


namespace sp
{
namespace io
{


OSInformator::OSInformator() :
    OSVersion_(allocOSVersion())
{
}
OSInformator::~OSInformator()
{
}

stringc OSInformator::getCompilerVersion() const
{
    #if defined(SP_COMPILER_GCC)
    
    return "GCC " + io::stringc(__GNUC__) + "." + io::stringc(__GNUC_MINOR__) + "." + io::stringc(__GNUC_PATCHLEVEL__);
    
    #elif defined(SP_COMPILER_VC)
    
    io::stringc Version;
    
    #   if _MSC_VER == 600
    Version = "MS. C Compiler 6.0";     // C 6.0
    #   elif _MSC_VER == 700
    Version = "MS. C/C++ Compiler 7.0"; // C/C++ 7.0
    #   elif _MSC_VER == 800
    Version = "Visual C++ 1.0";         // 1.0
    #   elif _MSC_VER == 900
    Version = "Visual C++ 2.0";         // 2.0
    #   elif _MSC_VER == 1000
    Version = "Visual C++ 4.0";         // 4.0
    #   elif _MSC_VER == 1100
    Version = "Visual C++ 5.0";         // 5.0
    #   elif _MSC_VER == 1200
    Version = "Visual C++ 6.0";         // 6.0
    #   elif _MSC_VER == 1300
    Version = "Visual C++ 7.0";         // .NET
    #   elif _MSC_VER == 1310
    Version = "Visual C++ 7.1";         // 2003
    #   elif _MSC_VER == 1400
    Version = "Visual C++ 8.0";         // 2005
    #   elif _MSC_VER == 1500
    Version = "Visual C++ 9.0";         // 2008
    #   elif _MSC_VER == 1600
    Version = "Visual C++ 10.0";        // 2010
    #   endif
    
    #   ifdef _DEBUG
    Version += " (DEBUG)";
    #   endif
    
    return Version;
    
    #else
    
    return "Unknown compiler";
    
    #endif
}

stringc OSInformator::getCompilationInfo() const
{
    stringc Info;
    
    Info += "Compiler Version:                " + getCompilerVersion();
    
    #ifdef SP_COMPILE_WITH_OPENGL
    Info += "\nOpenGL Renderer:                 yes";
    #else
    Info += "\nOpenGL Renderer:                 no";
    #endif
    
    #ifdef SP_COMPILE_WITH_OPENGLES1
    Info += "\nOpenGL|ES 1 Renderer:            yes";
    #else
    Info += "\nOpenGL|ES 1 Renderer:            no";
    #endif
    
    #ifdef SP_COMPILE_WITH_OPENGLES2
    Info += "\nOpenGL|ES 2 Renderer:            yes";
    #else
    Info += "\nOpenGL|ES 2 Renderer:            no";
    #endif
    
    #ifdef SP_COMPILE_WITH_DIRECT3D9
    Info += "\nDirect3D 9 Renderer:             yes";
    #else
    Info += "\nDirect3D 9 Renderer:             no";
    #endif
    
    #ifdef SP_COMPILE_WITH_DIRECT3D11
    Info += "\nDirect3D 11 Renderer:            yes";
    #else
    Info += "\nDirect3D 11 Renderer:            no";
    #endif
    
    #ifdef SP_COMPILE_WITH_NETWORKSYSTEM
    Info += "\nNetwork System:                  yes";
    #else
    Info += "\nNetwork System:                  no";
    #endif
    
    #ifdef SP_COMPILE_WITH_SOUNDSYSTEM
    Info += "\nSound System:                    yes";
    #else
    Info += "\nSound System:                    no";
    #endif
    
    #ifdef SP_COMPILE_WITH_WINMM
    Info += "\nWinMM Sound Device:              yes";
    #else
    Info += "\nWinMM Sound Device:              no";
    #endif
    
    #ifdef SP_COMPILE_WITH_OPENSLES
    Info += "\nOpenSL|ES Sound Device:          yes";
    #else
    Info += "\nOpenSL|ES Sound Device:          no";
    #endif
    
    #ifdef SP_COMPILE_WITH_XAUDIO2
    Info += "\nXAudio2 Sound Device:            yes";
    #else
    Info += "\nXAudio2 Sound Device:            no";
    #endif
    
    #ifdef SP_COMPILE_WITH_GUI
    Info += "\nGUI Manager:                     yes";
    #else
    Info += "\nGUI Manager:                     no";
    #endif
    
    #ifdef SP_COMPILE_WITH_NEWTONPHYSICS
    Info += "\nNewton Physics System:           yes";
    #else
    Info += "\nNewton Physics System:           no";
    #endif
    
    #ifdef SP_COMPILE_WITH_CG
    Info += "\nCg Toolkit:                      yes";
    #else
    Info += "\nCg Toolkit:                      no";
    #endif
    
    #ifdef SP_COMPILE_WITH_UTILITY
    Info += "\nTools:                           yes";
    #else
    Info += "\nTools:                           no";
    #endif
    
    #ifdef SP_COMPILE_WITH_SCENEGRAPH_SIMPLE
    Info += "\nSimple Scene Graph:              yes";
    #else
    Info += "\nSimple Scene Graph:              no";
    #endif
    
    #ifdef SP_COMPILE_WITH_SCENEGRAPH_FAMILY_TREE
    Info += "\nFamily Tree Scene Graph:         yes";
    #else
    Info += "\nFamily Tree Scene Graph:         no";
    #endif
    
    #ifdef SP_COMPILE_WITH_LIGHTMAPGENERATOR
    Info += "\nLightmap Generator:              yes";
    #else
    Info += "\nLightmap Generator:              no";
    #endif
    
    #ifdef SP_COMPILE_WITH_MODELCOMBINER
    Info += "\nModel Combiner:                  yes";
    #else
    Info += "\nModel Combiner:                  no";
    #endif
    
    #ifdef SP_COMPILE_WITH_SCRIPTLOADER
    Info += "\nScript Loader:                   yes";
    #else
    Info += "\nScript Loader:                   no";
    #endif
    
    #ifdef SP_COMPILE_WITH_PARTICLEANIMATOR
    Info += "\nParticle Animator:               yes";
    #else
    Info += "\nParticle Animator:               no";
    #endif
    
    #ifdef SP_COMPILE_WITH_IMAGEMODIFIER
    Info += "\nImage Modifier:                  yes";
    #else
    Info += "\nImage Modifier:                  no";
    #endif
    
    #ifdef SP_COMPILE_WITH_PRIMITIVES
    Info += "\nPrimitives:                      yes";
    #else
    Info += "\nPrimitives:                      no";
    #endif
    
    #ifdef SP_COMPILE_WITH_PRIMITIVE_CUBE
    Info += "\nCube Primitive:                  yes";
    #else
    Info += "\nCube Primitive:                  no";
    #endif
    
    #ifdef SP_COMPILE_WITH_PRIMITIVE_CONE
    Info += "\nCone Primitive:                  yes";
    #else
    Info += "\nCone Primitive:                  no";
    #endif
    
    #ifdef SP_COMPILE_WITH_PRIMITIVE_CYLINDER
    Info += "\nCylinder Primitive:              yes";
    #else
    Info += "\nCylinder Primitive:              no";
    #endif
    
    #ifdef SP_COMPILE_WITH_PRIMITIVE_SPHERE
    Info += "\nSphere Primitive:                yes";
    #else
    Info += "\nSphere Primitive:                no";
    #endif
    
    #ifdef SP_COMPILE_WITH_PRIMITIVE_ICOSPHERE
    Info += "\nIcoSphere Primitive:             yes";
    #else
    Info += "\nIcoSphere Primitive:             no";
    #endif
    
    #ifdef SP_COMPILE_WITH_PRIMITIVE_TORUS
    Info += "\nTorus Primitive:                 yes";
    #else
    Info += "\nTorus Primitive:                 no";
    #endif
    
    #ifdef SP_COMPILE_WITH_PRIMITIVE_TORUSKNOT
    Info += "\nTorusKnot Primitive:             yes";
    #else
    Info += "\nTorusKnot Primitive:             no";
    #endif
    
    #ifdef SP_COMPILE_WITH_PRIMITIVE_SPIRAL
    Info += "\nSpiral Primitive:                yes";
    #else
    Info += "\nSpiral Primitive:                no";
    #endif
    
    #ifdef SP_COMPILE_WITH_PRIMITIVE_PIPE
    Info += "\nPipe Primitive:                  yes";
    #else
    Info += "\nPipe Primitive:                  no";
    #endif
    
    #ifdef SP_COMPILE_WITH_PRIMITIVE_PLANE
    Info += "\nPlane Primitive:                 yes";
    #else
    Info += "\nPlane Primitive:                 no";
    #endif
    
    #ifdef SP_COMPILE_WITH_PRIMITIVE_DISK
    Info += "\nDisk Primitive:                  yes";
    #else
    Info += "\nDisk Primitive:                  no";
    #endif
    
    #ifdef SP_COMPILE_WITH_PRIMITIVE_CUBOCTAHEDRON
    Info += "\nCuboctahedron Primitive:         yes";
    #else
    Info += "\nCuboctahedron Primitive:         no";
    #endif
    
    #ifdef SP_COMPILE_WITH_PRIMITIVE_TETRAHEDRON
    Info += "\nTetrahedron Primitive:           yes";
    #else
    Info += "\nTetrahedron Primitive:           no";
    #endif
    
    #ifdef SP_COMPILE_WITH_PRIMITIVE_OCTAHEDRON
    Info += "\nOctahedron Primitive:            yes";
    #else
    Info += "\nOctahedron Primitive:            no";
    #endif
    
    #ifdef SP_COMPILE_WITH_PRIMITIVE_DODECAHEDRON
    Info += "\nDodecahedron Primitive:          yes";
    #else
    Info += "\nDodecahedron Primitive:          no";
    #endif
    
    #ifdef SP_COMPILE_WITH_PRIMITIVE_ICOSAHEDRON
    Info += "\nIcosahedron Primitive:           yes";
    #else
    Info += "\nIcosahedron Primitive:           no";
    #endif
    
    #ifdef SP_COMPILE_WITH_PRIMITIVE_TEAPOT
    Info += "\nTeapot Primitive:                yes";
    #else
    Info += "\nTeapot Primitive:                no";
    #endif
    
    #ifdef SP_COMPILE_WITH_PRIMITIVE_TEAPOT_DYNAMIC
    Info += "\nDynamic Teapot:                  yes";
    #else
    Info += "\nDynamic Teapot:                  no";
    #endif
    
    #ifdef SP_COMPILE_WITH_PRIMITIVE_SUPERSHAPE
    Info += "\nSuperShape:                      yes";
    #else
    Info += "\nSuperShape:                      no";
    #endif
    
    #ifdef SP_COMPILE_WITH_TEXLOADER_BMP
    Info += "\nBMP Image Loader:                yes";
    #else
    Info += "\nBMP Image Loader:                no";
    #endif
    
    #ifdef SP_COMPILE_WITH_TEXLOADER_JPG
    Info += "\nJPG Image Loader:                yes";
    #else
    Info += "\nJPG Image Loader:                no";
    #endif
    
    #ifdef SP_COMPILE_WITH_TEXLOADER_PCX
    Info += "\nPCX Image Loader:                yes";
    #else
    Info += "\nPCX Image Loader:                no";
    #endif
    
    #ifdef SP_COMPILE_WITH_TEXLOADER_PNG
    Info += "\nPNG Image Loader:                yes";
    #else
    Info += "\nPNG Image Loader:                no";
    #endif
    
    #ifdef SP_COMPILE_WITH_TEXLOADER_TGA
    Info += "\nTGA Image Loader:                yes";
    #else
    Info += "\nTGA Image Loader:                no";
    #endif
    
    #ifdef SP_COMPILE_WITH_TEXLOADER_WAD
    Info += "\nWAD Image Loader:                yes";
    #else
    Info += "\nWAD Image Loader:                no";
    #endif
    
    #ifdef SP_COMPILE_WITH_TEXLOADER_DDS
    Info += "\nDDS Image Loader:                yes";
    #else
    Info += "\nDDS Image Loader:                no";
    #endif
    
    #ifdef SP_COMPILE_WITH_TEXSAVER_BMP
    Info += "\nBMP Image Saver:                 yes";
    #else
    Info += "\nBMP Image Saver:                 no";
    #endif
    
    #ifdef SP_COMPILE_WITH_MESHLOADER_3DS
    Info += "\n3DS Mesh Loader:                 yes";
    #else
    Info += "\n3DS Mesh Loader:                 no";
    #endif
    
    #ifdef SP_COMPILE_WITH_MESHLOADER_B3D
    Info += "\nB3D Mesh Loader:                 yes";
    #else
    Info += "\nB3D Mesh Loader:                 no";
    #endif
    
    #ifdef SP_COMPILE_WITH_MESHLOADER_X
    Info += "\nX Mesh Loader:                   yes";
    #else
    Info += "\nX Mesh Loader:                   no";
    #endif
    
    #ifdef SP_COMPILE_WITH_MESHLOADER_MD2
    Info += "\nMD2 Mesh Loader:                 yes";
    #else
    Info += "\nMD2 Mesh Loader:                 no";
    #endif
    
    #ifdef SP_COMPILE_WITH_MESHLOADER_MS3D
    Info += "\nMS3D Mesh Loader:                yes";
    #else
    Info += "\nMS3D Mesh Loader:                no";
    #endif
    
    #ifdef SP_COMPILE_WITH_MESHLOADER_OBJ
    Info += "\nOBJ Mesh Loader:                 yes";
    #else
    Info += "\nOBJ Mesh Loader:                 no";
    #endif
    
    #ifdef SP_COMPILE_WITH_MESHLOADER_SPM
    Info += "\nSPM Mesh Loader:                 yes";
    #else
    Info += "\nSPM Mesh Loader:                 no";
    #endif
    
    #ifdef SP_COMPILE_WITH_MESHSAVER_B3D
    Info += "\nB3D Mesh Saver:                  yes";
    #else
    Info += "\nB3D Mesh Saver:                  no";
    #endif
    
    #ifdef SP_COMPILE_WITH_MESHSAVER_OBJ
    Info += "\nOBJ Mesh Saver:                  yes";
    #else
    Info += "\nOBJ Mesh Saver:                  no";
    #endif
    
    #ifdef SP_COMPILE_WITH_MESHSAVER_SPM
    Info += "\nSPM Mesh Saver:                  yes";
    #else
    Info += "\nSPM Mesh Saver:                  no";
    #endif
    
    #ifdef SP_COMPILE_WITH_SCENELOADER_BSP1
    Info += "\nBSP1 Scene Loader:               yes";
    #else
    Info += "\nBSP1 Scene Loader:               no";
    #endif
    
    #ifdef SP_COMPILE_WITH_SCENELOADER_BSP3
    Info += "\nBSP3 Scene Loader:               yes";
    #else
    Info += "\nBSP3 Scene Loader:               no";
    #endif
    
    #ifdef SP_COMPILE_WITH_SCENELOADER_SPSB
    Info += "\nSPSB Scene Loader:               yes";
    #else
    Info += "\nSPSB Scene Loader:               no";
    #endif
    
    return Info;
}

#if defined(SP_PLATFORM_WINDOWS)

void OSInformator::setClipboardText(const stringc &Text)
{
    if (!Text.size())
        return;
    
    const c8* cText = Text.c_str();
    
    if (!OpenClipboard(0) || !cText)
        return;
    
    EmptyClipboard();
    
    HGLOBAL ClipBuffer = GlobalAlloc(GMEM_DDESHARE, Text.size() + 1);
    c8* Buffer = (c8*)GlobalLock(ClipBuffer);
    
    strcpy(Buffer, cText);
    
    GlobalUnlock(ClipBuffer);
    SetClipboardData(CF_TEXT, ClipBuffer);
    CloseClipboard();
}

stringc OSInformator::getClipboardText() const
{
    if (!OpenClipboard(0))
        return "";
    
    HANDLE hData = GetClipboardData(CF_TEXT);
    c8* Buffer = (c8*)GlobalLock(hData);
    GlobalUnlock(hData);
    CloseClipboard();
    
    return stringc(Buffer);
}

u32 OSInformator::getProcessorSpeed() const
{
    HKEY Key;
    LONG Error = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", 0, KEY_READ, &Key);
    
    if (Error != ERROR_SUCCESS)
        return 0;
    
    DWORD Speed = 0;
    DWORD Size = sizeof(Speed);
    Error = RegQueryValueEx(Key, "~MHz", 0, 0, (LPBYTE)&Speed, &Size);
    
    RegCloseKey(Key);
    
    return (u32)Speed;
}

void OSInformator::getDiskSpace(stringc PartitionName, u32 &Total, u32 &Free) const
{
    
    // ! unfinished !
    
}

void OSInformator::getVirtualMemory(u64 &Total, u64 &Free, s32 SizeType) const
{
    MEMORYSTATUS MemStatus;
    MemStatus.dwLength = sizeof(MEMORYSTATUS);
    GlobalMemoryStatus(&MemStatus);
    
    Total   = static_cast<u64>(MemStatus.dwTotalPhys);
    Free    = static_cast<u64>(MemStatus.dwAvailPhys);
    
    switch (SizeType)
    {
        case SIZE_GB:
            Total   /= 1024;
            Free    /= 1024;
        case SIZE_MB:
            Total   /= 1024;
            Free    /= 1024;
        case SIZE_KB:
            Total   /= 1024;
            Free    /= 1024;
        case SIZE_BYTE:
            break;
    }
}


/*
 * ======= Private: =======
 */

stringc OSInformator::allocOSVersion()
{
    stringc Version;
    
    OSVERSIONINFOEX OSVersionInfo;
    BOOL bOsVersionInfoEx;
    
    ZeroMemory(&OSVersionInfo, sizeof(OSVERSIONINFOEX));
    OSVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    
    if ( !( bOsVersionInfoEx = GetVersionEx((OSVERSIONINFO*)&OSVersionInfo) ) )
    {
        OSVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
        if (!GetVersionEx((OSVERSIONINFO*)&OSVersionInfo))
            return "";
    }
    
    switch (OSVersionInfo.dwPlatformId)
    {
        case VER_PLATFORM_WIN32_NT:
        {
            if (OSVersionInfo.dwMajorVersion <= 4)
                Version += "Microsoft Windows NT ";
            else if (OSVersionInfo.dwMajorVersion == 5 && OSVersionInfo.dwMinorVersion == 0)
                Version += "Microsoft Windows 2000 ";
            else if (OSVersionInfo.dwMajorVersion == 5 && OSVersionInfo.dwMinorVersion == 1)
                Version += "Microsoft Windows XP ";
            else if (OSVersionInfo.dwMajorVersion == 6 && OSVersionInfo.dwMinorVersion == 0)
                Version += "Microsoft Windows Vista ";
            else if (OSVersionInfo.dwMajorVersion == 6 && OSVersionInfo.dwMinorVersion == 1)
                Version += "Microsoft Windows 7 ";
            
            if (bOsVersionInfoEx)
            {
                #ifdef VER_SUITE_ENTERPRISE
                if (OSVersionInfo.wProductType == VER_NT_WORKSTATION)
                {
                    if (OSVersionInfo.wSuiteMask & VER_SUITE_PERSONAL)
                        Version += "Personal ";
                    else
                        Version += "Professional ";
                }
                else if (OSVersionInfo.wProductType == VER_NT_SERVER)
                {
                    if (OSVersionInfo.wSuiteMask & VER_SUITE_DATACENTER)
                        Version += "DataCenter Server ";
                    else if (OSVersionInfo.wSuiteMask & VER_SUITE_ENTERPRISE)
                        Version += "Advanced Server ";
                    else
                        Version += "Server ";
                }
                #endif
            }
            else
            {
                HKEY hKey;
                c8 szProductType[80];
                DWORD dwBufLen;
                
                RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Control\\ProductOptions", 0, KEY_QUERY_VALUE, &hKey );
                RegQueryValueEx(hKey, "ProductType", 0, 0, (LPBYTE) szProductType, &dwBufLen);
                RegCloseKey(hKey);
                
                if (lstrcmpi("WINNT", szProductType) == 0)
                    Version += "Professional ";
                if (lstrcmpi("LANMANNT", szProductType) == 0)
                    Version += "Server " ;
                if (lstrcmpi("SERVERNT", szProductType) == 0)
                    Version += "Advanced Server ";
            }
            
            // Display version, service pack (if any), and build number.
            if (OSVersionInfo.dwMajorVersion <= 4)
            {
                Version += "version ";
                Version += io::stringc(OSVersionInfo.dwMajorVersion);
                Version += ".";
                Version += io::stringc(OSVersionInfo.dwMinorVersion);
                Version += " ";
            }
            
            Version += io::stringc(OSVersionInfo.szCSDVersion);
            Version += " (Build ";
            Version += io::stringc(OSVersionInfo.dwBuildNumber & 0xFFFF);
            Version += ")";
        }
        break;
        
        case VER_PLATFORM_WIN32_WINDOWS:
        {
            if (OSVersionInfo.dwMajorVersion == 4 && OSVersionInfo.dwMinorVersion == 0)
            {
                Version += "Microsoft Windows 95 ";
                if ( OSVersionInfo.szCSDVersion[1] == 'C' || OSVersionInfo.szCSDVersion[1] == 'B' )
                    Version += "OSR2 ";
            }
            
            if (OSVersionInfo.dwMajorVersion == 4 && OSVersionInfo.dwMinorVersion == 10)
            {
                Version += "Microsoft Windows 98 ";
                if ( OSVersionInfo.szCSDVersion[1] == 'A' )
                    Version += "SE ";
            }
            
            if (OSVersionInfo.dwMajorVersion == 4 && OSVersionInfo.dwMinorVersion == 90)
                Version += "Microsoft Windows Me ";
        }
        break;
        
        case VER_PLATFORM_WIN32s:
        {
            Version += "Microsoft Win32s ";
        }
        break;
    }
    
    return Version;
}

#elif defined(SP_PLATFORM_LINUX) || defined(SP_PLATFORM_ANDROID)

/*
 * ======= Private: =======
 */

void OSInformator::getVirtualMemory(u64 &Total, u64 &Free, s32 SizeType) const
{
    long PageSize       = sysconf(_SC_PAGESIZE);
    long PhysPages      = sysconf(_SC_PHYS_PAGES);
    long AVPhysPages    = sysconf(_SC_AVPHYS_PAGES);
    
    if (PageSize == -1 || PhysPages == -1 || AVPhysPages == -1)
        return;
    
    Total   = static_cast<u64>( (PageSize * (long long)PhysPages) >> 10 );
    Free    = static_cast<u64>( (PageSize * (long long)AVPhysPages) >> 10 );
    
    switch (SizeType)
    {
        case SIZE_GB:
            Total   /= 1024;
            Free    /= 1024;
        case SIZE_MB:
            Total   /= 1024;
            Free    /= 1024;
        case SIZE_KB:
            Total   /= 1024;
            Free    /= 1024;
        case SIZE_BYTE:
            break;
    }
}

stringc OSInformator::allocOSVersion()
{
    #if defined(SP_PLATFORM_LINUX)
    
    stringc Version;
    
    utsname LinuxInfo;
    uname(&LinuxInfo);
    
    Version += LinuxInfo.sysname;
    Version += " ";
    Version += LinuxInfo.release;
    Version += " ";
    Version += LinuxInfo.version;
    Version += " ";
    Version += LinuxInfo.machine;
    
    return Version;
    
    #elif defined(SP_PLATFORM_ANDROID)
    
    return "Android";
    
    #endif
}

#elif defined(SP_PLATFORM_IOS)

/*
 * ======= Private: =======
 */

void OSInformator::getVirtualMemory(u64 &Total, u64 &Free, s32 SizeType) const
{
    //todo
}

stringc OSInformator::allocOSVersion()
{
    return "Apple iOS";
}

#endif


} // /namespace io

} // /namespace sp



// ================================================================================
