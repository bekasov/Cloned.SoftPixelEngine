/*
 * Lightmap generator file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Framework/Tools/LightmapGenerator/spLightmapGenerator.hpp"

#ifdef SP_COMPILE_WITH_LIGHTMAPGENERATOR


#include "Framework/Tools/LightmapGenerator/spLightmapGeneratorStructs.hpp"
#include "Base/spMathCollisionLibrary.hpp"
#include "Base/spMathRasterizer.hpp"
#include "Base/spTimer.hpp"
#include "Base/spSharedObjects.hpp"
#include "Platform/spSoftPixelDeviceOS.hpp"
#include "SceneGraph/spSceneManager.hpp"

#include <boost/foreach.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>


namespace sp
{

extern video::RenderSystem* GlbRenderSys;
extern scene::SceneGraph* GlbSceneGraph;

namespace tool
{


using namespace LightmapGen;

/*
 * Static class members
 */

LightmapProgressCallback LightmapGenerator::ProgressCallback_ = 0;

s32 LightmapGenerator::Progress_                    = 0;
s32 LightmapGenerator::ProgressMax_                 = 0;
s32 LightmapGenerator::ProgressShadedTriangleNum_   = 0;

dim::size2di LightmapGenerator::LightmapSize_ = dim::size2di(512);


/*
 * LightmapGenerator class
 */

LightmapGenerator::LightmapGenerator() :
    FinalModel_ (0),
    CollMesh_   (0),
    CurLightmap_(0),
    CurRectRoot_(0)
{
}
LightmapGenerator::~LightmapGenerator()
{
    clearLightmapObjects();
}

bool LightmapGenerator::generateLightmaps(
    const std::list<SCastShadowObject> &CastShadowObjects,
    const std::list<SGetShadowObject> &GetShadowObjects,
    const std::list<SLightmapLight> &LightSources,
    const video::color &AmbientColor,
    const u32 MaxLightmapSize,
    const f32 DefaultDensity,
    const u8 TexelBlurRadius,
    const u8 ThreadCount,
    const s32 Flags)
{
    try
    {
        updateStateInfo(LIGHTMAPSTATE_INITIALIZING);
        
        // Initialize settings
        CastShadowObjects_  = CastShadowObjects;
        LightmapSize_       = MaxLightmapSize;
        
        State_.Flags                    = Flags;
        State_.AmbientColor             = AmbientColor;
        State_.TexelBlurRadius          = TexelBlurRadius;
        State_.ThreadCount              = ThreadCount;
        State_.HasGeneratedSuccessful   = false;
        
        State_.validateFlags();
        
        // Delete the old lightmap objects & textures
        clearScene();
        
        // Create initial lightmap
        createNewLightmap();
        
        // Create the get-shadow objects, cast-shadow objects, light sources
        foreach (const SGetShadowObject &Obj, GetShadowObjects)
        {
            if (Obj.Mesh->getVisible())
            {
                SModel* NewModel = new SModel(Obj.Mesh, Obj.StayAlone, Obj.TrianglesDensity);
                ModelMap_[Obj.Mesh] = NewModel;
                GetShadowObjects_.push_back(NewModel);
            }
        }
        
        // Create cast-shadow collision mesh
        std::list<scene::Mesh*> CollMeshList;
        
        foreach (const SCastShadowObject &Obj, CastShadowObjects_)
        {
            if (Obj.Mesh->getVisible())
                CollMeshList.push_back(Obj.Mesh);
        }
        
        //#define _DEB_LM_TIMER_
        #ifdef _DEB_LM_TIMER_//!!!
        u64 t = io::Timer::millisecs();
        #endif
        
        CollMesh_ = CollSys_.createMeshList(0, CollMeshList, 20);
        
        #ifdef _DEB_LM_TIMER_//!!!
        io::Log::message("kd-Tree generation Time: " + io::stringc(io::Timer::millisecs() - t) + " ms.");
        #endif
        
        foreach (const SLightmapLight &Light, LightSources)
        {
            if (Light.Visible)
                LightSources_.push_back(new SLight(Light));
        }
        
        // Create the root object & partition the add-shadow objects
        estimateEntireProgress(TexelBlurRadius > 0);
        
        FinalModel_ = GlbSceneGraph->createMesh();
        FinalModel_->getMaterial()->setLighting(false);
        
        partitionScene(DefaultDensity);
        
        if (!LightmapGenerator::processRunning(0))
            throw std::exception();
        
        shadeAllLightmaps();
        
        if (!LightmapGenerator::processRunning(0))
            throw std::exception();
        
        // Copy image buffers
        foreach (SLightmap* LMap, Lightmaps_)
            LMap->copyImageBuffers();
        
        // Blur lightmaps
        if (TexelBlurRadius > 0)
            blurAllLightmaps(TexelBlurRadius);
        
        // Create the final lightmap textures
        createFinalLightmapTextures(AmbientColor);
        
        // Build the final models
        buildAllFinalModels();
        
        // Store final lightmap textures
        foreach (SLightmap* LMap, Lightmaps_)
            LightmapTextures_.push_back(LMap->Texture);
        
        updateStateInfo(LIGHTMAPSTATE_COMPLETED);
        
        CurLightmap_ = 0;
        State_.HasGeneratedSuccessful = true;
    }
    catch (...)
    {
        io::Log::warning("Lightmap generation has been canceled");
        return false;
    }
    return true;
}

