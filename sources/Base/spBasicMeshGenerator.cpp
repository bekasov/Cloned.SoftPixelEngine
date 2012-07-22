/*
 * Basic mesh generator file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Base/spBasicMeshGenerator.hpp"
#include "Base/spImageBuffer.hpp"
#include "SceneGraph/spSceneGraph.hpp"

#include <boost/foreach.hpp>
#include <boost/shared_array.hpp>


namespace sp
{
namespace scene
{


/*
 * SMeshConstruct strucutre
 */

SMeshConstruct::SMeshConstruct() :
    SegmentsVert    (-1                     ),
    SegmentsHorz    (-1                     ),
    RadiusInner     (0.5f                   ),
    RadiusOuter     (0.25f                  ),
    HasCap          (true                   ),
    Shading         (video::SHADING_GOURAUD ),
    RotationDegree  (360.0f                 ),
    RotationDistance(1.0f                   )
{
}
SMeshConstruct::SMeshConstruct(
    s32 MeshSegments, f32 MeshRadius, bool MeshHasCap, const video::EShadingTypes DefShading) :
    SegmentsVert    (MeshSegments   ),
    SegmentsHorz    (MeshSegments   ),
    RadiusInner     (MeshRadius     ),
    RadiusOuter     (MeshRadius / 2 ),
    HasCap          (MeshHasCap     ),
    Shading         (DefShading     ),
    RotationDegree  (360.0f         ),
    RotationDistance(1.0f           )
{
}
SMeshConstruct::SMeshConstruct(
    s32 MeshSegments, f32 MeshRadiusInner, f32 MeshRadiusOuter, bool MeshHasCap, const video::EShadingTypes DefShading) :
    SegmentsVert    (MeshSegments   ),
    SegmentsHorz    (MeshSegments   ),
    RadiusInner     (MeshRadiusInner),
    RadiusOuter     (MeshRadiusOuter),
    HasCap          (MeshHasCap     ),
    Shading         (DefShading     ),
    RotationDegree  (360.0f         ),
    RotationDistance(1.0f           )
{
}
SMeshConstruct::SMeshConstruct(
    s32 MeshSegmentsVert, s32 MeshSegmentsHorz, f32 MeshRadiusInner, f32 MeshRadiusOuter,
    bool MeshHasCap, const video::EShadingTypes DefShading) :
    SegmentsVert    (MeshSegmentsVert   ),
    SegmentsHorz    (MeshSegmentsHorz   ),
    RadiusInner     (MeshRadiusInner    ),
    RadiusOuter     (MeshRadiusOuter    ),
    HasCap          (MeshHasCap         ),
    Shading         (DefShading         ),
    RotationDegree  (360.0f             ),
    RotationDistance(1.0f               )
{
}
SMeshConstruct::SMeshConstruct(
    s32 MeshSegmentsVert, s32 MeshSegmentsHorz, f32 MeshRadiusInner, f32 MeshRadiusOuter,
    f32 SpiralRotationDegree, f32 SpiralRotationDistance, bool MeshHasCap, const video::EShadingTypes DefShading) :
    SegmentsVert    (MeshSegmentsVert       ),
    SegmentsHorz    (MeshSegmentsHorz       ),
    RadiusInner     (MeshRadiusInner        ),
    RadiusOuter     (MeshRadiusOuter        ),
    HasCap          (MeshHasCap             ),
    Shading         (DefShading             ),
    RotationDegree  (SpiralRotationDegree   ),
    RotationDistance(SpiralRotationDistance )
{
}
SMeshConstruct::~SMeshConstruct()
{
}

void SMeshConstruct::checkDefaultSegments(const EBasicMeshes Model, s32 &Segments)
{
    if (Segments == -1)
    {
        switch (Model)
        {
            case MESH_ICOSPHERE:
                Segments = 3; break;
            case MESH_TEAPOT:
                Segments = 6; break;
            case MESH_SPHERE:
            case MESH_TORUS:
            case MESH_TORUSKNOT:
            case MESH_SPIRAL:
                Segments = 10; break;
            case MESH_CONE:
            case MESH_CYLINDER:
            case MESH_PIPE:
            case MESH_DISK:
                Segments = 20; break;
            default:
                Segments = 1; break;
        }
    }
}

void SMeshConstruct::checkDefaultShading(const EBasicMeshes Model)
{
    switch (Model)
    {
        case MESH_CUBE:
        case MESH_PLANE:
        case MESH_DISK:
        case MESH_DODECAHEDRON:
        case MESH_ICOSAHEDRON:
        case MESH_TETRAHEDRON:
        case MESH_CUBOCTAHEDRON:
            Shading = video::SHADING_FLAT; break;
    }
}


/*
 * BasicMeshGenerator class
 */

BasicMeshGenerator::BasicMeshGenerator() :
    Mesh_   (0),
    Surface_(0)
{
}
BasicMeshGenerator::~BasicMeshGenerator()
{
}

void BasicMeshGenerator::createMesh(Mesh* Object, const EBasicMeshes Model, const SMeshConstruct &BuildConstruct)
{
    if (!Object)
        return;
    
    BuildConstruct_ = BuildConstruct;
    BuildConstruct_.checkDefaultSegments(Model, BuildConstruct_.SegmentsVert);
    BuildConstruct_.checkDefaultSegments(Model, BuildConstruct_.SegmentsHorz);
    BuildConstruct_.checkDefaultShading(Model);
    
    if (BuildConstruct_.SegmentsVert <= 0 || BuildConstruct_.SegmentsHorz <= 0)
        return;
    
    Mesh_ = Object;
    
    Mesh_->setShading(BuildConstruct_.Shading);
    Surface_ = Mesh_->createMeshBuffer(SceneGraph::getDefaultVertexFormat(), SceneGraph::getDefaultIndexFormat());
    
    switch (Model)
    {
        #ifdef SP_COMPILE_WITH_PRIMITIVE_CUBE
        case MESH_CUBE:
            createCube(); break;
        #endif
        #ifdef SP_COMPILE_WITH_PRIMITIVE_CONE
        case MESH_CONE:
            createCone(); break;
        #endif
        #ifdef SP_COMPILE_WITH_PRIMITIVE_CYLINDER
        case MESH_CYLINDER:
            createCylinder(); break;
        #endif
        #ifdef SP_COMPILE_WITH_PRIMITIVE_SPHERE
        case MESH_SPHERE:
            createSphere(); break;
        #endif
        #ifdef SP_COMPILE_WITH_PRIMITIVE_ICOSPHERE
        case MESH_ICOSPHERE:
            createIcosphere(); break;
        #endif
        #ifdef SP_COMPILE_WITH_PRIMITIVE_TORUS
        case MESH_TORUS:
            createTorus(); break;
        #endif
        #ifdef SP_COMPILE_WITH_PRIMITIVE_TORUSKNOT
        case MESH_TORUSKNOT:
            createTorusknot(); break;
        #endif
        #ifdef SP_COMPILE_WITH_PRIMITIVE_PIPE
        case MESH_PIPE:
            createPipe(); break;
        #endif
        #ifdef SP_COMPILE_WITH_PRIMITIVE_SPIRAL
        case MESH_SPIRAL:
            createSpiral(); break;
        #endif
        #ifdef SP_COMPILE_WITH_PRIMITIVE_PLANE
        case MESH_PLANE:
            createPlane(); break;
        #endif
        #ifdef SP_COMPILE_WITH_PRIMITIVE_DISK
        case MESH_DISK:
            createDisk(); break;
        #endif
        #ifdef SP_COMPILE_WITH_PRIMITIVE_CUBOCTAHEDRON
        case MESH_CUBOCTAHEDRON:
            createCuboctahedron(); break;
        #endif
        #ifdef SP_COMPILE_WITH_PRIMITIVE_TETRAHEDRON
        case MESH_TETRAHEDRON:
            createTetrahedron(); break;
        #endif
        #ifdef SP_COMPILE_WITH_PRIMITIVE_OCTAHEDRON
        case MESH_OCTAHEDRON:
            createOctahedron(); break;
        #endif
        #ifdef SP_COMPILE_WITH_PRIMITIVE_DODECAHEDRON
        case MESH_DODECAHEDRON:
            createDodecahedron(); break;
        #endif
        #ifdef SP_COMPILE_WITH_PRIMITIVE_ICOSAHEDRON
        case MESH_ICOSAHEDRON:
            createIcosahedron(); break;
        #endif
        #ifdef SP_COMPILE_WITH_PRIMITIVE_TEAPOT
        case MESH_TEAPOT:
            createTeapot(); break;
        #endif
        
        #ifdef SP_COMPILE_WITH_PRIMITIVE_WIRE_CUBE
        case MESH_WIRE_CUBE:
            createWireCube(); break;
        #endif
        
        default:
            io::Log::warning("Specified primitive was not compiled with"); break;
    }
    
    Mesh_->updateIndexBuffer();
    Mesh_->updateNormals();
}

void BasicMeshGenerator::createSuperShape(Mesh* Object, const f32 ValueList[12], const s32 Segments)
{
    Mesh_ = Object;
    
    #ifdef SP_COMPILE_WITH_PRIMITIVE_SUPERSHAPE
    Surface_ = Mesh_->createMeshBuffer(SceneGraph::getDefaultVertexFormat(), SceneGraph::getDefaultIndexFormat());
    
    createSuperShapeSurface(ValueList, Segments);
    
    Mesh_->updateNormals();
    #endif
}

Mesh* BasicMeshGenerator::createBezierPatch(
    const dim::vector3df AnchorPoints[4][4], const s32 Segments, bool isFrontFace)
{
    Mesh_ = new Mesh();
    Surface_ = Mesh_->createMeshBuffer(SceneGraph::getDefaultVertexFormat(), SceneGraph::getDefaultIndexFormat());
    
    createBezierPatchFace(Mesh_, 0, AnchorPoints, Segments, isFrontFace);
    
    Mesh_->updateNormals();
    
    return Mesh_;
}

void BasicMeshGenerator::createBezierPatchFace(
    Mesh* Mesh, const u32 Surface, const dim::vector3df AnchorPoints[4][4], const s32 Segments, bool isFrontFace)
{
    /* Temporary variables */
    s32 u, v;
    f32 px, py, lpy;
    bool isStripCCW = true;
    
    dim::vector3df Temp[4];
    std::vector<dim::vector3df> VertexCoords;
    std::vector<dim::point2df> VertexTexCoords;
    
    boost::shared_array<dim::vector3df> VecList(new dim::vector3df[Segments + 1]);
    
    /* Precalculations */
    Surface_ = Mesh_->getMeshBuffer(Surface);
    Surface_->setIndexOffset(Surface_->getVertexCount());
    
    Temp[0] = AnchorPoints[0][3];
    Temp[1] = AnchorPoints[1][3];
    Temp[2] = AnchorPoints[2][3];
    Temp[3] = AnchorPoints[3][3];
    
    for (v = 0; v <= Segments; ++v)
    {
        px = static_cast<f32>(v) / Segments;
        VecList[v] = math::getBernsteinValue<dim::vector3df>(px, Temp);
    }
    
    /* Create the bezier patch */
    for (u = 1; u <= Segments; ++u)
    {
        py  = static_cast<f32>(u) / Segments;
        lpy = (static_cast<f32>(u) - 1.0f) / Segments;
        
        Temp[0] = math::getBernsteinValue<dim::vector3df>(py, AnchorPoints[0]);
        Temp[1] = math::getBernsteinValue<dim::vector3df>(py, AnchorPoints[1]);
        Temp[2] = math::getBernsteinValue<dim::vector3df>(py, AnchorPoints[2]);
        Temp[3] = math::getBernsteinValue<dim::vector3df>(py, AnchorPoints[3]);
        
        for (v = 0; v <= Segments; ++v)
        {
            px = static_cast<f32>(v) / Segments;
            
            VertexTexCoords.push_back(dim::point2df(lpy, px));
            VertexCoords.push_back(VecList[v]);
            
            VecList[v] = math::getBernsteinValue<dim::vector3df>(px, Temp);
            
            VertexTexCoords.push_back(dim::point2df(py, px));
            VertexCoords.push_back(VecList[v]);
        }
        
        for (s32 i = 0; i < VertexCoords.size() - 2; ++i)
        {
            Surface_->addVertex(VertexCoords[i + 0], VertexTexCoords[i + 0]);
            Surface_->addVertex(VertexCoords[i + 1], VertexTexCoords[i + 1]);
            Surface_->addVertex(VertexCoords[i + 2], VertexTexCoords[i + 2]);
            
            if ( ( isStripCCW && isFrontFace ) || ( !isStripCCW && !isFrontFace ) )
                Surface_->addTriangle(2, 1, 0);
            else
                Surface_->addTriangle(0, 1, 2);
            
            Surface_->addIndexOffset(3);
            
            isStripCCW = !isStripCCW;
        }
        
        VertexTexCoords.clear();
        VertexCoords.clear();
    }
}

Mesh* BasicMeshGenerator::createSkyBox(video::Texture* TextureList[6], f32 Radius)
{
    Mesh_ = new Mesh();
    
    const f32 UVMap1 = 0.0f;
    const f32 UVMap2 = 1.0f;
    
    // Front
    Surface_ = Mesh_->createMeshBuffer(SceneGraph::getDefaultVertexFormat(), SceneGraph::getDefaultIndexFormat());
    Surface_->addTexture(TextureList[0]);
    
    addVertex(-Radius,  Radius, -Radius, UVMap2, UVMap1);
    addVertex( Radius,  Radius, -Radius, UVMap1, UVMap1);
    addVertex( Radius, -Radius, -Radius, UVMap1, UVMap2);
    addVertex(-Radius, -Radius, -Radius, UVMap2, UVMap2);
    addFace(2, 1, 0); addFace(3, 2, 0);
    
    // Back
    Surface_ = Mesh_->createMeshBuffer(SceneGraph::getDefaultVertexFormat(), SceneGraph::getDefaultIndexFormat());
    Surface_->addTexture(TextureList[1]);
    
    addVertex(-Radius,  Radius,  Radius, UVMap1, UVMap1);
    addVertex( Radius,  Radius,  Radius, UVMap2, UVMap1);
    addVertex( Radius, -Radius,  Radius, UVMap2, UVMap2);
    addVertex(-Radius, -Radius,  Radius, UVMap1, UVMap2);
    addFace(0, 1, 2); addFace(0, 2, 3);
    
    // Top
    Surface_ = Mesh_->createMeshBuffer(SceneGraph::getDefaultVertexFormat(), SceneGraph::getDefaultIndexFormat());
    Surface_->addTexture(TextureList[2]);
    
    addVertex(-Radius,  Radius,  Radius, UVMap2, UVMap2);
    addVertex( Radius,  Radius,  Radius, UVMap2, UVMap1);
    addVertex( Radius,  Radius, -Radius, UVMap1, UVMap1);
    addVertex(-Radius,  Radius, -Radius, UVMap1, UVMap2);
    addFace(2, 1, 0); addFace(3, 2, 0);
    
    // Bottom
    Surface_ = Mesh_->createMeshBuffer(SceneGraph::getDefaultVertexFormat(), SceneGraph::getDefaultIndexFormat());
    Surface_->addTexture(TextureList[3]);
    
    addVertex(-Radius, -Radius,  Radius, UVMap2, UVMap1);
    addVertex( Radius, -Radius,  Radius, UVMap2, UVMap2);
    addVertex( Radius, -Radius, -Radius, UVMap1, UVMap2);
    addVertex(-Radius, -Radius, -Radius, UVMap1, UVMap1);
    addFace(0, 1, 2); addFace(0, 2, 3);
    
    // Right
    Surface_ = Mesh_->createMeshBuffer(SceneGraph::getDefaultVertexFormat(), SceneGraph::getDefaultIndexFormat());
    Surface_->addTexture(TextureList[4]);
    
    addVertex( Radius,  Radius,  Radius, UVMap1, UVMap1);
    addVertex( Radius,  Radius, -Radius, UVMap2, UVMap1);
    addVertex( Radius, -Radius, -Radius, UVMap2, UVMap2);
    addVertex( Radius, -Radius,  Radius, UVMap1, UVMap2);
    addFace(0, 1, 2); addFace(0, 2, 3);
    
    // Left
    Surface_ = Mesh_->createMeshBuffer(SceneGraph::getDefaultVertexFormat(), SceneGraph::getDefaultIndexFormat());
    Surface_->addTexture(TextureList[5]);
    
    addVertex(-Radius,  Radius,  Radius, UVMap2, UVMap1);
    addVertex(-Radius,  Radius, -Radius, UVMap1, UVMap1);
    addVertex(-Radius, -Radius, -Radius, UVMap1, UVMap2);
    addVertex(-Radius, -Radius,  Radius, UVMap2, UVMap2);
    addFace(2, 1, 0); addFace(3, 2, 0);
    
    Mesh_->setOrder(ORDER_BACKGROUND);
    Mesh_->getMaterial()->setDepthBuffer(false);
    Mesh_->getMaterial()->setLighting(false);
    
    Mesh_->updateMeshBuffer();
    
    return Mesh_;
}


// Height field (alternate of a terrain)

Mesh* BasicMeshGenerator::createHeightField(const video::Texture* TexHeightMap, const s32 Segments)
{
    Mesh_ = new Mesh();
    
    if (!TexHeightMap || !TexHeightMap->getImageBuffer())
        return Mesh_;
    
    /* Temporary variables */
    const f32 size = 1.0f / Segments;
    
    s32 x, z;
    s32 v0, v1, v2, v3;
    f32 y;

    s32 Width   = TexHeightMap->getSize().Width;
    s32 Height  = TexHeightMap->getSize().Height;
    
    const video::ImageBuffer* ImgBuffer = TexHeightMap->getImageBuffer();
    
    Surface_ = Mesh_->createMeshBuffer(SceneGraph::getDefaultVertexFormat(), SceneGraph::getDefaultIndexFormat());
    
    /* Create all vertices */
    for (z = 0; z <= Segments; ++z)
    {
        for (x = 0; x <= Segments; ++x)
        {
            y = ImgBuffer->getPixelColor(
                dim::point2di(s32(size*x*Width), s32(size*z*Height))
            ).getBrightness<f32>() / 255;
            
            addVertex(
                size*x - 0.5f, y, -size*z + 0.5f, size*x, size*z
            );
        }
    }
    
    /* Create all triangles */
    for (z = 0; z < Segments; ++z)
    {
        for (x = 0; x < Segments; ++x)
        {
            /* Compute the vertices indices */
            v0 = z * ( Segments + 1 ) + x;
            v1 = z * ( Segments + 1 ) + x + 1;
            v2 = ( z + 1 ) * ( Segments + 1 ) + x + 1;
            v3 = ( z + 1 ) * ( Segments + 1 ) + x;
            
            /* Add the triangel */
            addFace(v0, v1, v2, v3);
        }
    }
    
    Mesh_->updateIndexBuffer();
    Mesh_->updateNormals();
    
    return Mesh_;
}


/*
 * ========== Private: ==========
 */

#ifdef SP_COMPILE_WITH_PRIMITIVE_CUBE

/*

          4-------5
         /|      /|
        / |     / |
       0-------1  |
       |  7 - -|- 6
+Y     | /     | /
 ^ +Z  |/      |/
 | /   3-------2
 |/
 0----> +X

*/

void BasicMeshGenerator::createCube()
{
    const f32 Radius = BuildConstruct_.RadiusInner;
    
    // Back
    addQuadFace(
        dim::vector3df( Radius,  Radius,  Radius), dim::point2df(0, 0),
        dim::vector3df(-Radius, -Radius,  Radius), dim::point2df(1, 1),
        dim::vector3df(1, 0, 0), dim::vector3df(0, 1, 0)
    );
    
    // Front
    addQuadFace(
        dim::vector3df(-Radius,  Radius, -Radius), dim::point2df(0, 0),
        dim::vector3df( Radius, -Radius, -Radius), dim::point2df(1, 1),
        dim::vector3df(1, 0, 0), dim::vector3df(0, 1, 0)
    );
    
    // Top
    addQuadFace(
        dim::vector3df(-Radius,  Radius,  Radius), dim::point2df(0, 0),
        dim::vector3df( Radius,  Radius, -Radius), dim::point2df(1, 1),
        dim::vector3df(1, 0, 0), dim::vector3df(0, 0, 1)
    );
    
    // Bottom
    addQuadFace(
        dim::vector3df(-Radius, -Radius,  Radius), dim::point2df(0, 0),
        dim::vector3df( Radius, -Radius, -Radius), dim::point2df(1, 1),
        dim::vector3df(1, 0, 0), dim::vector3df(0, 0, 1), true
    );
    
    // Left
    addQuadFace(
        dim::vector3df(-Radius,  Radius,  Radius), dim::point2df(0, 0),
        dim::vector3df(-Radius, -Radius, -Radius), dim::point2df(1, 1),
        dim::vector3df(0, 0, 1), dim::vector3df(0, 1, 0)
    );
    
    // Right
    addQuadFace(
        dim::vector3df( Radius,  Radius, -Radius), dim::point2df(0, 0),
        dim::vector3df( Radius, -Radius,  Radius), dim::point2df(1, 1),
        dim::vector3df(0, 0, 1), dim::vector3df(0, 1, 0)
    );
}

#endif

#ifdef SP_COMPILE_WITH_PRIMITIVE_CONE

void BasicMeshGenerator::createCone()
{
    const s32 Segs = math::Max(3, BuildConstruct_.SegmentsVert);
    const f32 Radius = BuildConstruct_.RadiusInner;
    
    const f32 c = 360.0f / Segs;
    f32 i;
    s32 v;
    
    /* Build body */
    Surface_->addVertex(dim::vector3df(0.0f, 0.5f, 0.0f), dim::point2df(0.5f, 0));
    
    for (i = 0, v = 0; i < 360; i += c, v += 2)
    {
        Surface_->addVertex(dim::vector3df(SIN(i + c)*Radius, -0.5f, COS(i + c)*Radius), dim::point2df((360 - i - c)/360, 1));
        Surface_->addVertex(dim::vector3df(SIN(i    )*Radius, -0.5f, COS(i    )*Radius), dim::point2df((360 - i    )/360, 1));
        Surface_->addTriangle(v + 2, v + 1, 0);
    }
    
    /* Build cap */
    if (BuildConstruct_.HasCap)
    {
        Surface_->addIndexOffset(v + 1);
        Surface_->addVertex(dim::vector3df(0.0f, -0.5f, Radius), dim::point2df(0.5f, 1));
        Surface_->addVertex(dim::vector3df(SIN(c)*Radius, -0.5f, COS(c)*Radius), dim::point2df(0.5f + SIN(c)*Radius, 0.5f + COS(c)*Radius));
        
        for (i = c*2, v = 0; i < 360; i += c, ++v)
        {
            Surface_->addVertex(
                dim::vector3df(SIN(i)*Radius, -0.5f, COS(i)*Radius), dim::point2df(0.5f+SIN(i)*Radius, 0.5f+COS(i)*Radius)
            );
            Surface_->addTriangle(v + 2, v + 1, 0);
        }
    }
}

#endif

#ifdef SP_COMPILE_WITH_PRIMITIVE_CYLINDER

void BasicMeshGenerator::createCylinder()
{
    /* Temporary variables */
    const s32 Detail = math::MinMax(BuildConstruct_.SegmentsVert, 3, 360);
    const f32 Radius = BuildConstruct_.RadiusInner;
    
    const f32 c = 360.0f / Detail;
    
    f32 i;
    s32 v = 0;
    
    /* Body */
    Surface_->addVertex(dim::vector3df(0.0f,  0.5f, 0.5f), dim::point2df(3, 0));
    Surface_->addVertex(dim::vector3df(0.0f, -0.5f, 0.5f), dim::point2df(3, 1));
    
    for (i = c; i <= 360; i += c)
    {
        Surface_->addVertex(dim::vector3df(SIN(i)*Radius,  0.5f, COS(i)*Radius), dim::point2df((360 - i)/360*3, 0));
        Surface_->addVertex(dim::vector3df(SIN(i)*Radius, -0.5f, COS(i)*Radius), dim::point2df((360 - i)/360*3, 1));
        Surface_->addTriangle(3, 2, 0); Surface_->addTriangle(1, 3, 0);
        Surface_->addIndexOffset(2);
    }
    
    /* Cap */
    if (BuildConstruct_.HasCap)
    {
        /* Top */
        Surface_->addIndexOffset(2);
        Surface_->addVertex(dim::vector3df(         0.0f, 0.5f,        Radius), dim::point2df(            0.5f,             0.0f));
        Surface_->addVertex(dim::vector3df(SIN(c)*Radius, 0.5f, COS(c)*Radius), dim::point2df(0.5f+SIN(c)*0.5f, 0.5f-COS(c)*0.5f));
        
        for (i = c*2, v = 0; i < 360; i += c, ++v)
        {
            Surface_->addVertex(dim::vector3df(SIN(i)*Radius, 0.5f, COS(i)*Radius), dim::point2df(0.5f+SIN(i)*0.5f, 0.5f-COS(i)*0.5f));
            Surface_->addTriangle(0, v+1, v+2);
        }
        Surface_->addIndexOffset(v+2);
        
        /* Bottom */
        Surface_->addVertex(dim::vector3df(         0.0f, -0.5f,        Radius), dim::point2df(            0.5f,             1.0f));
        Surface_->addVertex(dim::vector3df(SIN(c)*Radius, -0.5f, COS(c)*Radius), dim::point2df(0.5f+SIN(c)*0.5f, 0.5f+COS(c)*0.5f));
        
        for (i = c*2, v = 0; i < 360; i += c, ++v)
        {
            Surface_->addVertex(dim::vector3df(SIN(i)*Radius, -0.5f, COS(i)*Radius), dim::point2df(0.5f+SIN(i)*0.5f, 0.5f+COS(i)*0.5f));
            Surface_->addTriangle(v+2, v+1, 0);
        }
    }
}

#endif

#ifdef SP_COMPILE_WITH_PRIMITIVE_SPHERE

void BasicMeshGenerator::createSphere()
{
    /* Temporary variables */
    s32 i, j;
    
    const s32 Detail = math::MinMax(BuildConstruct_.SegmentsVert, 2, 180) * 2;
    const f32 Radius = BuildConstruct_.RadiusInner;
    
    f32 x, y, z, u, v;
    
    const f32 FinalDetail = 360.0f / Detail;
    s32 DegX = Detail;
    s32 DegY = Detail / 2;
    
    u32 v0, v1, v2, v3;
    
    /* Create vertices */
    for (i = 0; i <= DegY; ++i)
    {
        /* Height */
        y = COS(i*FinalDetail) * Radius;
        
        for (j = 0; j <= DegX; ++j)
        {
            /* Coordination */
            x = SIN(j*FinalDetail) * SIN(i*FinalDetail) * Radius;
            z = COS(j*FinalDetail) * SIN(i*FinalDetail) * Radius;
            
            /* UV-Mapping */
            u = (f32)j / DegX;
            v = (f32)i / DegY;
            
            /* Add vertex */
            Surface_->addVertex(dim::vector3df(x, y, z), dim::point2df(u, v));
        }
    }
    
    /* Create triangles */
    ++DegX;
    
    /* Top */
    for (j = 0; j < DegX - 1; ++j)
    {
        v0 = j+1;
        v1 = DegX + j;
        v2 = DegX + j+1;
        
        Surface_->addTriangle(v0, v1, v2);
    }
    
    /* Body */
    for (i = 1; i < DegY - 1; ++i)
    {
        for (j = 0; j < DegX - 1; ++j)
        {
            v0 = i*DegX + j;
            v1 = i*DegX + j+1;
            v2 = (i+1)*DegX + j;
            v3 = (i+1)*DegX + j+1;
            
            Surface_->addTriangle(v2, v1, v0);
            Surface_->addTriangle(v1, v2, v3);
        }
    }
    
    /* Bottom */
    for (j = 0; j < DegX - 1; ++j)
    {
        v0 = (DegY-1)*DegX + j;
        v1 = (DegY-1)*DegX + j+1;
        v2 = DegY*DegX + j;
        
        Surface_->addTriangle(v2, v1, v0);
    }
}

