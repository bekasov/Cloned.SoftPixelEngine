/*
 * Stencil manager file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Base/spStencilManager.hpp"
#include "Platform/spSoftPixelDeviceOS.hpp"


namespace sp
{

extern video::RenderSystem* __spVideoDriver;

namespace scene
{


StencilManager::StencilManager() : MultiShadows_(false), SingleShadowColor_(0, 0, 0, 128)
{
}
StencilManager::~StencilManager()
{
}

CastCloudObject* StencilManager::addCastCloudMesh(Mesh* Model)
{
    CastCloudObject* NewObject = new CastCloudObject(Model);
    {
        updateConnectivity(NewObject);
        updateCalculationPlanes(NewObject);
    }
    ObjectList_.push_back(NewObject);
    
    return NewObject;
}
void StencilManager::removeCastCloudMesh(Mesh* Model)
{
    for (std::vector<CastCloudObject*>::iterator it = ObjectList_.begin(); it != ObjectList_.end(); ++it)
    {
        if ((*it)->Object_ == Model)
        {
            ObjectList_.erase(it);
            break;
        }
    }
}

void StencilManager::addLightSource(SShadowLightSource* LightSource)
{
    for (std::vector<CastCloudObject*>::iterator it = ObjectList_.begin(); it != ObjectList_.end(); ++it)
        (*it)->addLightSource(LightSource);
}
void StencilManager::removeLightSource(SShadowLightSource* LightSource)
{
    for (std::vector<CastCloudObject*>::iterator it = ObjectList_.begin(); it != ObjectList_.end(); ++it)
        (*it)->removeLightSource(LightSource);
}

void StencilManager::updateStencilShadow(Mesh* Model)
{
    for (std::vector<CastCloudObject*>::iterator it = ObjectList_.begin(); it != ObjectList_.end(); ++it)
    {
        if ((*it)->Object_ == Model)
        {
            updateConnectivity(*it);
            updateCalculationPlanes(*it);
            break;
        }
    }
}

void StencilManager::renderStencilShadows(Camera* hCamera)
{
    
    /* Temporary variables */
    dim::vector3df LightPos, ObjPos;
    
    dim::matrix4f TempMatrix, CameraMatrix( hCamera->getTransformation(true).getInverse() );
    
    /* Loop for each cast-cloud object */
    for (std::vector<CastCloudObject*>::iterator it = ObjectList_.begin(); it != ObjectList_.end(); ++it)
    {
        
        /* Loop for each light source */
        for (std::vector<SShadowLightSource*>::iterator itl = (*it)->LightSourcesList_.begin();
             itl != (*it)->LightSourcesList_.end(); ++itl)
        {
            if (!(*itl)->Visible)
                continue;
            
            LightPos = (*itl)->Object->getPosition(true);
            
            /* Get the global location */
            TempMatrix = (*it)->Object_->getTransformation(true);
            ObjPos = TempMatrix.getPosition();
            
            /* Terminate the position */
            TempMatrix.setPosition(0);
            
            /* Inverse rotation */
            TempMatrix.setInverse();
            spWorldMatrix = TempMatrix;
            
            /* Translate the light position */
            LightPos = spWorldMatrix * LightPos;
            spWorldMatrix.translate(-ObjPos);
            LightPos += spWorldMatrix * dim::vector3df();
            
            /* Object location */
            spViewMatrix = CameraMatrix;
            spWorldMatrix.reset();
            (*it)->Object_->updateTransformation();
            
            /* Renderer matrix */
            __spVideoDriver->updateModelviewMatrix();
            
            /* Rendering shadow */
            
            updateShadowVolume(*it, LightPos, (*itl)->ShadowLength);
            
            __spVideoDriver->drawStencilShadowVolume(
                (*it)->pShadowVertices_, (*it)->CountOfShadowVertices_, !true, (*itl)->Volumetric
            );
            
            if (MultiShadows_)
            {
                if ((*it)->ShadowIntensityCallback_)
                    (*it)->ShadowIntensityCallback_((*it)->ShadowIntensity_, *it, *itl);
                
                __spVideoDriver->drawStencilShadow(
                    (*itl)->ShadowColor.getIntensity((*it)->ShadowIntensity_)
                );
            }
            
        }
        
    }
    
    if (!MultiShadows_)
        __spVideoDriver->drawStencilShadow(SingleShadowColor_);
    
}

/*void StencilManager::addMirrorPlane(Mesh* Model, u8 Type) { }
void StencilManager::removeMirrorPlane(Mesh* Model) { }

void StencilManager::addMirrorObject(Mesh* Model) { }
void StencilManager::removeMirrorObject(Mesh* Model) { }

void StencilManager::updateMirrorPlane(Mesh* Model) { }

void StencilManager::renderStencilMirrors(Camera* CameraObj) { }*/