void LightmapGenerator::clearScene()
{
    // Delete all lightmap scene nodes
    GlbSceneGraph->deleteNode(FinalModel_);
    FinalModel_ = 0;
    
    foreach (scene::Mesh* Obj, SingleModels_)
        GlbSceneGraph->deleteNode(Obj);
    SingleModels_.clear();
    
    // Delete all lightmap textures
    foreach (video::Texture* Tex, LightmapTextures_)
        GlbRenderSys->deleteTexture(Tex);
    LightmapTextures_.clear();
    
    // Delete all collision nodes
    CollSys_.clearScene();
    
    // Delete old lightmaps
    clearLightmapObjects();
}

bool LightmapGenerator::updateBluring(u8 TexelBlurRadius)
{
    if (!hasGeneratedSuccessful() || TexelBlurRadius == State_.TexelBlurRadius)
        return false;
    
    #ifdef SP_DEBUGMODE
    if (TexelBlurRadius > 16)
    {
        io::Log::debug(
            "LightmapGenerator::updateBluring",
            "Very high blur radius (" + io::stringc(static_cast<u32>(TexelBlurRadius)) + ")"
        );
    }
    #endif
    
    // Update texture bluring
    blurAllLightmaps(TexelBlurRadius);
    createFinalLightmapTextures(State_.AmbientColor);
    
    State_.TexelBlurRadius = TexelBlurRadius;
    
    return true;
}

bool LightmapGenerator::updateAmbientColor(const video::color &AmbientColor)
{
    if (!hasGeneratedSuccessful() || AmbientColor == State_.AmbientColor)
        return false;
    
    foreach (SLightmap* LMap, Lightmaps_)
        LMap->createTexture(AmbientColor);
    
    State_.AmbientColor = AmbientColor;
    
    return true;
}

void LightmapGenerator::setProgressCallback(const LightmapProgressCallback &Callback)
{
    ProgressCallback_ = Callback;
}


/*
 * ======= Private: =======
 */

void LightmapGenerator::estimateEntireProgress(bool BlurEnabled)
{
    // Calculate the progress maximum
    LightmapGenerator::Progress_    = 0;
    LightmapGenerator::ProgressMax_ = GetShadowObjects_.size() * 8;
    
    LightmapGenerator::ProgressShadedTriangleNum_ = 0;
    
    foreach (SModel* Obj, GetShadowObjects_)
        LightmapGenerator::ProgressShadedTriangleNum_ += Obj->Mesh->getTriangleCount();
    
    LightmapGenerator::ProgressMax_ += LightmapGenerator::ProgressShadedTriangleNum_ * (LightSources_.size() + 1);
    
    if (BlurEnabled)
        LightmapGenerator::ProgressMax_ += GetShadowObjects_.size();
}