#endif

#ifdef SP_COMPILE_WITH_PRIMITIVE_ICOSPHERE

void BasicMeshGenerator::createIcosphere()
{
    /* Temporary variables */
    s32 i, j;
    
    const s32 Detail = math::MinMax(BuildConstruct_.SegmentsVert, 1, 8);
    const f32 Radius = BuildConstruct_.RadiusInner;
    
    std::list<dim::triangle3df> Triangles, NewTriangles;
    
    /* Temporary vertices */
    static const f32 size = 0.30902f;
    
    const dim::vector3df Vertices[] = {
        dim::vector3df( 0.0f,  0.5f, -0.5f - size) * Radius,
        dim::vector3df( 0.0f,  0.5f,  0.5f + size) * Radius,
        dim::vector3df( 0.0f, -0.5f,  0.5f + size) * Radius,
        dim::vector3df( 0.0f, -0.5f, -0.5f - size) * Radius,
        
        dim::vector3df(-0.5f,  0.5f + size,  0.0f) * Radius,
        dim::vector3df( 0.5f,  0.5f + size,  0.0f) * Radius,
        dim::vector3df( 0.5f, -0.5f - size,  0.0f) * Radius,
        dim::vector3df(-0.5f, -0.5f - size,  0.0f) * Radius,
        
        dim::vector3df(-0.5f - size,  0.0f, -0.5f) * Radius,
        dim::vector3df(-0.5f - size,  0.0f,  0.5f) * Radius,
        dim::vector3df( 0.5f + size,  0.0f,  0.5f) * Radius,
        dim::vector3df( 0.5f + size,  0.0f, -0.5f) * Radius
    };
    
    dim::vector3df NewVertices[6];
    
    const f32 OriginDistance = Vertices[0].getLength();
    
    /* Create basic of icosahedron */
    Triangles.push_back(dim::triangle3df(Vertices[ 0], Vertices[ 4], Vertices[ 5]));
    Triangles.push_back(dim::triangle3df(Vertices[ 0], Vertices[ 5], Vertices[11]));
    Triangles.push_back(dim::triangle3df(Vertices[ 3], Vertices[11], Vertices[ 6]));
    Triangles.push_back(dim::triangle3df(Vertices[ 3], Vertices[ 7], Vertices[ 8]));
    Triangles.push_back(dim::triangle3df(Vertices[ 3], Vertices[ 6], Vertices[ 7]));
    Triangles.push_back(dim::triangle3df(Vertices[ 0], Vertices[11], Vertices[ 3]));
    Triangles.push_back(dim::triangle3df(Vertices[ 0], Vertices[ 3], Vertices[ 8]));
    Triangles.push_back(dim::triangle3df(Vertices[ 8], Vertices[ 4], Vertices[ 0]));
    Triangles.push_back(dim::triangle3df(Vertices[ 5], Vertices[10], Vertices[11]));
    Triangles.push_back(dim::triangle3df(Vertices[ 5], Vertices[ 1], Vertices[10]));
    Triangles.push_back(dim::triangle3df(Vertices[ 6], Vertices[11], Vertices[10]));
    Triangles.push_back(dim::triangle3df(Vertices[ 9], Vertices[ 8], Vertices[ 7]));
    Triangles.push_back(dim::triangle3df(Vertices[ 1], Vertices[ 4], Vertices[ 9]));
    Triangles.push_back(dim::triangle3df(Vertices[ 1], Vertices[ 5], Vertices[ 4]));
    Triangles.push_back(dim::triangle3df(Vertices[ 6], Vertices[ 2], Vertices[ 7]));
    Triangles.push_back(dim::triangle3df(Vertices[ 4], Vertices[ 8], Vertices[ 9]));
    Triangles.push_back(dim::triangle3df(Vertices[10], Vertices[ 1], Vertices[ 2]));
    Triangles.push_back(dim::triangle3df(Vertices[ 2], Vertices[ 1], Vertices[ 9]));
    Triangles.push_back(dim::triangle3df(Vertices[10], Vertices[ 2], Vertices[ 6]));
    Triangles.push_back(dim::triangle3df(Vertices[ 9], Vertices[ 7], Vertices[ 2]));
    
    /* Process the subdevision */
    for (s32 i = 1; i < Detail; ++i)
    {
        for (std::list<dim::triangle3df>::iterator it = Triangles.begin(); it != Triangles.end(); ++it)
        {
            NewVertices[0] = it->PointA;
            NewVertices[1] = it->PointB;
            NewVertices[2] = it->PointC;
            
            NewVertices[3] = (NewVertices[0] + NewVertices[1]) * 0.5;
            NewVertices[4] = (NewVertices[1] + NewVertices[2]) * 0.5;
            NewVertices[5] = (NewVertices[2] + NewVertices[0]) * 0.5;
            
            NewVertices[3].setLength(OriginDistance);
            NewVertices[4].setLength(OriginDistance);
            NewVertices[5].setLength(OriginDistance);
            
            NewTriangles.push_back(dim::triangle3df(NewVertices[0], NewVertices[3], NewVertices[5]));
            NewTriangles.push_back(dim::triangle3df(NewVertices[5], NewVertices[3], NewVertices[4]));
            NewTriangles.push_back(dim::triangle3df(NewVertices[2], NewVertices[5], NewVertices[4]));
            NewTriangles.push_back(dim::triangle3df(NewVertices[4], NewVertices[3], NewVertices[1]));
        }
        
        // Delete the old triangles and add the new triangles
        Triangles = NewTriangles;
        NewTriangles.clear();
    }
    
    /* Create the final faces */
    foreach (const dim::triangle3df &Face, Triangles)
        addFace(Face.PointA, Face.PointB, Face.PointC);
}

#endif

#ifdef SP_COMPILE_WITH_PRIMITIVE_TORUS

void BasicMeshGenerator::createTorus()
{
    /* Temporary variables */
    s32 i, j;
    
    const s32 Detail    = math::MinMax(BuildConstruct_.SegmentsVert, 2, 180) * 2;
    const f32 Radius1   = BuildConstruct_.RadiusInner;
    const f32 Radius2   = BuildConstruct_.RadiusOuter;
    
    f32 x, y, z, u, v;
    
    const f32 FinalDetail = 360.0f / Detail;
    s32 DegX = Detail;
    s32 DegY = Detail / 2;
    
    u32 v0, v1, v2, v3, c;
    
    /* Create vertices - outside */
    for (i = 0; i <= DegY; ++i)
    {
        /* Height */
        y = COS(i*FinalDetail) * Radius2;
        
        for (j = 0; j <= DegX; ++j)
        {
            /* Coordination */
            x = SIN(j*FinalDetail)*Radius1 + SIN(j*FinalDetail) * SIN(i*FinalDetail) * Radius2;
            z = COS(j*FinalDetail)*Radius1 + COS(j*FinalDetail) * SIN(i*FinalDetail) * Radius2;
            
            /* UV-Mapping */
            u = (f32)j / DegX;
            v = (f32)i / DegY;
            
            /* Add vertex */
            Surface_->addVertex(dim::vector3df(x, y, z), dim::point2df(u, v));
        }
    }
    
    /* Create vertices - inside */
    for (i = 0; i <= DegY; ++i)
    {
        /* Height */
        y = COS(i*FinalDetail) * Radius2;
        
        for (j = 0; j <= DegX; ++j)
        {
            /* Coordination */
            x = SIN(j*FinalDetail)*Radius1 - SIN(j*FinalDetail) * SIN(i*FinalDetail) * Radius2;
            z = COS(j*FinalDetail)*Radius1 - COS(j*FinalDetail) * SIN(i*FinalDetail) * Radius2;
            
            /* UV-Mapping */
            u = (f32)j / DegX;
            v = (f32)i / DegY;
            
            /* Add vertex */
            Surface_->addVertex(dim::vector3df(x, y, z), dim::point2df(u, v));
        }
    }
    
    ++DegX;
    
    /* Create triangles - outside */
    for (i = 0; i < DegY; ++i)
    {
        for (j = 0; j < DegX-1; ++j)
        {
            v0 = i*DegX + j;
            v1 = i*DegX + j+1;
            v2 = (i+1)*DegX + j;
            v3 = (i+1)*DegX + j+1;
            
            Surface_->addTriangle(v2, v1, v0);
            Surface_->addTriangle(v1, v2, v3);
        }
    }
    
    /* Create triangles - inside */
    c = DegY * DegX + DegX;
    
    for (i = 0; i < DegY; ++i)
    {
        for (j = 0; j < DegX - 1; ++j)
        {
            v0 = c + i*DegX + j;
            v1 = c + i*DegX + j+1;
            v2 = c + (i+1)*DegX + j;
            v3 = c + (i+1)*DegX + j+1;
            
            Surface_->addTriangle(v0, v1, v2);
            Surface_->addTriangle(v3, v2, v1);
        }
    }
}

#endif

#ifdef SP_COMPILE_WITH_PRIMITIVE_TORUSKNOT

void BasicMeshGenerator::createTorusknot()
{
    #if 0
    
    /*
    x = ( 2 + cos(q * angle / p) ) * cos(angle)
    y = ( 2 + cos(q * angle / p) ) * sin(angle)
    z = sin(q * angle / p)
    */
    
    const f32 p = 3.0f;
    const f32 q = 8.0f;
    
    f32 x, y, z;
    f32 deg;
    
    for (deg = 0.0f; deg < 2*p*M_PI; deg += 0.05f)
    {
        x = ( 2 + cos(q * deg / p) ) * cos(deg);
        y = ( 2 + cos(q * deg / p) ) * sin(deg);
        z = sin(q * deg / p);
        
        Surface_->addVertex(
            dim::vector3df(x+3, y + 5, z)
        );
    }
    
    #endif
    
    /* Temporary variables */
    s32 i, j;
    
    const f32 Radius1   = BuildConstruct_.RadiusInner;
    const f32 Radius2   = BuildConstruct_.RadiusOuter;
    
    s32 Turns = 7, Slices = 16, Stacks = 256;
    f32 t;
    
    u32 v0, v1, v2, v3;
    
    boost::shared_array<dim::vector3df> RingCenters(new dim::vector3df[Stacks]);
    
    #if 1
    const f32 p = 5.0f, q = 8.0f;
    f32 deg = 0.0f;
    #endif
    
    /* Loop for each ring center */
    for (i = 0; i < Stacks; ++i, deg += 2*p*M_PI/Stacks)
    {
        t = M_PI*2*i / Stacks;
        
        RingCenters[i].set(
            #if 0
            ( 2 + cos(q * deg / p) ) * cos(deg),
            sin(q * deg / p),
            ( 2 + cos(q * deg / p) ) * sin(deg)
            #else
            ( 1.0f + Radius1*cos(t*Turns) )*cos(t*2),
            ( 1.0f + Radius1*cos(t*Turns) )*sin(t*Turns)*Radius1,
            ( 1.0f + Radius1*cos(t*Turns) )*sin(t*2)
            #endif
        );
    }
    
    /* Loop for each ring */
    for (i = 0; i < Stacks; ++i)
    {
        /* Loop for each vertex of the current ring */
        for (j = 0; j < Slices; ++j)
        {
            /* Compute the vector from the center of this ring to the next */
            dim::vector3df Tangent = RingCenters[i == Stacks - 1 ? 0 : i + 1] - RingCenters[i];
            
            /* Compute the vector perpendicular to the tangent, pointing approximately in the positive Y direction */
            dim::vector3df tmp1 = dim::vector3df(0.0f, 1.0f, 0.0f).cross(Tangent);
            dim::vector3df tmp2 = Tangent.cross(tmp1);
            tmp2.setLength(Radius2);
            
            /* Add the computed vertex */
            Surface_->addVertex(
                RingCenters[i] + tmp2.getRotatedAxis(360.0f*j / Slices, Tangent),
                dim::point2df((f32)i/5, (f32)j/Slices*3.0f)
            );
        }
    }
    
    /* Loop for each quad */
    for (i = 0; i < Stacks; ++i)
    {
        for (j = 0; j < Slices; ++j)
        {
            v0 = i*Slices + j;
            
            if (j != Slices - 1) 
                v1 = i*Slices + j + 1;
            else
                v1 = i*Slices;
            
            if (i != Stacks - 1)
                v2 = (i+1)*Slices + j;
            else
                v2 = j;
            
            if (i != Stacks - 1)
            {
                if (j != Slices - 1)
                    v3 = (i+1)*Slices + j + 1;
                else
                    v3 = (i+1)*Slices;
            }
            else
            {
                if (j != Slices - 1)
                    v3 = j + 1;
                else
                    v3 = 0;
            }
            
            /* Add the computed quad */
            addFace(v0, v1, v2);
            addFace(v3, v2, v1);
        }
    }
}

#endif

#ifdef SP_COMPILE_WITH_PRIMITIVE_SPIRAL

void BasicMeshGenerator::createSpiral()
{
    /* Temporary variables */
    s32 i, j;
    
    const s32 Detail        = math::MinMax(BuildConstruct_.SegmentsVert, 2, 180) * 2;
    const f32 Radius1       = BuildConstruct_.RadiusInner;
    const f32 Radius2       = BuildConstruct_.RadiusOuter;
    const f32 DegreeLength  = BuildConstruct_.RotationDegree;
    const f32 Size          = BuildConstruct_.RotationDistance;
    
    f32 x, y, z;
    
    const f32 FinalDetail = 360.0f/Detail;
    const f32 Height = f32(DegreeLength*Size) / 360 / 2;
    const s32 LengthDetail = s32(f32(Detail*DegreeLength)/360)+1;
    
    u32 v0, v1, v2, v3;
    
    /* === Create vertices === */
    
    /* Body */
    for (i = 0; i < LengthDetail; ++i)
    {
        for (j = 0; j <= Detail; ++j)
        {
            x = SIN(i*FinalDetail) * (Radius1 + COS(j*FinalDetail)*Radius2);
            y = f32(i*FinalDetail*Size) / 360 + SIN(j*FinalDetail)*Radius2 - Height;
            z = COS(i*FinalDetail) * (Radius1 + COS(j*FinalDetail)*Radius2);
            
            Surface_->addVertex(
                dim::vector3df(x, y, z), dim::point2df(f32(i*FinalDetail*3)/360, 0.5f + SIN(j*FinalDetail)*0.5f)
            );
        }
    }
    
    if (BuildConstruct_.HasCap)
    {
        /* Bottom */
        for (j = 0; j <= Detail; ++j)
        {
            y = SIN(j*FinalDetail)*Radius2 - Height;
            z = (Radius1 + COS(j*FinalDetail)*Radius2);
            
            Surface_->addVertex(
                dim::vector3df(0.0f, y, z), dim::point2df(0.5f + SIN(j*FinalDetail - 90)*0.5f, 0.5f - COS(j*FinalDetail - 90)*0.5f)
            );
        }
        
        /* Top */
        for (j = 0, i = LengthDetail - 1; j <= Detail; ++j)
        {
            x = SIN(i*FinalDetail) * (Radius1 + COS(j*FinalDetail)*Radius2);
            y = f32(i*FinalDetail*Size) / 360 + SIN(j*FinalDetail)*Radius2 - Height;
            z = COS(i*FinalDetail) * (Radius1 + COS(j*FinalDetail)*Radius2);
            
            Surface_->addVertex(
                dim::vector3df(x, y, z), dim::point2df(0.5f + SIN(j*FinalDetail + 90)*0.5f, 0.5f + COS(j*FinalDetail + 90)*0.5f)
            );
        }
    }
    
    /* === Create triangles === */
    
    /* Body */
    for (i = 0; i < LengthDetail - 1; ++i)
    {
        for (j = 0; j < Detail; ++j)
        {
            v0 =  i   *(Detail+1)+j;
            v1 = (i+1)*(Detail+1)+j;
            v2 = (i+1)*(Detail+1)+j+1;
            v3 =  i   *(Detail+1)+j+1;
            
            Surface_->addTriangle(v0, v1, v2);
            Surface_->addTriangle(v0, v2, v3);
        }
    }
    
    if (BuildConstruct_.HasCap)
    {
        /* Bottom */
        v0 = LengthDetail*(Detail+1);
        for (j = 1; j < Detail - 1; ++j)
        {
            v1 = LengthDetail*(Detail+1)+j;
            v2 = LengthDetail*(Detail+1)+j+1;
            
            Surface_->addTriangle(v0, v1, v2);
        }
        
        /* Top */
        v0 = (LengthDetail+1)*(Detail+1);
        for (j = 1; j < Detail - 1; ++j)
        {
            v1 = (LengthDetail+1)*(Detail+1)+j;
            v2 = (LengthDetail+1)*(Detail+1)+j+1;
            
            Surface_->addTriangle(v2, v1, v0);
        }
    }
}

#endif

#ifdef SP_COMPILE_WITH_PRIMITIVE_PIPE

// ! (incomplete) to many vertices costs !

void BasicMeshGenerator::createPipe()
{
    /* Temporary variables */
    const s32 Detail    = math::MinMax(BuildConstruct_.SegmentsVert, 3, 360);
    const f32 Radius1   = BuildConstruct_.RadiusInner;
    const f32 Radius2   = BuildConstruct_.RadiusOuter;
    
    const f32 c = 360.0f / Detail;
    
    for (f32 i = 0; i < 360; i += c)
    {
        /* Body outside */
        Surface_->addVertex(dim::vector3df(SIN(i  )*Radius1,  0.5f, COS(i  )*Radius1), dim::point2df((360-i  )/360*3, 0));
        Surface_->addVertex(dim::vector3df(SIN(i+c)*Radius1,  0.5f, COS(i+c)*Radius1), dim::point2df((360-i-c)/360*3, 0));
        Surface_->addVertex(dim::vector3df(SIN(i+c)*Radius1, -0.5f, COS(i+c)*Radius1), dim::point2df((360-i-c)/360*3, 1));
        Surface_->addVertex(dim::vector3df(SIN(i  )*Radius1, -0.5f, COS(i  )*Radius1), dim::point2df((360-i  )/360*3, 1));
        Surface_->addTriangle(2, 1, 0); Surface_->addTriangle(3, 2, 0);
        Surface_->addIndexOffset(4);
        
        /* Body inside */
        Surface_->addVertex(dim::vector3df(SIN(i  )*Radius2,  0.5f, COS(i  )*Radius2), dim::point2df((360-i  )/360*3, 0));
        Surface_->addVertex(dim::vector3df(SIN(i+c)*Radius2,  0.5f, COS(i+c)*Radius2), dim::point2df((360-i-c)/360*3, 0));
        Surface_->addVertex(dim::vector3df(SIN(i+c)*Radius2, -0.5f, COS(i+c)*Radius2), dim::point2df((360-i-c)/360*3, 1));
        Surface_->addVertex(dim::vector3df(SIN(i  )*Radius2, -0.5f, COS(i  )*Radius2), dim::point2df((360-i  )/360*3, 1));
        Surface_->addTriangle(0, 1, 2); Surface_->addTriangle(0, 2, 3);
        Surface_->addIndexOffset(4);
        
        if (BuildConstruct_.HasCap)
        {
            /* Cap of top */
            Surface_->addVertex(dim::vector3df(SIN(i  )*Radius1,  0.5f, COS(i  )*Radius1), dim::point2df(0.5f+SIN(i  )*Radius1, 0.5f-COS(i  )*Radius1));
            Surface_->addVertex(dim::vector3df(SIN(i+c)*Radius1,  0.5f, COS(i+c)*Radius1), dim::point2df(0.5f+SIN(i+c)*Radius1, 0.5f-COS(i+c)*Radius1));
            Surface_->addVertex(dim::vector3df(SIN(i+c)*Radius2,  0.5f, COS(i+c)*Radius2), dim::point2df(0.5f+SIN(i+c)*Radius2, 0.5f-COS(i+c)*Radius2));
            Surface_->addVertex(dim::vector3df(SIN(i  )*Radius2,  0.5f, COS(i  )*Radius2), dim::point2df(0.5f+SIN(i  )*Radius2, 0.5f-COS(i  )*Radius2));
            Surface_->addTriangle(0, 1, 2); Surface_->addTriangle(0, 2, 3);
            Surface_->addIndexOffset(4);
            
            /* Cap bottom */
            Surface_->addVertex(dim::vector3df(SIN(i  )*Radius1, -0.5f, COS(i  )*Radius1), dim::point2df(0.5f+SIN(i  )*Radius1, 0.5f+COS(i  )*Radius1));
            Surface_->addVertex(dim::vector3df(SIN(i+c)*Radius1, -0.5f, COS(i+c)*Radius1), dim::point2df(0.5f+SIN(i+c)*Radius1, 0.5f+COS(i+c)*Radius1));
            Surface_->addVertex(dim::vector3df(SIN(i+c)*Radius2, -0.5f, COS(i+c)*Radius2), dim::point2df(0.5f+SIN(i+c)*Radius2, 0.5f+COS(i+c)*Radius2));
            Surface_->addVertex(dim::vector3df(SIN(i  )*Radius2, -0.5f, COS(i  )*Radius2), dim::point2df(0.5f+SIN(i  )*Radius2, 0.5f+COS(i  )*Radius2));
            Surface_->addTriangle(2, 1, 0); Surface_->addTriangle(3, 2, 0);
            Surface_->addIndexOffset(4);
        }
    }
}

#endif

#ifdef SP_COMPILE_WITH_PRIMITIVE_PLANE

void BasicMeshGenerator::createPlane()
{
    /* Create the quad face */
    addQuadFace(
        dim::vector3df(-0.5f, 0.0f,  0.5f), dim::point2df(0, 0),
        dim::vector3df( 0.5f, 0.0f, -0.5f), dim::point2df(1, 1),
        dim::vector3df(1, 0, 0), dim::vector3df(0, 0, 1)
    );
}

#endif

#ifdef SP_COMPILE_WITH_PRIMITIVE_DISK

void BasicMeshGenerator::createDisk()
{
    /* Temporary variables */
    const s32 Detail    = math::MinMax(BuildConstruct_.SegmentsVert, 3, 360);
    const f32 Radius1   = BuildConstruct_.RadiusInner;
    const f32 Radius2   = BuildConstruct_.RadiusOuter;
    
    const f32 size = 360.0f / Detail;
    
    f32 x, y;
    
    if (BuildConstruct_.HasCap)
    {
        Surface_->addVertex(dim::vector3df(0.0f, 0.0f, 0.5f), dim::point2df(0.5f, 0.0f));
        
        x = SIN(size)*0.5f;
        y = COS(size)*0.5f;
        Surface_->addVertex(dim::vector3df(x, 0.0f, y), dim::point2df(0.5f+x, 0.5f-y));
        
        for (s32 i = 2; i < Detail; ++i)
        {
            x = SIN(i*size)*0.5f;
            y = COS(i*size)*0.5f;
            Surface_->addVertex(dim::vector3df(x, 0.0f, y), dim::point2df(0.5f+x, 0.5f-y));
            Surface_->addTriangle(0, i-1, i);
        }
    }
    else
    {
        for (s32 i = 0; i < Detail; ++i)
        {
            /* Vertices - outside */
            x = SIN(i*size)*Radius1;
            y = COS(i*size)*Radius1;
            Surface_->addVertex(dim::vector3df(x, 0.0f, y), dim::point2df(0.5f+x, 0.5f-y));
            
            x = SIN((i+1)*size)*Radius1;
            y = COS((i+1)*size)*Radius1;
            Surface_->addVertex(dim::vector3df(x, 0.0f, y), dim::point2df(0.5f+x, 0.5f-y));
            
            /* Vertices - inside */
            x = SIN((i+1)*size)*Radius2;
            y = COS((i+1)*size)*Radius2;
            Surface_->addVertex(dim::vector3df(x, 0.0f, y), dim::point2df(0.5f+x, 0.5f-y));
            
            x = SIN(i*size)*Radius2;
            y = COS(i*size)*Radius2;
            Surface_->addVertex(dim::vector3df(x, 0.0f, y), dim::point2df(0.5f+x, 0.5f-y));
            
            /* Triangles */
            Surface_->addTriangle(0, 1, 2);
            Surface_->addTriangle(0, 2, 3);
            Surface_->addIndexOffset(4);
        }
    }
}

#endif

#ifdef SP_COMPILE_WITH_PRIMITIVE_TETRAHEDRON

void BasicMeshGenerator::createTetrahedron()
{
    /* Temporary variables */
    const f32 size = sqrtf(0.5f);
    
    const dim::vector3df Vertices[4] =
    {
        dim::vector3df( size,  size, -size),
        dim::vector3df(-size,  size,  size),
        dim::vector3df( size, -size,  size),
        dim::vector3df(-size, -size, -size)
    };
    
    const dim::point2df TexCoords[3] =
    {
        dim::point2df(0.5f, 0.0f),
        dim::point2df(1.0f, 1.0f),
        dim::point2df(0.0f, 1.0f),
    };
    
    /* Create the triangles */
    addFace(Vertices[1], Vertices[0], Vertices[3], TexCoords[0], TexCoords[1], TexCoords[2]);
    addFace(Vertices[3], Vertices[0], Vertices[2], TexCoords[0], TexCoords[1], TexCoords[2]);
    addFace(Vertices[2], Vertices[0], Vertices[1], TexCoords[0], TexCoords[1], TexCoords[2]);
    addFace(Vertices[2], Vertices[1], Vertices[3], TexCoords[0], TexCoords[1], TexCoords[2]);
}

#endif

#ifdef SP_COMPILE_WITH_PRIMITIVE_CUBOCTAHEDRON

