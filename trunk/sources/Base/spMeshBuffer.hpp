/*
 * Mesh buffer header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_HWBUFFER_MESHBUFFER_H__
#define __SP_HWBUFFER_MESHBUFFER_H__


#include "Base/spStandard.hpp"
#include "Base/spInputOutputString.hpp"
#include "Base/spGeometryStructures.hpp"
#include "Base/spMaterialStates.hpp"
#include "Base/spVertexFormat.hpp"
#include "Base/spIndexFormat.hpp"
#include "Base/spMathTriangleCutter.hpp"

#include <vector>


namespace sp
{
namespace video
{


/**
This is the hardware mesh buffer class (also called a "Surface"). Containing a vertex- and index buffer. Since version 2.2 each vertex mesh manipulation
such as texturing, vertex- or triangle access is handled by this class.
*/
class SP_EXPORT MeshBuffer
{
    
    public:
        
        MeshBuffer(const video::VertexFormat* VertexFormat = 0, ERendererDataTypes IndexFormat = DATATYPE_UNSIGNED_INT);
        MeshBuffer(const MeshBuffer &Other, bool isCreateMeshBuffer = true);
        virtual ~MeshBuffer();
        
        /* === Buffer functions === */
        
        //! Returns mesh buffer identifier (e.g. 'mesh buffer "test"' or 'anonymous mesh buffer').
        virtual io::stringc getIdentifier() const;
        
        //! Returns pointer to the surface reference object if set otherwise zero.
        MeshBuffer* getReference();
        //! Returns constant pointer to the surface reference object if set otherwise zero.
        const MeshBuffer* getReference() const;
        
        /**
        Compares this mesh buffer with the given mesh buffer on their surfaces.
        This is used to sort and then merge a list of mesh buffers.
        \return True if the surface (textures, vertex- and index format) of this mesh buffer and the given mesh buffer are unequal.
        If they are euqal the return value is false.
        \note The return value is false if the surfaces are equal. That is because in that case the sort algorithm don't exchange
        the current two elements in the list.
        \see compare
        */
        bool sortCompare(const MeshBuffer &Other) const;
        
        /**
        Compares this mesh buffer with the given mesh buffer.
        \return True if the surface (textures, vertex- and index format) of this mesh buffer and the given mesh buffer are equal.
        \see sortCompare
        */
        bool compare(const MeshBuffer &Other) const;
        
        /**
        Sets the new vertex format. This function is very slow and should never be used inside a render loop.
        Software as well as hardware vertex buffer will be recreated.
        \param Format: Specifies the new vertex format. If it's null this call has no effect.
        The default vertex format is an instance of the class "DefaultVertexFormat".
        The previous vertex format will not be deleted. If you set a new vertex format
        you need to delete it self when your program ends!
        */
        void setVertexFormat(const VertexFormat* Format);
        
        /**
        Sets the new index format. This function is very slow and should never be used inside a render loop.
        Software as well as hardware index buffer will be recreated.
        \param Format: Specifies the new index format. Only the following values are valid:
        ATTRIBUTE_UNSIGNED_BYTE, ATTRIBUTE_UNSIGNED_SHORT and ATTRIBUTE_USNIGNED_INT.
        The default value is ATTRIBUTE_USNIGNED_INT.
        */
        void setIndexFormat(ERendererDataTypes Format);
        
        //! Save backup from the current mesh buffer. This can be useful before modifying the vertex- or index format.
        void saveBackup();
        //! Load backup to the current mesh buffer.
        void loadBackup();
        //! Clear the mesh buffer backup.
        void clearBackup();
        
        //! Creates the hardware vertex buffer.
        void createVertexBuffer();
        //! Creates the hardware index buffer.
        void createIndexBuffer();
        //! Creates the hardware vertex- and index buffer.
        void createMeshBuffer();
        
        //! Deletes the hardware vertex buffer.
        void deleteVertexBuffer();
        //! Deletes the hardware index buffer.
        void deleteIndexBuffer();
        //! Deletes the hardware vertex- and index buffer.
        void deleteMeshBuffer();
        
        //! Updates the hardware vertex buffer or rather uploads the vertex buffer into VRAM.
        void updateVertexBuffer();
        //! Updates the hardware index buffer or rather uploads the index buffer into VRAM.
        void updateIndexBuffer();
        //! Updates the hardware vertex- and index buffer.
        void updateMeshBuffer();
        
