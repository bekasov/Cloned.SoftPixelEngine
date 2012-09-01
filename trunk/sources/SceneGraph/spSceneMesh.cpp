/*
 * Mesh scene node file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "SceneGraph/spSceneGraph.hpp"
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

bool cmpSurfaceTexture(video::MeshBuffer* &obj1, video::MeshBuffer* &obj2)
{
    if (obj1->getTextureCount() != obj2->getTextureCount())
        return obj1->getTextureCount() < obj2->getTextureCount();
    
    std::vector<video::SMeshSurfaceTexture>::const_iterator ita, itb;
    
    ita = obj1->getSurfaceTextureList().begin();
    itb = obj2->getSurfaceTextureList().begin();
    
    for (; ita != obj1->getSurfaceTextureList().end() && itb != obj2->getSurfaceTextureList().end(); ++ita, ++itb)
    {
        if (*ita == *itb)
            continue;
        return (long)ita->TextureObject < (long)itb->TextureObject;
    }
    
    return (long)obj1 < (long)obj2;
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
    #if 1 //!deprecated!
    OctTreeRoot_        (0                  ),
    PickRef_            (0                  ),
    CollRef_            (0                  ),
    #endif
    UserRenderProc_     (0                  )
{
}
Mesh::~Mesh()
{
    MemoryManager::deleteList(OrigSurfaceList_);
    
    #if 1 //!deprecated!
    MemoryManager::deleteMemory(OctTreeRoot_);
    #endif
}


/*
 * ======= Textureing =======
 */

void Mesh::addTexture(video::Texture* Tex, const u8 Layer)
{
    foreach (video::MeshBuffer* Surface, OrigSurfaceList_)
        Surface->addTexture(Tex, Layer);
}