void BasicMeshGenerator::createCuboctahedron()
{
    /* === Quadrangles === */
    
    /* Back */
    Surface_->addVertex(dim::vector3df( 0.0f,  0.5f,  0.5f), dim::point2df(0.5, 0.0));
    Surface_->addVertex(dim::vector3df( 0.5f,  0.0f,  0.5f), dim::point2df(1.0, 0.5));
    Surface_->addVertex(dim::vector3df( 0.0f, -0.5f,  0.5f), dim::point2df(0.5, 1.0));
    Surface_->addVertex(dim::vector3df(-0.5f,  0.0f,  0.5f), dim::point2df(0.0, 0.5));
    Surface_->addTriangle(2, 1, 0); Surface_->addTriangle(3, 2, 0);
    Surface_->addIndexOffset(4);
    
    /* Front */
    Surface_->addVertex(dim::vector3df( 0.0f,  0.5f, -0.5f), dim::point2df(0.5, 0.0));
    Surface_->addVertex(dim::vector3df( 0.5f,  0.0f, -0.5f), dim::point2df(1.0, 0.5));
    Surface_->addVertex(dim::vector3df( 0.0f, -0.5f, -0.5f), dim::point2df(0.5, 1.0));
    Surface_->addVertex(dim::vector3df(-0.5f,  0.0f, -0.5f), dim::point2df(0.0, 0.5));
    Surface_->addTriangle(0, 1, 2); Surface_->addTriangle(0, 2, 3);
    Surface_->addIndexOffset(4);
    
    /* Top */
    Surface_->addVertex(dim::vector3df( 0.0f,  0.5f,  0.5f), dim::point2df(0.5, 0.0));
    Surface_->addVertex(dim::vector3df( 0.5f,  0.5f,  0.0f), dim::point2df(1.0, 0.5));
    Surface_->addVertex(dim::vector3df( 0.0f,  0.5f, -0.5f), dim::point2df(0.5, 1.0));
    Surface_->addVertex(dim::vector3df(-0.5f,  0.5f,  0.0f), dim::point2df(0.0, 0.5));
    Surface_->addTriangle(0, 1, 2); Surface_->addTriangle(0, 2, 3);
    Surface_->addIndexOffset(4);
    
    /* Bottom */
    Surface_->addVertex(dim::vector3df( 0.0f, -0.5f,  0.5f), dim::point2df(0.5, 0.0));
    Surface_->addVertex(dim::vector3df( 0.5f, -0.5f,  0.0f), dim::point2df(1.0, 0.5));
    Surface_->addVertex(dim::vector3df( 0.0f, -0.5f, -0.5f), dim::point2df(0.5, 1.0));
    Surface_->addVertex(dim::vector3df(-0.5f, -0.5f,  0.0f), dim::point2df(0.0, 0.5));
    Surface_->addTriangle(2, 1, 0); Surface_->addTriangle(3, 2, 0);
    Surface_->addIndexOffset(4);
    
    /* Left */
    Surface_->addVertex(dim::vector3df(-0.5f,  0.5f,  0.0f), dim::point2df(0.5, 0.0));
    Surface_->addVertex(dim::vector3df(-0.5f,  0.0f, -0.5f), dim::point2df(1.0, 0.5));
    Surface_->addVertex(dim::vector3df(-0.5f, -0.5f,  0.0f), dim::point2df(0.5, 1.0));
    Surface_->addVertex(dim::vector3df(-0.5f,  0.0f,  0.5f), dim::point2df(0.0, 0.5));
    Surface_->addTriangle(0, 1, 2); Surface_->addTriangle(0, 2, 3);
    Surface_->addIndexOffset(4);
    
    /* Right */
    Surface_->addVertex(dim::vector3df( 0.5f,  0.5f,  0.0f), dim::point2df(0.5, 0.0));
    Surface_->addVertex(dim::vector3df( 0.5f,  0.0f, -0.5f), dim::point2df(1.0, 0.5));
    Surface_->addVertex(dim::vector3df( 0.5f, -0.5f,  0.0f), dim::point2df(0.5, 1.0));
    Surface_->addVertex(dim::vector3df( 0.5f,  0.0f,  0.5f), dim::point2df(0.0, 0.5));
    Surface_->addTriangle(2, 1, 0); Surface_->addTriangle(3, 2, 0);
    Surface_->addIndexOffset(4);
    
    /* === Trianlges - top (front-right, front-left, back-right, back-left) === */
    
    Surface_->addVertex(dim::vector3df( 0.0f,  0.5f, -0.5f), dim::point2df(0.5, 0.0));
    Surface_->addVertex(dim::vector3df( 0.5f,  0.5f,  0.0f), dim::point2df(1.0, 0.0));
    Surface_->addVertex(dim::vector3df( 0.5f,  0.0f, -0.5f), dim::point2df(1.0, 0.5));
    Surface_->addTriangle(0, 1, 2); Surface_->addIndexOffset(3);
    
    Surface_->addVertex(dim::vector3df( 0.0f,  0.5f, -0.5f), dim::point2df(0.5, 0.0));
    Surface_->addVertex(dim::vector3df(-0.5f,  0.5f,  0.0f), dim::point2df(0.0, 0.0));
    Surface_->addVertex(dim::vector3df(-0.5f,  0.0f, -0.5f), dim::point2df(0.0, 0.5));
    Surface_->addTriangle(2, 1, 0); Surface_->addIndexOffset(3);
    
    Surface_->addVertex(dim::vector3df( 0.0f,  0.5f,  0.5f), dim::point2df(0.5, 0.0));
    Surface_->addVertex(dim::vector3df( 0.5f,  0.5f,  0.0f), dim::point2df(1.0, 0.0));
    Surface_->addVertex(dim::vector3df( 0.5f,  0.0f,  0.5f), dim::point2df(1.0, 0.5));
    Surface_->addTriangle(2, 1, 0); Surface_->addIndexOffset(3);
    
    Surface_->addVertex(dim::vector3df( 0.0f,  0.5f,  0.5f), dim::point2df(0.5, 0.0));
    Surface_->addVertex(dim::vector3df(-0.5f,  0.5f,  0.0f), dim::point2df(0.0, 0.0));
    Surface_->addVertex(dim::vector3df(-0.5f,  0.0f,  0.5f), dim::point2df(0.0, 0.5));
    Surface_->addTriangle(0, 1, 2); Surface_->addIndexOffset(3);
    
    /* === Trianlges - bottom (front-right, front-left, back-right, back-left) === */
    
    Surface_->addVertex(dim::vector3df( 0.0f, -0.5f, -0.5f), dim::point2df(0.5, 1.0));
    Surface_->addVertex(dim::vector3df( 0.5f, -0.5f,  0.0f), dim::point2df(1.0, 1.0));
    Surface_->addVertex(dim::vector3df( 0.5f,  0.0f, -0.5f), dim::point2df(1.0, 0.5));
    Surface_->addTriangle(2, 1, 0); Surface_->addIndexOffset(3);
    
    Surface_->addVertex(dim::vector3df( 0.0f, -0.5f, -0.5f), dim::point2df(0.5, 1.0));
    Surface_->addVertex(dim::vector3df(-0.5f, -0.5f,  0.0f), dim::point2df(0.0, 1.0));
    Surface_->addVertex(dim::vector3df(-0.5f,  0.0f, -0.5f), dim::point2df(0.0, 0.5));
    Surface_->addTriangle(0, 1, 2); Surface_->addIndexOffset(3);
    
    Surface_->addVertex(dim::vector3df( 0.0f, -0.5f,  0.5f), dim::point2df(0.5, 1.0));
    Surface_->addVertex(dim::vector3df( 0.5f, -0.5f,  0.0f), dim::point2df(1.0, 1.0));
    Surface_->addVertex(dim::vector3df( 0.5f,  0.0f,  0.5f), dim::point2df(1.0, 0.5));
    Surface_->addTriangle(0, 1, 2); Surface_->addIndexOffset(3);
    
    Surface_->addVertex(dim::vector3df( 0.0f, -0.5f,  0.5f), dim::point2df(0.5, 1.0));
    Surface_->addVertex(dim::vector3df(-0.5f, -0.5f,  0.0f), dim::point2df(0.0, 1.0));
    Surface_->addVertex(dim::vector3df(-0.5f,  0.0f,  0.5f), dim::point2df(0.0, 0.5));
    Surface_->addTriangle(2, 1, 0); Surface_->addIndexOffset(3);
}

#endif

#ifdef SP_COMPILE_WITH_PRIMITIVE_DODECAHEDRON

void BasicMeshGenerator::createDodecahedron()
{
    /* Temporary variables */
    const f32 Radius = BuildConstruct_.RadiusInner;
    
    /*
     * Helpful description of dodecahedron construction:
     * http://geometrie.uibk.ac.at/institutsangehoerige/schroecker/fh/dodekaeder-pres.pdf
     *
     * size = ((1 + sqrt(5))/2 - 1) / 2
     */
    static const f32 size = 0.3090169943749473f;
    
    const dim::vector3df Vertices[20] =
    {
        // Box body
        dim::vector3df(-Radius, -Radius, -Radius),
        dim::vector3df( Radius, -Radius, -Radius),
        dim::vector3df( Radius, -Radius,  Radius),
        dim::vector3df(-Radius, -Radius,  Radius),
        dim::vector3df(-Radius,  Radius, -Radius),
        dim::vector3df( Radius,  Radius, -Radius),
        dim::vector3df( Radius,  Radius,  Radius),
        dim::vector3df(-Radius,  Radius,  Radius),
        
        // Extensions
        dim::vector3df(-size, 0.0f, -0.5f - size) * Radius * 2,
        dim::vector3df( size, 0.0f, -0.5f - size) * Radius * 2,
        dim::vector3df(-size, 0.0f,  0.5f + size) * Radius * 2,
        dim::vector3df( size, 0.0f,  0.5f + size) * Radius * 2,
        dim::vector3df( 0.5f + size, -size, 0.0f) * Radius * 2,
        dim::vector3df( 0.5f + size,  size, 0.0f) * Radius * 2,
        dim::vector3df(-0.5f - size, -size, 0.0f) * Radius * 2,
        dim::vector3df(-0.5f - size,  size, 0.0f) * Radius * 2,
        dim::vector3df(0.0f, -0.5f - size, -size) * Radius * 2,
        dim::vector3df(0.0f, -0.5f - size,  size) * Radius * 2,
        dim::vector3df(0.0f,  0.5f + size, -size) * Radius * 2,
        dim::vector3df(0.0f,  0.5f + size,  size) * Radius * 2,
    };
    
    /* Create the triangles */
    addFace(Vertices, 0, 18, 5,  9,  8, 4);
    addFace(Vertices, 0, 16, 0,  8,  9, 1);
    addFace(Vertices, 0, 19, 7, 10, 11, 6);
    addFace(Vertices, 0, 17, 2, 11, 10, 3);
    addFace(Vertices, 0,  9, 5, 13, 12, 1);
    addFace(Vertices, 0, 11, 2, 12, 13, 6);
    addFace(Vertices, 0,  8, 0, 14, 15, 4);
    addFace(Vertices, 0, 10, 7, 15, 14, 3);
    addFace(Vertices, 0, 13, 5, 18, 19, 6);
    addFace(Vertices, 0, 15, 7, 19, 18, 4);
    addFace(Vertices, 0, 12, 2, 17, 16, 1);
    addFace(Vertices, 0, 14, 0, 16, 17, 3);
}

#endif

#ifdef SP_COMPILE_WITH_PRIMITIVE_OCTAHEDRON

void BasicMeshGenerator::createOctahedron()
{
    static const f32 size = 0.707106781f;
    
    /* Vertices */
    Surface_->addVertex(dim::vector3df(-0.5f,  0.0f,  0.5f), dim::point2df(1.0f, 1.0f));
    Surface_->addVertex(dim::vector3df( 0.5f,  0.0f,  0.5f), dim::point2df(0.0f, 1.0f));
    Surface_->addVertex(dim::vector3df( 0.5f,  0.0f, -0.5f), dim::point2df(1.0f, 1.0f));
    Surface_->addVertex(dim::vector3df(-0.5f,  0.0f, -0.5f), dim::point2df(0.0f, 1.0f));
    Surface_->addVertex(dim::vector3df( 0.0f,  size,  0.0f), dim::point2df(0.5f, 0.0f));
    Surface_->addVertex(dim::vector3df( 0.0f, -size,  0.0f), dim::point2df(0.5f, 0.0f));
    
    /* Triangles */
    Surface_->addTriangle(3, 4, 2);
    Surface_->addTriangle(3, 2, 5);
    Surface_->addTriangle(0, 3, 5);
    Surface_->addTriangle(0, 4, 3);
    Surface_->addTriangle(1, 4, 0);
    Surface_->addTriangle(1, 0, 5);
    Surface_->addTriangle(2, 4, 1);
    Surface_->addTriangle(2, 1, 5);
}

#endif

#ifdef SP_COMPILE_WITH_PRIMITIVE_ICOSAHEDRON

void BasicMeshGenerator::createIcosahedron()
{
    BuildConstruct_.SegmentsVert = 1;
    BuildConstruct_.SegmentsHorz = 1;
    createIcosphere();
}

#endif

#ifdef SP_COMPILE_WITH_PRIMITIVE_TEAPOT

#   ifdef SP_COMPILE_WITH_PRIMITIVE_TEAPOT_DYNAMIC
extern dim::vector3df __spTeapotPatchesData[32][4][4];
#   else
extern const f32 __spTeapotVertices[];
extern const s32 __spTeapotIndices[];
#   endif

void BasicMeshGenerator::createTeapot()
{
    #ifdef SP_COMPILE_WITH_PRIMITIVE_TEAPOT_DYNAMIC
    
    const s32 Segments = math::MinMax(BuildConstruct_.SegmentsVert, 1, 100);
    
    dim::vector3df Patches[4][4];
    
    for (s32 i = 0; i < 32; ++i)
    {
        for (s32 j = 0, k; j < 4; ++j)
            for (k = 0; k < 4; ++k)
                Patches[j][k] = __spTeapotPatchesData[i][j][k] * 0.02 * BuildConstruct_.Radius1;
        
        createBezierPatchFace(Mesh_, 0, Patches, Segments);
    }
    
    Mesh_->meshTurn(dim::vector3df(-90, 0, 0));
    
    #else
    
    const s32 VerticesCount = 1178;
    const s32 TriangleCount = 2256;
    
    /* Vertices */
    for (s32 i = 0; i < VerticesCount; ++i)
    {
        Surface_->addVertex(
            dim::vector3df(__spTeapotVertices[i*3+0], __spTeapotVertices[i*3+1], __spTeapotVertices[i*3+2]),
            dim::point2df(__spTeapotVertices[i*3+0], __spTeapotVertices[i*3+1])
        );
    }
    
    /* Triangles */
    for (s32 i = 0; i < TriangleCount; ++i)
        Surface_->addTriangle(__spTeapotIndices[i*3+0], __spTeapotIndices[i*3+1], __spTeapotIndices[i*3+2]);
    
    #endif
}

#endif

#ifdef SP_COMPILE_WITH_PRIMITIVE_WIRE_CUBE

void BasicMeshGenerator::createWireCube()
{
    const f32 Radius = BuildConstruct_.RadiusInner;
    
    /* Configure wire mesh */
    video::MaterialStates* Material = Mesh_->getMaterial();
    
    Material->setLighting(false);
    Material->setDiffuseColor(0);
    Material->setAmbientColor(255);
    
    Surface_->setPrimitiveType(video::PRIMITIVE_LINES);
    
    /* Build wire mesh */
    Surface_->addVertices(8);
    
    for (s32 i = 0; i < 8; ++i)
    {
        Surface_->setVertexCoord(i, dim::vector3df(
            i % 8 >= 4 ? Radius : -Radius,
            i % 4 >= 2 ? Radius : -Radius,
            i % 2 >= 1 ? Radius : -Radius
        ));
        Surface_->setVertexColor(i, video::color(255));
    }
    
    for (s32 i = 0; i < 4; ++i)
    {
        Surface_->addPrimitiveIndex(i*2), Surface_->addPrimitiveIndex(i*2+1);
        Surface_->addPrimitiveIndex(i), Surface_->addPrimitiveIndex(i+4);
    }
    
    Surface_->addPrimitiveIndex(0), Surface_->addPrimitiveIndex(2);
    Surface_->addPrimitiveIndex(1), Surface_->addPrimitiveIndex(3);
    Surface_->addPrimitiveIndex(4), Surface_->addPrimitiveIndex(6);
    Surface_->addPrimitiveIndex(5), Surface_->addPrimitiveIndex(7);
}

#endif


/*
 * ========== Super shape ==========
 * (Special thanks to the 'Blitz3D' board)
 * -> http://astronomy.swin.edu.au/~pbourke/curves/supershape/
 * -> http://astronomy.swin.edu.au/~pbourke/surfaces/supershape3d/
 * "This complex model can looks like a sphere but with other values it can looks
 * like a crazy object you dont know what it is! :p"
 */

void BasicMeshGenerator::getSuperShapeModel(const ESuperShapeModels Type, f32 ValueList[12])
{
    switch (Type)
    {
        case SUPERSHAPE_SMALL_CRYSTAL1:
        {
            f32 GenList[12] = { 8.0f, 60.0f, 100.0f, 30.0f, 1.0f, 1.0f, 12.0f, 10.0f, 10.0f, 10.0f, 1.0f, 1.0f };
            for (s32 i = 0; i < 12; ++i) ValueList[i] = GenList[i];
        }
        break;
        
        case SUPERSHAPE_SMALL_CRYSTAL2:
        {
            f32 GenList[12] = { 8.0f, 60.0f, 100.0f, 30.0f, 1.0f, 1.0f, 12.0f, 10.0f, 10.0f, 10.0f, 1.0f, 0.1f };
            for (s32 i = 0; i < 12; ++i) ValueList[i] = GenList[i];
        }
        break;
        
        case SUPERSHAPE_SMALL_STAR1:
        {
            f32 GenList[12] = { 80.0f, 43.7f, 75.4f, 88.55f, 1.6f, 1.95f, 1.16f, 81.0f, 2.0f, 4.0f, 1.8f, 1.4f };
            for (s32 i = 0; i < 12; ++i) ValueList[i] = GenList[i];
        }
        break;
        
        case SUPERSHAPE_SMALL_STAR2:
        {
            f32 GenList[12] = { 13.5f, 42.7f, 82.8f, 22.1f, 1.92f, 1.1f, 1.96f, 12.56f, 1.1f, 2.0f, 1.58f, 1.56f };
            for (s32 i = 0; i < 12; ++i) ValueList[i] = GenList[i];
        }
        break;
        
        case SUPERSHAPE_SMALL_STAR3:
        {
            f32 GenList[12] = { 12.0f, 33.65f, 76.67f, 53.5f, 2.0f, 1.3f, 1.0f, 77.2f, 7.22f, 5.0f, 1.53f, 1.29f };
            for (s32 i = 0; i < 12; ++i) ValueList[i] = GenList[i];
        }
        break;
        
        case SUPERSHAPE_SMALL_STAR4:
        {
            f32 GenList[12] = { 5.8f, 42.66f, 70.4f, 86.24f, 1.47f, 1.11f, 1.83f, 45.5f, 9.8f, 9.6f, 1.4f, 1.0f };
            for (s32 i = 0; i < 12; ++i) ValueList[i] = GenList[i];
        }
        break;
        
        case SUPERSHAPE_BIG_STAR1:
        {
            f32 GenList[12] = { 100.0f, 7.0f, 50, 12.4f, 1.5f, 1.6f, 1.1f, 54.0f, 7.8f, 1.2f, 1.8f, 1.0f };
            for (s32 i = 0; i < 12; ++i) ValueList[i] = GenList[i];
        }
        break;
        
        case SUPERSHAPE_URCHIN1:
        {
            f32 GenList[12] = { 17.0f, 51.1f, 31.4f, 30.3f, 1.0f, 1.1f, 1.2f, 69.2f, 1.3f, 7.7f, 1.48f, 1.6f };
            for (s32 i = 0; i < 12; ++i) ValueList[i] = GenList[i];
        }
        break;
        
        case SUPERSHAPE_CUSHION1:
        {
            f32 GenList[12] = { 86.0f, 56.3f, 56.5f, 48.4f, 1.8f, 2.0f, 1.1f, 89.7f, 4.7f, 5.0f, 1.8f, 1.34f };
            for (s32 i = 0; i < 12; ++i) ValueList[i] = GenList[i];
        }
        break;
        
        case SUPERSHAPE_RANDOM:
        {
            const f32 GenList[12] = {
                math::Randomizer::randFloat(1, 100), math::Randomizer::randFloat(1, 100), math::Randomizer::randFloat(1, 100),
                math::Randomizer::randFloat(1, 100), math::Randomizer::randFloat(1,   2), math::Randomizer::randFloat(1,   2),
                math::Randomizer::randFloat(1,   2), math::Randomizer::randFloat(1, 100), math::Randomizer::randFloat(1,  10),
                math::Randomizer::randFloat(1,  10), math::Randomizer::randFloat(1,   2), math::Randomizer::randFloat(1,   2)
            };
            for (s32 i = 0; i < 12; ++i) ValueList[i] = GenList[i];
        }
        break;
    }
}

#ifdef SP_COMPILE_WITH_PRIMITIVE_SUPERSHAPE

void BasicMeshGenerator::createSuperShapeSurface(const f32 ValueList[12], const s32 Segments)
{
    /* Temporary variables */
    s32 i, j;
    
    f32 r1, r2, phi, theta, alpha;
    f32 x, y, z;
    
    alpha = 360.0f / Segments;
    phi = -90.0f;
    
    r2 = computeSuperShapeNextFrame(ValueList[6], ValueList[7], ValueList[8], ValueList[9], ValueList[10], ValueList[11], phi);
    
    /* Compute & create the model */
    for (j = 0; j < Segments; ++j)
    {
        theta = j*alpha - 180;
        
        r1 = computeSuperShapeNextFrame(ValueList[0], ValueList[1], ValueList[2], ValueList[3], ValueList[4], ValueList[5], theta);
        
        x = r1*COS(theta)*r2*COS(phi);
        y = r1*SIN(theta)*r2*COS(phi);
        z = r2*SIN(phi);
        
        Surface_->addVertex(dim::vector3df(x, y, z), dim::point2df(x, y));
    }
    
    for (i = 1; i <= Segments/2; ++i)
    {
        phi = i*alpha - 90;
        
        r2 = computeSuperShapeNextFrame(ValueList[6], ValueList[7], ValueList[8], ValueList[9], ValueList[10], ValueList[11], phi);
        
        for (j = 0; j < Segments; ++j)
        {
            theta = j*alpha - 180;
            
            r1 = computeSuperShapeNextFrame(ValueList[0], ValueList[1], ValueList[2], ValueList[3], ValueList[4], ValueList[5], theta);
            
            x = r1*COS(theta)*r2*COS(phi);
            y = r1*SIN(theta)*r2*COS(phi);
            z = r2*SIN(phi);
            
            Surface_->addVertex(dim::vector3df(x, y, z), dim::point2df(x, y));
        }
        
        for (j = 0; j < Segments; ++j)
        {
            Surface_->addTriangle( (i-1)*Segments+j, (i-1)*Segments+((j+1)%Segments) , i*Segments+((j+1)%Segments) );
            Surface_->addTriangle( (i-1)*Segments+j, i*Segments+((j+1)%Segments) , i*Segments+j );
        }
    }
}

f32 BasicMeshGenerator::computeSuperShapeNextFrame(f32 m, f32 n1, f32 n2, f32 n3, f32 a, f32 b, f32 phi)
{
    return
        pow( pow( math::Abs<f32>( COS(m*phi/4)/a ), n2 ) +
        pow(      math::Abs<f32>( SIN(m*phi/4)/b ), n3 ), (1.0f/(-n1)) );
}

#endif


/* === Other modeling functions === */

void BasicMeshGenerator::addVertex(f32 x, f32 y, f32 z, f32 u, f32 v, f32 w)
{
    Surface_->addVertex(dim::vector3df(x, y, z), dim::vector3df(u, v, w));
}

void BasicMeshGenerator::addFace(u32 v0, u32 v1, u32 v2)
{
    Surface_->addTriangle(v0, v1, v2);
}
void BasicMeshGenerator::addFace(u32 v0, u32 v1, u32 v2, u32 v3)
{
    Surface_->addTriangle(v0, v1, v2);
    Surface_->addTriangle(v0, v2, v3);
}
void BasicMeshGenerator::addFace(u32 v0, u32 v1, u32 v2, u32 v3, u32 v4)
{
    Surface_->addTriangle(v0, v1, v2);
    Surface_->addTriangle(v0, v2, v3);
    Surface_->addTriangle(v0, v3, v4);
}

void BasicMeshGenerator::addFace(
    const dim::vector3df &v0, const dim::vector3df &v1, const dim::vector3df &v2,
    const dim::point2df &t0, const dim::point2df &t1, const dim::point2df &t2)
{
    Surface_->addVertex(v0, t0);
    Surface_->addVertex(v1, t1);
    Surface_->addVertex(v2, t2);
    
    Surface_->addTriangle(0, 1, 2);
    
    Surface_->addIndexOffset(3);
}

void BasicMeshGenerator::addFace(
    const dim::vector3df &v0, const dim::vector3df &v1, const dim::vector3df &v2)
{
    const dim::vector3df AbsNormal(math::getNormalVector(v0, v1, v2).getAbs());
    
    dim::point2df t0, t1, t2;
    
    if (AbsNormal.X >= AbsNormal.Y && AbsNormal.X >= AbsNormal.Z)
    {
        t0 = dim::point2df(v0.Z, -v0.Y);
        t1 = dim::point2df(v1.Z, -v1.Y);
        t2 = dim::point2df(v2.Z, -v2.Y);
    }
    else if (AbsNormal.Y >= AbsNormal.X && AbsNormal.Y >= AbsNormal.Z)
    {
        t0 = dim::point2df(v0.X, -v0.Z);
        t1 = dim::point2df(v1.X, -v1.Z);
        t2 = dim::point2df(v2.X, -v2.Z);
    }
    else
    {
        t0 = dim::point2df(v0.X, -v0.Y);
        t1 = dim::point2df(v1.X, -v1.Y);
        t2 = dim::point2df(v2.X, -v2.Y);
    }
    
    addFace(v0, v1, v2, t0, t1, t2);
}

void BasicMeshGenerator::addFace(
    const dim::vector3df* Vertices, const dim::point2df* TexCoords,
    u32 v0, u32 v1, u32 v2, u32 v3, u32 v4)
{
    if (Vertices)
    {
        if (TexCoords)
        {
            Surface_->addVertex(Vertices[v0], TexCoords[v0]);
            Surface_->addVertex(Vertices[v1], TexCoords[v1]);
            Surface_->addVertex(Vertices[v2], TexCoords[v2]);
            Surface_->addVertex(Vertices[v3], TexCoords[v3]);
            Surface_->addVertex(Vertices[v4], TexCoords[v4]);
        }
        else
        {
            Surface_->addVertex(Vertices[v0], dim::point2df(0.5f, 0.0f));
            Surface_->addVertex(Vertices[v1], dim::point2df(1.0f, 0.4f));
            Surface_->addVertex(Vertices[v2], dim::point2df(0.7f, 1.0f));
            Surface_->addVertex(Vertices[v3], dim::point2df(0.2f, 1.0f));
            Surface_->addVertex(Vertices[v4], dim::point2df(0.0f, 0.4f));
        }
        
        Surface_->addTriangle(0, 1, 2);
        Surface_->addTriangle(0, 2, 3);
        Surface_->addTriangle(0, 3, 4);
        
        Surface_->addIndexOffset(5);
    }
    else
        addFace(v0, v1, v2, v3, v4);
}

