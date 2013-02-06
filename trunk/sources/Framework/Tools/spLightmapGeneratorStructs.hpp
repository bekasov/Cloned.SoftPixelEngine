/*
 * Lightmap generator structures header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_LIGHTMAP_GENERATOR_STRUCTS_H__
#define __SP_LIGHTMAP_GENERATOR_STRUCTS_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_LIGHTMAPGENERATOR


#include "Base/spDimension.hpp"
#include "Base/spTreeNodeImage.hpp"
#include "SceneGraph/spSceneGraph.hpp"
#include "RenderSystem/spRenderSystem.hpp"
#include "Framework/Tools/spLightmapBase.hpp"

#include <list>
#include <vector>


namespace sp
{
namespace tool
{


//! Namespace with all data structures for lightmap generation.
namespace LightmapGen
{

/* === Declerations === */

struct SVertex;
struct STriangle;
struct SFace;
struct SLightmap;
struct SModel;
struct SAxisData;

/* === Structures === */

struct SVertex
{
    SVertex();
    SVertex(const SModel* Model, const u32 VertexSurface, const u32 VertexIndex);
    ~SVertex();
    
    /* Functions */
    bool adjacency(const STriangle &OpTriangle) const;
    
    void scaleProj(const dim::size2df &Scale);
    dim::point2df getMapCoord(const dim::size2di &MaxLightmapSize) const;
    
    /* Static functions */
    static dim::vector3df getVectorColor(const video::color &Color);
    
    /* Members */
    u32 Surface, Index;
    
    dim::vector3df Position;
    dim::vector3df Normal;
    dim::point2df TexCoord[MAX_COUNT_OF_TEXTURES];
    dim::point2di LMapCoord;
    video::color Color;
};

struct STriangle
{
    STriangle();
    STriangle(const SModel* Model, u32 TriangleSurface, u32 TriangleIndex, u32 DefIndices[3]);
    ~STriangle();
    
    /* Functions */
    bool adjacency(const STriangle &OpTriangle) const;
    f32 getDistance(const dim::vector3df &Point) const;
    
    /* Static functions */
    static dim::point2df getProjection(
        const dim::vector3df &Point, const dim::vector3df &Normal, const f32 Density
    );
    
    static void computeInterpolation(
        const scene::SIntersectionContact &Contact, u32 Indices[3], const u8 Layer,
        dim::point2df &TexCoord, dim::vector3df &Color, f32 &Alpha
    );
    
    /* Members */
    u32 Surface, Index;
    SVertex Vertices[3];
    dim::plane3df Plane;
    SFace* Face;
};

struct SFace
{
    SFace(SAxisData* FaceAxis);
    ~SFace();
    
    /* Functions */
    void computeDensityAverage(f32 DefaultDensity);
    void updateVertexProjection(const dim::size2di &MaxLightmapSize);
    void resizeVertexProjection(const dim::size2di &NewSize);
    
    //! \todo This is incomplete and very slow!
    bool adjacency(const SFace &OpFace) const;
    
    void build(scene::Mesh* Mesh, const dim::size2di &MaxLightmapSize);
    
    /* Members */
    f32 Density;
    u32 Surface;
    dim::size2di Size;              // Size of the area used in the lightmap texture
    std::list<STriangle> Triangles; // Adjacency triangle list
    SLightmap* Lightmap;
    SLightmap* RootLightmap;
    SAxisData* Axis;
};

struct SAxisData
{
    SAxisData();
    ~SAxisData();
    
    /* Functions */
    void createFaces();
    void optimizeFaces();
    void completeFaces(const dim::size2di &MaxLightmapSize, f32 DefaultDensity);
    
    /* Members */
    std::list<SFace> Faces;         // Each face has a list with adjacency triangles
    std::list<STriangle> Triangles; // Complete triangle list
    SModel* Model;
};

