/*
 * Math core header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_MATH_CORE_H__
#define __SP_MATH_CORE_H__


#include "Base/spStandard.hpp"

#include <cmath>


namespace sp
{
namespace math
{


static const f64 ROUNDING_ERROR64   = 0.00000001;   // 1.0e-8
static const f32 ROUNDING_ERROR     = 0.000001f;    // 1.0e-6

static const f32 OMEGA              = 999999.f;

static const f64 PI64   = 3.1415926535897932384626433832795028841971693993751;
static const f32 PI     = 3.14159265359f;

const f32 DEG       = PI / 180.0f;
const f32 RAD       = 180.0f / PI;
const f64 DEG64     = PI64 / 180.0;
const f64 RAD64     = 180.0 / PI64;

const f64 SQRT2     = sqrt(2.0);

const f64 STDASPECT = 4.0 / 3.0;

#define SIN(x)  sin((x)*M_PI/180.0)
#define COS(x)  cos((x)*M_PI/180.0)
#define TAN(x)  tan((x)*M_PI/180.0)
#define ASIN(x) asin(x)*180.0/M_PI
#define ACOS(x) acos(x)*180.0/M_PI
#define ATAN(x) atan(x)*180.0/M_PI


//! Returns the absolute value of the given parameter (5 -> 5 and -5 -> 5).
template <typename T> inline T Abs(const T &Value)
{
    return Value < 0 ? -Value : Value;
}

//! Returns the largest value of the specified variables.
template <typename T> inline T Max(const T &A, const T &B)
{
    return A > B ? A : B;
}
//! Returns the smallest value of the specified variables.
template <typename T> inline T Min(const T &A, const T &B)
{
    return A < B ? A : B;
}

//! Returns the largest value of the specified variables.
template <typename T> inline T Max(const T &A, const T &B, const T &C)
{
    return ( A >= B && A >= C ? A : ( B >= A && B >= C ? B : C ) );
}
//! Returns the smallest value of the specified variables.
template <typename T> inline T Min(const T &A, const T &B, const T &C)
{
    return ( A <= B && A <= C ? A : ( B <= A && B <= C ? B : C ) );
}

//! Increase the given value if the potentially new value is greater.
template <typename T> inline void Increase(T &Value, const T &PotNewValue)
{
    if (PotNewValue > Value)
        Value = PotNewValue;
}
//! Decrease the given value if the potentially new value is smaller.
template <typename T> inline void Decrease(T &Value, const T &PotNewValue)
{
    if (PotNewValue < Value)
        Value = PotNewValue;
}

//! Returns the input value if it is inside the range "Min" and "Max. Otherwise the clamped range.
template <typename T> inline T MinMax(const T &Value, const T &Min, const T &Max)
{
    if (Value > Max)
        return Max;
    else if (Value < Min)
        return Min;
    return Value;
}
//! Clamps the variable "Value" to the range "Min" and "Max".
template <typename T> inline void Clamp(T &Value, const T &Min, const T &Max)
{
    if (Value > Max)
        Value = Max;
    else if (Value < Min)
        Value = Min;
}

//! Returns a linear-interpolation ('lerp') between the two given points ('From' and 'To').
template <typename T, typename U> inline T Lerp(const T &From, const T &To, const U &Factor)
{
    return From + (To - From) * Factor;
}

//! Exchanges (or rather swaps) the content of the specified variables A and B.
template <typename T> inline void Swap(T &A, T &B)
{
    T tmp = A; A = B; B = tmp;
}

//! Returns the signed value. Resulting values can only be 1, -1 or 0.
template <typename T> inline T Sgn(const T &Value)
{
    return (Value > 0) ? 1 : (Value < 0) ? -1 : 0;
}

//! Returns the rounded value to the specified precision.
template <typename T> inline T Round(const T &Value, s32 Precision)
{
    s32 exp = static_cast<s32>(pow(10, Precision));
    return static_cast<T>(static_cast<s32>(Value*exp)) / exp;
}

//! Returns the square of the specified value (Value * Value).
template <typename T> inline T Pow2(const T &Value)
{
    return Value*Value;
}

//! Returns the sine of the specified value.
template <typename T> inline T Sin(const T &Value)
{
    return static_cast<T>(sin(DEG64*Value));
}
//! Returns the cosine of the specified value.
template <typename T> inline T Cos(const T &Value)
{
    return static_cast<T>(cos(DEG64*Value));
}

//! Returns the arcus sine of the specified value.
template <typename T> inline T ASin(const T &Value)
{
    return static_cast<T>(asin(Value)*RAD64);
}
//! Returns arcus cosine of the specified value.
template <typename T> inline T ACos(const T &Value)
{
    return static_cast<T>(acos(Value)*RAD64);
}

//! Returns the logarithm with the specified base.
template <typename T> inline T Log(const T &Value, const T &Base = T(10))
{
    return log(Value) / log(Base);
}

//! Rounds the float value to the nearest integer value (e.g. 3.4 to 3 and 3.5 to 4).
inline s32 Round(f32 Value)
{
    return static_cast<s32>(Value + 0.5f);
}

//! Rounds the given value to the nearest power of two value (e.g. 34 to 32 and 120 to 128).
inline s32 RoundPow2(s32 Value)
{
    s32 i;
    
    for (i = 1; i < Value; i <<= 1);
    
    if (i - Value <= Value - i/2)
        return i;
    
    return i/2;
}

//! Returns true if A and B are equal with the specified tolerance.
inline bool Equal(f32 A, f32 B, f32 Tolerance = ROUNDING_ERROR)
{
    return (A + Tolerance >= B) && (A - Tolerance <= B);
}
/**
Returns true if A and B are equal. The tolerance factor is only used to have the
same interface like the floating-point version of this function.
*/
inline bool Equal(s32 A, s32 B, s32 Tolerance = 0)
{
    return A == B;
}

