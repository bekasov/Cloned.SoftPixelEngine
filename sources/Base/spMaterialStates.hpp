/*
 * Material states header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_MATERIAL_STATES_H__
#define __SP_MATERIAL_STATES_H__


#include "Base/spStandard.hpp"
#include "Base/spInputOutputString.hpp"
#include "Base/spMaterialConfigTypes.hpp"
#include "Base/spMaterialColor.hpp"

#include <limits.h>
#include <boost/shared_ptr.hpp>


namespace sp
{
namespace video
{


//! MaterialStates class used to store and handle material attributes.
class SP_EXPORT MaterialStates
{
    
    public:
        
        MaterialStates();
        MaterialStates(const MaterialStates* Other);
        virtual ~MaterialStates();
        
        /* === Functions === */
        
        //! Copies all attributes.
        virtual void copy(const MaterialStates* Other);
        
        //! Returns true if the settings are equal with the other one. Otherwise false.
        virtual bool compare(const MaterialStates* Other) const;
        
        /**
        Updates the material states. Call this after changing material states manual.
        This is normally only used for the Direct3D11 video driver but as a good programming style always
        call it after changing a rasterizer-, depth-stencil- or blend state. This shall not include:
        colors (diffuse, ambient etc.), shininess, shading, color material and lighting.
        */
        virtual void update();
        
        /* === Inline functions === */
        
        //! Sets the wireframe only for this MaterialNode.
        inline void setWireframe(const EWireframeTypes Type)
        {
            RenderModeFront_ = RenderModeBack_ = Type;
            update();
        }
        
        /**
        Sets the wireframe only for this mesh for the front- and the back side.
        By default only the front side is visible. You can set front, back or both by using the
        "setRenderFace" function.
        */
        inline void setWireframe(const EWireframeTypes TypeFront, const EWireframeTypes TypeBack)
        {
            RenderModeFront_    = TypeFront;
            RenderModeBack_     = TypeBack;
            update();
        }
        
        //! Returns the wireframe mode for the front side of polygons.
        inline EWireframeTypes getWireframeFront() const
        {
            return RenderModeFront_;
        }
        //! Returns the wireframe mode for the back side of polygons.
        inline EWireframeTypes getWireframeBack() const
        {
            return RenderModeBack_;
        }
        
        /**
        Sets the render face type.
        \param Face: Face type. Three types are supported: Front, Back, Both.
        This type specifies which side of each triangle is visible. This feature is also called "cullmode".
        */
        inline void setRenderFace(const EFaceTypes Face)
        {
            RenderFace_ = Face;
            update();
        }
        inline EFaceTypes getRenderFace() const
        {
            return RenderFace_;
        }
        
        /**
        Sets the shininess.
        \param Intensity: Shininess factor (must be in a range of [0.0, 1.0]).
        */
        inline void setShininess(f32 Intensity)
        {
            Shininess_ = 128.0f - math::MinMax(Intensity, 0.0f, 1.0f)*128;
        }
        //! Returns shininess factor.
        inline f32 getShininess() const
        {
            return 1.0f - Shininess_/128;
        }
        
        //! Sets the real renderer shininess factor in the range [0.0, 128.0].
        inline void setShininessFactor(f32 Factor)
        {
            Shininess_ = math::MinMax(Factor, 0.0f, 128.0f);
        }
        //! Returns the real renderer shininess factor in the range [0.0, 128.0].
        inline f32 getShininessFactor() const
        {
            return Shininess_;
        }
        
        /**
        Enables or disables the color material. By default it is enabled.
        When disabling the color material the vertices' color will not be used but the "ColorDiffuse"
        front the material. This can be used when only one color for the whole Mesh/ Terrain shall be used and
        has to be changed very fast for large 3D models.
        */
        inline void setColorMaterial(bool isColorMaterial)
        {
            isColorMaterial_ = isColorMaterial;
        }
        //! Returns status of color material.
        inline bool getColorMaterial() const
        {
            return isColorMaterial_;
        }
        
        //! Enables or disables lighting computations.
        inline void setLighting(bool isLighting)
        {
            isLighting_ = isLighting;
        }
        //! Returns status of lighting computations.
        inline bool getLighting() const
        {
            return isLighting_;
        }
        
        //! Enabled or disables depth buffering.
        inline void setDepthBuffer(bool isDepthBuffer)
        {
            isZBuffer_ = isDepthBuffer;
            update();
        }
        //! Returns status of depth buffering.
        inline bool getDepthBuffer() const
        {
            return isZBuffer_;
        }
        
        //! Enabled or disables alpha blending.
        inline void setBlending(bool isBlending)
        {
            isBlending_ = isBlending;
            update();
        }
        //! Returns status of alpha blending.
        inline bool getBlending() const
        {
            return isBlending_;
        }
        
        //! Enables or disables fog effect.
        inline void setFog(bool isFog)
        {
            isFog_ = isFog;
        }
        //! Returns status of fog effect.
        inline bool getFog() const
        {
            return isFog_;
        }
        
        //! Enables or disables polygon offset without changing factor or units.
        inline void setPolygonOffset(bool isPolygonOffset)
        {
            isPolygonOffset_ = isPolygonOffset;
            update();
        }
        //! Returns status of polygon offset.
        inline bool getPolygonOffset() const
        {
            return isPolygonOffset_;
        }
        
        /**
        Sets all colors.
        \param Diffuse: Diffuse (or rather basic) color.
        \param Ambient: Ambient color (where no light is).
        \param Specular: Specular color (Color for shininess).
        \param Emission: Emission color (Self lighting color).
        */
        inline void setColors(
            const color &Diffuse, const color &Ambient, const color &Specular, const color &Emission)
        {
            ColorDiffuse_   = Diffuse;
            ColorAmbient_   = Ambient;
            ColorSpecular_  = Specular;
            ColorEmission_  = Emission;
        }
        
        //! Sets the diffuse color. This is the main color for coloring objects.
        inline void setDiffuseColor(const color &Color)
        {
            ColorDiffuse_ = Color;
        }
        //! Returns diffuse color.
        inline color getDiffuseColor() const
        {
            return ColorDiffuse_;
        }
        //! Returns diffuse color reference.
        inline color& getDiffuseColor()
        {
            return ColorDiffuse_;
        }
        
        //! Sets the ambient color. This is the color for the areas where no light is.
        inline void setAmbientColor(const color &Color)
        {
            ColorAmbient_ = Color;
        }
        //! Returns ambient color.
        inline color getAmbientColor() const
        {
            return ColorAmbient_;
        }
        //! Returns ambient color reference.
        inline color& getAmbientColor()
        {
            return ColorAmbient_;
        }
        
        //! Sets the specular color. Use "setShininess" to set the specular's intensity.
        inline void setSpecularColor(const color &Color)
        {
            ColorSpecular_ = Color;
        }
        //! Returns specular color.
        inline color getSpecularColor() const
        {
            return ColorSpecular_;
        }
        //! Returns specular color reference.
        inline color& getSpecularColor()
        {
            return ColorSpecular_;
        }
        
        /**
        Sets the emission color. This color is normally used only for shaders.
        It specifies the reflection color (or rather reflection intensity).
        */
        inline void setEmissionColor(const color &Color)
        {
            ColorEmission_ = Color;
        }
        //! Returns emission color.
        inline color getEmissionColor() const
        {
            return ColorEmission_;
        }
        //! Returns emission color reference.
        inline color& getEmissionColor()
        {
            return ColorEmission_;
        }
        
        //! Only sets the diffuse's alpha channel with a floating-point.
        inline void setDiffuseAlpha(f32 Alpha)
        {
            ColorDiffuse_.Alpha = static_cast<u8>(math::MinMax(Alpha * 255.0f, 0.0f, 255.0f));
        }
        
        /**
        Sets the shading type. By default "SHADING_GOURAUD" is used for a smooth lighting.
        When changing the shading type you have to call "updateNormals". Two methods are supported yet:
        Flat- and grouaud shading. There is also Phong shading but not supported yet, it will switch to Gouraud shading.
        \param Type: Type of shading (flat, gouraud, phong, per-pixel).
        */
        inline void setShading(const EShadingTypes Type)
        {
            Shading_ = Type;
        }
        //! Returns the shading type.
        inline EShadingTypes getShading() const
        {
            return Shading_;
        }
        
        /**
        Sets polygon offset. With polygon offset you can manipulate the pixel depht values.
        Use this when you have polygons which lie direct onto other polygons to avoid "Z-fighting".
        \param isPolygonOffset: Enables or disables polygon offset.
        \param OffsetFactor: Factor whereby the pixel depth is multiplied.
        \param OffsetUnits: Units which are added to the pixel depth.
        */
        inline void setPolygonOffset(bool isPolygonOffset, f32 OffsetFactor, f32 OffsetUnits = 0.0f)
        {
            isPolygonOffset_    = isPolygonOffset;
            OffsetFactor_       = OffsetFactor;
            OffsetUnits_        = OffsetUnits;
            update();
        }
        
        //! Sets the polygon offset factor. By defautl 0.0.
        inline void setPolygonOffsetFactor(f32 Factor)
        {
            OffsetFactor_ = Factor;
            update();
        }
        //! Returns the polygon offset factor. By default 0.0.
        inline f32 getPolygonOffsetFactor() const
        {
            return OffsetFactor_;
        }
        
        //! Sets the polygon offset units. By default 0.0.
        inline void setPolygonOffsetUnits(f32 Units)
        {
            OffsetUnits_ = Units;
            update();
        }
        //! Returns the polygon offset units. By default 0.0.
        inline f32 getPolygonOffsetUnits() const
        {
            return OffsetUnits_;
        }
        
        /**
        Sets the blending mode.
        \param Mode: Sets the blending bright, dark or normal.
        */
        inline void setBlendingMode(const EDefaultBlendingTypes Mode)
        {
            setDefaultBlending(Mode, BlendSource_, BlendTarget_);
        }
        
        /**
        Sets the blending source and target. Blending is particularly used for Billboards which represents a lense flare
        or kind of these things but can also be used for Mesh or Terrain objects.
        \param BlendSource: Source blending method.
        \param BlendTarget: Target/ destination blending method.
        */
        inline void setBlendingMode(const EBlendingTypes BlendSource, const EBlendingTypes BlendTarget)
        {
            BlendSource_ = BlendSource;
            BlendTarget_ = BlendTarget;
            update();
        }
        
        //! Sets the blending source.
        inline void setBlendSource(const EBlendingTypes BlendSource)
        {
            BlendSource_ = BlendSource;
            update();
        }
        //! Returns the blending source.
        inline EBlendingTypes getBlendSource() const
        {
            return BlendSource_;
        }
        
        //! Sets the blending target.
        inline void setBlendTarget(const EBlendingTypes BlendTarget)
        {
            BlendTarget_ = BlendTarget;
            update();
        }
        //! Returns the blending target.
        inline EBlendingTypes getBlendTarget() const
        {
            return BlendTarget_;
        }
        
        /**
        Sets the depth-test function. By default "CMPSIZE_LESSEQUAL".
        \param Method: Detph-test function which is proceeded before a pixel is rendered.
        */
        inline void setDepthMethod(const ESizeComparisionTypes Method)
        {
            DepthMethod_ = Method;
            update();
        }
        //! Returns the depth-test function.
        inline ESizeComparisionTypes getDepthMethod() const
        {
            return DepthMethod_;
        }
        
        /**
        Sets the alpha-test function. By default "CMPSIZE_ALWAYS".
        \param Method: Alpha-test function which is proceeded before a pixel is rendered.
        In many games this test-function is used to avoid that pixels are rendered which are
        anyway not visible because of their alpha-channel. But the depth is normally also rendered into
        the depth-buffer which affects that sometimes problems can occur when using transparency 3D models.
        e.g. a mesh wire fence (good example are some maps from Counter-Strike 1).
        In this case you can set the alpha-test function to "CMPSIZE_GREATER" and "AlphaReference" to 0.5
        and disable blending (using "isBlending = false" which is a member of MaterialStates).
        Then pixels with an alpha-channel lower than 0.5 are not rendered.
        */
        virtual void setAlphaMethod(const ESizeComparisionTypes Method, f32 AlphaReference);
        
        //! Sets the alpha-test method without changing the alpha reference value.
        inline void setAlphaMethod(const ESizeComparisionTypes Method)
        {
            setAlphaMethod(Method, AlphaReference_);
        }
        //! Returns the alpha-test method.
        inline ESizeComparisionTypes getAlphaMethod() const
        {
            return AlphaMethod_;
        }
        //! Sets the alpha-test reference value.
        inline void setAlphaReference(f32 AlphaReference)
        {
            setAlphaMethod(AlphaMethod_, AlphaReference);
        }
        //! Returns the alpha-test reference value.
        inline f32 getAlphaReference() const
        {
            return AlphaReference_;
        }
        
        /**
        Sets the material callback. Use this to make extended configurations on a geometry object.
        If set, this function will be called twice. First times you can enable your extended options
        and second times you can disable it.
        */
        inline void setMaterialCallback(const UserMaterialCallback &MaterialCallback)
        {
            UserMaterialProc_ = MaterialCallback;
        }
        //! Returns the material callback.
        inline UserMaterialCallback getMaterialCallback() const
        {
            return UserMaterialProc_;
        }
        
    protected:
        
        /* === Functions == */
        
        virtual void setDefaultBlending(
            const video::EDefaultBlendingTypes Mode, video::EBlendingTypes &SrcBlend, video::EBlendingTypes &DestBlend
        );
        
        /* === Members === */
        
        color                   ColorDiffuse_;      //!< Diffuse material color which represents the main object's color (by default 255, 255, 255, 255).
        color                   ColorAmbient_;      //!< Ambient material color (by default 50, 50, 50, 255).
        color                   ColorSpecular_;     //!< Specular material color (by default 255, 255, 255, 255).
        color                   ColorEmission_;     //!< Emission material color (by default 0, 0, 0, 255).
        
        f32                     Shininess_;         //!< Shininess factor (must be in a range of 0.0 to 1.0).
        f32                     AlphaReference_;    //!< Reference value for the alpha test method (must be in a range of 0.0 to 1.0).
        f32                     OffsetFactor_;      //!< Polygon offset factor. If polygon offset is enabled each pixel's depth value will be modified by this factor.
        f32                     OffsetUnits_;       //!< Polygon offset units. If polygon offset is enabled each pixel's depth value will be moved by this value.
        
        EShadingTypes           Shading_;           //!< Shading mode (flat, gouraud, phong or per-pixel).
        ESizeComparisionTypes   DepthMethod_;       //!< Depth test method.
        ESizeComparisionTypes   AlphaMethod_;       //!< Alpha test method.
        EBlendingTypes          BlendSource_;       //!< Source blend type for alpha-blending
        EBlendingTypes          BlendTarget_;       //!< Target blend type for alpha-blending
        EWireframeTypes         RenderModeFront_;   //!< Specifies the wireframe mode for the front side of each triangle.
        EWireframeTypes         RenderModeBack_;    //!< Specifies the wireframe mode for the back side of each triangle.
        EFaceTypes              RenderFace_;        //!< Specifies which face's sides are used (front, back or both).
        
        bool                    isColorMaterial_;   //!< Option for per-vertex coloring if enabled. By default enabled.
        bool                    isLighting_;        //!< Option for lighting. By default enabled. Global lighting must be enabled as well.
        bool                    isBlending_;        //!< Option for alpha-blending. By default enabled.
        bool                    isZBuffer_;         //!< Option for depth-buffering. By default enabled.
        //bool                    IsDepthTest_;       //!< Option for depth test. By default enabled.
        //bool                    IsAlphaTest_;       //!< Option for alpha test. By default enabled.
        bool                    isFog_;             //!< Option for fog effect. By default enabled. Global fog effect must be enabled as well.
        bool                    isPolygonOffset_;   //!< Option for polygon offset which manipulates each pixel depth value which is to be written. By default disabled.
        
        /**
        User material callback function. If this function is set it will be called
        in the "render" function of the Mesh class before the material is set and the object
        is rendered where the given parameter "isBegin" is true.
        A second time it will be called after the object is rendered where the given parameter is 'false'.
        This function can be used to extend the functionallity of material settings. e.g. the material does not
        have a parameter to change the fog type for each object individual. In your own callback function you can
        change it when "isBegin" is true and reset it when it is false. In this way your options are immense.
        */
        UserMaterialCallback    UserMaterialProc_;
        
    private:
        
        friend class Direct3D9RenderSystem;
        friend class Direct3D11RenderSystem;
        
        /* === Members === */
        
        // Independent video driver references
        void* RefRasterizerState_;
        void* RefDepthStencilState_;
        void* RefBlendState_;
        
};


//! MaterialStates smart pointer.
typedef boost::shared_ptr<MaterialStates> MaterialStatesPtr;


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
