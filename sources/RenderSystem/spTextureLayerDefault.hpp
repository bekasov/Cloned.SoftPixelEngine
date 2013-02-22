/*
 * Default texture layer header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_TEXTURE_LAYER_DEFAULT_H__
#define __SP_TEXTURE_LAYER_DEFAULT_H__


#include "Base/spStandard.hpp"
#include "Base/spMaterialConfigTypes.hpp"
#include "RenderSystem/spTextureLayer.hpp"


namespace sp
{
namespace video
{


/**
Default texture layer class. Particular used for fixed-function pipeline.
\since Version 3.2
\ingroup group_texture
*/
class SP_EXPORT TextureLayerDefault : public TextureLayer
{
    
    public:
        
        TextureLayerDefault();
        virtual ~TextureLayerDefault();
        
        /* === Functions === */
        
        virtual void bind() const;
        virtual void unbind() const;
        
        virtual void setupDefault() const;
        
        virtual bool compare(const TextureLayer* Other) const;
        
        /**
        Sets the mapping generation types. This describes how the texture coordinates are interpreted.
        If you don't use a shader, you can use a few pre-defined transformations for texture coordinates instead
        of using the per-vertex-stored texture coordinates.
        \param[in] Type Specifies the generation type. By default MAPGEN_DISABLE.
        \param[in] SetCoordsFlags Specifies whether the coordinate flags should also be set.
        Otherwise you have to do this by yourself using the "setMappingGenCoords" function.
        \see EMappingGenTypes
        \see setMappingGenCoords
        */
        void setMappingGen(const EMappingGenTypes Type, bool SetCoordsFlags = true);
        
        /* === Inline functions === */
        
        /**
        Sets the texture matrix. Use this to transform the texture coordinates.
        \param[in] Matrix Specifies the 4x4 matrix. If your texture coordiantes are only
        2-dimensional, the Z component of the matrix will be ignored.
        */
        inline void setMatrix(const dim::matrix4f &Matrix)
        {
            Matrix_ = Matrix;
        }
        //! Returns the texture matrix. By default the identity matrix.
        inline const dim::matrix4f& getMatrix() const
        {
            return Matrix_;
        }
        //! Returns a reference of the texture matrix.
        inline dim::matrix4f& getMatrix()
        {
            return Matrix_;
        }
        
        /**
        Sets the texture environment type. This describes how the texels are combined with the previous pixel colors.
        \param[in] Type Specifies the environment type. By default TEXENV_MODULATE.
        \see ETextureEnvTypes
        */
        inline void setTextureEnv(const ETextureEnvTypes Type)
        {
            EnvType_ = Type;
        }
        //! Returns the texture environment type. By default TEXENV_MODULATE.
        inline ETextureEnvTypes getTextureEnv() const
        {
            return EnvType_;
        }
        
        //! Returns the mapping generation types. By default MAPGEN_DISABLE.
        inline EMappingGenTypes getMappingGen() const
        {
            return MappingGen_;
        }
        
        /**
        Sets the texture mapping coordinate flags. This is used in combination with the
        mapping generation type.
        \param[in] CoordsFlags Specifies which texture coordinates are to be modified by the
        mapping generation procedure. This can be a combination of the flags listed in the
        "EMappingGenCoords" enumeration. By default MAPGEN_NONE.
        \see EMappingGenCoords
        \see setMappingGen
        */
        inline void setMappingGenCoords(s32 CoordsFlags)
        {
            MappingCoords_ = CoordsFlags;
        }
        //! Returns the texture mapping coordinate flags. By default MAPGEN_NONE.
        inline s32 getMappingGenCoords() const
        {
            return MappingCoords_;
        }
        
    protected:
        
        /* === Members === */
        
        dim::matrix4f Matrix_;
        ETextureEnvTypes EnvType_;
        EMappingGenTypes MappingGen_;
        s32 MappingCoords_;
        
};


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
