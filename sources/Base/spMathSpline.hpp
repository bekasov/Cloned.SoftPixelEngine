/*
 * Spline header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_MATH_SPLINE_H__
#define __SP_MATH_SPLINE_H__


#include "Base/spMath.hpp"
#include "Base/spInputOutput.hpp"

#include <vector>


namespace sp
{
namespace math
{


/*
 * Structures
 */

template <class C> struct SSplinePolynom
{
    SSplinePolynom() : a(0), b(0), c(0), d(0)
    {
    }
    ~SSplinePolynom()
    {
    }
    
    /* Functions */
    inline C calc(const C &t) const
    {
        return a + b*t + c*t*t + d*t*t*t;
    }
    
    /* Members */
    C a, b, c, d;
};


//! Multi-dimensional spline class.
template <class C, typename T, s32 Dimension> class SP_EXPORT Spline
{
    
    public:
        
        Spline(T Expansion = T(1)) :
            PointCount_ (0          ),
            Polynom_    (0          ),
            Expansion_  (Expansion  )
        {
        }
        ~Spline()
        {
            clear();
        }
        
        /* Functions */
        
        bool create(const C* PointArray, u32 Count)
        {
            if (!PointArray || !Count)
                return false;
            
            /* Check if the polynoms are already allocated */
            clear();
            
            /* Allocate new polynoms */
            PointCount_ = Count;
            Polynom_    = new SSplinePolynom<C>[PointCount_];
            
            update(PointArray);
            
            /* Exit the function */
            return true;
        }
        
        void update(const C* PointArray)
        {
            if (PointArray && Polynom_)
            {
                for (s32 i = 0; i < Dimension; ++i)
                    buildPolynom(PointArray, Polynom_, i);
            }
        }
        
        void clear()
        {
            /* Reset the counter of points */
            PointCount_ = 0;
            
            /* Delete the polynoms */
            MemoryManager::deleteBuffer(Polynom_);
        }
        
        /* Inline functions */
        
        inline bool create(const std::vector<C> &PointList)
        {
            return PointList.size() ? create(&PointList[0], PointList.size()) : false;
        }
        inline void update(const std::vector<C> &PointList)
        {
            if (Polynom_ && PointList.size() == PointCount_)
                update(&PointList[0]);
        }
        
        inline void setExpansion(T Expansion)
        {
            Expansion_ = Expansion;
        }
        inline T getExpansion() const
        {
            return Expansion_;
        }
        
        inline u32 getPointCount() const
        {
            return PointCount_;
        }
        
        inline SSplinePolynom<C>& getPolynom(const u32 Index)
        {
            return Polynom_[Index];
        }
        inline SSplinePolynom<C> getPolynom(const u32 Index) const
        {
            if (Index < PointCount_)
                return Polynom_[Index];
            return SSplinePolynom<C>();
        }
        
    private:
        
        /* Functions */
        
        void buildPolynom(const C* Points, SSplinePolynom<C>* f, s32 Comp)
        {
            T* s = new T[PointCount_];
            T* y = new T[PointCount_];
            T* v = new T[PointCount_];
            T* q = new T[PointCount_];
            
            s32 i, Count = static_cast<s32>(PointCount_);
            
            y[0] = 3 * (Points[1][Comp] - Points[0][Comp]);
            
            for (i = 1; i < Count - 1; ++i)
                y[i] = 3 * (Points[i+1][Comp] - Points[i-1][Comp]);
            
            y[Count - 1] = 3 * (Points[Count - 1][Comp] - Points[Count - 2][Comp]);
            
            v[0] = 0.5;
            q[0] = 0.5 * y[0];
            
            for (i = 1; i < Count - 1; ++i)
            {
                v[i] = 1.0 / (4 - v[i - 1]);
                q[i] = Expansion_ * v[i] * (y[i] - q[i - 1]);
            }
            
            q[Count - 1] = Expansion_ * (1.0 / (2 - v[Count - 2])) * (y[Count - 1] - q[Count - 2]);
            
            s[Count - 1] = q[Count - 1];
            
            for (i = Count - 2; i >= 0; --i)
                s[i] = q[i] - v[i]*s[i + 1];
            
            for (i = 0; i < Count - 1; ++i)
            {
                f[i].a[Comp] = Points[i][Comp];
                f[i].b[Comp] = s[i];
                f[i].c[Comp] = 3*Points[i+1][Comp] - 3*Points[i  ][Comp] - 2*s[i] - s[i+1];
                f[i].d[Comp] = 2*Points[i  ][Comp] - 2*Points[i+1][Comp] +   s[i] + s[i+1];
            }
            
            delete [] s;
            delete [] y;
            delete [] v;
            delete [] q;
        }
        
        /* Members */
        
        u32 PointCount_;
        SSplinePolynom<C>* Polynom_;
        T Expansion_;
        
};

typedef Spline<dim::point2df, f32, 2> Spline2D;
typedef Spline<dim::vector3df, f32, 3> Spline3D;


} // /namespace math

} // /namespace sp


#endif



// ================================================================================