        //! Updates the hardware vertex buffer only for the specified element.
        void updateVertexBufferElement(u32 Index);
        //! Updates the hardware index buffer only for the specified element.
        void updateIndexBufferElement(u32 Index);
        
        /**
        Sets the primitive type. By default PRIMITIVE_TRIANGLES. There are some types which are only supported
        by OpenGL which are: PRIMITIVE_LINE_LOOP, PRIMITIVE_QUADS, PRIMITIVE_QUAD_STRIP and PRIMITIVE_POLYGON.
        */
        void setPrimitiveType(const ERenderPrimitives Type);
        
        /**
        Returns the primitive size. If the primitive type is PRIMITIVE_TRIANGLES the size is 3. If the primitive type
        is PRIMITIVE_QUADS the size is 4 etc.
        \param Type: Specifies the primitive type.
        */
        static s32 getPrimitiveSize(const ERenderPrimitives Type);
        
        //! Returns the primitive size (see the static version of getPrimitiveSize).
        s32 getPrimitiveSize() const;
        
        /**
        Returns true if rendering this mesh buffer has any effect. False otherwise when the count of indices are 0
        or index buffer is used but count of vertices are 0.
        \note A mesh buffer is also only renderable if "updateVertexBuffer" or "updateMeshBuffer" has been called previously at least once.
        When the index buffer is used "updateIndexBuffer" or "updateMeshBuffer" must have been called as well.
        */
        bool renderable() const;
        
        /* === Mesh buffer manipulation functions */
        
        /**
        Adds a new vertex to the vertex buffer. A vertex can be seen as a point
        in 3D space which also have some further information such as color, normal, texture coordinates etc.
        */
        u32 addVertex();
        
        //! Adds the specified amount of new vertices to the vertex buffer.
        void addVertices(const u32 Count);
        
        /**
        \param Position: 3D coordiante. A left-handed coordinate system is used like in the whole engine.
        \param TexCoord: Texture coordinate which is used for each texture layer.
        \param Color: Specifies the vertex color.
        \return Vertex index (beginning with 0).
        */
        u32 addVertex(
            const dim::vector3df &Position,
            const dim::vector3df &TexCoord = 0,
            const video::color &Color = 255
        );
        
        /**
        \param Normal: Normal for the vertex. This normal vector will not be normalized.
        \param Fog: Fog coordinate. This is only supported with OpenGL.
        */
        u32 addVertex(
            const dim::vector3df &Position,
            const dim::vector3df &Normal,
            const dim::vector3df &TexCoord,
            const video::color &Color = 255,
            const f32 Fog = 0.0f
        );
        
        //! \param TexCoordList: List of all texture coordinates.
        u32 addVertex(
            const dim::vector3df &Position,
            const dim::vector3df &Normal,
            const std::vector<dim::vector3df> &TexCoordList,
            const video::color &Color = 255,
            const f32 Fog = 0.0f
        );
        
        //! Adds a vertex with the specified data.
        u32 addVertex(const scene::SMeshVertex3D &VertexData);
        
        //! Removes the specified vertex.
        bool removeVertex(const u32 Index);
        
        /**
        Adds a new triangle to the index buffer. A triangle can be seen as a delta connection between
        three vertices. The indices of this triangle are all 0.
        */
        u32 addTriangle();
        
        //! Adds the specified amount of new triangles to the index buffer.
        void addTriangles(const u32 Count);
        
        /**
        Adds a new triangle to the index buffer. A triangle is nothing more than a delta connection between three vertices.
        \param VertexA: First vertex index. Affected by the VertexCounter.
        \param VertexB: Second vertex index. Affected by the VertexCounter.
        \param VertexC: Third vertex index. Affected by the VertexCounter.
        \return Triangle's index (beginning with 0). If any of the three given indices do not refer
        to an existing vertex the triangle will not be created and the return value is 0.
        */
        u32 addTriangle(u32 VertexA, u32 VertexB, u32 VertexC);
        
        //! Adds a new triangle to the index buffer with the specified indices.
        u32 addTriangle(const u32 Indices[3]);
        
        /**
        Adds a new quadrangle to the index buffer. A quadrangle is nothing more than a connection between four vertices.
        \param VertexA: First vertex index. Affected by the VertexCounter.
        \param VertexB: Second vertex index. Affected by the VertexCounter.
        \param VertexC: Third vertex index. Affected by the VertexCounter.
        \param VertexD: Fourth vertex index. Affected by the VertexCounter.
        \return Quad's index (beginning with 0). If any of the three given indices do not refer
        to an existing vertex the quadrangle will not be created and the return value is 0.
        */
        u32 addQuadrangle(u32 VertexA, u32 VertexB, u32 VertexC, u32 VertexD);
        
