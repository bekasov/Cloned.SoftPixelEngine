/*
 * Mesh scene node header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_SCENE_MESH_H__
#define __SP_SCENE_MESH_H__


#include "Base/spStandard.hpp"
#include "Base/spInputOutputString.hpp"
#include "Base/spMaterialStates.hpp"
#include "Base/spMeshBuffer.hpp"
#include "Base/spViewFrustum.hpp"
#include "Base/spTreeNodeQuad.hpp"
#include "Base/spTreeNodeOct.hpp"
#include "Base/spTreeNodeBSP.hpp"
#include "SceneGraph/spMaterialNode.hpp"
#include "RenderSystem/spTextureBase.hpp"

#include <limits.h>
#include <vector>
#include <list>
#include <map>


namespace sp
{
namespace scene
{


/*
 * Macros
 */

static const u32 MESHBUFFER_IGNORE = UINT_MAX; //!< General value to use each Mesh's surface.


/*
 * Pre-declarations
 */

class Mesh;
class Camera;
class SceneGraph;

struct SPickingObject;
struct SCollisionObject;


/*!
 * Meshes represents the main 3D objects which are also called "Entity", "Model" or just "3D Object".
 * A Mesh consists basically of information about its location (position, rotation, scaling),
 * a material (see video::SMeshMaterial) with its color, shader etc. and multiple surfaces which form
 * the 3D data (vertices, triangles, textures). The way to create a 3D model is the following:
 * create a Mesh, add one or more surfaces, add vertices to the surface, add triangles to the
 * surface and finish! Don't forget to update the mesh buffer using the "updateMeshBuffer" function.
 * Then you can add textures on it. Actually there are only surfaces to use multiple textures which shall not
 * lie one upon the other (that would be multi-texturing). But each surface uses the same Mesh's material.
 * Consider that not only a large count of triangle can be a performance problem but also the count of surfaces and entities!
 * 
 * Example code:
 * \code
 * // Create the mesh
 * scene::Mesh* Obj = spSmngr->createMesh();
 * 
 * // Create a new mesh buffer
 * video::MeshBuffer* Shape = Obj->createMeshBuffer();
 * 
 * // Add a texture
 * Shape->addTexture(spDriver->loadTexture("ExampleTexture.jpg"));
 * 
 * // Add three vertices
 * Shape->addVertex(dim::vector3df( 0.0f,  1.0f, 0.0f), dim::point2df(0.5f, 0.0f), video::color(255, 0, 0)); // Vertex 0
 * Shape->addVertex(dim::vector3df( 1.0f, -1.0f, 0.0f), dim::point2df(1.0f, 1.0f), video::color(0, 255, 0)); // Vertex 1
 * Shape->addVertex(dim::vector3df(-1.0f, -1.0f, 0.0f), dim::point2df(0.0f, 1.0f), video::color(0, 0, 255)); // Vertex 2
 * 
 * // Add a triangle
 * Shape->addTriangle(0, 1, 2);
 * 
 * // Update vertex- and index buffer after creating the geometry
 * Shape->updateMeshBuffer();
 * \endcode
 */
class SP_EXPORT Mesh : public MaterialNode
{
    
    public:
        
        Mesh();
        virtual ~Mesh();
        
        /* === Texturing === */
        
        //! Adds the specifies texture to all mesh buffers.
        void addTexture(video::Texture* Tex, const u8 Layer = video::TEXTURE_IGNORE);
        
        /**
        Computes the texture coordinates automatically by the triangles' normal (typically used for lightmap texturing).
        \param[in] Layer Layer/ level which shall be used.
        \param[in] Density Factor which will be multiplied with the space coordinates to change it into texture coordinates.
        \param[in] MeshBufferIndex Specifies the index for the mesh buffer which is to be used. By default MESHBUFFER_IGNORE to use all.
        \param[in] GlobalProjection Specifies whether texture coordinate projection is to be done global or local. By default global.
        \param[in] AllowNegativeTexCoords Specifies whether negative texture coordinates are allowed. By default false.
        When you are using any normal-mapping techniques, you should avoid negative texture coordinates!
        */
        void textureAutoMap(
            const u8 Layer, const f32 Density = 1.0, const u32 MeshBufferIndex = MESHBUFFER_IGNORE,
            bool GlobalProjection = true, bool AllowNegativeTexCoords = false
        );
        
