/*
 * Billboard scene node header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_SCENE_BILLBOARD_H__
#define __SP_SCENE_BILLBOARD_H__


#include "Base/spStandard.hpp"
#include "Base/spMeshBuffer.hpp"
#include "SceneGraph/spMaterialNode.hpp"
#include "RenderSystem/spTextureBase.hpp"


namespace sp
{

class SoftPixelDevice;

namespace scene
{


class SceneGraph;


/**
Billboard transformation alignments.
\since Version 3.3
*/
enum EBillboardAlginments
{
    BILLBOARD_SCREEN_ALIGNED,       //!< The billboard is screen plane aligned. This is the default configuration.
    BILLBOARD_VIEWPOINT_ALIGNED,    //!< The billboard is view-point aligned. The rotation only changes if the camera moves.
    BILLBOARD_UPVECTOR_ALIGNED,     //!< Similar to 'BILLBOARD_VIEWPOINT_ALIGNED' but an up-vector is used.
};

/**
Billboards - also called "Sprites" - are used for special effects like fire, sparks, rain, lense flares etc.
A Billboard is always faced to the camera like a 2D image but it uses the 3D space (position with X, Y, Z).
A Billboard normally have only one texture but it can also be just a colored quad (two triangles in form of a square).
Inform yourself about alpha-blending that you know how to use the blending methods which are mostly used for Billboards
but also available for normal 3D models.
*/
class SP_EXPORT Billboard : public MaterialNode
{
    
    public:
        
        Billboard(video::Texture* BaseTexture);
        virtual ~Billboard();
        
        /* === Functions === */

        virtual void updateTransformation();
        
        Billboard* copy() const;
        
        virtual void render();
        
        /* === Inline functions === */

        /**
        Sets the base position.
        \param[in] Position Specifies the position to which the sprite origin is to be translated. By default (0, 0, 0).
        */
        inline void setBasePosition(const dim::vector3df &Position)
        {
            BasePosition_ = Position;
        }
        inline const dim::vector3df & getBasePosition() const
        {
            return BasePosition_;
        }

        /**
        Sets the base rotation.
        \param[in] Rotation Specifies the rotation angle (in degrees). By default 0.0.
        \since Version 3.3
        */
        inline void setBaseRotation(f32 Rotation)
        {
            BaseRotation_ = Rotation;
        }
        inline f32 getBaseRotation() const
        {
            return BaseRotation_;
        }
        
        //! Sets the base texture (may be 0).
        inline void setTexture(video::Texture* BaseTexture)
        {
            BaseTexture_ = BaseTexture;
        }
        inline video::Texture* getTexture() const
        {
            return BaseTexture_;
        }
        
        //! Sets the hardware instances. For more information see the MeshBuffer class.
        inline void setHardwareInstancing(u32 NumInstances)
        {
            NumInstances_ = NumInstances;
        }
        inline u32 getHardwareInstancing() const
        {
            return NumInstances_;
        }

        /**
        Sets the transformation alignment.
        \param[in] Alignment Specifies the new alignment. By default BILLBOARD_SCREEN_ALIGNED.
        If the new alignment is 'BILLBOARD_UPVECTOR_ALIGNED' you can also set the up-vector with 'setUpVector'.
        \see setUpVector
        \see EBillboardAlginments
        \since Version 3.3
        */
        inline void setAlignment(const EBillboardAlginments Alignment)
        {
            Alignment_ = Alignment;
        }
        //! Returns the transformation alignment.
        inline EBillboardAlginments getAlignment() const
        {
            return Alignment_;
        }

        /**
        Sets the new alignment up-vector for the 'BILLBOARD_UPVECTOR_ALIGNED' alginment type.
        \param[in] UpVector Specifies the new alignment up-vector. This vector will be normalized. By default (0, 1, 0).
        \see setAlignment
        \since Version 3.3
        */
        inline void setUpVector(const dim::vector3df &UpVector)
        {
            UpVector_ = UpVector;
            UpVector_.normalize();
        }
        //! Returns the alignment up-vector.
        inline const dim::vector3df& getUpVector() const
        {
            return UpVector_;
        }
        
    private:
        
        //friend bool cmpObjectBillboards(Billboard* &obj1, Billboard* &obj2);
        friend class SceneGraph;
        friend class SimpleSceneManager;
        friend class sp::SoftPixelDevice;
        
        /* === Members === */
        
        video::Texture* BaseTexture_;
        
        u32 NumInstances_;

        dim::vector3df BasePosition_;
        f32 BaseRotation_;

        EBillboardAlginments Alignment_;
        dim::vector3df UpVector_;
        
};


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