        //! Adds a new quadrangle to the index buffer with the specified indices.
        u32 addQuadrangle(const u32 Indices[4]);
        
        /**
        Adds a single index to the index buffer.
        \todo Change name to "addIndex"
        */
        u32 addPrimitiveIndex(u32 Index);
        //! Adds the specified amount of new indicies to the index buffer.
        void addIndices(const u32 Count);
        
        /**
        Removes the specified primitive. If the primitive type is PRIMITIVE_TRIANGLES three indices will be removed.
        If the primitive type is PRIMITIVE_QUADS four indices will be removed etc.
        \param Index: Specifies the primitive index. For triangles the index will be multiplied by 3, for quads it will
        be multiplied by four etc.
        \return True if the primitive was removed successful.
        */
        bool removePrimitive(const u32 Index);
        
        //! Deletes all vertices and all indices because the indices will became invalid.
        void clearVertices();
        //! Deletes all indices.
        void clearIndices();
        
        /**
        Inserts (or rather adds) the given mesh buffer to this mesh buffer, i.e. all vertices and indices will be copied.
        You still have to update the mesh buffer after inserting it.
        \return True if the new mesh buffer could be inserted successful. Otherwise false.
        \note You can only insert/ add mesh buffers when they have the same structure. use the "compare" function to check this equality.
        \see updateMeshBuffer
        \see compare
        */
        bool insertMeshBuffer(const MeshBuffer &Other);
        
        /**
        Sets the indices of the specified triangle.
        \param Index: Specifies the triangle index (position for the index buffer multiplied by 3).
        \param Indices: Array with the 3 indices which are to be set.
        */
        void setTriangleIndices(const u32 Index, const u32 (&Indices)[3]);
        
        /**
        Returns the indices of the specified triangle.
        \param Index: Specifies the triangle index (position for the index buffer multiplied by 3).
        \param Indices: Array where the 3 indices are to be stored.
        */
        void getTriangleIndices(const u32 Index, u32 (&Indices)[3]) const;
        
        /**
        Sets the vertex index for the specified primitive index.
        \param Index: Specifies the primitive index (e.g. when adding a triangle three indices will be added).
        \param VertexIndex: Specifies the new vertex index which is to be set.
        */
        void setPrimitiveIndex(const u32 Index, const u32 VertexIndex);
        
        /**
        Returns the vertex index of the specified primitive index.
        \param Index: Specifies the primitive index (e.g. when adding a triangle three indices will be added).
        \return Vertex index from the index buffer at the specified position.
        */
        u32 getPrimitiveIndex(const u32 Index) const;
        
        //! Returns an SMeshVertex3D structure with all available vertex information.
        scene::SMeshVertex3D getVertex(const u32 Index) const;
        //! Returns an SMeshTriangle3D structure with the three triangle indices.
        scene::SMeshTriangle3D getTriangle(const u32 Index) const;
        
        //! Returns the coordinates of the specified triangle.
        dim::triangle3df getTriangleCoords(const u32 Index) const;
        /**
        Returns the coordinates of the specified triangle as a reference.
        \deprecated
        */
        dim::ptriangle3df getTriangleReference(const u32 Index) const;
        
        //! Flips the first index with the last index of each triangle.
        void flipTriangles();
        
        //! Cuts the specified triangle with the specified clipping plane.
        bool cutTriangle(const u32 Index, const dim::plane3df &ClipPlane);
        
        /* === Vertex manipulation functions === */
        
        /**
        Sets the specifies vertex attribute data. This can be used when you want to set the data
        for a vertex attribute with an unconventional data type. Actually a vertex coordinate in the SPE
        is always a 3 component 32bit floating-point vector. If you created your own vertex format
        where the coordinate is stored in form of a 3 component 16bit integer vector you need to use this function
        to set the attribute data.
        \param Index: Specifies the vertex index.
        \param Attrib: Specifies the vertex attribute which is to be set (e.g. "myVertexFormat->getCoord()" or "myVertexFormat->getUniversals()[0]").
        \param AttribData: Pointer to the attribue data which is to be set (e.g. "&myVector.X").
        \param Size: Data size in bytes (e.g. "sizeof(myVector)").
        */
        void setVertexAttribute(const u32 Index, const SVertexAttribute &Attrib, const void* AttribData, u32 Size);
        
