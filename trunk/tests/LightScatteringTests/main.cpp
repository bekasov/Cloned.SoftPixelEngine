//
// SoftPixel Engine - Light Scattering Tests
//

#include <SoftPixelEngine.hpp>

using namespace sp;

#include "../common.hpp"

SP_TESTS_DECLARE

/* === Global members === */

video::Texture* LightTex = 0;

video::ShaderClass* ShdClassFx = 0;
video::Shader* ShdVertFx = 0;
video::Shader* ShdFragFx = 0;

video::ShaderClass* ShdClassLit = 0;

scene::Mesh* LightMdl = 0;

dim::rect2di ScreenRect;


/* === Functions === */

void RenderLightTexture()
{
    // Update sceen light position
    if (ShdFragFx)
    {
        dim::vector4df ScreenLightPos(LightMdl->getPosition());
        Cam->projectPoint(ScreenLightPos);
        
        ScreenLightPos.X /= ScreenRect.Right;
        ScreenLightPos.Y /= ScreenRect.Bottom;
        
        ShdFragFx->setConstant("ScreenLightPos", ScreenLightPos);
    }
    
    // Render scene into light texture
    spRenderer->setRenderTarget(LightTex);
    {
        spRenderer->clearBuffers();
        
        LightMdl->setVisible(true);
        Cam->setViewport(dim::rect2di(0, 0, LightTex->getSize().Width, LightTex->getSize().Height));
        
        spRenderer->setGlobalShaderClass(ShdClassLit);
        {
            spScene->renderScene();
        }
        spRenderer->setGlobalShaderClass(0);
    }
    spRenderer->setRenderTarget(0);
}

void RenderScene()
{
    spRenderer->clearBuffers();
    
    LightMdl->setVisible(false);
    Cam->setViewport(ScreenRect);
    
    spScene->renderScene();
}

void RenderPostProcessEffect()
{
    spRenderer->setRenderMode(video::RENDERMODE_DRAWING_2D);
    
    spRenderer->setRenderState(video::RENDER_BLEND, true);
    spRenderer->setBlending(video::BLEND_SRCALPHA, video::BLEND_ONE);
    
    ShdClassFx->bind();
    {
        spRenderer->draw2DImage(LightTex, ScreenRect);
    }
    ShdClassFx->unbind();
    
    spRenderer->setBlending(video::BLEND_SRCALPHA, video::BLEND_INVSRCALPHA);
    spRenderer->setRenderState(video::RENDER_BLEND, false);
}

void ShaderLitCallback(video::ShaderClass* ShdClass, const scene::MaterialNode* Object)
{
    ShdClass->getPixelShader()->setConstant(
        "Color", dim::vector4df(Object == LightMdl ? 1.0f : 0.0f, 1.0f)
    );
}

int main()
{
    SP_TESTS_INIT("Light Scattering")
    
    ScreenRect.Right    = spContext->getResolution().Width;
    ScreenRect.Bottom   = spContext->getResolution().Height;
    
    // Create resources
    LightTex = spRenderer->createTexture(spContext->getResolution());// / 2);
    LightTex->setRenderTarget(true);
    
    // Load post process shader
    ShdClassFx = spRenderer->createShaderClass();
    {
        ShdVertFx = spRenderer->loadShader(
            ShdClassFx, video::SHADER_VERTEX, video::GLSL_VERSION_1_20,
            "LightScatteringFx.glvert", "", video::SHADERFLAG_ALLOW_INCLUDES
        );
        ShdFragFx = spRenderer->loadShader(
            ShdClassFx, video::SHADER_PIXEL, video::GLSL_VERSION_1_20,
            "LightScatteringFx.glfrag", "", video::SHADERFLAG_ALLOW_INCLUDES
        );
    }
    if (ShdClassFx->link())
    {
        ShdFragFx->setConstant("LightTex",  0);
        ShdFragFx->setConstant("Exposure",  0.5f);
        ShdFragFx->setConstant("Weight",    0.5f);
        ShdFragFx->setConstant("Decay",     0.97f);
        ShdFragFx->setConstant("Density",   0.97f);
    }
    
    //! Load light shader
    ShdClassLit = spRenderer->createShaderClass();
    {
        spRenderer->loadShader(
            ShdClassLit, video::SHADER_VERTEX, video::GLSL_VERSION_1_20,
            "LightScatteringLit.glvert", "", video::SHADERFLAG_ALLOW_INCLUDES
        );
        spRenderer->loadShader(
            ShdClassLit, video::SHADER_PIXEL, video::GLSL_VERSION_1_20,
            "LightScatteringLit.glfrag", "", video::SHADERFLAG_ALLOW_INCLUDES
        );
    }
    if (ShdClassLit->link())
        ShdClassLit->setObjectCallback(ShaderLitCallback);
    
    // Load scene
    spSceneMngr->loadScene("TestScene.spsb");
    
    // Create light model
    LightMdl = spScene->createMesh(scene::MESH_ICOSPHERE);
    LightMdl->setScale(3);
    
    scene::SceneNode* LightObj = spScene->findNode("Light");
    if (LightObj)
        LightMdl->setPosition(LightObj->getPosition());
    
    while (spDevice->updateEvents() && !spControl->keyDown(io::KEY_ESCAPE))
    {
        if (spContext->isWindowActive())
            tool::Toolset::moveCameraFree();
        
        static f32 fov = 74.0f;
        fov += static_cast<f32>(spControl->getMouseWheel());
        Cam->setFOV(fov);
        
        RenderLightTexture();
        RenderScene();
        RenderPostProcessEffect();
        
        spContext->flipBuffers();
    }
    
    deleteDevice();
    
    return 0;
}
