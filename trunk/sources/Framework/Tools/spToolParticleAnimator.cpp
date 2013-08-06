/*
 * Particle animator file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Framework/Tools/spToolParticleAnimator.hpp"

#ifdef SP_COMPILE_WITH_PARTICLEANIMATOR


#include "Base/spTimer.hpp"
#include "Base/spSharedObjects.hpp"
#include "Platform/spSoftPixelDeviceOS.hpp"
#include "SceneGraph/spSceneManager.hpp"


namespace sp
{

extern SoftPixelDevice* GlbEngineDev;
extern scene::SceneGraph* GlbSceneGraph;

namespace tool
{


/*
 * Static internal functions
 */

bool DefaultParticleDestructionProc(SParticle* Object)
{
    return true; // Delete the billboard object
}

bool DefaultParticleEnduranceProc(SParticle* Object)
{
    if (io::Timer::millisecs() > Object->Time + Object->Endurance)
    {
        Object->BlendMode = PARTICLE_BLENDOUT;
        return true;
    }
    return false;
}


/*
 * ParticleAnimator class
 */

ParticleAnimator::ParticleAnimator()
    : Parent_(0), pDestructionProc_(DefaultParticleDestructionProc), pEnduranceProc_(DefaultParticleEnduranceProc)
{
    AnimSpeed_      = 1.0f;
    ParentGlobal_   = false;
}
ParticleAnimator::~ParticleAnimator()
{
    for (std::list<SParticle*>::iterator it = ParticleList_.begin(); it != ParticleList_.end(); ++it)
        MemoryManager::deleteMemory(*it);
}

SParticle* ParticleAnimator::addParticle(
    scene::Billboard* Object, const EParticleAttribute Attribute, const u64 Endurance, const f32 BlendSpeed,
    const dim::vector3df &Impulse, const dim::vector3df &Gravity,
    const dim::vector3df &Rotation, const dim::vector3df &Transformation)
{
    SParticle* NewParticle = new SParticle;
    {
        NewParticle->Object         = Object;
        
        NewParticle->Translation    = Impulse;
        NewParticle->Gravity        = Gravity;
        NewParticle->Rotation       = Rotation;
        NewParticle->Transformation = Transformation;
        
        NewParticle->Time           = io::Timer::millisecs();
        NewParticle->Endurance      = Endurance;
        NewParticle->Alpha          = 1.0f;
        NewParticle->BlendSpeed     = BlendSpeed;
        
        NewParticle->Attribute      = Attribute;
        
        if (Parent_)
            Object->setParent(Parent_, ParentGlobal_);
    }
    ParticleList_.push_back(NewParticle);
    
    return NewParticle;
}

SParticle* ParticleAnimator::addParticle(const SParticle &Object)
{
    SParticle* NewParticle = new SParticle;
    
    memcpy(NewParticle, &Object, sizeof(SParticle));
    
    if (Parent_)
        NewParticle->Object->setParent(Parent_, ParentGlobal_);
    
    ParticleList_.push_back(NewParticle);
    
    return NewParticle;
}

void ParticleAnimator::removeParticle(scene::Billboard* &Object, bool isDeleteBillboard)
{
    for (std::list<SParticle*>::iterator it = ParticleList_.begin(); it != ParticleList_.end(); ++it)
    {
        if ((*it)->Object == Object)
        {
            if (isDeleteBillboard)
                GlbSceneGraph->deleteNode(Object);
            MemoryManager::deleteMemory(*it);
            ParticleList_.erase(it);
            break;
        }
    }
}

void ParticleAnimator::removeParticle(SParticle* &Object, bool isDeleteBillboard)
{
    for (std::list<SParticle*>::iterator it = ParticleList_.begin(); it != ParticleList_.end(); ++it)
    {
        if (*it == Object)
        {
            if (isDeleteBillboard)
                GlbSceneGraph->deleteNode(Object->Object);
            MemoryManager::deleteMemory(Object);
            ParticleList_.erase(it);
            break;
        }
    }
}

void ParticleAnimator::setParent(scene::SceneNode* Parent, bool isGlobal)
{
    Parent_         = Parent;
    ParentGlobal_   = isGlobal;
    
    for (std::list<SParticle*>::iterator it = ParticleList_.begin(); it != ParticleList_.end(); ++it)
        (*it)->Object->setParent(Parent_, isGlobal);
}

void ParticleAnimator::setDestructionCallback(const PFNPARTICLECALLBACKPROC DestructionProc)
{
    if (DestructionProc)
        pDestructionProc_ = DestructionProc;
    else
        pDestructionProc_ = DefaultParticleDestructionProc;
}
void ParticleAnimator::setEnduranceCallback(const PFNPARTICLECALLBACKPROC EnduranceProc)
{
    if (EnduranceProc)
        pEnduranceProc_ = EnduranceProc;
    else
        pEnduranceProc_ = DefaultParticleEnduranceProc;
}

void ParticleAnimator::update()
{
    for (std::list<SParticle*>::iterator it = ParticleList_.begin(); it != ParticleList_.end();)
    {
        if (!updateParticle(it))
            ++it;
    }
}


/*
 * ======= Protected: =======
 */

bool ParticleAnimator::updateParticle(std::list<SParticle*>::iterator &it)
{
    SParticle* Obj = *it;
    
    /* Update the transformations */
    Obj->Translation += Obj->Gravity * AnimSpeed_;
    
    Obj->Object->translate(Obj->Translation * AnimSpeed_);
    Obj->Object->transform(Obj->Transformation * AnimSpeed_);
    
    if (!Obj->Rotation.empty())
        Obj->Object->turn(Obj->Rotation * AnimSpeed_);
    
    /* Update the blending mode */
    if (Obj->BlendMode == PARTICLE_BLENDIN)
    {
        Obj->Alpha += Obj->BlendSpeed * AnimSpeed_;
        if (Obj->Alpha >= 1.0f)
        {
            Obj->Alpha = 1.0f;
            Obj->BlendMode = PARTICLE_NOBLENDING;
        }
    }
    else if (Obj->BlendMode == PARTICLE_BLENDOUT)
    {
        Obj->Alpha -= Obj->BlendSpeed * AnimSpeed_;
        if (Obj->Alpha <= 0.0f)
        {
            Obj->Alpha = 0.0f;
            Obj->BlendMode = PARTICLE_NOBLENDING;
        }
    }
    
    setParticleAlpha(Obj);
    
    /* Check if the particle's time to blend out has come */
    if (pEnduranceProc_(Obj))
    {
        /* Check if the particle is invisible */
        if (Obj->Alpha <= Obj->BlendSpeed)
        {
            if (Obj->Attribute == PARTICLE_ONESHOT && pDestructionProc_(Obj))
            {
                GlbSceneGraph->deleteNode(Obj->Object);
                it = ParticleList_.erase(it);
                return true;
            }
            else if (Obj->Attribute == PARTICLE_LOOP)
            {
                Obj->Time = io::Timer::millisecs();
                
                pDestructionProc_(Obj);
            }
        }
    }
    
    return false;
}

void ParticleAnimator::setParticleAlpha(SParticle* Obj, const f32 Alpha)
{
    video::color Color(Obj->Object->getMaterial()->getDiffuseColor());
    Color.Alpha = u8(Alpha * 255);
    Obj->Object->getMaterial()->setDiffuseColor(Color);
}
void ParticleAnimator::setParticleAlpha(SParticle* Obj)
{
    setParticleAlpha(Obj, Obj->Alpha);
}


} // /namespace tool

} // /namespace sp


#endif



// ================================================================================
