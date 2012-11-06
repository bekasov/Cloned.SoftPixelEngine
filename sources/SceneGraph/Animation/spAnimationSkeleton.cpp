/*
 * Animation skeleton file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "SceneGraph/Animation/spAnimationSkeleton.hpp"
#include "Platform/spSoftPixelDeviceOS.hpp"

#if 1//!!!
#   include "Base/spTimer.hpp"
#endif

#include <boost/foreach.hpp>


namespace sp
{

extern video::RenderSystem* __spVideoDriver;

namespace scene
{


AnimationSkeleton::AnimationSkeleton()
{
}
AnimationSkeleton::~AnimationSkeleton()
{
    MemoryManager::deleteList(Joints_);
}

AnimationJoint* AnimationSkeleton::createJoint(
    const Transformation &OriginTransform, const io::stringc &Name, AnimationJoint* Parent)
{
    /* Create new joint */
    AnimationJoint* Joint = new AnimationJoint(OriginTransform, Name);
    
    /* Setup parnet for the first time */
    if (Parent)
    {
        Joint->setParent(Parent);
        Parent->addChild(Joint);
    }
    else
        RootJoints_.push_back(Joint);
    
    /* Store joint base list */
    Joints_.push_back(Joint);
    
    return Joint;
}

void AnimationSkeleton::deleteJoint(AnimationJoint* Joint)
{
    if (Joint)
    {
        /* Remove joint from parent */
        if (Joint->getParent())
            Joint->getParent()->removeChild(Joint);
        
        /* Remove joint from children */
        foreach (AnimationJoint* Child, Joint->getChildren())
            Child->setParent(0);
        
        /* Remove joint from root list */
        if (!Joint->getParent())
            MemoryManager::removeElement(RootJoints_, Joint);
        
        /* Delete joint finally */
        MemoryManager::removeElement(Joints_, Joint, true);
    }
}

void AnimationSkeleton::setJointParent(AnimationJoint* Joint, AnimationJoint* Parent)
{
    if ( Joint && Joint->getParent() != Parent && Joint != Parent && ( !Parent || Parent->checkParentIncest(Joint) ) )
    {
        /*
         * Add joint to the root list if it will have no parent or
         * remove it from the root list when it had no parent but it will get one
         */
        if (!Parent)
            RootJoints_.push_back(Joint);
        else if (!Joint->getParent())
            MemoryManager::removeElement(RootJoints_, Joint);
        
        /* Update parent and children references */
        if (Joint->getParent())
            Joint->getParent()->removeChild(Joint);
        
        Joint->setParent(Parent);
        
        if (Joint->getParent())
            Joint->getParent()->addChild(Joint);
    }
}

void AnimationSkeleton::updateSkeleton()
{
    /* Add all used surfaces */
    Surfaces_.clear();
    
    foreach (AnimationJoint* Joint, Joints_)
    {
        foreach (const SVertexGroup &Vert, Joint->getVertexGroups())
            Surfaces_.push_back(Vert.Surface);
    }
    
    Surfaces_.unique();
    
    /* Setup joint origin transformation and normalize vertex weights */
    struct SSurfaceVertex
    {
        /* Operators */
        inline bool operator < (const SSurfaceVertex &Other) const
        {
            return Surface < Other.Surface || ( Surface == Other.Surface && Index < Other.Index );
        }
        
        /* Members */
        video::MeshBuffer* Surface;
        u32 Index;
    };
    
    typedef std::list<SVertexGroup*> TGroupList;
    
    std::map<SSurfaceVertex, TGroupList> JointWeights;
    
    foreach (AnimationJoint* Joint, Joints_)
    {
        /* Store origin transformation */
        Joint->OriginMatrix_ = Joint->getGlobalTransformation().getInverse();
        
        /* Store vertex weights in the map */
        foreach (SVertexGroup &Group, Joint->getVertexGroups())
        {
            SSurfaceVertex SurfVert;
            {
                SurfVert.Surface    = Group.Surface;
                SurfVert.Index      = Group.Index;
            }
            JointWeights[SurfVert].push_back(&Group);
        }
    }
    
    /* Normalize vertex weights from the map */
    for (std::map<SSurfaceVertex, TGroupList>::iterator it = JointWeights.begin(); it != JointWeights.end(); ++it)
    {
        f32 WeightSum = 0.0f;
        
        foreach (SVertexGroup* Group, it->second)
            WeightSum += Group->Weight;
        
        if (WeightSum > math::ROUNDING_ERROR)
        {
            WeightSum = 1.0f / WeightSum;
            
            foreach (SVertexGroup* Group, it->second)
                Group->Weight *= WeightSum;
        }
    }
}

