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


/*
 * Global static constants
 */

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
const f32 SQRT2F    = sqrtf(2.0f);

const f64 STDASPECT = 4.0 / 3.0;


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
template <typename T> inline void increase(T &Value, const T &PotNewValue)
{
    if (PotNewValue > Value)
        Value = PotNewValue;
}
//! Decrease the given value if the potentially new value is smaller.
template <typename T> inline void decrease(T &Value, const T &PotNewValue)
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
template <typename T> inline void clamp(T &Value, const T &Min, const T &Max)
{
    if (Value > Max)
        Value = Max;
    else if (Value < Min)
        Value = Min;
}

//! Returns the signed value. Resulting values can only be 1, -1 or 0.
template <typename T> inline T sgn(const T &Value)
{
    return (Value > T(0)) ? T(1) : (Value < T(0)) ? T(-1) : T(0);
}

//! Returns the rounded value to the specified precision.
template <typename T> inline T round(const T &Value, s32 Precision)
{
    s32 exp = static_cast<s32>(pow(10, Precision));
    return static_cast<T>(static_cast<s32>(Value*exp)) / exp;
}

//! Rounds the float value to the nearest integer value (e.g. 3.4 to 3 and 3.5 to 4).
inline s32 round(f32 Value)
{
    return static_cast<s32>(Value + 0.5f);
}

//! Returns the square of the specified value (Value * Value).
template <typename T> inline T pow2(const T &Value)
{
    return Value*Value;
}

//! Returns the sine of the specified value as degree. \todo Rename to "sinDeg"
template <typename T> inline T Sin(const T &Value)
{
    return static_cast<T>(sin(DEG64*Value));
}
//! Returns the cosine of the specified value as degree. \todo Rename to "cosDeg"
template <typename T> inline T Cos(const T &Value)
{
    return static_cast<T>(cos(DEG64*Value));
}
//! Returns the tangent of the specified value as degree. \todo Rename to "tanDeg"
template <typename T> inline T Tan(const T &Value)
{
    return static_cast<T>(tan(DEG64*Value));
}

//! Returns the arcus sine of the specified value as degree.
template <typename T> inline T ASin(const T &Value)
{
    return static_cast<T>(asin(Value)*RAD64);
}
//! Returns arcus cosine of the specified value as degree.
template <typename T> inline T ACos(const T &Value)
{
    return static_cast<T>(acos(Value)*RAD64);
}
//! Returns arcus tangent of the specified value as degree.
template <typename T> inline T ATan(const T &Value)
{
    return static_cast<T>(atan(Value)*RAD64);
}

//! Returns the logarithm with the specified base.
template <typename T> inline T logBase(const T &Value, const T &Base = T(10))
{
    return log(Value) / log(Base);
}

/**
Returns a linear-interpolation ('lerp') between the two given points ('From' and 'To').
\param[out] Result Specifies the resulting output point.
\param[in] From Specifies the start point.
\param[in] To Specifies the end point.
\param[in] Factor Specifies the interpolation factor. This should be in the range [0.0 .. 1.0].
\tparam T Specifies the type of the points (e.g. float, dim::point2df, dim::vector3df etc.).
\tparam I Specifies the interpolation data type. This should be a float or a double.
\see dim::point2d
\see dim::vector3d
*/
template <typename T, typename I> inline void lerp(T &Result, const T &From, const T &To, const I &Factor)
{
    Result = To;
    Result -= From;
    Result *= Factor;
    Result += From;
}

//! Overloaded function. For more information read the documentation of the first variant of this function.
template <typename T, typename I> inline T lerp(const T &From, const T &To, const I &Factor)
{
    T Result;
    lerp<T, I>(Result, From, To, Factor);
    return Result;
}

//! Parabolic interpolation. This is equivalent to "Lerp(From, To, Factor*Factor)".
template <typename T, typename I> inline T lerpParabolic(const T &From, const T &To, const I &Factor)
{
    return lerp(From, To, Factor*Factor);
}

//! Sine interpolation. This is equivalent to "Lerp(From, To, Sin(Factor*90))".
template <typename T, typename I> inline T lerpSin(const T &From, const T &To, const I &Factor)
{
    return lerp(From, To, Sin(Factor*I(90)));
}