//! Returns the bit inside the specified integer at the specified position (right to left).
inline bool getBitR2L(u32 Integer, s32 Pos)
{
    return ((Integer >> Pos) & 0x00000001) != 0;
}
//! Sets the bit inside the specified integer at the specified position (right to left).
inline void setBitR2L(u32 &Integer, s32 Pos, bool Enable)
{
    if (Enable)
        Integer |= (0x00000001 << Pos);
    else
        Integer &= ((0xFFFFFFFE << Pos) + (0x7FFFFFFF << (Pos - 31)));
}

//! Returns the bit inside the specified integer at the specified position (left to right).
inline bool getBitL2R(u32 Integer, s32 Pos)
{
    return ((Integer << Pos) & 0x80000000) != 0;
}
//! Sets the bit inside the specified integer at the specified position (left to right).
inline void setBitL2R(u32 &Integer, s32 Pos, bool Enable)
{
    if (Enable)
        Integer |= (0x80000000 >> Pos);
    else
        Integer &= ((0x7FFFFFFF >> Pos) + (0xFFFFFFFE >> (Pos - 31)));
}

//! Returns the bit inside the specified integer at the specified position (right to left).
inline bool getBitR2L(u8 Integer, s32 Pos)
{
    return ((Integer >> Pos) & 0x01) != 0;
}
//! Sets the bit inside the specified integer at the specified position (right to left).
inline void setBitR2L(u8 &Integer, s32 Pos, bool Enable)
{
    if (Enable)
        Integer |= (0x01 << Pos);
    else
        Integer &= ((0xFE << Pos) + (0x7F << (Pos - 31)));
}

//! Returns the bit inside the specified integer at the specified position (left to right).
inline bool getBitL2R(u8 Integer, s32 Pos)
{
    return ((Integer << Pos) & 0x80) != 0;
}
//! Sets the bit inside the specified integer at the specified position (left to right).
inline void setBitL2R(u8 &Integer, s32 Pos, bool Enable)
{
    if (Enable)
        Integer |= (0x80 >> Pos);
    else
        Integer &= ((0x7F >> Pos) + (0xFE >> (Pos - 31)));
}


} // /namespace math

} // /namespace sp


#endif



// ================================================================================