void LightmapGenerator::createFacesLightmaps(SModel* Model)
{
    for (s32 i = 0; i < 6; ++i)
    {
        foreach (SFace &Face, Model->Axles[i].Faces)
        {
            Face.Lightmap = new SLightmap(Face.Size + 2, false);
            putFaceIntoLightmap(&Face);
        }
    }
}

void LightmapGenerator::generateLightTexelsSingleThreaded(SLight* Light)
{
    // kd-Tree relevant variables
    std::list<const scene::TreeNode*> TreeNodeList;
    scene::CollisionMesh::TreeNodeDataType* TreeNodeData = 0;
    
    std::map<STriangle*, bool> UsedTriangles;
    SModel* Obj = 0;
    
    // Find each triangle using the kd-Tree
    CollMesh_->getRootTreeNode()->findLeafList(
        TreeNodeList, Light->Position, Light->FixedVolumetricRadius
    );
    
    foreach (const scene::TreeNode* Node, TreeNodeList)
    {
        if (!Node->getUserData())
            continue;
        
        TreeNodeData = static_cast<scene::CollisionMesh::TreeNodeDataType*>(Node->getUserData());
        
        // Loop each tree-node's triangle
        #ifndef _DEB_NEW_KDTREE_
        foreach (scene::SCollisionFace* Face, *TreeNodeData)
        #else
        foreach (scene::SCollisionFace &NodeFace, *TreeNodeData)
        #endif
        {
            #ifdef _DEB_NEW_KDTREE_
            scene::SCollisionFace* Face = &NodeFace;
            #endif
            
            // Get model object
            std::map<scene::Mesh*, SModel*>::iterator it = ModelMap_.find(Face->Mesh);
            
            if (it == ModelMap_.end())
                continue;
            
            Obj = it->second;
            
            // Get triangle object
            STriangle* Triangle = (Obj->Triangles[Face->Surface])[Face->Index];
            
            if (UsedTriangles.find(Triangle) != UsedTriangles.end())
            {
                if (!LightmapGenerator::processRunning(0))
                    throw std::exception();
                continue;
            }
            UsedTriangles[Triangle] = true;
            
            if (!LightmapGenerator::processRunning())
                throw std::exception();
            
            // Rasterize triangle
            rasterizeTriangle(Light, *Triangle);
        }
    }
    
    /*
    
    The following is legacy code. It was used to iterate over all scene triangles
    without making use of tree hierarchy optimizations. When the optimization part
    produce problems, use this code:
    ==============================================================
    
    foreach (SModel* Obj, GetShadowObjects_)
    {
        for (s32 i = 0; i < 6; ++i)
        {
            foreach (SFace &Face, Obj->Axles[i].Faces)
            {
                foreach (STriangle &Triangle, Face.Triangles)
                {
                    rasterizeTriangle(Light, Triangle);
                    if (!LightmapGenerator::processRunning())
                        throw std::exception();
                }
            }
        }
    }
    
    */
}

struct SRasterizerThreadData
{
    std::list<STriangle*>::iterator itStart, itEnd;
    s32* NumRunningThreads;
    CriticalSection* Mutex;
    SLight* Light;
    LightmapGenerator* LMGen;
};

THREAD_PROC(RasterizerThreadProc)
{
    SRasterizerThreadData* ThreadData = reinterpret_cast<SRasterizerThreadData*>(Arguments);
    
    SLight* Light               = ThreadData->Light;
    LightmapGenerator* LMGen    = ThreadData->LMGen;
    
    // Rasterize each triangle given to this thread
    for (std::list<STriangle*>::iterator it = ThreadData->itStart, itEnd = ThreadData->itEnd; it != itEnd; ++it)
        LMGen->rasterizeTriangle(Light, **it);
    
    // Decrement running thread counter
    ThreadData->Mutex->lock();
    --(*ThreadData->NumRunningThreads);
    ThreadData->Mutex->unlock();
    
    delete ThreadData;
    
    return 0;
}

