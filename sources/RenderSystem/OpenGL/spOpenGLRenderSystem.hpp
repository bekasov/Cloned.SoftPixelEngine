/*
 * OpenGL render system header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_RENDERSYSTEM_OPENGL_H__
#define __SP_RENDERSYSTEM_OPENGL_H__


#include "Base/spStandard.hpp"

#if defined(SP_COMPILE_WITH_OPENGL)


#include "RenderSystem/OpenGL/spOpenGLPipelineFixed.hpp"
#include "RenderSystem/OpenGL/spOpenGLPipelineProgrammable.hpp"
#include "RenderSystem/OpenGL/spOpenGLTexture.hpp"


namespace sp
{
namespace video
{


//! OpenGL render system. This renderer supports OpenGL 1.1 - 4.1.
class SP_EXPORT OpenGLRenderSystem : public GLFixedFunctionPipeline, public GLProgrammableFunctionPipeline
{
    
    public:
        
        OpenGLRenderSystem();
        ~OpenGLRenderSystem();
        
        /* === Initialization functions === */
        
        void setupConfiguration();
        
        /* === Render system information === */
        
        bool queryVideoSupport(const EVideoFeatureQueries Query) const;
        
        /* === Configuration functions === */
        
        void setAntiAlias(bool isAntiAlias);
        
        void setDepthClip(bool Enable);
        bool getDepthClip() const;
        
        /* === Rendering functions === */
        
        void setupMaterialStates(const MaterialStates* Material);
        
        void drawPrimitiveList(
            const ERenderPrimitives Type,
            const scene::SMeshVertex3D* Vertices, u32 VertexCount, const void* Indices, u32 IndexCount,
            std::vector<SMeshSurfaceTexture>* TextureList
        );
        
        /* === Hardware mesh buffers === */
        
        void drawMeshBuffer(const MeshBuffer* MeshBuffer);
        void drawMeshBufferPlain(const MeshBuffer* MeshBuffer, bool useFirstTextureLayer = false);
        
        /* === Stencil buffer === */
        
        void clearStencilBuffer();
        
        void drawStencilShadowVolume(
            const dim::vector3df* pTriangleList, s32 Count, bool ZFailMethod, bool VolumetricShadow
        );
        void drawStencilShadow(const video::color &Color);
        
        /* === Shader programs === */
        
        Shader* createCgShader(
            ShaderClass* ShaderClassObj, const EShaderTypes Type, const EShaderVersions Version,
            const std::list<io::stringc> &ShaderBuffer, const io::stringc &EntryPoint = "",
            const c8** CompilerOptions = 0
        );
        
        /* === Image drawing === */
        
        void draw2DImage(
            Texture* Tex, const dim::point2di &Position, const color &Color = color(255)
        );
        
        void draw2DImage(
            Texture* Tex, const dim::rect2di &Position,
            const dim::rect2df &Clipping = dim::rect2df(0.0f, 0.0f, 1.0f, 1.0f),
            const color &Color = color(255)
        );
        
        void draw2DImage(
            Texture* Tex, const dim::point2di &Position, f32 Rotation, f32 Radius, const color &Color = color(255)
        );
        
        void draw2DImage(
            Texture* Tex,
            const dim::point2di &lefttopPosition,
            const dim::point2di &righttopPosition,
            const dim::point2di &rightbottomPosition,
            const dim::point2di &leftbottomPosition,
            const dim::point2df &lefttopClipping = dim::point2df(0.0f, 0.0f),
            const dim::point2df &righttopClipping = dim::point2df(1.0f, 0.0f),
            const dim::point2df &rightbottomClipping = dim::point2df(1.0f, 1.0f),
            const dim::point2df &leftbottomClipping = dim::point2df(0.0f, 1.0f),
            const color &lefttopColor = color(255),
            const color &righttopColor = color(255),
            const color &rightbottomColor = color(255),
            const color &leftbottomColor = color(255)
        );
        
        /* === Primitive drawing === */
        
        f32 getPixelDepth(const dim::point2di &Position) const;
        
        void draw2DPoint(const dim::point2di &Position, const color &Color = 255);
        
        void draw2DLine(
            const dim::point2di &PositionA, const dim::point2di &PositionB, const color &Color = 255
        );
        void draw2DLine(
            const dim::point2di &PositionA, const dim::point2di &PositionB, const color &ColorA, const color &ColorB
        );
        
        void draw2DRectangle(
            const dim::rect2di &Rect, const color &Color = 255, bool isSolid = true
        );
        void draw2DRectangle(
            const dim::rect2di &Rect, const color &lefttopColor, const color &righttopColor,
            const color &rightbottomColor, const color &leftbottomColor, bool isSolid = true
        );
        
        /* === Extra drawing functions === */
        
        void draw2DPolygon(
            const ERenderPrimitives Type, const scene::SPrimitiveVertex2D* VerticesList, u32 Count
        );
        
        /* === 3D drawing functions === */
        
        void draw3DPoint(
            const dim::vector3df &Position, const color &Color = 255
        );
        void draw3DLine(
            const dim::vector3df &PositionA, const dim::vector3df &PositionB, const color &Color = 255
        );
        void draw3DLine(
            const dim::vector3df &PositionA, const dim::vector3df &PositionB, const color &ColorA, const color &ColorB
        );
        void draw3DEllipse(
            const dim::vector3df &Position, const dim::vector3df &Rotation = 0.0f,
            const dim::size2df &Radius = 0.5f, const color &Color = 255
        );
        void draw3DTriangle(
            Texture* hTexture, const dim::triangle3df &Triangle, const color &Color = 255
        );
        
        /* === Font loading and text drawing === */
        
        Font* createBitmapFont(const io::stringc &FontName = "", s32 FontSize = 0, s32 Flags = 0);
        
        /* Drawing text */
        void draw3DText(
            Font* FontObj, const dim::matrix4f &Transformation, const io::stringc &Text, const color &Color = color(255)
        );
        
        /* === Matrix controll === */
        
        void setColorMatrix(const dim::matrix4f &Matrix);
        
    private:
        
        friend class OpenGLTexture;
        
        /* === Structures === */
        
        #if defined(SP_PLATFORM_LINUX)
        struct SX11FontPackage
        {
            GLuint DisplayListsID;
            XID FontID;
        };
        #endif
        
        /* === Functions === */
        
        void deleteFontObjects();
        void releaseFontObject(Font* FontObj);
        
        void loadExtensions();
        
        void defaultTextureGenMode();
        
        void drawBitmapFont(Font* FontObj, const dim::point2di &Position, const io::stringc &Text, const color &Color);
        
        /* === Members === */
        
        #if defined(SP_PLATFORM_WINDOWS)
        HGLRC hRC_;
        #endif
        
};


} // /namespace video

} // /namespace sp


#endif

#endif



// ================================================================================
