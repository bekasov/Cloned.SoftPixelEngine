/*
 * Lightmap generator structures file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Framework/Tools/spLightmapGeneratorStructs.hpp"

#ifdef SP_COMPILE_WITH_LIGHTMAPGENERATOR


#include "Framework/Tools/spLightmapGenerator.hpp"
#include "Base/spMathCollisionLibrary.hpp"
#include "Base/spMathRasterizer.hpp"
#include "Base/spSharedObjects.hpp"
#include "Platform/spSoftPixelDeviceOS.hpp"
#include "SceneGraph/spSceneManager.hpp"

#include <boost/foreach.hpp>


namespace sp
{

extern video::RenderSystem* __spVideoDriver;
extern scene::SceneGraph* __spSceneManager;

namespace tool
{


namespace LightmapGen
{

/*
 * SVertex structure
 */

SVertex::SVertex() :
    Surface (0),
    Index   (0)
{
}
SVertex::SVertex(const SModel* Model, const u32 VertexSurface, const u32 VertexIndex) :
    Surface (VertexSurface  ),
    Index   (VertexIndex    )
{
    video::MeshBuffer* Surface = Model->Mesh->getMeshBuffer(VertexSurface);
    
    Position    = Model->Matrix * Surface->getVertexCoord(Index);
    Normal      = Model->NormalMatrix * Surface->getVertexNormal(Index);
    Color       = Surface->getVertexColor(Index);
    
    Normal.normalize();
    
    for (u32 i = 0; i < MAX_COUNT_OF_TEXTURES && i < Surface->getTextureCount(); ++i)
        TexCoord[i] = Surface->getVertexTexCoord(Index, i);
}
SVertex::~SVertex()
{
}

bool SVertex::adjacency(const STriangle &OpTriangle) const
{
    return
        Position.equal(OpTriangle.Vertices[0].Position) ||
        Position.equal(OpTriangle.Vertices[1].Position) ||
        Position.equal(OpTriangle.Vertices[2].Position);
}

void SVertex::scaleProj(const dim::size2df &Scale)
{
    LMapCoord.X = static_cast<s32>(Scale.Width * LMapCoord.X);
    LMapCoord.Y = static_cast<s32>(Scale.Height * LMapCoord.Y);
}

dim::point2df SVertex::getMapCoord(const dim::size2di &MaxLightmapSize) const
{
    return dim::point2df(
        static_cast<f32>(LMapCoord.X) / MaxLightmapSize.Width,
        static_cast<f32>(LMapCoord.Y) / MaxLightmapSize.Height
    );
}

dim::vector3df SVertex::getVectorColor(const video::color &Color)
{
    return dim::vector3df(Color.Red, Color.Green, Color.Blue) / 255;
}


/*
 * STriangle structure
 */

STriangle::STriangle() :
    Surface (0),
    Index   (0),
    Face    (0)
{
}
STriangle::STriangle(
    const SModel* Model, u32 TriangleSurface, u32 TriangleIndex, u32 DefIndices[3]) :
    Surface (TriangleSurface),
    Index   (TriangleIndex  ),
    Face    (0              )
{
    for (s32 i = 0; i < 3; ++i)
        Vertices[i] = SVertex(Model, TriangleSurface, DefIndices[i]);
    
    Plane = dim::plane3df(Vertices[0].Position, Vertices[1].Position, Vertices[2].Position);
}
STriangle::~STriangle()
{
}

bool STriangle::adjacency(const STriangle &OpTriangle) const
{
    if (Surface != OpTriangle.Surface)
        return false;
    
    return
        Vertices[0].adjacency(OpTriangle) ||
        Vertices[1].adjacency(OpTriangle) ||
        Vertices[2].adjacency(OpTriangle);
}

f32 STriangle::getDistance(const dim::vector3df &Point) const
{
    return math::getDistance(
        math::CollisionLibrary::getClosestPoint(
            dim::triangle3df(Vertices[0].Position, Vertices[1].Position, Vertices[2].Position), Point
        ),
        Point
    );
}

dim::point2df STriangle::getProjection(
    const dim::vector3df &Point, const dim::vector3df &Normal, const f32 Density)
{
    const dim::vector3df AbsNormal(Normal.getAbs());
    
    if (AbsNormal.X >= AbsNormal.Y && AbsNormal.X >= AbsNormal.Z)
        return dim::point2df(Point.Z, -Point.Y) * Density;
    else if (AbsNormal.Y >= AbsNormal.X && AbsNormal.Y >= AbsNormal.Z)
        return dim::point2df(Point.X, -Point.Z) * Density;
    
    return dim::point2df(Point.X, -Point.Y) * Density;
}

