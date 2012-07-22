/*
 * Base types header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_BASETYPES_H__
#define __SP_BASETYPES_H__


namespace sp
{


/* === Standard types === */

typedef char                c8;
typedef wchar_t             c16;

typedef signed char         s8;
typedef signed short        s16;
typedef signed int          s32;
typedef signed long int     s64;

typedef unsigned char       u8;
typedef unsigned short      u16;
typedef unsigned int        u32;
typedef unsigned long int   u64;

typedef float               f32;
typedef double              f64;
typedef long double         f128;

/* === Macros === */

// !TODO! -> remove from this file!
static const s32 MAX_COUNT_OF_TEXTURES  = 8;
static const s32 MAX_COUNT_OF_LIGHTS    = 8;


} // /namespace sp


#endif



// ================================================================================
