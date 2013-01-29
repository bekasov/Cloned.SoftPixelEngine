/*
 * Mesh scene node file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "SceneGraph/spSceneGraph.hpp"
#include "SceneGraph/spMeshModifier.hpp"
#include "Platform/spSoftPixelDeviceOS.hpp"

#include <boost/foreach.hpp>


namespace sp
{

extern video::RenderSystem* __spVideoDriver;
extern scene::SceneGraph* __spSceneManager;

namespace scene
{


/*
 * Internal comparision structures
 */

struct SCmpTransTriangle
{
    u32 Index;
    u32 a, b, c;
    u32 AlphaSum;
};


/*
 * Internal comparision functions
 */

static bool sortMeshBufferProc(video::MeshBuffer* &Obj1, video::MeshBuffer* &Obj2)
{
    return Obj1->sortCompare(*Obj2);
}

bool cmpTransparentSurface(video::MeshBuffer* obj1, video::MeshBuffer* obj2)
{
    u32 AlphaSum1 = obj1->getName().val<u32>();
    u32 AlphaSum2 = obj2->getName().val<u32>();
    
    if (AlphaSum1 != AlphaSum2)
        return AlphaSum1 < AlphaSum2;
    
    return (long)obj1 < (long)obj2;
}

bool cmpTransparentTriangle(const SCmpTransTriangle &obj1, const SCmpTransTriangle &obj2)
{
    if (obj1.AlphaSum != obj2.AlphaSum)
        return obj1.AlphaSum < obj2.AlphaSum;
    return obj1.Index < obj2.Index;
}


/*
 * ======= Constructors =======
 */

Mesh::Mesh() :
    MaterialNode        (NODE_MESH          ),
    SurfaceList_        (&OrigSurfaceList_  ),
    LODSurfaceList_     (&OrigSurfaceList_  ),
    UseLODSubMeshes_    (false              ),
    LODSubMeshDistance_ (25.0f              ),
    Reference_          (0                  ),
    UserRenderProc_     (0                  )
{
}
Mesh::~Mesh()
{
    MemoryManager::deleteList(OrigSurfaceList_);
}


/*
 * ======= Textureing =======
 */

void Mesh::addTexture(video::Texture* Tex, const u8 Layer)
{
    foreach (video::MeshBuffer* Surface, OrigSurfaceList_)
        Surface->addTexture(Tex, Layer);
}

