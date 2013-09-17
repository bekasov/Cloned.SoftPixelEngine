/*
 * Forward renderer header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_FORWARD_RENDERER_H__
#define __SP_FORWARD_RENDERER_H__


#include "Base/spStandard.hpp"

#if defined(SP_COMPILE_WITH_ADVANCEDRENDERER)


#include "RenderSystem/AdvancedRenderer/spAdvancedRenderer.hpp"


namespace sp
{
namespace video
{


/**
Integrated forward-renderer which supports normal- and parallax-occlision mapping.
\since Version 3.3
*/
class SP_EXPORT ForwardRenderer : public AdvancedRenderer
{
    
    public:
        
        ForwardRenderer();
        virtual ~ForwardRenderer();
        
        /* === Functions === */
        
        virtual io::stringc getDescription() const;
        
        virtual void renderScene(scene::SceneGraph* Graph, scene::Camera* ActiveCamera = 0);
        
        virtual void setGIReflectivity(f32 Reflectivity);
        virtual void setAmbientColor(const dim::vector3df &ColorVec);
        
    protected:
        
        /* === Functions === */
        
        virtual void updateLightSources(scene::SceneGraph* Graph, scene::Camera* ActiveCamera);
        //! \warning Does not check for null pointers!
        virtual void renderSceneIntoFrameBuffer(scene::SceneGraph* Graph, scene::Camera* ActiveCamera);
        
        virtual bool loadAllShaders();
        virtual void deleteAllShaders();
        
        virtual bool setupFinalResources();
        
        /* === Members === */
        
        ShaderClass* ForwardShader_; //!< Forward rendering shader class.
        
        s32 ShadowMapStartSlot_;
        
    private:
        
        /* === Functions === */
        
        bool loadForwardShader();
        
        void setupForwardCompilerOptions(std::list<io::stringc> &CompilerOp);
        void setupForwardSampler(Shader* ShaderObj);
        
};


} // /namespace video

} // /namespace sp


#endif

#endif



// ================================================================================