//!TODO! -> replace this with a technique using barycentric coordinates!!!
void STriangle::computeInterpolation(
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

SFace::SFace(SAxisData* FaceAxis) :
    Density     (0.1f       ),
    Surface     (0          ),
    Lightmap    (0          ),
    RootLightmap(0          ),
    Axis        (FaceAxis   )
{
}
SFace::~SFace()
{
}

void SFace::computeDensityAverage(f32 DefaultDensity)
{
    if (!Triangles.empty())
    {
        Density = 0.0f;
        
        foreach (const STriangle &Tri, Triangles)
            Density += (Axis->Model->TrianglesDensity[Tri.Surface])[Tri.Index];
        
        if (math::Equal(Density, 0.0f))
            Density = DefaultDensity;
        else
            Density /= Triangles.size();
    }
}

void SFace::updateVertexProjection(const dim::size2di &MaxLightmapSize)
{
    dim::point2di TexCoord;
    
    dim::point2di Min = 999999;
    dim::point2di Max = -999999;
    
    // Compute the vertices' lightmap-texture-coordinates and the faces' lightmap space's bounding box
    foreach (STriangle &Tri, Triangles)
    {
        for (s32 i = 0; i < 3; ++i)
        {
            TexCoord = (STriangle::getProjection(Tri.Vertices[i].Position, Tri.Plane.Normal, Density) + 0.5).cast<s32>();
            
            if (TexCoord.X < Min.X) Min.X = TexCoord.X;
            if (TexCoord.Y < Min.Y) Min.Y = TexCoord.Y;
            
            if (TexCoord.X > Max.X) Max.X = TexCoord.X;
            if (TexCoord.Y > Max.Y) Max.Y = TexCoord.Y;
            
            Tri.Vertices[i].LMapCoord = TexCoord;
        }
    }
    
    // Fit the lightmap-texture-coordinates to the bounding box
    foreach (STriangle &Tri, Triangles)
    {
        Tri.Vertices[0].LMapCoord -= Min;
        Tri.Vertices[1].LMapCoord -= Min;
        Tri.Vertices[2].LMapCoord -= Min;
    }
    
    Size.Width  = Max.X - Min.X;
    Size.Height = Max.Y - Min.Y;
    
    // Resize the face if to big for lightmap
    if (Size.Width > MaxLightmapSize.Width - 2 || Size.Height > MaxLightmapSize.Height - 2)
        resizeVertexProjection(Size.getScaledSize(MaxLightmapSize - 2));
}

void SFace::resizeVertexProjection(const dim::size2di &NewSize)
{
    const dim::size2df Scale(
        static_cast<f32>(NewSize.Width) / Size.Width,
        static_cast<f32>(NewSize.Height) / Size.Height
    );
    
    foreach (STriangle &Tri, Triangles)
    {
        Tri.Vertices[0].scaleProj(Scale);
        Tri.Vertices[1].scaleProj(Scale);
        Tri.Vertices[2].scaleProj(Scale);
    }
    
    Size = NewSize;
}

// !INCOMPLETE AND SLOW!
bool SFace::adjacency(const SFace &OpFace) const
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

void SFace::build(scene::Mesh* Mesh, const dim::size2di &MaxLightmapSize)
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
                Tri.Vertices[i].getMapCoord(MaxLightmapSize),
                Tri.Vertices[i].Color
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
        
        video::TextureLayer* NewTexLayer = NewSurface->getTextureLayer(i);
        video::TextureLayer* OldTexLayer = OldSurface->getTextureLayer(i);
        
        if (NewTexLayer && OldTexLayer)
            NewTexLayer->setTexture(OldTexLayer->getTexture());
    }
    
    // Add the lightmap texture
    NewSurface->addTexture(RootLightmap->Texture);
}


/*
 * SAxisData structure
 */

SAxisData::SAxisData() :
    Model(0)
{
}
SAxisData::~SAxisData()
{
    foreach (SFace &Face, Faces)
        MemoryManager::deleteMemory(Face.Lightmap);
}