void AnimationSkeleton::transformVertices()
{
    /* Reset the vertices to support multi vertex weights */
    foreach (AnimationJoint* Joint, Joints_)
    {
        foreach (const SVertexGroup &Vert, Joint->getVertexGroups())
        {
            Vert.Surface->setVertexCoord(Vert.Index, 0.0f);
            Vert.Surface->setVertexNormal(Vert.Index, 0.0f);
        }
    }
    
    /* Transform the vertices for each joint */
    dim::matrix4f BaseMatrix;
    
    foreach (AnimationJoint* Joint, RootJoints_)
        Joint->transformVertices(BaseMatrix, false);
    
    /* Update vertex buffer for each surfaec */
    foreach (video::MeshBuffer* Surf, Surfaces_)
        Surf->updateVertexBuffer();
}

void AnimationSkeleton::render(const dim::matrix4f &BaseMatrix, const video::color &Color)
{
    foreach (AnimationJoint* Joint, RootJoints_)
    {
        drawJointConnections(
            Joint, BaseMatrix * Joint->getTransformation().getMatrix(), Color
        );
    }
}


/*
 * ======= Protected: =======
 */

void AnimationSkeleton::drawJointConnections(
    AnimationJoint* Joint, dim::matrix4f BaseMatrix, const video::color &Color)
{
    /* Draw joint orientation */
    static const f32 AXIS_SIZE = 0.5f;
    
    __spVideoDriver->setLineSize(3);
    
    __spVideoDriver->draw3DLine(BaseMatrix.getPosition(), BaseMatrix * dim::vector3df(AXIS_SIZE, 0, 0), video::color(255, 0, 0));
    __spVideoDriver->draw3DLine(BaseMatrix.getPosition(), BaseMatrix * dim::vector3df(0, AXIS_SIZE, 0), video::color(0, 255, 0));
    __spVideoDriver->draw3DLine(BaseMatrix.getPosition(), BaseMatrix * dim::vector3df(0, 0, AXIS_SIZE), video::color(0, 0, 255));
    
    __spVideoDriver->setLineSize(1);
    
    /* Draw joint connections */
    foreach (AnimationJoint* Child, Joint->getChildren())
    {
        /* Get child matrix */
        const dim::matrix4f ChildMatrix(
            BaseMatrix * Child->getTransformation().getMatrix()
        );
        
        /* Get joint connector transformation */
        dim::matrix4f DirMat(
            dim::getDirectionMatrix(BaseMatrix.getPosition(), ChildMatrix.getPosition())
        );
        
        const f32 Length = math::getDistance(BaseMatrix.getPosition(), ChildMatrix.getPosition());
        
        DirMat.scale(-Length);
        
        /* Draw current joint connection */
        drawJointConnector(DirMat, Color);
        
        /* Draw child joint connections */
        drawJointConnections(Child, ChildMatrix, Color);
    }
}

void AnimationSkeleton::drawJointConnector(const dim::matrix4f &Matrix, const video::color &Color)
{
    #define LINE(a, b) __spVideoDriver->draw3DLine(a, b, Color)
    
    /* Generate all control points */
    static const f32 CORNER_SIZE = 0.1f;
    
    const dim::vector3df Start(Matrix.getPosition());
    const dim::vector3df End(Matrix * dim::vector3df(0, 0, 1));
    
    const dim::vector3df CornerA(Matrix * dim::vector3df(-CORNER_SIZE,  CORNER_SIZE, CORNER_SIZE));
    const dim::vector3df CornerB(Matrix * dim::vector3df( CORNER_SIZE,  CORNER_SIZE, CORNER_SIZE));
    const dim::vector3df CornerC(Matrix * dim::vector3df( CORNER_SIZE, -CORNER_SIZE, CORNER_SIZE));
    const dim::vector3df CornerD(Matrix * dim::vector3df(-CORNER_SIZE, -CORNER_SIZE, CORNER_SIZE));
    
    /* Draw all lines */
    LINE(Start, CornerA); LINE(Start, CornerB);
    LINE(Start, CornerC); LINE(Start, CornerD);
    
    LINE(CornerA, CornerB); LINE(CornerB, CornerC);
    LINE(CornerC, CornerD); LINE(CornerD, CornerA);
    
    LINE(End, CornerA); LINE(End, CornerB);
    LINE(End, CornerC); LINE(End, CornerD);
    
    #undef LINE
}


} // /namespace scene

} // /namespace sp



// ================================================================================
