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


/* Pre-declarations */

class SceneGraph;


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
        
        virtual void updateTransformation();
        
        /**
        Sets the billboard's base position.
        \param Position: Position to which the sprite origin is to be translated.
        */
        inline void setBasePosition(const dim::point2df &Position)
        {
            BasePosition_ = Position;
        }
        inline dim::point2df getBasePosition() const
        {
            return BasePosition_;
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
        
        //! Sets the billboard's color.
        inline void setColor(const video::color &Color)
        {
            Color_ = Color;
        }
        inline video::color getColor() const
        {
            return Color_;
        }
        
        //! Sets the hardware instances. For more information see the MeshBuffer class.
        inline void setHardwareInstancing(s32 InstanceCount)
        {
            InstanceCount_ = InstanceCount;
        }
        inline s32 getHardwareInstancing() const
        {
            return InstanceCount_;
        }
        
        /* Rendering & copying */
        
        Billboard* copy() const;
        
        virtual void render();
        
    protected:
        
        //friend bool cmpObjectBillboards(Billboard* &obj1, Billboard* &obj2);
        friend class SceneGraph;
        friend class SimpleSceneManager;
        friend class sp::SoftPixelDevice;
        
        /* Functions */
        
        static void createDefaultMeshBuffer();
        static void deleteDefaultMeshBuffer();
        
        /* === Members === */
        
        video::Texture* BaseTexture_;
        dim::point2df BasePosition_;
        video::color Color_;
        
        s32 InstanceCount_;
        
        static video::MeshBuffer* MeshBuffer_;
        
};


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