// !THE NEXT TWO FUNCTIONS (createFaces & optimizeFaces) ARE NOT OPTIMIZED AND VERY SLOW!
void SAxisData::createFaces()
{
    // Temporary variables
    bool FoundList;
    std::list<STriangle>::iterator ita, itb;
    std::list<SFace>::iterator itFace;
    
    // Create the faces and insert the triangles
    for (ita = Triangles.begin(); ita != Triangles.end() && LightmapGenerator::processRunning(); ++ita)
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

void SAxisData::optimizeFaces()
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

void SAxisData::completeFaces(const dim::size2di &MaxLightmapSize, f32 DefaultDensity)
{
    // Complete each face in the axis (average vertex projection)
    foreach (SFace &Face, Faces)
    {
        Face.computeDensityAverage(DefaultDensity);
        Face.updateVertexProjection(MaxLightmapSize);
        
        if (!Face.Triangles.empty())
            Face.Surface = Face.Triangles.begin()->Surface;
        
        // Complete the triangles' face connection
        foreach (STriangle &Tri, Face.Triangles)
            Tri.Face = &Face;
    }
}


/*
 * SModel structure
 */

SModel::SModel(
    scene::Mesh* ObjMesh, bool DefStayAlone, const std::vector< std::vector<f32> > &InitTrianglesDensity) :
    Mesh            (ObjMesh                        ),
    DefaultDensity  (0.1f                           ),
    Matrix          (Mesh->getTransformMatrix(true) ),
    MatrixInv       (Matrix.getInverse()            ),
    NormalMatrix    (Matrix.getRotationMatrix()     ),
    StayAlone       (DefStayAlone                   ),
    TrianglesDensity(InitTrianglesDensity           )
{
    for (s32 i = 0; i < 6; ++i)
        Axles[i].Model = this;
    
    Triangles.resize(Mesh->getMeshBufferCount());
    for (u32 s = 0; s < Mesh->getMeshBufferCount(); ++s)
        Triangles[s].resize(Mesh->getMeshBuffer(s)->getTriangleCount());
}
SModel::~SModel()
{
}

void SModel::partitionMesh(const dim::size2di &MaxLightmapSize, f32 DefaultDensity)
{
    createAxles();
    
    for (s32 i = 0; i < 6 && LightmapGenerator::processRunning(); ++i)
    {
        Axles[i].createFaces();
        Axles[i].optimizeFaces();
        Axles[i].completeFaces(MaxLightmapSize, DefaultDensity);
        linkAxisTriangles(i);
    }
}

void SModel::createAxles()
{
    u32 Indices[3] = { 0 };
    STriangle CurTriangle;
    video::MeshBuffer* Surface = 0;
    
    dim::EAxisTypes AxisType;
    
    for (u32 s = 0, i; s < Mesh->getMeshBufferCount(); ++s)
    {
        Surface = Mesh->getMeshBuffer(s);
        
        for (i = 0; i < Surface->getTriangleCount(); ++i)
        {
            Surface->getTriangleIndices(i, Indices);
            
            CurTriangle = STriangle(this, s, i, Indices);
            AxisType    = CurTriangle.Plane.Normal.getAxisType();
            
            Axles[AxisType].Triangles.push_back(CurTriangle);
        }
    }
}

void SModel::linkAxisTriangles(const s32 Axis)
{
    foreach (SFace &Face, Axles[Axis].Faces)
    {
        foreach (STriangle &Tri, Face.Triangles)
            (Triangles[Tri.Surface])[Tri.Index] = &Tri;
    }
}

void SModel::buildFaces(scene::Mesh* Mesh, const dim::size2di &MaxLightmapSize)
{
    for (s32 i = 0; i < 6; ++i)
    {
        foreach (SFace &Face, Axles[i].Faces)
            Face.build(Mesh, MaxLightmapSize);
    }
}


/*
 * SLightmapTexel strucuture
 */

SLightmapTexel::SLightmapTexel() :
    Color       (0),
    OrigColor   (0),
    Face        (0)
{
}
SLightmapTexel::~SLightmapTexel()
{
}


/*
 * SLightmap strucutre
 */

SLightmap::SLightmap(const dim::size2di &ImageSize, bool UseTexelBuffer) :
    Size        (ImageSize  ),
    TexelBuffer (0          ),
    Texture     (0          ),
    RectNode    (0          )
{
    if (UseTexelBuffer)
    {
        TexelBuffer = MemoryManager::createBuffer<SLightmapTexel>(
            Size.Width * Size.Height, "SLightmap::TexelBuffer"
        );
    }
}
SLightmap::~SLightmap()
{
    delete [] TexelBuffer;
}

video::Texture* SLightmap::createTexture(const video::color &AmbientColor)
{
    if (!TexelBuffer)
        return 0;
    
    // Create texture if not already done
    if (!Texture)
    {
        Texture = __spVideoDriver->createTexture(Size, video::PIXELFORMAT_RGB);
        Texture->setFilename("Lightmap");
    }
    else
        Texture->setSize(Size);
    
    // Create temporary image buffer
    const s32 FormatSize = Texture->getImageBuffer()->getFormatSize();
    const s32 ImageBufferSize = Size.Width * Size.Height * FormatSize;
    
    u8* ImageBuffer = MemoryManager::createBuffer<u8>(
        ImageBufferSize, "SLightmap -> ImageBuffer"
    );
    
    const s32 AmbColor[3] = { AmbientColor.Red, AmbientColor.Green, AmbientColor.Blue };
    
    for (s32 i = 0, j = 0; i < ImageBufferSize; i += FormatSize, ++j)
    {
        ImageBuffer[i + 0] = math::MinMax(AmbColor[0] + TexelBuffer[j].Color.Red  , 0, 255);
        ImageBuffer[i + 1] = math::MinMax(AmbColor[1] + TexelBuffer[j].Color.Green, 0, 255);
        ImageBuffer[i + 2] = math::MinMax(AmbColor[2] + TexelBuffer[j].Color.Blue , 0, 255);
        if (FormatSize == 4)
            ImageBuffer[i + 3] = 255;
    }
    
    // Update texture's image buffer
    Texture->setupImageBuffer(ImageBuffer);
    
    // Delete temporary image buffer
    MemoryManager::deleteBuffer(ImageBuffer);
    
    return Texture;
}

void SLightmap::copyImageBuffers()
{
    if (TexelBuffer)
    {
        for (s32 i = 0, c = Size.Width * Size.Height; i < c; ++i)
            TexelBuffer[i].OrigColor = TexelBuffer[i].Color;
    }
}

void SLightmap::reduceBleeding()
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

dim::point2df SLightmap::getTexCoord(const dim::point2di &RealPos) const
{
    return dim::point2df(
        static_cast<f32>(RealPos.X) / Size.Width,
        static_cast<f32>(RealPos.Y) / Size.Height
    );
}

dim::vector3df SLightmap::getAverageColor(s32 X, s32 Y) const
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

void SLightmap::getAverageColorPart(s32 X, s32 Y, dim::vector3df &Color, s32 &Counter) const
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
 * SLight structure
 */

SLight::SLight(const SLightmapLight &LightData) :
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
SLight::~SLight()
{
}

f32 SLight::getIntensity(
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

f32 SLight::getAttenuationRadius() const
{
    // Last 5 black texel levels (5, 5, 5) to (0, 0, 0) are not rendered -> for optimization
    static const f32 COLOR_PRECISE = 5.0f;
    
    if (!FixedVolumetric || Type == scene::LIGHT_DIRECTIONAL)
        return math::OMEGA;
    
    // Attenuation calculation backwards using the pq-formula
    return -(Attn1/Attn2)/2 + sqrt(math::Pow2((Attn1/Attn2)/2) + (255.0f - COLOR_PRECISE*Attn0)/(COLOR_PRECISE*Attn2));
}

bool SLight::checkVisibility(const STriangle &Triangle) const
{
    return (
        Type == scene::LIGHT_DIRECTIONAL ?
        ( FixedDirection.dot(-Triangle.Plane.Normal) > 0.0f ) :
        ( Triangle.Plane.isPointFrontSide(Position) && ( !FixedVolumetric || Triangle.getDistance(Position) < FixedVolumetricRadius ) )
    );
}


/*
 * SRasterizerVertex structure
 */

SRasterizerVertex::SRasterizerVertex()
{
}
SRasterizerVertex::SRasterizerVertex(
    const dim::vector3df &InitPosition,
    const dim::vector3df &InitNormal,
    const dim::point2di &InitScreenCoord) :
    Position    (InitPosition   ),
    Normal      (InitNormal     ),
    ScreenCoord (InitScreenCoord)
{
}
SRasterizerVertex::~SRasterizerVertex()
{
}

SRasterizerVertex& SRasterizerVertex::operator = (const SRasterizerVertex &Other)
{
    Position = Other.Position;
    Normal = Other.Normal;
    return *this;
}

SRasterizerVertex& SRasterizerVertex::operator += (const SRasterizerVertex &Other)
{
    Position += Other.Position;
    Normal += Other.Normal;
    return *this;
}
SRasterizerVertex& SRasterizerVertex::operator -= (const SRasterizerVertex &Other)
{
    Position -= Other.Position;
    Normal -= Other.Normal;
    return *this;
}

SRasterizerVertex& SRasterizerVertex::operator *= (f32 Factor)
{
    Position *= Factor;
    Normal *= Factor;
    return *this;
}
SRasterizerVertex& SRasterizerVertex::operator /= (f32 Factor)
{
    Position /= Factor;
    Normal /= Factor;
    return *this;
}

s32 SRasterizerVertex::getScreenCoordX() const
{
    return ScreenCoord.X;
}
s32 SRasterizerVertex::getScreenCoordY() const
{
    return ScreenCoord.Y;
}

} // /namespace LightmapGen


} // /namespace tool

} // /namespace sp


#endif



// ================================================================================