void BasicMeshGenerator::addQuadFace(
    dim::vector3df v0, dim::point2df t0, dim::vector3df v1, dim::point2df t1,
    dim::vector3df diru, dim::vector3df dirv,
    bool FaceLinkCCW)
{
    /* Temporary variables */
    const s32 DetailVert = BuildConstruct_.SegmentsVert;
    const s32 DetailHorz = BuildConstruct_.SegmentsHorz;
    
    s32 x, y, c = 0;
    s32 i0, i1, i2, i3;
    
    dim::vector3df u, v;
    
    /* Create all vertices */
    for (y = 0; y <= DetailVert; ++y)
    {
        v = dirv * static_cast<f32>(y) / DetailVert;
        
        for (x = 0; x <= DetailHorz; ++x, ++c)
        {
            u = diru * static_cast<f32>(x) / DetailHorz;
            
            addVertex(
                // Coordinate
                v0.X + (v1.X - v0.X) * (u.X+v.X),
                v0.Y + (v1.Y - v0.Y) * (u.Y+v.Y),
                v0.Z + (v1.Z - v0.Z) * (u.Z+v.Z),
                // Texture coordinate
                t0.X + (t1.X - t0.X) * static_cast<f32>(x) / DetailHorz,
                t0.Y + (t1.Y - t0.Y) * static_cast<f32>(y) / DetailVert
            );
        }
    }
    
    /* Create all triangles */
    for (y = 0; y < DetailVert; ++y)
    {
        for (x = 0; x < DetailHorz; ++x)
        {
            /* Compute the vertex indices */
            i0 = y * ( DetailHorz + 1 ) + x;
            i1 = y * ( DetailHorz + 1 ) + x + 1;
            i2 = ( y + 1 ) * ( DetailHorz + 1 ) + x + 1;
            i3 = ( y + 1 ) * ( DetailHorz + 1 ) + x;
            
            /* Add the face connection */
            if (FaceLinkCCW)
                addFace(i3, i2, i1, i0);
            else
                addFace(i0, i1, i2, i3);
        }
    }
    
    Surface_->addIndexOffset(c);
}


#ifdef SP_COMPILE_WITH_PRIMITIVE_TEAPOT

#   ifndef SP_COMPILE_WITH_PRIMITIVE_TEAPOT_DYNAMIC

/*
 * ========== Teapot vertices: ==========
 */