        //! Returns a list with all textures of all mesh buffers.
        std::list<video::Texture*> getTextureList() const;
        //! Returns count of textures of the specified surface.
        u32 getTextureCount() const;
        
        /* === Mesh buidling === */
        
        /**
        Updates the normal vectors for each mesh buffer.
        \see {video::MeshBuffer}
        */
        virtual void updateNormals();
        
        /**
        Updates the tangent space for each mesh buffer.
        \see {video::MeshBuffer}
        */
        void updateTangentSpace(
            const u8 TangentLayer = video::TEXTURE_IGNORE, const u8 BinormalLayer = video::TEXTURE_IGNORE, bool UpdateNormals = true
        );
        
        //! Updates each vertex buffer.
        void updateVertexBuffer();
        //! Updates each index buffer.
        void updateIndexBuffer();
        //! Updates each vertex- and index buffer.
        void updateMeshBuffer();
        
        /**
        Translates each vertex to the specified direction.
        \param Direction: Direction in which the mesh shall be moved.
        \param Surface: Affected surface(s).
        */
        void meshTranslate(const dim::vector3df &Direction);
        void meshTransform(const dim::vector3df &Size);
        void meshTransform(const dim::matrix4f &Matrix);
        void meshTurn(const dim::vector3df &Rotation);
        
        //! Flips the mesh. i.e. each vertex's coordinate will be inverted.
        void meshFlip();
        void meshFlip(bool isXAxis, bool isYAxis, bool isZAxis);
        
        //! \see MeshModifier::meshFit
        void meshFit(const dim::vector3df &Position, const dim::vector3df &Size);
        //! \see MeshModifier::meshSpherify
        void meshSpherify(f32 Factor);
        //! \see MeshModifier::meshTwist
        void meshTwist(f32 Rotation);
        
        /**
        Merges each child mesh as a new surface to this mesh object. This only works if the children are specified
        in the node's children list. Use "Node::addChild" or "Node::addChildren".
        \param isDeleteChildren: If true each child mesh will be deleted after the merging process (by default true).
        */
        void mergeFamily(bool isDeleteChildren = true);
        
        /**
        Centers the mesh's origin. After this operation the mesh's origin (vector [0|0|0]) is in the middle of the model.
        \return Displacement vector.
        */
        dim::vector3df centerOrigin();
        
        //! Clips (or rather seperates) concatenated triangles for each mesh buffer. After calling this function each triangle has its own vertices.
        void clipConcatenatedTriangles();
        
        /**
        Flips each mesh's triangle. Each triangle's indices A and C are swapping their value.
        e.g. when a triangle has the indices (0, 1, 2) after flipping it has the indices (2, 1, 0).
        */
        void flipTriangles();
        
        /**
        Gives you the mesh's bounding box which is formed by each vertex-coordinate. There is still a function called
        "getBoundingBox" in the Node class which gives you user defined bounding box for frustum-culling.
        \param Min: Vector which will receive the minimal point of the bounding box.
        \param Max: Vector which will receive the maximal point of the bounding box.
        \param isGlobal: Specifies whether the coordinates are to be transformed by the object's matrix or not.
        \return True if a valid bounding box could be created. Otherwise the mesh has no vertices.
        */
        bool getMeshBoundingBox(dim::vector3df &Min, dim::vector3df &Max, bool isGlobal = false) const;
        //! Returns dimension of the mesh AABB (axis-aliend-bounding-box).
        dim::aabbox3df getMeshBoundingBox(bool isGlobal = false) const;
        //! Returns radius of the mesh bounding sphere.
        f32 getMeshBoundingSphere(bool isGlobal = false) const;
        
        /* === LOD sub meshes (level-of-detail optimization) === */
        
        /**
        Adds a LOD (level-of-detail) sub mesh.
        \param LODSubMesh: Mesh object which shall be added to the LOD list.
        When LOD is enabled the base mesh (this Mesh object) is the first object and has to be the best detailed mesh.
        \param isCopyMaterials: If true the "LODSubMesh" object's materials are copied to these of this Mesh object.
        */
        void addLODSubMesh(Mesh* LODSubMesh, bool isCopyMaterials = false);
        //! Clears the LOD (level-of-detail) sub mesh list.
        void clearLODSubMeshes();
        