void Mesh::textureAutoMap(
    const u8 Layer, const f32 Density, const u32 MeshBufferIndex, bool GlobalProjection, bool AllowNegativeTexCoords)
{
    if (Layer >= MAX_COUNT_OF_TEXTURES)
    {
        #ifdef SP_DEBUGMODE
        io::Log::debug("Mesh::textureAutoMap", "'Layer' index out of range");
        #endif
        return;
    }
    
    /* Initialization settings */
    u32 Indices[3];
    dim::vector3df AbsNormal, Normal, Pos;
    dim::point2df TexCoord;
    dim::triangle3df Face;
    s32 AxisType;
    
    std::vector<video::MeshBuffer*>::iterator it = OrigSurfaceList_.begin(), itEnd = OrigSurfaceList_.end();
    
    /* Get transformation matrices (complete transformation and rotation ) */
    dim::matrix4f Transformation, Rotation;
    
    if (GlobalProjection)
    {
        Transformation  = getTransformMatrix(true);
        Rotation        = getRotationMatrix(true);
    }
    else
        Transformation  = getScaleMatrix(true);
    
    /* Check if only one mesh buffer is to be auto mapped */
    if (MeshBufferIndex != MESHBUFFER_IGNORE)
    {
        if (MeshBufferIndex >= getMeshBufferCount())
        {
            #ifdef SP_DEBUGMODE
            io::Log::debug("Mesh::textureAutoMap", "'MeshBufferIndex' index out of range");
            #endif
            return;
        }
        it += MeshBufferIndex;
        itEnd = it + 1;
    }
    
    /* Auto map each mesh buffer */
    for (; it != itEnd; ++it)
    {
        if ((*it)->getPrimitiveType() != video::PRIMITIVE_TRIANGLES)
            continue;
        
        for (u32 i = 0, j, c = (*it)->getTriangleCount(); i < c; ++i)
        {
            (*it)->getTriangleIndices(i, Indices);
            
            Face = (*it)->getTriangleCoords(i);
            
            /* Compute mapping direction */
            Normal = Rotation.vecRotate(Face.getNormal());
            AbsNormal = Normal.getAbs();
            
            if (AbsNormal.X >= AbsNormal.Y && AbsNormal.X >= AbsNormal.Z)
                AxisType = (AllowNegativeTexCoords || Normal.X > 0.0f ? 0 : 1); // x
            else if (AbsNormal.Y >= AbsNormal.X && AbsNormal.Y >= AbsNormal.Z)
                AxisType = (AllowNegativeTexCoords || Normal.Y > 0.0f ? 2 : 3); // y
            else
                AxisType = (AllowNegativeTexCoords || Normal.Z > 0.0f ? 4 : 5); // z
            
            /* Set new texture coordinates */
            for (j = 0; j < 3; ++j)
            {
                Pos = Transformation * Face[j];
                
                switch (AxisType)
                {
                    case 0: TexCoord = dim::point2df( Pos.Z, -Pos.Y); break; // +x
                    case 1: TexCoord = dim::point2df(-Pos.Z, -Pos.Y); break; // -x
                    case 2: TexCoord = dim::point2df( Pos.X, -Pos.Z); break; // +y
                    case 3: TexCoord = dim::point2df( Pos.X,  Pos.Z); break; // -y
                    case 4: TexCoord = dim::point2df(-Pos.X, -Pos.Y); break; // +z
                    case 5: TexCoord = dim::point2df( Pos.X, -Pos.Y); break; // -z
                }
                
                (*it)->setVertexTexCoord(Indices[j], TexCoord * Density, Layer);
            }
        }
        
        (*it)->updateVertexBuffer();
    }
}

std::list<video::Texture*> Mesh::getTextureList() const
{
    std::list<video::Texture*> TexList;
    
    foreach (video::MeshBuffer* Surface, OrigSurfaceList_)
    {
        foreach (video::Texture* Tex, Surface->getTextureList())
            TexList.push_back(Tex);
    }
    
    return TexList;
}

u32 Mesh::getTextureCount() const
{
    u32 TexCount = 0;
    
    foreach (video::MeshBuffer* Surface, OrigSurfaceList_)
        TexCount += Surface->getTextureCount();
    
    return TexCount;
}


/*
 * ======= Mesh building =======
 */

void Mesh::updateNormals()
{
    foreach (video::MeshBuffer* Surface, OrigSurfaceList_)
        Surface->updateNormals(Material_.getShading());
}
void Mesh::updateTangentSpace(const u8 TangentLayer, const u8 BinormalLayer, bool UpdateNormals)
{
    foreach (video::MeshBuffer* Surface, OrigSurfaceList_)
        Surface->updateTangentSpace(TangentLayer, BinormalLayer, UpdateNormals);
}
void Mesh::updateVertexBuffer()
{
    foreach (video::MeshBuffer* Surface, OrigSurfaceList_)
        Surface->updateVertexBuffer();
}
void Mesh::updateIndexBuffer()
{
    foreach (video::MeshBuffer* Surface, OrigSurfaceList_)
        Surface->updateIndexBuffer();
}
void Mesh::updateMeshBuffer()
{
    foreach (video::MeshBuffer* Surface, OrigSurfaceList_)
        Surface->updateMeshBuffer();
}