static const f32 __spTeapotVertices[] = {

     0.678873,  0.330678,  0.000000,     0.654243,  0.330678,  0.187963,     0.645254,  0.358022,  0.185461,     0.669556,  0.358022,  0.000000,
     0.646650,  0.374428,  0.185850,     0.671003,  0.374428,  0.000000,     0.655751,  0.379897,  0.188383,     0.680435,  0.379897,  0.000000,
     0.669877,  0.374428,  0.192314,     0.695077,  0.374428,  0.000000,     0.686348,  0.358022,  0.196898,     0.712148,  0.358022,  0.000000,
     0.702484,  0.330678,  0.201389,     0.728873,  0.330678,  0.000000,     0.584502,  0.330678,  0.355704,     0.576441,  0.358022,  0.350969,
     0.577693,  0.374428,  0.351704,     0.585854,  0.379897,  0.356498,     0.598522,  0.374428,  0.363938,     0.613292,  0.358022,  0.372613,
     0.627762,  0.330678,  0.381111,     0.475873,  0.330678,  0.497000,     0.469258,  0.358022,  0.490385,     0.470285,  0.374428,  0.491412,
     0.476982,  0.379897,  0.498109,     0.487377,  0.374428,  0.508505,     0.499498,  0.358022,  0.520626,     0.511373,  0.330678,  0.532500,
     0.334576,  0.330678,  0.605630,     0.329842,  0.358022,  0.597569,     0.330577,  0.374428,  0.598820,     0.335370,  0.379897,  0.606982,
     0.342810,  0.374428,  0.619649,     0.351485,  0.358022,  0.634419,     0.359984,  0.330678,  0.648889,     0.166836,  0.330678,  0.675370,
     0.164334,  0.358022,  0.666381,     0.164722,  0.374428,  0.667777,     0.167255,  0.379897,  0.676878,     0.171187,  0.374428,  0.691004,
     0.175771,  0.358022,  0.707475,     0.180262,  0.330678,  0.723611,    -0.021127,  0.330678,  0.700000,    -0.021127,  0.358022,  0.690683,
    -0.021127,  0.374428,  0.692130,    -0.021127,  0.379897,  0.701563,    -0.021127,  0.374428,  0.716204,    -0.021127,  0.358022,  0.733276,
    -0.021127,  0.330678,  0.750000,    -0.224715,  0.330678,  0.675370,    -0.215631,  0.358022,  0.666381,    -0.211606,  0.374428,  0.667777,
    -0.211463,  0.379897,  0.676878,    -0.214020,  0.374428,  0.691004,    -0.218098,  0.358022,  0.707475,    -0.222516,  0.330678,  0.723611,
    -0.396831,  0.330678,  0.605630,    -0.383671,  0.358022,  0.597569,    -0.378758,  0.374428,  0.598820,    -0.380125,  0.379897,  0.606982,
    -0.385806,  0.374428,  0.619649,    -0.393832,  0.358022,  0.634419,    -0.402238,  0.330678,  0.648889,    -0.535002,  0.330678,  0.497000,
    -0.521278,  0.358022,  0.490385,    -0.517539,  0.374428,  0.491412,    -0.521346,  0.379897,  0.498109,    -0.530257,  0.374428,  0.508505,
    -0.541831,  0.358022,  0.520626,    -0.553627,  0.330678,  0.532500,    -0.636757,  0.330678,  0.355704,    -0.624483,  0.358022,  0.350969,
    -0.622910,  0.374428,  0.351704,    -0.629359,  0.379897,  0.356498,    -0.641146,  0.374428,  0.363938,    -0.655593,  0.358022,  0.372613,
    -0.670016,  0.330678,  0.381111,    -0.699623,  0.330678,  0.187963,    -0.689317,  0.358022,  0.185461,    -0.689830,  0.374428,  0.185850,
    -0.698396,  0.379897,  0.188382,    -0.712247,  0.374428,  0.192314,    -0.728617,  0.358022,  0.196898,    -0.744738,  0.330678,  0.201389,
    -0.721127,  0.330678,  0.000000,    -0.711810,  0.358022,  0.000000,    -0.713257,  0.374428,  0.000000,    -0.722690,  0.379897,  0.000000,
    -0.737331,  0.374428,  0.000000,    -0.754403,  0.358022,  0.000000,    -0.771127,  0.330678,  0.000000,    -0.696498,  0.330678, -0.187963,
    -0.687508,  0.358022, -0.185461,    -0.688904,  0.374428, -0.185850,    -0.698005,  0.379897, -0.188383,    -0.712131,  0.374428, -0.192314,
    -0.728602,  0.358022, -0.196898,    -0.744738,  0.330678, -0.201389,    -0.626757,  0.330678, -0.355704,    -0.618696,  0.358022, -0.350969,
    -0.619948,  0.374428, -0.351704,    -0.628109,  0.379897, -0.356498,    -0.640776,  0.374428, -0.363938,    -0.655546,  0.358022, -0.372613,
    -0.670016,  0.330678, -0.381111,    -0.518127,  0.330678, -0.497000,    -0.511512,  0.358022, -0.490385,    -0.512539,  0.374428, -0.491412,
    -0.519237,  0.379897, -0.498109,    -0.529632,  0.374428, -0.508505,    -0.541753,  0.358022, -0.520626,    -0.553627,  0.330678, -0.532500,
    -0.376831,  0.330678, -0.605630,    -0.372096,  0.358022, -0.597569,    -0.372832,  0.374428, -0.598820,    -0.377625,  0.379897, -0.606982,
    -0.385065,  0.374428, -0.619649,    -0.393740,  0.358022, -0.634419,    -0.402238,  0.330678, -0.648889,    -0.209090,  0.330678, -0.675370,
    -0.206588,  0.358022, -0.666381,    -0.206977,  0.374428, -0.667777,    -0.209510,  0.379897, -0.676878,    -0.213441,  0.374428, -0.691004,
    -0.218025,  0.358022, -0.707475,    -0.222516,  0.330678, -0.723611,    -0.021127,  0.330678, -0.700000,    -0.021127,  0.358022, -0.690683,
    -0.021127,  0.374428, -0.692130,    -0.021127,  0.379897, -0.701563,    -0.021127,  0.374428, -0.716204,    -0.021127,  0.358022, -0.733276,
    -0.021127,  0.330678, -0.750000,     0.166836,  0.330678, -0.675370,     0.164334,  0.358022, -0.666381,     0.164722,  0.374428, -0.667777,
     0.167255,  0.379897, -0.676878,     0.171187,  0.374428, -0.691004,     0.175771,  0.358022, -0.707475,     0.180262,  0.330678, -0.723611,
     0.334576,  0.330678, -0.605630,     0.329842,  0.358022, -0.597569,     0.330577,  0.374428, -0.598820,     0.335370,  0.379897, -0.606982,
     0.342810,  0.374428, -0.619649,     0.351485,  0.358022, -0.634419,     0.359984,  0.330678, -0.648889,     0.475873,  0.330678, -0.497000,
     0.469258,  0.358022, -0.490385,     0.470285,  0.374428, -0.491412,     0.476982,  0.379897, -0.498109,     0.487377,  0.374428, -0.508505,
     0.499498,  0.358022, -0.520626,     0.511373,  0.330678, -0.532500,     0.584502,  0.330678, -0.355704,     0.576441,  0.358022, -0.350969,
     0.577693,  0.374428, -0.351704,     0.585854,  0.379897, -0.356498,     0.598522,  0.374428, -0.363938,     0.613292,  0.358022, -0.372613,
     0.627762,  0.330678, -0.381111,     0.654243,  0.330678, -0.187963,     0.645254,  0.358022, -0.185461,     0.646650,  0.374428, -0.185850,
     0.655751,  0.379897, -0.188382,     0.669877,  0.374428, -0.192314,     0.686348,  0.358022, -0.196898,     0.702484,  0.330678, -0.201389,
     0.762227,  0.199602,  0.218016,     0.790794,  0.199602,  0.000000,     0.818619,  0.069567,  0.233711,     0.849243,  0.069567,  0.000000,
     0.868312, -0.058384,  0.247541,     0.900748, -0.058384,  0.000000,     0.907954, -0.183211,  0.258573,     0.941836, -0.183211,  0.000000,
     0.934196, -0.303870,  0.265877,     0.969035, -0.303870,  0.000000,     0.943688, -0.419322,  0.268519,     0.978873, -0.419322,  0.000000,
     0.681335,  0.199602,  0.412576,     0.731904,  0.069567,  0.442277,     0.776465, -0.058384,  0.468449,     0.812014, -0.183211,  0.489328,
     0.835546, -0.303870,  0.503149,     0.844058, -0.419322,  0.508148,     0.555337,  0.199602,  0.576464,     0.596836,  0.069567,  0.617963,
     0.633404, -0.058384,  0.654531,     0.662577, -0.183211,  0.683704,     0.681888, -0.303870,  0.703015,     0.688873, -0.419322,  0.710000,
     0.391449,  0.199602,  0.702462,     0.421150,  0.069567,  0.753032,     0.447322, -0.058384,  0.797593,     0.468201, -0.183211,  0.833141,
     0.482022, -0.303870,  0.856674,     0.487021, -0.419322,  0.865185,     0.196889,  0.199602,  0.783354,     0.212583,  0.069567,  0.839746,
     0.226413, -0.058384,  0.889439,     0.237446, -0.183211,  0.929081,     0.244750, -0.303870,  0.955323,     0.247391, -0.419322,  0.964815,
    -0.021127,  0.199602,  0.811921,    -0.021127,  0.069567,  0.870370,    -0.021127, -0.058384,  0.921875,    -0.021127, -0.183211,  0.962963,
    -0.021127, -0.303870,  0.990162,    -0.021127, -0.419322,  1.000000,    -0.239143,  0.199602,  0.783354,    -0.254838,  0.069567,  0.839746,
    -0.268668, -0.058384,  0.889439,    -0.279701, -0.183211,  0.929081,    -0.287004, -0.303870,  0.955323,    -0.289646, -0.419322,  0.964815,
    -0.433704,  0.199602,  0.702462,    -0.463404,  0.069567,  0.753032,    -0.489576, -0.058384,  0.797593,    -0.510455, -0.183211,  0.833141,
    -0.524276, -0.303870,  0.856674,    -0.529275, -0.419322,  0.865185,    -0.597591,  0.199602,  0.576464,    -0.639090,  0.069567,  0.617963,
    -0.675658, -0.058384,  0.654531,    -0.704831, -0.183211,  0.683704,    -0.724142, -0.303870,  0.703015,    -0.731127, -0.419322,  0.710000,
    -0.723589,  0.199602,  0.412576,    -0.774159,  0.069567,  0.442277,    -0.818720, -0.058384,  0.468449,    -0.854269, -0.183211,  0.489328,
    -0.877801, -0.303870,  0.503149,    -0.886312, -0.419322,  0.508148,    -0.804481,  0.199602,  0.218016,    -0.860873,  0.069567,  0.233711,
    -0.910566, -0.058384,  0.247540,    -0.950208, -0.183211,  0.258573,    -0.976450, -0.303870,  0.265877,    -0.985942, -0.419322,  0.268518,
    -0.833049,  0.199602,  0.000000,    -0.891498,  0.069567,  0.000000,    -0.943002, -0.058384,  0.000000,    -0.984090, -0.183211,  0.000000,
    -1.011290, -0.303870,  0.000000,    -1.021130, -0.419322,  0.000000,    -0.804481,  0.199602, -0.218016,    -0.860873,  0.069567, -0.233711,
    -0.910566, -0.058384, -0.247541,    -0.950208, -0.183211, -0.258573,    -0.976450, -0.303870, -0.265877,    -0.985942, -0.419322, -0.268519,
    -0.723589,  0.199602, -0.412576,    -0.774159,  0.069567, -0.442277,    -0.818720, -0.058384, -0.468449,    -0.854269, -0.183211, -0.489328,
    -0.877801, -0.303870, -0.503149,    -0.886312, -0.419322, -0.508148,    -0.597591,  0.199602, -0.576464,    -0.639090,  0.069567, -0.617963,
    -0.675658, -0.058384, -0.654531,    -0.704831, -0.183211, -0.683704,    -0.724142, -0.303870, -0.703015,    -0.731127, -0.419322, -0.710000,
    -0.433704,  0.199602, -0.702462,    -0.463404,  0.069567, -0.753032,    -0.489576, -0.058384, -0.797593,    -0.510455, -0.183211, -0.833141,
    -0.524276, -0.303870, -0.856674,    -0.529275, -0.419322, -0.865185,    -0.239143,  0.199602, -0.783354,    -0.254838,  0.069567, -0.839746,
    -0.268668, -0.058384, -0.889439,    -0.279701, -0.183211, -0.929081,    -0.287004, -0.303870, -0.955323,    -0.289646, -0.419322, -0.964815,
    -0.021127,  0.199602, -0.811921,    -0.021127,  0.069567, -0.870370,    -0.021127, -0.058384, -0.921875,    -0.021127, -0.183211, -0.962963,
    -0.021127, -0.303870, -0.990162,    -0.021127, -0.419322, -1.000000,     0.196889,  0.199602, -0.783354,     0.212583,  0.069567, -0.839746,
     0.226413, -0.058384, -0.889439,     0.237446, -0.183211, -0.929081,     0.244750, -0.303870, -0.955323,     0.247391, -0.419322, -0.964815,
     0.391449,  0.199602, -0.702462,     0.421150,  0.069567, -0.753032,     0.447322, -0.058384, -0.797593,     0.468201, -0.183211, -0.833141,
     0.482022, -0.303870, -0.856674,     0.487021, -0.419322, -0.865185,     0.555337,  0.199602, -0.576464,     0.596836,  0.069567, -0.617963,
     0.633404, -0.058384, -0.654531,     0.662577, -0.183211, -0.683704,     0.681888, -0.303870, -0.703015,     0.688873, -0.419322, -0.710000,
     0.681335,  0.199602, -0.412576,     0.731904,  0.069567, -0.442277,     0.776465, -0.058384, -0.468449,     0.812014, -0.183211, -0.489328,
     0.835546, -0.303870, -0.503149,     0.844058, -0.419322, -0.508148,     0.762227,  0.199602, -0.218016,     0.818619,  0.069567, -0.233711,
     0.868312, -0.058384, -0.247540,     0.907954, -0.183211, -0.258573,     0.934196, -0.303870, -0.265877,     0.943688, -0.419322, -0.268518,
     0.925821, -0.522620,  0.263546,     0.960354, -0.522620,  0.000000,     0.881153, -0.608211,  0.251115,     0.914058, -0.608211,  0.000000,
     0.823086, -0.677134,  0.234954,     0.853873, -0.677134,  0.000000,     0.765018, -0.730433,  0.218793,     0.793688, -0.730433,  0.000000,
     0.720351, -0.769148,  0.206361,     0.747391, -0.769148,  0.000000,     0.702484, -0.794322,  0.201389,     0.728873, -0.794322,  0.000000,
     0.828036, -0.522620,  0.498738,     0.787981, -0.608211,  0.475213,     0.735910, -0.677134,  0.444630,     0.683839, -0.730433,  0.414047,
     0.643784, -0.769148,  0.390521,     0.627762, -0.794322,  0.381111,     0.675725, -0.522620,  0.696852,     0.642854, -0.608211,  0.663981,
     0.600123, -0.677134,  0.621250,     0.557391, -0.730433,  0.578519,     0.524521, -0.769148,  0.545648,     0.511373, -0.794322,  0.532500,
     0.477611, -0.522620,  0.849163,     0.454085, -0.608211,  0.809108,     0.423502, -0.677134,  0.757037,     0.392919, -0.730433,  0.704966,
     0.369394, -0.769148,  0.664911,     0.359984, -0.794322,  0.648889,     0.242419, -0.522620,  0.946948,     0.229987, -0.608211,  0.902281,
     0.213826, -0.677134,  0.844213,     0.197666, -0.730433,  0.786145,     0.185234, -0.769148,  0.741478,     0.180262, -0.794322,  0.723611,
    -0.021127, -0.522620,  0.981482,    -0.021127, -0.608211,  0.935185,    -0.021127, -0.677134,  0.875000,    -0.021127, -0.730433,  0.814815,
    -0.021127, -0.769148,  0.768519,    -0.021127, -0.794322,  0.750000,    -0.284673, -0.522620,  0.946948,    -0.272242, -0.608211,  0.902281,
    -0.256081, -0.677134,  0.844213,    -0.239920, -0.730433,  0.786145,    -0.227489, -0.769148,  0.741478,    -0.222516, -0.794322,  0.723611,
    -0.519865, -0.522620,  0.849163,    -0.496340, -0.608211,  0.809108,    -0.465757, -0.677134,  0.757037,    -0.435174, -0.730433,  0.704966,
    -0.411649, -0.769148,  0.664911,    -0.402238, -0.794322,  0.648889,    -0.717979, -0.522620,  0.696852,    -0.685109, -0.608211,  0.663981,
    -0.642377, -0.677134,  0.621250,    -0.599646, -0.730433,  0.578519,    -0.566775, -0.769148,  0.545648,    -0.553627, -0.794322,  0.532500,
    -0.870290, -0.522620,  0.498738,    -0.830236, -0.608211,  0.475213,    -0.778164, -0.677134,  0.444630,    -0.726093, -0.730433,  0.414047,
    -0.686038, -0.769148,  0.390521,    -0.670016, -0.794322,  0.381111,    -0.968075, -0.522620,  0.263546,    -0.923408, -0.608211,  0.251115,
    -0.865340, -0.677134,  0.234954,    -0.807273, -0.730433,  0.218793,    -0.762605, -0.769148,  0.206361,    -0.744738, -0.794322,  0.201389,
    -1.002610, -0.522620,  0.000000,    -0.956312, -0.608211,  0.000000,    -0.896127, -0.677134,  0.000000,    -0.835942, -0.730433,  0.000000,
    -0.789646, -0.769148,  0.000000,    -0.771127, -0.794322,  0.000000,    -0.968075, -0.522620, -0.263546,    -0.923408, -0.608211, -0.251115,
    -0.865340, -0.677134, -0.234954,    -0.807273, -0.730433, -0.218793,    -0.762605, -0.769148, -0.206361,    -0.744738, -0.794322, -0.201389,
    -0.870290, -0.522620, -0.498738,    -0.830236, -0.608211, -0.475213,    -0.778164, -0.677134, -0.444630,    -0.726093, -0.730433, -0.414047,
    -0.686038, -0.769148, -0.390521,    -0.670016, -0.794322, -0.381111,    -0.717979, -0.522620, -0.696852,    -0.685109, -0.608211, -0.663981,
    -0.642377, -0.677134, -0.621250,    -0.599646, -0.730433, -0.578519,    -0.566775, -0.769148, -0.545648,    -0.553627, -0.794322, -0.532500,
    -0.519865, -0.522620, -0.849163,    -0.496340, -0.608211, -0.809108,    -0.465757, -0.677134, -0.757037,    -0.435174, -0.730433, -0.704966,
    -0.411648, -0.769148, -0.664911,    -0.402238, -0.794322, -0.648889,    -0.284673, -0.522620, -0.946948,    -0.272242, -0.608211, -0.902281,
    -0.256081, -0.677134, -0.844213,    -0.239920, -0.730433, -0.786145,    -0.227489, -0.769148, -0.741478,    -0.222516, -0.794322, -0.723611,
    -0.021127, -0.522620, -0.981482,    -0.021127, -0.608211, -0.935185,    -0.021127, -0.677134, -0.875000,    -0.021127, -0.730433, -0.814815,
    -0.021127, -0.769148, -0.768519,    -0.021127, -0.794322, -0.750000,     0.242419, -0.522620, -0.946948,     0.229987, -0.608211, -0.902281,
     0.213827, -0.677134, -0.844213,     0.197666, -0.730433, -0.786145,     0.185234, -0.769148, -0.741478,     0.180262, -0.794322, -0.723611,
     0.477611, -0.522620, -0.849163,     0.454085, -0.608211, -0.809108,     0.423502, -0.677134, -0.757037,     0.392919, -0.730433, -0.704966,
     0.369394, -0.769148, -0.664911,     0.359984, -0.794322, -0.648889,     0.675725, -0.522620, -0.696852,     0.642854, -0.608211, -0.663981,
     0.600123, -0.677134, -0.621250,     0.557391, -0.730433, -0.578519,     0.524521, -0.769148, -0.545648,     0.511373, -0.794322, -0.532500,
     0.828036, -0.522620, -0.498738,     0.787981, -0.608211, -0.475213,     0.735910, -0.677134, -0.444630,     0.683839, -0.730433, -0.414047,
     0.643784, -0.769148, -0.390521,     0.627762, -0.794322, -0.381111,     0.925821, -0.522620, -0.263546,     0.881153, -0.608211, -0.251115,
     0.823086, -0.677134, -0.234954,     0.765018, -0.730433, -0.218793,     0.720351, -0.769148, -0.206361,     0.702484, -0.794322, -0.201389,
     0.696621, -0.812898,  0.199757,     0.722796, -0.812898,  0.000000,     0.667643, -0.830433,  0.191692,     0.692762, -0.830433,  0.000000,
     0.598465, -0.845884,  0.172439,     0.621060, -0.845884,  0.000000,     0.472000, -0.858211,  0.137243,     0.489984, -0.858211,  0.000000,
     0.271165, -0.866370,  0.081348,     0.281824, -0.866370,  0.000000,    -0.021127, -0.869322,  0.000000,     0.622505, -0.812898,  0.378023,
     0.596519, -0.830433,  0.362761,     0.534484, -0.845884,  0.326326,     0.421079, -0.858211,  0.259720,     0.240982, -0.866370,  0.153944,
     0.507059, -0.812898,  0.528186,     0.485734, -0.830433,  0.506861,     0.434826, -0.845884,  0.455953,     0.341762, -0.858211,  0.362889,
     0.193968, -0.866370,  0.215095,     0.356896, -0.812898,  0.643632,     0.341634, -0.830433,  0.617646,     0.305199, -0.845884,  0.555611,
     0.238593, -0.858211,  0.442206,     0.132817, -0.866370,  0.262109,     0.178630, -0.812898,  0.717749,     0.170565, -0.830433,  0.688771,
     0.151312, -0.845884,  0.619592,     0.116116, -0.858211,  0.493128,     0.060221, -0.866370,  0.292292,    -0.021127, -0.812898,  0.743924,
    -0.021127, -0.830433,  0.713889,    -0.021127, -0.845884,  0.642188,    -0.021127, -0.858211,  0.511111,    -0.021127, -0.866370,  0.302951,
    -0.220884, -0.812898,  0.717749,    -0.212820, -0.830433,  0.688771,    -0.193566, -0.845884,  0.619592,    -0.158370, -0.858211,  0.493128,
    -0.102475, -0.866370,  0.292292,    -0.399151, -0.812898,  0.643632,    -0.383889, -0.830433,  0.617646,    -0.347454, -0.845884,  0.555611,
    -0.280847, -0.858211,  0.442206,    -0.175071, -0.866370,  0.262109,    -0.549313, -0.812898,  0.528186,    -0.527988, -0.830433,  0.506861,
    -0.477080, -0.845884,  0.455953,    -0.384016, -0.858211,  0.362889,    -0.236223, -0.866370,  0.215095,    -0.664759, -0.812898,  0.378023,
    -0.638773, -0.830433,  0.362761,    -0.576738, -0.845884,  0.326326,    -0.463333, -0.858211,  0.259720,    -0.283236, -0.866370,  0.153944,
    -0.738876, -0.812898,  0.199757,    -0.709898, -0.830433,  0.191692,    -0.640719, -0.845884,  0.172439,    -0.514255, -0.858211,  0.137243,
    -0.313419, -0.866370,  0.081348,    -0.765051, -0.812898,  0.000000,    -0.735016, -0.830433,  0.000000,    -0.663315, -0.845884,  0.000000,
    -0.532238, -0.858211,  0.000000,    -0.324079, -0.866370,  0.000000,    -0.738876, -0.812898, -0.199757,    -0.709898, -0.830433, -0.191692,
    -0.640719, -0.845884, -0.172439,    -0.514255, -0.858211, -0.137243,    -0.313419, -0.866370, -0.081348,    -0.664759, -0.812898, -0.378023,
    -0.638773, -0.830433, -0.362761,    -0.576738, -0.845884, -0.326326,    -0.463333, -0.858211, -0.259720,    -0.283236, -0.866370, -0.153944,
    -0.549313, -0.812898, -0.528186,    -0.527988, -0.830433, -0.506861,    -0.477080, -0.845884, -0.455953,    -0.384016, -0.858211, -0.362889,
    -0.236223, -0.866370, -0.215095,    -0.399151, -0.812898, -0.643632,    -0.383889, -0.830433, -0.617646,    -0.347454, -0.845884, -0.555611,
    -0.280847, -0.858211, -0.442206,    -0.175071, -0.866370, -0.262109,    -0.220884, -0.812898, -0.717749,    -0.212820, -0.830433, -0.688771,
    -0.193566, -0.845884, -0.619592,    -0.158370, -0.858211, -0.493128,    -0.102475, -0.866370, -0.292292,    -0.021127, -0.812898, -0.743924,
    -0.021127, -0.830433, -0.713889,    -0.021127, -0.845884, -0.642188,    -0.021127, -0.858211, -0.511111,    -0.021127, -0.866370, -0.302951,
     0.178630, -0.812898, -0.717749,     0.170565, -0.830433, -0.688771,     0.151312, -0.845884, -0.619592,     0.116116, -0.858211, -0.493128,
     0.060221, -0.866370, -0.292292,     0.356896, -0.812898, -0.643632,     0.341634, -0.830433, -0.617646,     0.305199, -0.845884, -0.555611,
     0.238593, -0.858211, -0.442206,     0.132817, -0.866370, -0.262109,     0.507059, -0.812898, -0.528186,     0.485734, -0.830433, -0.506861,
     0.434826, -0.845884, -0.455953,     0.341762, -0.858211, -0.362889,     0.193968, -0.866370, -0.215095,     0.622505, -0.812898, -0.378023,
     0.596519, -0.830433, -0.362761,     0.534484, -0.845884, -0.326326,     0.421079, -0.858211, -0.259720,     0.240982, -0.866370, -0.153944,
     0.696621, -0.812898, -0.199757,     0.667643, -0.830433, -0.191692,     0.598465, -0.845884, -0.172439,     0.472000, -0.858211, -0.137243,
     0.271165, -0.866370, -0.081348,    -0.821127,  0.143178,  0.000000,    -0.817424,  0.151512,  0.062500,    -0.984648,  0.150952,  0.062500,
    -0.983396,  0.142657,  0.000000,    -1.124350,  0.147036,  0.062500,    -1.119270,  0.139012,  0.000000,    -1.235250,  0.136407,  0.062500,
    -1.227380,  0.129116,  0.000000,    -1.316050,  0.115709,  0.062500,    -1.306310,  0.109845,  0.000000,    -1.365480,  0.081585,  0.062500,
    -1.354690,  0.078074,  0.000000,    -1.382240,  0.030678,  0.062500,    -1.371130,  0.030678,  0.000000,    -0.808164,  0.172345,  0.100000,
    -0.987777,  0.171689,  0.100000,    -1.137040,  0.167098,  0.100000,    -1.254920,  0.154637,  0.100000,    -1.340400,  0.130370,  0.100000,
    -1.392440,  0.090362,  0.100000,    -1.410020,  0.030678,  0.100000,    -0.796127,  0.199428,  0.112500,    -0.991845,  0.198647,  0.112500,
    -1.153540,  0.193178,  0.112500,    -1.280500,  0.178335,  0.112500,    -1.372050,  0.149428,  0.112500,    -1.427490,  0.101772,  0.112500,
    -1.446130,  0.030678,  0.112500,    -0.784090,  0.226511,  0.100000,    -0.995913,  0.225605,  0.100000,    -1.170030,  0.219258,  0.100000,
    -1.306080,  0.202032,  0.100000,    -1.403710,  0.168487,  0.100000,    -1.462550,  0.113182,  0.100000,    -1.482240,  0.030678,  0.100000,
    -0.774831,  0.247345,  0.062500,    -0.999042,  0.246342,  0.062500,    -1.182720,  0.239320,  0.062500,    -1.325760,  0.220261,  0.062500,
    -1.428050,  0.183147,  0.062500,    -1.489510,  0.121959,  0.062500,    -1.510020,  0.030678,  0.062500,    -0.771127,  0.255678,  0.000000,
    -1.000290,  0.254636,  0.000000,    -1.187790,  0.247345,  0.000000,    -1.333630,  0.227553,  0.000000,    -1.437790,  0.189011,  0.000000,
    -1.500290,  0.125470,  0.000000,    -1.521130,  0.030678,  0.000000,    -0.774831,  0.247345, -0.062500,    -0.999042,  0.246342, -0.062500,
    -1.182720,  0.239320, -0.062500,    -1.325760,  0.220261, -0.062500,    -1.428050,  0.183147, -0.062500,    -1.489510,  0.121959, -0.062500,
    -1.510020,  0.030678, -0.062500,    -0.784090,  0.226511, -0.100000,    -0.995913,  0.225605, -0.100000,    -1.170030,  0.219258, -0.100000,
    -1.306080,  0.202032, -0.100000,    -1.403710,  0.168487, -0.100000,    -1.462550,  0.113182, -0.100000,    -1.482240,  0.030678, -0.100000,
    -0.796127,  0.199428, -0.112500,    -0.991845,  0.198647, -0.112500,    -1.153540,  0.193178, -0.112500,    -1.280500,  0.178335, -0.112500,
    -1.372050,  0.149428, -0.112500,    -1.427490,  0.101772, -0.112500,    -1.446130,  0.030678, -0.112500,    -0.808164,  0.172345, -0.100000,
    -0.987777,  0.171689, -0.100000,    -1.137040,  0.167098, -0.100000,    -1.254920,  0.154637, -0.100000,    -1.340400,  0.130370, -0.100000,
    -1.392440,  0.090362, -0.100000,    -1.410020,  0.030678, -0.100000,    -0.817424,  0.151512, -0.062500,    -0.984648,  0.150952, -0.062500,
    -1.124350,  0.147036, -0.062500,    -1.235250,  0.136407, -0.062500,    -1.316050,  0.115709, -0.062500,    -1.365480,  0.081585, -0.062500,
    -1.382240,  0.030678, -0.062500,    -1.373220, -0.037332,  0.062500,    -1.362560, -0.033905,  0.000000,    -1.345270, -0.116647,  0.062500,
    -1.335940, -0.110988,  0.000000,    -1.297050, -0.201440,  0.062500,    -1.289880, -0.194322,  0.000000,    -1.227230, -0.285886,  0.062500,
    -1.222980, -0.277655,  0.000000,    -1.134470, -0.364159,  0.062500,    -1.133860, -0.354739,  0.000000,    -1.017420, -0.430433,  0.062500,
    -1.021130, -0.419322,  0.000000,    -1.399860, -0.045900,  0.100000,    -1.368590, -0.130793,  0.100000,    -1.314990, -0.219235,  0.100000,
    -1.237860, -0.306462,  0.100000,    -1.135990, -0.387709,  0.100000,    -1.008160, -0.458211,  0.100000,    -1.434490, -0.057039,  0.112500,
    -1.398910, -0.149183,  0.112500,    -1.338320, -0.242369,  0.112500,    -1.251680, -0.333211,  0.112500,    -1.137970, -0.418324,  0.112500,
    -0.996127, -0.494322,  0.112500,    -1.469130, -0.068177,  0.100000,    -1.429220, -0.167573,  0.100000,    -1.361640, -0.265502,  0.100000,
    -1.265500, -0.359960,  0.100000,    -1.139950, -0.448939,  0.100000,    -0.984090, -0.530433,  0.100000,    -1.495770, -0.076745,  0.062500,
    -1.452540, -0.181719,  0.062500,    -1.379580, -0.283298,  0.062500,    -1.276130, -0.380536,  0.062500,    -1.141470, -0.472489,  0.062500,
    -0.974831, -0.558211,  0.062500,    -1.506430, -0.080173,  0.000000,    -1.461870, -0.187377,  0.000000,    -1.386750, -0.290416,  0.000000,
    -1.280390, -0.388766,  0.000000,    -1.142080, -0.481909,  0.000000,    -0.971127, -0.569322,  0.000000,    -1.495770, -0.076745, -0.062500,
    -1.452540, -0.181719, -0.062500,    -1.379580, -0.283298, -0.062500,    -1.276130, -0.380536, -0.062500,    -1.141470, -0.472489, -0.062500,
    -0.974831, -0.558211, -0.062500,    -1.469130, -0.068177, -0.100000,    -1.429220, -0.167573, -0.100000,    -1.361640, -0.265502, -0.100000,
    -1.265500, -0.359960, -0.100000,    -1.139950, -0.448939, -0.100000,    -0.984090, -0.530433, -0.100000,    -1.434490, -0.057039, -0.112500,
    -1.398910, -0.149183, -0.112500,    -1.338320, -0.242369, -0.112500,    -1.251680, -0.333211, -0.112500,    -1.137970, -0.418324, -0.112500,
    -0.996127, -0.494322, -0.112500,    -1.399860, -0.045900, -0.100000,    -1.368590, -0.130793, -0.100000,    -1.314990, -0.219235, -0.100000,
    -1.237860, -0.306462, -0.100000,    -1.135990, -0.387709, -0.100000,    -1.008160, -0.458211, -0.100000,    -1.373220, -0.037332, -0.062500,
    -1.345270, -0.116647, -0.062500,    -1.297050, -0.201440, -0.062500,    -1.227230, -0.285886, -0.062500,    -1.134470, -0.364159, -0.062500,
    -1.017420, -0.430433, -0.062500,     0.828873, -0.156822,  0.000000,     0.828873, -0.187377,  0.137500,     1.015060, -0.156719,  0.131173,
     1.008270, -0.131127,  0.000000,     1.123930, -0.083314,  0.115355,     1.114060, -0.063766,  0.000000,     1.183730,  0.017484,  0.094792,
     1.172620,  0.030678,  0.000000,     1.222700,  0.130318,  0.074228,     1.210350,  0.137623,  0.000000,     1.269070,  0.239835,  0.058411,
     1.253640,  0.242484,  0.000000,     1.351090,  0.330678,  0.052083,     1.328870,  0.330678,  0.000000,     0.828873, -0.263766,  0.220000,
     1.032040, -0.220698,  0.209877,     1.148630, -0.132182,  0.184568,     1.211510, -0.015502,  0.151667,     1.253560,  0.112057,  0.118765,
     1.307650,  0.233212,  0.093457,     1.406650,  0.330678,  0.083333,     0.828873, -0.363072,  0.247500,     1.054100, -0.303870,  0.236111,
     1.180720, -0.195711,  0.207639,     1.247620, -0.058384,  0.170625,     1.293690,  0.088317,  0.133611,     1.357810,  0.224602,  0.105139,
     1.478870,  0.330678,  0.093750,     0.828873, -0.462377,  0.220000,     1.076170, -0.387043,  0.209877,     1.212820, -0.259240,  0.184568,
     1.283730, -0.101266,  0.151667,     1.333810,  0.064577,  0.118765,     1.407960,  0.215992,  0.093457,     1.551100,  0.330678,  0.083333,
     0.828873, -0.538766,  0.137500,     1.093150, -0.451022,  0.131173,     1.237510, -0.308108,  0.115355,     1.311510, -0.134252,  0.094792,
     1.364680,  0.046316,  0.074228,     1.446540,  0.209369,  0.058410,     1.606650,  0.330678,  0.052083,     0.828873, -0.569322,  0.000000,
     1.099940, -0.476614,  0.000000,     1.247390, -0.327655,  0.000000,     1.322620, -0.147447,  0.000000,     1.377020,  0.039012,  0.000000,
     1.461970,  0.206720,  0.000000,     1.628870,  0.330678,  0.000000,     0.828873, -0.538766, -0.137500,     1.093150, -0.451022, -0.131173,
     1.237510, -0.308108, -0.115355,     1.311510, -0.134252, -0.094792,     1.364680,  0.046316, -0.074228,     1.446540,  0.209369, -0.058410,
     1.606650,  0.330678, -0.052083,     0.828873, -0.462377, -0.220000,     1.076170, -0.387043, -0.209877,     1.212820, -0.259240, -0.184568,
     1.283730, -0.101266, -0.151667,     1.333810,  0.064577, -0.118765,     1.407960,  0.215992, -0.093457,     1.551100,  0.330678, -0.083333,
     0.828873, -0.363072, -0.247500,     1.054100, -0.303870, -0.236111,     1.180720, -0.195711, -0.207639,     1.247620, -0.058384, -0.170625,
     1.293690,  0.088317, -0.133611,     1.357810,  0.224602, -0.105139,     1.478870,  0.330678, -0.093750,     0.828873, -0.263766, -0.220000,
     1.032040, -0.220698, -0.209877,     1.148630, -0.132182, -0.184568,     1.211510, -0.015502, -0.151667,     1.253560,  0.112057, -0.118765,
     1.307650,  0.233212, -0.093457,     1.406650,  0.330678, -0.083333,     0.828873, -0.187377, -0.137500,     1.015060, -0.156719, -0.131173,
     1.123930, -0.083314, -0.115355,     1.183730,  0.017484, -0.094792,     1.222700,  0.130318, -0.074228,     1.269070,  0.239835, -0.058410,
     1.351090,  0.330678, -0.052083,     1.377080,  0.346641,  0.050540,     1.353410,  0.346303,  0.000000,     1.398760,  0.356295,  0.046682,
     1.375170,  0.355678,  0.000000,     1.413710,  0.359584,  0.041667,     1.391370,  0.358803,  0.000000,     1.419480,  0.356450,  0.036651,
     1.399240,  0.355678,  0.000000,     1.413620,  0.346834,  0.032793,     1.396000,  0.346303,  0.000000,     1.393690,  0.330678,  0.031250,
     1.378870,  0.330678,  0.000000,     1.436240,  0.347485,  0.080864,     1.457750,  0.357839,  0.074691,     1.469560,  0.361538,  0.066667,
     1.470060,  0.358379,  0.058642,     1.457650,  0.348160,  0.052469,     1.430720,  0.330678,  0.050000,     1.513160,  0.348582,  0.090972,
     1.534430,  0.359845,  0.084028,     1.542150,  0.364077,  0.075000,     1.535820,  0.360886,  0.065972,     1.514900,  0.349884,  0.059028,
     1.478870,  0.330678,  0.056250,     1.590080,  0.349679,  0.080864,     1.611110,  0.361851,  0.074691,     1.614750,  0.366616,  0.066667,
     1.601580,  0.363394,  0.058642,     1.572140,  0.351608,  0.052469,     1.527020,  0.330678,  0.050000,     1.649250,  0.350523,  0.050540,
     1.670090,  0.363394,  0.046682,     1.670600,  0.368569,  0.041667,     1.652160,  0.365323,  0.036651,     1.616180,  0.352934,  0.032793,
     1.564060,  0.330678,  0.031250,     1.672910,  0.350860,  0.000000,     1.693690,  0.364011,  0.000000,     1.692930,  0.369350,  0.000000,
     1.672390,  0.366095,  0.000000,     1.633790,  0.353465,  0.000000,     1.578870,  0.330678,  0.000000,     1.649250,  0.350523, -0.050540,
     1.670090,  0.363394, -0.046682,     1.670600,  0.368569, -0.041667,     1.652160,  0.365323, -0.036651,     1.616180,  0.352934, -0.032793,
     1.564060,  0.330678, -0.031250,     1.590080,  0.349679, -0.080864,     1.611110,  0.361851, -0.074691,     1.614750,  0.366616, -0.066667,
     1.601580,  0.363394, -0.058642,     1.572140,  0.351608, -0.052469,     1.527020,  0.330678, -0.050000,     1.513160,  0.348582, -0.090972,
     1.534430,  0.359845, -0.084028,     1.542150,  0.364077, -0.075000,     1.535820,  0.360886, -0.065972,     1.514900,  0.349884, -0.059028,
     1.478870,  0.330678, -0.056250,     1.436240,  0.347485, -0.080864,     1.457750,  0.357839, -0.074691,     1.469560,  0.361538, -0.066667,
     1.470060,  0.358379, -0.058642,     1.457650,  0.348160, -0.052469,     1.430720,  0.330678, -0.050000,     1.377080,  0.346641, -0.050540,
     1.398760,  0.356295, -0.046682,     1.413710,  0.359584, -0.041667,     1.419480,  0.356450, -0.036651,     1.413620,  0.346834, -0.032793,
     1.393690,  0.330678, -0.031250,     0.113346,  0.694220,  0.037539,     0.118225,  0.694220,  0.000000,    -0.021127,  0.705678,  0.000000,
     0.154000,  0.664011,  0.048885,     0.160354,  0.664011,  0.000000,     0.135681,  0.621303,  0.043764,     0.141373,  0.621303,  0.000000,
     0.093237,  0.572345,  0.031902,     0.097391,  0.572345,  0.000000,     0.061512,  0.523386,  0.023022,     0.064521,  0.523386,  0.000000,
     0.075354,  0.480678,  0.026852,     0.078873,  0.480678,  0.000000,     0.099515,  0.694220,  0.070966,     0.135987,  0.664011,  0.092417,
     0.119549,  0.621303,  0.082741,     0.081463,  0.572345,  0.060324,     0.052990,  0.523386,  0.043553,     0.065391,  0.480678,  0.050815,
     0.077943,  0.694220,  0.099070,     0.107891,  0.664011,  0.129019,     0.094388,  0.621303,  0.115516,     0.063104,  0.572345,  0.084231,
     0.039709,  0.523386,  0.060836,     0.049873,  0.480678,  0.071000,     0.049838,  0.694220,  0.120642,     0.071290,  0.664011,  0.157114,
     0.061614,  0.621303,  0.140676,     0.039197,  0.572345,  0.102590,     0.022426,  0.523386,  0.074117,     0.029688,  0.480678,  0.086519,
     0.016412,  0.694220,  0.134473,     0.027758,  0.664011,  0.175127,     0.022637,  0.621303,  0.156808,     0.010774,  0.572345,  0.114364,
     0.001895,  0.523386,  0.082639,     0.005725,  0.480678,  0.096482,    -0.021127,  0.694220,  0.139352,    -0.021127,  0.664011,  0.181482,
    -0.021127,  0.621303,  0.162500,    -0.021127,  0.572345,  0.118519,    -0.021127,  0.523386,  0.085648,    -0.021127,  0.480678,  0.100000,
    -0.058666,  0.694220,  0.134473,    -0.070013,  0.664011,  0.175127,    -0.064892,  0.621303,  0.156808,    -0.053029,  0.572345,  0.114364,
    -0.044149,  0.523386,  0.082639,    -0.047979,  0.480678,  0.096481,    -0.092093,  0.694220,  0.120642,    -0.113544,  0.664011,  0.157114,
    -0.103868,  0.621303,  0.140676,    -0.081451,  0.572345,  0.102590,    -0.064680,  0.523386,  0.074117,    -0.071942,  0.480678,  0.086519,
    -0.120197,  0.694220,  0.099070,    -0.150146,  0.664011,  0.129019,    -0.136643,  0.621303,  0.115516,    -0.105359,  0.572345,  0.084231,
    -0.081963,  0.523386,  0.060836,    -0.092127,  0.480678,  0.071000,    -0.141770,  0.694220,  0.070966,    -0.178241,  0.664011,  0.092417,
    -0.161803,  0.621303,  0.082741,    -0.123717,  0.572345,  0.060324,    -0.095244,  0.523386,  0.043553,    -0.107646,  0.480678,  0.050815,
    -0.155600,  0.694220,  0.037539,    -0.196254,  0.664011,  0.048885,    -0.177936,  0.621303,  0.043764,    -0.135491,  0.572345,  0.031902,
    -0.103767,  0.523386,  0.023022,    -0.117609,  0.480678,  0.026852,    -0.160479,  0.694220,  0.000000,    -0.202609,  0.664011,  0.000000,
    -0.183627,  0.621303,  0.000000,    -0.139646,  0.572345,  0.000000,    -0.106775,  0.523386,  0.000000,    -0.121127,  0.480678,  0.000000,
    -0.155600,  0.694220, -0.037539,    -0.196254,  0.664011, -0.048885,    -0.177936,  0.621303, -0.043764,    -0.135491,  0.572345, -0.031902,
    -0.103767,  0.523386, -0.023022,    -0.117609,  0.480678, -0.026852,    -0.141770,  0.694220, -0.070966,    -0.178241,  0.664011, -0.092417,
    -0.161803,  0.621303, -0.082741,    -0.123717,  0.572345, -0.060324,    -0.095244,  0.523386, -0.043553,    -0.107646,  0.480678, -0.050815,
    -0.120197,  0.694220, -0.099070,    -0.150146,  0.664011, -0.129019,    -0.136643,  0.621303, -0.115516,    -0.105359,  0.572345, -0.084231,
    -0.081963,  0.523386, -0.060836,    -0.092127,  0.480678, -0.071000,    -0.092093,  0.694220, -0.120642,    -0.113544,  0.664011, -0.157114,
    -0.103868,  0.621303, -0.140676,    -0.081451,  0.572345, -0.102590,    -0.064680,  0.523386, -0.074117,    -0.071942,  0.480678, -0.086519,
    -0.058666,  0.694220, -0.134473,    -0.070013,  0.664011, -0.175127,    -0.064892,  0.621303, -0.156808,    -0.053029,  0.572345, -0.114364,
    -0.044149,  0.523386, -0.082639,    -0.047979,  0.480678, -0.096482,    -0.021127,  0.694220, -0.139352,    -0.021127,  0.664011, -0.181482,
    -0.021127,  0.621303, -0.162500,    -0.021127,  0.572345, -0.118519,    -0.021127,  0.523386, -0.085648,    -0.021127,  0.480678, -0.100000,
     0.016412,  0.694220, -0.134473,     0.027758,  0.664011, -0.175127,     0.022637,  0.621303, -0.156808,     0.010774,  0.572345, -0.114364,
     0.001895,  0.523386, -0.082639,     0.005725,  0.480678, -0.096481,     0.049838,  0.694220, -0.120642,     0.071290,  0.664011, -0.157114,
     0.061614,  0.621303, -0.140676,     0.039197,  0.572345, -0.102590,     0.022426,  0.523386, -0.074117,     0.029688,  0.480678, -0.086519,
     0.077943,  0.694220, -0.099070,     0.107891,  0.664011, -0.129019,     0.094388,  0.621303, -0.115516,     0.063104,  0.572345, -0.084231,
     0.039709,  0.523386, -0.060836,     0.049873,  0.480678, -0.071000,     0.099515,  0.694220, -0.070966,     0.135987,  0.664011, -0.092417,
     0.119549,  0.621303, -0.082741,     0.081463,  0.572345, -0.060324,     0.052990,  0.523386, -0.043553,     0.065391,  0.480678, -0.050815,
     0.113346,  0.694220, -0.037539,     0.154000,  0.664011, -0.048885,     0.135681,  0.621303, -0.043764,     0.093237,  0.572345, -0.031902,
     0.061512,  0.523386, -0.023022,     0.075354,  0.480678, -0.026852,     0.148162,  0.448734,  0.047115,     0.154336,  0.448734,  0.000000,
     0.255810,  0.425123,  0.077075,     0.265910,  0.425123,  0.000000,     0.376859,  0.405678,  0.110764,     0.391373,  0.405678,  0.000000,
     0.489867,  0.386234,  0.142215,     0.508502,  0.386234,  0.000000,     0.573395,  0.362623,  0.165462,     0.595077,  0.362623,  0.000000,
     0.606002,  0.330678,  0.174537,     0.628873,  0.330678,  0.000000,     0.130681,  0.448734,  0.089161,     0.227213,  0.425123,  0.145857,
     0.335762,  0.405678,  0.209611,     0.437101,  0.386234,  0.269130,     0.512003,  0.362623,  0.313123,     0.541243,  0.330678,  0.330296,
     0.103451,  0.448734,  0.124579,     0.182669,  0.425123,  0.203796,     0.271748,  0.405678,  0.292875,     0.354910,  0.386234,  0.376037,
     0.416377,  0.362623,  0.437505,     0.440373,  0.330678,  0.461500,     0.068034,  0.448734,  0.151808,     0.124730,  0.425123,  0.248340,
     0.188484,  0.405678,  0.356889,     0.248003,  0.386234,  0.458228,     0.291995,  0.362623,  0.533130,     0.309169,  0.330678,  0.562370,
     0.025988,  0.448734,  0.169289,     0.055948,  0.425123,  0.276938,     0.089637,  0.405678,  0.397986,     0.121088,  0.386234,  0.510995,
     0.144335,  0.362623,  0.594523,     0.153410,  0.330678,  0.627130,    -0.021127,  0.448734,  0.175463,    -0.021127,  0.425123,  0.287037,
    -0.021127,  0.405678,  0.412500,    -0.021127,  0.386234,  0.529630,    -0.021127,  0.362623,  0.616204,    -0.021127,  0.330678,  0.650000,
    -0.068242,  0.448734,  0.169289,    -0.098202,  0.425123,  0.276938,    -0.131891,  0.405678,  0.397986,    -0.163343,  0.386234,  0.510995,
    -0.186589,  0.362623,  0.594523,    -0.195664,  0.330678,  0.627130,    -0.110288,  0.448734,  0.151808,    -0.166985,  0.425123,  0.248340,
    -0.230738,  0.405678,  0.356889,    -0.290258,  0.386234,  0.458228,    -0.334250,  0.362623,  0.533130,    -0.351424,  0.330678,  0.562370,
    -0.145706,  0.448734,  0.124579,    -0.224924,  0.425123,  0.203796,    -0.314002,  0.405678,  0.292875,    -0.397164,  0.386234,  0.376037,
    -0.458632,  0.362623,  0.437505,    -0.482627,  0.330678,  0.461500,    -0.172935,  0.448734,  0.089161,    -0.269467,  0.425123,  0.145857,
    -0.378016,  0.405678,  0.209611,    -0.479355,  0.386234,  0.269130,    -0.554258,  0.362623,  0.313123,    -0.583498,  0.330678,  0.330296,
    -0.190416,  0.448734,  0.047115,    -0.298065,  0.425123,  0.077075,    -0.419113,  0.405678,  0.110764,    -0.532122,  0.386234,  0.142215,
    -0.615650,  0.362623,  0.165462,    -0.648257,  0.330678,  0.174537,    -0.196590,  0.448734,  0.000000,    -0.308164,  0.425123,  0.000000,
    -0.433627,  0.405678,  0.000000,    -0.550757,  0.386234,  0.000000,    -0.637331,  0.362623,  0.000000,    -0.671127,  0.330678,  0.000000,
    -0.190416,  0.448734, -0.047115,    -0.298065,  0.425123, -0.077075,    -0.419113,  0.405678, -0.110764,    -0.532122,  0.386234, -0.142215,
    -0.615650,  0.362623, -0.165462,    -0.648257,  0.330678, -0.174537,    -0.172935,  0.448734, -0.089161,    -0.269467,  0.425123, -0.145857,
    -0.378016,  0.405678, -0.209611,    -0.479355,  0.386234, -0.269130,    -0.554258,  0.362623, -0.313123,    -0.583498,  0.330678, -0.330296,
    -0.145706,  0.448734, -0.124579,    -0.224924,  0.425123, -0.203796,    -0.314002,  0.405678, -0.292875,    -0.397164,  0.386234, -0.376037,
    -0.458632,  0.362623, -0.437505,    -0.482627,  0.330678, -0.461500,    -0.110288,  0.448734, -0.151808,    -0.166985,  0.425123, -0.248340,
    -0.230738,  0.405678, -0.356889,    -0.290258,  0.386234, -0.458228,    -0.334250,  0.362623, -0.533130,    -0.351424,  0.330678, -0.562370,
    -0.068242,  0.448734, -0.169289,    -0.098202,  0.425123, -0.276938,    -0.131891,  0.405678, -0.397986,    -0.163343,  0.386234, -0.510995,
    -0.186589,  0.362623, -0.594523,    -0.195664,  0.330678, -0.627130,    -0.021127,  0.448734, -0.175463,    -0.021127,  0.425123, -0.287037,
    -0.021127,  0.405678, -0.412500,    -0.021127,  0.386234, -0.529630,    -0.021127,  0.362623, -0.616204,    -0.021127,  0.330678, -0.650000,
     0.025988,  0.448734, -0.169289,     0.055948,  0.425123, -0.276938,     0.089637,  0.405678, -0.397986,     0.121088,  0.386234, -0.510995,
     0.144335,  0.362623, -0.594523,     0.153410,  0.330678, -0.627130,     0.068034,  0.448734, -0.151808,     0.124730,  0.425123, -0.248340,
     0.188484,  0.405678, -0.356889,     0.248003,  0.386234, -0.458228,     0.291996,  0.362623, -0.533130,     0.309169,  0.330678, -0.562370,
     0.103451,  0.448734, -0.124579,     0.182669,  0.425123, -0.203796,     0.271748,  0.405678, -0.292875,     0.354910,  0.386234, -0.376037,
     0.416377,  0.362623, -0.437505,     0.440373,  0.330678, -0.461500,     0.130681,  0.448734, -0.089161,     0.227213,  0.425123, -0.145857,
     0.335762,  0.405678, -0.209611,     0.437101,  0.386234, -0.269130,     0.512003,  0.362623, -0.313123,     0.541243,  0.330678, -0.330296,
     0.148162,  0.448734, -0.047115,     0.255810,  0.425123, -0.077075,     0.376859,  0.405678, -0.110764,     0.489867,  0.386234, -0.142215,
     0.573395,  0.362623, -0.165462,     0.606002,  0.330678, -0.174537,     0.000000,  0.000000,  0.000000,     0.000000,  0.000000,  0.000000,

};

