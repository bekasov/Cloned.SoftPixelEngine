/*
 * Vector arithmetic header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_VECTOR_ARITHMETIC_H__
#define __SP_VECTOR_ARITHMETIC_H__


#include <math.h>


namespace sp
{
namespace dim
{


/**
Returns the cross product of the specified 3D vectors A and B.
\tparam T Specifies the vector type. This type must have the member varaibles X, Y and Z
and a constructor which takes three parameters for these coordinates.
This could be vector3d, vector4d or your own class for a vector with three components.
\param[in] A Specifies the first vector.
\param[in] B Specifies the second vector.
\return Cross product which is of the same type as the input parameters.
\since Version 3.3
*/
template <typename T> inline T cross(const T &A, const T &B)
{
    return T(
        A.Y*B.Z - B.Y*A.Z,
        B.X*A.Z - A.X*B.Z,
        A.X*B.Y - B.X*A.Y
    );
}

/**
Returns the dot- (or rather scalar-) product of the specified 3D vectors A and B.
\tparam Va Specifies the vector type. This type must have the member varaibles X, Y and Z.
This could be vector3d, vector4d or your own class for a vector with three components.
\tparam Vb Same rules as for Va.
\tparam T Specifies the data type. This should be a floating point type (e.g. float or double).
\param[in] A Specifies the first vector.
\param[in] B Specifies the second vector.
\return Dot product which is of the same type as the input parameters.
\since Version 3.3
*/
template < template <typename> class Va, template <typename> class Vb, typename T > inline T dot(const Va<T> &A, const Vb<T> &B)
{
    return A.X*B.X + A.Y*B.Y + A.Z*B.Z;
}

/**
Returns the length of the specified 3D vector.
\tparam V Specifies the vector type. This must be suitable for the 'dot' function.
\tparam T Specifies the data type. This should be a floating point type (e.g. float or double).
\param[in] Vec Specifies the vector whose length is to be computed.
\return Length (or rather the euclidean norm) of the specified vector.
\see dot
\since Version 3.3
*/
template < template <typename> class V, typename T > inline T length(const V<T> &Vec)
{
    return sqrt(dot(Vec, Vec));
}

/**
Returns the distance bewteen the two specified 3D vectors A and B.
\tparam V Specifies the vector type. This must be suitable for the 'length'
function and must implement the member operator '-='.
\tparam T Specifies the data type. This should be a floating point type (e.g. float or double).
\param[in] A Specifies the first vector.
\param[in] B Specifies the second vector.
\return Distance (with the euclidean norm) between the two vectors. This is equivalent to the following code:
\code
length(B - A);
\endcode
\see length
\since Version 3.3
*/
template < template <typename> class V, typename T > inline T distance(const V<T> &A, const V<T> &B)
{
    V<T> Vec = B;
    Vec -= A;
    return length(Vec);
}

/**
Normalizes the specified 3D vector. After that the vector has the length of 1.
\tparam V Specifies the vector type. This must be suitable for the 'dot' function.
\tparam T Specifies the data type. This should be a floating point type (e.g. float or double).
\param[in,out] Vec Specifies the vector which is to be normalized.
\note If the specified vector is a zero vector (i.e. X, Y and Z are all zero) this function has no effect.
\see dot
\since Version 3.3
*/
template < template <typename> class V, typename T > inline void normalize(V<T> &Vec)
{
    T n = dot(Vec, Vec);
	
    if (n != T(0) && n != T(1))
    {
        n = T(1) / sqrt(n);
		
        Vec.X *= n;
        Vec.Y *= n;
        Vec.Z *= n;
    }
}

/**
Returns the angle (in radian) bewteen the two specified 3D vectors A and B.
\tparam Va Specifies the vector type. This must be suitable for the 'length'.
\tparam Vb Same rules as for Va.
\tparam T Specifies the data type. This should be a floating point type (e.g. float or double).
\param[in] A Specifies the first vector.
\param[in] B Specifies the second vector.
\return Angle (in radian) between the two vectors. Use the following code to get the angle in degree:
\code
angle(A, B) * math::RAD;
\endcode
\see length
\since Version 3.3
*/
template < template <typename> class Va, template <typename> class Vb, typename T > inline T angle(const Va<T> &A, const Vb<T> &B)
{
    return acos(dot(A, B) / (length(A)*length(B)));
}


} // /namespace dim

} // /namespace sp


#endif



// ================================================================================
