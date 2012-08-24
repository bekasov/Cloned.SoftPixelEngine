/*
 * Orientated BoundingBox header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_DIMENSION_OBB_H__
#define __SP_DIMENSION_OBB_H__


#include "Base/spStandard.hpp"
#include "Base/spMathCore.hpp"
#include "Base/spDimensionVector3D.hpp"


namespace sp
{
namespace dim
{


template <typename T> class obbox3d
{
    
    public:
        
        obbox3d()
        {
        }
        obbox3d(const vector3d<T> &Min, const vector3d<T> &Max) :
            Center  ((Min + Max) / 2),
            Axis(
                vector3d<T>(1, 0, 0),
                vector3d<T>(0, 1, 0),
                vector3d<T>(0, 0, 1)
            ),
            HalfSize((Max - Min) / 2)
        {
        }
        obbox3d(const vector3d<T> &BoxCenter, const vector3d<T> &AxisX, const vector3d<T> &AxisY, const vector3d<T> &AxisZ) :
            Center  (BoxCenter          ),
            Axis    (AxisX, AxisY, AxisZ)
        {
            updateHalfSize();
        }
        obbox3d(const obbox3d<T> &Other) :
            Center  (Other.Center   ),
            Axis    (Other.Axis     ),
            HalfSize(Other.HalfSize )
        {
        }
        ~obbox3d()
        {
        }
        
        /* === Extra functions === */
        
        inline void updateHalfSize()
        {
            /* Store half size */
            HalfSize.X = Axis.X.getLength();
            HalfSize.Y = Axis.Y.getLength();
            HalfSize.Z = Axis.Z.getLength();
            
            /* Normalize axles */
            Axis.X.normalize();
            Axis.Y.normalize();
            Axis.Z.normalize();
        }
        
        inline T getVolume() const
        {
            return (HalfSize * vector3d<T>(2)).getVolume();
        }
        
        /**
        Returns true if the specified inverse point is inside the box. 
        \note This point must be transformed by the inverse matrix of this box!
        */
        inline bool isInversePointInside(const vector3d<T> &Point) const
        {
            return
                math::Abs(Point.X) < T(1) &&
                math::Abs(Point.Y) < T(1) &&
                math::Abs(Point.Z) < T(1);
        }
        
        //! Returns true if the specified point is inside the box.
        inline bool isPointInside(const vector3d<T> &Point) const;
        
        //! Returns true if the specifies box is inside this box.
        inline bool isBoxInside(const obbox3d<T> &Other) const;
        
        /* Members */
        
        vector3d<T> Center;
        vector3d< vector3d<T> > Axis;
        vector3d<T> HalfSize; // (HalfSize.X is the length of Axis.X vector)
        
};

typedef obbox3d<s32> obbox3di;
typedef obbox3d<f32> obbox3df;


} // /namespace dim

} // /namespace sp


#endif



// ================================================================================