        //! Sets the LOD (level-of-detail) sub meshes list directly.
        void setLODSubMeshList(const std::vector<Mesh*> &LODSubMeshList);
        
        /**
        Sets the LOD distance.
        \param Distance: Specifies the distance which shall be used for LOD computing. Only linear and not exponentially yet.
        */
        void setLODDistance(f32 Distance);
        
        //! Enables or disables the LOD (level-of-detail) management.
        void setLOD(bool Enable);
        
        /* === Mesh buffers === */
        
        /**
        Creates a new video::MeshBuffer object. This object can hold 3D geometry data.
        \param VertexFormat: Specifies the initial vertex format. By default video::VertexFormatDefault (can be used with RenderSystem::getVertexFormatDefault()).
        \param IndexFormat: Specifies the initial index format. By default ATTRIBUTE_USNIGNED_SHORT.
        \return Pointer to the new video::MeshBuffer object.
        */
        video::MeshBuffer* createMeshBuffer(
            const video::VertexFormat* VertexFormat = 0,
            const video::ERendererDataTypes IndexFormat = video::DATATYPE_UNSIGNED_SHORT
        );
        
        //! Deletes the specified mesh buffer.
        void deleteMeshBuffer(const u32 Index);
        
        //! Deletes all mesh buffers.
        void deleteMeshBuffers();
        
        /**
        Merges the given mesh, i.e. all mesh buffers which are equal in its structure (textures, vertex- and index format) are summarized.
        \see MeshBuffer::sortCompare
        \see MeshBuffer::compare
        */
        void mergeMeshBuffers();
        
        /**
        Optimizes the mesh vertices order depending on their transparency. Most transparent vertices will be placed at the end
        and less transparent vertices at the begin. This function can be used to avoid the overlapping problem with alpha-blending.
        \todo This is incomplete
        */
        void optimizeTransparency(); // !!! TODO !!!
        
        /* === Vertices === */
        
        //! Returns count of vertices for the specified surface.
        u32 getVertexCount() const;
        //! Returns count of triangles for the specified surface.
        u32 getTriangleCount() const;
        
        //! Returns count of vertices for the specified surface.
        u32 getOrigVertexCount() const;
        //! Returns count of triangles for the specified surface.
        u32 getOrigTriangleCount() const;
        
        /**
        Sets a mesh reference. This feature is very usfull when rendering multiple equal 3D models.
        This saves much memory and for animation much performance costs. e.g. very good for animated grass when
        you have thousands of Mesh objects which all refers to only one animated mesh. Each of these objecsts
        uses the same mesh buffer. Small memory costs and small animation performance costs but many movement in the scene.
        \param ReferenceMesh: Mesh object which holds the mesh buffer which is to be used.
        \param CopyLocation: If true the location is copied from the "ReferenceEntity" object.
        \param CopyMaterial: If true the material settings are copied from the "ReferenceEntity" object.
        */
        void setReference(Mesh* ReferenceMesh, bool CopyLocation = false, bool CopyMaterial = false);
        
        Mesh* getReference();
        const Mesh* getReference() const;
        
        /**
        Paints the whole mesh witht he specified color.
        \param Color: Specifies the color which is to be set.
        \param CombineColors: Specifies whether the color is to be multiplied with each vertex color or not.
        */
        void paint(const video::color &Color, bool CombineColors = false);
        
        /**
        Sets the shading type.
        \param Type: Type of shading (flat, gouraud, phong, per-pixel).
        \param UpdateImmediate: If true the normals will be updated immediately (updateNormals).
        \see MaterialNode::setShading
        */
        void setShading(const video::EShadingTypes Type, bool UpdateImmediate = false);
        
        /* === Rendering & copying === */
        
        /**
        Copies the mesh. Normally used inside the SceneManager but can also be used manual.
        \param other: Specifies the mesh where the data of this mesh shall be stored.
        */
        void copy(const Mesh* Other);
        