void Mesh::meshTranslate(const dim::vector3df &Direction)
{
    foreach (video::MeshBuffer* Surface, OrigSurfaceList_)
        Surface->meshTranslate(Direction);
}
void Mesh::meshTransform(const dim::vector3df &Size)
{
    foreach (video::MeshBuffer* Surface, OrigSurfaceList_)
        Surface->meshTransform(Size);
}
void Mesh::meshTransform(const dim::matrix4f &Matrix)
{
    foreach (video::MeshBuffer* Surface, OrigSurfaceList_)
        Surface->meshTransform(Matrix);
}
void Mesh::meshTurn(const dim::vector3df &Rotation)
{
    foreach (video::MeshBuffer* Surface, OrigSurfaceList_)
        Surface->meshTransform(dim::getRotationMatrix(Rotation));
}

void Mesh::meshFlip()
{
    foreach (video::MeshBuffer* Surface, OrigSurfaceList_)
        Surface->meshFlip();
}
void Mesh::meshFlip(bool isXAxis, bool isYAxis, bool isZAxis)
{
    foreach (video::MeshBuffer* Surface, OrigSurfaceList_)
        Surface->meshFlip(isXAxis, isYAxis, isZAxis);
}

void Mesh::meshFit(const dim::vector3df &Position, const dim::vector3df &Size)
{
    MeshModifier::meshFit(*this, Position, Size);
}
void Mesh::meshSpherify(f32 Factor)
{
    MeshModifier::meshSpherify(*this, Factor);
}
void Mesh::meshTwist(f32 Rotation)
{
    MeshModifier::meshTwist(*this, Rotation);
}

void Mesh::mergeFamily(bool isDeleteChildren)
{
    #ifdef SP_DEBUGMODE
    io::Log::debug("Mesh::mergeFamily", "Not implemented yet");
    #endif
    
    /*u32 MeshChildrenCount = 0;
    Mesh* CurMesh = 0;
    
    for (std::list<Node*>::iterator it = Children_.begin(); it != Children_.end();)
    {
        if ((*it)->getType() == NODE_MESH)
        {
            ++it;
            continue;
        }
        
        CurMesh = (Mesh*)(*it);
        
        
        
        
        if (isDeleteChildren)
            __spSceneManager->deleteNode(*it);
        
        ++MeshChildrenCount;
        it = Children_.erase(it);
    }
    
    if (MeshChildrenCount)
        updateMeshBuffer();*/
}

dim::vector3df Mesh::centerOrigin()
{
    const dim::vector3df Center(getMeshBoundingBox().getCenter());
    
    /* Fit the mesh */
    foreach (video::MeshBuffer* Surface, OrigSurfaceList_)
    {
        for (u32 i = 0; i < Surface->getVertexCount(); ++i)
            Surface->setVertexCoord(i, Surface->getVertexCoord(i) - Center);
        Surface->updateVertexBuffer();
    }
    
    /* Fit the object */
    move(getScaleMatrix() * Center);
    
    return -Center;
}

void Mesh::clipConcatenatedTriangles()
{
    foreach (video::MeshBuffer* Surface, OrigSurfaceList_)
        Surface->clipConcatenatedTriangles();
}

void Mesh::flipTriangles()
{
    foreach (video::MeshBuffer* Surface, OrigSurfaceList_)
        Surface->flipTriangles();
}


/*
 * ======= LOD (level-of-detail) =======
 */

void Mesh::addLODSubMesh(Mesh* LODSubMesh, bool isCopyMaterials)
{
    LODSubMeshList_.push_back(LODSubMesh);
    
    LODSubMesh->setVisible(false);
    
    if (isCopyMaterials)
    {
        LODSubMesh->Material_.copy(&Material_);
        
        for (u32 s = 0; s < LODSubMesh->OrigSurfaceList_.size() && s < OrigSurfaceList_.size(); ++s)
            (LODSubMesh->OrigSurfaceList_)[s]->setSurfaceTextureList(OrigSurfaceList_[s]->getSurfaceTextureList());
    }
}
void Mesh::clearLODSubMeshes()
{
    LODSubMeshList_.clear();
    setLOD(false);
}

