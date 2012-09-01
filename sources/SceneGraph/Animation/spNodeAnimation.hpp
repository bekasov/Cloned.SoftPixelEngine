/*
 * Node animation header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_NODE_ANIMATION_H__
#define __SP_NODE_ANIMATION_H__


#include "Base/spStandard.hpp"
#include "Base/spMathSpline.hpp"
#include "SceneGraph/Animation/spAnimation.hpp"
#include "SceneGraph/Animation/spAnimationBaseStructures.hpp"

#include <vector>


namespace sp
{
namespace scene
{


/**
Node animation class. This animation supports only object transformation (position, rotation and scale).
No vertices will be modified. Moreover this is the only animation which supports spline-translation for
smooth movements.
\ingroup group_animation
*/
class SP_EXPORT NodeAnimation : public Animation
{
    
    public:
        
        NodeAnimation();
        virtual ~NodeAnimation();
        
        /* === Functions === */
        
        //! Adds a new keyframe at the end of the list.
        void addKeyframe(const Transformation &Transform, u32 Duration = 1000);
        
        //! Inserts the new keyframe at the specified index (i.e. when index is 0 it will be inserted at the front).
        void insertKeyframe(u32 Index, const Transformation &Transform, u32 Duration = 1000);
        
        //! Sets the new keyframe.
        void setKeyframe(u32 Index, const Transformation &Transform, u32 Duration = 1000);
        
        //! Removes the specified keyframe.
        void removeKeyframe(u32 Index);
        
        void clearKeyframes();
        
        virtual void setupManualAnimation(SceneNode* Node);
        
        virtual void updateAnimation(SceneNode* Node);
        
        virtual u32 getKeyframeCount() const;
        
        virtual void interpolate(u32 IndexFrom, u32 IndexTo, f32 Interpolation);
        
        /**
        Enables or disables the spline translation. If enabled the position's translation
        is processed using splines for smooth movements.
        */
        void setSplineTranslation(bool Enable);
        bool getSplineTranslation() const;
        
        /**
        Sets the spline's expansion.
        \param Expansion: Spline's expansion. By default 1.0.
        If 0.0 the animation looks the same when spline translations are disabled.
        \note This only works when the spline translation was already enabled!
        */
        void setSplineExpansion(const f32 Expansion);
        f32 getSplineExpansion() const;
        
    private:
        
        /* === Fucntions === */
        
        void updateSplineTrack(bool isRebuild);
        
        /* === Members === */
        
        std::vector<SNodeKeyframe> Keyframes_;
        
        SceneNode* Object_;
        
        math::Spline3D* Spline_;
        
};


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