        //! Returns a pointer to a new Mesh object which has been copied by this Mesh.
        Mesh* copy() const;
        
        /**
        Renders the whole mesh. This function is called in the "renderScene" function of the SceneManager.
        Here the object-transformation, frustum-culling, level of detail and drawing the surfaces' mesh buffers are proceeded.
        */
        virtual void render();
        
        /* === Inline functions === */
        
        //! Returns the specified video::MeshBuffer object.
        inline video::MeshBuffer* getMeshBuffer(const u32 Index)
        {
            return Index < SurfaceList_->size() ? (*SurfaceList_)[Index] : 0;
        }
        //! Returns the specified constant video::MeshBuffer object.
        inline const video::MeshBuffer* getMeshBuffer(const u32 Index) const
        {
            return Index < SurfaceList_->size() ? (*SurfaceList_)[Index] : 0;
        }
        
        //! Returns the specified original video::MeshBuffer object.
        inline video::MeshBuffer* getOrigMeshBuffer(const u32 Index)
        {
            return Index < OrigSurfaceList_.size() ? OrigSurfaceList_[Index] : 0;
        }
        //! Returns the specified constant original video::MeshBuffer object.
        inline const video::MeshBuffer* getOrigMeshBuffer(const u32 Index) const
        {
            return Index < OrigSurfaceList_.size() ? OrigSurfaceList_[Index] : 0;
        }
        
        //! Returns the last video::MeshBuffer object.
        inline video::MeshBuffer* getLastMeshBuffer()
        {
            return !OrigSurfaceList_.empty() ? OrigSurfaceList_[OrigSurfaceList_.size() - 1] : 0;
        }
        //! Returns the last constant video::MeshBuffer object.
        inline const video::MeshBuffer* getLastMeshBuffer() const
        {
            return !OrigSurfaceList_.empty() ? OrigSurfaceList_[OrigSurfaceList_.size() - 1] : 0;
        }
        
        //! Returns cound of mesh buffers.
        inline u32 getMeshBufferCount() const
        {
            return SurfaceList_->size();
        }
        //! Returns cound of mesh buffers.
        inline u32 getOrigMeshBufferCount() const
        {
            return OrigSurfaceList_.size();
        }
        
        //! Returns the mesh buffer (or rather surface) list. Useful for iterating with boost/foreach.
        inline const std::vector<video::MeshBuffer*>& getMeshBufferList() const
        {
            return *SurfaceList_;
        }
        
        //! Sets the render callback. By default 0.
        inline void setRenderCallback(const video::UserRenderCallback &RenderCallback)
        {
            UserRenderProc_ = RenderCallback;
        }
        
        //! Returns list of the LOD (level-of-detail) sub meshes.
        inline const std::vector<Mesh*>& getLODSubMeshList() const
        {
            return LODSubMeshList_;
        }
        //! Returns LOD distance.
        inline f32 getLODDistance() const
        {
            return LODSubMeshDistance_;
        }
        //! Returns status of the LOD management.
        inline bool getLOD() const
        {
            return UseLODSubMeshes_;
        }
        
        /**
        Returns true if this mesh is an instance of another mesh, i.e. "setReference" was used.
        \see setReference
        */
        inline bool isInstanced() const
        {
            return Reference_ != 0;
        }
        
    protected:
        
        friend class SceneGraph;
        friend class SimpleSceneManager;
        friend struct SCollisionSystemObject;
        friend struct SPickingObject;
        friend struct SCollisionObject;
        
        friend bool cmpObjectMeshes(Mesh* &obj1, Mesh* &obj2);
        
        /* === Functions === */
        
        u32 updateLevelOfDetail();
        
        void copyMesh(Mesh* NewMesh) const;
        
        /* === Members === */
        
        std::vector<video::MeshBuffer*> OrigSurfaceList_;
        std::vector<video::MeshBuffer*> * SurfaceList_, * LODSurfaceList_;
        
        bool UseLODSubMeshes_;
        f32 LODSubMeshDistance_;
        std::vector<Mesh*> LODSubMeshList_;
        
        Mesh* Reference_;
        
        video::UserRenderCallback UserRenderProc_;
        
};


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
