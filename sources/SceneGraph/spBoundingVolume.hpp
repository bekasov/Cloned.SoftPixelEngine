/*
 * Bounding volume header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_SCENE_BOUNDINGVOLUME_H__
#define __SP_SCENE_BOUNDINGVOLUME_H__


#include "Base/spStandard.hpp"
#include "Base/spDimension.hpp"
#include "Base/spMath.hpp"
#include "Base/spViewFrustum.hpp"


namespace sp
{
namespace scene
{


//! Bounding volume types.
enum EBoundingVolumes
{
    BOUNDING_NONE,      //!< Not used.
    BOUNDING_SPHERE,    //!< Bounding sphere.
    BOUNDING_BOX,       //!< Bounding box.
};


class SP_EXPORT BoundingVolume
{
    
    public:
        
        BoundingVolume();
        BoundingVolume(const BoundingVolume &Other);
        ~BoundingVolume();
        
        /* === Functions === */
        
        /**
        Proceeds frustum-culling for more optimization. This function is used in the mesh's "render" function.
        \param Frustum: ViewFrustum of the active camera. Camera objects can be activated by the
        SceneManager using "SceneManager::setActiveCamera".
        \param Transformation: Frustum's matrix transformation. Use the object's global transformation.
        \return True if the object is inside the frustum otherwise false.
        */
        bool checkFrustumCulling(const scene::ViewFrustum &Frustum, const dim::matrix4f &Transformation) const;
        
        /* === Inline functions === */
        
        /**
        Sets the type of bounding box. This is used for frustum-culling only for Entity object.
        Use BOUNDING_NONE to disable frustum culling. Mostly used are bounding boxes.
        But bounding spheres are faster because of less calculation steps.
        \param Type: Type of the bounding volume.
        */
        inline void setType(const EBoundingVolumes Type)
        {
            Type_ = Type;
        }
        inline EBoundingVolumes getType() const
        {
            return Type_;
        }
        
        /**
        Sets the bounding box if the bounding type is BOUNDING_BOX.
        \param BoundBox: Axis-aligned-bounding-box which is to used for frustum-culling.
        */
        inline void setBox(const dim::aabbox3df &BoundBox)
        {
            Box_ = BoundBox;
        }
        inline const dim::aabbox3df& getBox() const
        {
            return Box_;
        }
        
        //! Sets the bounding-sphere radius if the bounding type is BOUNDING_SPHERE.
        inline void setRadius(const f32 Radius)
        {
            Radius_ = Radius;
        }
        inline f32 getRadius() const
        {
            return Radius_;
        }
        
    private:
        
        /* === Members === */
        
        EBoundingVolumes Type_;
        dim::aabbox3df Box_;
        f32 Radius_;
        
};


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
