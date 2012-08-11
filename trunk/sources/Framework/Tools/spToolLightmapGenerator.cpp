/*
 * Lightmap generator file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Framework/Tools/spToolLightmapGenerator.hpp"

#ifdef SP_COMPILE_WITH_LIGHTMAPGENERATOR


#include "Base/spMathCollisionLibrary.hpp"
#include "Platform/spSoftPixelDeviceOS.hpp"

#include <boost/foreach.hpp>


namespace sp
{

extern video::RenderSystem* __spVideoDriver;
extern scene::SceneGraph* __spSceneManager;

namespace tool
{


/*
 * Static class members
 */

LightmapCallback LightmapGenerator::Callback_ = 0;

s32 LightmapGenerator::Progress_        = 0;
s32 LightmapGenerator::ProgressMax_     = 0;

f32 LightmapGenerator::DefaultDensity_  = 0.1f;

video::color LightmapGenerator::AmbientColor_;
dim::size2di LightmapGenerator::LightmapSize_ = dim::size2di(512);


/*
 * LightmapGenerator class
 */

LightmapGenerator::LightmapGenerator() :
    FinalModel_     (0),
    CollMesh_       (0),
    CurLightmap_    (0),
    CurRectRoot_    (0),
    TexelBlurRadius_(0),
    Flags_          (0)
{
}
LightmapGenerator::~LightmapGenerator()
{
}

scene::Mesh* LightmapGenerator::generateLightmaps(
    const std::list<SCastShadowObject> &CastShadowObjects,
    const std::list<SGetShadowObject> &GetShadowObjects,
    const std::list<SLightmapLight> &LightSources,
    const video::color &AmbientColor,
    const u32 LightmapsSize, const f32 DefaultDensity, const u32 TexelBlurRadius,
    const s32 Flags)
{
    // General settings
    CastShadowObjects_  = CastShadowObjects;
    LightmapSize_       = LightmapsSize;
    DefaultDensity_     = DefaultDensity;
    AmbientColor_       = AmbientColor;
    TexelBlurRadius_    = TexelBlurRadius;
    Flags_              = Flags;
    
    // Delete the old lightmap objects & textures
    clearLightmapObjects();
    
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
    
    CollMesh_ = CollSys_.createMeshList(0, CollMeshList, 20);
    
    foreach (const SLightmapLight &Light, LightSources)
    {
        if (Light.Visible)
            LightSources_.push_back(new SLight(Light));
    }
    
    // Calculate the progress maximum
    Progress_       = 0;
    ProgressMax_    = GetShadowObjects_.size() * 8;
    
    foreach (SModel* Obj, GetShadowObjects_)
        ProgressMax_ += Obj->Mesh->getTriangleCount() * (LightSources_.size() + 1);
    
    if (TexelBlurRadius_)
        ProgressMax_ += GetShadowObjects_.size();
    
    // Create the root object & partition the add-shadow objects
    FinalModel_ = __spSceneManager->createMesh();
    FinalModel_->getMaterial()->setLighting(false);
    
    for (std::list<SModel*>::iterator it = GetShadowObjects_.begin(); it != GetShadowObjects_.end() && processRunning(); ++it)
    {
        (*it)->partitionMesh();
        createFacesLightmaps(*it);
    }
    
    if (!processRunning(false))
        return FinalModel_;
    
    // Compute each texel color of the infected triangle's face's lightmap by each light source
    for (std::list<SLight*>::iterator it = LightSources_.begin(); it != LightSources_.end() && processRunning(false); ++it)
        generateLightTexels(*it);
    
    if (!processRunning(false))
        return FinalModel_;
    
    if (TexelBlurRadius_)
    {
        // Copy image buffers
        for (std::list<SLightmap*>::iterator it = Lightmaps_.begin(); it != Lightmaps_.end(); ++it)
            (*it)->copyImageBuffers();
        
        // Blur the lightmaps' texels
        for (std::list<SModel*>::iterator it = GetShadowObjects_.begin(); it != GetShadowObjects_.end() && processRunning(); ++it)
            (*it)->blurLightmapTexels(TexelBlurRadius_);
    }
    
    // Create the final lightmap textures
    foreach (SLightmap* LMap, Lightmaps_)
    {
        LMap->reduceBleeding();
        LMap->createTexture();
    }
    
    // Build the final faces
    for (std::list<SModel*>::iterator it = GetShadowObjects_.begin(); it != GetShadowObjects_.end() && processRunning(); ++it)
        buildFinalMesh(*it);
    
    // Delete the get-shadow objects, light sources & lightmap textures
    MemoryManager::deleteList(GetShadowObjects_);
    MemoryManager::deleteList(LightSources_);
    
    foreach (SLightmap* LMap, Lightmaps_)
    {
        LightmapTextures_.push_back(LMap->Texture);
        MemoryManager::deleteMemory(LMap);
    }
    Lightmaps_.clear();
    
    // Optimize surfaces & transparent triangles
    FinalModel_->updateMeshBuffer();
    //FinalModel_->optimizeMeshBuffers();
    //FinalModel_->optimizeTransparency();
    
    CollMesh_ = 0;
    
    return FinalModel_;
}

void LightmapGenerator::clearLightmapObjects()
{
    __spSceneManager->deleteNode(FinalModel_);
    FinalModel_ = 0;
    
    foreach (scene::Mesh* Obj, SingleModels_)
        __spSceneManager->deleteNode(Obj);
    
    foreach (video::Texture* Tex, LightmapTextures_)
        __spVideoDriver->deleteTexture(Tex);
    
    SingleModels_.clear();
    LightmapTextures_.clear();
}

void LightmapGenerator::setCallback(const LightmapCallback &Callback)
{
    Callback_ = Callback;
}


/*
 * ======= Private: =======
 */

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

