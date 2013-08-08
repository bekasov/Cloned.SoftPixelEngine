/*
 * Quadrangle 3D header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_DIMENSION_QUADRANGLE3D_H__
#define __SP_DIMENSION_QUADRANGLE3D_H__


#include "Base/spStandard.hpp"
#include "Base/spDimensionVector3D.hpp"


namespace sp
{
namespace dim
{


/**
Quadrangle 3D (PointA, PointB, PointC, PointD) class which is basically only used for collision detection.
\ingroup group_data_types
*/
template <typename T> class quadrangle3d
{
    
    public:
        
        quadrangle3d()
        {
        }
        quadrangle3d(const vector3d<T> &A, const vector3d<T> &B, const vector3d<T> &C, const vector3d<T> &D) :
            PointA(A),
            PointB(B),
            PointC(C),
            PointD(D)
        {
        }
        quadrangle3d(const quadrangle3d<T> &Other) :
            PointA(Other.PointA),
            PointB(Other.PointB),
            PointC(Other.PointC),
            PointD(Other.PointD)
        {
        }
        ~quadrangle3d()
        {
        }
        
        /* === Operators === */
        
        inline void operator = (const quadrangle3d<T> &Other)
        {
            PointA = Other.PointA;
            PointB = Other.PointB;
            PointC = Other.PointC;
            PointD = Other.PointD;
        }
        
        /* === Additional operators === */
        
        inline const vector3d<T>& operator [] (u32 i) const
        {
            return *(&PointA + i);
        }
        inline vector3d<T>& operator [] (u32 i)
        {
            return *(&PointA + i);
        }
        
        /* === Inline functions === */
        
        inline vector3d<T> getNormal() const
        {
            return dim::vector3d<T>( (PointB - PointA).cross(PointC - PointA) ).normalize();
        }
        
        inline vector3d<T> getCenter() const
        {
            return (PointA + PointB + PointC + PointD) / 4;
        }
        
        inline T getArea() const
        {
            return dim::vector3d<T>( (PointB - PointA).cross(PointC - PointA) ).getLength();
        }
        
        //! \todo This has not been tested yet!
        inline bool isPointInside(const vector3d<T> &Vector) const
        {
            return
                ( vector3d<T>::isPointOnSameSide(Vector, PointA, PointB, PointC) &&
                  vector3d<T>::isPointOnSameSide(Vector, PointB, PointA, PointC) &&
                  vector3d<T>::isPointOnSameSide(Vector, PointC, PointA, PointB) ) ||
                ( vector3d<T>::isPointOnSameSide(Vector, PointA, PointC, PointD) &&
                  vector3d<T>::isPointOnSameSide(Vector, PointC, PointA, PointD) &&
                  vector3d<T>::isPointOnSameSide(Vector, PointD, PointA, PointC) );
        }
        
        inline quadrangle3d<T> getSwaped() const
        {
            return quadrangle3d<T>(PointA, PointD, PointC, PointB);
        }
        inline quadrangle3d<T>& swap()
        {
            std::swap(PointB, PointD);
            return *this;
        }
        
        inline bool equal(const quadrangle3d<T> &other, f32 Precision = math::ROUNDING_ERROR) const
        {
            return
                PointA.equal(other.PointA, Precision) &&
                PointB.equal(other.PointB, Precision) &&
                PointC.equal(other.PointC, Precision) &&
                PointD.equal(other.PointC, Precision);
        }
        inline bool empty() const
        {
            return PointA.empty() && PointB.empty() && PointC.empty() && PointD.empty();
        }
        
        template <typename B> inline quadrangle3d<B> cast() const
        {
            return triangle3d<B>(
                PointA.cast<B>(), PointB.cast<B>(), PointC.cast<B>(), PointD.cast<B>()
            );
        }
        
        /* === Members === */
        
        vector3d<T> PointA, PointB, PointC, PointD;
        
};

typedef quadrangle3d<s32> quadrangle3di;
typedef quadrangle3d<f32> quadrangle3df;


} // /namespace dim

} // /namespace sp


#endif



// ================================================================================
