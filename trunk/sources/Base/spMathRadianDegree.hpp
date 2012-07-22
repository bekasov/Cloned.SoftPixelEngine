/*
 * Radian and degree header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_MATH_RADIANDEGREE_H__
#define __SP_MATH_RADIANDEGREE_H__


#include "Base/spStandard.hpp"


namespace sp
{
namespace math
{


class Degree;
class Radian;

class RadianDegreeAngle
{
	
	public:
		
		RadianDegreeAngle()
		{
		}
		virtual ~RadianDegreeAngle()
		{
		}
		
		virtual f32 getDegree() const = 0;
		virtual f32 getRadian() const = 0;
		
};

//! Degree class. An instance of this class will always store a value in the range [0.0 .. 360.0).
class Degree : public RadianDegreeAngle
{
	
	public:
		
		Degree(f32 init = 0) : Degree_(init)
		{
			clamp();
		}
		Degree(const Degree &init) : Degree_(init.Degree_)
		{
		}
		Degree(const Radian &init);
		~Degree()
		{
		}
		
		inline Degree& operator = (const Degree &other)
		{
			Degree_ = other.Degree_;
			return *this;
		}
		
		inline Degree operator + (const Degree &other) const
		{
			return Degree(Degree_ + other.Degree_);
		}
		inline Degree& operator += (const Degree &other)
		{
			Degree_ += other.Degree_;
			clamp();
			return *this;
		}
		
		inline Degree operator - (const Degree &other) const
		{
			return Degree(Degree_ - other.Degree_);
		}
		inline Degree& operator -= (const Degree &other)
		{
			Degree_ -= other.Degree_;
			clamp();
			return *this;
		}
		
		inline Degree operator * (const Degree &other) const
		{
			return Degree(Degree_ * other.Degree_);
		}
		inline Degree& operator *= (const Degree &other)
		{
			Degree_ *= other.Degree_;
			clamp();
			return *this;
		}
		
		inline Degree operator / (const Degree &other) const
		{
			return Degree(Degree_ / other.Degree_);
		}
		inline Degree& operator /= (const Degree &other)
		{
			Degree_ /= other.Degree_;
			clamp();
			return *this;
		}
		
		inline bool operator > (const Degree &other) const
		{
			return Degree_ > other.Degree_;
		}
		inline bool operator >= (const Degree &other) const
		{
			return Degree_ >= other.Degree_;
		}
		inline bool operator < (const Degree &other) const
		{
			return Degree_ < other.Degree_;
		}
		inline bool operator <= (const Degree &other) const
		{
			return Degree_ <= other.Degree_;
		}
		
		inline bool operator == (const Degree &other) const
		{
			return Degree_ == other.Degree_;
		}
		inline bool operator != (const Degree &other) const
		{
			return Degree_ != other.Degree_;
		}
		
		f32 getDegree() const
		{
			return Degree_;
		}
		f32 getRadian() const
		{
			return Degree_ * M_PI / 180;
		}
		
	private:
		
		/* Functions */
		
		inline void clamp()
		{
			Degree_ -= floor(Degree_ / 360) * 360;
		}
		
		/* Members */
		
		f32 Degree_;
		
};

//! Radian class. An instance of this class will always store a value in the range [0.0 .. 2*pi).
class Radian : public RadianDegreeAngle
{
	
	public:
		
		Radian(f32 init = 0) : Radian_(init)
		{
			clamp();
		}
		Radian(const Radian &init) : Radian_(init.Radian_)
		{
		}
		Radian(const Degree &init) : Radian_(init.getRadian())
		{
		}
		~Radian()
		{
		}
		
		inline Radian& operator = (const Radian &other)
		{
			Radian_ = other.Radian_;
			return *this;
		}
		
		inline Radian operator + (const Radian &other) const
		{
			return Radian(Radian_ + other.Radian_);
		}
		inline Radian& operator += (const Radian &other)
		{
			Radian_ += other.Radian_;
			clamp();
			return *this;
		}
		
		inline Radian operator - (const Radian &other) const
		{
			return Radian(Radian_ - other.Radian_);
		}
		inline Radian& operator -= (const Radian &other)
		{
			Radian_ -= other.Radian_;
			clamp();
			return *this;
		}
		
		inline Radian operator * (const Radian &other) const
		{
			return Radian(Radian_ * other.Radian_);
		}
		inline Radian& operator *= (const Radian &other)
		{
			Radian_ *= other.Radian_;
			clamp();
			return *this;
		}
		
		inline Radian operator / (const Radian &other) const
		{
			return Radian(Radian_ / other.Radian_);
		}
		inline Radian& operator /= (const Radian &other)
		{
			Radian_ /= other.Radian_;
			clamp();
			return *this;
		}
		
		inline bool operator > (const Radian &other) const
		{
			return Radian_ > other.Radian_;
		}
		inline bool operator >= (const Radian &other) const
		{
			return Radian_ >= other.Radian_;
		}
		inline bool operator < (const Radian &other) const
		{
			return Radian_ < other.Radian_;
		}
		inline bool operator <= (const Radian &other) const
		{
			return Radian_ <= other.Radian_;
		}
		
		inline bool operator == (const Radian &other) const
		{
			return Radian_ == other.Radian_;
		}
		inline bool operator != (const Radian &other) const
		{
			return Radian_ != other.Radian_;
		}
		
		f32 getDegree() const
		{
			return Radian_ * 180 / M_PI;
		}
		f32 getRadian() const
		{
			return Radian_;
		}
		
	private:
		
		/* Functions */
		
		inline void clamp()
		{
			Radian_ -= floor(Radian_ / (2*M_PI)) * (2*M_PI);
		}
		
		/* Members */
		
		f32 Radian_;
		
};


} // /namespace math

} // /namespace sp


#endif



// ================================================================================