void LightmapGenerator::generateLightTexels(SLight* Light)
{
    #if 1 //!!!
    
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
        foreach (scene::SCollisionFace* Face, *TreeNodeData)
        {
            // Get model object
            std::map<scene::Mesh*, SModel*>::iterator it = ModelMap_.find(Face->Mesh);
            
            if (it == ModelMap_.end())
                continue;
            
            Obj = it->second;
            
            // Get triangle object
            STriangle* Triangle = (Obj->Triangles[Face->Surface])[Face->Index];
            
            if (UsedTriangles.find(Triangle) != UsedTriangles.end())
            {
                if (!processRunning(false))
                    return;
                continue;
            }
            UsedTriangles[Triangle] = true;
            
            if (!processRunning())
                return;
            
            // Rasterize triangle
            CurLightmap_ = Triangle->Face->RootLightmap;
            rasterizeTriangle(Light, *Triangle);
        }
    }
    
    #else
    
    foreach (SModel* Obj, GetShadowObjects_)
    {
        for (s32 i = 0; i < 6; ++i)
        {
            foreach (SFace &Face, Obj->Axles[i].Faces)
            {
                CurLightmap_ = Face.RootLightmap;
                foreach (STriangle &Triangle, Face.Triangles)
                {
                    rasterizeTriangle(Light, Triangle);
                    if (!processRunning())
                        return;
                }
            }
        }
    }
    
    #endif
}

void LightmapGenerator::rasterizeTriangle(const SLight* Light, const STriangle &Triangle)
{
    // Check if the triangle is able to get any light
    const SVertex* v[3] = { &Triangle.Vertices[0], &Triangle.Vertices[1], &Triangle.Vertices[2] };
    
    if (!Light->checkVisibility(Triangle))
        return;
    
    // Temporary variables
    s32 x = 0, y = 0, xStart = 0, xEnd = 0, yStart = 0, yMiddle = 0, yEnd = 0, yMiddleStart = 0, yEndMiddle = 0, yEndStart = 0;
    
    SRasterPolygonSide lside, rside, step, cur;
    
    SLightmapTexel* Texel = 0;
    
    // Compute rasterization area
    STriangle::computeRasterArea(v, yStart, yMiddle, yEnd, yMiddleStart, yEndMiddle, yEndStart);
    
    // Loop for each scanline
    for (y = yStart; y < yEnd; ++y)
    {
        // Compute the scanline dimension
        STriangle::computeRasterScanline(
            v, xStart, xEnd, y, yStart, yMiddle, yMiddleStart, yEndMiddle, yEndStart
        );
        
        // Compute the polygon sides
        if (xStart > xEnd)
        {
            math::Swap(xStart, xEnd);
            STriangle::rasterizePolygonSide(v, y, yStart, yMiddle, lside, rside);
        }
        else
            STriangle::rasterizePolygonSide(v, y, yStart, yMiddle, rside, lside);
        
        // Compute the steps
        step.Normal = (rside.Normal - lside.Normal) / static_cast<f32>(xEnd - xStart);
        cur.Normal  = lside.Normal;
        
        step.Position   = (rside.Position - lside.Position) / static_cast<f32>(xEnd - xStart);
        cur.Position    = lside.Position;
        
        // Loop for each texel
        for (x = xStart; x < xEnd; ++x)
        {
            cur.Normal.normalize();
            
            // Set the face into the texel buffer
            Texel = &CurLightmap_->getTexel(x, y);
            Texel->Face = Triangle.Face;
            
            // Process the texel lighting
            processTexelLighting(Texel, Light, cur);
            
            // Interpolate the normal
            cur.Normal      += step.Normal;
            cur.Position    += step.Position;
        }
    }
}

