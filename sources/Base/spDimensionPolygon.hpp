/*
 * Polygon header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_DIMENSION_POLYGON_H__
#define __SP_DIMENSION_POLYGON_H__


#include "Base/spStandard.hpp"
#include "Base/spDimensionVector3D.hpp"

#include <vector>


namespace sp
{
namespace dim
{


/**
Simple polygon class for triangle clipping.
\since Version 3.2
\todo Make tests with this class.
*/
template <typename T> class polygon
{
    
    public:
        
        polygon()
        {
        }
        polygon(const polygon<T> &Other) :
            Points(Other.Points)
        {
        }
        ~polygon()
        {
        }
        
        /* === Operators === */
        
        inline polygon<T>& operator = (const polygon<T> &Other)
        {
            Points = Other.Points;
            return *this;
        }
        
        /* === Additional operators === */
        
        inline const T& operator [] (u32 Index) const
        {
            return Points[Index];
        }
        inline T& operator [] (u32 Index)
        {
            return Points[Index];
        }
        
        /* === Extra functions === */
        
        inline u32 getCount() const
        {
            return Points.size();
        }
        
        inline void clear()
        {
            Points.clear();
        }
        
        inline void push(const T &Point)
        {
            Points.push_back(Point);
        }
        inline void pop()
        {
            Points.pop_back();
        }
        
        inline polygon<T> getSwaped() const
        {
            polygon<T> Swaped(*this);
            Swaped.swap();
            return Swaped;
        }
        inline polygon<T>& swap()
        {
            for (u32 i = 0, c = getCount()/2; i < c; ++i)
                math::Swap(Points[i], Points[getCount() - i - 1]);
            return *this;
        }
        
        template <typename T2> inline polygon<T2> cast() const
        {
            polygon<T2> Casted;
            
            for (u32 i = 0; i < getCount(); ++i)
                Casted.Points[i] = Points[i].cast<T2>();
            
            return Casted;
        }
        
        /* === Members === */
        
        std::vector<T> Points;
        
};

typedef polygon<vector3di> polygon3di;
typedef polygon<vector3df> polygon3df;


} // /namespace dim

} // /namespace sp


#endif



// ================================================================================