void Mesh::setLODSubMeshList(const std::vector<Mesh*> &LODSubMeshList)
{
    LODSubMeshList_ = LODSubMeshList;
    setLOD(!LODSubMeshList_.empty());
}

void Mesh::setLODDistance(f32 Distance)
{
    LODSubMeshDistance_ = math::Abs(Distance);
}

void Mesh::setLOD(bool Enable)
{
    UseLODSubMeshes_ = Enable;
    
    if (!Enable)
        LODSurfaceList_ = SurfaceList_;
}


/*
 * ======= Surfaces =======
 */

video::MeshBuffer* Mesh::createMeshBuffer(
    const video::VertexFormat* VertexFormat, const video::ERendererDataTypes IndexFormat)
{
    /* Create new mesh buffer */
    video::MeshBuffer* NewBuffer = new video::MeshBuffer(VertexFormat, IndexFormat);
    OrigSurfaceList_.push_back(NewBuffer);
    
    /* Allocate hardware mesh buffer */
    NewBuffer->createMeshBuffer();
    
    return NewBuffer;
}

void Mesh::deleteMeshBuffer(const u32 Index)
{
    if (Index < getMeshBufferCount())
    {
        MemoryManager::deleteMemory(OrigSurfaceList_[Index]);
        OrigSurfaceList_.erase(OrigSurfaceList_.begin() + Index);
    }
    #ifdef SP_DEBUGMODE
    else
        io::Log::debug("Mesh::deleteMeshBuffer", "'Index' out of range");
    #endif
}

void Mesh::deleteMeshBuffers()
{
    foreach (video::MeshBuffer* Surface, OrigSurfaceList_)
        MemoryManager::deleteMemory(Surface);
    OrigSurfaceList_.clear();
}

void Mesh::mergeMeshBuffers()
{
    if (OrigSurfaceList_.empty())
        return;
    
    /* Copy and the surface list */
    std::list<video::MeshBuffer*> SurfaceList(OrigSurfaceList_.begin(), OrigSurfaceList_.end());
    SurfaceList.sort(sortMeshBufferProc);
    
    /* Delete all old surfaces' mesh buffers but don't delete the surface memory until the end of optimization */
    foreach (video::MeshBuffer* Surface, OrigSurfaceList_)
        Surface->deleteMeshBuffer();
    OrigSurfaceList_.clear();
    
    /* Find all equal mesh buffers */
    std::list<video::MeshBuffer*>::iterator it, itSub, itStart;
    
    bool GenLastGroup = false;
    it = itStart = SurfaceList.begin();
    
    while (1)
    {
        /* Check if a new different mesh buffer has been found or the end as been arrived */
        if ( GenLastGroup || ( it != SurfaceList.end() && it != itStart && !(*it)->compare(**itStart) ) )
        {
            /* Create new mesh buffer for founded summarized group */
            video::MeshBuffer* Surface = createMeshBuffer(
                (*itStart)->getVertexFormat(), (*itStart)->getIndexFormat()->getDataType()
            );
            
            /* Setup mesh buffer settings */
            Surface->setSurfaceTextureList((*itStart)->getSurfaceTextureList());
            Surface->setIndexBufferEnable((*itStart)->getIndexBufferEnable());
            Surface->setPrimitiveType((*itStart)->getPrimitiveType());
            
            /* Merge founded mesh buffer group */
            io::stringc Name;
            
            for (itSub = itStart; itSub != it; ++itSub)
            {
                /* Insert current mesh buffer and add the name to the final name-stack */
                Surface->insertMeshBuffer(**itSub);
                if ((*itSub)->getName().size())
                    Name += (*itSub)->getName() + ";";
            }
            
            Surface->setName(Name);
            
            /* Finalize new summarized mesh buffer */
            Surface->updateMeshBuffer();
            
            /* Set new start position */
            itStart = it;
        }
        
        if (GenLastGroup)
            break;
        
        if (it != SurfaceList.end())
            ++it;
        else
            GenLastGroup = true;
    }
}