/*
 * ========== Private: ==========
 */

void StencilManager::updateConnectivity(CastCloudObject* Object)
{
    
    /* Temporary variables */
    s32 p1i, p2i, p1j, p2j;
    s32 P1i, P2i, P1j, P2j;
    s32 i, j, ki, kj;
    
    /* Loop for each plane */
    for (i = 0; i < Object->CountOfPlanes_ - 1; ++i)
    {
        for (j = i+1; j < Object->CountOfPlanes_; ++j)
        {
            for (ki = 0; ki < 3; ++ki)
            {
                
                if (!Object->pPlanes_[i].Neigh[ki])
                {
                    for (kj = 0; kj < 3; ++kj)
                    {
                        p1i = ki;
                        p1j = kj;
                        p2i = (ki+1)%3;
                        p2j = (kj+1)%3;
                        
                        p1i = Object->pPlanes_[i].p[p1i];
                        p2i = Object->pPlanes_[i].p[p2i];
                        p1j = Object->pPlanes_[j].p[p1j];
                        p2j = Object->pPlanes_[j].p[p2j];
                        
                        P1i = ( (p1i+p2i) - math::Abs(p1i - p2i) )/2;
                        P2i = ( (p1i+p2i) + math::Abs(p1i - p2i) )/2;
                        P1j = ( (p1j+p2j) - math::Abs(p1j - p2j) )/2;
                        P2j = ( (p1j+p2j) + math::Abs(p1j - p2j) )/2;
                        
                        /* Check if the planes are heighbours */
                        if( ( P1i == P1j ) && ( P2i == P2j ) )
                        {
                            Object->pPlanes_[i].Neigh[ki] = j+1;
                            Object->pPlanes_[j].Neigh[kj] = i+1;
                        }
                    }
                } // fi
                
            }
        } // next plane (second pass)
    } // next plane (first pass)
    
}

void StencilManager::updateCalculationPlanes(CastCloudObject* Object)
{
    
    /* Temporary variables */
    CastCloudObject::SPlane CurPlane;
    dim::vector3df v[3];
    
    /* Loop for each plane */
    for (s32 i = 0; i < Object->CountOfPlanes_; ++i)
    {
        CurPlane = Object->pPlanes_[i];
        
        v[0] = Object->pMeshVertices_[ CurPlane.p[0] ];
        v[1] = Object->pMeshVertices_[ CurPlane.p[1] ];
        v[2] = Object->pMeshVertices_[ CurPlane.p[2] ];
        
        CurPlane.PlaneEq.a = v[0].Y*(v[1].Z - v[2].Z)   +   v[1].Y*(v[2].Z - v[0].Z)   +   v[2].Y*(v[0].Z - v[1].Z);
        CurPlane.PlaneEq.b = v[0].Z*(v[1].X - v[2].X)   +   v[1].Z*(v[2].X - v[0].X)   +   v[2].Z*(v[0].X - v[1].X);
        CurPlane.PlaneEq.c = v[0].X*(v[1].Y - v[2].Y)   +   v[1].X*(v[2].Y - v[0].Y)   +   v[2].X*(v[0].Y - v[1].Y);
        CurPlane.PlaneEq.d = -( v[0].X*(v[1].Y*v[2].Z - v[2].Y*v[1].Z) +
                                v[1].X*(v[2].Y*v[0].Z - v[0].Y*v[2].Z) +
                                v[2].X*(v[0].Y*v[1].Z - v[1].Y*v[0].Z) );
        
        Object->pPlanes_[i] = CurPlane;
    }
    
}

