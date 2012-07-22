/*
 * Particle animator header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_TOOL_PARTICLEANIMATOR_H__
#define __SP_TOOL_PARTICLEANIMATOR_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_PARTICLEANIMATOR


#include "SceneGraph/spSceneGraph.hpp"


namespace sp
{
namespace tool
{


/*
 * Enumerations
 */

enum EParticleAttribute
{
    PARTICLE_LOOP,
    PARTICLE_ONESHOT,
};

enum EParticleBlendMode
{
    PARTICLE_NOBLENDING,
    PARTICLE_BLENDIN,
    PARTICLE_BLENDOUT,
};


/*
 * Structures
 */

struct SParticle
{
    scene::Billboard* Object;
    
    dim::vector3df Translation;
    dim::vector3df Gravity;
    dim::vector3df Rotation;
    dim::vector3df Transformation;
    
    u64 Time;
    u64 Endurance;
    f32 Alpha;
    f32 BlendSpeed;
    EParticleBlendMode BlendMode;
    
    EParticleAttribute Attribute;
};

typedef bool (*PFNPARTICLECALLBACKPROC)(SParticle* Object);


/**
ParticleAnimator is a further 'GameEngine like' tool to simplify particle animations.
Normally for each particle effect (e.g. fire effect, smoke effect etc.) one ParticleAnimator has to be
created and not only one for all effects because you would get problems with callback handling.
The ParticleAnimator does not create any billboards (or sprites). You have to create the sprites yourself.
*/
class SP_EXPORT ParticleAnimator
{
    
    public:
        
        ParticleAnimator();
        virtual ~ParticleAnimator();
        
        virtual SParticle* addParticle(
            scene::Billboard* Object, const EParticleAttribute Attribute, const u64 Endurance, const f32 BlendSpeed,
            const dim::vector3df &Impulse, const dim::vector3df &Gravity = 0.0f,
            const dim::vector3df &Rotation = 0.0f, const dim::vector3df &Transformation = 0.0f
        );
        virtual SParticle* addParticle(const SParticle &Object);
        
        virtual void removeParticle(scene::Billboard* &Object, bool isDeleteBillboard = false);
        virtual void removeParticle(SParticle* &Object, bool isDeleteBillboard = false);
        
        virtual void setParent(scene::SceneNode* Parent, bool isGlobal = false);
        
        virtual void setDestructionCallback(const PFNPARTICLECALLBACKPROC DestructionProc);
        virtual void setEnduranceCallback(const PFNPARTICLECALLBACKPROC EnduranceProc);
        
        //! Updates each particle. Its animation and callback procedures.
        virtual void update();
        
        /* Inline functions */
        
        inline void setBoundingBox(const dim::aabbox3df &BoundBox)
        {
            BoundBox_ = BoundBox;
        }
        inline dim::aabbox3df getBoundingBox() const
        {
            return BoundBox_;
        }
        
        inline void setSpeed(const f32 Speed)
        {
            AnimSpeed_ = Speed;
        }
        inline f32 getSpeed() const
        {
            return AnimSpeed_;
        }
        
        inline scene::SceneNode* getParent() const
        {
            return Parent_;
        }
        
    protected:
        
        /* Functions */
        
        virtual bool updateParticle(std::list<SParticle*>::iterator &it);
        
        virtual void setParticleAlpha(SParticle* Obj, const f32 Alpha);
        virtual void setParticleAlpha(SParticle* Obj);
        
        /* Members */
        
        std::list<SParticle*> ParticleList_;
        scene::SceneNode* Parent_;
        bool ParentGlobal_;
        
        dim::aabbox3df BoundBox_;
        
        f32 AnimSpeed_;
        
        PFNPARTICLECALLBACKPROC pDestructionProc_;
        PFNPARTICLECALLBACKPROC pEnduranceProc_;
        
};


} // /namespace tool
    
} // /namespace sp


#endif

#endif



// ================================================================================