// !!!TESTING unfinished!!!
void Mesh::optimizeTransparency()
{
    #ifdef SP_DEBUGMODE
    io::Log::debug("Mesh::optimizeTransparency", "Not yet implemented");
    #endif
    
    #if 0 // !!!!!!!!!!!!!!!!!!!!!!!!! TODO !!!!!!!!!!!!!!!!!!!!!!!!!
    
    /* Sort the surfaces in dependent of the most transparent vertices */
    std::map<u32, io::stringc> SurfaceNameList;
    
    u32 s, i;
    u32 AlphaSum;
    
    for (s = 0; s < getSurfaceCount(); ++s)
    {
        AlphaSum = 0;
        
        for (i = 0; i < (*SurfaceList_)[s]->VerticesList.size(); ++i)
            AlphaSum += (255 - (*SurfaceList_)[s]->VerticesList[i].getColor().Alpha);
        
        SurfaceNameList[(*SurfaceList_)[s]->getID()] = (*SurfaceList_)[s]->Name;
        (*SurfaceList_)[s]->Name = io::stringc(AlphaSum);
    }
    
    std::sort(SurfaceList_->begin(), SurfaceList_->end(), cmpTransparentSurface);
    
    for (s = 0; s < SurfaceNameList.size(); ++s)
        (*SurfaceList_)[s]->Name = SurfaceNameList[(*SurfaceList_)[s]->getID()];
    
    /* Sort the triangles in dependent of the most transparent vertices */
    std::list<SCmpTransTriangle> NewTriangleList;
    SCmpTransTriangle TransTriangle;
    
    for (std::vector<video::MeshBuffer*>::iterator itSurf = SurfaceList_->begin(); itSurf != SurfaceList_->end(); ++itSurf)
    {
        i = 0;
        for (std::vector<SMeshTriangle3D>::iterator it = (*itSurf)->TriangleList.begin(); it != (*itSurf)->TriangleList.end(); ++it, ++i)
        {
            TransTriangle.Index = i;
            
            TransTriangle.a = it->a;
            TransTriangle.b = it->b;
            TransTriangle.c = it->c;
            
            TransTriangle.AlphaSum = 255*3;
            TransTriangle.AlphaSum -= (*itSurf)->VerticesList[it->a].getColor().Alpha;
            TransTriangle.AlphaSum -= (*itSurf)->VerticesList[it->b].getColor().Alpha;
            TransTriangle.AlphaSum -= (*itSurf)->VerticesList[it->c].getColor().Alpha;
            
            NewTriangleList.push_back(TransTriangle);
        }
        
        NewTriangleList.sort(cmpTransparentTriangle);
        
        i = 0;
        for (std::list<SCmpTransTriangle>::iterator it = NewTriangleList.begin(); it != NewTriangleList.end(); ++it, ++i)
        {
            (*itSurf)->TriangleList[i].a = it->a;
            (*itSurf)->TriangleList[i].b = it->b;
            (*itSurf)->TriangleList[i].c = it->c;
        }
        
        NewTriangleList.clear();
    }
    
    #endif
    
    updateIndexBuffer();
}


/*
 * ======= Vertices =======
 */

u32 Mesh::getVertexCount() const
{
    u32 VertCount = 0;
    
    foreach (video::MeshBuffer* Surface, *SurfaceList_)
        VertCount += Surface->getReference()->getVertexCount();
    
    return VertCount;
}
u32 Mesh::getTriangleCount() const
{
    u32 TriCount = 0;
    
    foreach (video::MeshBuffer* Surface, *SurfaceList_)
        TriCount += Surface->getReference()->getTriangleCount();
    
    return TriCount;
}