void Mesh::textureAutoMap(const u8 Layer, const f32 Density, const u32 MeshBufferIndex, bool isGlobal)
{
    if (Layer >= MAX_COUNT_OF_TEXTURES)
        return;
    
    /* Initialization settings */
    u32 Indices[3];
    dim::vector3df AbsNormal, Pos;
    dim::point2df TexCoord;
    dim::triangle3df Face;
    s32 AxisType;
    
    std::vector<video::MeshBuffer*>::iterator it = OrigSurfaceList_.begin(), itEnd = OrigSurfaceList_.end();
    
    /* Get transformation matrices (complete transformation and rotation ) */
    dim::matrix4f Transformation, Rotation;
    
    if (isGlobal)
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
            return;
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
            AbsNormal = (Rotation * Face.getNormal()).getAbs();
            
            if (AbsNormal.X >= AbsNormal.Y && AbsNormal.X >= AbsNormal.Z)
                AxisType = 0;
            else if (AbsNormal.Y >= AbsNormal.X && AbsNormal.Y >= AbsNormal.Z)
                AxisType = 1;
            else
                AxisType = 2;
            
            /* Set new texture coordinates */
            for (j = 0; j < 3; ++j)
            {
                Pos = Transformation * Face[j];
                
                switch (AxisType)
                {
                    case 0: TexCoord = dim::point2df(Pos.Z, -Pos.Y); break; // +x
                    case 1: TexCoord = dim::point2df(Pos.X, -Pos.Z); break; // +y
                    case 2: TexCoord = dim::point2df(Pos.X, -Pos.Y); break; // +z
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
    const f32 Max = 999999.0f;
    
    dim::vector3df MinPos(Max, Max, Max);
    dim::vector3df MaxPos(-Max, -Max, -Max);
    dim::vector3df Pos;
    
    /* Find the dimensions */
    foreach (video::MeshBuffer* Surface, OrigSurfaceList_)
    {
        for (u32 i = 0; i < Surface->getVertexCount(); ++i)
        {
            Pos = Surface->getVertexCoord(i);
            
            if (Pos.X < MinPos.X) MinPos.X = Pos.X;
            if (Pos.Y < MinPos.Y) MinPos.Y = Pos.Y;
            if (Pos.Z < MinPos.Z) MinPos.Z = Pos.Z;
            
            if (Pos.X > MaxPos.X) MaxPos.X = Pos.X;
            if (Pos.Y > MaxPos.Y) MaxPos.Y = Pos.Y;
            if (Pos.Z > MaxPos.Z) MaxPos.Z = Pos.Z;
        }
    }
    
    /* Fit the mesh */
    foreach (video::MeshBuffer* Surface, OrigSurfaceList_)
    {
        for (u32 i = 0; i < Surface->getVertexCount(); ++i)
        {
            Pos = Surface->getVertexCoord(i);
            
            Pos -= MinPos;
            Pos /= (MaxPos - MinPos);
            Pos *= Size;
            Pos += Position;
            
            Surface->setVertexCoord(i, Pos);
        }
        Surface->updateNormals(Material_.getShading());
    }
}

void Mesh::meshSpherify(f32 Factor)
{
    math::Clamp(Factor, -1.0f, 1.0f);
    
    dim::vector3df OrigPos, NormPos;
    
    foreach (video::MeshBuffer* Surface, OrigSurfaceList_)
    {
        for (u32 i = 0; i < Surface->getVertexCount(); ++i)
        {
            OrigPos = Surface->getVertexCoord(i);
            NormPos = OrigPos;
            NormPos.normalize();
            
            Surface->setVertexCoord(i, OrigPos * (1.0f - Factor) + NormPos * Factor);
        }
        Surface->updateNormals(Material_.getShading());
    }
}

void Mesh::meshTwist(f32 Rotation)
{
    dim::matrix4f Mat;
    dim::vector3df Pos;
    f32 Factor;
    
    const dim::aabbox3df BoundBox(getMeshBoundingBox());
    
    const f32 MinHeight = BoundBox.Min.Y;
    const f32 MaxHeight = BoundBox.Max.Y - MinHeight;
    
    foreach (video::MeshBuffer* Surface, OrigSurfaceList_)
    {
        for (u32 i = 0; i < Surface->getVertexCount(); ++i)
        {
            Pos     = Surface->getVertexCoord(i);
            Factor  = (Pos.Y - MinHeight) / MaxHeight;
            
            Mat.reset();
            Mat.rotateY(Rotation * Factor);
            
            Surface->setVertexCoord(i, Mat * Pos);
        }
        Surface->updateNormals(Material_.getShading());
    }
}

void Mesh::mergeFamily(bool isDeleteChildren)
{
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

void Mesh::centerOrigin()
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
}

void Mesh::deleteMeshBuffers()
{
    foreach (video::MeshBuffer* Surface, OrigSurfaceList_)
        MemoryManager::deleteMemory(Surface);
    OrigSurfaceList_.clear();
}

void Mesh::optimizeMeshBuffers() // !!! update for surfaces without a texture !!!
{
    /* Copy and the surface list */
    std::list<video::MeshBuffer*> SurfaceList;
    
    foreach (video::MeshBuffer* Surface, OrigSurfaceList_)
        SurfaceList.push_back(Surface);
    
    SurfaceList.sort(cmpSurfaceTexture);
    
    /* Delete all old surfaces' mesh buffers but don't delete the surface memory until the end of optimization */
    foreach (video::MeshBuffer* Surface, OrigSurfaceList_)
        Surface->deleteMeshBuffer();
    OrigSurfaceList_.clear();
    
    /* Loop for each surface of the sorted list */
    video::MeshBuffer* CurSurf = 0;
    const std::vector<video::SMeshSurfaceTexture>* LastTextureList = 0;
    u32 BoostCounter = 0;
    
    for (std::list<video::MeshBuffer*>::iterator itSurf = SurfaceList.begin(); itSurf != SurfaceList.end(); ++itSurf)
    {
        /* Check if the surface's texture list has changed */
        if (LastTextureList)
        {
            bool Equal = (LastTextureList->size() == (*itSurf)->getTextureCount());
            
            if (Equal)
            {
                for (u32 i = 0; i < LastTextureList->size(); ++i)
                {
                    if ((*LastTextureList)[i] != (*itSurf)->getSurfaceTextureList()[i])
                    {
                        Equal = false;
                        break;
                    }
                }
            }
            
            if (!Equal)
            {
                /* Create the current surface */
                CurSurf->setSurfaceTextureList(*LastTextureList);
                CurSurf->createMeshBuffer();
                OrigSurfaceList_.push_back(CurSurf);
                
                /* Resetings */
                BoostCounter    = 0;
                CurSurf         = 0;
            }
        }
        
        /* Create a new surface */
        if (!CurSurf)
            CurSurf = MemoryManager::createMemory<video::MeshBuffer>();
        
        /* Store the last texture list */
        LastTextureList = &(*itSurf)->getSurfaceTextureList();
        
        /* Add the surface name */
        if ((*itSurf)->getName() != "")
        {
            if (CurSurf->getName() != "")
                CurSurf->setName(CurSurf->getName() + ";");
            CurSurf->setName(CurSurf->getName() + (*itSurf)->getName());
        }
        
        /* Add the vertices */
        for (u32 i = 0; i < (*itSurf)->getVertexCount(); ++i)
            CurSurf->addVertex((*itSurf)->getVertex(i));
        
        /* Add the triangles */
        for (u32 i = 0; i < (*itSurf)->getTriangleCount(); ++i)
        {
            u32 Indices[3];
            (*itSurf)->getTriangleIndices(i, Indices);
            CurSurf->addTriangle(Indices);
        }
        
        CurSurf->setIndexOffset(CurSurf->getVertexCount());
    }
    
    /* Add the last surface */
    if (LastTextureList && !CurSurf->getVertexCount() && !CurSurf->getTriangleCount())
    {
        CurSurf->setSurfaceTextureList(*LastTextureList);
        CurSurf->createMeshBuffer();
        OrigSurfaceList_.push_back(CurSurf);
    }
    
    /* Delete the surface memory finally */
    foreach (video::MeshBuffer* Surface, SurfaceList)
        MemoryManager::deleteMemory(Surface);
    
    updateMeshBuffer();
}

// !!!TESTING unfinished!!!
void Mesh::optimizeTransparency()
{
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

/*dim::ptriangle3df Mesh::getTriangleLink(u32 TriangleIndex)
{
    if (CurSurface_ && TriangleIndex < CurSurface_->TriangleList.size())
    {
        SMeshTriangle3D Triangle = CurSurface_->TriangleList[TriangleIndex];
        
        dim::ptriangle3df TriangleLink;
        
        TriangleLink.PointA = &CurSurface_->VerticesList[Triangle.a].getPosition();
        TriangleLink.PointB = &CurSurface_->VerticesList[Triangle.b].getPosition();
        TriangleLink.PointC = &CurSurface_->VerticesList[Triangle.c].getPosition();
        
        return TriangleLink;
    }
    return dim::ptriangle3df();
}*/

bool Mesh::getMeshBoundingBox(dim::vector3df &Min, dim::vector3df &Max, bool isGlobal) const
{
    if (!getVertexCount())
        return false;
    
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
        return 0.0f;
    
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

/*void Mesh::saveVertexBasicData()
{
    for (std::vector<video::MeshBuffer*>::iterator it = SurfaceList_->begin(); it != SurfaceList_->end(); ++it)
        (*it)->saveVertexBasicData();
}
void Mesh::loadVertexBasicData()
{
    for (std::vector<video::MeshBuffer*>::iterator it = SurfaceList_->begin(); it != SurfaceList_->end(); ++it)
    {
        (*it)->loadVertexBasicData();
        (*it)->updateVertexBuffer();
    }
}
void Mesh::clearVertexBasicData()
{
    for (std::vector<video::MeshBuffer*>::iterator it = SurfaceList_->begin(); it != SurfaceList_->end(); ++it)
        (*it)->clearVertexBasicData();
}*/


/*
 * ======= Oct-tree optimization =======
 */

void Mesh::createOctTree(s8 ForksCount)
{
    /* Delete the old oct-tree and create a new one */
    MemoryManager::deleteMemory(OctTreeRoot_);
    OctTreeRoot_ = MemoryManager::createMemory<OcTreeNode>();
    OctTreeRoot_->createTree(this, ForksCount);
}
void Mesh::deleteOctTree()
{
    MemoryManager::deleteMemory(OctTreeRoot_);
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

void Mesh::copy(const Mesh* other)
{
    if (other)
    {
        other->copyRoot(this);
        other->copyMesh(this);
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
    
    /* Copy the oct-tree */
    //if (OctTreeRoot_)
    //    NewMesh->OctTreeRoot_ = OctTreeRoot_->copy(NewMesh);
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
