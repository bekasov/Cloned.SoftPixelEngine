/*
 * Mesh generator file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Base/spBasicMeshGenerator.hpp"
#include "Base/spImageBuffer.hpp"
#include "SceneGraph/spSceneGraph.hpp"
#include "SceneGraph/spSceneManager.hpp"

#include <boost/foreach.hpp>
#include <boost/shared_array.hpp>


namespace sp
{
namespace scene
{


#ifdef SP_COMPILE_WITH_PRIMITIVE_TEAPOT

#   ifdef SP_COMPILE_WITH_PRIMITIVE_TEAPOT_DYNAMIC
extern const dim::vector3df __spTeapotPatchesData[32][4][4];
#   else
extern const f32 __spTeapotVertices[];
extern const s32 __spTeapotIndices[];
#   endif

#endif

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
        default:
            break;
    }
}


namespace MeshGenerator
{

/* === Static functions === */

static void addVertex(
    video::MeshBuffer &Surface, f32 x, f32 y, f32 z, f32 u = 0.0f, f32 v = 0.0f, f32 w = 0.0f)
{
    Surface.addVertex(dim::vector3df(x, y, z), dim::vector3df(u, v, w));
}

static void addFace(video::MeshBuffer &Surface, u32 v0, u32 v1, u32 v2)
{
    Surface.addTriangle(v0, v1, v2);
}
static void addFace(video::MeshBuffer &Surface, u32 v0, u32 v1, u32 v2, u32 v3)
{
    Surface.addTriangle(v0, v1, v2);
    Surface.addTriangle(v0, v2, v3);
}
static void addFace(video::MeshBuffer &Surface, u32 v0, u32 v1, u32 v2, u32 v3, u32 v4)
{
    Surface.addTriangle(v0, v1, v2);
    Surface.addTriangle(v0, v2, v3);
    Surface.addTriangle(v0, v3, v4);
}

static void addFace(
    video::MeshBuffer &Surface,
    const dim::vector3df &v0, const dim::vector3df &v1, const dim::vector3df &v2,
    const dim::point2df &t0, const dim::point2df &t1, const dim::point2df &t2)
{
    Surface.addVertex(v0, t0);
    Surface.addVertex(v1, t1);
    Surface.addVertex(v2, t2);
    
    Surface.addTriangle(0, 1, 2);
    
    Surface.addIndexOffset(3);
}

static void addFace(
    video::MeshBuffer &Surface,
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
    
    addFace(Surface, v0, v1, v2, t0, t1, t2);
}

static void addFace(
    video::MeshBuffer &Surface,
    const dim::vector3df* Vertices, const dim::point2df* TexCoords,
    u32 v0, u32 v1, u32 v2, u32 v3, u32 v4)
{
    if (Vertices)
    {
        if (TexCoords)
        {
            Surface.addVertex(Vertices[v0], TexCoords[v0]);
            Surface.addVertex(Vertices[v1], TexCoords[v1]);
            Surface.addVertex(Vertices[v2], TexCoords[v2]);
            Surface.addVertex(Vertices[v3], TexCoords[v3]);
            Surface.addVertex(Vertices[v4], TexCoords[v4]);
        }
        else
        {
            Surface.addVertex(Vertices[v0], dim::point2df(0.5f, 0.0f));
            Surface.addVertex(Vertices[v1], dim::point2df(1.0f, 0.4f));
            Surface.addVertex(Vertices[v2], dim::point2df(0.7f, 1.0f));
            Surface.addVertex(Vertices[v3], dim::point2df(0.2f, 1.0f));
            Surface.addVertex(Vertices[v4], dim::point2df(0.0f, 0.4f));
        }
        
        Surface.addTriangle(0, 1, 2);
        Surface.addTriangle(0, 2, 3);
        Surface.addTriangle(0, 3, 4);
        
        Surface.addIndexOffset(5);
    }
    else
        addFace(Surface, v0, v1, v2, v3, v4);
}