        /**
        Gets the specifies vertex attribute data.
        \param Index: Specifies the vertex index.
        \param Attrib: Specifies the vertex attribute. For more information see "setVertexAttribute".
        \param AttribData: Pointer to the buffer where the vertex attribute data is to be stored.
        \param Size: Data size in bytes of the "AttribData" buffer.
        */
        void getVertexAttribute(const u32 Index, const SVertexAttribute &Attrib, void* AttribData, u32 Size);
        
        /**
        Sets the specified vertex coordinate.
        \param Index: Specifies the vertex index.
        \param Coord: Specifies the vertex coordinate which is to be set.
        */
        void setVertexCoord(const u32 Index, const dim::vector3df &Coord);
        
        //! Returns the specified vertex coordinate.
        dim::vector3df getVertexCoord(const u32 Index) const;
        
        /**
        Sets the specified vertex normal. Normals are important for lighting calculations.
        \param Index: Specifies the vertex index.
        \param Normal: Specifies the normal vector which is to be set. This vector will not be normalized.
        The GPU will perform normalization in real-time. So normalize the vector before setting it
        or do it in your shader program.
        */
        void setVertexNormal(const u32 Index, const dim::vector3df &Normal);
        
        //! Returns the specified vertex normal.
        dim::vector3df getVertexNormal(const u32 Index) const;
        
        /**
        Sets the specified vertex tangent. Tangent vectors are used for normal mapping effects like bump- or parallax mapping.
        \param Index: Specifies the vertex index.
        \param Tangent: Specifies the tangent vector which is to be set. This vector will not be normalized.
        */
        void setVertexTangent(const u32 Index, const dim::vector3df &Tangent);
        
        //! Returns the specified vertex tangent.
        dim::vector3df getVertexTangent(const u32 Index) const;
        
        /**
        Sets the specified vertex binormal.
        \param Index: Specifies the vertex index.
        \param Binormal: Specifies the binormal vector which is to be set. This vector will not be normalized.
        */
        void setVertexBinormal(const u32 Index, const dim::vector3df &Binormal);
        
        //! Returns the specified vertex binormal.
        dim::vector3df getVertexBinormal(const u32 Index) const;
        
        /**
        Sets the specified vertex color.
        \param Index: Specifies the vertex index.
        \param Color: Specifies the vertex color which is to be set.
        */
        void setVertexColor(const u32 Index, const color &Color);
        
        //! Returns the specified vertex color.
        color getVertexColor(const u32 Index) const;
        
        /**
        Sets the specified vertex texture coordinate.
        \param Index: Specifies the vertex index.
        \param TexCoord: Specifies the texture coordinate which is to be set.
        \param Layer: Specifies the texture layer. By default TEXTURE_IGNORE which means that each layer will be used.
        */
        void setVertexTexCoord(const u32 Index, const dim::vector3df &TexCoord, const u8 Layer = TEXTURE_IGNORE);
        
        //! Returns the specified texture coordinate for the specified layer.
        dim::vector3df getVertexTexCoord(const u32 Index, const u8 Layer = 0) const;
        
        /**
        Sets the specified vertex fog coordinate. Fog coordinates are only visible with OpenGL if the fog is volumetric (FOG_VOLUMETRIC).
        \param Index: Specifies the vertex index.
        \param FogCoord: Specifies the fog coordinate which is to be set.
        */
        void setVertexFog(const u32 Index, const f32 FogCoord);
        
        //! Returns the specified fog coordinate.
        f32 getVertexFog(const u32 Index) const;
        
        /* === Mesh manipulation functions === */
        
        //! Updates each normal vector for flat- or gouraud shading.
        virtual void updateNormals(const EShadingTypes Shading = SHADING_GOURAUD);
        
        /**
        Updates the tangent space (i.e. tangent- and binormal vectors for each vertex).
        This function stores the computed vectors in the specified texture coordinates to use them
        in your shader programs. Direct3D11 can use the special tangent and binormal vectors.
        \param TangentLayer: Texture layer for the tangent vector.
        \param BinormalLayer: Texture layer for the binormal vector.
        \param UpdateNormals: Specifies whether the normal vectors are also to be updated or not.
        */
        void updateTangentSpace(
            const u8 TangentLayer = TEXTURE_IGNORE, const u8 BinormalLayer = TEXTURE_IGNORE, bool UpdateNormals = true
        );
        
