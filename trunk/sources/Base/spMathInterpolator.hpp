/*
 * Interpolator header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_MATH_INTERPOLATOR_H__
#define __SP_MATH_INTERPOLATOR_H__


#include "Base/spMath.hpp"
#include "Base/spTimer.hpp"


namespace sp
{
namespace math
{


/**
Interface for the interpolator classes
\see FunctionalInterpolator
*/
class Interpolator
{
    
    public:
        
        virtual ~Interpolator()
        {
        }
        
        virtual void update() = 0;
        
        virtual bool playing() const = 0;
        
    protected:
        
        Interpolator()
        {
        }
        
};


namespace InterpolatorManager
{

/**
Adds a new interpolation instance to the list. This will be called automatically
when you 'play' an interpolation 'animation'.
*/
SP_EXPORT void add(Interpolator* Interp);

/**
Removes an interpolation instance from the list. This will be called automatically
when the interpolation 'animation' is being updated and has just finished.
*/
SP_EXPORT void remove(Interpolator* Interp);

//! Updates all interpolation instances. Call this function every frame only once.
SP_EXPORT void update();

} // /namespace InterpolatorManager


/**
The functional interpolator class can be used to make comfortable interpolations.
You can pass an interpolation function as template parameter. There are several predefined functions.
\tparam T Specifies the type of the points (e.g. float, dim::point2df, dim::vector3df etc.).
\tparam I Specifies the interpolation data type. This should be a float or a double.
\tparam Func Specifies the interpolation function. This can be "Lerp", "LerpParabolic" or "LerpSin".
\see LinearInterpolator
\see ParabolicInterpolator
\see SinInterpolator
\see Lerp
\see LerpParabolic
\see LerpSin
\since Version 3.2
*/
template <typename T, typename I, T Func(const T&, const T&, const I&)>
    class FunctionalInterpolator : public Interpolator
{
    
    public:
        
        FunctionalInterpolator(const T &Min = T(0), const T &Max = T(1)) :
            Min_        (Min    ),
            Max_        (Max    ),
            State_      (I(0)   ),
            Speed_      (I(0)   ),
            Playing_    (false  ),
            Finished_   (false  )
        {
        }
        ~FunctionalInterpolator()
        {
            InterpolatorManager::remove(this);
        }
        
        /* === Functions === */
        
        void play(const I &Speed)
        {
            Playing_    = true;
            Finished_   = false;
            Speed_      = Speed;
            State_      = I(0);
            
            InterpolatorManager::add(this);
        }
        
        void play(u64 Duration)
        {
            play(static_cast<I>(1000.0 / (io::Timer::getFPS() * Duration)));
        }
        
        void stop(bool Reset = false)
        {
            Playing_ = false;
            if (Reset)
                State_ = I(0);
        }
        
        void update()
        {
            if (Playing_)
            {
                State_ += Speed_;
                
                if (State_ >= I(1))
                {
                    State_      = I(1);
                    Finished_   = true;
                    
                    stop();
                }
            }
        }
        
        T get() const
        {
            return Func(Min_, Max_, State_);
        }
        
        //! Returns true if the interpolation 'animation' is currently being played.
        bool playing() const
        {
            return Playing_;
        }
        
        /* === Inline functions === */
        
        //! Returns true if the interpolation 'animation' has finished.
        inline bool finished() const
        {
            return Finished_;
        }
        
        inline void setState(const I &State)
        {
            State_ = State;
        }
        inline const I& getState() const
        {
            return State_;
        }
        
        inline void setMin(const T &Min)
        {
            Min_ = Min;
        }
        inline const T& getMin() const
        {
            return Min_;
        }
        
        inline void setMax(const T &Max)
        {
            Max_ = Max;
        }
        inline const T& getMax() const
        {
            return Max_;
        }
        
    private:
        
        /* === Members === */
        
        T Min_, Max_;
        I State_;       //!< Interpolation state. This is always in the range [0.0 .. 1.0].
        I Speed_;
        
        bool Playing_;
        bool Finished_;
        
};

typedef FunctionalInterpolator<f32, f32, &Lerp> LinearInterpolator;
typedef FunctionalInterpolator<f32, f32, &LerpParabolic> ParabolicInterpolator;
typedef FunctionalInterpolator<f32, f32, &LerpSin> SinInterpolator;


} // /namespace math

} // /namespace sp


#endif



// ================================================================================
