/*
 * OpenGL|ES 1 render system header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_RENDERER_OPENGLES1_H__
#define __SP_RENDERER_OPENGLES1_H__


#include "Base/spStandard.hpp"

#if defined(SP_COMPILE_WITH_OPENGLES1)


#include "RenderSystem/OpenGL/spOpenGLPipelineFixed.hpp"
#include "RenderSystem/OpenGLES/spOpenGLES1Texture.hpp"


namespace sp
{
namespace video
{


//! OpenGL|ES 1 render system. This renderer supports OpenGL|ES 1.1.
class SP_EXPORT OpenGLES1RenderSystem : public GLFixedFunctionPipeline
{
    
    public:
        
        OpenGLES1RenderSystem();
        ~OpenGLES1RenderSystem();
        
        /* === Render system information === */
        
        io::stringc getVersion() const;
        
        bool queryVideoSupport(const EVideoFeatureQueries Query) const;
        
        /* === Context functions === */
        
        void setupConfiguration();
        
        /* === Rendering functions === */
        
        void setupMaterialStates(const MaterialStates* Material);
        
        void drawPrimitiveList(
            const ERenderPrimitives Type,
            const scene::SMeshVertex3D* Vertices, u32 VertexCount, const void* Indices, u32 IndexCount,
            const TextureLayerListType* TextureLayers
        );
        
        /* === Hardware mesh buffers === */
        
        void drawMeshBuffer(const MeshBuffer* MeshBuffer);
        
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
        void draw3DTriangle(
            Texture* hTexture, const dim::triangle3df &Triangle, const color &Color = 255
        );
        
};


} // /namespace video

} // /namespace sp


#endif

#endif



// ================================================================================