        //! Translates each vertex coordinate in the specified direction.
        void meshTranslate(const dim::vector3df &Direction);
        //! Transforms each vertex coordinate by multiplying it with the specified size.
        void meshTransform(const dim::vector3df &Size);
        //! Transforms each vertex coordinate by multiplying it with the specified transformation matrix.
        void meshTransform(const dim::matrix4f &Matrix);
        //! Turns each vertex coordinate by rotating them with the specified rotation vector. This function performs a YXZ matrix rotation.
        void meshTurn(const dim::vector3df &Rotation);
        
        //! Flips the mesh. i.e. each vertex coordinate will be inverted.
        void meshFlip();
        //! Flips each vertex coordiante only for the specified axles.
        void meshFlip(bool isXAxis, bool isYAxis, bool isZAxis);
        
        //! Clips (or rather seperates) concatenated triangles for each mesh buffer.
        void clipConcatenatedTriangles();
        
        /**
        Paints each vertex with the specified color.
        \param Color: Specifies the color which is to be painted.
        \param CombineColors: Specifies whether the color is to be multiplied with each vertex color or not.
        */
        void paint(const video::color &Color, bool CombineColors = false);
        
        /* === Texture functions === */
        
        /**
        Adds a texture to the list.
        \param Tex: Texture which is to be mapped onto the mesh buffer.
        \param Layer: Specifies the layer (or level) in which the texture is to be mapped.
        Be aware of the layer you use when working with the texture of the mesh buffer.
        Each mesh buffer can hold a maximum of 8 textures (for multi-texturing) but layer is between 0 and 255 to sort the list.
        A value of "video::TEXTURE_IGNORE" (255) means the texture is put at the end of the list, i.e. it is mapped onto the mesh buffer at least.
        */
        void addTexture(Texture* Tex = 0, const u8 Layer = TEXTURE_IGNORE);
        
        /**
        Adds a surface texture to the list.
        \param Tex: Surface texture which is to be added.
        \param Layer: Specifies the texture layer (for more detail see the other "addTexture" function).
        */
        void addTexture(const SMeshSurfaceTexture &Tex, const u8 Layer = TEXTURE_IGNORE);
        
        //! Removes the texture of the specified layer.
        void removeTexture(const u8 Layer = TEXTURE_IGNORE);
        
        //! Removes the specified texture for each layer which holds it.
        void removeTexture(Texture* Tex);
        
        //! Clears the whole texture list.
        void clearTextureList();
        
        /**
        Translates each texture coordinate in the specified direction.
        \param Layer: Specifies the texture layer.
        \param Direction: 2D vector in which direction the texture coordinates are to be translated.
        */
        void textureTranslate(const u8 Layer, const dim::vector3df &Direction);
        
        /**
        Transforms each texture coordinate with the specified size.
        \param Layer: Specifies the texture layer.
        \param Size: Specifies the 2D factor which will be multiplied with each texture coordinate.
        */
        void textureTransform(const u8 Layer, const dim::vector3df &Size);
        
        /**
        Rotates each texture coordinate. This is a matrix rotation around the Z-Axis.
        \param Layer: Specifies the texture layer.
        \param Rotation: Specifies the rotation angle. This is in degree.
        To work with radian use the "math::Radian" class and call the "getDegree()" function.
        */
        void textureTurn(const u8 Layer, const f32 Rotation);
        
        /**
        Sets the new surface texture. With this function you can also change the texture settings such as texture matrix, mapping configuration etc.
        \param Layer: Specifies the texture layer.
        \param SurfaceTexture: SMeshSurfaceTexture structure with all texture settings information.
        */
        void setSurfaceTexture(const u8 Layer, const SMeshSurfaceTexture &SurfaceTex);
        
        //! Returns the surface texture of the specified layer.
        SMeshSurfaceTexture getSurfaceTexture(const u8 Layer) const;
        
        /**
        Sets the new texture.
        \param Layer: Specifies the texture layer.
        \param Tex: Pointer to the new Texture object (must not be 0).
        */
        void setTexture(const u8 Layer, Texture* Tex);
        
        //! Returns pointer to the Texture object of the specified layer.
        Texture* getTexture(const u8 Layer = 0) const;
        
