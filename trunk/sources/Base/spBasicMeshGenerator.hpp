/*
 * Basic mesh generator header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_SCENEMANAGER_MODELCREATOR_H__
#define __SP_SCENEMANAGER_MODELCREATOR_H__


#include "Base/spStandard.hpp"
#include "Base/spMath.hpp"
#include "Base/spMathRandomizer.hpp"
#include "SceneGraph/spSceneMesh.hpp"


namespace sp
{
namespace scene
{


//! Basic meshes construction settings
struct SP_EXPORT SMeshConstruct
{
    SMeshConstruct();
    SMeshConstruct(
        s32 MeshSegments, f32 MeshRadius = 0.5f, bool MeshHasCap = true,
        const video::EShadingTypes DefShading = video::SHADING_GOURAUD
    );
    SMeshConstruct(
        s32 MeshSegments, f32 MeshRadiusInner, f32 MeshRadiusOuter, bool MeshHasCap = true,
        const video::EShadingTypes DefShading = video::SHADING_GOURAUD
    );
    SMeshConstruct(
        s32 MeshSegmentsVert, s32 MeshSegmentsHorz, f32 MeshRadiusInner, f32 MeshRadiusOuter,
        bool MeshHasCap = true, const video::EShadingTypes DefShading = video::SHADING_GOURAUD
    );
    SMeshConstruct(
        s32 MeshSegmentsVert, s32 MeshSegmentsHorz, f32 MeshRadiusInner, f32 MeshRadiusOuter,
        f32 SpiralRotationDegree, f32 SpiralRotationDistance, bool MeshHasCap = true,
        const video::EShadingTypes DefShading = video::SHADING_GOURAUD
    );
    ~SMeshConstruct();
    
    /* Functions */
    
    void checkDefaultSegments(const EBasicMeshes Model, s32 &Segments);
    void checkDefaultShading(const EBasicMeshes Model);
    
    /* === Members === */
    
    // Standard members
    s32 SegmentsVert;               //!< Vertical segments count (main segments indication, when only one segments can be used).
    s32 SegmentsHorz;               //!< Horizontal segments count.
    f32 RadiusInner;                //!< Inner radius (main radius indication, when only one radius can be used).
    f32 RadiusOuter;                //!< Outer radius.
    bool HasCap;                    //!< Specifies if the mesh has a cap (only used for Cone, Cylinder, Spiral, Pipe and Disk).
    video::EShadingTypes Shading;   //!< Default shading type.
    
    // Extended spiral members
    f32 RotationDegree;             //!< Rotation degree for a spiral.
    f32 RotationDistance;           //!< Rotation distance for a spiral.
    
    // Torusknot
    // !TODO ...
};


/**
This is the main class for generating procedural 3D models such as cube, sphere, spiral etc.
\todo Change this to a namespace and rename it to "MeshGenerator"
*/
class SP_EXPORT BasicMeshGenerator
{
    
    public:
        
        BasicMeshGenerator();
        ~BasicMeshGenerator();
        
        /* Functions */
        
        /**
        Creates one of the standard primitives (or rather basic meshes).
        \param Object: Pointer to the Mesh object that is to get the new surface.
        \param Model: Type of basic mesh.
        \param BuildConstruct: Structure with all the construction settings.
        */
        void createMesh(Mesh* Object, const EBasicMeshes Model, const SMeshConstruct &BuildConstruct);
        
        /**
        Creates a "super shape" with the specified control values.
        If you don't know how to set your values use the "getSuperShapeModel" function first.
        */
        void createSuperShape(Mesh* Object, const f32 ValueList[12], const s32 Segments = 20);
        
        /**
        Gets one of the predefined super shape values.
        \param Type: Type of the predefined super shape.
        \param ValueList: Resulting array with the super shape values.
        */
        void getSuperShapeModel(const ESuperShapeModels Type, f32 ValueList[12]);
        
        /**
        Creates a Bézier patch 3D model.
        \param AnchorPoints: Array of 4*4 3D points which hold the patch construction data.
        \param Segments: Count of segments for the patch. Segments ^ 2 quads (quad -> two triangles) will be created.
        \return Pointer to the new Entity object.
        */
        Mesh* createBezierPatch(
            const dim::vector3df AnchorPoints[4][4], const s32 Segments = 10, bool isFrontFace = true
        );
        
        /**
        Creates a Bézier patch surface. This function is used in the "createBezierPatch" function.
        \param Mesh: Entity object which is to get the patch.
        \param Surface: Surface which is to be used.
        */
        void createBezierPatchFace(
            Mesh* Mesh, const u32 Surface,
            const dim::vector3df AnchorPoints[4][4], const s32 Segments = 10, bool isFrontFace = true
        );
        