void LightmapGenerator::generateLightTexelsMultiThreaded(SLight* Light)
{
    // Find surrounding faces
    std::list<const scene::TreeNode*> TreeNodeList;
    scene::CollisionMesh::TreeNodeDataType* TreeNodeData = 0;
    
    std::map<STriangle*, bool> UsedTriangles;
    SModel* Obj = 0;
    
    std::list<STriangle*> SurroundingTriangleList;
    
    // Find each triangle using the kd-Tree
    CollMesh_->getRootTreeNode()->findLeafList(
        TreeNodeList, Light->Position, Light->FixedVolumetricRadius
    );
    
    foreach (const scene::TreeNode* Node, TreeNodeList)
    {
        if (!Node->getUserData())
            continue;
        
        TreeNodeData = static_cast<scene::CollisionMesh::TreeNodeDataType*>(Node->getUserData());
        
        #ifndef _DEB_NEW_KDTREE_
        foreach (scene::SCollisionFace* Face, *TreeNodeData)
        #else
        foreach (scene::SCollisionFace &NodeFace, *TreeNodeData)
        #endif
        {
            #ifdef _DEB_NEW_KDTREE_
            scene::SCollisionFace* Face = &NodeFace;
            #endif
            
            // Get model object
            std::map<scene::Mesh*, SModel*>::iterator it = ModelMap_.find(Face->Mesh);
            
            if (it == ModelMap_.end())
                continue;
            
            Obj = it->second;
            
            // Get triangle object
            STriangle* Triangle = (Obj->Triangles[Face->Surface])[Face->Index];
            
            if (UsedTriangles.find(Triangle) == UsedTriangles.end())
            {
                SurroundingTriangleList.push_back(Triangle);
                UsedTriangles[Triangle] = true;
            }
        }
    }
    
    // Distribute faces to thread lists
    const u32 MaxBlockSize = math::Max(size_t(1), SurroundingTriangleList.size() / static_cast<size_t>(State_.ThreadCount));
    
    std::list<STriangle*>::iterator it      = SurroundingTriangleList.begin();
    std::list<STriangle*>::iterator itPrev  = it;
    
    u32 BlockSize = 0;
    s32 NumRunningThreads = 0;
    u32 NumStartedThreads = 0;
    
    CriticalSection Mutex;
    
    typedef boost::shared_ptr<ThreadManager> ThreadManagerPtr;
    std::vector<ThreadManagerPtr> Threads;
    
    while (1)
    {
        bool EndOfList          = (it == SurroundingTriangleList.end());
        bool ThreadsAlmostFull  = (NumStartedThreads + 1 >= State_.ThreadCount);
        
        ++BlockSize;
        
        if ( ( !ThreadsAlmostFull && BlockSize > MaxBlockSize ) || EndOfList )
        {
            // Setup thread data
            SRasterizerThreadData* ThreadData = new SRasterizerThreadData;
            
            ThreadData->itStart             = itPrev;
            ThreadData->itEnd               = it;
            ThreadData->NumRunningThreads   = (&NumRunningThreads);
            ThreadData->Mutex               = (&Mutex);
            ThreadData->Light               = Light;
            ThreadData->LMGen               = this;
            
            // Increment running thread counter
            Mutex.lock();
            ++NumRunningThreads;
            Mutex.unlock();
            
            ++NumStartedThreads;
            
            // Start new thread
            Threads.push_back(boost::make_shared<ThreadManager>(RasterizerThreadProc, ThreadData));
            
            // Reset counters
            BlockSize   = 0;
            itPrev      = it;
            
            if (EndOfList)
                break;
        }
        
        ++it;
    }
    
    // Wait here until all threads are finish
    static const u64 CALLBACK_UPDATE_INTERVAL = 100;
    u64 Time = io::Timer::millisecs();
    
    while (1)
    {
        // Check if threads are no longer running
        Mutex.lock();
        if (NumRunningThreads <= 0)
            break;
        Mutex.unlock();
        
        // Yield to give the threads time to run
        io::Timer::yield();
        
        // Progress callback
        if (io::Timer::millisecs() > Time)
        {
            Time = io::Timer::millisecs() + CALLBACK_UPDATE_INTERVAL;
            LightmapGenerator::processRunning(0);
        }
    }
    
    // Boost process
    LightmapGenerator::processRunning(LightmapGenerator::ProgressShadedTriangleNum_);
}