        /**
        Sets the texture matrix.
        \param Layer: Specifies the texture layer.
        \param Matrix: New matrix transformation. This is a 4x4 matrix but a transformation for a 2D texture normally
        should be limited to X-/ Y axle transformations and Z axis rotations. For this useful are functions
        like "matrix4::setTextureRotation".
        */
        void setTextureMatrix(const u8 Layer, const dim::matrix4f &Matrix);
        
        //! Returns texture matrix of the specified layer.
        dim::matrix4f getTextureMatrix(const u8 Layer) const;
        
        /**
        Sets the texture environment type. Usefull for detailmaps for example (video::TEXENV_ADDSIGNED).
        It configures how the texels shall be rendered onto the mesh and/or how they are to be combined with the previous texel.
        \param Type: Environment type.
        */
        void setTextureEnv(const u8 Layer, const ETextureEnvTypes Type);
        
        //! Returns the texture environment type for the specified layer.
        ETextureEnvTypes getTextureEnv(const u8 Layer) const;
        
        /**
        Sets the texture mapping generation type. This specifies if and how the texture coordinates are to
        be automatically generated. This can be used for "sphere-mapping" for example (video::MAPGEN_SPHERE_MAP).
        The value "video::MAPGEN_DISABLE" disables the auto. generation and the originally stored texture coordinates
        will be used for texture mapping. The specified texture coordinates of each vertex will not get lost when
        auto. generation was used because this process will be performed in real-time on the GPU.
        \param Layer: Specifies the texture layer.
        \param Type: Mapping generation type.
        */
        void setMappingGen(const u8 Layer, const EMappingGenTypes Type);
        
        //! Returns texture mapping generation type of the specified layer.
        EMappingGenTypes getMappingGen(const u8 Layer) const;
        
        /**
        Sets the texture mapping generation axles which are affected by auto. mapping generation.
        e.g. for "sphere-mapping" the axles "S" and "T" are need to be used. In texture space the axles are
        called S, T, R and Q. Which represent X, Y, Z and W.
        \param Layer: Specifies the texture layer.
        \param Coords: This is a flags parameter so several values can be combined.
        e.g. for "sphere-mapping" the following combination has to be used: video::MAPGEN_S | video::MAPGEN_T.
        But you don't need to call this function to use "sphere-mapping". "setMappingGen" will automatically
        configure the right axles. Only use this function to configure this feature in more detail for your own purposes.
        */
        void setMappingGenCoords(const u8 Layer, s32 Coords);
        
        //! Returns texture mapping generation axles of the specified layer.
        s32 getMappingGenCoords(const u8 Layer) const;
        
        //! Returns a list with Pointers of all Texture objects.
        std::list<Texture*> getTextureList() const;
        
        /**
        Sets the reference to the surface texture list.
        \param Reference: Specifies the MeshBuffer object where its texture list will be used as reference.
        If 0 the original texture list will be used which is the default configuration.
        \note There is no "getTexturesReference". Use the "getSurfaceTextureList" function to get the current
        used texture list. Or use "hasTexturesReference" to determine if a reference is used.
        */
        void setTexturesReference(MeshBuffer* Reference);
        
        /**
        Sets the reference to the surface texture list. Use this if you want to use the same texture list
        for several surfaces. This can be usful for terrains where every terrain patch has the same textures.
        \param Reference: Pointer to an std::vector<SMeshSurfaceTexture> object.
        */
        void setTexturesReference(std::vector<SMeshSurfaceTexture>* &Reference);
        
        /* === Inline functions === */
        
        //! Sets a reference. Use this for mesh buffer instancing. By default 0 to disable instancing.
        inline void setReference(MeshBuffer* ReferenceSurface)
        {
            Reference_ = ReferenceSurface;
        }
        
        /**
        Sets the count of instances for hardware instancing. To check if hardware instancing is supported
        call "RenderSystem::queryVideoSupport" with parameter "video::QUERY_HARDWARE_INSTANCING".
        \param InstanceCount: Specifies the count of instancies. If this parameters smaller or equal to 1 hardware instancing will be disabled.
        */
        inline void setHardwareInstancing(s32 InstanceCount)
        {
            InstanceCount_ = InstanceCount;
        }
        //! Returns the count of instancies for hardware instancing.
        inline s32 getHardwareInstancing() const
        {
            return InstanceCount_;
        }
        
        //! Sets the buffers description name.
        inline void setName(const io::stringc &Name)
        {
            Name_ = Name;
        }
        //! Returns the buffers description name.
        inline io::stringc getName() const
        {
            return Name_;
        }
        