/*
 * ========== Teapot triangles: ==========
 */

static const s32 __spTeapotIndices[] = {

       0,    1,    2,       2,    3,    0,       3,    2,    4,       4,    5,    3,       5,    4,    6,       6,    7,    5,
       7,    6,    8,       8,    9,    7,       9,    8,   10,      10,   11,    9,      11,   10,   12,      12,   13,   11,
       1,   14,   15,      15,    2,    1,       2,   15,   16,      16,    4,    2,       4,   16,   17,      17,    6,    4,
       6,   17,   18,      18,    8,    6,       8,   18,   19,      19,   10,    8,      10,   19,   20,      20,   12,   10,
      14,   21,   22,      22,   15,   14,      15,   22,   23,      23,   16,   15,      16,   23,   24,      24,   17,   16,
      17,   24,   25,      25,   18,   17,      18,   25,   26,      26,   19,   18,      19,   26,   27,      27,   20,   19,
      21,   28,   29,      29,   22,   21,      22,   29,   30,      30,   23,   22,      23,   30,   31,      31,   24,   23,
      24,   31,   32,      32,   25,   24,      25,   32,   33,      33,   26,   25,      26,   33,   34,      34,   27,   26,
      28,   35,   36,      36,   29,   28,      29,   36,   37,      37,   30,   29,      30,   37,   38,      38,   31,   30,
      31,   38,   39,      39,   32,   31,      32,   39,   40,      40,   33,   32,      33,   40,   41,      41,   34,   33,
      35,   42,   43,      43,   36,   35,      36,   43,   44,      44,   37,   36,      37,   44,   45,      45,   38,   37,
      38,   45,   46,      46,   39,   38,      39,   46,   47,      47,   40,   39,      40,   47,   48,      48,   41,   40,
      42,   49,   50,      50,   43,   42,      43,   50,   51,      51,   44,   43,      44,   51,   52,      52,   45,   44,
      45,   52,   53,      53,   46,   45,      46,   53,   54,      54,   47,   46,      47,   54,   55,      55,   48,   47,
      49,   56,   57,      57,   50,   49,      50,   57,   58,      58,   51,   50,      51,   58,   59,      59,   52,   51,
      52,   59,   60,      60,   53,   52,      53,   60,   61,      61,   54,   53,      54,   61,   62,      62,   55,   54,
      56,   63,   64,      64,   57,   56,      57,   64,   65,      65,   58,   57,      58,   65,   66,      66,   59,   58,
      59,   66,   67,      67,   60,   59,      60,   67,   68,      68,   61,   60,      61,   68,   69,      69,   62,   61,
      63,   70,   71,      71,   64,   63,      64,   71,   72,      72,   65,   64,      65,   72,   73,      73,   66,   65,
      66,   73,   74,      74,   67,   66,      67,   74,   75,      75,   68,   67,      68,   75,   76,      76,   69,   68,
      70,   77,   78,      78,   71,   70,      71,   78,   79,      79,   72,   71,      72,   79,   80,      80,   73,   72,
      73,   80,   81,      81,   74,   73,      74,   81,   82,      82,   75,   74,      75,   82,   83,      83,   76,   75,
      77,   84,   85,      85,   78,   77,      78,   85,   86,      86,   79,   78,      79,   86,   87,      87,   80,   79,
      80,   87,   88,      88,   81,   80,      81,   88,   89,      89,   82,   81,      82,   89,   90,      90,   83,   82,
      84,   91,   92,      92,   85,   84,      85,   92,   93,      93,   86,   85,      86,   93,   94,      94,   87,   86,
      87,   94,   95,      95,   88,   87,      88,   95,   96,      96,   89,   88,      89,   96,   97,      97,   90,   89,
      91,   98,   99,      99,   92,   91,      92,   99,  100,     100,   93,   92,      93,  100,  101,     101,   94,   93,
      94,  101,  102,     102,   95,   94,      95,  102,  103,     103,   96,   95,      96,  103,  104,     104,   97,   96,
      98,  105,  106,     106,   99,   98,      99,  106,  107,     107,  100,   99,     100,  107,  108,     108,  101,  100,
     101,  108,  109,     109,  102,  101,     102,  109,  110,     110,  103,  102,     103,  110,  111,     111,  104,  103,
     105,  112,  113,     113,  106,  105,     106,  113,  114,     114,  107,  106,     107,  114,  115,     115,  108,  107,
     108,  115,  116,     116,  109,  108,     109,  116,  117,     117,  110,  109,     110,  117,  118,     118,  111,  110,
     112,  119,  120,     120,  113,  112,     113,  120,  121,     121,  114,  113,     114,  121,  122,     122,  115,  114,
     115,  122,  123,     123,  116,  115,     116,  123,  124,     124,  117,  116,     117,  124,  125,     125,  118,  117,
     119,  126,  127,     127,  120,  119,     120,  127,  128,     128,  121,  120,     121,  128,  129,     129,  122,  121,
     122,  129,  130,     130,  123,  122,     123,  130,  131,     131,  124,  123,     124,  131,  132,     132,  125,  124,
     126,  133,  134,     134,  127,  126,     127,  134,  135,     135,  128,  127,     128,  135,  136,     136,  129,  128,
     129,  136,  137,     137,  130,  129,     130,  137,  138,     138,  131,  130,     131,  138,  139,     139,  132,  131,
     133,  140,  141,     141,  134,  133,     134,  141,  142,     142,  135,  134,     135,  142,  143,     143,  136,  135,
     136,  143,  144,     144,  137,  136,     137,  144,  145,     145,  138,  137,     138,  145,  146,     146,  139,  138,
     140,  147,  148,     148,  141,  140,     141,  148,  149,     149,  142,  141,     142,  149,  150,     150,  143,  142,
     143,  150,  151,     151,  144,  143,     144,  151,  152,     152,  145,  144,     145,  152,  153,     153,  146,  145,
     147,  154,  155,     155,  148,  147,     148,  155,  156,     156,  149,  148,     149,  156,  157,     157,  150,  149,
     150,  157,  158,     158,  151,  150,     151,  158,  159,     159,  152,  151,     152,  159,  160,     160,  153,  152,
     154,  161,  162,     162,  155,  154,     155,  162,  163,     163,  156,  155,     156,  163,  164,     164,  157,  156,
     157,  164,  165,     165,  158,  157,     158,  165,  166,     166,  159,  158,     159,  166,  167,     167,  160,  159,
     161,    0,    3,       3,  162,  161,     162,    3,    5,       5,  163,  162,     163,    5,    7,       7,  164,  163,
     164,    7,    9,       9,  165,  164,     165,    9,   11,      11,  166,  165,     166,   11,   13,      13,  167,  166,
      13,   12,  168,     168,  169,   13,     169,  168,  170,     170,  171,  169,     171,  170,  172,     172,  173,  171,
     173,  172,  174,     174,  175,  173,     175,  174,  176,     176,  177,  175,     177,  176,  178,     178,  179,  177,
      12,   20,  180,     180,  168,   12,     168,  180,  181,     181,  170,  168,     170,  181,  182,     182,  172,  170,
     172,  182,  183,     183,  174,  172,     174,  183,  184,     184,  176,  174,     176,  184,  185,     185,  178,  176,
      20,   27,  186,     186,  180,   20,     180,  186,  187,     187,  181,  180,     181,  187,  188,     188,  182,  181,
     182,  188,  189,     189,  183,  182,     183,  189,  190,     190,  184,  183,     184,  190,  191,     191,  185,  184,
      27,   34,  192,     192,  186,   27,     186,  192,  193,     193,  187,  186,     187,  193,  194,     194,  188,  187,
     188,  194,  195,     195,  189,  188,     189,  195,  196,     196,  190,  189,     190,  196,  197,     197,  191,  190,
      34,   41,  198,     198,  192,   34,     192,  198,  199,     199,  193,  192,     193,  199,  200,     200,  194,  193,
     194,  200,  201,     201,  195,  194,     195,  201,  202,     202,  196,  195,     196,  202,  203,     203,  197,  196,
      41,   48,  204,     204,  198,   41,     198,  204,  205,     205,  199,  198,     199,  205,  206,     206,  200,  199,
     200,  206,  207,     207,  201,  200,     201,  207,  208,     208,  202,  201,     202,  208,  209,     209,  203,  202,
      48,   55,  210,     210,  204,   48,     204,  210,  211,     211,  205,  204,     205,  211,  212,     212,  206,  205,
     206,  212,  213,     213,  207,  206,     207,  213,  214,     214,  208,  207,     208,  214,  215,     215,  209,  208,
      55,   62,  216,     216,  210,   55,     210,  216,  217,     217,  211,  210,     211,  217,  218,     218,  212,  211,
     212,  218,  219,     219,  213,  212,     213,  219,  220,     220,  214,  213,     214,  220,  221,     221,  215,  214,
      62,   69,  222,     222,  216,   62,     216,  222,  223,     223,  217,  216,     217,  223,  224,     224,  218,  217,
     218,  224,  225,     225,  219,  218,     219,  225,  226,     226,  220,  219,     220,  226,  227,     227,  221,  220,
      69,   76,  228,     228,  222,   69,     222,  228,  229,     229,  223,  222,     223,  229,  230,     230,  224,  223,
     224,  230,  231,     231,  225,  224,     225,  231,  232,     232,  226,  225,     226,  232,  233,     233,  227,  226,
      76,   83,  234,     234,  228,   76,     228,  234,  235,     235,  229,  228,     229,  235,  236,     236,  230,  229,
     230,  236,  237,     237,  231,  230,     231,  237,  238,     238,  232,  231,     232,  238,  239,     239,  233,  232,
      83,   90,  240,     240,  234,   83,     234,  240,  241,     241,  235,  234,     235,  241,  242,     242,  236,  235,
     236,  242,  243,     243,  237,  236,     237,  243,  244,     244,  238,  237,     238,  244,  245,     245,  239,  238,
      90,   97,  246,     246,  240,   90,     240,  246,  247,     247,  241,  240,     241,  247,  248,     248,  242,  241,
     242,  248,  249,     249,  243,  242,     243,  249,  250,     250,  244,  243,     244,  250,  251,     251,  245,  244,
      97,  104,  252,     252,  246,   97,     246,  252,  253,     253,  247,  246,     247,  253,  254,     254,  248,  247,
     248,  254,  255,     255,  249,  248,     249,  255,  256,     256,  250,  249,     250,  256,  257,     257,  251,  250,
     104,  111,  258,     258,  252,  104,     252,  258,  259,     259,  253,  252,     253,  259,  260,     260,  254,  253,
     254,  260,  261,     261,  255,  254,     255,  261,  262,     262,  256,  255,     256,  262,  263,     263,  257,  256,
     111,  118,  264,     264,  258,  111,     258,  264,  265,     265,  259,  258,     259,  265,  266,     266,  260,  259,
     260,  266,  267,     267,  261,  260,     261,  267,  268,     268,  262,  261,     262,  268,  269,     269,  263,  262,
     118,  125,  270,     270,  264,  118,     264,  270,  271,     271,  265,  264,     265,  271,  272,     272,  266,  265,
     266,  272,  273,     273,  267,  266,     267,  273,  274,     274,  268,  267,     268,  274,  275,     275,  269,  268,
     125,  132,  276,     276,  270,  125,     270,  276,  277,     277,  271,  270,     271,  277,  278,     278,  272,  271,
     272,  278,  279,     279,  273,  272,     273,  279,  280,     280,  274,  273,     274,  280,  281,     281,  275,  274,
     132,  139,  282,     282,  276,  132,     276,  282,  283,     283,  277,  276,     277,  283,  284,     284,  278,  277,
     278,  284,  285,     285,  279,  278,     279,  285,  286,     286,  280,  279,     280,  286,  287,     287,  281,  280,
     139,  146,  288,     288,  282,  139,     282,  288,  289,     289,  283,  282,     283,  289,  290,     290,  284,  283,
     284,  290,  291,     291,  285,  284,     285,  291,  292,     292,  286,  285,     286,  292,  293,     293,  287,  286,
     146,  153,  294,     294,  288,  146,     288,  294,  295,     295,  289,  288,     289,  295,  296,     296,  290,  289,
     290,  296,  297,     297,  291,  290,     291,  297,  298,     298,  292,  291,     292,  298,  299,     299,  293,  292,
     153,  160,  300,     300,  294,  153,     294,  300,  301,     301,  295,  294,     295,  301,  302,     302,  296,  295,
     296,  302,  303,     303,  297,  296,     297,  303,  304,     304,  298,  297,     298,  304,  305,     305,  299,  298,
     160,  167,  306,     306,  300,  160,     300,  306,  307,     307,  301,  300,     301,  307,  308,     308,  302,  301,
     302,  308,  309,     309,  303,  302,     303,  309,  310,     310,  304,  303,     304,  310,  311,     311,  305,  304,
     167,   13,  169,     169,  306,  167,     306,  169,  171,     171,  307,  306,     307,  171,  173,     173,  308,  307,
     308,  173,  175,     175,  309,  308,     309,  175,  177,     177,  310,  309,     310,  177,  179,     179,  311,  310,
     179,  178,  312,     312,  313,  179,     313,  312,  314,     314,  315,  313,     315,  314,  316,     316,  317,  315,
     317,  316,  318,     318,  319,  317,     319,  318,  320,     320,  321,  319,     321,  320,  322,     322,  323,  321,
     178,  185,  324,     324,  312,  178,     312,  324,  325,     325,  314,  312,     314,  325,  326,     326,  316,  314,
     316,  326,  327,     327,  318,  316,     318,  327,  328,     328,  320,  318,     320,  328,  329,     329,  322,  320,
     185,  191,  330,     330,  324,  185,     324,  330,  331,     331,  325,  324,     325,  331,  332,     332,  326,  325,
     326,  332,  333,     333,  327,  326,     327,  333,  334,     334,  328,  327,     328,  334,  335,     335,  329,  328,
     191,  197,  336,     336,  330,  191,     330,  336,  337,     337,  331,  330,     331,  337,  338,     338,  332,  331,
     332,  338,  339,     339,  333,  332,     333,  339,  340,     340,  334,  333,     334,  340,  341,     341,  335,  334,
     197,  203,  342,     342,  336,  197,     336,  342,  343,     343,  337,  336,     337,  343,  344,     344,  338,  337,
     338,  344,  345,     345,  339,  338,     339,  345,  346,     346,  340,  339,     340,  346,  347,     347,  341,  340,
     203,  209,  348,     348,  342,  203,     342,  348,  349,     349,  343,  342,     343,  349,  350,     350,  344,  343,
     344,  350,  351,     351,  345,  344,     345,  351,  352,     352,  346,  345,     346,  352,  353,     353,  347,  346,
     209,  215,  354,     354,  348,  209,     348,  354,  355,     355,  349,  348,     349,  355,  356,     356,  350,  349,
     350,  356,  357,     357,  351,  350,     351,  357,  358,     358,  352,  351,     352,  358,  359,     359,  353,  352,
     215,  221,  360,     360,  354,  215,     354,  360,  361,     361,  355,  354,     355,  361,  362,     362,  356,  355,
     356,  362,  363,     363,  357,  356,     357,  363,  364,     364,  358,  357,     358,  364,  365,     365,  359,  358,
     221,  227,  366,     366,  360,  221,     360,  366,  367,     367,  361,  360,     361,  367,  368,     368,  362,  361,
     362,  368,  369,     369,  363,  362,     363,  369,  370,     370,  364,  363,     364,  370,  371,     371,  365,  364,
     227,  233,  372,     372,  366,  227,     366,  372,  373,     373,  367,  366,     367,  373,  374,     374,  368,  367,
     368,  374,  375,     375,  369,  368,     369,  375,  376,     376,  370,  369,     370,  376,  377,     377,  371,  370,
     233,  239,  378,     378,  372,  233,     372,  378,  379,     379,  373,  372,     373,  379,  380,     380,  374,  373,
     374,  380,  381,     381,  375,  374,     375,  381,  382,     382,  376,  375,     376,  382,  383,     383,  377,  376,
     239,  245,  384,     384,  378,  239,     378,  384,  385,     385,  379,  378,     379,  385,  386,     386,  380,  379,
     380,  386,  387,     387,  381,  380,     381,  387,  388,     388,  382,  381,     382,  388,  389,     389,  383,  382,
     245,  251,  390,     390,  384,  245,     384,  390,  391,     391,  385,  384,     385,  391,  392,     392,  386,  385,
     386,  392,  393,     393,  387,  386,     387,  393,  394,     394,  388,  387,     388,  394,  395,     395,  389,  388,
     251,  257,  396,     396,  390,  251,     390,  396,  397,     397,  391,  390,     391,  397,  398,     398,  392,  391,
     392,  398,  399,     399,  393,  392,     393,  399,  400,     400,  394,  393,     394,  400,  401,     401,  395,  394,
     257,  263,  402,     402,  396,  257,     396,  402,  403,     403,  397,  396,     397,  403,  404,     404,  398,  397,
     398,  404,  405,     405,  399,  398,     399,  405,  406,     406,  400,  399,     400,  406,  407,     407,  401,  400,
     263,  269,  408,     408,  402,  263,     402,  408,  409,     409,  403,  402,     403,  409,  410,     410,  404,  403,
     404,  410,  411,     411,  405,  404,     405,  411,  412,     412,  406,  405,     406,  412,  413,     413,  407,  406,
     269,  275,  414,     414,  408,  269,     408,  414,  415,     415,  409,  408,     409,  415,  416,     416,  410,  409,
     410,  416,  417,     417,  411,  410,     411,  417,  418,     418,  412,  411,     412,  418,  419,     419,  413,  412,
     275,  281,  420,     420,  414,  275,     414,  420,  421,     421,  415,  414,     415,  421,  422,     422,  416,  415,
     416,  422,  423,     423,  417,  416,     417,  423,  424,     424,  418,  417,     418,  424,  425,     425,  419,  418,
     281,  287,  426,     426,  420,  281,     420,  426,  427,     427,  421,  420,     421,  427,  428,     428,  422,  421,
     422,  428,  429,     429,  423,  422,     423,  429,  430,     430,  424,  423,     424,  430,  431,     431,  425,  424,
     287,  293,  432,     432,  426,  287,     426,  432,  433,     433,  427,  426,     427,  433,  434,     434,  428,  427,
     428,  434,  435,     435,  429,  428,     429,  435,  436,     436,  430,  429,     430,  436,  437,     437,  431,  430,
     293,  299,  438,     438,  432,  293,     432,  438,  439,     439,  433,  432,     433,  439,  440,     440,  434,  433,
     434,  440,  441,     441,  435,  434,     435,  441,  442,     442,  436,  435,     436,  442,  443,     443,  437,  436,
     299,  305,  444,     444,  438,  299,     438,  444,  445,     445,  439,  438,     439,  445,  446,     446,  440,  439,
     440,  446,  447,     447,  441,  440,     441,  447,  448,     448,  442,  441,     442,  448,  449,     449,  443,  442,
     305,  311,  450,     450,  444,  305,     444,  450,  451,     451,  445,  444,     445,  451,  452,     452,  446,  445,
     446,  452,  453,     453,  447,  446,     447,  453,  454,     454,  448,  447,     448,  454,  455,     455,  449,  448,
     311,  179,  313,     313,  450,  311,     450,  313,  315,     315,  451,  450,     451,  315,  317,     317,  452,  451,
     452,  317,  319,     319,  453,  452,     453,  319,  321,     321,  454,  453,     454,  321,  323,     323,  455,  454,
     323,  322,  456,     456,  457,  323,     457,  456,  458,     458,  459,  457,     459,  458,  460,     460,  461,  459,
     461,  460,  462,     462,  463,  461,     463,  462,  464,     464,  465,  463,     465,  464,  466,     322,  329,  467,
     467,  456,  322,     456,  467,  468,     468,  458,  456,     458,  468,  469,     469,  460,  458,     460,  469,  470,
     470,  462,  460,     462,  470,  471,     471,  464,  462,     464,  471,  466,     329,  335,  472,     472,  467,  329,
     467,  472,  473,     473,  468,  467,     468,  473,  474,     474,  469,  468,     469,  474,  475,     475,  470,  469,
     470,  475,  476,     476,  471,  470,     471,  476,  466,     335,  341,  477,     477,  472,  335,     472,  477,  478,
     478,  473,  472,     473,  478,  479,     479,  474,  473,     474,  479,  480,     480,  475,  474,     475,  480,  481,
     481,  476,  475,     476,  481,  466,     341,  347,  482,     482,  477,  341,     477,  482,  483,     483,  478,  477,
     478,  483,  484,     484,  479,  478,     479,  484,  485,     485,  480,  479,     480,  485,  486,     486,  481,  480,
     481,  486,  466,     347,  353,  487,     487,  482,  347,     482,  487,  488,     488,  483,  482,     483,  488,  489,
     489,  484,  483,     484,  489,  490,     490,  485,  484,     485,  490,  491,     491,  486,  485,     486,  491,  466,
     353,  359,  492,     492,  487,  353,     487,  492,  493,     493,  488,  487,     488,  493,  494,     494,  489,  488,
     489,  494,  495,     495,  490,  489,     490,  495,  496,     496,  491,  490,     491,  496,  466,     359,  365,  497,
     497,  492,  359,     492,  497,  498,     498,  493,  492,     493,  498,  499,     499,  494,  493,     494,  499,  500,
     500,  495,  494,     495,  500,  501,     501,  496,  495,     496,  501,  466,     365,  371,  502,     502,  497,  365,
     497,  502,  503,     503,  498,  497,     498,  503,  504,     504,  499,  498,     499,  504,  505,     505,  500,  499,
     500,  505,  506,     506,  501,  500,     501,  506,  466,     371,  377,  507,     507,  502,  371,     502,  507,  508,
     508,  503,  502,     503,  508,  509,     509,  504,  503,     504,  509,  510,     510,  505,  504,     505,  510,  511,
     511,  506,  505,     506,  511,  466,     377,  383,  512,     512,  507,  377,     507,  512,  513,     513,  508,  507,
     508,  513,  514,     514,  509,  508,     509,  514,  515,     515,  510,  509,     510,  515,  516,     516,  511,  510,
     511,  516,  466,     383,  389,  517,     517,  512,  383,     512,  517,  518,     518,  513,  512,     513,  518,  519,
     519,  514,  513,     514,  519,  520,     520,  515,  514,     515,  520,  521,     521,  516,  515,     516,  521,  466,
     389,  395,  522,     522,  517,  389,     517,  522,  523,     523,  518,  517,     518,  523,  524,     524,  519,  518,
     519,  524,  525,     525,  520,  519,     520,  525,  526,     526,  521,  520,     521,  526,  466,     395,  401,  527,
     527,  522,  395,     522,  527,  528,     528,  523,  522,     523,  528,  529,     529,  524,  523,     524,  529,  530,
     530,  525,  524,     525,  530,  531,     531,  526,  525,     526,  531,  466,     401,  407,  532,     532,  527,  401,
     527,  532,  533,     533,  528,  527,     528,  533,  534,     534,  529,  528,     529,  534,  535,     535,  530,  529,
     530,  535,  536,     536,  531,  530,     531,  536,  466,     407,  413,  537,     537,  532,  407,     532,  537,  538,
     538,  533,  532,     533,  538,  539,     539,  534,  533,     534,  539,  540,     540,  535,  534,     535,  540,  541,
     541,  536,  535,     536,  541,  466,     413,  419,  542,     542,  537,  413,     537,  542,  543,     543,  538,  537,
     538,  543,  544,     544,  539,  538,     539,  544,  545,     545,  540,  539,     540,  545,  546,     546,  541,  540,
     541,  546,  466,     419,  425,  547,     547,  542,  419,     542,  547,  548,     548,  543,  542,     543,  548,  549,
     549,  544,  543,     544,  549,  550,     550,  545,  544,     545,  550,  551,     551,  546,  545,     546,  551,  466,
     425,  431,  552,     552,  547,  425,     547,  552,  553,     553,  548,  547,     548,  553,  554,     554,  549,  548,
     549,  554,  555,     555,  550,  549,     550,  555,  556,     556,  551,  550,     551,  556,  466,     431,  437,  557,
     557,  552,  431,     552,  557,  558,     558,  553,  552,     553,  558,  559,     559,  554,  553,     554,  559,  560,
     560,  555,  554,     555,  560,  561,     561,  556,  555,     556,  561,  466,     437,  443,  562,     562,  557,  437,
     557,  562,  563,     563,  558,  557,     558,  563,  564,     564,  559,  558,     559,  564,  565,     565,  560,  559,
     560,  565,  566,     566,  561,  560,     561,  566,  466,     443,  449,  567,     567,  562,  443,     562,  567,  568,
     568,  563,  562,     563,  568,  569,     569,  564,  563,     564,  569,  570,     570,  565,  564,     565,  570,  571,
     571,  566,  565,     566,  571,  466,     449,  455,  572,     572,  567,  449,     567,  572,  573,     573,  568,  567,
     568,  573,  574,     574,  569,  568,     569,  574,  575,     575,  570,  569,     570,  575,  576,     576,  571,  570,
     571,  576,  466,     455,  323,  457,     457,  572,  455,     572,  457,  459,     459,  573,  572,     573,  459,  461,
     461,  574,  573,     574,  461,  463,     463,  575,  574,     575,  463,  465,     465,  576,  575,     576,  465,  466,
     577,  578,  579,     579,  580,  577,     580,  579,  581,     581,  582,  580,     582,  581,  583,     583,  584,  582,
     584,  583,  585,     585,  586,  584,     586,  585,  587,     587,  588,  586,     588,  587,  589,     589,  590,  588,
     578,  591,  592,     592,  579,  578,     579,  592,  593,     593,  581,  579,     581,  593,  594,     594,  583,  581,
     583,  594,  595,     595,  585,  583,     585,  595,  596,     596,  587,  585,     587,  596,  597,     597,  589,  587,
     591,  598,  599,     599,  592,  591,     592,  599,  600,     600,  593,  592,     593,  600,  601,     601,  594,  593,
     594,  601,  602,     602,  595,  594,     595,  602,  603,     603,  596,  595,     596,  603,  604,     604,  597,  596,
     598,  605,  606,     606,  599,  598,     599,  606,  607,     607,  600,  599,     600,  607,  608,     608,  601,  600,
     601,  608,  609,     609,  602,  601,     602,  609,  610,     610,  603,  602,     603,  610,  611,     611,  604,  603,
     605,  612,  613,     613,  606,  605,     606,  613,  614,     614,  607,  606,     607,  614,  615,     615,  608,  607,
     608,  615,  616,     616,  609,  608,     609,  616,  617,     617,  610,  609,     610,  617,  618,     618,  611,  610,
     612,  619,  620,     620,  613,  612,     613,  620,  621,     621,  614,  613,     614,  621,  622,     622,  615,  614,
     615,  622,  623,     623,  616,  615,     616,  623,  624,     624,  617,  616,     617,  624,  625,     625,  618,  617,
     619,  626,  627,     627,  620,  619,     620,  627,  628,     628,  621,  620,     621,  628,  629,     629,  622,  621,
     622,  629,  630,     630,  623,  622,     623,  630,  631,     631,  624,  623,     624,  631,  632,     632,  625,  624,
     626,  633,  634,     634,  627,  626,     627,  634,  635,     635,  628,  627,     628,  635,  636,     636,  629,  628,
     629,  636,  637,     637,  630,  629,     630,  637,  638,     638,  631,  630,     631,  638,  639,     639,  632,  631,
     633,  640,  641,     641,  634,  633,     634,  641,  642,     642,  635,  634,     635,  642,  643,     643,  636,  635,
     636,  643,  644,     644,  637,  636,     637,  644,  645,     645,  638,  637,     638,  645,  646,     646,  639,  638,
     640,  647,  648,     648,  641,  640,     641,  648,  649,     649,  642,  641,     642,  649,  650,     650,  643,  642,
     643,  650,  651,     651,  644,  643,     644,  651,  652,     652,  645,  644,     645,  652,  653,     653,  646,  645,
     647,  654,  655,     655,  648,  647,     648,  655,  656,     656,  649,  648,     649,  656,  657,     657,  650,  649,
     650,  657,  658,     658,  651,  650,     651,  658,  659,     659,  652,  651,     652,  659,  660,     660,  653,  652,
     654,  577,  580,     580,  655,  654,     655,  580,  582,     582,  656,  655,     656,  582,  584,     584,  657,  656,
     657,  584,  586,     586,  658,  657,     658,  586,  588,     588,  659,  658,     659,  588,  590,     590,  660,  659,
     590,  589,  661,     661,  662,  590,     662,  661,  663,     663,  664,  662,     664,  663,  665,     665,  666,  664,
     666,  665,  667,     667,  668,  666,     668,  667,  669,     669,  670,  668,     670,  669,  671,     671,  672,  670,
     589,  597,  673,     673,  661,  589,     661,  673,  674,     674,  663,  661,     663,  674,  675,     675,  665,  663,
     665,  675,  676,     676,  667,  665,     667,  676,  677,     677,  669,  667,     669,  677,  678,     678,  671,  669,
     597,  604,  679,     679,  673,  597,     673,  679,  680,     680,  674,  673,     674,  680,  681,     681,  675,  674,
     675,  681,  682,     682,  676,  675,     676,  682,  683,     683,  677,  676,     677,  683,  684,     684,  678,  677,
     604,  611,  685,     685,  679,  604,     679,  685,  686,     686,  680,  679,     680,  686,  687,     687,  681,  680,
     681,  687,  688,     688,  682,  681,     682,  688,  689,     689,  683,  682,     683,  689,  690,     690,  684,  683,
     611,  618,  691,     691,  685,  611,     685,  691,  692,     692,  686,  685,     686,  692,  693,     693,  687,  686,
     687,  693,  694,     694,  688,  687,     688,  694,  695,     695,  689,  688,     689,  695,  696,     696,  690,  689,
     618,  625,  697,     697,  691,  618,     691,  697,  698,     698,  692,  691,     692,  698,  699,     699,  693,  692,
     693,  699,  700,     700,  694,  693,     694,  700,  701,     701,  695,  694,     695,  701,  702,     702,  696,  695,
     625,  632,  703,     703,  697,  625,     697,  703,  704,     704,  698,  697,     698,  704,  705,     705,  699,  698,
     699,  705,  706,     706,  700,  699,     700,  706,  707,     707,  701,  700,     701,  707,  708,     708,  702,  701,
     632,  639,  709,     709,  703,  632,     703,  709,  710,     710,  704,  703,     704,  710,  711,     711,  705,  704,
     705,  711,  712,     712,  706,  705,     706,  712,  713,     713,  707,  706,     707,  713,  714,     714,  708,  707,
     639,  646,  715,     715,  709,  639,     709,  715,  716,     716,  710,  709,     710,  716,  717,     717,  711,  710,
     711,  717,  718,     718,  712,  711,     712,  718,  719,     719,  713,  712,     713,  719,  720,     720,  714,  713,
     646,  653,  721,     721,  715,  646,     715,  721,  722,     722,  716,  715,     716,  722,  723,     723,  717,  716,
     717,  723,  724,     724,  718,  717,     718,  724,  725,     725,  719,  718,     719,  725,  726,     726,  720,  719,
     653,  660,  727,     727,  721,  653,     721,  727,  728,     728,  722,  721,     722,  728,  729,     729,  723,  722,
     723,  729,  730,     730,  724,  723,     724,  730,  731,     731,  725,  724,     725,  731,  732,     732,  726,  725,
     660,  590,  662,     662,  727,  660,     727,  662,  664,     664,  728,  727,     728,  664,  666,     666,  729,  728,
     729,  666,  668,     668,  730,  729,     730,  668,  670,     670,  731,  730,     731,  670,  672,     672,  732,  731,
     733,  734,  735,     735,  736,  733,     736,  735,  737,     737,  738,  736,     738,  737,  739,     739,  740,  738,
     740,  739,  741,     741,  742,  740,     742,  741,  743,     743,  744,  742,     744,  743,  745,     745,  746,  744,
     734,  747,  748,     748,  735,  734,     735,  748,  749,     749,  737,  735,     737,  749,  750,     750,  739,  737,
     739,  750,  751,     751,  741,  739,     741,  751,  752,     752,  743,  741,     743,  752,  753,     753,  745,  743,
     747,  754,  755,     755,  748,  747,     748,  755,  756,     756,  749,  748,     749,  756,  757,     757,  750,  749,
     750,  757,  758,     758,  751,  750,     751,  758,  759,     759,  752,  751,     752,  759,  760,     760,  753,  752,
     754,  761,  762,     762,  755,  754,     755,  762,  763,     763,  756,  755,     756,  763,  764,     764,  757,  756,
     757,  764,  765,     765,  758,  757,     758,  765,  766,     766,  759,  758,     759,  766,  767,     767,  760,  759,
     761,  768,  769,     769,  762,  761,     762,  769,  770,     770,  763,  762,     763,  770,  771,     771,  764,  763,
     764,  771,  772,     772,  765,  764,     765,  772,  773,     773,  766,  765,     766,  773,  774,     774,  767,  766,
     768,  775,  776,     776,  769,  768,     769,  776,  777,     777,  770,  769,     770,  777,  778,     778,  771,  770,
     771,  778,  779,     779,  772,  771,     772,  779,  780,     780,  773,  772,     773,  780,  781,     781,  774,  773,
     775,  782,  783,     783,  776,  775,     776,  783,  784,     784,  777,  776,     777,  784,  785,     785,  778,  777,
     778,  785,  786,     786,  779,  778,     779,  786,  787,     787,  780,  779,     780,  787,  788,     788,  781,  780,
     782,  789,  790,     790,  783,  782,     783,  790,  791,     791,  784,  783,     784,  791,  792,     792,  785,  784,
     785,  792,  793,     793,  786,  785,     786,  793,  794,     794,  787,  786,     787,  794,  795,     795,  788,  787,
     789,  796,  797,     797,  790,  789,     790,  797,  798,     798,  791,  790,     791,  798,  799,     799,  792,  791,
     792,  799,  800,     800,  793,  792,     793,  800,  801,     801,  794,  793,     794,  801,  802,     802,  795,  794,
     796,  803,  804,     804,  797,  796,     797,  804,  805,     805,  798,  797,     798,  805,  806,     806,  799,  798,
     799,  806,  807,     807,  800,  799,     800,  807,  808,     808,  801,  800,     801,  808,  809,     809,  802,  801,
     803,  810,  811,     811,  804,  803,     804,  811,  812,     812,  805,  804,     805,  812,  813,     813,  806,  805,
     806,  813,  814,     814,  807,  806,     807,  814,  815,     815,  808,  807,     808,  815,  816,     816,  809,  808,
     810,  733,  736,     736,  811,  810,     811,  736,  738,     738,  812,  811,     812,  738,  740,     740,  813,  812,
     813,  740,  742,     742,  814,  813,     814,  742,  744,     744,  815,  814,     815,  744,  746,     746,  816,  815,
     746,  745,  817,     817,  818,  746,     818,  817,  819,     819,  820,  818,     820,  819,  821,     821,  822,  820,
     822,  821,  823,     823,  824,  822,     824,  823,  825,     825,  826,  824,     826,  825,  827,     827,  828,  826,
     745,  753,  829,     829,  817,  745,     817,  829,  830,     830,  819,  817,     819,  830,  831,     831,  821,  819,
     821,  831,  832,     832,  823,  821,     823,  832,  833,     833,  825,  823,     825,  833,  834,     834,  827,  825,
     753,  760,  835,     835,  829,  753,     829,  835,  836,     836,  830,  829,     830,  836,  837,     837,  831,  830,
     831,  837,  838,     838,  832,  831,     832,  838,  839,     839,  833,  832,     833,  839,  840,     840,  834,  833,
     760,  767,  841,     841,  835,  760,     835,  841,  842,     842,  836,  835,     836,  842,  843,     843,  837,  836,
     837,  843,  844,     844,  838,  837,     838,  844,  845,     845,  839,  838,     839,  845,  846,     846,  840,  839,
     767,  774,  847,     847,  841,  767,     841,  847,  848,     848,  842,  841,     842,  848,  849,     849,  843,  842,
     843,  849,  850,     850,  844,  843,     844,  850,  851,     851,  845,  844,     845,  851,  852,     852,  846,  845,
     774,  781,  853,     853,  847,  774,     847,  853,  854,     854,  848,  847,     848,  854,  855,     855,  849,  848,
     849,  855,  856,     856,  850,  849,     850,  856,  857,     857,  851,  850,     851,  857,  858,     858,  852,  851,
     781,  788,  859,     859,  853,  781,     853,  859,  860,     860,  854,  853,     854,  860,  861,     861,  855,  854,
     855,  861,  862,     862,  856,  855,     856,  862,  863,     863,  857,  856,     857,  863,  864,     864,  858,  857,
     788,  795,  865,     865,  859,  788,     859,  865,  866,     866,  860,  859,     860,  866,  867,     867,  861,  860,
     861,  867,  868,     868,  862,  861,     862,  868,  869,     869,  863,  862,     863,  869,  870,     870,  864,  863,
     795,  802,  871,     871,  865,  795,     865,  871,  872,     872,  866,  865,     866,  872,  873,     873,  867,  866,
     867,  873,  874,     874,  868,  867,     868,  874,  875,     875,  869,  868,     869,  875,  876,     876,  870,  869,
     802,  809,  877,     877,  871,  802,     871,  877,  878,     878,  872,  871,     872,  878,  879,     879,  873,  872,
     873,  879,  880,     880,  874,  873,     874,  880,  881,     881,  875,  874,     875,  881,  882,     882,  876,  875,
     809,  816,  883,     883,  877,  809,     877,  883,  884,     884,  878,  877,     878,  884,  885,     885,  879,  878,
     879,  885,  886,     886,  880,  879,     880,  886,  887,     887,  881,  880,     881,  887,  888,     888,  882,  881,
     816,  746,  818,     818,  883,  816,     883,  818,  820,     820,  884,  883,     884,  820,  822,     822,  885,  884,
     885,  822,  824,     824,  886,  885,     886,  824,  826,     826,  887,  886,     887,  826,  828,     828,  888,  887,
     889,  890,  891,     890,  889,  892,     892,  893,  890,     893,  892,  894,     894,  895,  893,     895,  894,  896,
     896,  897,  895,     897,  896,  898,     898,  899,  897,     899,  898,  900,     900,  901,  899,     902,  889,  891,
     889,  902,  903,     903,  892,  889,     892,  903,  904,     904,  894,  892,     894,  904,  905,     905,  896,  894,
     896,  905,  906,     906,  898,  896,     898,  906,  907,     907,  900,  898,     908,  902,  891,     902,  908,  909,
     909,  903,  902,     903,  909,  910,     910,  904,  903,     904,  910,  911,     911,  905,  904,     905,  911,  912,
     912,  906,  905,     906,  912,  913,     913,  907,  906,     914,  908,  891,     908,  914,  915,     915,  909,  908,
     909,  915,  916,     916,  910,  909,     910,  916,  917,     917,  911,  910,     911,  917,  918,     918,  912,  911,
     912,  918,  919,     919,  913,  912,     920,  914,  891,     914,  920,  921,     921,  915,  914,     915,  921,  922,
     922,  916,  915,     916,  922,  923,     923,  917,  916,     917,  923,  924,     924,  918,  917,     918,  924,  925,
     925,  919,  918,     926,  920,  891,     920,  926,  927,     927,  921,  920,     921,  927,  928,     928,  922,  921,
     922,  928,  929,     929,  923,  922,     923,  929,  930,     930,  924,  923,     924,  930,  931,     931,  925,  924,
     932,  926,  891,     926,  932,  933,     933,  927,  926,     927,  933,  934,     934,  928,  927,     928,  934,  935,
     935,  929,  928,     929,  935,  936,     936,  930,  929,     930,  936,  937,     937,  931,  930,     938,  932,  891,
     932,  938,  939,     939,  933,  932,     933,  939,  940,     940,  934,  933,     934,  940,  941,     941,  935,  934,
     935,  941,  942,     942,  936,  935,     936,  942,  943,     943,  937,  936,     944,  938,  891,     938,  944,  945,
     945,  939,  938,     939,  945,  946,     946,  940,  939,     940,  946,  947,     947,  941,  940,     941,  947,  948,
     948,  942,  941,     942,  948,  949,     949,  943,  942,     950,  944,  891,     944,  950,  951,     951,  945,  944,
     945,  951,  952,     952,  946,  945,     946,  952,  953,     953,  947,  946,     947,  953,  954,     954,  948,  947,
     948,  954,  955,     955,  949,  948,     956,  950,  891,     950,  956,  957,     957,  951,  950,     951,  957,  958,
     958,  952,  951,     952,  958,  959,     959,  953,  952,     953,  959,  960,     960,  954,  953,     954,  960,  961,
     961,  955,  954,     962,  956,  891,     956,  962,  963,     963,  957,  956,     957,  963,  964,     964,  958,  957,
     958,  964,  965,     965,  959,  958,     959,  965,  966,     966,  960,  959,     960,  966,  967,     967,  961,  960,
     968,  962,  891,     962,  968,  969,     969,  963,  962,     963,  969,  970,     970,  964,  963,     964,  970,  971,
     971,  965,  964,     965,  971,  972,     972,  966,  965,     966,  972,  973,     973,  967,  966,     974,  968,  891,
     968,  974,  975,     975,  969,  968,     969,  975,  976,     976,  970,  969,     970,  976,  977,     977,  971,  970,
     971,  977,  978,     978,  972,  971,     972,  978,  979,     979,  973,  972,     980,  974,  891,     974,  980,  981,
     981,  975,  974,     975,  981,  982,     982,  976,  975,     976,  982,  983,     983,  977,  976,     977,  983,  984,
     984,  978,  977,     978,  984,  985,     985,  979,  978,     986,  980,  891,     980,  986,  987,     987,  981,  980,
     981,  987,  988,     988,  982,  981,     982,  988,  989,     989,  983,  982,     983,  989,  990,     990,  984,  983,
     984,  990,  991,     991,  985,  984,     992,  986,  891,     986,  992,  993,     993,  987,  986,     987,  993,  994,
     994,  988,  987,     988,  994,  995,     995,  989,  988,     989,  995,  996,     996,  990,  989,     990,  996,  997,
     997,  991,  990,     998,  992,  891,     992,  998,  999,     999,  993,  992,     993,  999, 1000,    1000,  994,  993,
     994, 1000, 1001,    1001,  995,  994,     995, 1001, 1002,    1002,  996,  995,     996, 1002, 1003,    1003,  997,  996,
    1004,  998,  891,     998, 1004, 1005,    1005,  999,  998,     999, 1005, 1006,    1006, 1000,  999,    1000, 1006, 1007,
    1007, 1001, 1000,    1001, 1007, 1008,    1008, 1002, 1001,    1002, 1008, 1009,    1009, 1003, 1002,    1010, 1004,  891,
    1004, 1010, 1011,    1011, 1005, 1004,    1005, 1011, 1012,    1012, 1006, 1005,    1006, 1012, 1013,    1013, 1007, 1006,
    1007, 1013, 1014,    1014, 1008, 1007,    1008, 1014, 1015,    1015, 1009, 1008,    1016, 1010,  891,    1010, 1016, 1017,
    1017, 1011, 1010,    1011, 1017, 1018,    1018, 1012, 1011,    1012, 1018, 1019,    1019, 1013, 1012,    1013, 1019, 1020,
    1020, 1014, 1013,    1014, 1020, 1021,    1021, 1015, 1014,    1022, 1016,  891,    1016, 1022, 1023,    1023, 1017, 1016,
    1017, 1023, 1024,    1024, 1018, 1017,    1018, 1024, 1025,    1025, 1019, 1018,    1019, 1025, 1026,    1026, 1020, 1019,
    1020, 1026, 1027,    1027, 1021, 1020,    1028, 1022,  891,    1022, 1028, 1029,    1029, 1023, 1022,    1023, 1029, 1030,
    1030, 1024, 1023,    1024, 1030, 1031,    1031, 1025, 1024,    1025, 1031, 1032,    1032, 1026, 1025,    1026, 1032, 1033,
    1033, 1027, 1026,     890, 1028,  891,    1028,  890,  893,     893, 1029, 1028,    1029,  893,  895,     895, 1030, 1029,
    1030,  895,  897,     897, 1031, 1030,    1031,  897,  899,     899, 1032, 1031,    1032,  899,  901,     901, 1033, 1032,
     901,  900, 1034,    1034, 1035,  901,    1035, 1034, 1036,    1036, 1037, 1035,    1037, 1036, 1038,    1038, 1039, 1037,
    1039, 1038, 1040,    1040, 1041, 1039,    1041, 1040, 1042,    1042, 1043, 1041,    1043, 1042, 1044,    1044, 1045, 1043,
     900,  907, 1046,    1046, 1034,  900,    1034, 1046, 1047,    1047, 1036, 1034,    1036, 1047, 1048,    1048, 1038, 1036,
    1038, 1048, 1049,    1049, 1040, 1038,    1040, 1049, 1050,    1050, 1042, 1040,    1042, 1050, 1051,    1051, 1044, 1042,
     907,  913, 1052,    1052, 1046,  907,    1046, 1052, 1053,    1053, 1047, 1046,    1047, 1053, 1054,    1054, 1048, 1047,
    1048, 1054, 1055,    1055, 1049, 1048,    1049, 1055, 1056,    1056, 1050, 1049,    1050, 1056, 1057,    1057, 1051, 1050,
     913,  919, 1058,    1058, 1052,  913,    1052, 1058, 1059,    1059, 1053, 1052,    1053, 1059, 1060,    1060, 1054, 1053,
    1054, 1060, 1061,    1061, 1055, 1054,    1055, 1061, 1062,    1062, 1056, 1055,    1056, 1062, 1063,    1063, 1057, 1056,
     919,  925, 1064,    1064, 1058,  919,    1058, 1064, 1065,    1065, 1059, 1058,    1059, 1065, 1066,    1066, 1060, 1059,
    1060, 1066, 1067,    1067, 1061, 1060,    1061, 1067, 1068,    1068, 1062, 1061,    1062, 1068, 1069,    1069, 1063, 1062,
     925,  931, 1070,    1070, 1064,  925,    1064, 1070, 1071,    1071, 1065, 1064,    1065, 1071, 1072,    1072, 1066, 1065,
    1066, 1072, 1073,    1073, 1067, 1066,    1067, 1073, 1074,    1074, 1068, 1067,    1068, 1074, 1075,    1075, 1069, 1068,
     931,  937, 1076,    1076, 1070,  931,    1070, 1076, 1077,    1077, 1071, 1070,    1071, 1077, 1078,    1078, 1072, 1071,
    1072, 1078, 1079,    1079, 1073, 1072,    1073, 1079, 1080,    1080, 1074, 1073,    1074, 1080, 1081,    1081, 1075, 1074,
     937,  943, 1082,    1082, 1076,  937,    1076, 1082, 1083,    1083, 1077, 1076,    1077, 1083, 1084,    1084, 1078, 1077,
    1078, 1084, 1085,    1085, 1079, 1078,    1079, 1085, 1086,    1086, 1080, 1079,    1080, 1086, 1087,    1087, 1081, 1080,
     943,  949, 1088,    1088, 1082,  943,    1082, 1088, 1089,    1089, 1083, 1082,    1083, 1089, 1090,    1090, 1084, 1083,
    1084, 1090, 1091,    1091, 1085, 1084,    1085, 1091, 1092,    1092, 1086, 1085,    1086, 1092, 1093,    1093, 1087, 1086,
     949,  955, 1094,    1094, 1088,  949,    1088, 1094, 1095,    1095, 1089, 1088,    1089, 1095, 1096,    1096, 1090, 1089,
    1090, 1096, 1097,    1097, 1091, 1090,    1091, 1097, 1098,    1098, 1092, 1091,    1092, 1098, 1099,    1099, 1093, 1092,
     955,  961, 1100,    1100, 1094,  955,    1094, 1100, 1101,    1101, 1095, 1094,    1095, 1101, 1102,    1102, 1096, 1095,
    1096, 1102, 1103,    1103, 1097, 1096,    1097, 1103, 1104,    1104, 1098, 1097,    1098, 1104, 1105,    1105, 1099, 1098,
     961,  967, 1106,    1106, 1100,  961,    1100, 1106, 1107,    1107, 1101, 1100,    1101, 1107, 1108,    1108, 1102, 1101,
    1102, 1108, 1109,    1109, 1103, 1102,    1103, 1109, 1110,    1110, 1104, 1103,    1104, 1110, 1111,    1111, 1105, 1104,
     967,  973, 1112,    1112, 1106,  967,    1106, 1112, 1113,    1113, 1107, 1106,    1107, 1113, 1114,    1114, 1108, 1107,
    1108, 1114, 1115,    1115, 1109, 1108,    1109, 1115, 1116,    1116, 1110, 1109,    1110, 1116, 1117,    1117, 1111, 1110,
     973,  979, 1118,    1118, 1112,  973,    1112, 1118, 1119,    1119, 1113, 1112,    1113, 1119, 1120,    1120, 1114, 1113,
    1114, 1120, 1121,    1121, 1115, 1114,    1115, 1121, 1122,    1122, 1116, 1115,    1116, 1122, 1123,    1123, 1117, 1116,
     979,  985, 1124,    1124, 1118,  979,    1118, 1124, 1125,    1125, 1119, 1118,    1119, 1125, 1126,    1126, 1120, 1119,
    1120, 1126, 1127,    1127, 1121, 1120,    1121, 1127, 1128,    1128, 1122, 1121,    1122, 1128, 1129,    1129, 1123, 1122,
     985,  991, 1130,    1130, 1124,  985,    1124, 1130, 1131,    1131, 1125, 1124,    1125, 1131, 1132,    1132, 1126, 1125,
    1126, 1132, 1133,    1133, 1127, 1126,    1127, 1133, 1134,    1134, 1128, 1127,    1128, 1134, 1135,    1135, 1129, 1128,
     991,  997, 1136,    1136, 1130,  991,    1130, 1136, 1137,    1137, 1131, 1130,    1131, 1137, 1138,    1138, 1132, 1131,
    1132, 1138, 1139,    1139, 1133, 1132,    1133, 1139, 1140,    1140, 1134, 1133,    1134, 1140, 1141,    1141, 1135, 1134,
     997, 1003, 1142,    1142, 1136,  997,    1136, 1142, 1143,    1143, 1137, 1136,    1137, 1143, 1144,    1144, 1138, 1137,
    1138, 1144, 1145,    1145, 1139, 1138,    1139, 1145, 1146,    1146, 1140, 1139,    1140, 1146, 1147,    1147, 1141, 1140,
    1003, 1009, 1148,    1148, 1142, 1003,    1142, 1148, 1149,    1149, 1143, 1142,    1143, 1149, 1150,    1150, 1144, 1143,
    1144, 1150, 1151,    1151, 1145, 1144,    1145, 1151, 1152,    1152, 1146, 1145,    1146, 1152, 1153,    1153, 1147, 1146,
    1009, 1015, 1154,    1154, 1148, 1009,    1148, 1154, 1155,    1155, 1149, 1148,    1149, 1155, 1156,    1156, 1150, 1149,
    1150, 1156, 1157,    1157, 1151, 1150,    1151, 1157, 1158,    1158, 1152, 1151,    1152, 1158, 1159,    1159, 1153, 1152,
    1015, 1021, 1160,    1160, 1154, 1015,    1154, 1160, 1161,    1161, 1155, 1154,    1155, 1161, 1162,    1162, 1156, 1155,
    1156, 1162, 1163,    1163, 1157, 1156,    1157, 1163, 1164,    1164, 1158, 1157,    1158, 1164, 1165,    1165, 1159, 1158,
    1021, 1027, 1166,    1166, 1160, 1021,    1160, 1166, 1167,    1167, 1161, 1160,    1161, 1167, 1168,    1168, 1162, 1161,
    1162, 1168, 1169,    1169, 1163, 1162,    1163, 1169, 1170,    1170, 1164, 1163,    1164, 1170, 1171,    1171, 1165, 1164,
    1027, 1033, 1172,    1172, 1166, 1027,    1166, 1172, 1173,    1173, 1167, 1166,    1167, 1173, 1174,    1174, 1168, 1167,
    1168, 1174, 1175,    1175, 1169, 1168,    1169, 1175, 1176,    1176, 1170, 1169,    1170, 1176, 1177,    1177, 1171, 1170,
    1033,  901, 1035,    1035, 1172, 1033,    1172, 1035, 1037,    1037, 1173, 1172,    1173, 1037, 1039,    1039, 1174, 1173,
    1174, 1039, 1041,    1041, 1175, 1174,    1175, 1041, 1043,    1043, 1176, 1175,    1176, 1043, 1045,    1045, 1177, 1176,

};

