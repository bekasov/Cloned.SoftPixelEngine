/*
 * Matrix texture header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_UTILITY_MATRIX_TEXTURE_H__
#define __SP_UTILITY_MATRIX_TEXTURE_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_MATRIXTEXTURE


#include "Base/spDimensionMatrix4.hpp"
#include "RenderSystem/spTextureBase.hpp"


namespace sp
{
namespace tool
{


/**
Base class for vector- and matrix textures.
\see MatrixTexture
\see VectorTexture
\since Version 3.2
*/
class SP_EXPORT GeneralPurposeTexture
{
    
    public:
        
        virtual ~GeneralPurposeTexture();
        
        /* === Functions === */
        
        //! Returns the texture coordinate for the given index.
        virtual dim::point2di getTexCoord(u32 Index) const = 0;
        
        //! Resizes the texture for the new count of elements.
        virtual void resize(u32 Count) = 0;
        
        /**
        Updates the texture's image buffer. This is equivalent to the following code:
        \code
        MyUtilTex->getTexture()->updateImageBuffer();
        \endcode
        */
        virtual void update();
        
        /* === Static functions === */
        
        /**
        Computes the size for the texture by the given count.
        \param[in] Count Specifies the count of matrices. Must be grater or equal to 1.
        \return The size which is required for a texture to store the given count of matrices.
        If the specified count of 0, the return value is dim::size2di(0, 0).
        */
        static dim::size2di getSizeByCount(u32 Count);
        
        /* === Inline functions === */
        
        //! Returns the count of maximal matrics or vectors which can be stored inside the texture. By default 1.
        inline u32 getCount() const
        {
            return Count_;
        }
        
        //! Returns a constant pointer to the Texture object. This is never a null pointer.
        inline video::Texture* getTexture() const
        {
            return Tex_;
        }
        
    protected:
        
        GeneralPurposeTexture(const io::stringc &TypeName, const dim::size2di &TexSize, u32 Count);
        
        /* === Functions === */
        
        void generate(const dim::size2di &TexSize, u32 Count);
        
        /* === Members === */
        
        video::Texture* Tex_;
        u32 Count_;
        
};


/**
A matrix texture can be used to store 4x4 matrices inside a texture object.
This can be useful for texture-lookups on a vertex shader, e.g. for hardware-instancing.
\see video::Shader
\see video::Texture
\see dim::matrix4
\see VectorTexture
\since Version 3.2
*/
class SP_EXPORT MatrixTexture : public GeneralPurposeTexture
{
    
    public:
        
        //! \throws io::RenderSystemException When this object will be created before the global render-system has been created.
        MatrixTexture(u32 Count);
        ~MatrixTexture();
        
        /* === Functions === */
        
        /**
        Writes the specified matrix into the texture.
        \param[in] Index Specifies the index at which point the matrix will be inserted.
        Must be smaller than the maximal count of matrices.
        \param[in] Matrix Specifies the matrix which is to be stored inside the texture.
        \param[in] ImmediateUpdate Specifies whether the texture is to be updated immediately.
        Otherwise you have to update the image buffer by yourself using the "Texture::updateImageBuffer" function.
        */
        void setMatrix(u32 Index, const dim::matrix4f &Matrix, bool ImmediateUpdate = false);
        //! Returns a matrix which is stored inside the texture at the specified index.
        dim::matrix4f getMatrix(u32 Index) const;
        
        //! Returns the texture coordinate for the given index.
        dim::point2di getTexCoord(u32 Index) const;
        
        //! Resizes the texture for the new count of 4x4 matrices.
        void resize(u32 Count);
        
};


/**
A vector texture can be used to store 4 comoponent vectors inside a texture object.
This can be useful for texture-lookups on a vertex shader, e.g. for hardware-instancing.
\see video::Shader
\see video::Texture
\see dim::matrix4
\see MatrixTexture
\since Version 3.2
*/
class SP_EXPORT VectorTexture : public GeneralPurposeTexture
{
    
    public:
        
        //! \throws io::RenderSystemException When this object will be created before the global render-system has been created.
        VectorTexture(u32 Count);
        ~VectorTexture();
        
        /* === Functions === */
        
        /**
        Writes the specified vector into the texture.
        \param[in] Index Specifies the index at which point the vector will be inserted.
        Must be smaller than the maximal count of vectors.
        \param[in] Vector Specifies the vector which is to be stored inside the texture.
        \param[in] ImmediateUpdate Specifies whether the texture is to be updated immediately.
        Otherwise you have to update the image buffer by yourself using the "Texture::updateImageBuffer" function.
        */
        void setVector(u32 Index, const dim::vector4df &Vector, bool ImmediateUpdate = false);
        //! Returns a vector which is stored inside the texture at the specified index.
        dim::vector4df getVector(u32 Index) const;
        
        //! Returns the texture coordinate for the given index.
        dim::point2di getTexCoord(u32 Index) const;
        
        //! Resizes the texture for the new count of vectors.
        void resize(u32 Count);
        
};


} // /namespace tool

} // /namespace sp


#endif

#endif



// ================================================================================