u32 Mesh::getOrigVertexCount() const
{
    u32 VertCount = 0;
    
    foreach (video::MeshBuffer* Surface, OrigSurfaceList_)
        VertCount += Surface->getVertexCount();
    
    return VertCount;
}
u32 Mesh::getOrigTriangleCount() const
{
    u32 TriCount = 0;
    
    foreach (video::MeshBuffer* Surface, OrigSurfaceList_)
        TriCount += Surface->getTriangleCount();
    
    return TriCount;
}

bool Mesh::getMeshBoundingBox(dim::vector3df &Min, dim::vector3df &Max, bool isGlobal) const
{
    if (!getVertexCount())
    {
        #ifdef SP_DEBUGMODE
        io::Log::debug("Mesh::getMeshBoundingBox", "No vertices to compute bounding box");
        #endif
        return false;
    }
    
    const dim::matrix4f Matrix(isGlobal ? getTransformMatrix(true) : dim::matrix4f());
    dim::aabbox3df BoundBox(dim::aabbox3df::OMEGA);
    
    /* Get bounding box of the current surface */
    foreach (video::MeshBuffer* Surface, *SurfaceList_)
    {
        for (u32 i = 0; i < Surface->getVertexCount(); ++i)
            BoundBox.insertPoint(Matrix * Surface->getVertexCoord(i));
    }
    
    Min = BoundBox.Min;
    Max = BoundBox.Max;
    
    return true;
}

dim::aabbox3df Mesh::getMeshBoundingBox(bool isGlobal) const
{
    dim::aabbox3df BoundBox;
    getMeshBoundingBox(BoundBox.Min, BoundBox.Max, isGlobal);
    return BoundBox;
}

f32 Mesh::getMeshBoundingSphere(bool isGlobal) const
{
    if (!getVertexCount())
    {
        #ifdef SP_DEBUGMODE
        io::Log::debug("Mesh::getMeshBoundingSphere", "No vertices to compute bounding sphere");
        #endif
        return 0.0f;
    }
    
    const dim::matrix4f Matrix(isGlobal ? getTransformMatrix(true) : dim::matrix4f());
    f32 Radius = 0.0f;
    
    /* Loop for each surface's vertex */
    foreach (video::MeshBuffer* Surface, *SurfaceList_)
    {
        for (u32 i = 0; i < Surface->getVertexCount(); ++i)
            math::Increase(Radius, (Matrix * Surface->getVertexCoord(i)).getLengthSq());
    }
    
    return (Radius > 0.0f ? sqrtf(Radius) : Radius);
}

void Mesh::setReference(Mesh* ReferenceMesh, bool CopyLocation, bool CopyMaterial)
{
    Reference_ = ReferenceMesh;
    
    if (Reference_)
    {
        LODSurfaceList_ = SurfaceList_ = Reference_->SurfaceList_;
        
        //if (CopyBoundVolume)
            BoundVolume_ = Reference_->BoundVolume_;
        
        if (CopyLocation)
        {
            setPositionMatrix(Reference_->getPositionMatrix());
            setRotationMatrix(Reference_->getRotationMatrix());
            setScaleMatrix(Reference_->getScaleMatrix());
        }
        
        if (CopyMaterial)
            Material_.copy(&(Reference_->Material_));
    }
    else
    {
        SurfaceList_    = &OrigSurfaceList_;
        LODSurfaceList_ = &OrigSurfaceList_;
    }
}

Mesh* Mesh::getReference()
{
    if (Reference_)
        return Reference_->getReference();
    return this;
}
const Mesh* Mesh::getReference() const
{
    if (Reference_)
        return Reference_->getReference();
    return this;
}


/*
 * ======= Something else =======
 */

void Mesh::paint(const video::color &Color, bool CombineColors)
{
    foreach (video::MeshBuffer* Surface, OrigSurfaceList_)
        Surface->paint(Color, CombineColors);
}

void Mesh::setShading(const video::EShadingTypes Type, bool UpdateImmediate)
{
    Material_.setShading(Type);
    if (UpdateImmediate)
        updateNormals();
}

