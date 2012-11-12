/*
 * Mesh generator header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_MESH_GENERATOR_H__
#define __SP_MESH_GENERATOR_H__


#include "Base/spStandard.hpp"
#include "Base/spMath.hpp"
#include "Base/spMathRandomizer.hpp"
#include "Base/spMeshBuffer.hpp"
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


namespace MeshGenerator
{

/**
Universal mesh creation function.
\param[out] Surface Specifies the mesh buffer (or rather surface) which is to be created.
It will be cleared before the mesh is constructed.
\param[in] Model Specifies the standard model type.
\param[in] Construct Describes the mesh construction. This contains all information for any kind of basic meshes:
inner- and outer radius, segmentation and other options.
\see EBasicMeshes
*/
SP_EXPORT void createMesh(
    video::MeshBuffer &Surface, const EBasicMeshes Model, SMeshConstruct Construct = SMeshConstruct()
);

SP_EXPORT void createCube           (video::MeshBuffer &Surface, f32 Radius, s32 SegmentsVert, s32 SegmentsHorz);
SP_EXPORT void createCone           (video::MeshBuffer &Surface, f32 Radius = 0.5f, f32 Height = 1.0f, s32 Segments = 20, bool HasCap = true);
SP_EXPORT void createCylinder       (video::MeshBuffer &Surface, f32 Radius = 0.5f, f32 Height = 1.0f, s32 Segments = 20, bool HasCap = true);
//SP_EXPORT void createCapsule        (video::MeshBuffer &Surface, f32 Radius = 0.5f, f32 Height = 1.0f, s32 Segments = 10);
SP_EXPORT void createSphere         (video::MeshBuffer &Surface, f32 Radius = 0.5f, s32 Segments = 10);
SP_EXPORT void createIcoSphere      (video::MeshBuffer &Surface, f32 Radius = 0.5f, s32 Segments = 3);
SP_EXPORT void createTorus          (video::MeshBuffer &Surface, f32 RadiusOuter = 0.5f, f32 RadiusInner = 0.25f, s32 Segments = 10);
SP_EXPORT void createTorusknot      (video::MeshBuffer &Surface, f32 RadiusOuter = 0.5f, f32 RadiusInner = 0.25f, s32 Segments = 10);
SP_EXPORT void createSpiral         (video::MeshBuffer &Surface, f32 RadiusOuter = 0.5f, f32 RadiusInner = 0.25f, f32 TwirlDegree = 360.0f, f32 TwirlDistance = 1.0f, s32 Segments = 10, bool HasCap = true);
SP_EXPORT void createPipe           (video::MeshBuffer &Surface, f32 RadiusOuter = 0.5f, f32 RadiusInner = 0.25f, f32 Height = 1.0f, s32 Segments = 20, bool HasCap = true);
SP_EXPORT void createPlane          (video::MeshBuffer &Surface, s32 SegmentsVert, s32 SegmentsHorz);
SP_EXPORT void createDisk           (video::MeshBuffer &Surface, f32 RadiusOuter = 0.5f, f32 RadiusInner = 0.25f, s32 Segments = 20, bool HasHole = true);
SP_EXPORT void createCuboctahedron  (video::MeshBuffer &Surface);
SP_EXPORT void createTetrahedron    (video::MeshBuffer &Surface);
SP_EXPORT void createOctahedron     (video::MeshBuffer &Surface);
SP_EXPORT void createDodecahedron   (video::MeshBuffer &Surface);
SP_EXPORT void createIcosahedron    (video::MeshBuffer &Surface);
SP_EXPORT void createTeapot         (video::MeshBuffer &Surface);

SP_EXPORT void createWireCube(scene::Mesh &MeshObj, f32 Radius = 0.5f);

/**
Creates a skybox mesh.
\param[out] MeshObj Specifies the mesh object which is to be constructed.
\param[in] TextureList Array of 6 textures.
\param[in] Radius Radius (or rather size) of the skybox.
*/
SP_EXPORT void createSkyBox(scene::Mesh &MeshObj, video::Texture* (&TextureList)[6], f32 Radius);

inline void createCube(video::MeshBuffer &Surface, f32 Radius = 0.5f, s32 Segments = 1)
{
    createCube(Surface, Radius, Segments, Segments);
}

inline void createPlane(video::MeshBuffer &Surface, s32 Segments = 1)
{
    createPlane(Surface, Segments, Segments);
}

//SP_EXPORT void createBatchingMesh(BatchingMesh &Object, const std::list<Mesh*> &MeshList);
//SP_EXPORT void createBatchingMesh(BatchingMesh &Object, const std::list<video::MeshBuffer*> &SurfaceList);

} // /namespace MeshGenerator

#if 1 //!!!

/**
This is the main class for generating procedural 3D models such as cube, sphere, spiral etc.
\todo Change this to a namespace and rename it to "MeshGenerator"
\deprecated
*/
class SP_EXPORT BasicMeshGenerator
{
    
    public:
        
        BasicMeshGenerator();
        ~BasicMeshGenerator();
        
        /* Functions */
        
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
        Creates a height field.
        \param TexHeightMap: Height map texture where the height field data will be sampled.
        \param Segments: Count of segments. The Mesh object will have Segments^2 quads (or rather 2*Segments^2 triangles).
        \return Pointer to the new Entity object.
        */
        Mesh* createHeightField(const video::Texture* TexHeightMap, const s32 Segments);
        
    private:
        
        /* === Functions === */
        
        #ifdef SP_COMPILE_WITH_PRIMITIVE_SUPERSHAPE
        void createSuperShapeSurface(const f32 ValueList[12], const s32 Segments);
        f32 computeSuperShapeNextFrame(f32 m, f32 n1, f32 n2, f32 n3, f32 a, f32 b, f32 phi);
        #endif
        
        /* === Members === */
        
        Mesh* Mesh_;
        video::MeshBuffer* Surface_;
        
        SMeshConstruct BuildConstruct_;
        
};

#endif


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