        //! Returns the hardware vertex buffer id.
        inline void* getVertexBufferID() const
        {
            return VertexBuffer_.Reference;
        }
        //! Returns the hardware index buffer id.
        inline void* getIndexBufferID() const
        {
            return IndexBuffer_.Reference;
        }
        
        //! Returns the vertex buffer.
        inline const dim::UniversalBuffer& getVertexBuffer() const
        {
            return VertexBuffer_.RawBuffer;
        }
        //! Returns the index buffer.
        inline const dim::UniversalBuffer& getIndexBuffer() const
        {
            return IndexBuffer_.RawBuffer;
        }
        
        //! Returns the vertex format.
        inline const VertexFormat* getVertexFormat() const
        {
            return VertexFormat_;
        }
        //! Returns the index format.
        inline const IndexFormat* getIndexFormat() const
        {
            return &IndexFormat_;
        }
        
        //! Returns count of vertices.
        inline u32 getVertexCount() const
        {
            return VertexBuffer_.RawBuffer.getCount();
        }
        
        //! Returns the count of indices.
        inline u32 getIndexCount() const
        {
            return IndexBuffer_.RawBuffer.getCount();
        }
        //! Returns count of triangles.
        inline u32 getTriangleCount() const
        {
            return IndexBuffer_.RawBuffer.getCount() / 3;
        }
        
        //! Sets the mesh buffer usage. For more detail see "setVertexBufferUsage".
        inline void setMeshBufferUsage(const EMeshBufferUsage Usage)
        {
            setVertexBufferUsage(Usage);
            setIndexBufferUsage(Usage);
        }
        
        /**
        Sets the vertex buffer usage. Use this to optimize usage on VRAM. If you have an animated mesh
        use the dynamic configuration. If you have a static mesh use the static configuration. By default static.
        */
        inline void setVertexBufferUsage(const EMeshBufferUsage Usage)
        {
            VertexBuffer_.Usage = Usage;
        }
        //! Returns the vertex buffer usage.
        inline EMeshBufferUsage getVertexBufferUsage() const
        {
            return VertexBuffer_.Usage;
        }
        
        //! Sets the index buffer usage. For more information see "setVertexBufferUsage".
        inline void setIndexBufferUsage(const EMeshBufferUsage Usage)
        {
            IndexBuffer_.Usage = Usage;
        }
        //! Returns the vertex buffer usage.
        inline EMeshBufferUsage getIndexBufferUsage() const
        {
            return IndexBuffer_.Usage;
        }
        
        //! Sets the surface textures.
        inline void setSurfaceTextureList(const std::vector<SMeshSurfaceTexture> &TextureList)
        {
            *TextureList_ = TextureList;
        }
        //! Returns constant reference vector with all surface textures.
        inline const std::vector<SMeshSurfaceTexture>& getSurfaceTextureList() const
        {
            return *TextureList_;
        }
        
        //! Returns count of textures.
        inline u32 getTextureCount() const
        {
            return TextureList_->size();
        }
        
        //! Returns true if the texture list is a reference to another one.
        inline bool hasTexturesReference() const
        {
            return &OrigTextureList_ != TextureList_;
        }
        
        //! Sets the index offset which will be added to each vertex index when adding a new triangle.
        inline void setIndexOffset(u32 Offset)
        {
            IndexOffset_ = Offset;
        }
        //! Increments the index offset which will be added to each vertex index when adding a new triangle.
        inline void addIndexOffset(u32 Offset)
        {
            IndexOffset_ += Offset;
        }
        
        /**
        Enables or disables the vertex buffer. If disabled the mesh buffer will be
        rendered as an array of vertices only. By default enabled.
        */
        inline void setIndexBufferEnable(bool Enable)
        {
            useIndexBuffer_ = Enable;
        }
        //! Returns true if the index buffer is to be used. Otherwise false.
        inline bool getIndexBufferEnable() const
        {
            return useIndexBuffer_;
        }
        
        /**
        Enables or disables updating mesh buffer immediatly. By default disabled.
        \param Enable: If true each vertex manipulation will be updated immediatly.
        This is very fast when just changing a few vertices in a large mesh.
        But when the model has a MorphTarget- or SkeletalAnimation it should be disabled.
        */
        inline void setUpdateImmediate(bool Enable)
        {
            UpdateImmediate_ = Enable;
        }
        //! Returns status of immediate mesh buffer updating.
        inline bool getUpdateImmediate() const
        {
            return UpdateImmediate_;
        }
        