struct SModel
{
    SModel(scene::Mesh* ObjMesh, bool DefStayAlone, const std::vector< std::vector<f32> > &InitTrianglesDensity);
    ~SModel();
    
    /* Functions */
    void partitionMesh(const dim::size2di &MaxLightmapSize, f32 DefaultDensity);
    void createAxles();
    void linkAxisTriangles(const s32 Axis);
    void buildFaces(scene::Mesh* Mesh, const dim::size2di &MaxLightmapSize);
    
    /* Members */
    scene::Mesh* Mesh;
    SAxisData Axles[6];
    
    f32 DefaultDensity;
    dim::matrix4f Matrix, MatrixInv, NormalMatrix;
    
    bool StayAlone;
    
    std::vector< std::vector<f32> > TrianglesDensity;
    std::vector< std::vector<STriangle*> > Triangles;
};

struct SLightmapTexel
{
    SLightmapTexel();
    ~SLightmapTexel();
    
    /* Members */
    video::color Color, OrigColor;
    
    const SFace* Face; // Face to which the texel belongs
};

struct SLightmap
{
    SLightmap(const dim::size2di &ImageSize, bool UseTexelBuffer = true);
    ~SLightmap();
    
    /* Functions */
    video::Texture* createTexture(const video::color &AmbientColor);
    void copyImageBuffers();
    void reduceBleeding();
    
    dim::point2df getTexCoord(const dim::point2di &RealPos) const;
    
    dim::vector3df getAverageColor(s32 X, s32 Y) const;
    void getAverageColorPart(s32 X, s32 Y, dim::vector3df &Color, s32 &Counter) const;
    
    /* Inline functions */
    inline SLightmapTexel& getTexel(s32 X, s32 Y)
    {
        return TexelBuffer[Y * Size.Width + X];
    }
    inline const SLightmapTexel& getTexel(s32 X, s32 Y) const
    {
        return TexelBuffer[Y * Size.Width + X];
    }
    inline dim::size2di getSize() const
    {
        return Size;
    }
    inline void setupTreeNode(TRectNode* Node)
    {
        RectNode = Node;
    }
    
    /* Members */
    dim::size2di Size;
    SLightmapTexel* TexelBuffer;
    video::Texture* Texture;
    TRectNode* RectNode;
};

struct SLight
{
    SLight(const SLightmapLight &LightData);
    ~SLight();
    
    /* Functions */
    f32 getIntensity(const dim::vector3df &Point, const dim::vector3df &Normal) const;
    f32 getAttenuationRadius() const;
    
    bool checkVisibility(const STriangle &Triangle) const;
    
    /* Members */
    scene::ELightModels Type;
    
    dim::matrix4f Matrix;
    dim::vector3df Position;
    dim::vector3df Color;
    
    f32 Attn0, Attn1, Attn2;
    f32 InnerConeAngle, OuterConeAngle;
    
    dim::vector3df FixedDirection;
    f32 FixedVolumetricRadius;
    bool FixedVolumetric;
};

struct SRasterizerVertex
{
    SRasterizerVertex();
    SRasterizerVertex(
        const dim::vector3df &InitPosition,
        const dim::vector3df &InitNormal,
        const dim::point2di &InitScreenCoord
    );
    ~SRasterizerVertex();
    
    /* Operators */
    SRasterizerVertex& operator = (const SRasterizerVertex &Other);
    
    SRasterizerVertex& operator += (const SRasterizerVertex &Other);
    SRasterizerVertex& operator -= (const SRasterizerVertex &Other);
    
    SRasterizerVertex& operator *= (f32 Factor);
    SRasterizerVertex& operator /= (f32 Factor);
    
    /* Functions */
    s32 getScreenCoordX() const;
    s32 getScreenCoordY() const;
    
    /* Members */
    dim::vector3df Position;
    dim::vector3df Normal;
    dim::point2di ScreenCoord;
};

} // /namespace LightmapGen


} // /namespace tool

} // /namespace sp


#endif

#endif



// ================================================================================
