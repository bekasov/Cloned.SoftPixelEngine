/*
 * Radian and degree header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_MATH_RADIANDEGREE_H__
#define __SP_MATH_RADIANDEGREE_H__


#include "Base/spStandard.hpp"

#include <cmath>


namespace sp
{
namespace math
{


template <typename T> class Degree;
template <typename T> class Radian;

template <typename T> class RadianDegreeAngle
{
    
    public:
        
        virtual ~RadianDegreeAngle()
        {
        }
        
        virtual T getDegree() const = 0;
        virtual T getRadian() const = 0;
        
    protected:
        
        RadianDegreeAngle()
        {
        }
        
};

//! Degree class. An instance of this class will always store a value in the range [0.0 .. 360.0).
template <typename T> class Degree : public RadianDegreeAngle<T>
{
    
    public:
        
        Degree(const T &Deg = 0) :
            Degree_(Deg)
        {
            clamp();
        }
        Degree(const Degree<T> &Deg) :
            Degree_(Deg.Degree_)
        {
        }
        Degree(const Radian<T> &Rad);
        ~Degree()
        {
        }
        
        /* === Functions === */
        
        T getDegree() const
        {
            return Degree_;
        }
        T getRadian() const
        {
            return Degree_ * static_cast<T>(M_PI / 180.0);
        }
        
        /* === Operators === */
        
        inline Degree<T>& operator = (const Degree<T> &Other)
        {
            Degree_ = Other.Degree_;
            return *this;
        }
        
        inline Degree<T>& operator += (const Degree<T> &Other)
        {
            Degree_ += Other.Degree_;
            clamp();
            return *this;
        }
        inline Degree<T>& operator -= (const Degree<T> &Other)
        {
            Degree_ -= Other.Degree_;
            clamp();
            return *this;
        }
        inline Degree<T>& operator *= (const Degree<T> &Other)
        {
            Degree_ *= Other.Degree_;
            clamp();
            return *this;
        }
        inline Degree<T>& operator /= (const Degree<T> &Other)
        {
            Degree_ /= Other.Degree_;
            clamp();
            return *this;
        }
        
        inline Degree<T> operator + (const Degree<T> &Other) const
        {
            return Degree(Degree_ + Other.Degree_);
        }
        inline Degree<T> operator - (const Degree<T> &Other) const
        {
            return Degree(Degree_ - Other.Degree_);
        }
        inline Degree<T> operator * (const Degree<T> &Other) const
        {
            return Degree(Degree_ * Other.Degree_);
        }
        inline Degree<T> operator / (const Degree<T> &Other) const
        {
            return Degree(Degree_ / Other.Degree_);
        }
        
        inline bool operator > (const Degree<T> &Other) const
        {
            return Degree_ > Other.Degree_;
        }
        inline bool operator >= (const Degree<T> &Other) const
        {
            return Degree_ >= Other.Degree_;
        }
        inline bool operator < (const Degree<T> &Other) const
        {
            return Degree_ < Other.Degree_;
        }
        inline bool operator <= (const Degree<T> &Other) const
        {
            return Degree_ <= Other.Degree_;
        }
        
    private:
        
        /* === Functions === */
        
        inline void clamp()
        {
            Degree_ -= static_cast<T>(std::floor(Degree_ / T(360)) * T(360));
        }
        
        /* === Members === */
        
        T Degree_;
        
};


//! Radian class. An instance of this class will always store a value in the range [0.0 .. 2*pi).
template <typename T> class Radian : public RadianDegreeAngle<T>
{
    
    public:
        
        Radian(const T &Rad = T(0)) :
            Radian_(Rad)
        {
            clamp();
        }
        Radian(const Radian<T> &Rad) :
            Radian_(Rad.Radian_)
        {
        }
        Radian(const Degree<T> &Deg) :
            Radian_(Deg.getRadian())
        {
        }
        ~Radian()
        {
        }
        
        /* === Functions === */
        
        T getDegree() const
        {
            return Radian_ * static_cast<T>(180.0 / M_PI);
        }
        T getRadian() const
        {
            return Radian_;
        }
        
        /* === Operators === */
        
        inline Radian<T>& operator = (const Radian<T> &Other)
        {
            Radian_ = Other.Radian_;
            return *this;
        }
        
        inline Radian<T>& operator += (const Radian<T> &Other)
        {
            Radian_ += Other.Radian_;
            clamp();
            return *this;
        }
        inline Radian<T>& operator -= (const Radian<T> &Other)
        {
            Radian_ -= Other.Radian_;
            clamp();
            return *this;
        }
        inline Radian<T>& operator *= (const Radian<T> &Other)
        {
            Radian_ *= Other.Radian_;
            clamp();
            return *this;
        }
        inline Radian<T>& operator /= (const Radian<T> &Other)
        {
            Radian_ /= Other.Radian_;
            clamp();
            return *this;
        }
        
        inline Radian<T> operator + (const Radian<T> &Other) const
        {
            return Radian(Radian_ + Other.Radian_);
        }
        inline Radian<T> operator - (const Radian<T> &Other) const
        {
            return Radian(Radian_ - Other.Radian_);
        }
        inline Radian<T> operator * (const Radian<T> &Other) const
        {
            return Radian(Radian_ * Other.Radian_);
        }
        inline Radian<T> operator / (const Radian<T> &Other) const
        {
            return Radian(Radian_ / Other.Radian_);
        }
        
        inline bool operator > (const Radian<T> &Other) const
        {
            return Radian_ > Other.Radian_;
        }
        inline bool operator >= (const Radian<T> &Other) const
        {
            return Radian_ >= Other.Radian_;
        }
        inline bool operator < (const Radian<T> &Other) const
        {
            return Radian_ < Other.Radian_;
        }
        inline bool operator <= (const Radian<T> &Other) const
        {
            return Radian_ <= Other.Radian_;
        }
        
    private:
        
        /* === Functions === */
        
        inline void clamp()
        {
            Radian_ -= static_cast<T>(std::floor(Radian_ / T(2.0*M_PI)) * T(2.0*M_PI));
        }
        
        /* === Members === */
        
        T Radian_;
        
};


template <typename T> Degree<T>::Degree(const Radian<T> &Rad) :
    Degree_(Rad.getDegree())
{
}


} // /namespace math

} // /namespace sp


#endif



// ================================================================================
