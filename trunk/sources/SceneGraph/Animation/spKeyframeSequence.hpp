/*
 * Keyframe sequence header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_KEYFRAME_SEQUENCE_H__
#define __SP_KEYFRAME_SEQUENCE_H__


#include "Base/spStandard.hpp"
#include "Base/spTransformation3D.hpp"

#include <vector>


namespace sp
{
namespace scene
{


/**
This is the animation keyframe sequence class. It holds all keyframe transformations
for a node object which can be a scene node or a bone.
\ingroup group_animation
*/
class SP_EXPORT KeyframeSequence
{
    
    public:
        
        KeyframeSequence();
        ~KeyframeSequence();
        
        /* === Functions === */
        
        /**
        Adds a new keyframe transformation.
        \param Transform: Specifies the transformation for the new keyframe.
        \param Frame: Specifies the frame index. When this index is more 1 greater than the last index the
        keyframes between these values will be interpolated and also be added. But only the keyframes
        you add manual are so called 'root' keyframes. You can only remove those root keyframes.
        All the other interpolated keyframes will be removed automatically.
        */
        void addKeyframe(const Transformation &Transform, u32 Frame);
        
        //! Removes the specified keyframe if this is a 'root' keyframe i.e. you previously added it.
        void removeKeyframe(u32 Frame);
        
        /* === Inline functions === */
        
        //! Returns the specified keyframe as constant reference. This function does not check if the index is out of bounds!
        inline const Transformation& getKeyframe(u32 Frame) const
        {
            return Keyframes_[Frame];
        }
        //! Returns the specified keyframe as reference. This function does not check if the index is out of bounds!
        inline Transformation& getKeyframe(u32 Frame)
        {
            return Keyframes_[Frame];
        }
        
        /**
        Stores an interpolated transformation between the two given frames in the result parameter.
        \param Result: Specifies the variable where the result is to be stored.
        \param From: Specifies the start frame index.
        \param To: Specifies the end frame index.
        \param Interpolation: Specifies the interpolation factor [0.0 .. 1.0].
        */
        inline void interpolate(Transformation &Result, u32 From, u32 To, f32 Interpolation)
        {
            const u32 FrameCount = Keyframes_.size();
            if (From < FrameCount && To < FrameCount)
                Result.interpolate(Keyframes_[From], Keyframes_[To], Interpolation);
        }
        
        //! Returns the keyframe size.
        inline u32 getKeyframeCount() const
        {
            return Keyframes_.size();
        }
        
    private:
        
        /* === Functions === */
        
        void findRootFrameRange(u32 Frame, u32* LeftFrame, u32* RightFrame);
        
        void pushBackKeyframe(const Transformation &Transform, u32 Frame);
        void insertKeyframe(const Transformation &Transform, u32 Frame);
        
        void popBackKeyframe(u32 Frame);
        void extractKeyframe(u32 Frame);
        
        /* === Members === */
        
        std::vector<Transformation> Keyframes_;
        std::vector<bool> RootKeyframes_;               //!< Specifies whether the given frame is a root keyframe or not.
};


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
