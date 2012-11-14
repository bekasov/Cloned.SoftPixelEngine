/*
 * Animation skeleton header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_ANIMATION_SKELETON_H__
#define __SP_ANIMATION_SKELETON_H__


#include "Base/spStandard.hpp"
#include "SceneGraph/spSceneMesh.hpp"
#include "SceneGraph/Animation/spAnimationJoint.hpp"
#include "SceneGraph/Animation/spAnimationBaseStructures.hpp"

#include <vector>


namespace sp
{
namespace scene
{


/**
Animation skeletons are constructed out of animation joints. It forms the foundation of a skeletal animation.
\ingroup group_animation
*/
class SP_EXPORT AnimationSkeleton
{
    
    public:
        
        AnimationSkeleton();
        virtual ~AnimationSkeleton();
        
        /* === Functions === */
        
        /**
        Creates a new AnimationJoint object and adds it into the skeleton graph.
        \param OriginTransform: Specifies the origin transformation.
        \param Name: Specifies the joint's name.
        \param Parent: Specifies the joint parent.
        \return Pointer to the new AnimationJoint object.
        */
        AnimationJoint* createJoint(
            const Transformation &OriginTransform, const io::stringc &Name = "", AnimationJoint* Parent = 0
        );
        
        //! Deletes the specified joint from the list.
        void deleteJoint(AnimationJoint* Joint);
        
        //! Returns a pointer to the first joint with the specified name.
        AnimationJoint* findJoint(const io::stringc &Name);
        
        /**
        Sets the new joint parent and automatically updates the children list.
        You have to call "updateSkeleton" after changing the parent hierarchy!
        \param Joint: Specifies the joint which will get the new parnet.
        \param Parent: Specifies the new AnimationJoint parent object. Can also be 0.
        */
        void setJointParent(AnimationJoint* Joint, AnimationJoint* Parent);
        
        /**
        Stores all surfaces used by the joints in a unique list.
        This should be called after all joints have been created.
        */
        void updateSkeleton();
        
        /**
        Transforms the vertices (given by the vertex groups surface) by the current skeleton transformation.
        Each joint has a 'origin transformation' and a 'current transformation'. If these two
        transformation of each joint are equal the mesh trnsformation has no effect.
        */
        void transformVertices();
        
        /**
        Renders the skeleton as a wire mesh. Call this function inside a 'beginDrawing2D' and 'endDrawing2D'
        block of your render system (RenderSystem object).
        \param BaseMatrix: Specifies the base matrix transformation. Use the object's global location (SceneNode::getGlobalLcoation()).
        \param Color: Specifies the color which is to be used to render the skeleton.
        */
        virtual void render(const dim::matrix4f &BaseMatrix, const video::color &Color = video::color(255, 255, 0));
        
        /* === Inline fuctions === */
        
        //! Returns the list of all animation joints.
        inline std::list<AnimationJoint*> getJointList() const
        {
            return Joints_;
        }
        
    protected:
        
        /* === Functions === */
        
        virtual void drawJointConnections(
            AnimationJoint* Joint, dim::matrix4f BaseMatrix, const video::color &Color
        );
        virtual void drawJointConnector(const dim::matrix4f &Matrix, const video::color &Color);
        
    private:
        
        /* === Members === */
        
        std::vector<AnimationJoint*> RootJoints_;   //!< Root joints don't have a parent.
        std::list<AnimationJoint*> Joints_;         //!< All joints of this skeleton.
        
        std::list<video::MeshBuffer*> Surfaces_;    //!< Unique list of all surfaces.
        
};


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