#   else

/*
 * ======= Teapot patches data: =======
 */

dim::vector3df __spTeapotPatchesData[32][4][4] = {
    dim::vector3df(-80.0, 0.0, 30.0), dim::vector3df(-80.0, -44.80, 30.0), dim::vector3df(-44.80, -80.0, 30.0), dim::vector3df(0.0, -80.0, 30.0),
    dim::vector3df(-80.0, 0.0, 12.0), dim::vector3df(-80.0, -44.80, 12.0), dim::vector3df(-44.80, -80.0, 12.0), dim::vector3df(0.0, -80.0, 12.0),
    dim::vector3df(-60.0, 0.0, 3.0), dim::vector3df(-60.0, -33.60, 3.0), dim::vector3df(-33.60, -60.0, 3.0), dim::vector3df(0.0, -60.0, 3.0),
    dim::vector3df(-60.0, 0.0, 0.0), dim::vector3df(-60.0, -33.60, 0.0), dim::vector3df(-33.60, -60.0, 0.0), dim::vector3df(0.0, -60.0, 0.0),
    
    dim::vector3df(0.0, -80.0, 30.0), dim::vector3df(44.80, -80.0, 30.0), dim::vector3df(80.0, -44.80, 30.0), dim::vector3df(80.0, 0.0, 30.0),
    dim::vector3df(0.0, -80.0, 12.0), dim::vector3df(44.80, -80.0, 12.0), dim::vector3df(80.0, -44.80, 12.0), dim::vector3df(80.0, 0.0, 12.0),
    dim::vector3df(0.0, -60.0, 3.0), dim::vector3df(33.60, -60.0, 3.0), dim::vector3df(60.0, -33.60, 3.0), dim::vector3df(60.0, 0.0, 3.0),
    dim::vector3df(0.0, -60.0, 0.0), dim::vector3df(33.60, -60.0, 0.0), dim::vector3df(60.0, -33.60, 0.0), dim::vector3df(60.0, 0.0, 0.0),
    
    dim::vector3df(-60.0, 0.0, 90.0), dim::vector3df(-60.0, -33.60, 90.0), dim::vector3df(-33.60, -60.0, 90.0), dim::vector3df(0.0, -60.0, 90.0),
    dim::vector3df(-70.0, 0.0, 69.0), dim::vector3df(-70.0, -39.20, 69.0), dim::vector3df(-39.20, -70.0, 69.0), dim::vector3df(0.0, -70.0, 69.0),
    dim::vector3df(-80.0, 0.0, 48.0), dim::vector3df(-80.0, -44.80, 48.0), dim::vector3df(-44.80, -80.0, 48.0), dim::vector3df(0.0, -80.0, 48.0),
    dim::vector3df(-80.0, 0.0, 30.0), dim::vector3df(-80.0, -44.80, 30.0), dim::vector3df(-44.80, -80.0, 30.0), dim::vector3df(0.0, -80.0, 30.0),
    
    dim::vector3df(0.0, -60.0, 90.0), dim::vector3df(33.60, -60.0, 90.0), dim::vector3df(60.0, -33.60, 90.0), dim::vector3df(60.0, 0.0, 90.0),
    dim::vector3df(0.0, -70.0, 69.0), dim::vector3df(39.20, -70.0, 69.0), dim::vector3df(70.0, -39.20, 69.0), dim::vector3df(70.0, 0.0, 69.0),
    dim::vector3df(0.0, -80.0, 48.0), dim::vector3df(44.80, -80.0, 48.0), dim::vector3df(80.0, -44.80, 48.0), dim::vector3df(80.0, 0.0, 48.0),
    dim::vector3df(0.0, -80.0, 30.0), dim::vector3df(44.80, -80.0, 30.0), dim::vector3df(80.0, -44.80, 30.0), dim::vector3df(80.0, 0.0, 30.0),
    
    dim::vector3df(-56.0, 0.0, 90.0), dim::vector3df(-56.0, -31.36, 90.0), dim::vector3df(-31.36, -56.0, 90.0), dim::vector3df(0.0, -56.0, 90.0),
    dim::vector3df(-53.50, 0.0, 95.25), dim::vector3df(-53.50, -29.96, 95.25), dim::vector3df(-29.96, -53.50, 95.25), dim::vector3df(0.0, -53.50, 95.25),
    dim::vector3df(-57.50, 0.0, 95.25), dim::vector3df(-57.50, -32.20, 95.25), dim::vector3df(-32.20, -57.50, 95.25), dim::vector3df(0.0, -57.50, 95.25),
    dim::vector3df(-60.0, 0.0, 90.0), dim::vector3df(-60.0, -33.60, 90.0), dim::vector3df(-33.60, -60.0, 90.0), dim::vector3df(0.0, -60.0, 90.0),
    
    dim::vector3df(0.0, -56.0, 90.0), dim::vector3df(31.36, -56.0, 90.0), dim::vector3df(56.0, -31.36, 90.0), dim::vector3df(56.0, 0.0, 90.0),
    dim::vector3df(0.0, -53.50, 95.25), dim::vector3df(29.96, -53.50, 95.25), dim::vector3df(53.50, -29.96, 95.25), dim::vector3df(53.50, 0.0, 95.25),
    dim::vector3df(0.0, -57.50, 95.25), dim::vector3df(32.20, -57.50, 95.25), dim::vector3df(57.50, -32.20, 95.25), dim::vector3df(57.50, 0.0, 95.25),
    dim::vector3df(0.0, -60.0, 90.0), dim::vector3df(33.60, -60.0, 90.0), dim::vector3df(60.0, -33.60, 90.0), dim::vector3df(60.0, 0.0, 90.0),
    
    dim::vector3df(80.0, 0.0, 30.0), dim::vector3df(80.0, 44.80, 30.0), dim::vector3df(44.80, 80.0, 30.0), dim::vector3df(0.0, 80.0, 30.0),
    dim::vector3df(80.0, 0.0, 12.0), dim::vector3df(80.0, 44.80, 12.0), dim::vector3df(44.80, 80.0, 12.0), dim::vector3df(0.0, 80.0, 12.0),
    dim::vector3df(60.0, 0.0, 3.0), dim::vector3df(60.0, 33.60, 3.0), dim::vector3df(33.60, 60.0, 3.0), dim::vector3df(0.0, 60.0, 3.0),
    dim::vector3df(60.0, 0.0, 0.0), dim::vector3df(60.0, 33.60, 0.0), dim::vector3df(33.60, 60.0, 0.0), dim::vector3df(0.0, 60.0, 0.0),
    
    dim::vector3df(0.0, 80.0, 30.0), dim::vector3df(-44.80, 80.0, 30.0), dim::vector3df(-80.0, 44.80, 30.0), dim::vector3df(-80.0, 0.0, 30.0),
    dim::vector3df(0.0, 80.0, 12.0), dim::vector3df(-44.80, 80.0, 12.0), dim::vector3df(-80.0, 44.80, 12.0), dim::vector3df(-80.0, 0.0, 12.0),
    dim::vector3df(0.0, 60.0, 3.0), dim::vector3df(-33.60, 60.0, 3.0), dim::vector3df(-60.0, 33.60, 3.0), dim::vector3df(-60.0, 0.0, 3.0),
    dim::vector3df(0.0, 60.0, 0.0), dim::vector3df(-33.60, 60.0, 0.0), dim::vector3df(-60.0, 33.60, 0.0), dim::vector3df(-60.0, 0.0, 0.0),
    
    dim::vector3df(60.0, 0.0, 90.0), dim::vector3df(60.0, 33.60, 90.0), dim::vector3df(33.60, 60.0, 90.0), dim::vector3df(0.0, 60.0, 90.0),
    dim::vector3df(70.0, 0.0, 69.0), dim::vector3df(70.0, 39.20, 69.0), dim::vector3df(39.20, 70.0, 69.0), dim::vector3df(0.0, 70.0, 69.0),
    dim::vector3df(80.0, 0.0, 48.0), dim::vector3df(80.0, 44.80, 48.0), dim::vector3df(44.80, 80.0, 48.0), dim::vector3df(0.0, 80.0, 48.0),
    dim::vector3df(80.0, 0.0, 30.0), dim::vector3df(80.0, 44.80, 30.0), dim::vector3df(44.80, 80.0, 30.0), dim::vector3df(0.0, 80.0, 30.0),
    
    dim::vector3df(0.0, 60.0, 90.0), dim::vector3df(-33.60, 60.0, 90.0), dim::vector3df(-60.0, 33.60, 90.0), dim::vector3df(-60.0, 0.0, 90.0),
    dim::vector3df(0.0, 70.0, 69.0), dim::vector3df(-39.20, 70.0, 69.0), dim::vector3df(-70.0, 39.20, 69.0), dim::vector3df(-70.0, 0.0, 69.0),
    dim::vector3df(0.0, 80.0, 48.0), dim::vector3df(-44.80, 80.0, 48.0), dim::vector3df(-80.0, 44.80, 48.0), dim::vector3df(-80.0, 0.0, 48.0),
    dim::vector3df(0.0, 80.0, 30.0), dim::vector3df(-44.80, 80.0, 30.0), dim::vector3df(-80.0, 44.80, 30.0), dim::vector3df(-80.0, 0.0, 30.0),
    
    dim::vector3df(56.0, 0.0, 90.0), dim::vector3df(56.0, 31.36, 90.0), dim::vector3df(31.36, 56.0, 90.0), dim::vector3df(0.0, 56.0, 90.0),
    dim::vector3df(53.50, 0.0, 95.25), dim::vector3df(53.50, 29.96, 95.25), dim::vector3df(29.96, 53.50, 95.25), dim::vector3df(0.0, 53.50, 95.25),
    dim::vector3df(57.50, 0.0, 95.25), dim::vector3df(57.50, 32.20, 95.25), dim::vector3df(32.20, 57.50, 95.25), dim::vector3df(0.0, 57.50, 95.25),
    dim::vector3df(60.0, 0.0, 90.0), dim::vector3df(60.0, 33.60, 90.0), dim::vector3df(33.60, 60.0, 90.0), dim::vector3df(0.0, 60.0, 90.0),
    
    dim::vector3df(0.0, 56.0, 90.0), dim::vector3df(-31.36, 56.0, 90.0), dim::vector3df(-56.0, 31.36, 90.0), dim::vector3df(-56.0, 0.0, 90.0),
    dim::vector3df(0.0, 53.50, 95.25), dim::vector3df(-29.96, 53.50, 95.25), dim::vector3df(-53.50, 29.96, 95.25), dim::vector3df(-53.50, 0.0, 95.25),
    dim::vector3df(0.0, 57.50, 95.25), dim::vector3df(-32.20, 57.50, 95.25), dim::vector3df(-57.50, 32.20, 95.25), dim::vector3df(-57.50, 0.0, 95.25),
    dim::vector3df(0.0, 60.0, 90.0), dim::vector3df(-33.60, 60.0, 90.0), dim::vector3df(-60.0, 33.60, 90.0), dim::vector3df(-60.0, 0.0, 90.0),
    
    dim::vector3df(-64.0, 0.0, 75.0), dim::vector3df(-64.0, 12.0, 75.0), dim::vector3df(-60.0, 12.0, 84.0), dim::vector3df(-60.0, 0.0, 84.0),
    dim::vector3df(-92.0, 0.0, 75.0), dim::vector3df(-92.0, 12.0, 75.0), dim::vector3df(-100.0, 12.0, 84.0), dim::vector3df(-100.0, 0.0, 84.0),
    dim::vector3df(-108.0, 0.0, 75.0), dim::vector3df(-108.0, 12.0, 75.0), dim::vector3df(-120.0, 12.0, 84.0), dim::vector3df(-120.0, 0.0, 84.0),
    dim::vector3df(-108.0, 0.0, 66.0), dim::vector3df(-108.0, 12.0, 66.0), dim::vector3df(-120.0, 12.0, 66.0), dim::vector3df(-120.0, 0.0, 66.0),
    
    dim::vector3df(-60.0, 0.0, 84.0), dim::vector3df(-60.0, -12.0, 84.0), dim::vector3df(-64.0, -12.0, 75.0), dim::vector3df(-64.0, 0.0, 75.0),
    dim::vector3df(-100.0, 0.0, 84.0), dim::vector3df(-100.0, -12.0, 84.0), dim::vector3df(-92.0, -12.0, 75.0), dim::vector3df(-92.0, 0.0, 75.0),
    dim::vector3df(-120.0, 0.0, 84.0), dim::vector3df(-120.0, -12.0, 84.0), dim::vector3df(-108.0, -12.0, 75.0), dim::vector3df(-108.0, 0.0, 75.0),
    dim::vector3df(-120.0, 0.0, 66.0), dim::vector3df(-120.0, -12.0, 66.0), dim::vector3df(-108.0, -12.0, 66.0), dim::vector3df(-108.0, 0.0, 66.0),
    
    dim::vector3df(-108.0, 0.0, 66.0), dim::vector3df(-108.0, 12.0, 66.0), dim::vector3df(-120.0, 12.0, 66.0), dim::vector3df(-120.0, 0.0, 66.0),
    dim::vector3df(-108.0, 0.0, 57.0), dim::vector3df(-108.0, 12.0, 57.0), dim::vector3df(-120.0, 12.0, 48.0), dim::vector3df(-120.0, 0.0, 48.0),
    dim::vector3df(-100.0, 0.0, 39.0), dim::vector3df(-100.0, 12.0, 39.0), dim::vector3df(-106.0, 12.0, 31.50), dim::vector3df(-106.0, 0.0, 31.50),
    dim::vector3df(-80.0, 0.0, 30.0), dim::vector3df(-80.0, 12.0, 30.0), dim::vector3df(-76.0, 12.0, 18.0), dim::vector3df(-76.0, 0.0, 18.0),
    
    dim::vector3df(-120.0, 0.0, 66.0), dim::vector3df(-120.0, -12.0, 66.0), dim::vector3df(-108.0, -12.0, 66.0), dim::vector3df(-108.0, 0.0, 66.0),
    dim::vector3df(-120.0, 0.0, 48.0), dim::vector3df(-120.0, -12.0, 48.0), dim::vector3df(-108.0, -12.0, 57.0), dim::vector3df(-108.0, 0.0, 57.0),
    dim::vector3df(-106.0, 0.0, 31.50), dim::vector3df(-106.0, -12.0, 31.50), dim::vector3df(-100.0, -12.0, 39.0), dim::vector3df(-100.0, 0.0, 39.0),
    dim::vector3df(-76.0, 0.0, 18.0), dim::vector3df(-76.0, -12.0, 18.0), dim::vector3df(-80.0, -12.0, 30.0), dim::vector3df(-80.0, 0.0, 30.0),
    
    dim::vector3df(68.0, 0.0, 51.0), dim::vector3df(68.0, 26.40, 51.0), dim::vector3df(68.0, 26.40, 18.0), dim::vector3df(68.0, 0.0, 18.0),
    dim::vector3df(104.0, 0.0, 51.0), dim::vector3df(104.0, 26.40, 51.0), dim::vector3df(124.0, 26.40, 27.0), dim::vector3df(124.0, 0.0, 27.0),
    dim::vector3df(92.0, 0.0, 78.0), dim::vector3df(92.0, 10.0, 78.0), dim::vector3df(96.0, 10.0, 75.0), dim::vector3df(96.0, 0.0, 75.0),
    dim::vector3df(108.0, 0.0, 90.0), dim::vector3df(108.0, 10.0, 90.0), dim::vector3df(132.0, 10.0, 90.0), dim::vector3df(132.0, 0.0, 90.0),
    
    dim::vector3df(68.0, 0.0, 18.0), dim::vector3df(68.0, -26.40, 18.0), dim::vector3df(68.0, -26.40, 51.0), dim::vector3df(68.0, 0.0, 51.0),
    dim::vector3df(124.0, 0.0, 27.0), dim::vector3df(124.0, -26.40, 27.0), dim::vector3df(104.0, -26.40, 51.0), dim::vector3df(104.0, 0.0, 51.0),
    dim::vector3df(96.0, 0.0, 75.0), dim::vector3df(96.0, -10.0, 75.0), dim::vector3df(92.0, -10.0, 78.0), dim::vector3df(92.0, 0.0, 78.0),
    dim::vector3df(132.0, 0.0, 90.0), dim::vector3df(132.0, -10.0, 90.0), dim::vector3df(108.0, -10.0, 90.0), dim::vector3df(108.0, 0.0, 90.0),
    
    dim::vector3df(108.0, 0.0, 90.0), dim::vector3df(108.0, 10.0, 90.0), dim::vector3df(132.0, 10.0, 90.0), dim::vector3df(132.0, 0.0, 90.0),
    dim::vector3df(112.0, 0.0, 93.0), dim::vector3df(112.0, 10.0, 93.0), dim::vector3df(141.0, 10.0, 93.75), dim::vector3df(141.0, 0.0, 93.75),
    dim::vector3df(116.0, 0.0, 93.0), dim::vector3df(116.0, 6.0, 93.0), dim::vector3df(138.0, 6.0, 94.50), dim::vector3df(138.0, 0.0, 94.50),
    dim::vector3df(112.0, 0.0, 90.0), dim::vector3df(112.0, 6.0, 90.0), dim::vector3df(128.0, 6.0, 90.0), dim::vector3df(128.0, 0.0, 90.0),
    
    dim::vector3df(132.0, 0.0, 90.0), dim::vector3df(132.0, -10.0, 90.0), dim::vector3df(108.0, -10.0, 90.0), dim::vector3df(108.0, 0.0, 90.0),
    dim::vector3df(141.0, 0.0, 93.75), dim::vector3df(141.0, -10.0, 93.75), dim::vector3df(112.0, -10.0, 93.0), dim::vector3df(112.0, 0.0, 93.0),
    dim::vector3df(138.0, 0.0, 94.50), dim::vector3df(138.0, -6.0, 94.50), dim::vector3df(116.0, -6.0, 93.0), dim::vector3df(116.0, 0.0, 93.0),
    dim::vector3df(128.0, 0.0, 90.0), dim::vector3df(128.0, -6.0, 90.0), dim::vector3df(112.0, -6.0, 90.0), dim::vector3df(112.0, 0.0, 90.0),
    
    dim::vector3df(50.0, 0.0, 90.0), dim::vector3df(50.0, 28.0, 90.0), dim::vector3df(28.0, 50.0, 90.0), dim::vector3df(0.0, 50.0, 90.0),
    dim::vector3df(52.0, 0.0, 90.0), dim::vector3df(52.0, 29.12, 90.0), dim::vector3df(29.12, 52.0, 90.0), dim::vector3df(0.0, 52.0, 90.0),
    dim::vector3df(54.0, 0.0, 90.0), dim::vector3df(54.0, 30.24, 90.0), dim::vector3df(30.24, 54.0, 90.0), dim::vector3df(0.0, 54.0, 90.0),
    dim::vector3df(56.0, 0.0, 90.0), dim::vector3df(56.0, 31.36, 90.0), dim::vector3df(31.36, 56.0, 90.0), dim::vector3df(0.0, 56.0, 90.0),
    
    dim::vector3df(0.0, 50.0, 90.0), dim::vector3df(-28.0, 50.0, 90.0), dim::vector3df(-50.0, 28.0, 90.0), dim::vector3df(-50.0, 0.0, 90.0),
    dim::vector3df(0.0, 52.0, 90.0), dim::vector3df(-29.12, 52.0, 90.0), dim::vector3df(-52.0, 29.12, 90.0), dim::vector3df(-52.0, 0.0, 90.0),
    dim::vector3df(0.0, 54.0, 90.0), dim::vector3df(-30.24, 54.0, 90.0), dim::vector3df(-54.0, 30.24, 90.0), dim::vector3df(-54.0, 0.0, 90.0),
    dim::vector3df(0.0, 56.0, 90.0), dim::vector3df(-31.36, 56.0, 90.0), dim::vector3df(-56.0, 31.36, 90.0), dim::vector3df(-56.0, 0.0, 90.0),
    
    dim::vector3df(-50.0, 0.0, 90.0), dim::vector3df(-50.0, -28.0, 90.0), dim::vector3df(-28.0, -50.0, 90.0), dim::vector3df(0.0, -50.0, 90.0),
    dim::vector3df(-52.0, 0.0, 90.0), dim::vector3df(-52.0, -29.12, 90.0), dim::vector3df(-29.12, -52.0, 90.0), dim::vector3df(0.0, -52.0, 90.0),
    dim::vector3df(-54.0, 0.0, 90.0), dim::vector3df(-54.0, -30.24, 90.0), dim::vector3df(-30.24, -54.0, 90.0), dim::vector3df(0.0, -54.0, 90.0),
    dim::vector3df(-56.0, 0.0, 90.0), dim::vector3df(-56.0, -31.36, 90.0), dim::vector3df(-31.36, -56.0, 90.0), dim::vector3df(0.0, -56.0, 90.0),
    
    dim::vector3df(0.0, -50.0, 90.0), dim::vector3df(28.0, -50.0, 90.0), dim::vector3df(50.0, -28.0, 90.0), dim::vector3df(50.0, 0.0, 90.0),
    dim::vector3df(0.0, -52.0, 90.0), dim::vector3df(29.12, -52.0, 90.0), dim::vector3df(52.0, -29.12, 90.0), dim::vector3df(52.0, 0.0, 90.0),
    dim::vector3df(0.0, -54.0, 90.0), dim::vector3df(30.24, -54.0, 90.0), dim::vector3df(54.0, -30.24, 90.0), dim::vector3df(54.0, 0.0, 90.0),
    dim::vector3df(0.0, -56.0, 90.0), dim::vector3df(31.36, -56.0, 90.0), dim::vector3df(56.0, -31.36, 90.0), dim::vector3df(56.0, 0.0, 90.0),
    
    dim::vector3df(8.0, 0.0, 102.0), dim::vector3df(8.0, 4.48, 102.0), dim::vector3df(4.48, 8.0, 102.0), dim::vector3df(0.0, 8.0, 102.0),
    dim::vector3df(16.0, 0.0, 96.0), dim::vector3df(16.0, 8.96, 96.0), dim::vector3df(8.96, 16.0, 96.0), dim::vector3df(0.0, 16.0, 96.0),
    dim::vector3df(52.0, 0.0, 96.0), dim::vector3df(52.0, 29.12, 96.0), dim::vector3df(29.12, 52.0, 96.0), dim::vector3df(0.0, 52.0, 96.0),
    dim::vector3df(52.0, 0.0, 90.0), dim::vector3df(52.0, 29.12, 90.0), dim::vector3df(29.12, 52.0, 90.0), dim::vector3df(0.0, 52.0, 90.0),
    
    dim::vector3df(0.0, 8.0, 102.0), dim::vector3df(-4.48, 8.0, 102.0), dim::vector3df(-8.0, 4.48, 102.0), dim::vector3df(-8.0, 0.0, 102.0),
    dim::vector3df(0.0, 16.0, 96.0), dim::vector3df(-8.96, 16.0, 96.0), dim::vector3df(-16.0, 8.96, 96.0), dim::vector3df(-16.0, 0.0, 96.0),
    dim::vector3df(0.0, 52.0, 96.0), dim::vector3df(-29.12, 52.0, 96.0), dim::vector3df(-52.0, 29.12, 96.0), dim::vector3df(-52.0, 0.0, 96.0),
    dim::vector3df(0.0, 52.0, 90.0), dim::vector3df(-29.12, 52.0, 90.0), dim::vector3df(-52.0, 29.12, 90.0), dim::vector3df(-52.0, 0.0, 90.0),
    
    dim::vector3df(-8.0, 0.0, 102.0), dim::vector3df(-8.0, -4.48, 102.0), dim::vector3df(-4.48, -8.0, 102.0), dim::vector3df(0.0, -8.0, 102.0),
    dim::vector3df(-16.0, 0.0, 96.0), dim::vector3df(-16.0, -8.96, 96.0), dim::vector3df(-8.96, -16.0, 96.0), dim::vector3df(0.0, -16.0, 96.0),
    dim::vector3df(-52.0, 0.0, 96.0), dim::vector3df(-52.0, -29.12, 96.0), dim::vector3df(-29.12, -52.0, 96.0), dim::vector3df(0.0, -52.0, 96.0),
    dim::vector3df(-52.0, 0.0, 90.0), dim::vector3df(-52.0, -29.12, 90.0), dim::vector3df(-29.12, -52.0, 90.0), dim::vector3df(0.0, -52.0, 90.0),
    
    dim::vector3df(0.0, -8.0, 102.0), dim::vector3df(4.48, -8.0, 102.0), dim::vector3df(8.0, -4.48, 102.0), dim::vector3df(8.0, 0.0, 102.0),
    dim::vector3df(0.0, -16.0, 96.0), dim::vector3df(8.96, -16.0, 96.0), dim::vector3df(16.0, -8.96, 96.0), dim::vector3df(16.0, 0.0, 96.0),
    dim::vector3df(0.0, -52.0, 96.0), dim::vector3df(29.12, -52.0, 96.0), dim::vector3df(52.0, -29.12, 96.0), dim::vector3df(52.0, 0.0, 96.0),
    dim::vector3df(0.0, -52.0, 90.0), dim::vector3df(29.12, -52.0, 90.0), dim::vector3df(52.0, -29.12, 90.0), dim::vector3df(52.0, 0.0, 90.0),
    
    dim::vector3df(0.0, 0.0, 120.0), dim::vector3df(0.0, 0.0, 120.0), dim::vector3df(0.0, 0.0, 120.0), dim::vector3df(0.0, 0.0, 120.0),
    dim::vector3df(32.0, 0.0, 120.0), dim::vector3df(32.0, 18.0, 120.0), dim::vector3df(18.0, 32.0, 120.0), dim::vector3df(0.0, 32.0, 120.0),
    dim::vector3df(0.0, 0.0, 108.0), dim::vector3df(0.0, 0.0, 108.0), dim::vector3df(0.0, 0.0, 108.0), dim::vector3df(0.0, 0.0, 108.0),
    dim::vector3df(8.0, 0.0, 102.0), dim::vector3df(8.0, 4.48, 102.0), dim::vector3df(4.48, 8.0, 102.0), dim::vector3df(0.0, 8.0, 102.0),
    
    dim::vector3df(0.0, 0.0, 120.0), dim::vector3df(0.0, 0.0, 120.0), dim::vector3df(0.0, 0.0, 120.0), dim::vector3df(0.0, 0.0, 120.0),
    dim::vector3df(0.0, 32.0, 120.0), dim::vector3df(-18.0, 32.0, 120.0), dim::vector3df(-32.0, 18.0, 120.0), dim::vector3df(-32.0, 0.0, 120.0),
    dim::vector3df(0.0, 0.0, 108.0), dim::vector3df(0.0, 0.0, 108.0), dim::vector3df(0.0, 0.0, 108.0), dim::vector3df(0.0, 0.0, 108.0),
    dim::vector3df(0.0, 8.0, 102.0), dim::vector3df(-4.48, 8.0, 102.0), dim::vector3df(-8.0, 4.48, 102.0), dim::vector3df(-8.0, 0.0, 102.0),
    
    dim::vector3df(0.0, 0.0, 120.0), dim::vector3df(0.0, 0.0, 120.0), dim::vector3df(0.0, 0.0, 120.0), dim::vector3df(0.0, 0.0, 120.0),
    dim::vector3df(-32.0, 0.0, 120.0), dim::vector3df(-32.0, -18.0, 120.0), dim::vector3df(-18.0, -32.0, 120.0), dim::vector3df(0.0, -32.0, 120.0),
    dim::vector3df(0.0, 0.0, 108.0), dim::vector3df(0.0, 0.0, 108.0), dim::vector3df(0.0, 0.0, 108.0), dim::vector3df(0.0, 0.0, 108.0),
    dim::vector3df(-8.0, 0.0, 102.0), dim::vector3df(-8.0, -4.48, 102.0), dim::vector3df(-4.48, -8.0, 102.0), dim::vector3df(0.0, -8.0, 102.0),
    
    dim::vector3df(0.0, 0.0, 120.0), dim::vector3df(0.0, 0.0, 120.0), dim::vector3df(0.0, 0.0, 120.0), dim::vector3df(0.0, 0.0, 120.0),
    dim::vector3df(0.0, -32.0, 120.0), dim::vector3df(18.0, -32.0, 120.0), dim::vector3df(32.0, -18.0, 120.0), dim::vector3df(32.0, 0.0, 120.0),
    dim::vector3df(0.0, 0.0, 108.0), dim::vector3df(0.0, 0.0, 108.0), dim::vector3df(0.0, 0.0, 108.0), dim::vector3df(0.0, 0.0, 108.0),
    dim::vector3df(0.0, -8.0, 102.0), dim::vector3df(4.48, -8.0, 102.0), dim::vector3df(8.0, -4.48, 102.0), dim::vector3df(8.0, 0.0, 102.0),
};

#   endif

#endif



} // /namespace scene

} // /namespace sp







// ================================================================================