//! Used for "LMapRasterizePixelCallback" callback
struct SRasterizePixelData
{
    LightmapGenerator* LMGen;
    SFace* Face;
    SLightmap* Lightmap;
    const SLight* Light;
    dim::triangle3df TriangleCoords;
    dim::triangle3df TriangleMap;
    dim::vector3df Tangent;
};

void LMapRasterizePixelCallback(
    s32 x, s32 y, const SRasterizerVertex &Vertex, void* UserData)
{
    SRasterizePixelData* RasterData = reinterpret_cast<SRasterizePixelData*>(UserData);
    
    // Set the face into the texel buffer
    SLightmapTexel* Texel = &(RasterData->Lightmap->getTexel(x, y));
    Texel->Face = RasterData->Face;
    
    // Normalize normal vector
    dim::vector3df Normal(Vertex.Normal);
    Normal.normalize();
    
    // Compute final coordinate in 3D space via barycentric coordinates
    const dim::vector3df BarycentricCoord(
        math::getBarycentricCoord(
            RasterData->TriangleMap,
            dim::vector3df(static_cast<f32>(x) + 0.5f, static_cast<f32>(y) + 0.5f, 0.0f)
        )
    );
    
    const dim::vector3df Point = RasterData->TriangleCoords.getBarycentricPoint(BarycentricCoord);
    
    // Setup texel location (if used)
    if (RasterData->Lightmap->TexelLocBuffer)
    {
        SLightmapTexelLoc* TexelLoc = &(RasterData->Lightmap->getTexelLoc(x, y));
        
        TexelLoc->WorldPos  = Point;
        TexelLoc->Normal    = Normal;
        TexelLoc->Tangent   = RasterData->Tangent;
    }
    
    // Process the texel lighting
    RasterData->LMGen->processTexelLighting(Texel, RasterData->Light, Point, Normal);
}

void LightmapGenerator::rasterizeTriangle(const SLight* Light, const STriangle &Triangle)
{
    // Check if the triangle is able to get any light
    if (!Light->checkVisibility(Triangle))
        return;
    
    const SVertex* v = Triangle.Vertices;
    
    // Fill user-data for rasterize pixel callback
    SRasterizePixelData RasterData;
    {
        RasterData.LMGen    = this;
        RasterData.Face     = Triangle.Face;
        RasterData.Lightmap = Triangle.Face->RootLightmap;
        RasterData.Light    = Light;
        
        // Setup tangent space (if texel location buffer is used)
        if (RasterData.Lightmap->TexelLocBuffer)
        {
            // We only need any tangent lying onto the triangle's plane,
            // because this tangent is not used for normal mapping.
            RasterData.Tangent = v[1].Position - v[0].Position;
            RasterData.Tangent.normalize();
        }
        
        RasterData.TriangleCoords.PointA = v[0].Position;
        RasterData.TriangleCoords.PointB = v[1].Position;
        RasterData.TriangleCoords.PointC = v[2].Position;
        
        for (s32 i = 0; i < 3; ++i)
        {
            RasterData.TriangleMap[i].X = static_cast<f32>(v[i].LMapCoord.X);
            RasterData.TriangleMap[i].Y = static_cast<f32>(v[i].LMapCoord.Y);
        }
    }
    
    // Rasterize triangle
    math::Rasterizer::rasterizeTriangle<SRasterizerVertex>(
        LMapRasterizePixelCallback,
        SRasterizerVertex(v[0].Position, v[0].Normal, v[0].LMapCoord),
        SRasterizerVertex(v[1].Position, v[1].Normal, v[1].LMapCoord),
        SRasterizerVertex(v[2].Position, v[2].Normal, v[2].LMapCoord),
        (&RasterData)
    );
}

