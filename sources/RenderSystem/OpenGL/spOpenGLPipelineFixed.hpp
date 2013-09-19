/*
 * OpenGL fixed function renderer header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_OPENGL_FIXEDFUNCTIONPIPELINE_H__
#define __SP_OPENGL_FIXEDFUNCTIONPIPELINE_H__


#include "Base/spStandard.hpp"

#if defined(SP_COMPILE_WITH_OPENGL) || defined(SP_COMPILE_WITH_OPENGLES1)

#include "RenderSystem/OpenGL/spOpenGLPipelineBase.hpp"


namespace sp
{
namespace video
{


#if defined(SP_COMPILE_WITH_OPENGLES1)
#   define __DRAW2DARRAYS__
#endif


//! OpenGL fixed function render system. This is the parent class of all OpenGL and OpenGL|ES renderers.
class SP_EXPORT GLFixedFunctionPipeline : virtual public GLBasePipeline
{
    
    public:
        
        virtual ~GLFixedFunctionPipeline();
        
        /* === Render system information === */
        
        virtual s32 getMultitexCount() const;
        virtual s32 getMaxLightCount() const;
        
        /* === Configuration functions === */
        
        virtual void setShadeMode(const EShadeModeTypes ShadeMode);
        
        /* === Rendering functions === */
        
        virtual void updateLight(
            u32 LightID, const scene::ELightModels LightType, bool IsVolumetric,
            const dim::vector3df &Direction, const scene::SLightCone &SpotCone, const scene::SLightAttenuation &Attn
        );
        
        /* === Render states === */
        
        virtual void setupTextureLayer(
            u8 LayerIndex, const dim::matrix4f &TexMatrix, const ETextureEnvTypes EnvType,
            const EMappingGenTypes GenType, s32 MappingCoordsFlags
        );
        
        virtual void setRenderState(const video::ERenderStates Type, s32 State);
        virtual s32 getRenderState(const video::ERenderStates Type) const;
        
        /* === Lighting === */
        
        virtual void addDynamicLightSource(
            u32 LightID, scene::ELightModels Type,
            video::color &Diffuse, video::color &Ambient, video::color &Specular,
            f32 AttenuationConstant, f32 AttenuationLinear, f32 AttenuationQuadratic
        );
        
        virtual void setLightStatus(u32 LightID, bool Enable, bool UseAllRCs = false);
        
        virtual void setLightColor(
            u32 LightID,
            const video::color &Diffuse, const video::color &Ambient, const video::color &Specular,
            bool UseAllRCs = false
        );
        
        /* === Fog effect === */
        
        virtual void setFog(const EFogTypes Type);
        virtual void setFogColor(const video::color &Color);
        virtual void setFogRange(f32 Range, f32 NearPlane = 1.0f, f32 FarPlane = 1000.0f, const EFogModes Mode = FOG_PALE);
        
        /* === Clipping planes === */
        
        virtual void setClipPlane(u32 Index, const dim::plane3df &Plane, bool Enable);
        
        /* === Simple drawing functions === */
        
        virtual void beginDrawing2D();
        virtual void endDrawing2D();
        
        virtual void beginDrawing3D();
        
        virtual void setPointSize(s32 Size = 1);
        
        /* === Primitive drawing === */
        
        virtual color getPixelColor(const dim::point2di &Position) const;
        
        /* === Extra drawing functions === */
        
        virtual void draw2DPolygonImage(
            const ERenderPrimitives Type, Texture* Tex, const scene::SPrimitiveVertex2D* VerticesList, u32 Count
        );
        
        /* === Texture loading and creation === */
        
        virtual Texture* createScreenShot(const dim::point2di &Position = 0, dim::size2di Size = 0);
        virtual void createScreenShot(Texture* Tex, const dim::point2di &Position = 0);
        
        /* === Matrix controll === */
        
        virtual void updateModelviewMatrix();
        
        virtual void setProjectionMatrix(const dim::matrix4f &Matrix);
        virtual void setViewMatrix      (const dim::matrix4f &Matrix);
        virtual void setTextureMatrix   (const dim::matrix4f &Matrix, u8 TextureLayer = 0);
        
    protected:
        
        friend class OpenGLTexture;
        friend class OpenGLES1Texture;
        friend class OpenGLES2Texture;
        
        /* === Macros === */
        
        #define VBO_OFFSET_PTR(i)       ((char*)0 + (i))
        
        #define VBO_OFFSET_VERTEX       (0)
        #define VBO_OFFSET_NORMAL       (12)
        #define VBO_OFFSET_TEXCOORD(i)  (28 + 12*i)
        #define VBO_OFFSET_COLOR        (28 + 12*MAX_COUNT_OF_TEXTURES)
        #define VBO_OFFSET_FOG          (32 + 12*MAX_COUNT_OF_TEXTURES)
        
        #define ARY_OFFSET_VERTEX       (28)
        #define ARY_OFFSET_TEXCOORD     (20)
        #define ARY_OFFSET_COLOR        (44)
        
        /* === Structures === */
        
        struct SClipPlane
        {
            SClipPlane()
                : Index(0), Enable(false)
            {
            }
            SClipPlane(u32 PlaneIndex, const dim::plane3df ClipPlane, bool PlaneEnable)
                : Index(PlaneIndex), Enable(PlaneEnable)
            {
                PlaneEquation[0] = ClipPlane.Normal.X;
                PlaneEquation[1] = ClipPlane.Normal.Y;
                PlaneEquation[2] = ClipPlane.Normal.Z;
                PlaneEquation[3] = ClipPlane.Distance;
            }
            ~SClipPlane()
            {
            }
            
            /* Members */
            u32 Index;
            bool Enable;
            
            #if defined(SP_COMPILE_WITH_OPENGL)
            f64 PlaneEquation[4];
            #else
            f32 PlaneEquation[4];
            #endif
        };
        
        /* === Functions === */
        
        GLFixedFunctionPipeline();
        
        void drawTexturedFont(
            const Font* FontObj, const dim::point2di &Position, const io::stringc &Text, const color &Color
        );
        
        /* === Inline functions === */
        
        #if defined(SP_COMPILE_WITH_OPENGL)
        inline void setupTextureLayerCoordinate(GLenum RenderState, GLenum Coordinate, bool Enable, s32 GenType)
        {
            if (Enable)
            {
                glEnable(RenderState);
                glTexGeni(Coordinate, GL_TEXTURE_GEN_MODE, GenType);
            }
            else
                glDisable(RenderState);
        }
        #endif
        
        /* === Members === */
        
        std::vector<SClipPlane> ClippingPlanes_;
        scene::SPrimitiveVertex2D Vertices2D_[4];
        
        GLboolean isCullFace_;
        f32 TempColor_[4];
        
        static dim::matrix4f ExtTmpMat_;
        
};


} // /namespace video

} // /namespace sp


#endif

#endif



// ================================================================================