        /**
        Creates a skybox mesh.
        \param TextureList: Array of 6 textures.
        \param Radius: Radius (or rather size) of the skybox.
        \param Type: Mapping type of the skybox.
        \return Pointer to the new Entity object.
        */
        Mesh* createSkyBox(video::Texture* TextureList[6], f32 Radius = 50.0f);
        
        /**
        Creates a height field.
        \param TexHeightMap: Height map texture where the height field data will be sampled.
        \param Segments: Count of segments. The Mesh object will have Segments^2 quads (or rather 2*Segments^2 triangles).
        \return Pointer to the new Entity object.
        */
        Mesh* createHeightField(const video::Texture* TexHeightMap, const s32 Segments);
        
    private:
        
        /* === Functions === */
        
        #ifdef SP_COMPILE_WITH_PRIMITIVE_CUBE
        void createCube();
        #endif
        #ifdef SP_COMPILE_WITH_PRIMITIVE_CONE
        void createCone();
        #endif
        #ifdef SP_COMPILE_WITH_PRIMITIVE_CYLINDER
        void createCylinder();
        #endif
        #ifdef SP_COMPILE_WITH_PRIMITIVE_SPHERE
        void createSphere();
        #endif
        #ifdef SP_COMPILE_WITH_PRIMITIVE_ICOSPHERE
        void createIcosphere();
        #endif
        #ifdef SP_COMPILE_WITH_PRIMITIVE_TORUS
        void createTorus();
        #endif
        #ifdef SP_COMPILE_WITH_PRIMITIVE_TORUSKNOT
        void createTorusknot();
        #endif
        #ifdef SP_COMPILE_WITH_PRIMITIVE_PIPE
        void createPipe();
        #endif
        #ifdef SP_COMPILE_WITH_PRIMITIVE_SPIRAL
        void createSpiral();
        #endif
        #ifdef SP_COMPILE_WITH_PRIMITIVE_PLANE
        void createPlane();
        #endif
        #ifdef SP_COMPILE_WITH_PRIMITIVE_DISK
        void createDisk();
        #endif
        #ifdef SP_COMPILE_WITH_PRIMITIVE_CUBOCTAHEDRON
        void createCuboctahedron();
        #endif
        #ifdef SP_COMPILE_WITH_PRIMITIVE_TETRAHEDRON
        void createTetrahedron();
        #endif
        #ifdef SP_COMPILE_WITH_PRIMITIVE_OCTAHEDRON
        void createOctahedron();
        #endif
        #ifdef SP_COMPILE_WITH_PRIMITIVE_DODECAHEDRON
        void createDodecahedron();
        #endif
        #ifdef SP_COMPILE_WITH_PRIMITIVE_ICOSAHEDRON
        void createIcosahedron();
        #endif
        #ifdef SP_COMPILE_WITH_PRIMITIVE_TEAPOT
        void createTeapot();
        #endif
        
        #ifdef SP_COMPILE_WITH_PRIMITIVE_WIRE_CUBE
        void createWireCube();
        #endif
        
        #ifdef SP_COMPILE_WITH_PRIMITIVE_SUPERSHAPE
        void createSuperShapeSurface(const f32 ValueList[12], const s32 Segments);
        f32 computeSuperShapeNextFrame(f32 m, f32 n1, f32 n2, f32 n3, f32 a, f32 b, f32 phi);
        #endif
        
        void addVertex(
            f32 x, f32 y, f32 z,
            f32 u = 0.0f, f32 v = 0.0f, f32 w = 0.0f
        );
        
        void addFace(u32 v0, u32 v1, u32 v2);
        void addFace(u32 v0, u32 v1, u32 v2, u32 v3);
        void addFace(u32 v0, u32 v1, u32 v2, u32 v3, u32 v4);
        
        void addFace(
            const dim::vector3df &v0, const dim::vector3df &v1, const dim::vector3df &v2,
            const dim::point2df &t0, const dim::point2df &t1, const dim::point2df &t2
        );
        void addFace(
            const dim::vector3df &v0, const dim::vector3df &v1, const dim::vector3df &v2
        );
        
        void addFace(
            const dim::vector3df* Vertices, const dim::point2df* TexCoords,
            u32 v0, u32 v1, u32 v2, u32 v3, u32 v4
        );
        
        void addQuadFace(
            dim::vector3df v0, dim::point2df t0, dim::vector3df v1, dim::point2df t1,
            dim::vector3df diru, dim::vector3df dirv,
            bool FaceLinkCCW = false
        );
        
        /* === Members === */
        
        Mesh* Mesh_;
        video::MeshBuffer* Surface_;
        
        SMeshConstruct BuildConstruct_;
        
};


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