        //! Returns the primitive type. By default PRIMITIVE_TRIANGLES.
        inline ERenderPrimitives getPrimitiveType() const
        {
            return PrimitiveType_;
        }
        
    protected:
        
        /* === Structures === */
        
        struct SMeshBufferBackup
        {
            SMeshBufferBackup() :
                BUVertexFormat(0)
            {
            }
            ~SMeshBufferBackup()
            {
            }
            
            /* Members */
            dim::UniversalBuffer BUVertexBuffer;
            dim::UniversalBuffer BUIndexBuffer;
            
            const VertexFormat* BUVertexFormat;
            IndexFormat BUIndexFormat;
        };
        
        struct SBuffer
        {
            SBuffer() :
                Reference   (0                  ),
                Validated   (false              ),
                Usage       (MESHBUFFER_STATIC  )
            {
            }
            SBuffer(const SBuffer &Other) :
                Reference   (0              ),
                RawBuffer   (Other.RawBuffer),
                Validated   (false          ),
                Usage       (Other.Usage    )
            {
            }
            ~SBuffer()
            {
            }
            
            /* Members */
            void* Reference;
            dim::UniversalBuffer RawBuffer;
            bool Validated;
            EMeshBufferUsage Usage;
        };
        
        /* === Functions === */
        
        void convertVertexAttribute(
            const dim::UniversalBuffer &OldBuffer, u32 Index, const SVertexAttribute &OldAttrib,
            const SVertexAttribute &NewAttrib, bool isClamp = true
        );
        void fillVertexAttribute(u32 Index, const SVertexAttribute &Attrib);
        
        virtual void updateNormalsFlat();
        virtual void updateNormalsGouraud();
        
        void checkIndexFormat(ERendererDataTypes &Format);
        
        /* === Inline functions === */
        
        template <typename T, typename D> inline void setDefaultVertexAttribute(
            const ERendererDataTypes Type, s32 MaxSize, u32 Index, const SVertexAttribute &Attrib, const T &Data)
        {
            if (Attrib.Type == Type)
                VertexBuffer_.RawBuffer.setBuffer(Index, Attrib.Offset, (const void*)&Data, sizeof(D) * math::Min(Attrib.Size, MaxSize));
        }
        template <typename T, typename D> inline T getDefaultVertexAttribute(
            const ERendererDataTypes Type, s32 MaxSize, u32 Index, const SVertexAttribute &Attrib) const
        {
            T Data;
            
            if (Attrib.Type == Type)
                VertexBuffer_.RawBuffer.getBuffer(Index, Attrib.Offset, (void*)&Data, sizeof(D) * math::Min(Attrib.Size, MaxSize));
            
            return Data;
        }
        
        template <typename T> inline void addTriangleIndices(const u32 VertexA, const u32 VertexB, const u32 VertexC)
        {
            IndexBuffer_.RawBuffer.add<T>((T)VertexA);
            IndexBuffer_.RawBuffer.add<T>((T)VertexB);
            IndexBuffer_.RawBuffer.add<T>((T)VertexC);
        }
        template <typename T> inline void addQuadrangleIndices(const u32 VertexA, const u32 VertexB, const u32 VertexC, const u32 VertexD)
        {
            IndexBuffer_.RawBuffer.add<T>((T)VertexA);
            IndexBuffer_.RawBuffer.add<T>((T)VertexB);
            IndexBuffer_.RawBuffer.add<T>((T)VertexC);
            IndexBuffer_.RawBuffer.add<T>((T)VertexD);
        }
        
        /* === Members === */
        
        io::stringc Name_;
        
        SBuffer VertexBuffer_;
        SBuffer IndexBuffer_;
        
        const VertexFormat* VertexFormat_;
        IndexFormat IndexFormat_;
        
        MeshBuffer* Reference_;
        
        std::vector<SMeshSurfaceTexture> OrigTextureList_;
        std::vector<SMeshSurfaceTexture>* TextureList_;
        
        u32 IndexOffset_;
        s32 InstanceCount_;
        
        ERenderPrimitives PrimitiveType_;
        bool useIndexBuffer_;
        bool UpdateImmediate_;
        
        SMeshBufferBackup* Backup_;
        
    private:
        
        /* === Functions === */
        
        void setupDefaultBuffers();
        
};


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
