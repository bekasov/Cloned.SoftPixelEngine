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
        \param[in] MeshObj Specifies the mesh object which is to be transformed. This mesh should have the same
        count of mesh buffers with the same count of vertices and triangles as the base mesh used when the skeleton was created.
        */
        void transformVertices(Mesh* MeshObj) const;
        
        /**
        Fills all joint transformations into the given matrix list.
        \param[in,out] JointMatrices Specifies the container which is to be filled with the joint transformations.
        \param[in] KeepJointOrder Specifies whether the joint order is to be kept or not.
        If true each joint matrix is stored at the same index as the joint has been created for the skeleton.
        Use this when you need the transformations for hardware accelerated animation. If false the
        transformations can be computed a little faster but the order is arbitrary. By default true.
        \note This will not resize the container! Initialize the container with the maximum size,
        e.g. count of joints.
        */
        void fillJointTransformations(std::vector<dim::matrix4f> &JointMatrices, bool KeepJointOrder = true) const;
        
        /**
        Sets up the vertex buffer attributes of the specified mesh to use this skeleton for hardware accelerated animation.
        The final vertex shader must be written by yourself, but the workaround to setup the indices and joint weights
        for each vertex can be calculated by this function.
        \param[in] MeshObj Specifies the mesh object whose vertex buffers are to be set up.
        \param[in] IndexAttributes Specifies the list of vertex attributes for the joint indices.
        \param[in] WeightAttributes Specifies the list of vertex attributes for the joint weights.
        \return True if the vertex buffers could be set up successful. Otherwise an error has occured.
        \note 'IndexAttributes' and 'WeightAttributes' must have the same count of elements.
        It's also recommended to use 4-component float vectors for these attributes.
        \code
        // Create a universal vertex format.
        video::VertexFormatUniversal* VertFmt = spRenderer->createVertexFormat<video::VertexFormatUniversal>();
        
        // Add the default components.
        VertFmt->addCoord();
        VertFmt->addNormal();
        VertFmt->addTexCoord();
        
        // Add the attribute for the indices. This is a 4 component vector, thus each vertex can be transformed by 4 joints at once.
        VertFmt->addTexCoord(video::DATATYPE_FLOAT, 4);
        // Add the attribute for the weights. Same as above.
        VertFmt->addTexCoord(video::DATATYPE_FLOAT, 4);
        
        // Create your shader.
        video::ShaderClass* AnimShaderClass = spRenderer->createShaderClass(VertFmt);
        //...
        
        // Setup the vertex buffers
        std::vector<video::SVertexAttribute> IndexAttributes, WeightAttributes;
        
        IndexAttributes.push_back(VertFmt->getTexCoord()[1]);
        WeightAttributes.push_back(VertFmt->getTexCoord()[2]);
        
        AnimSkeleton->setupVertexBuffers(AnimMesh, IndexAttributes, WeightAttributes);
        
        // Disable software per-vertex transformation.
        Anim->setFlags(scene::ANIMFLAG_NO_TRANSFORMATION);
        \endcode
        \see video::VertexFormatUniversal
        */
        bool setupVertexBufferAttributes(
            Mesh* MeshObj,
            const std::vector<video::SVertexAttribute> &IndexAttributes,
            const std::vector<video::SVertexAttribute> &WeightAttributes
        ) const;
        
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
        
        /* === Functions === */
        
        bool checkAttributeListForHWAnim(
            const std::vector<video::SVertexAttribute> &Attributes, const io::stringc &Name
        ) const;
        
        void fillSubJointTransformations(
            AnimationJoint* Joint, dim::matrix4f BaseMatrix,
            std::vector<dim::matrix4f> &JointMatrices, u32 &Index
        ) const;
        
        /* === Members === */
        
        std::vector<AnimationJoint*> RootJoints_;   //!< Root joints don't have a parent.
        std::list<AnimationJoint*> Joints_;         //!< All joints of this skeleton.
        
        //std::list<video::MeshBuffer*> Surfaces_;    //!< Unique list of all surfaces.
        
};


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