void LightmapGenerator::processTexelLighting(
    SLightmapTexel* Texel, const SLight* Light, const dim::vector3df &Position, const dim::vector3df &Normal)
{
    static const f32 PICK_ROUND_ERR = 1.0e-4f;
    
    // Configure the picking ray
    dim::line3df PickLine;
    
    PickLine.End = Position;
    
    if (Light->Type == scene::LIGHT_DIRECTIONAL)
        PickLine.Start = PickLine.End - Light->FixedDirection * 100;
    else
        PickLine.Start = Light->Position;
    
    // Temporary variables
    dim::vector3df Color(1.0f);
    
    if (!(State_.Flags & LIGHTMAPFLAG_NOCOLORS))
        Color = Light->Color;
    
    video::MeshBuffer* Surface = 0;
    scene::Mesh* Mesh = 0;
    u32 Indices[3];
    
    if (State_.Flags & LIGHTMAPFLAG_NOTRANSPARENCY)
    {
        if (CollSys_.checkIntersection(PickLine, true))
            return;
    }
    else
    {
        // Make intersection tests
        std::list<scene::SIntersectionContact> ContactList;
        CollSys_.findIntersections(PickLine, ContactList);
        
        // Analyse the intersection results
        foreach (const scene::SIntersectionContact &Contact, ContactList)
        {
            // Finish the picking analyse
            if (math::getDistanceSq(Contact.Point, Position) <= PICK_ROUND_ERR)
                break;
            
            // Process transparency objects
            if (!Contact.Face)
                continue;
            
            Mesh    = Contact.Face->Mesh;
            Surface = Contact.Face->Mesh->getMeshBuffer(Contact.Face->Surface);
            
            Surface->getTriangleIndices(Contact.Face->Index, Indices);
            
            if ( Mesh->getMaterial()->getDiffuseColor().Alpha < 255 ||
                 Surface->getVertexColor(Indices[0]).Alpha < 255 ||
                 Surface->getVertexColor(Indices[1]).Alpha < 255 ||
                 Surface->getVertexColor(Indices[2]).Alpha < 255 ||
                 ( Surface->getTexture(0) && Surface->getTexture(0)->getColorKey().Alpha < 255 ) )
            {
                dim::point2df TexCoord;
                dim::vector3df VertexColor;
                f32 Alpha;
                
                STriangle::computeInterpolation(Contact, Indices, 0, TexCoord, VertexColor, Alpha);
                
                if (Surface->getTexture(0))
                {
                    video::ImageBuffer* ImgBuffer = Surface->getTexture(0)->getImageBuffer();
                    const video::color TexelColor(
                        ImgBuffer->getPixelColor(ImgBuffer->getPixelCoord(TexCoord))
                    );
                    
                    Alpha *= static_cast<f32>(TexelColor.Alpha) / 255;
                    Color *= ( SVertex::getVectorColor(TexelColor) * Alpha + (1.0f - Alpha) );
                }
                
                Color *= VertexColor * (1.0f - Alpha);
            }
            else
                return;
        }
    }
    
    Color *= Light->getIntensity(Position, Normal);
    
    Texel->Color.Red    = math::MinMax<s32>(static_cast<s32>(Color.X * 255.0f) + Texel->Color.Red   , 0, 255);
    Texel->Color.Green  = math::MinMax<s32>(static_cast<s32>(Color.Y * 255.0f) + Texel->Color.Green , 0, 255);
    Texel->Color.Blue   = math::MinMax<s32>(static_cast<s32>(Color.Z * 255.0f) + Texel->Color.Blue  , 0, 255);
}

void LightmapGenerator::shadeAllLightmaps()
{
    if (State_.Flags & LIGHTMAPFLAG_GPU_ACCELERATION)
        shadeAllLightmapsOnGPU();
    else
        shadeAllLightmapsOnCPU();
}

void LightmapGenerator::shadeAllLightmapsOnCPU()
{
    // Compute each texel color of the infected triangle's face's lightmap by each light source
    u32 InfoLightNum = 0;
    
    foreach (SLight* Lit, LightSources_)
    {
        if (!LightmapGenerator::processRunning(0))
            throw std::exception();
        
        updateStateInfo(
            LIGHTMAPSTATE_SHADING,
            "Light source " + io::stringc(++InfoLightNum) + " / " + io::stringc(LightSources_.size())
        );
        
        if (State_.ThreadCount > 1)
            generateLightTexelsMultiThreaded(Lit);
        else
            generateLightTexelsSingleThreaded(Lit);
    }
}

