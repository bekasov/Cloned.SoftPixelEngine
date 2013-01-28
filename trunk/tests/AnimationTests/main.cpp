//
// SoftPixel Engine - Animation Tests
//

#include <SoftPixelEngine.hpp>

using namespace sp;

#include "../common.hpp"

SP_TESTS_DECLARE

int main()
{
    SP_TESTS_INIT("Animation")
    
    // Create scene
    Cam->setPosition(dim::vector3df(0, 0, -3));
    
    scene::Mesh* Obj = spScene->loadMesh("dwarf2.b3d");
    Obj->setScale(0.05f);
    Obj->setRotation(dim::vector3df(0, -120, 0));
    Obj->setPosition(dim::vector3df(0, -1.25f, 0));
    
    scene::SkeletalAnimation* Anim = 0;
    scene::AnimationSkeleton* Skeleton = 0;
    
    if (Obj->getFirstAnimation() && Obj->getFirstAnimation()->getType() == scene::ANIMATION_SKELETAL)
    {
        Anim = static_cast<scene::SkeletalAnimation*>(Obj->getFirstAnimation());
        Skeleton = Anim->getActiveSkeleton();
    }
    
    scene::AnimationPlayback PlaybackA, PlaybackB;
    
    PlaybackA.play(scene::PLAYBACK_LOOP, 0, 12);//1, 12);
    PlaybackB.play(scene::PLAYBACK_LOOP, 15, 24);
    
    PlaybackA.stop();
    
    // Create font
    video::Font* Fnt = spRenderer->createFont("Arial", 20);
    
    f32 Blending = 0.0f;
    
    #define INTERP_TEST
    #ifdef INTERP_TEST
    
    math::ParabolicInterpolator Interp;
    Interp.setState(1.0f);
    
    #endif
    
    while (spDevice->updateEvents() && !spControl->keyDown(io::KEY_ESCAPE))
    {
        spRenderer->clearBuffers();
        
        // Update animation
        const f32 Speed = math::Lerp(0.15f, 0.25f, Blending);
        
        PlaybackA.update(Speed);
        PlaybackB.update(Speed);
        
        if (Anim && Skeleton)
        {
            Anim->interpolateBlended(PlaybackA, PlaybackB, Blending);
            Skeleton->transformVertices(Obj);
        }
        
        Blending += static_cast<f32>(spControl->getMouseWheel()) * 0.1f;
        math::Clamp(Blending, 0.0f, 1.0f);
        
        // Draw scene
        spScene->renderScene();
        
        spRenderer->beginDrawing2D();
        {
            #ifdef INTERP_TEST
            
            static bool Start;
            
            if (spControl->mouseHit(io::MOUSE_LEFT))
            {
                Start = !Start;
                Interp.play(0.05f);
            }
            
            #   if 0
            if (Interp.playing())
                io::Log::message(Interp.get());
            #   endif
            
            math::InterpolatorManager::update();
            
            if (Start)
                Blending = Interp.get();
            else
                Blending = 1.0f - Interp.get();
            
            #endif
            
            spRenderer->draw2DText(Fnt, 15, "Blending: " + io::stringc(Blending));
        }
        spRenderer->endDrawing2D();
        
        spContext->flipBuffers();
    }
    
    deleteDevice();
    
    return 0;
}