//! Rounds the given value to the nearest power of two value (e.g. 34 to 32 and 120 to 128).
inline s32 roundPow2(s32 Value)
{
    s32 i;
    
    for (i = 1; i < Value; i <<= 1);
    
    if (i - Value <= Value - i/2)
        return i;
    
    return i/2;
}

//! Returns true if A and B are equal with the specified tolerance.
inline bool equal(f32 A, f32 B, f32 Tolerance = ROUNDING_ERROR)
{
    return (A + Tolerance >= B) && (A - Tolerance <= B);
}
/**
Returns true if A and B are equal. The tolerance factor is only used to have the
same interface like the floating-point version of this function.
*/
inline bool equal(s32 A, s32 B, s32 Tolerance = 0)
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

//! Adds the specified flag bits to the bit mask (BitMask | Flag).
template <typename A, typename B> inline void addFlag(A &BitMask, const B &Flag)
{
    BitMask |= Flag;
}
//! Removes the specified flag bits from the bit mask (BitMask & (~Flag)).
template <typename A, typename B> inline void removeFlag(A &BitMask, const B &Flag)
{
    BitMask &= (~Flag);
}

/**
Returns the 2D triangle area.
\note This is actually only used inside the dim::triangle3d::getBarycentricCoord function.
*/
template <typename T> inline T getTriangleArea2D(
    const T &x1, const T &y1, const T &x2, const T &y2, const T &x3, const T &y3)
{
    return (x1 - x2)*(y2 - y3) - (x2 - x3)*(y1 - y2);
}

//! Returns a bezier value used for bezier curves.
template <typename T> T getBezierValue(const f32 t, const T &Pos1, const T &Pos2, const T &Radial1, const T &Radial2)
{
    const f32 invt    = 1.0f - t;
    const f32 invt2   = invt*invt;
    const f32 invt3   = invt2*invt;
    const f32 t2      = t*t;
    const f32 t3      = t*t*t;
    
    return Pos1*invt3 + Radial1*3*t*invt2 + Radial2*3*t2*invt + Pos2*t3;
}

//! Returns a bernstein value used for bezier patch generation.
template <typename T> T getBernsteinValue(const f32 t, const T Points[4])
{
    const f32 invt = 1.0f - t;
    
    return
        Points[0] * pow(t, 3) +
        Points[1] * (T(3) * pow(t, 2) * invt) +
        Points[2] * (T(3) * pow(invt, 2) * t) +
        Points[3] * pow(invt, 3);
}

//! Returns a gaussian value used for gaussian blur.
template <typename T> T getGaussianValue(const T &X, const T &Mean, const T &StdDeviation)
{
    return (
        ( T(1) / sqrt( T(2) * static_cast<T>(math::PI) * StdDeviation * StdDeviation ) )
        * exp( ( -( ( X - Mean ) * ( X - Mean ) ) ) / ( T(2) * StdDeviation * StdDeviation ) )
    );
}

/**
Returns the halton sequence for the given input and base. This is used for quasi-random values.
For more details take a look at: http://orion.math.iastate.edu/reu/2001/voronoi/halton_sequence.html.
\param[in] Index Specifies the sequence index.
\param[in] Base Specifies the sequence base. This has to be a prime number! Usually beginning with 2, 3, etc.
\todo This is currently not used and has not been tested!
*/
template <typename T> T getHaltonSequence(s32 Index, s32 Base)
{
    T Result = T(0);
    
    T f = T(1) / Base;
    
    while (Index > 0)
    {
        Result += f * (Index % Base);
        Index /= Base;
        f /= Base;
    }
    
    return Result;
}

/**
Computes the efficient modular pow value.
\tparam T This type must support the following operators: %, %=, *= and >>=.
\param[in] Base Specifies the base value.
\param[in] Exp Specifies the exponent value.
\param[in] Modulus Specifies the modulus value.
\return 'Base' power of 'Exp' modulo 'Modulus'. This is equivalent to '(Base ^ Exp) % Modulus' but faster.
*/
template <typename T> T modularPow(T Base, T Exp, const T &Modulus)
{
    T Result = T(1);
    
    while (Exp > T(0))
    {
        if (Exp % 2 == 1)
        {
            Result *= Base;
            Result %= Modulus;
        }
        Exp >>= 1;
        Base *= Base;
        Base %= Modulus;
    }
    
    return Result;
}


} // /namespace math

} // /namespace sp


#endif



// ================================================================================
