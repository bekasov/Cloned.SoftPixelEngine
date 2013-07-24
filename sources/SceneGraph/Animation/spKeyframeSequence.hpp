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


enum EKeyframeFlags
{
    KEYFRAME_POSITION   = 0x01,
    KEYFRAME_ROTATION   = 0x02,
    KEYFRAME_SCALE      = 0x04,
    KEYFRAME_ALL        = KEYFRAME_POSITION | KEYFRAME_ROTATION | KEYFRAME_SCALE,
};


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
        
        #if 1 //!!!
        
        void addKeyPosition(u32 Frame, const dim::vector3df &Position);
        void addKeyRotation(u32 Frame, const dim::quaternion &Rotation);
        void addKeyScale(u32 Frame, const dim::vector3df &Scale);
        
        void addKeyframe(u32 Frame, s32 _deb_ToBeremoved_, const Transformation &Transform, s32 Flags = KEYFRAME_ALL);
        
        /**
        Removes the specified keyframe. Any keyframe (position, rotation or scale) must have been added
        previously at the same frame index.
        \param[in] Frame Specifies the frame index.
        \param[in] Flags Specifies the keys which are to be removed.
        Use a combination of KEYFRAME_POSITION, KEYFRAME_ROTATION and KEYFRAME_SCALE.
        \return True if a keyframe could be removed. Otherwise the frame index was invalid or the flags bit-mask was zero.
        \see EKeyframeFlags
        */
        bool removeKeyframe(u32 Frame, s32 _deb_ToBeremoved_, s32 Flags = KEYFRAME_ALL);
        
        /**
        Updates or rather finally constructs the transformations out of all added keyframes.
        This is similar to the "AnimationSkeleton::updateSkeleton" function to finalize the construction.
        \return True if the sequence has been updated. Otherwise an update was not necessary.
        \code
        // Add all your keyframes.
        AnimSeq->addKeyframe(0, ...);
        AnimSeq->addKeyframe(5, ...);
        AnimSeq->addKeyframe(12, ...);
        AnimSeq->addKeyframe(27, ...);
        
        // Finalize the sequence by updating it.
        AnimSeq->updateSequence();
        \endcode
        \see modified
        */
        bool updateSequence();
        
        /**
        Enables or disables immediate update mode. Use this to let the engine update the sequence
        every time you add a new or remove an old keyframe.
        \param[in] Enable Specifies whether the immediate update mode is to be enabled or disabled. By default disabled.
        If this parameter is true and the sequence was modified previously it will be updated with this function call.
        \note Updating the sequence every time it is modified can be very slow!
        Only use this for real-time editing purposes.
        */
        void setUpdateImmediate(bool Enable);
        
        #endif
        
        /**
        Adds a new keyframe transformation.
        \param[in] Frame Specifies the frame index. When this index is more 1 greater than the last index the
        \param[in] Transform Specifies the transformation for the new keyframe.
        keyframes between these values will be interpolated and also be added. But only the keyframes
        you add manual are so called 'root' keyframes. You can only remove those root keyframes.
        All the other interpolated keyframes will be removed automatically.
        */
        void addKeyframe(u32 Frame, const Transformation &Transform);
        
        //! Removes the specified keyframe if this is a 'root' keyframe i.e. you previously added it.
        void removeKeyframe(u32 Frame);
        
        /* === Inline functions === */
        
        /**
        Returns the specified keyframe as constant reference. This function does not check if the index is out of bounds!
        Use "getKeyframeCount" to determine the range of keyframes.
        */
        inline const Transformation& getKeyframe(u32 Frame) const
        {
            return Keyframes_[Frame];
        }
        /**
        Returns the specified keyframe as reference. This function does not check if the index is out of bounds!
        Use "getKeyframeCount" to determine the range of keyframes.
        */
        inline Transformation& getKeyframe(u32 Frame)
        {
            return Keyframes_[Frame];
        }
        
        /**
        Stores an interpolated transformation between the two given frames in the result parameter.
        \param[out] Result Specifies the variable where the result is to be stored.
        \param[in] From Specifies the start frame index.
        \param[in] To Specifies the end frame index.
        \param[in] Interpolation Specifies the interpolation factor [0.0 .. 1.0].
        */
        inline void interpolate(Transformation &Result, u32 From, u32 To, f32 Interpolation)
        {
            const u32 FrameCount = Keyframes_.size();
            if (From < FrameCount && To < FrameCount)
                Result.interpolate(Keyframes_[From], Keyframes_[To], Interpolation);
        }
        
        //! Returns the count of final keyframes.
        inline u32 getKeyframeCount() const
        {
            return Keyframes_.size();
        }
        
        //! Returns the minimal frame index. This is not used for the final frame transformations!
        inline u32 getMinKeyframe() const
        {
            return MinFrame_;
        }
        //! Returns the maximal frame index. This is not used for the final frame transformations!
        inline u32 getMaxKeyframe() const
        {
            return MaxFrame_;
        }
        
        //! Returns status of immediate sequence updating. By defautl false.
        inline bool getUpdateImmediate() const
        {
            return UpdateImmediate_;
        }
        
        /**
        Returns true if the sequence has been modified. In this case you have to update the sequence.
        This happens when a keyframe has been added or removed.
        \see updateSequence
        */
        inline bool modified() const
        {
            return Modified_;
        }
        
    private:
        
        /* === Structures === */
        
        #if 1 //!!!
        
        template <class VecT> struct SKey
        {
            SKey() :
                Frame(0)
            {
            }
            SKey(u32 FrameIndex, const VecT &KeyVec) :
                Frame   (FrameIndex ),
                Vec     (KeyVec     )
            {
            }
            ~SKey()
            {
            }
            
            /* Members */
            u32 Frame;
            VecT Vec;
        };
        
        typedef SKey<dim::vector3df> SKeyPos;
        typedef SKey<dim::quaternion> SKeyRot;
        typedef SKey<dim::vector3df> SKeyScl;
        
        #endif
        
        /* === Functions === */
        
        void findRootFrameRange(u32 Frame, u32* LeftFrame, u32* RightFrame);
        
        void pushBackKeyframe(const Transformation &Transform, u32 Frame);
        void insertKeyframe(const Transformation &Transform, u32 Frame);
        
        void popBackKeyframe(u32 Frame);
        void extractKeyframe(u32 Frame);
        
        #if 1 //!!!
        
        void updateFrameRangeComplete();
        void markAsModified();
        
        #endif
        
        /* === Templates === */
        
        template <typename T> void insertKey(std::vector<T> &Keyframes, const T &Key)
        {
            /* Presuem that new keyframes will be added from left to right (smaller frame indices up to higher ones).
               So insert the new key before the first existing key whose frame is smaller that the new one's.
               By iterating from front to back in the list. */
            typename std::vector<T>::reverse_iterator it = Keyframes.rbegin();
            
            while (it != Keyframes.rend() && it->Frame > Key.Frame)
                ++it;
            
            if (it != Keyframes.rend() && it->Frame == Key.Frame)
                it->Vec = Key.Vec;
            else
            {
                /* Insert new keyframe */
                Keyframes.insert(it.base(), Key);
                
                /* Update minimal- and maximal frame indices */
                math::decrease(MinFrame_, Key.Frame);
                math::increase(MaxFrame_, Key.Frame);
            }
        }
        
        template <typename T> bool removeKey(std::vector<T> &Keyframes, u32 Frame)
        {
            for (typename std::vector<T>::iterator it = Keyframes.begin(); it != Keyframes.end(); ++it)
            {
                if (it->Frame == Frame)
                {
                    Keyframes.erase(it);
                    return true;
                }
            }
            return false;
        }
        
        template <typename T> void updateFrameRange(std::vector<T> &Keyframes)
        {
            for (typename std::vector<T>::iterator it = Keyframes.begin(); it != Keyframes.end(); ++it)
            {
                math::decrease(MinFrame_, it->Frame);
                math::increase(MaxFrame_, it->Frame);
            }
        }
        
        template <typename T> void filterMinFrame(std::vector<T> &Keyframes)
        {
            if (!Keyframes.empty())
            {
                const u32 Frame = Keyframes.back().Frame;
                if (MinFrame_ > Frame)
                    MinFrame_ = Frame;
            }
        }
        
        template <typename T> void filterMaxFrame(std::vector<T> &Keyframes)
        {
            if (!Keyframes.empty())
            {
                const u32 Frame = Keyframes.front().Frame;
                if (MaxFrame_ < Frame)
                    MaxFrame_ = Frame;
            }
        }
        
        //! Receives the first two interpolation vectors (from- and to vectors).
        template <typename T, class VecT> typename std::vector<T>::iterator getFirstInterpVectors(
            std::vector<T> &Keyframes, VecT &FromVec, VecT &ToVec, u32 &FromIndex, u32 &ToIndex)
        {
            typename std::vector<T>::iterator it = Keyframes.begin();
            
            if (it != Keyframes.end())
            {
                FromVec     = it->Vec;
                FromIndex   = it->Frame;
                
                ++it;
                if (it != Keyframes.end())
                {
                    ToVec   = it->Vec;
                    ToIndex = it->Frame;
                }
                else
                {
                    ToVec   = FromVec;
                    ToIndex = FromIndex;
                }
            }
            
            ++ToIndex;
            
            return it;
        }
        
        //! Get next interpolation iterators
        template <typename T, class VecT> void getNextInterpIterator(
            std::vector<T> &Keyframes, typename std::vector<T>::iterator &it, u32 Frame,
            VecT &FromVec, VecT &ToVec, u32 &FromIndex, u32 &ToIndex)
        {
            if (it != Keyframes.end() && Frame >= it->Frame)
            {
                ++it;
                
                FromVec     = ToVec;
                FromIndex   = Frame;
                
                if (it != Keyframes.end())
                {
                    ToVec   = it->Vec;
                    ToIndex = it->Frame + 1;
                }
                else
                    ToIndex = Frame + 1;
            }
        }
        
        /* === Members === */
        
        //std::vector<Transformation> Keyframes_;
        std::vector<bool> RootKeyframes_;               //!< Specifies whether the given frame is a root keyframe or not.
        
        #if 1 //!!!
        
        std::vector<SKeyPos> ConstructKeysPos_;
        std::vector<SKeyRot> ConstructKeysRot_;
        std::vector<SKeyScl> ConstructKeysScl_;
        
        std::vector<Transformation> Keyframes_;
        
        u32 MinFrame_, MaxFrame_;
        
        bool Modified_;
        bool UpdateImmediate_;
        
        #endif
        
};


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
