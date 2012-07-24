/*
 * Base data types header
 * 
 * This file is part of the "SoftPixel Scene Importer/Exporter" (Copyright (c) 2012 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __SPS_BASE_DATA_TYPES_H__
#define __SPS_BASE_DATA_TYPES_H__


#if defined(_MSC_VER)
#   pragma pack(push, packing)
#   pragma pack(1)
#   define SPS_PACK_STRUCT
#elif defined(__GNUC__)
#   define SPS_PACK_STRUCT __attribute__((packed))
#else
#   define SPS_PACK_STRUCT
#endif


struct SpVector2
{
    SpVector2() : x(0.0f), y(0.0f)
    {
    }
    ~SpVector2()
    {
    }
    
    /* Members */
    float32 x, y;
}
SPS_PACK_STRUCT;

struct SpVector3
{
    SpVector3() : x(0.0f), y(0.0f), z(0.0f)
    {
    }
    ~SpVector3()
    {
    }
    
    /* Members */
    float32 x, y, z;
}
SPS_PACK_STRUCT;

struct SpVector4
{
    SpVector4() : x(0.0f), y(0.0f), z(0.0f), w(1.0f)
    {
    }
    ~SpVector4()
    {
    }
    
    /* Members */
    float32 x, y, z, w;
}
SPS_PACK_STRUCT;

struct SpMatrix4
{
    SpMatrix4()
    {
        memset(m, 0, sizeof(m));
    }
    ~SpMatrix4()
    {
    }
    
    /* Members */
    float32 m[16];
}
SPS_PACK_STRUCT;

struct SpColor
{
    SpColor() : r(255), g(255), b(255), a(255)
    {
    }
    ~SpColor()
    {
    }
    
    /* Members */
    uint8 r, g, b, a;
}
SPS_PACK_STRUCT;

struct SpDimension
{
    SpDimension() : w(0), h(0)
    {
    }
    ~SpDimension()
    {
    }
    
    /* Members */
    int32 w, h;
}
SPS_PACK_STRUCT;


#ifdef _MSC_VER
#   pragma pack(pop, packing)
#endif

#undef SPS_PACK_STRUCT


#endif



// ================================================================================