void LightmapGenerator::shadeAllLightmapsOnGPU()
{
    
    
    
    
}

void LightmapGenerator::partitionScene(f32 DefaultDensity)
{
    updateStateInfo(LIGHTMAPSTATE_PARTITIONING);
    
    foreach (SModel* Mdl, GetShadowObjects_)
    {
        if (!LightmapGenerator::processRunning())
            throw std::exception();
        
        Mdl->partitionMesh(LightmapGenerator::LightmapSize_, DefaultDensity);
        createFacesLightmaps(Mdl);
    }
}

void LightmapGenerator::createNewLightmap()
{
    bool UseTexelLocBuffer = (State_.Flags & LIGHTMAPFLAG_GPU_ACCELERATION) != 0;
    
    CurLightmap_ = new SLightmap(LightmapSize_, true, UseTexelLocBuffer);
    {
        CurLightmap_->RectNode = new TRectNode();
        CurLightmap_->RectNode->setRect(dim::rect2di(0, 0, LightmapSize_.Width, LightmapSize_.Height));
        
        CurRectRoot_ = CurLightmap_->RectNode;
    }
    Lightmaps_.push_back(CurLightmap_);
}

void LightmapGenerator::putFaceIntoLightmap(SFace* Face)
{
    TRectNode* Node = CurRectRoot_->insert(Face->Lightmap);
    Face->RootLightmap = CurLightmap_;
    
    if (Node)
    {
        const dim::rect2di Rect(Face->Lightmap->RectNode->getRect());
        
        foreach (STriangle &Tri, Face->Triangles)
        {
            for (s32 i = 0; i < 3; ++i)
            {
                Tri.Vertices[i].LMapCoord.X += Rect.Left + 1;
                Tri.Vertices[i].LMapCoord.Y += Rect.Top  + 1;
            }
        }
    }
    else
    {
        createNewLightmap();
        putFaceIntoLightmap(Face);
    }
}

void LightmapGenerator::buildFinalMesh(SModel* Model)
{
    if (Model->StayAlone)
    {
        scene::Mesh* Mesh = GlbSceneGraph->createMesh();
        
        Mesh->setName(Model->Mesh->getName());
        Mesh->setUserData(Model->Mesh->getUserData());
        
        Model->buildFaces(Mesh, LightmapGenerator::LightmapSize_);
        Mesh->mergeMeshBuffers();
        Mesh->getMaterial()->setLighting(false);
        
        SingleModels_.push_back(Mesh);
    }
    else
        Model->buildFaces(FinalModel_, LightmapGenerator::LightmapSize_);
}

void LightmapGenerator::buildAllFinalModels()
{
    // Build all final models
    foreach (SModel* Obj, GetShadowObjects_)
    {
        if (!LightmapGenerator::processRunning())
            throw std::exception();
        buildFinalMesh(Obj);
    }
    
    // Finalize and optimize final model
    FinalModel_->updateMeshBuffer();
    FinalModel_->mergeMeshBuffers();
}

//! Used for "LMapBlurPixelCallback" callback.
struct SBlurPixelData
{
    SLightmap* Map;
    SFace* Face;
    s32 Factor;
};

void LMapBlurPixelCallback(s32 x, s32 y, void* UserData)
{
    SBlurPixelData* BlurData = reinterpret_cast<SBlurPixelData*>(UserData);
    
    // Store options
    SLightmap* Map      = BlurData->Map;
    SFace* Face         = BlurData->Face;
    const s32 Factor    = BlurData->Factor;
    
    // Blur the texel
    s32 dx, dy, c = 0;
    dim::vector3df Color;
    
    for (dy = y - Factor; dy <= y + Factor; ++dy)
    {
        if (dy < 0 || dy >= LightmapGenerator::LightmapSize_.Width)
            continue;
        
        for (dx = x - Factor; dx <= x + Factor; ++dx)
        {
            // Check if the texel is inside the lightmap's face area
            if (dx < 0 || dx >= LightmapGenerator::LightmapSize_.Height || Map->getTexel(dx, dy).Face != Face)
                continue;
            
            Color += Map->getTexel(dx, dy).OrigColor.getVector();
            ++c;
        }
    }
    
    if (c)
        Map->getTexel(x, y).Color = video::color(Color / static_cast<f32>(c), false);
}