static void addQuadFace(
    video::MeshBuffer &Surface, s32 SegmentsVert, s32 SegmentsHorz,
    dim::vector3df v0, dim::point2df t0, dim::vector3df v1, dim::point2df t1,
    dim::vector3df diru, dim::vector3df dirv,
    bool FaceLinkCCW = false)
{
    /* Temporary variables */
    const s32 DetailVert = SegmentsVert;
    const s32 DetailHorz = SegmentsHorz;
    
    s32 x, y, c = 0;
    s32 i0, i1, i2, i3;
    
    dim::vector3df u, v;
    
    /* Create all vertices */
    for (y = 0; y <= DetailVert; ++y)
    {
        v = dirv * (static_cast<f32>(y) / DetailVert);
        
        for (x = 0; x <= DetailHorz; ++x, ++c)
        {
            u = diru * (static_cast<f32>(x) / DetailHorz);
            
            addVertex(
                Surface,
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
                addFace(Surface, i3, i2, i1, i0);
            else
                addFace(Surface, i0, i1, i2, i3);
        }
    }
    
    Surface.addIndexOffset(c);
}

static void finalizeFlat(video::MeshBuffer &Surface)
{
    Surface.updateIndexBuffer();
    Surface.updateNormals(video::SHADING_FLAT);
}

static void finalizeGouraud(video::MeshBuffer &Surface)
{
    Surface.updateIndexBuffer();
    Surface.updateNormals(video::SHADING_GOURAUD);
}

static void warning3DModelCompilation(const io::stringc &Name)
{
    io::Log::warning("Engine was not compiled with the \"" + Name + "\" standard 3D model");
}


/* === Global functions === */

SP_EXPORT void createMesh(
    video::MeshBuffer &Surface, const EBasicMeshes Model, SMeshConstruct Construct)
{
    /* Setup mesh construction */
    Construct.checkDefaultSegments(Model, Construct.SegmentsVert);
    Construct.checkDefaultSegments(Model, Construct.SegmentsHorz);
    Construct.checkDefaultShading(Model);
    
    if (Construct.SegmentsVert <= 0 || Construct.SegmentsHorz <= 0)
        return;
    
    /* Create specified 3D model */
    // note -> Since version 3.2 inner/outer radii have exchanged its meaning
    switch (Model)
    {
        case MESH_CUBE:
            createCube(Surface, Construct.RadiusInner, Construct.SegmentsVert);
            break;
        case MESH_CONE:
            createCone(Surface, Construct.RadiusInner, 1.0f, Construct.SegmentsVert, Construct.HasCap);
            break;
        case MESH_CYLINDER:
            createCylinder(Surface, Construct.RadiusInner, 1.0f, Construct.SegmentsVert, Construct.HasCap);
            break;
        case MESH_SPHERE:
            createSphere(Surface, Construct.RadiusInner, Construct.SegmentsVert);
            break;
        case MESH_ICOSPHERE:
            createIcoSphere(Surface, Construct.RadiusInner, Construct.SegmentsVert);
            break;
        case MESH_TORUS:
            createTorus(Surface, Construct.RadiusInner, Construct.RadiusOuter, Construct.SegmentsVert);
            break;
        case MESH_TORUSKNOT:
            createTorusknot(Surface, Construct.RadiusInner, Construct.RadiusOuter, Construct.SegmentsVert);
            break;
        case MESH_SPIRAL:
            createSpiral(
                Surface, Construct.RadiusInner, Construct.RadiusOuter, Construct.RotationDegree,
                Construct.RotationDistance, Construct.SegmentsVert, Construct.HasCap
            );
            break;
        case MESH_PIPE:
            createPipe(Surface, Construct.RadiusInner, Construct.RadiusOuter, 1.0f, Construct.SegmentsVert, Construct.HasCap);
            break;
        case MESH_PLANE:
            createPlane(Surface, Construct.SegmentsVert);
            break;
        case MESH_DISK:
            createDisk(Surface, Construct.RadiusInner, Construct.RadiusOuter, Construct.SegmentsVert, !Construct.HasCap);
            break;
        case MESH_CUBOCTAHEDRON:
            createCuboctahedron(Surface);
            break;
        case MESH_TETRAHEDRON:
            createTetrahedron(Surface);
            break;
        case MESH_OCTAHEDRON:
            createOctahedron(Surface);
            break;
        case MESH_DODECAHEDRON:
            createDodecahedron(Surface);
            break;
        case MESH_ICOSAHEDRON:
            createIcosahedron(Surface);
            break;
        case MESH_TEAPOT:
            createTeapot(Surface);
            break;
            
        default:
            io::Log::warning("Unknown standard 3D model");
            break;
    }
}

SP_EXPORT void createCube(video::MeshBuffer &Surface, f32 Radius, s32 SegmentsVert, s32 SegmentsHorz)
{
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
    
    Surface.clearVertices();
    
    // Back
    addQuadFace(
        Surface, SegmentsVert, SegmentsHorz,
        dim::vector3df( Radius,  Radius,  Radius), dim::point2df(0, 0),
        dim::vector3df(-Radius, -Radius,  Radius), dim::point2df(1, 1),
        dim::vector3df(1, 0, 0), dim::vector3df(0, 1, 0)
    );
    
    // Front
    addQuadFace(
        Surface, SegmentsVert, SegmentsHorz,
        dim::vector3df(-Radius,  Radius, -Radius), dim::point2df(0, 0),
        dim::vector3df( Radius, -Radius, -Radius), dim::point2df(1, 1),
        dim::vector3df(1, 0, 0), dim::vector3df(0, 1, 0)
    );
    
    // Top
    addQuadFace(
        Surface, SegmentsVert, SegmentsHorz,
        dim::vector3df(-Radius,  Radius,  Radius), dim::point2df(0, 0),
        dim::vector3df( Radius,  Radius, -Radius), dim::point2df(1, 1),
        dim::vector3df(1, 0, 0), dim::vector3df(0, 0, 1)
    );
    
    // Bottom
    addQuadFace(
        Surface, SegmentsVert, SegmentsHorz,
        dim::vector3df(-Radius, -Radius,  Radius), dim::point2df(0, 0),
        dim::vector3df( Radius, -Radius, -Radius), dim::point2df(1, 1),
        dim::vector3df(1, 0, 0), dim::vector3df(0, 0, 1), true
    );
    
    // Left
    addQuadFace(
        Surface, SegmentsVert, SegmentsHorz,
        dim::vector3df(-Radius,  Radius,  Radius), dim::point2df(0, 0),
        dim::vector3df(-Radius, -Radius, -Radius), dim::point2df(1, 1),
        dim::vector3df(0, 0, 1), dim::vector3df(0, 1, 0)
    );
    
    // Right
    addQuadFace(
        Surface, SegmentsVert, SegmentsHorz,
        dim::vector3df( Radius,  Radius, -Radius), dim::point2df(0, 0),
        dim::vector3df( Radius, -Radius,  Radius), dim::point2df(1, 1),
        dim::vector3df(0, 0, 1), dim::vector3df(0, 1, 0)
    );
    
    finalizeFlat(Surface);
    
    #else
    
    warning3DModelCompilation("Cube");
    
    #endif
}

SP_EXPORT void createCone(
    video::MeshBuffer &Surface, f32 Radius, f32 Height, s32 Segments, bool HasCap)
{
    #ifdef SP_COMPILE_WITH_PRIMITIVE_CONE
    
    Surface.clearVertices();
    
    const s32 Segs = math::Max(3, Segments);
    
    const f32 c = 360.0f / Segs;
    f32 i;
    s32 v;
    
    /* Build body */
    Surface.addVertex(dim::vector3df(0.0f, 0.5f, 0.0f), dim::point2df(0.5f, 0));
    
    for (i = 0, v = 0; i < 360; i += c, v += 2)
    {
        Surface.addVertex(
            dim::vector3df(math::Sin(static_cast<f32>(i + c))*Radius, -0.5f, math::Cos(static_cast<f32>(i + c))*Radius),
            dim::point2df((360 - i - c)/360, 1)
        );
        Surface.addVertex(
            dim::vector3df(math::Sin(static_cast<f32>(i))*Radius, -0.5f, math::Cos(static_cast<f32>(i))*Radius),
            dim::point2df((360 - i)/360, 1)
        );
        Surface.addTriangle(v + 2, v + 1, 0);
    }
    
    /* Build cap */
    if (HasCap)
    {
        const f32 SinRadius = math::Sin(static_cast<f32>(c))*Radius;
        const f32 CosRadius = math::Cos(static_cast<f32>(c))*Radius;
        
        Surface.addIndexOffset(v + 1);
        Surface.addVertex(dim::vector3df(0.0f, -0.5f, Radius), dim::point2df(0.5f, 1));
        Surface.addVertex(
            dim::vector3df(SinRadius, -0.5f, CosRadius), dim::point2df(0.5f + SinRadius, 0.5f + CosRadius)
        );
        
        for (i = c*2, v = 0; i < 360; i += c, ++v)
        {
            const f32 SinRadius = math::Sin(static_cast<f32>(i))*Radius;
            const f32 CosRadius = math::Cos(static_cast<f32>(i))*Radius;
            
            Surface.addVertex(
                dim::vector3df(SinRadius, -0.5f, CosRadius),
                dim::point2df(0.5f + SinRadius, 0.5f + CosRadius)
            );
            Surface.addTriangle(v + 2, v + 1, 0);
        }
    }
    
    finalizeGouraud(Surface);
    
    #else
    
    warning3DModelCompilation("Cone");
    
    #endif
}

SP_EXPORT void createCylinder(
    video::MeshBuffer &Surface, f32 Radius, f32 Height, s32 Segments, bool HasCap)
{
    #ifdef SP_COMPILE_WITH_PRIMITIVE_CYLINDER
    
    Surface.clearVertices();
    
    /* Temporary variables */
    const s32 Detail = math::MinMax(Segments, 3, 360);
    
    const f32 c = 360.0f / Detail;
    
    const f32 SinC = math::Sin(c);
    const f32 CosC = math::Cos(c);
    
    f32 i;
    s32 v = 0;
    
    /* Body */
    Surface.addVertex(dim::vector3df(0.0f,  0.5f, 0.5f), dim::point2df(3, 0));
    Surface.addVertex(dim::vector3df(0.0f, -0.5f, 0.5f), dim::point2df(3, 1));
    
    for (i = c; i <= 360; i += c)
    {
        const f32 SinI = math::Sin(static_cast<f32>(i))*Radius;
        const f32 CosI = math::Cos(static_cast<f32>(i))*Radius;
        
        Surface.addVertex(dim::vector3df(SinI,  0.5f, CosI), dim::point2df((360 - i)/360*3, 0));
        Surface.addVertex(dim::vector3df(SinI, -0.5f, CosI), dim::point2df((360 - i)/360*3, 1));
        Surface.addTriangle(3, 2, 0); Surface.addTriangle(1, 3, 0);
        Surface.addIndexOffset(2);
    }
    
    /* Cap */
    if (HasCap)
    {
        /* Top */
        Surface.addIndexOffset(2);
        Surface.addVertex(dim::vector3df(       0.0f, 0.5f,      Radius), dim::point2df(            0.5f,             0.0f));
        Surface.addVertex(dim::vector3df(SinC*Radius, 0.5f, CosC*Radius), dim::point2df(0.5f + SinC*0.5f, 0.5f - CosC*0.5f));
        
        for (i = c*2, v = 0; i < 360; i += c, ++v)
        {
            const f32 SinI = math::Sin(static_cast<f32>(i));
            const f32 CosI = math::Cos(static_cast<f32>(i));
            
            Surface.addVertex(
                dim::vector3df(SinI*Radius, 0.5f, CosI*Radius),
                dim::point2df(0.5f + SinI*0.5f, 0.5f - CosI*0.5f)
            );
            Surface.addTriangle(0, v+1, v+2);
        }
        Surface.addIndexOffset(v+2);
        
        /* Bottom */
        Surface.addVertex(dim::vector3df(       0.0f, -0.5f,      Radius), dim::point2df(            0.5f,             1.0f));
        Surface.addVertex(dim::vector3df(SinC*Radius, -0.5f, CosC*Radius), dim::point2df(0.5f + SinC*0.5f, 0.5f + CosC*0.5f));
        
        for (i = c*2, v = 0; i < 360; i += c, ++v)
        {
            const f32 SinI = math::Sin(static_cast<f32>(i));
            const f32 CosI = math::Cos(static_cast<f32>(i));
            
            Surface.addVertex(
                dim::vector3df(SinI*Radius, -0.5f, CosI*Radius),
                dim::point2df(0.5f + SinI*0.5f, 0.5f + CosI*0.5f)
            );
            Surface.addTriangle(v+2, v+1, 0);
        }
    }
    
    finalizeGouraud(Surface);
    
    #else
    
    warning3DModelCompilation("Cylinder");
    
    #endif
}

SP_EXPORT void createSphere(video::MeshBuffer &Surface, f32 Radius, s32 Segments)
{
    #ifdef SP_COMPILE_WITH_PRIMITIVE_SPHERE
    
    Surface.clearVertices();
    
    /* Temporary variables */
    s32 i, j;
    
    const s32 Detail = math::MinMax(Segments, 2, 180) * 2;
    
    f32 x, y, z, u, v;
    
    const f32 FinalDetail = 360.0f / Detail;
    s32 DegX = Detail;
    s32 DegY = Detail / 2;
    
    u32 v0, v1, v2, v3;
    
    /* Create vertices */
    for (i = 0; i <= DegY; ++i)
    {
        const f32 SinI = math::Sin(FinalDetail*i);
        
        /* Height */
        y = math::Cos(FinalDetail*i) * Radius;
        
        for (j = 0; j <= DegX; ++j)
        {
            /* Coordination */
            x = math::Sin(FinalDetail*j) * SinI * Radius;
            z = math::Cos(FinalDetail*j) * SinI * Radius;
            
            /* UV-Mapping */
            u = static_cast<f32>(j) / DegX;
            v = static_cast<f32>(i) / DegY;
            
            /* Add vertex */
            Surface.addVertex(dim::vector3df(x, y, z), dim::point2df(u, v));
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
        
        Surface.addTriangle(v0, v1, v2);
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
            
            Surface.addTriangle(v2, v1, v0);
            Surface.addTriangle(v1, v2, v3);
        }
    }
    
    /* Bottom */
    for (j = 0; j < DegX - 1; ++j)
    {
        v0 = (DegY-1)*DegX + j;
        v1 = (DegY-1)*DegX + j+1;
        v2 = DegY*DegX + j;
        
        Surface.addTriangle(v2, v1, v0);
    }
    
    finalizeGouraud(Surface);
    
    #else
    
    warning3DModelCompilation("Sphere");
    
    #endif
}

SP_EXPORT void createIcoSphere(video::MeshBuffer &Surface, f32 Radius, s32 Segments)
{
    #ifdef SP_COMPILE_WITH_PRIMITIVE_ICOSPHERE
    
    Surface.clearVertices();
    
    /* Temporary variables */
    const s32 Detail = math::MinMax(Segments, 1, 8);
    
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
        addFace(Surface, Face.PointA, Face.PointB, Face.PointC);
    
    finalizeGouraud(Surface);
    
    #else
    
    warning3DModelCompilation("IcoSphere");
    
    #endif
}

SP_EXPORT void createTorus(
    video::MeshBuffer &Surface, f32 RadiusOuter, f32 RadiusInner, s32 Segments)
{
    #ifdef SP_COMPILE_WITH_PRIMITIVE_TORUS
    
    Surface.clearVertices();
    
    /* Temporary variables */
    s32 i, j;
    
    const s32 Detail = math::MinMax(Segments, 2, 180) * 2;
    
    f32 x, y, z, u, v;
    
    const f32 FinalDetail = 360.0f / Detail;
    s32 DegX = Detail;
    s32 DegY = Detail / 2;
    
    u32 v0, v1, v2, v3, c;
    
    /* Create vertices - outside */
    for (i = 0; i <= DegY; ++i)
    {
        const f32 SinI = math::Sin(static_cast<f32>(i*FinalDetail));
        const f32 CosI = math::Cos(static_cast<f32>(i*FinalDetail));
        
        /* Height */
        y = CosI * RadiusInner;
        
        for (j = 0; j <= DegX; ++j)
        {
            const f32 SinJ = math::Sin(static_cast<f32>(j*FinalDetail));
            const f32 CosJ = math::Cos(static_cast<f32>(j*FinalDetail));
            
            /* Coordination */
            x = SinJ*RadiusOuter + SinJ * SinI * RadiusInner;
            z = CosJ*RadiusOuter + CosJ * SinI * RadiusInner;
            
            /* UV-Mapping */
            u = static_cast<f32>(j) / DegX;
            v = static_cast<f32>(i) / DegY;
            
            /* Add vertex */
            Surface.addVertex(dim::vector3df(x, y, z), dim::point2df(u, v));
        }
    }
    
    /* Create vertices - inside */
    for (i = 0; i <= DegY; ++i)
    {
        const f32 SinI = math::Sin(static_cast<f32>(i*FinalDetail));
        const f32 CosI = math::Cos(static_cast<f32>(i*FinalDetail));
        
        /* Height */
        y = CosI * RadiusInner;
        
        for (j = 0; j <= DegX; ++j)
        {
            const f32 SinJ = math::Sin(static_cast<f32>(j*FinalDetail));
            const f32 CosJ = math::Cos(static_cast<f32>(j*FinalDetail));
            
            /* Coordination */
            x = SinJ*RadiusOuter - SinJ * SinI * RadiusInner;
            z = CosJ*RadiusOuter - CosJ * SinI * RadiusInner;
            
            /* UV-Mapping */
            u = static_cast<f32>(j) / DegX;
            v = static_cast<f32>(i) / DegY;
            
            /* Add vertex */
            Surface.addVertex(dim::vector3df(x, y, z), dim::point2df(u, v));
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
            
            Surface.addTriangle(v2, v1, v0);
            Surface.addTriangle(v1, v2, v3);
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
            
            Surface.addTriangle(v0, v1, v2);
            Surface.addTriangle(v3, v2, v1);
        }
    }
    
    finalizeGouraud(Surface);
    
    #else
    
    warning3DModelCompilation("Torus");
    
    #endif
}

SP_EXPORT void createTorusknot(
    video::MeshBuffer &Surface, f32 RadiusOuter, f32 RadiusInner, s32 Segments)
{
    #ifdef SP_COMPILE_WITH_PRIMITIVE_TORUSKNOT
    
    Surface.clearVertices();
    
    /* Temporary variables */
    s32 i, j;
    s32 Turns = 7, Slices = 16, Stacks = 256;
    f32 t;
    
    u32 v0, v1, v2, v3;
    
    boost::shared_array<dim::vector3df> RingCenters(new dim::vector3df[Stacks]);
    
    #if 1
    const f32 p = 5.0f;//, q = 8.0f;
    f32 deg = 0.0f;
    #endif
    
    /* Loop for each ring center */
    for (i = 0; i < Stacks; ++i, deg += 2*p*math::PI/Stacks)
    {
        t = math::PI*2*i / Stacks;
        
        #if 0
        RingCenters[i].X = ( 2 + cos(q * deg / p) ) * cos(deg);
        RingCenters[i].Y = sin(q * deg / p);
        RingCenters[i].Z = ( 2 + cos(q * deg / p) ) * sin(deg);
        #else
        RingCenters[i].X = ( 1.0f + RadiusOuter*cos(t*Turns) )*cos(t*2);
        RingCenters[i].Y = ( 1.0f + RadiusOuter*cos(t*Turns) )*sin(t*Turns)*RadiusOuter;
        RingCenters[i].Z = ( 1.0f + RadiusOuter*cos(t*Turns) )*sin(t*2);
        #endif
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
            tmp2.setLength(RadiusInner);
            
            /* Add the computed vertex */
            Surface.addVertex(
                RingCenters[i] + tmp2.getRotatedAxis(360.0f*j / Slices, Tangent),
                dim::point2df(static_cast<f32>(i)/5, static_cast<f32>(j)/Slices*3.0f)
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
            addFace(Surface, v0, v1, v2);
            addFace(Surface, v3, v2, v1);
        }
    }
    
    finalizeGouraud(Surface);
    
    #else
    
    warning3DModelCompilation("Torus");
    
    #endif
}

SP_EXPORT void createSpiral(
    video::MeshBuffer &Surface, f32 RadiusOuter, f32 RadiusInner, f32 TwirlDegree, f32 TwirlDistance, s32 Segments, bool HasCap)
{
    #ifdef SP_COMPILE_WITH_PRIMITIVE_SPIRAL
    
    Surface.clearVertices();
    
    /* Temporary variables */
    s32 i, j;
    f32 x, y, z;
    
    const s32 Detail = math::MinMax(Segments, 2, 180) * 2;
    
    const f32 FinalDetail = 360.0f/Detail;
    const f32 Height = static_cast<f32>(TwirlDegree*TwirlDistance) / 360 / 2;
    const s32 LengthDetail = static_cast<s32>(static_cast<f32>(Detail*TwirlDegree)/360) + 1;
    
    u32 v0, v1, v2, v3;
    
    /* === Create vertices === */
    
    /* Body */
    for (i = 0; i < LengthDetail; ++i)
    {
        const f32 SinI = math::Sin(static_cast<f32>(i*FinalDetail));
        const f32 CosI = math::Cos(static_cast<f32>(i*FinalDetail));
        
        for (j = 0; j <= Detail; ++j)
        {
            const f32 SinJ = math::Sin(static_cast<f32>(j*FinalDetail));
            const f32 CosJ = math::Cos(static_cast<f32>(j*FinalDetail));
            
            x = SinI * (RadiusOuter + CosJ*RadiusInner);
            y = static_cast<f32>(i*FinalDetail*TwirlDistance) / 360 + SinJ*RadiusInner - Height;
            z = CosI * (RadiusOuter + CosJ*RadiusInner);
            
            Surface.addVertex(
                dim::vector3df(x, y, z),
                dim::point2df(static_cast<f32>(i*FinalDetail*3)/360, 0.5f + SinJ*0.5f)
            );
        }
    }
    
    if (HasCap)
    {
        /* Bottom */
        for (j = 0; j <= Detail; ++j)
        {
            const f32 SinJ = math::Sin(static_cast<f32>(j*FinalDetail));
            const f32 CosJ = math::Cos(static_cast<f32>(j*FinalDetail));
            
            y = SinJ*RadiusInner - Height;
            z = (RadiusOuter + CosJ*RadiusInner);
            
            Surface.addVertex(
                dim::vector3df(0.0f, y, z),
                dim::point2df(
                    0.5f + math::Sin(static_cast<f32>(j*FinalDetail - 90))*0.5f,
                    0.5f - math::Cos(static_cast<f32>(j*FinalDetail - 90))*0.5f
                )
            );
        }
        
        /* Top */
        --i;
        
        const f32 SinI = math::Sin(static_cast<f32>(i*FinalDetail));
        const f32 CosI = math::Cos(static_cast<f32>(i*FinalDetail));
        
        for (j = 0, i = LengthDetail - 1; j <= Detail; ++j)
        {
            const f32 SinJ = math::Sin(static_cast<f32>(j*FinalDetail));
            const f32 CosJ = math::Cos(static_cast<f32>(j*FinalDetail));
            
            x = SinI * (RadiusOuter + CosJ*RadiusInner);
            y = static_cast<f32>(i*FinalDetail*TwirlDistance) / 360 + SinJ*RadiusInner - Height;
            z = CosI * (RadiusOuter + CosJ*RadiusInner);
            
            Surface.addVertex(
                dim::vector3df(x, y, z),
                dim::point2df(
                    0.5f + math::Sin(static_cast<f32>(j*FinalDetail + 90))*0.5f,
                    0.5f + math::Cos(static_cast<f32>(j*FinalDetail + 90))*0.5f
                )
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
            
            Surface.addTriangle(v0, v1, v2);
            Surface.addTriangle(v0, v2, v3);
        }
    }
    
    if (HasCap)
    {
        /* Bottom */
        v0 = LengthDetail*(Detail+1);
        for (j = 1; j < Detail - 1; ++j)
        {
            v1 = LengthDetail*(Detail+1)+j;
            v2 = LengthDetail*(Detail+1)+j+1;
            
            Surface.addTriangle(v0, v1, v2);
        }
        
        /* Top */
        v0 = (LengthDetail+1)*(Detail+1);
        for (j = 1; j < Detail - 1; ++j)
        {
            v1 = (LengthDetail+1)*(Detail+1)+j;
            v2 = (LengthDetail+1)*(Detail+1)+j+1;
            
            Surface.addTriangle(v2, v1, v0);
        }
    }
    
    finalizeGouraud(Surface);
    
    #else
    
    warning3DModelCompilation("Spiral");
    
    #endif
}

SP_EXPORT void createPipe(
    video::MeshBuffer &Surface, f32 RadiusOuter, f32 RadiusInner, f32 Height, s32 Segments, bool HasCap)
{
    #ifdef SP_COMPILE_WITH_PRIMITIVE_PIPE
    
    Surface.clearVertices();
    
    /* Temporary variables */
    const s32 Detail    = math::MinMax(Segments, 3, 360);
    
    const f32 c = 360.0f / Detail;
    const f32 HalfHeight = Height * 0.5f;
    
    for (f32 i = 0; i < 360; i += c)
    {
        const f32 SinI  = math::Sin(static_cast<f32>(i  ));
        const f32 CosI  = math::Cos(static_cast<f32>(i  ));
        const f32 SinIC = math::Sin(static_cast<f32>(i+c));
        const f32 CosIC = math::Cos(static_cast<f32>(i+c));
        
        /* Body outside */
        Surface.addVertex(dim::vector3df(SinI *RadiusOuter,  HalfHeight, CosI *RadiusOuter), dim::point2df((360-i  )/360*3, 0));
        Surface.addVertex(dim::vector3df(SinIC*RadiusOuter,  HalfHeight, CosIC*RadiusOuter), dim::point2df((360-i-c)/360*3, 0));
        Surface.addVertex(dim::vector3df(SinIC*RadiusOuter, -HalfHeight, CosIC*RadiusOuter), dim::point2df((360-i-c)/360*3, 1));
        Surface.addVertex(dim::vector3df(SinI *RadiusOuter, -HalfHeight, CosI *RadiusOuter), dim::point2df((360-i  )/360*3, 1));
        Surface.addTriangle(2, 1, 0); Surface.addTriangle(3, 2, 0);
        Surface.addIndexOffset(4);
        
        /* Body inside */
        Surface.addVertex(dim::vector3df(SinI *RadiusInner,  HalfHeight, CosI *RadiusInner), dim::point2df((360-i  )/360*3, 0));
        Surface.addVertex(dim::vector3df(SinIC*RadiusInner,  HalfHeight, CosIC*RadiusInner), dim::point2df((360-i-c)/360*3, 0));
        Surface.addVertex(dim::vector3df(SinIC*RadiusInner, -HalfHeight, CosIC*RadiusInner), dim::point2df((360-i-c)/360*3, 1));
        Surface.addVertex(dim::vector3df(SinI *RadiusInner, -HalfHeight, CosI *RadiusInner), dim::point2df((360-i  )/360*3, 1));
        Surface.addTriangle(0, 1, 2); Surface.addTriangle(0, 2, 3);
        Surface.addIndexOffset(4);
        
        if (HasCap)
        {
            /* Cap of top */
            Surface.addVertex(dim::vector3df(SinI *RadiusOuter,  HalfHeight, CosI *RadiusOuter), dim::point2df(0.5f+SinI *RadiusOuter, 0.5f-CosI *RadiusOuter));
            Surface.addVertex(dim::vector3df(SinIC*RadiusOuter,  HalfHeight, CosIC*RadiusOuter), dim::point2df(0.5f+SinIC*RadiusOuter, 0.5f-CosIC*RadiusOuter));
            Surface.addVertex(dim::vector3df(SinIC*RadiusInner,  HalfHeight, CosIC*RadiusInner), dim::point2df(0.5f+SinIC*RadiusInner, 0.5f-CosIC*RadiusInner));
            Surface.addVertex(dim::vector3df(SinI *RadiusInner,  HalfHeight, CosI *RadiusInner), dim::point2df(0.5f+SinI *RadiusInner, 0.5f-CosI *RadiusInner));
            Surface.addTriangle(0, 1, 2); Surface.addTriangle(0, 2, 3);
            Surface.addIndexOffset(4);
            
            /* Cap bottom */
            Surface.addVertex(dim::vector3df(SinI *RadiusOuter, -HalfHeight, CosI *RadiusOuter), dim::point2df(0.5f+SinI *RadiusOuter, 0.5f+CosI *RadiusOuter));
            Surface.addVertex(dim::vector3df(SinIC*RadiusOuter, -HalfHeight, CosIC*RadiusOuter), dim::point2df(0.5f+SinIC*RadiusOuter, 0.5f+CosIC*RadiusOuter));
            Surface.addVertex(dim::vector3df(SinIC*RadiusInner, -HalfHeight, CosIC*RadiusInner), dim::point2df(0.5f+SinIC*RadiusInner, 0.5f+CosIC*RadiusInner));
            Surface.addVertex(dim::vector3df(SinI *RadiusInner, -HalfHeight, CosI *RadiusInner), dim::point2df(0.5f+SinI *RadiusInner, 0.5f+CosI *RadiusInner));
            Surface.addTriangle(2, 1, 0); Surface.addTriangle(3, 2, 0);
            Surface.addIndexOffset(4);
        }
    }
    
    finalizeGouraud(Surface);
    
    #else
    
    warning3DModelCompilation("Pipe");
    
    #endif
}

SP_EXPORT void createPlane(video::MeshBuffer &Surface, s32 SegmentsVert, s32 SegmentsHorz)
{
    #ifdef SP_COMPILE_WITH_PRIMITIVE_PLANE
    
    Surface.clearVertices();
    
    /* Create the quad face */
    addQuadFace(
        Surface, SegmentsVert, SegmentsHorz,
        dim::vector3df(-0.5f, 0.0f,  0.5f), dim::point2df(0, 0),
        dim::vector3df( 0.5f, 0.0f, -0.5f), dim::point2df(1, 1),
        dim::vector3df(1, 0, 0), dim::vector3df(0, 0, 1)
    );
    
    finalizeFlat(Surface);
    
    #else
    
    warning3DModelCompilation("Plane");
    
    #endif
}

SP_EXPORT void createDisk(
    video::MeshBuffer &Surface, f32 RadiusOuter, f32 RadiusInner, s32 Segments, bool HasHole)
{
    #ifdef SP_COMPILE_WITH_PRIMITIVE_DISK
    
    Surface.clearVertices();
    
    /* Temporary variables */
    const s32 Detail = math::MinMax(Segments, 3, 360);
    
    const f32 size = 360.0f / Detail;
    
    f32 x, y;
    
    if (!HasHole)
    {
        Surface.addVertex(dim::vector3df(0.0f, 0.0f, 0.5f), dim::point2df(0.5f, 0.0f));
        
        x = math::Sin(size)*0.5f;
        y = math::Cos(size)*0.5f;
        Surface.addVertex(dim::vector3df(x, 0.0f, y), dim::point2df(0.5f+x, 0.5f-y));
        
        for (s32 i = 2; i < Detail; ++i)
        {
            x = math::Sin(i*size)*0.5f;
            y = math::Cos(i*size)*0.5f;
            Surface.addVertex(dim::vector3df(x, 0.0f, y), dim::point2df(0.5f+x, 0.5f-y));
            Surface.addTriangle(0, i-1, i);
        }
    }
    else
    {
        for (s32 i = 0; i < Detail; ++i)
        {
            /* Vertices - outside */
            x = math::Sin(i*size)*RadiusOuter;
            y = math::Cos(i*size)*RadiusOuter;
            Surface.addVertex(dim::vector3df(x, 0.0f, y), dim::point2df(0.5f+x, 0.5f-y));
            
            x = math::Sin((i+1)*size)*RadiusOuter;
            y = math::Cos((i+1)*size)*RadiusOuter;
            Surface.addVertex(dim::vector3df(x, 0.0f, y), dim::point2df(0.5f+x, 0.5f-y));
            
            /* Vertices - inside */
            x = math::Sin((i+1)*size)*RadiusInner;
            y = math::Cos((i+1)*size)*RadiusInner;
            Surface.addVertex(dim::vector3df(x, 0.0f, y), dim::point2df(0.5f+x, 0.5f-y));
            
            x = math::Sin(i*size)*RadiusInner;
            y = math::Cos(i*size)*RadiusInner;
            Surface.addVertex(dim::vector3df(x, 0.0f, y), dim::point2df(0.5f+x, 0.5f-y));
            
            /* Triangles */
            Surface.addTriangle(0, 1, 2);
            Surface.addTriangle(0, 2, 3);
            Surface.addIndexOffset(4);
        }
    }
    
    finalizeFlat(Surface);
    
    #else
    
    warning3DModelCompilation("Disk");
    
    #endif
}

SP_EXPORT void createCuboctahedron(video::MeshBuffer &Surface)
{
    #ifdef SP_COMPILE_WITH_PRIMITIVE_CUBOCTAHEDRON
    
    Surface.clearVertices();
    
    /* === Quadrangles === */
    
    /* Back */
    Surface.addVertex(dim::vector3df( 0.0f,  0.5f,  0.5f), dim::point2df(0.5, 0.0));
    Surface.addVertex(dim::vector3df( 0.5f,  0.0f,  0.5f), dim::point2df(1.0, 0.5));
    Surface.addVertex(dim::vector3df( 0.0f, -0.5f,  0.5f), dim::point2df(0.5, 1.0));
    Surface.addVertex(dim::vector3df(-0.5f,  0.0f,  0.5f), dim::point2df(0.0, 0.5));
    Surface.addTriangle(2, 1, 0); Surface.addTriangle(3, 2, 0);
    Surface.addIndexOffset(4);
    
    /* Front */
    Surface.addVertex(dim::vector3df( 0.0f,  0.5f, -0.5f), dim::point2df(0.5, 0.0));
    Surface.addVertex(dim::vector3df( 0.5f,  0.0f, -0.5f), dim::point2df(1.0, 0.5));
    Surface.addVertex(dim::vector3df( 0.0f, -0.5f, -0.5f), dim::point2df(0.5, 1.0));
    Surface.addVertex(dim::vector3df(-0.5f,  0.0f, -0.5f), dim::point2df(0.0, 0.5));
    Surface.addTriangle(0, 1, 2); Surface.addTriangle(0, 2, 3);
    Surface.addIndexOffset(4);
    
    /* Top */
    Surface.addVertex(dim::vector3df( 0.0f,  0.5f,  0.5f), dim::point2df(0.5, 0.0));
    Surface.addVertex(dim::vector3df( 0.5f,  0.5f,  0.0f), dim::point2df(1.0, 0.5));
    Surface.addVertex(dim::vector3df( 0.0f,  0.5f, -0.5f), dim::point2df(0.5, 1.0));
    Surface.addVertex(dim::vector3df(-0.5f,  0.5f,  0.0f), dim::point2df(0.0, 0.5));
    Surface.addTriangle(0, 1, 2); Surface.addTriangle(0, 2, 3);
    Surface.addIndexOffset(4);
    
    /* Bottom */
    Surface.addVertex(dim::vector3df( 0.0f, -0.5f,  0.5f), dim::point2df(0.5, 0.0));
    Surface.addVertex(dim::vector3df( 0.5f, -0.5f,  0.0f), dim::point2df(1.0, 0.5));
    Surface.addVertex(dim::vector3df( 0.0f, -0.5f, -0.5f), dim::point2df(0.5, 1.0));
    Surface.addVertex(dim::vector3df(-0.5f, -0.5f,  0.0f), dim::point2df(0.0, 0.5));
    Surface.addTriangle(2, 1, 0); Surface.addTriangle(3, 2, 0);
    Surface.addIndexOffset(4);
    
    /* Left */
    Surface.addVertex(dim::vector3df(-0.5f,  0.5f,  0.0f), dim::point2df(0.5, 0.0));
    Surface.addVertex(dim::vector3df(-0.5f,  0.0f, -0.5f), dim::point2df(1.0, 0.5));
    Surface.addVertex(dim::vector3df(-0.5f, -0.5f,  0.0f), dim::point2df(0.5, 1.0));
    Surface.addVertex(dim::vector3df(-0.5f,  0.0f,  0.5f), dim::point2df(0.0, 0.5));
    Surface.addTriangle(0, 1, 2); Surface.addTriangle(0, 2, 3);
    Surface.addIndexOffset(4);
    
    /* Right */
    Surface.addVertex(dim::vector3df( 0.5f,  0.5f,  0.0f), dim::point2df(0.5, 0.0));
    Surface.addVertex(dim::vector3df( 0.5f,  0.0f, -0.5f), dim::point2df(1.0, 0.5));
    Surface.addVertex(dim::vector3df( 0.5f, -0.5f,  0.0f), dim::point2df(0.5, 1.0));
    Surface.addVertex(dim::vector3df( 0.5f,  0.0f,  0.5f), dim::point2df(0.0, 0.5));
    Surface.addTriangle(2, 1, 0); Surface.addTriangle(3, 2, 0);
    Surface.addIndexOffset(4);
    
    /* === Trianlges - top (front-right, front-left, back-right, back-left) === */
    
    Surface.addVertex(dim::vector3df( 0.0f,  0.5f, -0.5f), dim::point2df(0.5, 0.0));
    Surface.addVertex(dim::vector3df( 0.5f,  0.5f,  0.0f), dim::point2df(1.0, 0.0));
    Surface.addVertex(dim::vector3df( 0.5f,  0.0f, -0.5f), dim::point2df(1.0, 0.5));
    Surface.addTriangle(0, 1, 2); Surface.addIndexOffset(3);
    
    Surface.addVertex(dim::vector3df( 0.0f,  0.5f, -0.5f), dim::point2df(0.5, 0.0));
    Surface.addVertex(dim::vector3df(-0.5f,  0.5f,  0.0f), dim::point2df(0.0, 0.0));
    Surface.addVertex(dim::vector3df(-0.5f,  0.0f, -0.5f), dim::point2df(0.0, 0.5));
    Surface.addTriangle(2, 1, 0); Surface.addIndexOffset(3);
    
    Surface.addVertex(dim::vector3df( 0.0f,  0.5f,  0.5f), dim::point2df(0.5, 0.0));
    Surface.addVertex(dim::vector3df( 0.5f,  0.5f,  0.0f), dim::point2df(1.0, 0.0));
    Surface.addVertex(dim::vector3df( 0.5f,  0.0f,  0.5f), dim::point2df(1.0, 0.5));
    Surface.addTriangle(2, 1, 0); Surface.addIndexOffset(3);
    
    Surface.addVertex(dim::vector3df( 0.0f,  0.5f,  0.5f), dim::point2df(0.5, 0.0));
    Surface.addVertex(dim::vector3df(-0.5f,  0.5f,  0.0f), dim::point2df(0.0, 0.0));
    Surface.addVertex(dim::vector3df(-0.5f,  0.0f,  0.5f), dim::point2df(0.0, 0.5));
    Surface.addTriangle(0, 1, 2); Surface.addIndexOffset(3);
    
    /* === Trianlges - bottom (front-right, front-left, back-right, back-left) === */
    
    Surface.addVertex(dim::vector3df( 0.0f, -0.5f, -0.5f), dim::point2df(0.5, 1.0));
    Surface.addVertex(dim::vector3df( 0.5f, -0.5f,  0.0f), dim::point2df(1.0, 1.0));
    Surface.addVertex(dim::vector3df( 0.5f,  0.0f, -0.5f), dim::point2df(1.0, 0.5));
    Surface.addTriangle(2, 1, 0); Surface.addIndexOffset(3);
    
    Surface.addVertex(dim::vector3df( 0.0f, -0.5f, -0.5f), dim::point2df(0.5, 1.0));
    Surface.addVertex(dim::vector3df(-0.5f, -0.5f,  0.0f), dim::point2df(0.0, 1.0));
    Surface.addVertex(dim::vector3df(-0.5f,  0.0f, -0.5f), dim::point2df(0.0, 0.5));
    Surface.addTriangle(0, 1, 2); Surface.addIndexOffset(3);
    
    Surface.addVertex(dim::vector3df( 0.0f, -0.5f,  0.5f), dim::point2df(0.5, 1.0));
    Surface.addVertex(dim::vector3df( 0.5f, -0.5f,  0.0f), dim::point2df(1.0, 1.0));
    Surface.addVertex(dim::vector3df( 0.5f,  0.0f,  0.5f), dim::point2df(1.0, 0.5));
    Surface.addTriangle(0, 1, 2); Surface.addIndexOffset(3);
    
    Surface.addVertex(dim::vector3df( 0.0f, -0.5f,  0.5f), dim::point2df(0.5, 1.0));
    Surface.addVertex(dim::vector3df(-0.5f, -0.5f,  0.0f), dim::point2df(0.0, 1.0));
    Surface.addVertex(dim::vector3df(-0.5f,  0.0f,  0.5f), dim::point2df(0.0, 0.5));
    Surface.addTriangle(2, 1, 0); Surface.addIndexOffset(3);
    
    finalizeFlat(Surface);
    
    #else
    
    warning3DModelCompilation("Cuboctahedron");
    
    #endif
}

SP_EXPORT void createTetrahedron(video::MeshBuffer &Surface)
{
    #ifdef SP_COMPILE_WITH_PRIMITIVE_TETRAHEDRON
    
    Surface.clearVertices();
    
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
    addFace(Surface, Vertices[1], Vertices[0], Vertices[3], TexCoords[0], TexCoords[1], TexCoords[2]);
    addFace(Surface, Vertices[3], Vertices[0], Vertices[2], TexCoords[0], TexCoords[1], TexCoords[2]);
    addFace(Surface, Vertices[2], Vertices[0], Vertices[1], TexCoords[0], TexCoords[1], TexCoords[2]);
    addFace(Surface, Vertices[2], Vertices[1], Vertices[3], TexCoords[0], TexCoords[1], TexCoords[2]);
    
    finalizeFlat(Surface);
    
    #else
    
    warning3DModelCompilation("Tetrahedron");
    
    #endif
}

SP_EXPORT void createOctahedron(video::MeshBuffer &Surface)
{
    #ifdef SP_COMPILE_WITH_PRIMITIVE_OCTAHEDRON
    
    Surface.clearVertices();
    
    static const f32 size = 0.707106781f;
    
    /* Vertices */
    Surface.addVertex(dim::vector3df(-0.5f,  0.0f,  0.5f), dim::point2df(1.0f, 1.0f));
    Surface.addVertex(dim::vector3df( 0.5f,  0.0f,  0.5f), dim::point2df(0.0f, 1.0f));
    Surface.addVertex(dim::vector3df( 0.5f,  0.0f, -0.5f), dim::point2df(1.0f, 1.0f));
    Surface.addVertex(dim::vector3df(-0.5f,  0.0f, -0.5f), dim::point2df(0.0f, 1.0f));
    Surface.addVertex(dim::vector3df( 0.0f,  size,  0.0f), dim::point2df(0.5f, 0.0f));
    Surface.addVertex(dim::vector3df( 0.0f, -size,  0.0f), dim::point2df(0.5f, 0.0f));
    
    /* Triangles */
    Surface.addTriangle(3, 4, 2);
    Surface.addTriangle(3, 2, 5);
    Surface.addTriangle(0, 3, 5);
    Surface.addTriangle(0, 4, 3);
    Surface.addTriangle(1, 4, 0);
    Surface.addTriangle(1, 0, 5);
    Surface.addTriangle(2, 4, 1);
    Surface.addTriangle(2, 1, 5);
    
    finalizeFlat(Surface);
    
    #else
    
    warning3DModelCompilation("Octahedron");
    
    #endif
}

SP_EXPORT void createDodecahedron(video::MeshBuffer &Surface)
{
    #ifdef SP_COMPILE_WITH_PRIMITIVE_DODECAHEDRON
    
    /* Temporary variables */
    const f32 Radius = 0.5f;
    
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
    addFace(Surface, Vertices, 0, 18, 5,  9,  8, 4);
    addFace(Surface, Vertices, 0, 16, 0,  8,  9, 1);
    addFace(Surface, Vertices, 0, 19, 7, 10, 11, 6);
    addFace(Surface, Vertices, 0, 17, 2, 11, 10, 3);
    addFace(Surface, Vertices, 0,  9, 5, 13, 12, 1);
    addFace(Surface, Vertices, 0, 11, 2, 12, 13, 6);
    addFace(Surface, Vertices, 0,  8, 0, 14, 15, 4);
    addFace(Surface, Vertices, 0, 10, 7, 15, 14, 3);
    addFace(Surface, Vertices, 0, 13, 5, 18, 19, 6);
    addFace(Surface, Vertices, 0, 15, 7, 19, 18, 4);
    addFace(Surface, Vertices, 0, 12, 2, 17, 16, 1);
    addFace(Surface, Vertices, 0, 14, 0, 16, 17, 3);
    
    finalizeFlat(Surface);
    
    #else
    
    warning3DModelCompilation("Dodecahedron");
    
    #endif
}

SP_EXPORT void createIcosahedron(video::MeshBuffer &Surface)
{
    #ifdef SP_COMPILE_WITH_PRIMITIVE_ICOSAHEDRON
    
    createIcoSphere(Surface, 0.5f, 1);
    
    #else
    
    warning3DModelCompilation("Icosahedron");
    
    #endif
}

SP_EXPORT void createTeapot(video::MeshBuffer &Surface)
{
    #ifdef SP_COMPILE_WITH_PRIMITIVE_TEAPOT
    
    Surface.clearVertices();
    
    #   ifdef SP_COMPILE_WITH_PRIMITIVE_TEAPOT_DYNAMIC
    
    const s32 Segments = math::MinMax(BuildConstruct_.SegmentsVert, 1, 100);
    
    dim::vector3df Patches[4][4];
    
    for (s32 i = 0; i < 32; ++i)
    {
        for (s32 j = 0, k; j < 4; ++j)
            for (k = 0; k < 4; ++k)
                Patches[j][k] = __spTeapotPatchesData[i][j][k] * 0.02 * BuildConstruct_.RadiusInner;
        
        createBezierPatchFace(Mesh_, 0, Patches, Segments);
    }
    
    Mesh_->meshTurn(dim::vector3df(-90, 0, 0));
    
    #   else
    
    const s32 VerticesCount = 1178;
    const s32 TriangleCount = 2256;
    
    /* Vertices */
    for (s32 i = 0; i < VerticesCount; ++i)
    {
        Surface.addVertex(
            dim::vector3df(__spTeapotVertices[i*3+0], __spTeapotVertices[i*3+1], __spTeapotVertices[i*3+2]),
            dim::point2df(__spTeapotVertices[i*3+0], __spTeapotVertices[i*3+1])
        );
    }
    
    /* Triangles */
    for (s32 i = 0; i < TriangleCount; ++i)
        Surface.addTriangle(__spTeapotIndices[i*3+0], __spTeapotIndices[i*3+1], __spTeapotIndices[i*3+2]);
    
    #   endif
    
    finalizeGouraud(Surface);
    
    #else
    
    warning3DModelCompilation("Teapot");
    
    #endif
}

SP_EXPORT void createWireCube(scene::Mesh &MeshObj, f32 Radius)
{
    #ifdef SP_COMPILE_WITH_PRIMITIVE_WIRE_CUBE
    
    MeshObj.deleteMeshBuffers();
    
    /* Configure wire mesh */
    video::MaterialStates* Material = MeshObj.getMaterial();
    
    Material->setLighting(false);
    Material->setDiffuseColor(0);
    Material->setAmbientColor(255);
    
    video::MeshBuffer* Surface = MeshObj.createMeshBuffer();
    
    Surface->setPrimitiveType(video::PRIMITIVE_LINES);
    
    /* Build wire mesh */
    Surface->addVertices(8);
    
    for (s32 i = 0; i < 8; ++i)
    {
        Surface->setVertexCoord(i, dim::vector3df(
            i % 8 >= 4 ? Radius : -Radius,
            i % 4 >= 2 ? Radius : -Radius,
            i % 2 >= 1 ? Radius : -Radius
        ));
        Surface->setVertexColor(i, video::color(255));
    }
    
    for (s32 i = 0; i < 4; ++i)
    {
        Surface->addPrimitiveIndex(i*2); Surface->addPrimitiveIndex(i*2+1);
        Surface->addPrimitiveIndex(i  ); Surface->addPrimitiveIndex(i  +4);
    }
    
    Surface->addPrimitiveIndex(0); Surface->addPrimitiveIndex(2);
    Surface->addPrimitiveIndex(1); Surface->addPrimitiveIndex(3);
    Surface->addPrimitiveIndex(4); Surface->addPrimitiveIndex(6);
    Surface->addPrimitiveIndex(5); Surface->addPrimitiveIndex(7);
    
    MeshObj.updateMeshBuffer();
    
    #else
    
    warning3DModelCompilation("Wire Cube");
    
    #endif
}

SP_EXPORT void createSkyBox(scene::Mesh &MeshObj, video::Texture* (&TextureList)[6], f32 Radius)
{
    MeshObj.deleteMeshBuffers();
    
    const f32 UVMap1 = 0.0f;
    const f32 UVMap2 = 1.0f;
    
    video::MeshBuffer* Surface = 0;
    
    // Front
    Surface = MeshObj.createMeshBuffer(SceneManager::getDefaultVertexFormat(), SceneManager::getDefaultIndexFormat());
    Surface->addTexture(TextureList[0]);
    
    addVertex(*Surface, -Radius,  Radius, -Radius, UVMap2, UVMap1);
    addVertex(*Surface,  Radius,  Radius, -Radius, UVMap1, UVMap1);
    addVertex(*Surface,  Radius, -Radius, -Radius, UVMap1, UVMap2);
    addVertex(*Surface, -Radius, -Radius, -Radius, UVMap2, UVMap2);
    addFace(*Surface, 2, 1, 0); addFace(*Surface, 3, 2, 0);
    
    // Back
    Surface = MeshObj.createMeshBuffer(SceneManager::getDefaultVertexFormat(), SceneManager::getDefaultIndexFormat());
    Surface->addTexture(TextureList[1]);
    
    addVertex(*Surface, -Radius,  Radius,  Radius, UVMap1, UVMap1);
    addVertex(*Surface,  Radius,  Radius,  Radius, UVMap2, UVMap1);
    addVertex(*Surface,  Radius, -Radius,  Radius, UVMap2, UVMap2);
    addVertex(*Surface, -Radius, -Radius,  Radius, UVMap1, UVMap2);
    addFace(*Surface, 0, 1, 2); addFace(*Surface, 0, 2, 3);
    
    // Top
    Surface = MeshObj.createMeshBuffer(SceneManager::getDefaultVertexFormat(), SceneManager::getDefaultIndexFormat());
    Surface->addTexture(TextureList[2]);
    
    addVertex(*Surface, -Radius,  Radius,  Radius, UVMap2, UVMap2);
    addVertex(*Surface,  Radius,  Radius,  Radius, UVMap2, UVMap1);
    addVertex(*Surface,  Radius,  Radius, -Radius, UVMap1, UVMap1);
    addVertex(*Surface, -Radius,  Radius, -Radius, UVMap1, UVMap2);
    addFace(*Surface, 2, 1, 0); addFace(*Surface, 3, 2, 0);
    
    // Bottom
    Surface = MeshObj.createMeshBuffer(SceneManager::getDefaultVertexFormat(), SceneManager::getDefaultIndexFormat());
    Surface->addTexture(TextureList[3]);
    
    addVertex(*Surface, -Radius, -Radius,  Radius, UVMap2, UVMap1);
    addVertex(*Surface,  Radius, -Radius,  Radius, UVMap2, UVMap2);
    addVertex(*Surface,  Radius, -Radius, -Radius, UVMap1, UVMap2);
    addVertex(*Surface, -Radius, -Radius, -Radius, UVMap1, UVMap1);
    addFace(*Surface, 0, 1, 2); addFace(*Surface, 0, 2, 3);
    
    // Right
    Surface = MeshObj.createMeshBuffer(SceneManager::getDefaultVertexFormat(), SceneManager::getDefaultIndexFormat());
    Surface->addTexture(TextureList[4]);
    
    addVertex(*Surface,  Radius,  Radius,  Radius, UVMap1, UVMap1);
    addVertex(*Surface,  Radius,  Radius, -Radius, UVMap2, UVMap1);
    addVertex(*Surface,  Radius, -Radius, -Radius, UVMap2, UVMap2);
    addVertex(*Surface,  Radius, -Radius,  Radius, UVMap1, UVMap2);
    addFace(*Surface, 0, 1, 2); addFace(*Surface, 0, 2, 3);
    
    // Left
    Surface = MeshObj.createMeshBuffer(SceneManager::getDefaultVertexFormat(), SceneManager::getDefaultIndexFormat());
    Surface->addTexture(TextureList[5]);
    
    addVertex(*Surface, -Radius,  Radius,  Radius, UVMap2, UVMap1);
    addVertex(*Surface, -Radius,  Radius, -Radius, UVMap1, UVMap1);
    addVertex(*Surface, -Radius, -Radius, -Radius, UVMap1, UVMap2);
    addVertex(*Surface, -Radius, -Radius,  Radius, UVMap2, UVMap2);
    addFace(*Surface, 2, 1, 0); addFace(*Surface, 3, 2, 0);
    
    /* Setup mesh material */
    MeshObj.setOrder(ORDER_BACKGROUND);
    MeshObj.getMaterial()->setDepthBuffer(false);
    MeshObj.getMaterial()->setLighting(false);
    
    MeshObj.updateMeshBuffer();
}

//SP_EXPORT void createBatchingMesh(BatchingMesh &Object, const std::list<Mesh*> &MeshList)
//SP_EXPORT void createBatchingMesh(BatchingMesh &Object, const std::list<video::MeshBuffer*> &SurfaceList)

} // /namespace MeshGenerator


#if 1 //!!!

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

void BasicMeshGenerator::createSuperShape(Mesh* Object, const f32 ValueList[12], const s32 Segments)
{
    Mesh_ = Object;
    
    #ifdef SP_COMPILE_WITH_PRIMITIVE_SUPERSHAPE
    Surface_ = Mesh_->createMeshBuffer(SceneManager::getDefaultVertexFormat(), SceneManager::getDefaultIndexFormat());
    
    createSuperShapeSurface(ValueList, Segments);
    
    Mesh_->updateNormals();
    #endif
}

Mesh* BasicMeshGenerator::createBezierPatch(
    const dim::vector3df AnchorPoints[4][4], const s32 Segments, bool isFrontFace)
{
    Mesh_ = new Mesh();
    Surface_ = Mesh_->createMeshBuffer(SceneManager::getDefaultVertexFormat(), SceneManager::getDefaultIndexFormat());
    
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
        
        for (u32 i = 0; i < VertexCoords.size() - 2; ++i)
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
    
    Surface_ = Mesh_->createMeshBuffer(SceneManager::getDefaultVertexFormat(), SceneManager::getDefaultIndexFormat());
    
    /* Create all vertices */
    for (z = 0; z <= Segments; ++z)
    {
        for (x = 0; x <= Segments; ++x)
        {
            const dim::point2di TexelCoord(x*Width/Segments, z*Height/Segments);
            
            y = ImgBuffer->getPixelColor(TexelCoord).getBrightness<f32>() / 255;
            
            MeshGenerator::addVertex(
                *Surface_, size*x - 0.5f, y, -size*z + 0.5f, size*x, size*z
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
            MeshGenerator::addFace(*Surface_, v0, v1, v2, v3);
        }
    }
    
    Mesh_->updateIndexBuffer();
    Mesh_->updateNormals();
    
    return Mesh_;
}


/*
 * ========== Private: ==========
 */


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
    
    const f32 CosPhi = math::Cos(phi) * r2;
    
    /* Compute & create the model */
    for (j = 0; j < Segments; ++j)
    {
        theta = j*alpha - 180;
        
        r1 = computeSuperShapeNextFrame(ValueList[0], ValueList[1], ValueList[2], ValueList[3], ValueList[4], ValueList[5], theta);
        
        x = r1*math::Cos(theta)*CosPhi;
        y = r1*math::Sin(theta)*CosPhi;
        z = r2*math::Sin(phi);
        
        Surface_->addVertex(dim::vector3df(x, y, z), dim::point2df(x, y));
    }
    
    for (i = 1; i <= Segments/2; ++i)
    {
        phi = i*alpha - 90;
        
        const f32 CosPhi = math::Cos(phi) * r2 * r1;
        
        r2 = computeSuperShapeNextFrame(ValueList[6], ValueList[7], ValueList[8], ValueList[9], ValueList[10], ValueList[11], phi);
        
        for (j = 0; j < Segments; ++j)
        {
            theta = j*alpha - 180;
            
            r1 = computeSuperShapeNextFrame(ValueList[0], ValueList[1], ValueList[2], ValueList[3], ValueList[4], ValueList[5], theta);
            
            x = math::Cos(theta)*CosPhi;
            y = math::Sin(theta)*CosPhi;
            z = r2*math::Sin(phi);
            
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
        pow( pow( math::Abs<f32>( math::Cos(m*phi/4)/a ), n2 ) +
        pow(      math::Abs<f32>( math::Sin(m*phi/4)/b ), n3 ), (1.0f/(-n1)) );
}

#endif

#endif //!!!


/* === Other modeling functions === */


#ifdef SP_COMPILE_WITH_PRIMITIVE_TEAPOT

#   ifndef SP_COMPILE_WITH_PRIMITIVE_TEAPOT_DYNAMIC

/*
 * ========== Teapot vertices: ==========
 */

const f32 __spTeapotVertices[] = {

     0.678873f,  0.330678f,  0.000000f,     0.654243f,  0.330678f,  0.187963f,     0.645254f,  0.358022f,  0.185461f,     0.669556f,  0.358022f,  0.000000f,
     0.646650f,  0.374428f,  0.185850f,     0.671003f,  0.374428f,  0.000000f,     0.655751f,  0.379897f,  0.188383f,     0.680435f,  0.379897f,  0.000000f,
     0.669877f,  0.374428f,  0.192314f,     0.695077f,  0.374428f,  0.000000f,     0.686348f,  0.358022f,  0.196898f,     0.712148f,  0.358022f,  0.000000f,
     0.702484f,  0.330678f,  0.201389f,     0.728873f,  0.330678f,  0.000000f,     0.584502f,  0.330678f,  0.355704f,     0.576441f,  0.358022f,  0.350969f,
     0.577693f,  0.374428f,  0.351704f,     0.585854f,  0.379897f,  0.356498f,     0.598522f,  0.374428f,  0.363938f,     0.613292f,  0.358022f,  0.372613f,
     0.627762f,  0.330678f,  0.381111f,     0.475873f,  0.330678f,  0.497000f,     0.469258f,  0.358022f,  0.490385f,     0.470285f,  0.374428f,  0.491412f,
     0.476982f,  0.379897f,  0.498109f,     0.487377f,  0.374428f,  0.508505f,     0.499498f,  0.358022f,  0.520626f,     0.511373f,  0.330678f,  0.532500f,
     0.334576f,  0.330678f,  0.605630f,     0.329842f,  0.358022f,  0.597569f,     0.330577f,  0.374428f,  0.598820f,     0.335370f,  0.379897f,  0.606982f,
     0.342810f,  0.374428f,  0.619649f,     0.351485f,  0.358022f,  0.634419f,     0.359984f,  0.330678f,  0.648889f,     0.166836f,  0.330678f,  0.675370f,
     0.164334f,  0.358022f,  0.666381f,     0.164722f,  0.374428f,  0.667777f,     0.167255f,  0.379897f,  0.676878f,     0.171187f,  0.374428f,  0.691004f,
     0.175771f,  0.358022f,  0.707475f,     0.180262f,  0.330678f,  0.723611f,    -0.021127f,  0.330678f,  0.700000f,    -0.021127f,  0.358022f,  0.690683f,
    -0.021127f,  0.374428f,  0.692130f,    -0.021127f,  0.379897f,  0.701563f,    -0.021127f,  0.374428f,  0.716204f,    -0.021127f,  0.358022f,  0.733276f,
    -0.021127f,  0.330678f,  0.750000f,    -0.224715f,  0.330678f,  0.675370f,    -0.215631f,  0.358022f,  0.666381f,    -0.211606f,  0.374428f,  0.667777f,
    -0.211463f,  0.379897f,  0.676878f,    -0.214020f,  0.374428f,  0.691004f,    -0.218098f,  0.358022f,  0.707475f,    -0.222516f,  0.330678f,  0.723611f,
    -0.396831f,  0.330678f,  0.605630f,    -0.383671f,  0.358022f,  0.597569f,    -0.378758f,  0.374428f,  0.598820f,    -0.380125f,  0.379897f,  0.606982f,
    -0.385806f,  0.374428f,  0.619649f,    -0.393832f,  0.358022f,  0.634419f,    -0.402238f,  0.330678f,  0.648889f,    -0.535002f,  0.330678f,  0.497000f,
    -0.521278f,  0.358022f,  0.490385f,    -0.517539f,  0.374428f,  0.491412f,    -0.521346f,  0.379897f,  0.498109f,    -0.530257f,  0.374428f,  0.508505f,
    -0.541831f,  0.358022f,  0.520626f,    -0.553627f,  0.330678f,  0.532500f,    -0.636757f,  0.330678f,  0.355704f,    -0.624483f,  0.358022f,  0.350969f,
    -0.622910f,  0.374428f,  0.351704f,    -0.629359f,  0.379897f,  0.356498f,    -0.641146f,  0.374428f,  0.363938f,    -0.655593f,  0.358022f,  0.372613f,
    -0.670016f,  0.330678f,  0.381111f,    -0.699623f,  0.330678f,  0.187963f,    -0.689317f,  0.358022f,  0.185461f,    -0.689830f,  0.374428f,  0.185850f,
    -0.698396f,  0.379897f,  0.188382f,    -0.712247f,  0.374428f,  0.192314f,    -0.728617f,  0.358022f,  0.196898f,    -0.744738f,  0.330678f,  0.201389f,
    -0.721127f,  0.330678f,  0.000000f,    -0.711810f,  0.358022f,  0.000000f,    -0.713257f,  0.374428f,  0.000000f,    -0.722690f,  0.379897f,  0.000000f,
    -0.737331f,  0.374428f,  0.000000f,    -0.754403f,  0.358022f,  0.000000f,    -0.771127f,  0.330678f,  0.000000f,    -0.696498f,  0.330678f, -0.187963f,
    -0.687508f,  0.358022f, -0.185461f,    -0.688904f,  0.374428f, -0.185850f,    -0.698005f,  0.379897f, -0.188383f,    -0.712131f,  0.374428f, -0.192314f,
    -0.728602f,  0.358022f, -0.196898f,    -0.744738f,  0.330678f, -0.201389f,    -0.626757f,  0.330678f, -0.355704f,    -0.618696f,  0.358022f, -0.350969f,
    -0.619948f,  0.374428f, -0.351704f,    -0.628109f,  0.379897f, -0.356498f,    -0.640776f,  0.374428f, -0.363938f,    -0.655546f,  0.358022f, -0.372613f,
    -0.670016f,  0.330678f, -0.381111f,    -0.518127f,  0.330678f, -0.497000f,    -0.511512f,  0.358022f, -0.490385f,    -0.512539f,  0.374428f, -0.491412f,
    -0.519237f,  0.379897f, -0.498109f,    -0.529632f,  0.374428f, -0.508505f,    -0.541753f,  0.358022f, -0.520626f,    -0.553627f,  0.330678f, -0.532500f,
    -0.376831f,  0.330678f, -0.605630f,    -0.372096f,  0.358022f, -0.597569f,    -0.372832f,  0.374428f, -0.598820f,    -0.377625f,  0.379897f, -0.606982f,
    -0.385065f,  0.374428f, -0.619649f,    -0.393740f,  0.358022f, -0.634419f,    -0.402238f,  0.330678f, -0.648889f,    -0.209090f,  0.330678f, -0.675370f,
    -0.206588f,  0.358022f, -0.666381f,    -0.206977f,  0.374428f, -0.667777f,    -0.209510f,  0.379897f, -0.676878f,    -0.213441f,  0.374428f, -0.691004f,
    -0.218025f,  0.358022f, -0.707475f,    -0.222516f,  0.330678f, -0.723611f,    -0.021127f,  0.330678f, -0.700000f,    -0.021127f,  0.358022f, -0.690683f,
    -0.021127f,  0.374428f, -0.692130f,    -0.021127f,  0.379897f, -0.701563f,    -0.021127f,  0.374428f, -0.716204f,    -0.021127f,  0.358022f, -0.733276f,
    -0.021127f,  0.330678f, -0.750000f,     0.166836f,  0.330678f, -0.675370f,     0.164334f,  0.358022f, -0.666381f,     0.164722f,  0.374428f, -0.667777f,
     0.167255f,  0.379897f, -0.676878f,     0.171187f,  0.374428f, -0.691004f,     0.175771f,  0.358022f, -0.707475f,     0.180262f,  0.330678f, -0.723611f,
     0.334576f,  0.330678f, -0.605630f,     0.329842f,  0.358022f, -0.597569f,     0.330577f,  0.374428f, -0.598820f,     0.335370f,  0.379897f, -0.606982f,
     0.342810f,  0.374428f, -0.619649f,     0.351485f,  0.358022f, -0.634419f,     0.359984f,  0.330678f, -0.648889f,     0.475873f,  0.330678f, -0.497000f,
     0.469258f,  0.358022f, -0.490385f,     0.470285f,  0.374428f, -0.491412f,     0.476982f,  0.379897f, -0.498109f,     0.487377f,  0.374428f, -0.508505f,
     0.499498f,  0.358022f, -0.520626f,     0.511373f,  0.330678f, -0.532500f,     0.584502f,  0.330678f, -0.355704f,     0.576441f,  0.358022f, -0.350969f,
     0.577693f,  0.374428f, -0.351704f,     0.585854f,  0.379897f, -0.356498f,     0.598522f,  0.374428f, -0.363938f,     0.613292f,  0.358022f, -0.372613f,
     0.627762f,  0.330678f, -0.381111f,     0.654243f,  0.330678f, -0.187963f,     0.645254f,  0.358022f, -0.185461f,     0.646650f,  0.374428f, -0.185850f,
     0.655751f,  0.379897f, -0.188382f,     0.669877f,  0.374428f, -0.192314f,     0.686348f,  0.358022f, -0.196898f,     0.702484f,  0.330678f, -0.201389f,
     0.762227f,  0.199602f,  0.218016f,     0.790794f,  0.199602f,  0.000000f,     0.818619f,  0.069567f,  0.233711f,     0.849243f,  0.069567f,  0.000000f,
     0.868312f, -0.058384f,  0.247541f,     0.900748f, -0.058384f,  0.000000f,     0.907954f, -0.183211f,  0.258573f,     0.941836f, -0.183211f,  0.000000f,
     0.934196f, -0.303870f,  0.265877f,     0.969035f, -0.303870f,  0.000000f,     0.943688f, -0.419322f,  0.268519f,     0.978873f, -0.419322f,  0.000000f,
     0.681335f,  0.199602f,  0.412576f,     0.731904f,  0.069567f,  0.442277f,     0.776465f, -0.058384f,  0.468449f,     0.812014f, -0.183211f,  0.489328f,
     0.835546f, -0.303870f,  0.503149f,     0.844058f, -0.419322f,  0.508148f,     0.555337f,  0.199602f,  0.576464f,     0.596836f,  0.069567f,  0.617963f,
     0.633404f, -0.058384f,  0.654531f,     0.662577f, -0.183211f,  0.683704f,     0.681888f, -0.303870f,  0.703015f,     0.688873f, -0.419322f,  0.710000f,
     0.391449f,  0.199602f,  0.702462f,     0.421150f,  0.069567f,  0.753032f,     0.447322f, -0.058384f,  0.797593f,     0.468201f, -0.183211f,  0.833141f,
     0.482022f, -0.303870f,  0.856674f,     0.487021f, -0.419322f,  0.865185f,     0.196889f,  0.199602f,  0.783354f,     0.212583f,  0.069567f,  0.839746f,
     0.226413f, -0.058384f,  0.889439f,     0.237446f, -0.183211f,  0.929081f,     0.244750f, -0.303870f,  0.955323f,     0.247391f, -0.419322f,  0.964815f,
    -0.021127f,  0.199602f,  0.811921f,    -0.021127f,  0.069567f,  0.870370f,    -0.021127f, -0.058384f,  0.921875f,    -0.021127f, -0.183211f,  0.962963f,
    -0.021127f, -0.303870f,  0.990162f,    -0.021127f, -0.419322f,  1.000000f,    -0.239143f,  0.199602f,  0.783354f,    -0.254838f,  0.069567f,  0.839746f,
    -0.268668f, -0.058384f,  0.889439f,    -0.279701f, -0.183211f,  0.929081f,    -0.287004f, -0.303870f,  0.955323f,    -0.289646f, -0.419322f,  0.964815f,
    -0.433704f,  0.199602f,  0.702462f,    -0.463404f,  0.069567f,  0.753032f,    -0.489576f, -0.058384f,  0.797593f,    -0.510455f, -0.183211f,  0.833141f,
    -0.524276f, -0.303870f,  0.856674f,    -0.529275f, -0.419322f,  0.865185f,    -0.597591f,  0.199602f,  0.576464f,    -0.639090f,  0.069567f,  0.617963f,
    -0.675658f, -0.058384f,  0.654531f,    -0.704831f, -0.183211f,  0.683704f,    -0.724142f, -0.303870f,  0.703015f,    -0.731127f, -0.419322f,  0.710000f,
    -0.723589f,  0.199602f,  0.412576f,    -0.774159f,  0.069567f,  0.442277f,    -0.818720f, -0.058384f,  0.468449f,    -0.854269f, -0.183211f,  0.489328f,
    -0.877801f, -0.303870f,  0.503149f,    -0.886312f, -0.419322f,  0.508148f,    -0.804481f,  0.199602f,  0.218016f,    -0.860873f,  0.069567f,  0.233711f,
    -0.910566f, -0.058384f,  0.247540f,    -0.950208f, -0.183211f,  0.258573f,    -0.976450f, -0.303870f,  0.265877f,    -0.985942f, -0.419322f,  0.268518f,
    -0.833049f,  0.199602f,  0.000000f,    -0.891498f,  0.069567f,  0.000000f,    -0.943002f, -0.058384f,  0.000000f,    -0.984090f, -0.183211f,  0.000000f,
    -1.011290f, -0.303870f,  0.000000f,    -1.021130f, -0.419322f,  0.000000f,    -0.804481f,  0.199602f, -0.218016f,    -0.860873f,  0.069567f, -0.233711f,
    -0.910566f, -0.058384f, -0.247541f,    -0.950208f, -0.183211f, -0.258573f,    -0.976450f, -0.303870f, -0.265877f,    -0.985942f, -0.419322f, -0.268519f,
    -0.723589f,  0.199602f, -0.412576f,    -0.774159f,  0.069567f, -0.442277f,    -0.818720f, -0.058384f, -0.468449f,    -0.854269f, -0.183211f, -0.489328f,
    -0.877801f, -0.303870f, -0.503149f,    -0.886312f, -0.419322f, -0.508148f,    -0.597591f,  0.199602f, -0.576464f,    -0.639090f,  0.069567f, -0.617963f,
    -0.675658f, -0.058384f, -0.654531f,    -0.704831f, -0.183211f, -0.683704f,    -0.724142f, -0.303870f, -0.703015f,    -0.731127f, -0.419322f, -0.710000f,
    -0.433704f,  0.199602f, -0.702462f,    -0.463404f,  0.069567f, -0.753032f,    -0.489576f, -0.058384f, -0.797593f,    -0.510455f, -0.183211f, -0.833141f,
    -0.524276f, -0.303870f, -0.856674f,    -0.529275f, -0.419322f, -0.865185f,    -0.239143f,  0.199602f, -0.783354f,    -0.254838f,  0.069567f, -0.839746f,
    -0.268668f, -0.058384f, -0.889439f,    -0.279701f, -0.183211f, -0.929081f,    -0.287004f, -0.303870f, -0.955323f,    -0.289646f, -0.419322f, -0.964815f,
    -0.021127f,  0.199602f, -0.811921f,    -0.021127f,  0.069567f, -0.870370f,    -0.021127f, -0.058384f, -0.921875f,    -0.021127f, -0.183211f, -0.962963f,
    -0.021127f, -0.303870f, -0.990162f,    -0.021127f, -0.419322f, -1.000000f,     0.196889f,  0.199602f, -0.783354f,     0.212583f,  0.069567f, -0.839746f,
     0.226413f, -0.058384f, -0.889439f,     0.237446f, -0.183211f, -0.929081f,     0.244750f, -0.303870f, -0.955323f,     0.247391f, -0.419322f, -0.964815f,
     0.391449f,  0.199602f, -0.702462f,     0.421150f,  0.069567f, -0.753032f,     0.447322f, -0.058384f, -0.797593f,     0.468201f, -0.183211f, -0.833141f,
     0.482022f, -0.303870f, -0.856674f,     0.487021f, -0.419322f, -0.865185f,     0.555337f,  0.199602f, -0.576464f,     0.596836f,  0.069567f, -0.617963f,
     0.633404f, -0.058384f, -0.654531f,     0.662577f, -0.183211f, -0.683704f,     0.681888f, -0.303870f, -0.703015f,     0.688873f, -0.419322f, -0.710000f,
     0.681335f,  0.199602f, -0.412576f,     0.731904f,  0.069567f, -0.442277f,     0.776465f, -0.058384f, -0.468449f,     0.812014f, -0.183211f, -0.489328f,
     0.835546f, -0.303870f, -0.503149f,     0.844058f, -0.419322f, -0.508148f,     0.762227f,  0.199602f, -0.218016f,     0.818619f,  0.069567f, -0.233711f,
     0.868312f, -0.058384f, -0.247540f,     0.907954f, -0.183211f, -0.258573f,     0.934196f, -0.303870f, -0.265877f,     0.943688f, -0.419322f, -0.268518f,
     0.925821f, -0.522620f,  0.263546f,     0.960354f, -0.522620f,  0.000000f,     0.881153f, -0.608211f,  0.251115f,     0.914058f, -0.608211f,  0.000000f,
     0.823086f, -0.677134f,  0.234954f,     0.853873f, -0.677134f,  0.000000f,     0.765018f, -0.730433f,  0.218793f,     0.793688f, -0.730433f,  0.000000f,
     0.720351f, -0.769148f,  0.206361f,     0.747391f, -0.769148f,  0.000000f,     0.702484f, -0.794322f,  0.201389f,     0.728873f, -0.794322f,  0.000000f,
     0.828036f, -0.522620f,  0.498738f,     0.787981f, -0.608211f,  0.475213f,     0.735910f, -0.677134f,  0.444630f,     0.683839f, -0.730433f,  0.414047f,
     0.643784f, -0.769148f,  0.390521f,     0.627762f, -0.794322f,  0.381111f,     0.675725f, -0.522620f,  0.696852f,     0.642854f, -0.608211f,  0.663981f,
     0.600123f, -0.677134f,  0.621250f,     0.557391f, -0.730433f,  0.578519f,     0.524521f, -0.769148f,  0.545648f,     0.511373f, -0.794322f,  0.532500f,
     0.477611f, -0.522620f,  0.849163f,     0.454085f, -0.608211f,  0.809108f,     0.423502f, -0.677134f,  0.757037f,     0.392919f, -0.730433f,  0.704966f,
     0.369394f, -0.769148f,  0.664911f,     0.359984f, -0.794322f,  0.648889f,     0.242419f, -0.522620f,  0.946948f,     0.229987f, -0.608211f,  0.902281f,
     0.213826f, -0.677134f,  0.844213f,     0.197666f, -0.730433f,  0.786145f,     0.185234f, -0.769148f,  0.741478f,     0.180262f, -0.794322f,  0.723611f,
    -0.021127f, -0.522620f,  0.981482f,    -0.021127f, -0.608211f,  0.935185f,    -0.021127f, -0.677134f,  0.875000f,    -0.021127f, -0.730433f,  0.814815f,
    -0.021127f, -0.769148f,  0.768519f,    -0.021127f, -0.794322f,  0.750000f,    -0.284673f, -0.522620f,  0.946948f,    -0.272242f, -0.608211f,  0.902281f,
    -0.256081f, -0.677134f,  0.844213f,    -0.239920f, -0.730433f,  0.786145f,    -0.227489f, -0.769148f,  0.741478f,    -0.222516f, -0.794322f,  0.723611f,
    -0.519865f, -0.522620f,  0.849163f,    -0.496340f, -0.608211f,  0.809108f,    -0.465757f, -0.677134f,  0.757037f,    -0.435174f, -0.730433f,  0.704966f,
    -0.411649f, -0.769148f,  0.664911f,    -0.402238f, -0.794322f,  0.648889f,    -0.717979f, -0.522620f,  0.696852f,    -0.685109f, -0.608211f,  0.663981f,
    -0.642377f, -0.677134f,  0.621250f,    -0.599646f, -0.730433f,  0.578519f,    -0.566775f, -0.769148f,  0.545648f,    -0.553627f, -0.794322f,  0.532500f,
    -0.870290f, -0.522620f,  0.498738f,    -0.830236f, -0.608211f,  0.475213f,    -0.778164f, -0.677134f,  0.444630f,    -0.726093f, -0.730433f,  0.414047f,
    -0.686038f, -0.769148f,  0.390521f,    -0.670016f, -0.794322f,  0.381111f,    -0.968075f, -0.522620f,  0.263546f,    -0.923408f, -0.608211f,  0.251115f,
    -0.865340f, -0.677134f,  0.234954f,    -0.807273f, -0.730433f,  0.218793f,    -0.762605f, -0.769148f,  0.206361f,    -0.744738f, -0.794322f,  0.201389f,
    -1.002610f, -0.522620f,  0.000000f,    -0.956312f, -0.608211f,  0.000000f,    -0.896127f, -0.677134f,  0.000000f,    -0.835942f, -0.730433f,  0.000000f,
    -0.789646f, -0.769148f,  0.000000f,    -0.771127f, -0.794322f,  0.000000f,    -0.968075f, -0.522620f, -0.263546f,    -0.923408f, -0.608211f, -0.251115f,
    -0.865340f, -0.677134f, -0.234954f,    -0.807273f, -0.730433f, -0.218793f,    -0.762605f, -0.769148f, -0.206361f,    -0.744738f, -0.794322f, -0.201389f,
    -0.870290f, -0.522620f, -0.498738f,    -0.830236f, -0.608211f, -0.475213f,    -0.778164f, -0.677134f, -0.444630f,    -0.726093f, -0.730433f, -0.414047f,
    -0.686038f, -0.769148f, -0.390521f,    -0.670016f, -0.794322f, -0.381111f,    -0.717979f, -0.522620f, -0.696852f,    -0.685109f, -0.608211f, -0.663981f,
    -0.642377f, -0.677134f, -0.621250f,    -0.599646f, -0.730433f, -0.578519f,    -0.566775f, -0.769148f, -0.545648f,    -0.553627f, -0.794322f, -0.532500f,
    -0.519865f, -0.522620f, -0.849163f,    -0.496340f, -0.608211f, -0.809108f,    -0.465757f, -0.677134f, -0.757037f,    -0.435174f, -0.730433f, -0.704966f,
    -0.411648f, -0.769148f, -0.664911f,    -0.402238f, -0.794322f, -0.648889f,    -0.284673f, -0.522620f, -0.946948f,    -0.272242f, -0.608211f, -0.902281f,
    -0.256081f, -0.677134f, -0.844213f,    -0.239920f, -0.730433f, -0.786145f,    -0.227489f, -0.769148f, -0.741478f,    -0.222516f, -0.794322f, -0.723611f,
    -0.021127f, -0.522620f, -0.981482f,    -0.021127f, -0.608211f, -0.935185f,    -0.021127f, -0.677134f, -0.875000f,    -0.021127f, -0.730433f, -0.814815f,
    -0.021127f, -0.769148f, -0.768519f,    -0.021127f, -0.794322f, -0.750000f,     0.242419f, -0.522620f, -0.946948f,     0.229987f, -0.608211f, -0.902281f,
     0.213827f, -0.677134f, -0.844213f,     0.197666f, -0.730433f, -0.786145f,     0.185234f, -0.769148f, -0.741478f,     0.180262f, -0.794322f, -0.723611f,
     0.477611f, -0.522620f, -0.849163f,     0.454085f, -0.608211f, -0.809108f,     0.423502f, -0.677134f, -0.757037f,     0.392919f, -0.730433f, -0.704966f,
     0.369394f, -0.769148f, -0.664911f,     0.359984f, -0.794322f, -0.648889f,     0.675725f, -0.522620f, -0.696852f,     0.642854f, -0.608211f, -0.663981f,
     0.600123f, -0.677134f, -0.621250f,     0.557391f, -0.730433f, -0.578519f,     0.524521f, -0.769148f, -0.545648f,     0.511373f, -0.794322f, -0.532500f,
     0.828036f, -0.522620f, -0.498738f,     0.787981f, -0.608211f, -0.475213f,     0.735910f, -0.677134f, -0.444630f,     0.683839f, -0.730433f, -0.414047f,
     0.643784f, -0.769148f, -0.390521f,     0.627762f, -0.794322f, -0.381111f,     0.925821f, -0.522620f, -0.263546f,     0.881153f, -0.608211f, -0.251115f,
     0.823086f, -0.677134f, -0.234954f,     0.765018f, -0.730433f, -0.218793f,     0.720351f, -0.769148f, -0.206361f,     0.702484f, -0.794322f, -0.201389f,
     0.696621f, -0.812898f,  0.199757f,     0.722796f, -0.812898f,  0.000000f,     0.667643f, -0.830433f,  0.191692f,     0.692762f, -0.830433f,  0.000000f,
     0.598465f, -0.845884f,  0.172439f,     0.621060f, -0.845884f,  0.000000f,     0.472000f, -0.858211f,  0.137243f,     0.489984f, -0.858211f,  0.000000f,
     0.271165f, -0.866370f,  0.081348f,     0.281824f, -0.866370f,  0.000000f,    -0.021127f, -0.869322f,  0.000000f,     0.622505f, -0.812898f,  0.378023f,
     0.596519f, -0.830433f,  0.362761f,     0.534484f, -0.845884f,  0.326326f,     0.421079f, -0.858211f,  0.259720f,     0.240982f, -0.866370f,  0.153944f,
     0.507059f, -0.812898f,  0.528186f,     0.485734f, -0.830433f,  0.506861f,     0.434826f, -0.845884f,  0.455953f,     0.341762f, -0.858211f,  0.362889f,
     0.193968f, -0.866370f,  0.215095f,     0.356896f, -0.812898f,  0.643632f,     0.341634f, -0.830433f,  0.617646f,     0.305199f, -0.845884f,  0.555611f,
     0.238593f, -0.858211f,  0.442206f,     0.132817f, -0.866370f,  0.262109f,     0.178630f, -0.812898f,  0.717749f,     0.170565f, -0.830433f,  0.688771f,
     0.151312f, -0.845884f,  0.619592f,     0.116116f, -0.858211f,  0.493128f,     0.060221f, -0.866370f,  0.292292f,    -0.021127f, -0.812898f,  0.743924f,
    -0.021127f, -0.830433f,  0.713889f,    -0.021127f, -0.845884f,  0.642188f,    -0.021127f, -0.858211f,  0.511111f,    -0.021127f, -0.866370f,  0.302951f,
    -0.220884f, -0.812898f,  0.717749f,    -0.212820f, -0.830433f,  0.688771f,    -0.193566f, -0.845884f,  0.619592f,    -0.158370f, -0.858211f,  0.493128f,
    -0.102475f, -0.866370f,  0.292292f,    -0.399151f, -0.812898f,  0.643632f,    -0.383889f, -0.830433f,  0.617646f,    -0.347454f, -0.845884f,  0.555611f,
    -0.280847f, -0.858211f,  0.442206f,    -0.175071f, -0.866370f,  0.262109f,    -0.549313f, -0.812898f,  0.528186f,    -0.527988f, -0.830433f,  0.506861f,
    -0.477080f, -0.845884f,  0.455953f,    -0.384016f, -0.858211f,  0.362889f,    -0.236223f, -0.866370f,  0.215095f,    -0.664759f, -0.812898f,  0.378023f,
    -0.638773f, -0.830433f,  0.362761f,    -0.576738f, -0.845884f,  0.326326f,    -0.463333f, -0.858211f,  0.259720f,    -0.283236f, -0.866370f,  0.153944f,
    -0.738876f, -0.812898f,  0.199757f,    -0.709898f, -0.830433f,  0.191692f,    -0.640719f, -0.845884f,  0.172439f,    -0.514255f, -0.858211f,  0.137243f,
    -0.313419f, -0.866370f,  0.081348f,    -0.765051f, -0.812898f,  0.000000f,    -0.735016f, -0.830433f,  0.000000f,    -0.663315f, -0.845884f,  0.000000f,
    -0.532238f, -0.858211f,  0.000000f,    -0.324079f, -0.866370f,  0.000000f,    -0.738876f, -0.812898f, -0.199757f,    -0.709898f, -0.830433f, -0.191692f,
    -0.640719f, -0.845884f, -0.172439f,    -0.514255f, -0.858211f, -0.137243f,    -0.313419f, -0.866370f, -0.081348f,    -0.664759f, -0.812898f, -0.378023f,
    -0.638773f, -0.830433f, -0.362761f,    -0.576738f, -0.845884f, -0.326326f,    -0.463333f, -0.858211f, -0.259720f,    -0.283236f, -0.866370f, -0.153944f,
    -0.549313f, -0.812898f, -0.528186f,    -0.527988f, -0.830433f, -0.506861f,    -0.477080f, -0.845884f, -0.455953f,    -0.384016f, -0.858211f, -0.362889f,
    -0.236223f, -0.866370f, -0.215095f,    -0.399151f, -0.812898f, -0.643632f,    -0.383889f, -0.830433f, -0.617646f,    -0.347454f, -0.845884f, -0.555611f,
    -0.280847f, -0.858211f, -0.442206f,    -0.175071f, -0.866370f, -0.262109f,    -0.220884f, -0.812898f, -0.717749f,    -0.212820f, -0.830433f, -0.688771f,
    -0.193566f, -0.845884f, -0.619592f,    -0.158370f, -0.858211f, -0.493128f,    -0.102475f, -0.866370f, -0.292292f,    -0.021127f, -0.812898f, -0.743924f,
    -0.021127f, -0.830433f, -0.713889f,    -0.021127f, -0.845884f, -0.642188f,    -0.021127f, -0.858211f, -0.511111f,    -0.021127f, -0.866370f, -0.302951f,
     0.178630f, -0.812898f, -0.717749f,     0.170565f, -0.830433f, -0.688771f,     0.151312f, -0.845884f, -0.619592f,     0.116116f, -0.858211f, -0.493128f,
     0.060221f, -0.866370f, -0.292292f,     0.356896f, -0.812898f, -0.643632f,     0.341634f, -0.830433f, -0.617646f,     0.305199f, -0.845884f, -0.555611f,
     0.238593f, -0.858211f, -0.442206f,     0.132817f, -0.866370f, -0.262109f,     0.507059f, -0.812898f, -0.528186f,     0.485734f, -0.830433f, -0.506861f,
     0.434826f, -0.845884f, -0.455953f,     0.341762f, -0.858211f, -0.362889f,     0.193968f, -0.866370f, -0.215095f,     0.622505f, -0.812898f, -0.378023f,
     0.596519f, -0.830433f, -0.362761f,     0.534484f, -0.845884f, -0.326326f,     0.421079f, -0.858211f, -0.259720f,     0.240982f, -0.866370f, -0.153944f,
     0.696621f, -0.812898f, -0.199757f,     0.667643f, -0.830433f, -0.191692f,     0.598465f, -0.845884f, -0.172439f,     0.472000f, -0.858211f, -0.137243f,
     0.271165f, -0.866370f, -0.081348f,    -0.821127f,  0.143178f,  0.000000f,    -0.817424f,  0.151512f,  0.062500f,    -0.984648f,  0.150952f,  0.062500f,
    -0.983396f,  0.142657f,  0.000000f,    -1.124350f,  0.147036f,  0.062500f,    -1.119270f,  0.139012f,  0.000000f,    -1.235250f,  0.136407f,  0.062500f,
    -1.227380f,  0.129116f,  0.000000f,    -1.316050f,  0.115709f,  0.062500f,    -1.306310f,  0.109845f,  0.000000f,    -1.365480f,  0.081585f,  0.062500f,
    -1.354690f,  0.078074f,  0.000000f,    -1.382240f,  0.030678f,  0.062500f,    -1.371130f,  0.030678f,  0.000000f,    -0.808164f,  0.172345f,  0.100000f,
    -0.987777f,  0.171689f,  0.100000f,    -1.137040f,  0.167098f,  0.100000f,    -1.254920f,  0.154637f,  0.100000f,    -1.340400f,  0.130370f,  0.100000f,
    -1.392440f,  0.090362f,  0.100000f,    -1.410020f,  0.030678f,  0.100000f,    -0.796127f,  0.199428f,  0.112500f,    -0.991845f,  0.198647f,  0.112500f,
    -1.153540f,  0.193178f,  0.112500f,    -1.280500f,  0.178335f,  0.112500f,    -1.372050f,  0.149428f,  0.112500f,    -1.427490f,  0.101772f,  0.112500f,
    -1.446130f,  0.030678f,  0.112500f,    -0.784090f,  0.226511f,  0.100000f,    -0.995913f,  0.225605f,  0.100000f,    -1.170030f,  0.219258f,  0.100000f,
    -1.306080f,  0.202032f,  0.100000f,    -1.403710f,  0.168487f,  0.100000f,    -1.462550f,  0.113182f,  0.100000f,    -1.482240f,  0.030678f,  0.100000f,
    -0.774831f,  0.247345f,  0.062500f,    -0.999042f,  0.246342f,  0.062500f,    -1.182720f,  0.239320f,  0.062500f,    -1.325760f,  0.220261f,  0.062500f,
    -1.428050f,  0.183147f,  0.062500f,    -1.489510f,  0.121959f,  0.062500f,    -1.510020f,  0.030678f,  0.062500f,    -0.771127f,  0.255678f,  0.000000f,
    -1.000290f,  0.254636f,  0.000000f,    -1.187790f,  0.247345f,  0.000000f,    -1.333630f,  0.227553f,  0.000000f,    -1.437790f,  0.189011f,  0.000000f,
    -1.500290f,  0.125470f,  0.000000f,    -1.521130f,  0.030678f,  0.000000f,    -0.774831f,  0.247345f, -0.062500f,    -0.999042f,  0.246342f, -0.062500f,
    -1.182720f,  0.239320f, -0.062500f,    -1.325760f,  0.220261f, -0.062500f,    -1.428050f,  0.183147f, -0.062500f,    -1.489510f,  0.121959f, -0.062500f,
    -1.510020f,  0.030678f, -0.062500f,    -0.784090f,  0.226511f, -0.100000f,    -0.995913f,  0.225605f, -0.100000f,    -1.170030f,  0.219258f, -0.100000f,
    -1.306080f,  0.202032f, -0.100000f,    -1.403710f,  0.168487f, -0.100000f,    -1.462550f,  0.113182f, -0.100000f,    -1.482240f,  0.030678f, -0.100000f,
    -0.796127f,  0.199428f, -0.112500f,    -0.991845f,  0.198647f, -0.112500f,    -1.153540f,  0.193178f, -0.112500f,    -1.280500f,  0.178335f, -0.112500f,
    -1.372050f,  0.149428f, -0.112500f,    -1.427490f,  0.101772f, -0.112500f,    -1.446130f,  0.030678f, -0.112500f,    -0.808164f,  0.172345f, -0.100000f,
    -0.987777f,  0.171689f, -0.100000f,    -1.137040f,  0.167098f, -0.100000f,    -1.254920f,  0.154637f, -0.100000f,    -1.340400f,  0.130370f, -0.100000f,
    -1.392440f,  0.090362f, -0.100000f,    -1.410020f,  0.030678f, -0.100000f,    -0.817424f,  0.151512f, -0.062500f,    -0.984648f,  0.150952f, -0.062500f,
    -1.124350f,  0.147036f, -0.062500f,    -1.235250f,  0.136407f, -0.062500f,    -1.316050f,  0.115709f, -0.062500f,    -1.365480f,  0.081585f, -0.062500f,
    -1.382240f,  0.030678f, -0.062500f,    -1.373220f, -0.037332f,  0.062500f,    -1.362560f, -0.033905f,  0.000000f,    -1.345270f, -0.116647f,  0.062500f,
    -1.335940f, -0.110988f,  0.000000f,    -1.297050f, -0.201440f,  0.062500f,    -1.289880f, -0.194322f,  0.000000f,    -1.227230f, -0.285886f,  0.062500f,
    -1.222980f, -0.277655f,  0.000000f,    -1.134470f, -0.364159f,  0.062500f,    -1.133860f, -0.354739f,  0.000000f,    -1.017420f, -0.430433f,  0.062500f,
    -1.021130f, -0.419322f,  0.000000f,    -1.399860f, -0.045900f,  0.100000f,    -1.368590f, -0.130793f,  0.100000f,    -1.314990f, -0.219235f,  0.100000f,
    -1.237860f, -0.306462f,  0.100000f,    -1.135990f, -0.387709f,  0.100000f,    -1.008160f, -0.458211f,  0.100000f,    -1.434490f, -0.057039f,  0.112500f,
    -1.398910f, -0.149183f,  0.112500f,    -1.338320f, -0.242369f,  0.112500f,    -1.251680f, -0.333211f,  0.112500f,    -1.137970f, -0.418324f,  0.112500f,
    -0.996127f, -0.494322f,  0.112500f,    -1.469130f, -0.068177f,  0.100000f,    -1.429220f, -0.167573f,  0.100000f,    -1.361640f, -0.265502f,  0.100000f,
    -1.265500f, -0.359960f,  0.100000f,    -1.139950f, -0.448939f,  0.100000f,    -0.984090f, -0.530433f,  0.100000f,    -1.495770f, -0.076745f,  0.062500f,
    -1.452540f, -0.181719f,  0.062500f,    -1.379580f, -0.283298f,  0.062500f,    -1.276130f, -0.380536f,  0.062500f,    -1.141470f, -0.472489f,  0.062500f,
    -0.974831f, -0.558211f,  0.062500f,    -1.506430f, -0.080173f,  0.000000f,    -1.461870f, -0.187377f,  0.000000f,    -1.386750f, -0.290416f,  0.000000f,
    -1.280390f, -0.388766f,  0.000000f,    -1.142080f, -0.481909f,  0.000000f,    -0.971127f, -0.569322f,  0.000000f,    -1.495770f, -0.076745f, -0.062500f,
    -1.452540f, -0.181719f, -0.062500f,    -1.379580f, -0.283298f, -0.062500f,    -1.276130f, -0.380536f, -0.062500f,    -1.141470f, -0.472489f, -0.062500f,
    -0.974831f, -0.558211f, -0.062500f,    -1.469130f, -0.068177f, -0.100000f,    -1.429220f, -0.167573f, -0.100000f,    -1.361640f, -0.265502f, -0.100000f,
    -1.265500f, -0.359960f, -0.100000f,    -1.139950f, -0.448939f, -0.100000f,    -0.984090f, -0.530433f, -0.100000f,    -1.434490f, -0.057039f, -0.112500f,
    -1.398910f, -0.149183f, -0.112500f,    -1.338320f, -0.242369f, -0.112500f,    -1.251680f, -0.333211f, -0.112500f,    -1.137970f, -0.418324f, -0.112500f,
    -0.996127f, -0.494322f, -0.112500f,    -1.399860f, -0.045900f, -0.100000f,    -1.368590f, -0.130793f, -0.100000f,    -1.314990f, -0.219235f, -0.100000f,
    -1.237860f, -0.306462f, -0.100000f,    -1.135990f, -0.387709f, -0.100000f,    -1.008160f, -0.458211f, -0.100000f,    -1.373220f, -0.037332f, -0.062500f,
    -1.345270f, -0.116647f, -0.062500f,    -1.297050f, -0.201440f, -0.062500f,    -1.227230f, -0.285886f, -0.062500f,    -1.134470f, -0.364159f, -0.062500f,
    -1.017420f, -0.430433f, -0.062500f,     0.828873f, -0.156822f,  0.000000f,     0.828873f, -0.187377f,  0.137500f,     1.015060f, -0.156719f,  0.131173f,
     1.008270f, -0.131127f,  0.000000f,     1.123930f, -0.083314f,  0.115355f,     1.114060f, -0.063766f,  0.000000f,     1.183730f,  0.017484f,  0.094792f,
     1.172620f,  0.030678f,  0.000000f,     1.222700f,  0.130318f,  0.074228f,     1.210350f,  0.137623f,  0.000000f,     1.269070f,  0.239835f,  0.058411f,
     1.253640f,  0.242484f,  0.000000f,     1.351090f,  0.330678f,  0.052083f,     1.328870f,  0.330678f,  0.000000f,     0.828873f, -0.263766f,  0.220000f,
     1.032040f, -0.220698f,  0.209877f,     1.148630f, -0.132182f,  0.184568f,     1.211510f, -0.015502f,  0.151667f,     1.253560f,  0.112057f,  0.118765f,
     1.307650f,  0.233212f,  0.093457f,     1.406650f,  0.330678f,  0.083333f,     0.828873f, -0.363072f,  0.247500f,     1.054100f, -0.303870f,  0.236111f,
     1.180720f, -0.195711f,  0.207639f,     1.247620f, -0.058384f,  0.170625f,     1.293690f,  0.088317f,  0.133611f,     1.357810f,  0.224602f,  0.105139f,
     1.478870f,  0.330678f,  0.093750f,     0.828873f, -0.462377f,  0.220000f,     1.076170f, -0.387043f,  0.209877f,     1.212820f, -0.259240f,  0.184568f,
     1.283730f, -0.101266f,  0.151667f,     1.333810f,  0.064577f,  0.118765f,     1.407960f,  0.215992f,  0.093457f,     1.551100f,  0.330678f,  0.083333f,
     0.828873f, -0.538766f,  0.137500f,     1.093150f, -0.451022f,  0.131173f,     1.237510f, -0.308108f,  0.115355f,     1.311510f, -0.134252f,  0.094792f,
     1.364680f,  0.046316f,  0.074228f,     1.446540f,  0.209369f,  0.058410f,     1.606650f,  0.330678f,  0.052083f,     0.828873f, -0.569322f,  0.000000f,
     1.099940f, -0.476614f,  0.000000f,     1.247390f, -0.327655f,  0.000000f,     1.322620f, -0.147447f,  0.000000f,     1.377020f,  0.039012f,  0.000000f,
     1.461970f,  0.206720f,  0.000000f,     1.628870f,  0.330678f,  0.000000f,     0.828873f, -0.538766f, -0.137500f,     1.093150f, -0.451022f, -0.131173f,
     1.237510f, -0.308108f, -0.115355f,     1.311510f, -0.134252f, -0.094792f,     1.364680f,  0.046316f, -0.074228f,     1.446540f,  0.209369f, -0.058410f,
     1.606650f,  0.330678f, -0.052083f,     0.828873f, -0.462377f, -0.220000f,     1.076170f, -0.387043f, -0.209877f,     1.212820f, -0.259240f, -0.184568f,
     1.283730f, -0.101266f, -0.151667f,     1.333810f,  0.064577f, -0.118765f,     1.407960f,  0.215992f, -0.093457f,     1.551100f,  0.330678f, -0.083333f,
     0.828873f, -0.363072f, -0.247500f,     1.054100f, -0.303870f, -0.236111f,     1.180720f, -0.195711f, -0.207639f,     1.247620f, -0.058384f, -0.170625f,
     1.293690f,  0.088317f, -0.133611f,     1.357810f,  0.224602f, -0.105139f,     1.478870f,  0.330678f, -0.093750f,     0.828873f, -0.263766f, -0.220000f,
     1.032040f, -0.220698f, -0.209877f,     1.148630f, -0.132182f, -0.184568f,     1.211510f, -0.015502f, -0.151667f,     1.253560f,  0.112057f, -0.118765f,
     1.307650f,  0.233212f, -0.093457f,     1.406650f,  0.330678f, -0.083333f,     0.828873f, -0.187377f, -0.137500f,     1.015060f, -0.156719f, -0.131173f,
     1.123930f, -0.083314f, -0.115355f,     1.183730f,  0.017484f, -0.094792f,     1.222700f,  0.130318f, -0.074228f,     1.269070f,  0.239835f, -0.058410f,
     1.351090f,  0.330678f, -0.052083f,     1.377080f,  0.346641f,  0.050540f,     1.353410f,  0.346303f,  0.000000f,     1.398760f,  0.356295f,  0.046682f,
     1.375170f,  0.355678f,  0.000000f,     1.413710f,  0.359584f,  0.041667f,     1.391370f,  0.358803f,  0.000000f,     1.419480f,  0.356450f,  0.036651f,
     1.399240f,  0.355678f,  0.000000f,     1.413620f,  0.346834f,  0.032793f,     1.396000f,  0.346303f,  0.000000f,     1.393690f,  0.330678f,  0.031250f,
     1.378870f,  0.330678f,  0.000000f,     1.436240f,  0.347485f,  0.080864f,     1.457750f,  0.357839f,  0.074691f,     1.469560f,  0.361538f,  0.066667f,
     1.470060f,  0.358379f,  0.058642f,     1.457650f,  0.348160f,  0.052469f,     1.430720f,  0.330678f,  0.050000f,     1.513160f,  0.348582f,  0.090972f,
     1.534430f,  0.359845f,  0.084028f,     1.542150f,  0.364077f,  0.075000f,     1.535820f,  0.360886f,  0.065972f,     1.514900f,  0.349884f,  0.059028f,
     1.478870f,  0.330678f,  0.056250f,     1.590080f,  0.349679f,  0.080864f,     1.611110f,  0.361851f,  0.074691f,     1.614750f,  0.366616f,  0.066667f,
     1.601580f,  0.363394f,  0.058642f,     1.572140f,  0.351608f,  0.052469f,     1.527020f,  0.330678f,  0.050000f,     1.649250f,  0.350523f,  0.050540f,
     1.670090f,  0.363394f,  0.046682f,     1.670600f,  0.368569f,  0.041667f,     1.652160f,  0.365323f,  0.036651f,     1.616180f,  0.352934f,  0.032793f,
     1.564060f,  0.330678f,  0.031250f,     1.672910f,  0.350860f,  0.000000f,     1.693690f,  0.364011f,  0.000000f,     1.692930f,  0.369350f,  0.000000f,
     1.672390f,  0.366095f,  0.000000f,     1.633790f,  0.353465f,  0.000000f,     1.578870f,  0.330678f,  0.000000f,     1.649250f,  0.350523f, -0.050540f,
     1.670090f,  0.363394f, -0.046682f,     1.670600f,  0.368569f, -0.041667f,     1.652160f,  0.365323f, -0.036651f,     1.616180f,  0.352934f, -0.032793f,
     1.564060f,  0.330678f, -0.031250f,     1.590080f,  0.349679f, -0.080864f,     1.611110f,  0.361851f, -0.074691f,     1.614750f,  0.366616f, -0.066667f,
     1.601580f,  0.363394f, -0.058642f,     1.572140f,  0.351608f, -0.052469f,     1.527020f,  0.330678f, -0.050000f,     1.513160f,  0.348582f, -0.090972f,
     1.534430f,  0.359845f, -0.084028f,     1.542150f,  0.364077f, -0.075000f,     1.535820f,  0.360886f, -0.065972f,     1.514900f,  0.349884f, -0.059028f,
     1.478870f,  0.330678f, -0.056250f,     1.436240f,  0.347485f, -0.080864f,     1.457750f,  0.357839f, -0.074691f,     1.469560f,  0.361538f, -0.066667f,
     1.470060f,  0.358379f, -0.058642f,     1.457650f,  0.348160f, -0.052469f,     1.430720f,  0.330678f, -0.050000f,     1.377080f,  0.346641f, -0.050540f,
     1.398760f,  0.356295f, -0.046682f,     1.413710f,  0.359584f, -0.041667f,     1.419480f,  0.356450f, -0.036651f,     1.413620f,  0.346834f, -0.032793f,
     1.393690f,  0.330678f, -0.031250f,     0.113346f,  0.694220f,  0.037539f,     0.118225f,  0.694220f,  0.000000f,    -0.021127f,  0.705678f,  0.000000f,
     0.154000f,  0.664011f,  0.048885f,     0.160354f,  0.664011f,  0.000000f,     0.135681f,  0.621303f,  0.043764f,     0.141373f,  0.621303f,  0.000000f,
     0.093237f,  0.572345f,  0.031902f,     0.097391f,  0.572345f,  0.000000f,     0.061512f,  0.523386f,  0.023022f,     0.064521f,  0.523386f,  0.000000f,
     0.075354f,  0.480678f,  0.026852f,     0.078873f,  0.480678f,  0.000000f,     0.099515f,  0.694220f,  0.070966f,     0.135987f,  0.664011f,  0.092417f,
     0.119549f,  0.621303f,  0.082741f,     0.081463f,  0.572345f,  0.060324f,     0.052990f,  0.523386f,  0.043553f,     0.065391f,  0.480678f,  0.050815f,
     0.077943f,  0.694220f,  0.099070f,     0.107891f,  0.664011f,  0.129019f,     0.094388f,  0.621303f,  0.115516f,     0.063104f,  0.572345f,  0.084231f,
     0.039709f,  0.523386f,  0.060836f,     0.049873f,  0.480678f,  0.071000f,     0.049838f,  0.694220f,  0.120642f,     0.071290f,  0.664011f,  0.157114f,
     0.061614f,  0.621303f,  0.140676f,     0.039197f,  0.572345f,  0.102590f,     0.022426f,  0.523386f,  0.074117f,     0.029688f,  0.480678f,  0.086519f,
     0.016412f,  0.694220f,  0.134473f,     0.027758f,  0.664011f,  0.175127f,     0.022637f,  0.621303f,  0.156808f,     0.010774f,  0.572345f,  0.114364f,
     0.001895f,  0.523386f,  0.082639f,     0.005725f,  0.480678f,  0.096482f,    -0.021127f,  0.694220f,  0.139352f,    -0.021127f,  0.664011f,  0.181482f,
    -0.021127f,  0.621303f,  0.162500f,    -0.021127f,  0.572345f,  0.118519f,    -0.021127f,  0.523386f,  0.085648f,    -0.021127f,  0.480678f,  0.100000f,
    -0.058666f,  0.694220f,  0.134473f,    -0.070013f,  0.664011f,  0.175127f,    -0.064892f,  0.621303f,  0.156808f,    -0.053029f,  0.572345f,  0.114364f,
    -0.044149f,  0.523386f,  0.082639f,    -0.047979f,  0.480678f,  0.096481f,    -0.092093f,  0.694220f,  0.120642f,    -0.113544f,  0.664011f,  0.157114f,
    -0.103868f,  0.621303f,  0.140676f,    -0.081451f,  0.572345f,  0.102590f,    -0.064680f,  0.523386f,  0.074117f,    -0.071942f,  0.480678f,  0.086519f,
    -0.120197f,  0.694220f,  0.099070f,    -0.150146f,  0.664011f,  0.129019f,    -0.136643f,  0.621303f,  0.115516f,    -0.105359f,  0.572345f,  0.084231f,
    -0.081963f,  0.523386f,  0.060836f,    -0.092127f,  0.480678f,  0.071000f,    -0.141770f,  0.694220f,  0.070966f,    -0.178241f,  0.664011f,  0.092417f,
    -0.161803f,  0.621303f,  0.082741f,    -0.123717f,  0.572345f,  0.060324f,    -0.095244f,  0.523386f,  0.043553f,    -0.107646f,  0.480678f,  0.050815f,
    -0.155600f,  0.694220f,  0.037539f,    -0.196254f,  0.664011f,  0.048885f,    -0.177936f,  0.621303f,  0.043764f,    -0.135491f,  0.572345f,  0.031902f,
    -0.103767f,  0.523386f,  0.023022f,    -0.117609f,  0.480678f,  0.026852f,    -0.160479f,  0.694220f,  0.000000f,    -0.202609f,  0.664011f,  0.000000f,
    -0.183627f,  0.621303f,  0.000000f,    -0.139646f,  0.572345f,  0.000000f,    -0.106775f,  0.523386f,  0.000000f,    -0.121127f,  0.480678f,  0.000000f,
    -0.155600f,  0.694220f, -0.037539f,    -0.196254f,  0.664011f, -0.048885f,    -0.177936f,  0.621303f, -0.043764f,    -0.135491f,  0.572345f, -0.031902f,
    -0.103767f,  0.523386f, -0.023022f,    -0.117609f,  0.480678f, -0.026852f,    -0.141770f,  0.694220f, -0.070966f,    -0.178241f,  0.664011f, -0.092417f,
    -0.161803f,  0.621303f, -0.082741f,    -0.123717f,  0.572345f, -0.060324f,    -0.095244f,  0.523386f, -0.043553f,    -0.107646f,  0.480678f, -0.050815f,
    -0.120197f,  0.694220f, -0.099070f,    -0.150146f,  0.664011f, -0.129019f,    -0.136643f,  0.621303f, -0.115516f,    -0.105359f,  0.572345f, -0.084231f,
    -0.081963f,  0.523386f, -0.060836f,    -0.092127f,  0.480678f, -0.071000f,    -0.092093f,  0.694220f, -0.120642f,    -0.113544f,  0.664011f, -0.157114f,
    -0.103868f,  0.621303f, -0.140676f,    -0.081451f,  0.572345f, -0.102590f,    -0.064680f,  0.523386f, -0.074117f,    -0.071942f,  0.480678f, -0.086519f,
    -0.058666f,  0.694220f, -0.134473f,    -0.070013f,  0.664011f, -0.175127f,    -0.064892f,  0.621303f, -0.156808f,    -0.053029f,  0.572345f, -0.114364f,
    -0.044149f,  0.523386f, -0.082639f,    -0.047979f,  0.480678f, -0.096482f,    -0.021127f,  0.694220f, -0.139352f,    -0.021127f,  0.664011f, -0.181482f,
    -0.021127f,  0.621303f, -0.162500f,    -0.021127f,  0.572345f, -0.118519f,    -0.021127f,  0.523386f, -0.085648f,    -0.021127f,  0.480678f, -0.100000f,
     0.016412f,  0.694220f, -0.134473f,     0.027758f,  0.664011f, -0.175127f,     0.022637f,  0.621303f, -0.156808f,     0.010774f,  0.572345f, -0.114364f,
     0.001895f,  0.523386f, -0.082639f,     0.005725f,  0.480678f, -0.096481f,     0.049838f,  0.694220f, -0.120642f,     0.071290f,  0.664011f, -0.157114f,
     0.061614f,  0.621303f, -0.140676f,     0.039197f,  0.572345f, -0.102590f,     0.022426f,  0.523386f, -0.074117f,     0.029688f,  0.480678f, -0.086519f,
     0.077943f,  0.694220f, -0.099070f,     0.107891f,  0.664011f, -0.129019f,     0.094388f,  0.621303f, -0.115516f,     0.063104f,  0.572345f, -0.084231f,
     0.039709f,  0.523386f, -0.060836f,     0.049873f,  0.480678f, -0.071000f,     0.099515f,  0.694220f, -0.070966f,     0.135987f,  0.664011f, -0.092417f,
     0.119549f,  0.621303f, -0.082741f,     0.081463f,  0.572345f, -0.060324f,     0.052990f,  0.523386f, -0.043553f,     0.065391f,  0.480678f, -0.050815f,
     0.113346f,  0.694220f, -0.037539f,     0.154000f,  0.664011f, -0.048885f,     0.135681f,  0.621303f, -0.043764f,     0.093237f,  0.572345f, -0.031902f,
     0.061512f,  0.523386f, -0.023022f,     0.075354f,  0.480678f, -0.026852f,     0.148162f,  0.448734f,  0.047115f,     0.154336f,  0.448734f,  0.000000f,
     0.255810f,  0.425123f,  0.077075f,     0.265910f,  0.425123f,  0.000000f,     0.376859f,  0.405678f,  0.110764f,     0.391373f,  0.405678f,  0.000000f,
     0.489867f,  0.386234f,  0.142215f,     0.508502f,  0.386234f,  0.000000f,     0.573395f,  0.362623f,  0.165462f,     0.595077f,  0.362623f,  0.000000f,
     0.606002f,  0.330678f,  0.174537f,     0.628873f,  0.330678f,  0.000000f,     0.130681f,  0.448734f,  0.089161f,     0.227213f,  0.425123f,  0.145857f,
     0.335762f,  0.405678f,  0.209611f,     0.437101f,  0.386234f,  0.269130f,     0.512003f,  0.362623f,  0.313123f,     0.541243f,  0.330678f,  0.330296f,
     0.103451f,  0.448734f,  0.124579f,     0.182669f,  0.425123f,  0.203796f,     0.271748f,  0.405678f,  0.292875f,     0.354910f,  0.386234f,  0.376037f,
     0.416377f,  0.362623f,  0.437505f,     0.440373f,  0.330678f,  0.461500f,     0.068034f,  0.448734f,  0.151808f,     0.124730f,  0.425123f,  0.248340f,
     0.188484f,  0.405678f,  0.356889f,     0.248003f,  0.386234f,  0.458228f,     0.291995f,  0.362623f,  0.533130f,     0.309169f,  0.330678f,  0.562370f,
     0.025988f,  0.448734f,  0.169289f,     0.055948f,  0.425123f,  0.276938f,     0.089637f,  0.405678f,  0.397986f,     0.121088f,  0.386234f,  0.510995f,
     0.144335f,  0.362623f,  0.594523f,     0.153410f,  0.330678f,  0.627130f,    -0.021127f,  0.448734f,  0.175463f,    -0.021127f,  0.425123f,  0.287037f,
    -0.021127f,  0.405678f,  0.412500f,    -0.021127f,  0.386234f,  0.529630f,    -0.021127f,  0.362623f,  0.616204f,    -0.021127f,  0.330678f,  0.650000f,
    -0.068242f,  0.448734f,  0.169289f,    -0.098202f,  0.425123f,  0.276938f,    -0.131891f,  0.405678f,  0.397986f,    -0.163343f,  0.386234f,  0.510995f,
    -0.186589f,  0.362623f,  0.594523f,    -0.195664f,  0.330678f,  0.627130f,    -0.110288f,  0.448734f,  0.151808f,    -0.166985f,  0.425123f,  0.248340f,
    -0.230738f,  0.405678f,  0.356889f,    -0.290258f,  0.386234f,  0.458228f,    -0.334250f,  0.362623f,  0.533130f,    -0.351424f,  0.330678f,  0.562370f,
    -0.145706f,  0.448734f,  0.124579f,    -0.224924f,  0.425123f,  0.203796f,    -0.314002f,  0.405678f,  0.292875f,    -0.397164f,  0.386234f,  0.376037f,
    -0.458632f,  0.362623f,  0.437505f,    -0.482627f,  0.330678f,  0.461500f,    -0.172935f,  0.448734f,  0.089161f,    -0.269467f,  0.425123f,  0.145857f,
    -0.378016f,  0.405678f,  0.209611f,    -0.479355f,  0.386234f,  0.269130f,    -0.554258f,  0.362623f,  0.313123f,    -0.583498f,  0.330678f,  0.330296f,
    -0.190416f,  0.448734f,  0.047115f,    -0.298065f,  0.425123f,  0.077075f,    -0.419113f,  0.405678f,  0.110764f,    -0.532122f,  0.386234f,  0.142215f,
    -0.615650f,  0.362623f,  0.165462f,    -0.648257f,  0.330678f,  0.174537f,    -0.196590f,  0.448734f,  0.000000f,    -0.308164f,  0.425123f,  0.000000f,
    -0.433627f,  0.405678f,  0.000000f,    -0.550757f,  0.386234f,  0.000000f,    -0.637331f,  0.362623f,  0.000000f,    -0.671127f,  0.330678f,  0.000000f,
    -0.190416f,  0.448734f, -0.047115f,    -0.298065f,  0.425123f, -0.077075f,    -0.419113f,  0.405678f, -0.110764f,    -0.532122f,  0.386234f, -0.142215f,
    -0.615650f,  0.362623f, -0.165462f,    -0.648257f,  0.330678f, -0.174537f,    -0.172935f,  0.448734f, -0.089161f,    -0.269467f,  0.425123f, -0.145857f,
    -0.378016f,  0.405678f, -0.209611f,    -0.479355f,  0.386234f, -0.269130f,    -0.554258f,  0.362623f, -0.313123f,    -0.583498f,  0.330678f, -0.330296f,
    -0.145706f,  0.448734f, -0.124579f,    -0.224924f,  0.425123f, -0.203796f,    -0.314002f,  0.405678f, -0.292875f,    -0.397164f,  0.386234f, -0.376037f,
    -0.458632f,  0.362623f, -0.437505f,    -0.482627f,  0.330678f, -0.461500f,    -0.110288f,  0.448734f, -0.151808f,    -0.166985f,  0.425123f, -0.248340f,
    -0.230738f,  0.405678f, -0.356889f,    -0.290258f,  0.386234f, -0.458228f,    -0.334250f,  0.362623f, -0.533130f,    -0.351424f,  0.330678f, -0.562370f,
    -0.068242f,  0.448734f, -0.169289f,    -0.098202f,  0.425123f, -0.276938f,    -0.131891f,  0.405678f, -0.397986f,    -0.163343f,  0.386234f, -0.510995f,
    -0.186589f,  0.362623f, -0.594523f,    -0.195664f,  0.330678f, -0.627130f,    -0.021127f,  0.448734f, -0.175463f,    -0.021127f,  0.425123f, -0.287037f,
    -0.021127f,  0.405678f, -0.412500f,    -0.021127f,  0.386234f, -0.529630f,    -0.021127f,  0.362623f, -0.616204f,    -0.021127f,  0.330678f, -0.650000f,
     0.025988f,  0.448734f, -0.169289f,     0.055948f,  0.425123f, -0.276938f,     0.089637f,  0.405678f, -0.397986f,     0.121088f,  0.386234f, -0.510995f,
     0.144335f,  0.362623f, -0.594523f,     0.153410f,  0.330678f, -0.627130f,     0.068034f,  0.448734f, -0.151808f,     0.124730f,  0.425123f, -0.248340f,
     0.188484f,  0.405678f, -0.356889f,     0.248003f,  0.386234f, -0.458228f,     0.291996f,  0.362623f, -0.533130f,     0.309169f,  0.330678f, -0.562370f,
     0.103451f,  0.448734f, -0.124579f,     0.182669f,  0.425123f, -0.203796f,     0.271748f,  0.405678f, -0.292875f,     0.354910f,  0.386234f, -0.376037f,
     0.416377f,  0.362623f, -0.437505f,     0.440373f,  0.330678f, -0.461500f,     0.130681f,  0.448734f, -0.089161f,     0.227213f,  0.425123f, -0.145857f,
     0.335762f,  0.405678f, -0.209611f,     0.437101f,  0.386234f, -0.269130f,     0.512003f,  0.362623f, -0.313123f,     0.541243f,  0.330678f, -0.330296f,
     0.148162f,  0.448734f, -0.047115f,     0.255810f,  0.425123f, -0.077075f,     0.376859f,  0.405678f, -0.110764f,     0.489867f,  0.386234f, -0.142215f,
     0.573395f,  0.362623f, -0.165462f,     0.606002f,  0.330678f, -0.174537f,     0.000000f,  0.000000f,  0.000000f,     0.000000f,  0.000000f,  0.000000f,

};


/*
 * ========== Teapot triangles: ==========
 */

const s32 __spTeapotIndices[] = {

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

const dim::vector3df __spTeapotPatchesData[32][4][4] = {
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