void Mesh::copy(const Mesh* Other)
{
    if (Other)
    {
        Other->copyRoot(this);
        Other->copyMesh(this);
    }
}

Mesh* Mesh::copy() const
{
    Mesh* NewMesh = MemoryManager::createMemory<Mesh>();
    
    copyRoot(NewMesh);
    copyMesh(NewMesh);
    
    return NewMesh;
}

void Mesh::copyMesh(Mesh* NewMesh) const // !ANY ERROR DETECTED! (when copying meshes)
{
    /* Copy mesh surfaces */
    NewMesh->SurfaceList_->resize(SurfaceList_->size());
    
    for (u32 i = 0; i < SurfaceList_->size(); ++i)
        (*NewMesh->SurfaceList_)[i] = new video::MeshBuffer(*(*SurfaceList_)[i]);
    
    /* Copy mesh materials */
    NewMesh->LODSurfaceList_        = NewMesh->SurfaceList_;
    NewMesh->Order_                 = Order_;
    
    NewMesh->UseLODSubMeshes_       = UseLODSubMeshes_;
    NewMesh->LODSubMeshDistance_    = LODSubMeshDistance_;
    NewMesh->LODSubMeshList_        = LODSubMeshList_;
    
    NewMesh->Material_.copy(&Material_);
}


/*
 * ======= Rendering =======
 */

void Mesh::render()
{
    /* Check if the entity has any surfaces */
    if (LODSurfaceList_->empty())
        return;
    
    /* Matrix transformation */
    loadTransformation();
    
    if (__spSceneManager)
    {
        /* Frustum culling */
        if (__spSceneManager->getActiveCamera() && !BoundVolume_.checkFrustumCulling(__spSceneManager->getActiveCamera()->getViewFrustum(), spWorldMatrix))
            return;
        
        #if 1
        __spSceneManager->setActiveMesh(this); // !!! (only needed for Direct3D11 renderer)
        #endif
    }
    
    /* Update the render matrix */
    __spVideoDriver->updateModelviewMatrix();
    
    /* Update level of detail mesh */
    const u32 LODIndex = updateLevelOfDetail();
    
    /* Process the possible user matrial begin procedure */
    if (Material_.getMaterialCallback())
        Material_.getMaterialCallback()(this, true);
    
    /* Setup material states */
    if (EnableMaterial_)
        __spVideoDriver->setupMaterialStates(getMaterial());
    __spVideoDriver->setupShaderClass(this, getShaderClass());
    
    /* Draw the current mesh object */
    if (UserRenderProc_)
        UserRenderProc_(this, LODSurfaceList_, LODIndex);
    else
    {
        foreach (video::MeshBuffer* Surface, *LODSurfaceList_)
            __spVideoDriver->drawMeshBuffer(Surface);
    }
    
    /* Process the possible user matrial end procedure */
    if (Material_.getMaterialCallback())
        Material_.getMaterialCallback()(this, false);
    
    /* Unbinding the shader */
    __spVideoDriver->unbindShaders();
}


/*
 * ======= Private: =======
 */

u32 Mesh::updateLevelOfDetail()
{
    if (!UseLODSubMeshes_)
        return 0;
    
    /* Compute LOD index */
    const u32 LODIndex = static_cast<u32>(DepthDistance_ / LODSubMeshDistance_);
    s32 SubMeshesIndex = static_cast<s32>(LODIndex) - 1;
    
    /* Clamp LOD index */
    if (SubMeshesIndex >= static_cast<s32>(LODSubMeshList_.size()))
        SubMeshesIndex = static_cast<s32>(LODSubMeshList_.size()) - 1;
    
    /* Set the current LOD surface list */
    if (SubMeshesIndex >= 0)
        LODSurfaceList_ = LODSubMeshList_[SubMeshesIndex]->SurfaceList_;
    else
        LODSurfaceList_ = SurfaceList_;
    
    return LODIndex;
}


} // /namespace scene

} // /namespace sp



// ================================================================================
