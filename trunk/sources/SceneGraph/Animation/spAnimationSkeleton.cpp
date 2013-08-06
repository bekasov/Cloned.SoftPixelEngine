/*
 * Animation skeleton file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "SceneGraph/Animation/spAnimationSkeleton.hpp"
#include "Platform/spSoftPixelDeviceOS.hpp"

#include <boost/foreach.hpp>


namespace sp
{

extern video::RenderSystem* GlbRenderSys;

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

AnimationJoint* AnimationSkeleton::findJoint(const io::stringc &Name)
{
    foreach (AnimationJoint* Joint, Joints_)
    {
        if (Joint->getName() == Name)
            return Joint;
    }
    return 0;
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

/*
Surface vertex structure for "updateSkeleton" function.
This can't be a local structure for GCC!
*/
struct SSurfaceVertex
{
    u32 Surface;
    u32 Index;
};

inline bool operator < (const SSurfaceVertex &ObjA, const SSurfaceVertex &ObjB)
{
    return ObjA.Surface < ObjB.Surface || ( ObjA.Surface == ObjB.Surface && ObjA.Index < ObjB.Index );
}

void AnimationSkeleton::updateSkeleton()
{
    /* Add all used surfaces */
    /*Surfaces_.clear();
    
    foreach (AnimationJoint* Joint, Joints_)
    {
        foreach (const SVertexGroup &Vert, Joint->getVertexGroups())
            Surfaces_.push_back(Vert.Surface);
    }
    
    //Surfaces_.sort();
    Surfaces_.unique();*/
    
    /* Setup joint origin transformation and normalize vertex weights */
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

void AnimationSkeleton::transformVertices(Mesh* MeshObj) const
{
    if (!MeshObj)
        return;
    
    /* Reset the vertices to support multi vertex weights */
    //!TODO! -> optimize this, sometimes a vertex will be reseted several times!
    foreach (AnimationJoint* Joint, Joints_)
    {
        foreach (const SVertexGroup &Vert, Joint->getVertexGroups())
        {
            video::MeshBuffer* Surf = MeshObj->getMeshBuffer(Vert.Surface);
            
            if (Surf)
            {
                Surf->setVertexCoord(Vert.Index, 0.0f);
                Surf->setVertexNormal(Vert.Index, 0.0f);
            }
        }
    }
    
    /* Transform the vertices for each joint */
    dim::matrix4f BaseMatrix;
    
    foreach (AnimationJoint* Joint, RootJoints_)
        Joint->transformVertices(MeshObj, BaseMatrix, false);
    
    /* Update vertex buffer for each surface */
    //!TODO! -> optmize this!
    MeshObj->updateVertexBuffer();
    //foreach (video::MeshBuffer* Surf, Surfaces_)
    //    Surf->updateVertexBuffer();
}

void AnimationSkeleton::fillJointTransformations(
    std::vector<dim::matrix4f> &JointMatrices, bool KeepJointOrder) const
{
    if (Joints_.size() < JointMatrices.size())
    {
        u32 i = 0;
        
        if (KeepJointOrder)
        {
            foreach (AnimationJoint* Joint, Joints_)
                JointMatrices[i++] = Joint->getVertexTransformation();
        }
        else
        {
            dim::matrix4f BaseMatrix;
            foreach (AnimationJoint* Joint, RootJoints_)
                fillSubJointTransformations(Joint, BaseMatrix, JointMatrices, i);
        }
    }
    #ifdef SP_DEBUGMODE
    else
        io::Log::debug("AnimationSkeleton::fillJointTransformations", "Joint matrices container is too small");
    #endif
}

/**
Vertex joint weight structure for "setupVertexBufferAttributes" function.
This can't be a local structure for GCC!
*/
struct SWeight
{
    SWeight() :
        JointIndex  (-1     ),
        Weight      (0.0f   )
    {
    }
    SWeight(s32 JntIndex, f32 JntWeight) :
        JointIndex  (JntIndex   ),
        Weight      (JntWeight  )
    {
    }
    SWeight(const SWeight &Other) :
        JointIndex  (Other.JointIndex   ),
        Weight      (Other.Weight       )
    {
    }
    ~SWeight()
    {
    }
    
    /* Operators */
    inline bool operator < (const SWeight &Other) const
    {
        return Weight < Other.Weight;
    }
    
    /* Members */
    s32 JointIndex;
    f32 Weight;
};

struct SVertex
{
    std::vector<SWeight> Weights;
};

bool AnimationSkeleton::setupVertexBufferAttributes(
    Mesh* MeshObj,
    const std::vector<video::SVertexAttribute> &IndexAttributes,
    const std::vector<video::SVertexAttribute> &WeightAttributes) const
{
    /* Check parameter validity */
    if (!MeshObj || IndexAttributes.empty() || WeightAttributes.empty())
    {
        #ifdef SP_DEBUGMODE
        io::Log::debug("AnimationSkeleton::setupVertexBuffers");
        #endif
        return false;
    }
    
    /* Check attributes */
    if (IndexAttributes.size() != WeightAttributes.size())
    {
        io::Log::error("Joint index- and weight attribute lists have different count of elements");
        return false;
    }
    
    if ( !checkAttributeListForHWAnim(IndexAttributes, "index") ||
         !checkAttributeListForHWAnim(WeightAttributes, "weight") )
    {
        return false;
    }
    
    /* Setup weight surface lists */
    std::vector< std::vector<SVertex> > WeightSurfaces(MeshObj->getMeshBufferCount());
    
    for (u32 s = 0; s < MeshObj->getMeshBufferCount(); ++s)
        WeightSurfaces[s].resize(MeshObj->getMeshBuffer(s)->getVertexCount());
    
    /* Setup joint weights for each vertex in the temporary lists */
    const u32 MaxSurface = WeightSurfaces.size();
    
    s32 JointIndex = 0;
    foreach (AnimationJoint* Joint, Joints_)
    {
        foreach (const SVertexGroup &Group, Joint->getVertexGroups())
        {
            if (Group.Surface < MaxSurface && Group.Index < WeightSurfaces[Group.Surface].size())
            {
                WeightSurfaces[Group.Surface][Group.Index].Weights.push_back(
                    SWeight(JointIndex, Group.Weight)
                );
            }
        }
        ++JointIndex;
    }
    
    /* Sort weights */
    for (u32 s = 0; s < WeightSurfaces.size(); ++s)
    {
        foreach (SVertex &Vert, WeightSurfaces[s])
            std::sort(Vert.Weights.begin(), Vert.Weights.end());
    }
    
    /* Setup vertex format */
    const f32 NullVec[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    u32 s = 0;
    
    foreach (video::MeshBuffer* Surface, MeshObj->getMeshBufferList())
    {
        /* Setup vertex weights and joint indices */
        for (u32 i = 0, c = Surface->getVertexCount(); i < c; ++i)
        {
            /* Get vertex weights and joint indices for current vertex */
            const SVertex& Vert = WeightSurfaces[s][i];
            
            const u32 WeightCount = Vert.Weights.size();
            
            /* Arrange indices into the 4-component vectors */
            {
                u32 j = 0;
                std::vector<video::SVertexAttribute>::const_iterator it = IndexAttributes.begin();
                
                while (j < WeightCount && it != IndexAttributes.end())
                {
                    f32 Vec[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
                    
                    for (s32 k = 0; k < it->Size && j < WeightCount; ++k, ++j)
                        Vec[k] = static_cast<f32>(Vert.Weights[j].JointIndex);
                    
                    Surface->setVertexAttribute(i, *it, Vec, sizeof(f32)*it->Size);
                    ++it;
                }
                
                /* Clear rest of attributes */
                for (; it != IndexAttributes.end(); ++it)
                    Surface->setVertexAttribute(i, *it, NullVec, sizeof(f32)*it->Size);
            }
            
            /* Arrange weights into the 4-component vectors */
            {
                u32 j = 0;
                std::vector<video::SVertexAttribute>::const_iterator it = WeightAttributes.begin();
                
                while (j < WeightCount && it != WeightAttributes.end())
                {
                    f32 Vec[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
                    
                    for (s32 k = 0; k < it->Size && j < WeightCount; ++k, ++j)
                        Vec[k] = Vert.Weights[j].Weight;
                    
                    Surface->setVertexAttribute(i, *it, Vec, sizeof(f32)*it->Size);
                    ++it;
                }
                
                /* Clear rest of attributes */
                for (; it != WeightAttributes.end(); ++it)
                    Surface->setVertexAttribute(i, *it, NullVec, sizeof(f32)*it->Size);
            }
        }
        
        Surface->updateVertexBuffer();
        ++s;
    }
    
    return true;
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
    
    GlbRenderSys->setLineSize(3);
    
    GlbRenderSys->draw3DLine(BaseMatrix.getPosition(), BaseMatrix * dim::vector3df(AXIS_SIZE, 0, 0), video::color(255, 0, 0));
    GlbRenderSys->draw3DLine(BaseMatrix.getPosition(), BaseMatrix * dim::vector3df(0, AXIS_SIZE, 0), video::color(0, 255, 0));
    GlbRenderSys->draw3DLine(BaseMatrix.getPosition(), BaseMatrix * dim::vector3df(0, 0, AXIS_SIZE), video::color(0, 0, 255));
    
    GlbRenderSys->setLineSize(1);
    
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
    #define LINE(a, b) GlbRenderSys->draw3DLine(a, b, Color)
    
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


/*
 * ======= Private: =======
 */

bool AnimationSkeleton::checkAttributeListForHWAnim(
    const std::vector<video::SVertexAttribute> &Attributes, const io::stringc &Name) const
{
    foreach (const video::SVertexAttribute &Attr, Attributes)
    {
        if (Attr.Size < 1 || Attr.Size > 4)
        {
            io::Log::error("Invalid attribute size in joint " + Name + " attribute list");
            return false;
        }
        if (Attr.Type != video::DATATYPE_FLOAT)
        {
            io::Log::error("Invalid attribute data-type in joint " + Name + " attribute list (must be DATATYPE_FLOAT)");
            return false;
        }
    }
    return true;
}

void AnimationSkeleton::fillSubJointTransformations(
    AnimationJoint* Joint, dim::matrix4f BaseMatrix, std::vector<dim::matrix4f> &JointMatrices, u32 &Index) const
{
    /* Setup joint transformation */
    BaseMatrix *= Joint->getTransformation().getMatrix();
    JointMatrices[Index++] = BaseMatrix * Joint->getOriginMatrix();
    
    /* Setup joint children */
    foreach (AnimationJoint* Child, Joint->getChildren())
        fillSubJointTransformations(Child, BaseMatrix, JointMatrices, Index);
}


} // /namespace scene

} // /namespace sp



// ================================================================================