void StencilManager::updateShadowVolume(CastCloudObject* Object, const dim::vector3df LightPos, f32 ShadowLength)
{
    
    /* Temporary variables */
    register s32 i, j, k, jj, p1, p2, c = 0;
    
    dim::vector3df vec1, vec2;
    
    Object->CountOfShadowVertices_ = 0;
    
    /* Loop for each plane (to calculate the visibility) */
    for (i = 0; i < Object->CountOfPlanes_; ++i)
    {
        
        Object->pPlanes_[i].isVisible = (
            Object->pPlanes_[i].PlaneEq.a*LightPos.X +
            Object->pPlanes_[i].PlaneEq.b*LightPos.Y +
            Object->pPlanes_[i].PlaneEq.c*LightPos.Z +
            Object->pPlanes_[i].PlaneEq.d < 0
        );
        
        if (Object->pPlanes_[i].isVisible)
        {
            for (j = 0; j < 3; ++j)
            {
                k = Object->pPlanes_[i].Neigh[j];
                if (!k || !Object->pPlanes_[k - 1].isVisible)
                    ++Object->CountOfShadowVertices_;
            }
        }
    }
    
    Object->CountOfShadowVertices_ *= 6;
    
    MemoryManager::deleteBuffer(Object->pShadowVertices_);
    Object->pShadowVertices_ = new dim::vector3df[ Object->CountOfShadowVertices_ ];
    
    /* Loop for each plane */
    for (i = 0; i < Object->CountOfPlanes_; ++i)
    {
        
        if (Object->pPlanes_[i].isVisible)
        {
            for (j = 0; j < 3; ++j)
            {
                
                k = Object->pPlanes_[i].Neigh[j];
                
                if (!k || !Object->pPlanes_[k - 1].isVisible)
                {
                    
                    /* Here we have an edge, we must draw a polygon */
                    p1 = Object->pPlanes_[i].p[j];
                    jj = (j+1)%3;
                    p2 = Object->pPlanes_[i].p[jj];
                    
                    /* Compute the length of the vector */
                    vec1.X = (Object->pMeshVertices_[p1].X - LightPos.X)*ShadowLength;
                    vec1.Y = (Object->pMeshVertices_[p1].Y - LightPos.Y)*ShadowLength;
                    vec1.Z = (Object->pMeshVertices_[p1].Z - LightPos.Z)*ShadowLength;
                    
                    vec2.X = (Object->pMeshVertices_[p2].X - LightPos.X)*ShadowLength;
                    vec2.Y = (Object->pMeshVertices_[p2].Y - LightPos.Y)*ShadowLength;
                    vec2.Z = (Object->pMeshVertices_[p2].Z - LightPos.Z)*ShadowLength;
                    
                    /* Save the vertices */
                    Object->pShadowVertices_[c++] = Object->pMeshVertices_[p1];
                    Object->pShadowVertices_[c++] = Object->pMeshVertices_[p2];
                    Object->pShadowVertices_[c++] = Object->pMeshVertices_[p1] + vec1;
                    
                    Object->pShadowVertices_[c++] = Object->pMeshVertices_[p1] + vec1;
                    Object->pShadowVertices_[c++] = Object->pMeshVertices_[p2];
                    Object->pShadowVertices_[c++] = Object->pMeshVertices_[p2] + vec2;
                    
                } // fi
                
            } // next
        } // fi
        
    } // next
    
}


/*
 * CastCloudObject class
 */

CastCloudObject::CastCloudObject(Mesh* Object)
    : Object_(Object),
    /*pPlanes_(0), */CountOfPlanes_(0),
    /*pMeshVertices_(0), */CountOfMeshVertices_(0),
    pShadowVertices_(0), CountOfShadowVertices_(0),
    ShadowIntensityCallback_(0)
{
    CountOfMeshVertices_    = Object_->getVertexCount();
    CountOfPlanes_          = Object_->getTriangleCount();
    
    //pMeshVertices_          = new dim::vector3df[CountOfMeshVertices_];
    //pPlanes_                = new SPlane[CountOfPlanes_];
    
    u32* pTriangles         = new u32[CountOfPlanes_*3];
    
    // Initialize vertex coordinates and triangle indices
    for (u32 s = 0, i; s < Object_->getMeshBufferCount(); ++s)
    {
        video::MeshBuffer* Surface = Object_->getMeshBuffer(s);
        
        for (i = 0; i < Surface->getVertexCount(); ++i)
            pMeshVertices_[i] = Surface->getVertexCoord(i);
        
        for (i = 0; i < Surface->getIndexCount(); ++i)
            pTriangles[i] = Surface->getPrimitiveIndex(i);
    }
    
    // Initialize planes
    for (s32 i = 0, c; i < CountOfPlanes_; ++i)
    {
        for (c = 0; c < 3; ++c)
            pPlanes_[i].p[c] = pTriangles[i*3+c];
    }
    
    delete [] pTriangles;
    
    ShadowIntensity_ = 1.0f;
}
CastCloudObject::~CastCloudObject()
{
    //MemoryManager::deleteBuffer(pPlanes_);
    //MemoryManager::deleteBuffer(pMeshVertices_);
    MemoryManager::deleteBuffer(pShadowVertices_);
}

void CastCloudObject::addLightSource(SShadowLightSource* LightSource)
{
    LightSourcesList_.push_back(LightSource);
}
void CastCloudObject::removeLightSource(SShadowLightSource* LightSource)
{
    if (LightSource)
        MemoryManager::removeElement(LightSourcesList_, LightSource);
    else
        LightSourcesList_.clear();
}


} // /namespace scene

} // /namespace sp



// ================================================================================