void LightmapGenerator::blurLightmapTexels(SModel* Model, s32 Factor)
{
    SBlurPixelData BlurData;
    BlurData.Factor = Factor;
    
    for (s32 i = 0; i < 6; ++i)
    {
        foreach (SFace &Face, Model->Axles[i].Faces)
        {
            BlurData.Map    = Face.RootLightmap;
            BlurData.Face   = (&Face);
            
            foreach (STriangle &Tri, Face.Triangles)
            {
                math::Rasterizer::rasterizeTriangle(
                    LMapBlurPixelCallback,
                    Tri.Vertices[0].LMapCoord,
                    Tri.Vertices[1].LMapCoord,
                    Tri.Vertices[2].LMapCoord,
                    (&BlurData)
                );
            }
        }
    }
}

void LightmapGenerator::blurAllLightmaps(u8 TexelBlurRadius)
{
    updateStateInfo(LIGHTMAPSTATE_BLURING);
    
    // Blur the lightmaps' texels
    foreach (SModel* Mdl, GetShadowObjects_)
    {
        if (!LightmapGenerator::processRunning())
            throw std::exception();
        blurLightmapTexels(Mdl, static_cast<s32>(TexelBlurRadius));
    }
}

void LightmapGenerator::createFinalLightmapTextures(const video::color &AmbientColor)
{
    updateStateInfo(LIGHTMAPSTATE_BAKING);
    
    foreach (SLightmap* LMap, Lightmaps_)
    {
        // Reduce texture bleeding and create final texture with given ambient color
        LMap->reduceBleeding();
        LMap->createTexture(AmbientColor);
    }
}

void LightmapGenerator::updateStateInfo(const ELightmapGenerationStates State, const io::stringc &Info)
{
    if (StateCallback_)
        StateCallback_(State, Info);
}

void LightmapGenerator::clearLightmapObjects()
{
    // Delete all old lightmaps
    foreach (SLightmap* LMap, Lightmaps_)
        MemoryManager::deleteMemory(LMap);
    Lightmaps_.clear();
    
    // Delete the get-shadow objects, light sources & lightmap textures
    MemoryManager::deleteList(GetShadowObjects_);
    MemoryManager::deleteList(LightSources_);
}

bool LightmapGenerator::processRunning(s32 BoostFactor)
{
    if (!ProgressCallback_)
        return true;
    
    Progress_ += BoostFactor;
    
    f32 Percent = static_cast<f32>(Progress_);
    
    if (ProgressMax_)
        Percent /= ProgressMax_;
    
    return ProgressCallback_(Percent);
}


/*
 * SInternalState structure
 */

LightmapGenerator::SInternalState::SInternalState() :
    Flags                   (0      ),
    AmbientColor            (20     ),
    TexelBlurRadius         (0      ),
    ThreadCount             (0      ),
    HasGeneratedSuccessful  (false  )
{
}
LightmapGenerator::SInternalState::~SInternalState()
{
}

void LightmapGenerator::SInternalState::validateFlags()
{
    if ((Flags & LIGHTMAPFLAG_GPU_ACCELERATION) != 0 && !GlbRenderSys->queryVideoSupport(video::QUERY_COMPUTE_SHADER))
    {
        io::Log::warning("Hardware acceleration for lightmap generation is not available");
        math::removeFlag(Flags, LIGHTMAPFLAG_GPU_ACCELERATION);
    }
    if ((Flags & LIGHTMAPFLAG_RADIOSITY) != 0 && (Flags & LIGHTMAPFLAG_GPU_ACCELERATION) == 0)
    {
        io::Log::warning("Radiosity lightmap generation is only supported with hardware acceleration");
        math::removeFlag(Flags, LIGHTMAPFLAG_RADIOSITY);
    }
}


} // /namespace tool

} // /namespace sp


#endif



// ================================================================================