void LightmapGenerator::processTexelLighting(
    SLightmapTexel* Texel, const SLight* Light, const SRasterPolygonSide &Point)
{
    static const f32 PICK_ROUND_ERR = 1.0e-4f;
    
    // Configure the picking ray
    dim::line3df PickLine;
    
    PickLine.End = Point.Position;
    
    if (Light->Type == scene::LIGHT_DIRECTIONAL)
        PickLine.Start = PickLine.End - Light->FixedDirection * 100;
    else
        PickLine.Start = Light->Position;
    
    // Temporary variables
    dim::vector3df Color(1.0f);
    
    if (!(Flags_ & LIGHTMAPFLAG_NOCOLORS))
        Color = Light->Color;
    
    video::MeshBuffer* Surface = 0;
    scene::Mesh* Mesh = 0;
    u32 Indices[3];
    
    // Make intersection tests
    std::list<scene::SIntersectionContact> ContactList;
    CollSys_.findIntersections(PickLine, ContactList);
    
    // Analyse the intersection results
    foreach (const scene::SIntersectionContact &Contact, ContactList)
    {
        // Finish the picking analyse
        if (math::getDistanceSq(Contact.Point, Point.Position) <= PICK_ROUND_ERR)
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
    
    Color *= Light->getIntensity(Point.Position, Point.Normal);
    
    Texel->Color.Red    = math::MinMax<s32>(static_cast<s32>(Color.X * 255.0f) + Texel->Color.Red   , 0, 255);
    Texel->Color.Green  = math::MinMax<s32>(static_cast<s32>(Color.Y * 255.0f) + Texel->Color.Green , 0, 255);
    Texel->Color.Blue   = math::MinMax<s32>(static_cast<s32>(Color.Z * 255.0f) + Texel->Color.Blue  , 0, 255);
}

void LightmapGenerator::createNewLightmap()
{
    CurLightmap_ = new SLightmap(LightmapSize_);
    {
        CurLightmap_->RectNode          = new SRectNode();
        CurLightmap_->RectNode->Rect    = dim::rect2di(0, 0, LightmapSize_.Width, LightmapSize_.Height);
        CurRectRoot_                    = CurLightmap_->RectNode;
    }
    Lightmaps_.push_back(CurLightmap_);
}

void LightmapGenerator::putFaceIntoLightmap(SFace* Face)
{
    SRectNode* Node = CurRectRoot_->insert(Face->Lightmap);
    Face->RootLightmap = CurLightmap_;
    
    if (Node)
    {
        const dim::rect2di Rect(Face->Lightmap->RectNode->Rect);
        
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
        scene::Mesh* Mesh = __spSceneManager->createMesh();
        
        Mesh->setName(Model->Mesh->getName());
        Mesh->setUserData(Model->Mesh->getUserData());
        
        Model->buildFaces(Mesh);
        Mesh->optimizeMeshBuffers();
        Mesh->getMaterial()->setLighting(false);
        
        SingleModels_.push_back(Mesh);
    }
    else
        Model->buildFaces(FinalModel_);
}

bool LightmapGenerator::processRunning(bool BoostProgress)
{
    if (!Callback_)
        return true;
    
    if (BoostProgress)
        ++Progress_;
    
    f32 Percent = static_cast<f32>(Progress_);
    
    if (ProgressMax_)
        Percent /= ProgressMax_;
    
    return Callback_(Percent);
}


/*
 * SCastShadowObject structure
 */

SCastShadowObject::SCastShadowObject(scene::Mesh* Obj) : Mesh(Obj)
{
}
SCastShadowObject::~SCastShadowObject()
{
}


/*
 * SGetShadowObject structure
 */

SGetShadowObject::SGetShadowObject(scene::Mesh* Obj, bool DefStayAlone) :
    Mesh        (Obj            ),
    StayAlone   (DefStayAlone   )
{
    TrianglesDensity.resize(Mesh->getMeshBufferCount());
    for (u32 s = 0; s < TrianglesDensity.size(); ++s)
        TrianglesDensity[s].resize(Mesh->getMeshBuffer(s)->getTriangleCount());
}
SGetShadowObject::SGetShadowObject(
    scene::Mesh* Obj, const std::vector< std::vector<f32> > &DefTrianglesDensity, bool DefStayAlone) :
    Mesh            (Obj                ),
    StayAlone       (DefStayAlone       ),
    TrianglesDensity(DefTrianglesDensity)
{
    if (TrianglesDensity.size() != Mesh->getMeshBufferCount())
        TrianglesDensity.resize(Mesh->getMeshBufferCount());
    for (u32 s = 0; s < TrianglesDensity.size(); ++s)
    {
        u32 TriCount = Mesh->getMeshBuffer(s)->getTriangleCount();
        if (TrianglesDensity[s].size() != TriCount)
            TrianglesDensity[s].resize(TriCount);
    }
}
SGetShadowObject::~SGetShadowObject()
{
}


/*
 * SLightmapLight structure
 */

SLightmapLight::SLightmapLight() :
    Type            (scene::LIGHT_DIRECTIONAL   ),
    Attn0           (1.0f                       ),
    Attn1           (0.0f                       ),
    Attn2           (0.0f                       ),
    InnerConeAngle  (30.0f                      ),
    OuterConeAngle  (60.0f                      ),
    Visible         (false                      )
{
}
SLightmapLight::SLightmapLight(const scene::Light* Obj) :
    Type            (scene::LIGHT_DIRECTIONAL   ),
    Attn0           (1.0f                       ),
    Attn1           (0.0f                       ),
    Attn2           (0.0f                       ),
    InnerConeAngle  (30.0f                      ),
    OuterConeAngle  (60.0f                      ),
    Visible         (false                      )
{
    if (!Obj)
        return;
    
    Type = Obj->getLightingType();
    Matrix = Obj->getTransformation(true);
    
    video::color ClrDiffuse, ClrAmbient, ClrSpecular;
    Obj->getLightingColor(ClrDiffuse, ClrAmbient, ClrSpecular);
    
    Color = ClrDiffuse;
    
    if (Obj->getVolumetric())
    {
        Attn0 = 1.0f;
        Attn1 = 1.0f / Obj->getVolumetricRadius();
        Attn2 = 1.0f / Obj->getVolumetricRadius();
    }
    
    Obj->getSpotCone(InnerConeAngle, OuterConeAngle);
    Visible = Obj->getVisible();
}
SLightmapLight::~SLightmapLight()
{
}


/*
 * SVertex structure
 */

LightmapGenerator::SVertex::SVertex() :
    Surface (0      ),
    Index   (0      ),
    Fog     (0.0f   )
{
}
LightmapGenerator::SVertex::SVertex(const SModel* Model, const u32 VertexSurface, const u32 VertexIndex) :
    Surface (VertexSurface  ),
    Index   (VertexIndex    ),
    Fog     (0.0f           )
{
    video::MeshBuffer* Surface = Model->Mesh->getMeshBuffer(VertexSurface);
    
    Position    = Model->Matrix * Surface->getVertexCoord(Index);
    Normal      = Model->NormalMatrix * Surface->getVertexNormal(Index);
    Color       = Surface->getVertexColor(Index);
    Fog         = Surface->getVertexFog(Index);
    
    Normal.normalize();
    
    for (u32 i = 0; i < MAX_COUNT_OF_TEXTURES && i < Surface->getTextureCount(); ++i)
        TexCoord[i] = Surface->getVertexTexCoord(Index, i);
}
LightmapGenerator::SVertex::~SVertex()
{
}

bool LightmapGenerator::SVertex::adjacency(const STriangle &OpTriangle) const
{
    return
        Position.equal(OpTriangle.Vertices[0].Position) ||
        Position.equal(OpTriangle.Vertices[1].Position) ||
        Position.equal(OpTriangle.Vertices[2].Position);
}

void LightmapGenerator::SVertex::scaleProj(const dim::size2df &Scale)
{
    LMapCoord.X = static_cast<s32>(Scale.Width * LMapCoord.X);
    LMapCoord.Y = static_cast<s32>(Scale.Height * LMapCoord.Y);
}

dim::point2df LightmapGenerator::SVertex::getMapCoord() const
{
    return dim::point2df(
        static_cast<f32>(LMapCoord.X) / LightmapSize_.Width,
        static_cast<f32>(LMapCoord.Y) / LightmapSize_.Height
    );
}

dim::vector3df LightmapGenerator::SVertex::getVectorColor(const video::color &Color)
{
    return dim::vector3df(Color.Red, Color.Green, Color.Blue) / 255;
}


/*
 * STriangle structure
 */

LightmapGenerator::STriangle::STriangle() :
    Surface (0),
    Index   (0),
    Face    (0)
{
}
LightmapGenerator::STriangle::STriangle(
    const SModel* Model, const u32 TriangleSurface, const u32 TriangleIndex, const u32 DefIndices[3]) :
    Surface (TriangleSurface),
    Index   (TriangleIndex  ),
    Face    (0              )
{
    for (s32 i = 0; i < 3; ++i)
        Vertices[i] = SVertex(Model, TriangleSurface, DefIndices[i]);
    
    Plane = dim::plane3df(Vertices[0].Position, Vertices[1].Position, Vertices[2].Position);
}
LightmapGenerator::STriangle::~STriangle()
{
}

bool LightmapGenerator::STriangle::adjacency(const STriangle &OpTriangle) const
{
    if (Surface != OpTriangle.Surface)
        return false;
    
    return
        Vertices[0].adjacency(OpTriangle) ||
        Vertices[1].adjacency(OpTriangle) ||
        Vertices[2].adjacency(OpTriangle);
}

f32 LightmapGenerator::STriangle::getDistance(const dim::vector3df &Point) const
{
    return math::getDistance(
        math::CollisionLibrary::getClosestPoint(
            dim::triangle3df(Vertices[0].Position, Vertices[1].Position, Vertices[2].Position), Point
        ),
        Point
    );
}

void LightmapGenerator::STriangle::blurTexels(const s32 Factor)
{
    // Temporary variables
    const SVertex* v[3] = { &Vertices[0], &Vertices[1], &Vertices[2] };
    
    s32 dx = 0, dy = 0, dc = 0;
    s32 x = 0, y = 0, xStart = 0, xEnd = 0, yStart = 0, yMiddle = 0, yEnd = 0, yMiddleStart = 0, yEndMiddle = 0, yEndStart = 0;
    
    SLightmap* Map = Face->RootLightmap;
    dim::vector3df Color;
    
    // Compute the rasterization area
    STriangle::computeRasterArea(v, yStart, yMiddle, yEnd, yMiddleStart, yEndMiddle, yEndStart);
    
    // Loop for each scanline
    for (y = yStart; y < yEnd; ++y)
    {
        // Compute the scanline dimension
        STriangle::computeRasterScanline(
            v, xStart, xEnd, y, yStart, yMiddle, yMiddleStart, yEndMiddle, yEndStart
        );
        
        // Compute the polygon sides
        if (xStart > xEnd)
            math::Swap(xStart, xEnd);
        
        // Loop for each texel
        for (x = xStart; x < xEnd; ++x)
        {
            // Blur the texel
            dc = 0;
            Color = 0.0f;
            
            for (dy = y - Factor; dy <= y + Factor; ++dy)
            {
                if (dy < 0 || dy >= LightmapSize_.Width)
                    continue;
                
                for (dx = x - Factor; dx <= x + Factor; ++dx)
                {
                    // Check if the texel is inside the lightmap's face area
                    if (dx < 0 || dx >= LightmapSize_.Height || Map->getTexel(dx, dy).Face != Face)
                        continue;
                    
                    Color += Map->getTexel(dx, dy).OrigColor.getVector();
                    ++dc;
                }
            }
            
            if (dc)
                Map->getTexel(x, y).Color = video::color(Color / static_cast<f32>(dc), false);
        }
    }
}

dim::point2df LightmapGenerator::STriangle::getProjection(
    const dim::vector3df &Point, const dim::vector3df &Normal, const f32 Density)
{
    const dim::vector3df AbsNormal(Normal.getAbs());
    
    if (AbsNormal.X >= AbsNormal.Y && AbsNormal.X >= AbsNormal.Z)
        return dim::point2df(Point.Z, -Point.Y) * Density;
    else if (AbsNormal.Y >= AbsNormal.X && AbsNormal.Y >= AbsNormal.Z)
        return dim::point2df(Point.X, -Point.Z) * Density;
    
    return dim::point2df(Point.X, -Point.Y) * Density;
}

void LightmapGenerator::STriangle::computeRasterArea(
    const SVertex* (&v)[3], s32 &yStart, s32 &yMiddle, s32 &yEnd, s32 &yMiddleStart, s32 &yEndMiddle, s32 &yEndStart)
{
    // Sort the vertices in dependet of the y axis
    if (v[0]->LMapCoord.Y > v[1]->LMapCoord.Y) math::Swap(v[0], v[1]);
    if (v[0]->LMapCoord.Y > v[2]->LMapCoord.Y) math::Swap(v[0], v[2]);
    if (v[1]->LMapCoord.Y > v[2]->LMapCoord.Y) math::Swap(v[1], v[2]);
    
    // Set the vertices position
    yStart  = v[0]->LMapCoord.Y;
    yMiddle = v[1]->LMapCoord.Y;
    yEnd    = v[2]->LMapCoord.Y;
    
    // Compute the dimensions
    yMiddleStart    = yMiddle - yStart;
    yEndMiddle      = yEnd - yMiddle;
    yEndStart       = yEnd - yStart;
}

void LightmapGenerator::STriangle::computeRasterScanline(
    const SVertex* (&v)[3], s32 &xStart, s32 &xEnd,
    const s32 y, const s32 yStart, const s32 yMiddle, const s32 yMiddleStart, const s32 yEndMiddle, const s32 yEndStart)
{
    // Compute the scanline dimension
    if (y < yMiddle)
        xStart = v[0]->LMapCoord.X + ( v[1]->LMapCoord.X - v[0]->LMapCoord.X ) * ( y - yStart ) / yMiddleStart;
    else if (y > yMiddle)
        xStart = v[1]->LMapCoord.X + ( v[2]->LMapCoord.X - v[1]->LMapCoord.X ) * ( y - yMiddle ) / yEndMiddle;
    else
        xStart = v[1]->LMapCoord.X;
    
    xEnd = v[0]->LMapCoord.X + ( v[2]->LMapCoord.X - v[0]->LMapCoord.X ) * ( y - yStart ) / yEndStart;
}

void LightmapGenerator::STriangle::rasterizePolygonSide(
    const SVertex* (&v)[3], s32 y, s32 yStart, s32 yMiddle, SRasterPolygonSide &a, SRasterPolygonSide &b)
{
    f32 Factor  = static_cast<f32>(y - yStart) / (v[2]->LMapCoord.Y - v[0]->LMapCoord.Y);
    a.Normal    = v[0]->Normal      + (v[2]->Normal     - v[0]->Normal  ) * Factor;
    a.Position  = v[0]->Position    + (v[2]->Position   - v[0]->Position) * Factor;
    
    if (y < yMiddle)
    {
        Factor      = static_cast<f32>(y - yStart) / (v[1]->LMapCoord.Y - v[0]->LMapCoord.Y);
        b.Normal    = v[0]->Normal      + (v[1]->Normal     - v[0]->Normal  ) * Factor;
        b.Position  = v[0]->Position    + (v[1]->Position   - v[0]->Position) * Factor;
    }
    else
    {
        Factor      = static_cast<f32>(y - yMiddle) / (v[2]->LMapCoord.Y - v[1]->LMapCoord.Y);
        b.Normal    = v[1]->Normal      + (v[2]->Normal     - v[1]->Normal  ) * Factor;
        b.Position  = v[1]->Position    + (v[2]->Position   - v[1]->Position) * Factor;
    }
}

void LightmapGenerator::STriangle::computeInterpolation(
    const scene::SIntersectionContact &Contact, u32 Indices[3], const u8 Layer,
    dim::point2df &TexCoord, dim::vector3df &Color, f32 &Alpha)
{
    // Get triangle construction
    if (!Contact.Face)
        return;
    
    const dim::triangle3df Triangle(Contact.Triangle);
    scene::Mesh* Mesh = Contact.Face->Mesh;
    
    // Temporary variables
    const dim::vector3df u(Triangle.PointB - Triangle.PointA);
    const dim::vector3df v(Triangle.PointC - Triangle.PointA);
    
    const f32 lenu = u.getLength();
    const f32 lenv = v.getLength();
    
    // Get the texture coordinates & vertex colors
    video::MeshBuffer* Surface = Mesh->getMeshBuffer(Mesh->getMeshBufferCount() - 1);
    
    const dim::point2df TexCoordA(Surface->getVertexTexCoord(Indices[0], Layer));
    const dim::point2df TexCoordB(Surface->getVertexTexCoord(Indices[1], Layer));
    const dim::point2df TexCoordC(Surface->getVertexTexCoord(Indices[2], Layer));
    
    const video::color RealColorA(Surface->getVertexColor(Indices[0]));
    const video::color RealColorB(Surface->getVertexColor(Indices[1]));
    const video::color RealColorC(Surface->getVertexColor(Indices[2]));
    
    const dim::vector3df ColorA(SVertex::getVectorColor(RealColorA));
    const dim::vector3df ColorB(SVertex::getVectorColor(RealColorB));
    const dim::vector3df ColorC(SVertex::getVectorColor(RealColorC));
    
    const f32 AlphaA = static_cast<f32>(RealColorA.Alpha) / 255;
    const f32 AlphaB = static_cast<f32>(RealColorB.Alpha) / 255;
    const f32 AlphaC = static_cast<f32>(RealColorC.Alpha) / 255;
    
    // Compute the first coordinates
    dim::vector3df pa, pb;
    
    math::CollisionLibrary::getLineLineIntersection(
        dim::line3df(Triangle.PointA, Triangle.PointB), dim::line3df(Contact.Point, Contact.Point - v), pa
    );
    math::CollisionLibrary::getLineLineIntersection(
        dim::line3df(Triangle.PointA, Triangle.PointC), dim::line3df(Contact.Point, Contact.Point - u), pb
    );
    
    const dim::point2df map(math::getDistance(Contact.Point, pb), math::getDistance(Contact.Point, pa));
    
    // Get the texture coordinate
    TexCoord    = TexCoordA + (TexCoordB    - TexCoordA ) * map.X / lenu + (TexCoordC   - TexCoordA ) * map.Y / lenv;
    Color       = ColorA    + (ColorB       - ColorA    ) * map.X / lenu + (ColorC      - ColorA    ) * map.Y / lenv;
    Alpha       = AlphaA    + (AlphaB       - AlphaA    ) * map.X / lenu + (AlphaC      - AlphaA    ) * map.Y / lenv;
}


/*
 * SFace structure
 */

LightmapGenerator::SFace::SFace(SAxisData* FaceAxis) :
    Density     (0.1f       ),
    Surface     (0          ),
    Lightmap    (0          ),
    RootLightmap(0          ),
    Axis        (FaceAxis   )
{
}
LightmapGenerator::SFace::~SFace()
{
}

void LightmapGenerator::SFace::computeDensityAverage()
{
    Density = 0.0f;
    
    foreach (const STriangle &Tri, Triangles)
        Density += (Axis->Model->TrianglesDensity[Tri.Surface])[Tri.Index];
    
    if (math::Equal(Density, 0.0f))
        Density = DefaultDensity_;
    else
        Density /= Triangles.size();
}

void LightmapGenerator::SFace::updateVertexProjection()
{
    dim::point2di TexCoord;
    
    dim::point2di Min = 99999;
    dim::point2di Max = -99999;
    
    s32 i;
    
    // Compute the vertices' lightmap-texture-coordinates and the faces' lightmap space's bounding box
    for (std::list<STriangle>::iterator it = Triangles.begin(); it != Triangles.end(); ++it)
    {
        for (i = 0; i < 3; ++i)
        {
            TexCoord = (STriangle::getProjection(it->Vertices[i].Position, it->Plane.Normal, Density) + 0.5).cast<s32>();
            
            if (TexCoord.X < Min.X) Min.X = TexCoord.X;
            if (TexCoord.Y < Min.Y) Min.Y = TexCoord.Y;
            
            if (TexCoord.X > Max.X) Max.X = TexCoord.X;
            if (TexCoord.Y > Max.Y) Max.Y = TexCoord.Y;
            
            it->Vertices[i].LMapCoord = TexCoord;
        }
    }
    
    // Fit the lightmap-texture-coordinates to the bounding box
    for (std::list<STriangle>::iterator it = Triangles.begin(); it != Triangles.end(); ++it)
    {
        it->Vertices[0].LMapCoord -= Min;
        it->Vertices[1].LMapCoord -= Min;
        it->Vertices[2].LMapCoord -= Min;
    }
    
    Size.Width  = Max.X - Min.X;
    Size.Height = Max.Y - Min.Y;
    
    // Resize the face if to big for lightmap
    if (Size.Width > LightmapSize_.Width - 2 || Size.Height > LightmapSize_.Height - 2)
        resizeVertexProjection(getAspectRatio(LightmapSize_ - 2));
}

void LightmapGenerator::SFace::resizeVertexProjection(const dim::size2di &NewSize)
{
    const dim::size2df Scale(
        static_cast<f32>(NewSize.Width) / Size.Width,
        static_cast<f32>(NewSize.Height) / Size.Height
    );
    
    for (std::list<STriangle>::iterator it = Triangles.begin(); it != Triangles.end(); ++it)
    {
        it->Vertices[0].scaleProj(Scale);
        it->Vertices[1].scaleProj(Scale);
        it->Vertices[2].scaleProj(Scale);
    }
    
    Size = NewSize;
}

// !INCOMPLETE AND SLOW!
bool LightmapGenerator::SFace::adjacency(const SFace &OpFace) const
{
    foreach (const STriangle &TriA, Triangles)
    {
        foreach (const STriangle &TriB, OpFace.Triangles)
        {
            if (TriA.adjacency(TriB))
                return true;
        }
    }
    
    return false;
}

void LightmapGenerator::SFace::build(scene::Mesh* Mesh)
{
    u32 i, j, c = 0;
    u32 VertexIndex;
    
    scene::Mesh* SourceMesh = Axis->Model->Mesh;
    video::MeshBuffer* OldSurface = SourceMesh->getMeshBuffer(Surface);
    
    const u32 TextureCount = math::Min(
        static_cast<u32>(OldSurface->getVertexFormat()->getTexCoords().size() - 1),
        OldSurface->getTextureCount()
    );
    
    // Create the face with its vertices & triangles
    video::MeshBuffer* NewSurface = Mesh->createMeshBuffer(
        OldSurface->getVertexFormat(), OldSurface->getIndexFormat()->getDataType()
    );
    
    foreach (STriangle &Tri, Triangles)
    {
        for (i = 0; i < 3; ++i)
        {
            VertexIndex = NewSurface->addVertex(
                Tri.Vertices[i].Position,
                Tri.Vertices[i].Normal,
                Tri.Vertices[i].getMapCoord(),
                Tri.Vertices[i].Color,
                Tri.Vertices[i].Fog
            );
            
            for (j = 0; j < TextureCount; ++j)
                NewSurface->setVertexTexCoord(VertexIndex, Tri.Vertices[i].TexCoord[j], j);
        }
        
        NewSurface->addTriangle(c, c + 1, c + 2);
        c += 3;
    }
    
    // Add the original textures
    for (i = 0; i < TextureCount; ++i)
    {
        NewSurface->addTexture(OldSurface->getTexture(i));
        NewSurface->setSurfaceTexture(i, OldSurface->getSurfaceTexture(i));
    }
    
    // Add the lightmap texture
    NewSurface->addTexture(RootLightmap->Texture);
}

dim::size2di LightmapGenerator::SFace::getAspectRatio(const dim::size2di &MaxSize) const
{
    if (Size < MaxSize)
        return Size;
    
    f32 Scale = 1.0f;
    
    if (Size.Width - MaxSize.Width > Size.Height - MaxSize.Height)
        Scale = static_cast<f32>(MaxSize.Width) / Size.Width;
    else
        Scale = static_cast<f32>(MaxSize.Height) / Size.Height;
    
    return dim::size2di(
        math::Min(static_cast<s32>(Scale * Size.Width), MaxSize.Width),
        math::Min(static_cast<s32>(Scale * Size.Height), MaxSize.Width)
    );
}


/*
 * SAxisData structure
 */

LightmapGenerator::SAxisData::SAxisData() :
    Model(0)
{
}
LightmapGenerator::SAxisData::~SAxisData()
{
    foreach (SFace &Face, Faces)
        MemoryManager::deleteMemory(Face.Lightmap);
}

// !THE NEXT TWO FUNCTIONS (createFaces & optimizeFaces) ARE NOT OPTIMIZED AND VERY SLOW!

void LightmapGenerator::SAxisData::createFaces()
{
    // Temporary variables
    bool FoundList;
    std::list<STriangle>::iterator ita, itb;
    std::list<SFace>::iterator itFace;
    
    // Create the faces and insert the triangles
    for (ita = Triangles.begin(); ita != Triangles.end() && processRunning(); ++ita)
    {
        FoundList = false;
        
        // Loop for each face and check if the current triangle is adjacency to the face's triangle list
        for (itFace = Faces.begin(); itFace != Faces.end() && !FoundList; ++itFace)
        {
            for (itb = itFace->Triangles.begin(); itb != itFace->Triangles.end(); ++itb)
            {
                // If adjacency add the current triangle (ita) to the face's triangle list
                if (ita->adjacency(*itb))
                {
                    itFace->Triangles.push_back(*ita);
                    FoundList = true;
                    break;
                }
            }
        }
        
        // If the triangle was not adjacency to any of the faces' triangle list create a new face
        if (!FoundList)
        {
            SFace NewFace(this);
            NewFace.Triangles.push_back(*ita);
            Faces.push_back(NewFace);
        }
    }
}

void LightmapGenerator::SAxisData::optimizeFaces()
{
    // Temporary variables
    std::list<SFace>::iterator ita, itb;
    std::list<STriangle>::iterator itTri;
    
    // Summarize the faces as far as possible
    for (ita = Faces.begin(); ita != Faces.end(); ++ita)
    {
        for (itb = Faces.begin(); itb != Faces.end();)
        {
            if (ita != itb && ita->adjacency(*itb))
            {
                // Add the triangle to the first list
                for (itTri = itb->Triangles.begin(); itTri != itb->Triangles.end(); ++itTri)
                    ita->Triangles.push_back(*itTri);
                
                // Delete the old list
                itb = Faces.erase(itb);
            }
            else
                ++itb;
        }
    }
}

void LightmapGenerator::SAxisData::completeFaces()
{
    // Complete each face in the axis (average vertex projection)
    foreach (SFace &Face, Faces)
    {
        Face.computeDensityAverage();
        Face.updateVertexProjection();
        
        if (!Face.Triangles.empty())
            Face.Surface = Face.Triangles.begin()->Surface;
        
        // Complete the triangles' face connection
        foreach (STriangle &Tri, Face.Triangles)
            Tri.Face = &Face;
    }
}

LightmapGenerator::EAxisTypes LightmapGenerator::SAxisData::getAxisType(const dim::vector3df &Normal)
{
    const dim::vector3df AbsNormal(Normal.getAbs());
    
    if (AbsNormal.X >= AbsNormal.Y && AbsNormal.X >= AbsNormal.Z)
        return (Normal.X > 0 ? AXIS_X_POSITIVE : AXIS_X_NEGATIVE);
    else if (AbsNormal.Y >= AbsNormal.X && AbsNormal.Y >= AbsNormal.Z)
        return (Normal.Y > 0 ? AXIS_Y_POSITIVE : AXIS_Y_NEGATIVE);
    
    return (Normal.Z > 0 ? AXIS_Z_POSITIVE : AXIS_Z_NEGATIVE);
}


/*
 * SModel structure
 */

LightmapGenerator::SModel::SModel(
    scene::Mesh* ObjMesh, bool DefStayAlone, const std::vector< std::vector<f32> > &InitTrianglesDensity) :
    Mesh            (ObjMesh                ),
    DefaultDensity  (0.1f                   ),
    StayAlone       (DefStayAlone           ),
    TrianglesDensity(InitTrianglesDensity   )
{
    Matrix          = Mesh->getTransformation(true);
    MatrixInv       = Matrix.getInverse();
    NormalMatrix    = Matrix.getRotationMatrix();
    
    for (s32 i = 0; i < 6; ++i)
        Axles[i].Model = this;
    
    Triangles.resize(Mesh->getMeshBufferCount());
    for (u32 s = 0; s < Mesh->getMeshBufferCount(); ++s)
        Triangles[s].resize(Mesh->getMeshBuffer(s)->getTriangleCount());
}
LightmapGenerator::SModel::~SModel()
{
}

void LightmapGenerator::SModel::partitionMesh()
{
    createAxles();
    
    for (s32 i = 0; i < 6 && processRunning(); ++i)
    {
        Axles[i].createFaces();
        Axles[i].optimizeFaces();
        Axles[i].completeFaces();
        linkAxisTriangles(i);
    }
}

void LightmapGenerator::SModel::createAxles()
{
    u32 Indices[3];
    STriangle CurTriangle;
    video::MeshBuffer* Surface = 0;
    
    EAxisTypes AxisType;
    
    for (u32 s = 0, i; s < Mesh->getMeshBufferCount(); ++s)
    {
        Surface = Mesh->getMeshBuffer(s);
        
        for (i = 0; i < Surface->getTriangleCount(); ++i)
        {
            Surface->getTriangleIndices(i, Indices);
            
            CurTriangle = STriangle(this, s, i, Indices);
            AxisType    = SAxisData::getAxisType(CurTriangle.Plane.Normal);
            
            Axles[AxisType].Triangles.push_back(CurTriangle);
        }
    }
}

void LightmapGenerator::SModel::linkAxisTriangles(const s32 Axis)
{
    foreach (SFace &Face, Axles[Axis].Faces)
    {
        foreach (STriangle &Tri, Face.Triangles)
            (Triangles[Tri.Surface])[Tri.Index] = &Tri;
    }
}

void LightmapGenerator::SModel::buildFaces(scene::Mesh* Mesh)
{
    for (s32 i = 0; i < 6; ++i)
    {
        foreach (SFace &Face, Axles[i].Faces)
            Face.build(Mesh);
    }
}

void LightmapGenerator::SModel::blurLightmapTexels(const s32 Factor)
{
    for (s32 i = 0; i < 6; ++i)
    {
        foreach (SFace &Face, Axles[i].Faces)
        {
            foreach (STriangle &Tri, Face.Triangles)
                Tri.blurTexels(Factor);
        }
    }
}


/*
 * SLightmapTexel strucuture
 */

LightmapGenerator::SLightmapTexel::SLightmapTexel() :
    Color       (0),
    OrigColor   (0),
    Face        (0)
{
}
LightmapGenerator::SLightmapTexel::~SLightmapTexel()
{
}


/*
 * SLightmap strucutre
 */

LightmapGenerator::SLightmap::SLightmap(const dim::size2di &ImageSize, bool UseTexelBuffer) :
    Size        (ImageSize  ),
    TexelBuffer (0          ),
    Texture     (0          ),
    RectNode    (0          )
{
    if (UseTexelBuffer)
    {
        TexelBuffer = MemoryManager::createBuffer<SLightmapTexel>(
            Size.Width * Size.Height, "LightmapGenerator::SLightmap::TexelBuffer"
        );
    }
}
LightmapGenerator::SLightmap::~SLightmap()
{
    MemoryManager::deleteBuffer(TexelBuffer);
}

video::Texture* LightmapGenerator::SLightmap::createTexture()
{
    if (!TexelBuffer)
        return 0;
    
    if (Texture)
        __spVideoDriver->deleteTexture(Texture);
    
    const s32 ImageBufferSize = Size.Width * Size.Height * 3;
    
    u8* ImageBuffer = MemoryManager::createBuffer<u8>(
        ImageBufferSize, "LightmapGenerator::SLightmap -> ImageBuffer"
    );
    
    for (s32 i = 0, j = 0; i < ImageBufferSize; i += 3, ++j)
    {
        ImageBuffer[i + 0] = math::MinMax(static_cast<s32>(AmbientColor_.Red  ) + TexelBuffer[j].Color.Red  , 0, 255);
        ImageBuffer[i + 1] = math::MinMax(static_cast<s32>(AmbientColor_.Green) + TexelBuffer[j].Color.Green, 0, 255);
        ImageBuffer[i + 2] = math::MinMax(static_cast<s32>(AmbientColor_.Blue ) + TexelBuffer[j].Color.Blue , 0, 255);
    }
    
    Texture = __spVideoDriver->createTexture(Size, video::PIXELFORMAT_RGB, ImageBuffer);
    
    MemoryManager::deleteBuffer(ImageBuffer);
    
    return Texture;
}

void LightmapGenerator::SLightmap::copyImageBuffers()
{
    if (TexelBuffer)
    {
        for (s32 i = 0, c = Size.Width * Size.Height; i < c; ++i)
            TexelBuffer[i].OrigColor = TexelBuffer[i].Color;
    }
}

void LightmapGenerator::SLightmap::reduceBleeding()
{
    if (!TexelBuffer)
        return;
    
    s32 x, y;
    
    SLightmapTexel* Texel = TexelBuffer;
    
    for (y = 0; y < Size.Height; ++y)
    {
        for (x = 0; x < Size.Width; ++x, ++Texel)
        {
            if (!Texel->Face)
                Texel->Color = video::color(getAverageColor(x, y), false);
        }
    }
}

dim::point2df LightmapGenerator::SLightmap::getTexCoord(const dim::point2di &RealPos) const
{
    return dim::point2df(
        static_cast<f32>(RealPos.X) / Size.Width,
        static_cast<f32>(RealPos.Y) / Size.Height
    );
}

dim::vector3df LightmapGenerator::SLightmap::getAverageColor(s32 X, s32 Y) const
{
    s32 c = 0;
    dim::vector3df Color;
    
    getAverageColorPart(X - 1, Y - 1, Color, c);
    getAverageColorPart(X    , Y - 1, Color, c);
    getAverageColorPart(X + 1, Y - 1, Color, c);
    
    getAverageColorPart(X - 1, Y    , Color, c);
    getAverageColorPart(X + 1, Y    , Color, c);
    
    getAverageColorPart(X - 1, Y + 1, Color, c);
    getAverageColorPart(X    , Y + 1, Color, c);
    getAverageColorPart(X + 1, Y + 1, Color, c);
    
    return c ? Color / static_cast<f32>(c) : 0.0f;
}

void LightmapGenerator::SLightmap::getAverageColorPart(s32 X, s32 Y, dim::vector3df &Color, s32 &Counter) const
{
    if (X >= 0 && X < Size.Width && Y >= 0 && Y < Size.Height)
    {
        const SLightmapTexel* Texel = &getTexel(X, Y);
        
        if (Texel->Face)
        {
            Color += dim::vector3df(Texel->Color.Red, Texel->Color.Green, Texel->Color.Blue);
            ++Counter;
        }
    }
}


/*
 * SRectNode structure
 */

LightmapGenerator::SRectNode::SRectNode() :
    Lightmap(0)
{
    Child[0] = Child[1] = 0;
}
LightmapGenerator::SRectNode::~SRectNode()
{
    MemoryManager::deleteMemory(Child[0]);
    MemoryManager::deleteMemory(Child[1]);
}

LightmapGenerator::SRectNode* LightmapGenerator::SRectNode::insert(SLightmap* Image)
{
    if (Child[0])
    {
        SRectNode* NewNode = Child[0]->insert(Image);
        
        if (NewNode)
            return NewNode;
        
        return Child[1]->insert(Image);
    }
    else
    {
        if (Lightmap || Image->Size.Width > Rect.getWidth() || Image->Size.Height > Rect.getHeight())
            return 0;
        
        if (Image->Size == Rect.getSize())
        {
            Lightmap = Image;
            Lightmap->RectNode = this;
            return this;
        }
        
        Child[0] = new SRectNode();
        Child[1] = new SRectNode();
        
        if (Rect.getWidth() - Image->Size.Width > Rect.getHeight() - Image->Size.Height)
        {
            Child[0]->Rect = dim::rect2di(Rect.Left, Rect.Top, Rect.Left + Image->Size.Width, Rect.Bottom);
            Child[1]->Rect = dim::rect2di(Rect.Left + Image->Size.Width, Rect.Top, Rect.Right, Rect.Bottom);
        }
        else
        {
            Child[0]->Rect = dim::rect2di(Rect.Left, Rect.Top, Rect.Right, Rect.Top + Image->Size.Height);
            Child[1]->Rect = dim::rect2di(Rect.Left, Rect.Top + Image->Size.Height, Rect.Right, Rect.Bottom);
        }
        
        return Child[0]->insert(Image);
    }
}


/*
 * SLight structure
 */

LightmapGenerator::SLight::SLight(const SLightmapLight &LightData) :
    Type                    (LightData.Type                     ),
    Matrix                  (LightData.Matrix                   ),
    Position                (Matrix.getPosition()               ),
    Color                   (LightData.Color.getVector(true)    ),
    Attn0                   (LightData.Attn0                    ),
    Attn1                   (LightData.Attn1                    ),
    Attn2                   (LightData.Attn2                    ),
    InnerConeAngle          (LightData.InnerConeAngle / math::PI),
    OuterConeAngle          (LightData.OuterConeAngle / math::PI),
    FixedDirection          (
        (Matrix.getRotationMatrix() * dim::vector3df(0, 0, 1)).normalize()
    ),
    FixedVolumetricRadius   (getAttenuationRadius()),
    FixedVolumetric         (
        !math::Equal(Attn0, 1.0f) || !math::Equal(Attn1, 0.0f) || !math::Equal(Attn2, 0.0f)
    )
{
}
LightmapGenerator::SLight::~SLight()
{
}

f32 LightmapGenerator::SLight::getIntensity(
    const dim::vector3df &Point, const dim::vector3df &Normal) const
{
    /* Directional light calculations */
    if (Type == scene::LIGHT_DIRECTIONAL)
        return math::Max(0.0f, -FixedDirection.dot(Normal));
    
    /* General light calculations */
    f32 Intensity = 0.0f;
    
    const dim::vector3df RayDirection((Point - Position).normalize());
    
    Intensity = -RayDirection.dot(Normal);
    
    /* Volumetric light calculations */
    if (FixedVolumetric)
    {
        const f32 Distance = math::getDistance(Position, Point);
        
        Intensity /= ( Attn0 + Attn1*Distance + Attn2*Distance*Distance );
    }
    
    /* Spot light calculations */
    if (Type == scene::LIGHT_SPOT)
    {
        const f32 Angle = RayDirection.getAngle(FixedDirection);
        
        if (Angle > OuterConeAngle)
            Intensity = 0.0f;
        else if (Angle > InnerConeAngle)
        {
            /* Calculate splot-light cone angle linear-interpolation (lerp) */
            const f32 ConeAngleLerp = (Angle - InnerConeAngle) / (OuterConeAngle - InnerConeAngle);
            Intensity *= (1.0f - ConeAngleLerp);
        }
    }
    
    return math::Max(0.0f, Intensity);
}

f32 LightmapGenerator::SLight::getAttenuationRadius() const
{
    // Last 5 black texel levels (5, 5, 5) to (0, 0, 0) are not rendered -> for optimization
    static const f32 COLOR_PRECISE = 5.0f;
    
    if (!FixedVolumetric || Type == scene::LIGHT_DIRECTIONAL)
        return math::OMEGA;
    
    // Attenuation calculation backwards using the pq-formula
    return -(Attn1/Attn2)/2 + sqrt(math::Pow2((Attn1/Attn2)/2) + (255.0f - COLOR_PRECISE*Attn0)/(COLOR_PRECISE*Attn2));
}

bool LightmapGenerator::SLight::checkVisibility(const STriangle &Triangle) const
{
    return (
        Type == scene::LIGHT_DIRECTIONAL ?
        ( FixedDirection.dot(-Triangle.Plane.Normal) > 0.0f ) :
        ( Triangle.Plane.isPointFrontSide(Position) && ( !FixedVolumetric || Triangle.getDistance(Position) < FixedVolumetricRadius ) )
    );
}


} // /namespace tool

} // /namespace sp


#endif



// ================================================================================
