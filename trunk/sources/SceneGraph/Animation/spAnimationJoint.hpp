/*
 * Animation bone header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_ANIMATION_BONE_H__
#define __SP_ANIMATION_BONE_H__


#include "Base/spStandard.hpp"
#include "Base/spInputOutputString.hpp"
#include "Base/spDimension.hpp"
#include "Base/spMemoryManagement.hpp"
#include "SceneGraph/spSceneMesh.hpp"
#include "Base/spTransformation3D.hpp"
#include "SceneGraph/Animation/spAnimationBaseStructures.hpp"

#include <vector>


namespace sp
{
namespace scene
{


class AnimationSkeleton;


/**
Animation joints are the foundation of animation skeletons (used for skeletal animations).
\see AnimationSkeleton.
\ingroup group_animation
*/
class SP_EXPORT AnimationJoint : public BaseObject
{
    
    public:
        
        AnimationJoint(
            const Transformation &OriginTransform, const io::stringc Name = ""
        );
        virtual ~AnimationJoint();
        
        /* === Functions === */
        
        //! Returns the global joint transformation.
        virtual dim::matrix4f getGlobalTransformation() const;
        //! Returns the final vertex transformation. Use this to transform the vertices by yourself.
        virtual dim::matrix4f getVertexTransformation() const;
        
        /* === Inline functions === */
        
        /**
        Enables or disables the bone. If disabled the bone won't be transformed automatically by the animation
        but you can still transform it manual. By default enabled.
        \todo RENAME this to "setAutoTransform"
        */
        inline void setEnable(bool Enable) // !TODO! -> RENAME this to "setAutoTransform"
        {
            isEnable_ = Enable;
        }
        inline bool getEnable() const
        {
            return isEnable_;
        }
        
        //! Returns a pointer to the AnimationJoint parent object.
        inline AnimationJoint* getParent() const
        {
            return Parent_;
        }
        
        //! Sets the vertex groups list.
        inline void setVertexGroups(const std::vector<SVertexGroup> &VertexGroups)
        {
            VertexGroups_ = VertexGroups;
        }
        
        inline const std::vector<SVertexGroup>& getVertexGroups() const
        {
            return VertexGroups_;
        }
        inline std::vector<SVertexGroup>& getVertexGroups()
        {
            return VertexGroups_;
        }
        
        //! Sets the new (local) origin transformation. Internally the origin transformation is stored as inverse matrix.
        inline void setOriginTransformation(const Transformation &Transform)
        {
            OriginTransform_ = Transform;
        }
        inline Transformation getOriginTransformation() const
        {
            return OriginTransform_;
        }
        
        //! Sets the new (local) current transformation.
        inline void setTransformation(const Transformation &Transform)
        {
            Transform_ = Transform;
        }
        inline const Transformation& getTransformation() const
        {
            return Transform_;
        }
        inline Transformation& getTransformation()
        {
            return Transform_;
        }
        
        //! Returns the final origin matrix which will be computed when the "AnimationSkeleton::updateSkeleton" function is called.
        inline dim::matrix4f getOriginMatrix() const
        {
            return OriginMatrix_;
        }
        
        //! Returns the joint children list.
        inline const std::vector<AnimationJoint*>& getChildren() const
        {
            return Children_;
        }
        
    protected:
        
        friend class AnimationSkeleton;
        
        /* === Functions === */
        
        /**
        Transforms the vertices of the specified Mesh object. Which vertices will be transformed depends on
        the bone's vertex groups.
        \param[in] MeshObj Specifies the mesh object which vertices are to be transformed. This pointer must never be null!
        \param[in] BaseMatrix: Specifies the base matrix transformation. If the bone has a parent this matrix should be
        its parent's matrix transformation.
        \param[in] useTangentSpace: Specifies whether tanget space is used or not.
        \note The pointer is not checked for validity!
        */
        void transformVertices(scene::Mesh* MeshObj, dim::matrix4f BaseMatrix, bool useTangentSpace) const;
        
        bool checkParentIncest(AnimationJoint* Joint) const;
        
        /* === Inline functions === */
        
        inline void setParent(AnimationJoint* Parent)
        {
            Parent_ = Parent;
        }
        
        inline void addChild(AnimationJoint* Child)
        {
            Children_.push_back(Child);
        }
        inline void removeChild(AnimationJoint* Child)
        {
            MemoryManager::removeElement(Children_, Child);
        }
        
    private:
        
        /* === Members === */
        
        bool isEnable_;
        
        AnimationJoint* Parent_;
        std::vector<AnimationJoint*> Children_;
        
        Transformation OriginTransform_;    //!< Origin transformation.
        Transformation Transform_;          //!< Current transformation.
        
        dim::matrix4f OriginMatrix_;        //!< Final origin transformation matrix. Stored as inverse matrix for combining with the current transformation.
        
        std::vector<SVertexGroup> VertexGroups_;
        
};


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
