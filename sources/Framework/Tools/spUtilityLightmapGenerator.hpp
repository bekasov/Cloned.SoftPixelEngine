/*
 * Lightmap generator header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_TOOL_LIGHTMAPGENERATOR_H__
#define __SP_TOOL_LIGHTMAPGENERATOR_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_LIGHTMAPGENERATOR


#include "Base/spInputOutputString.hpp"
#include "Base/spDimension.hpp"
#include "Base/spTreeNodeImage.hpp"
#include "SceneGraph/spSceneGraph.hpp"
#include "SceneGraph/Collision/spCollisionConfigTypes.hpp"
#include "SceneGraph/Collision/spCollisionGraph.hpp"
#include "RenderSystem/spRenderSystem.hpp"

#include <boost/function.hpp>
#include <list>
#include <vector>
#include <map>


namespace sp
{
namespace tool
{


/*
 * Macros
 */

static const video::color DEF_LIGHTMAP_AMBIENT  = video::color(0);
static const u32 DEF_LIGHTMAP_SIZE              = 512;
static const f32 DEF_LIGHTMAP_DENSITY           = 10.0f;
static const u32 DEF_LIGHTMAP_BLURRADIUS        = 2;


/*
 * Typedefinitions
 */

typedef boost::function<bool (f32 Progress)> LightmapCallback;


/*
 * Enumerations
 */

enum ELightmapGenerationsFlags
{
    LIGHTMAPFLAG_NOCOLORS       = 0x00000001, //!< Colored lighting is disabled. When all lights have the diffuse color (255, 255, 255) this flag has no effect.
    LIGHTMAPFLAG_NOTRANSPARENCY = 0x00000002, //!< Transparency textures ray-casting is disabled. This may occur in much faster lightmap generation.
};


/*
 * Structures
 */

struct SP_EXPORT SCastShadowObject
{
    SCastShadowObject(scene::Mesh* Obj);
    ~SCastShadowObject();
    
    /* Members */
    scene::Mesh* Mesh;
};

struct SP_EXPORT SGetShadowObject
{
    SGetShadowObject(scene::Mesh* Obj, bool DefStayAlone = false);
    SGetShadowObject(
        scene::Mesh* Obj, const std::vector< std::vector<f32> > &DefTrianglesDensity,
        bool DefStayAlone = false
    );
    ~SGetShadowObject();
    
    /* Members */
    scene::Mesh* Mesh;
    bool StayAlone;
    
    std::vector< std::vector<f32> > TrianglesDensity;
};

struct SP_EXPORT SLightmapLight
{
    SLightmapLight();
    SLightmapLight(const scene::Light* Obj);
    ~SLightmapLight();
    
    /* Members */
    scene::ELightModels Type;
    
    dim::matrix4f Matrix;
    video::color Color;
    
    f32 Attn0, Attn1, Attn2;
    f32 InnerConeAngle, OuterConeAngle;
    
    bool Visible;
};


//! The lightmap generator class is a utility actually only used in a world editor.
class SP_EXPORT LightmapGenerator
{
    
    public:
        
        LightmapGenerator();
        ~LightmapGenerator();
        
        /**
        Generates the lightmaps for each get-shadow-object.
        This is a very time-consuming procedure which has been created for a level editor.
        Only simple-shadows are supported and no radiosity.
        \param CastShadowObjects: List of all 3D models which cast shadows.
        \param GetShadowObjects: List of all 3D models which get shadows. Only these objects build the resulting model.
        \param LightSources: List of all light sources which are to be used in the lightmap generation process.
        \param AmbientColor: Darkest color for each lightmap texel. Normally (in radiosity) those parameters are not used
        because the light calculations are physicially correct and the light rays can arrive into dark rooms where no light
        sources is placed. But in this case (with just simple-shadows) the texels are complete black when it has no direct
        connection to the light source. This happens when a cast-shadow-object stands in the texel's way. So this
        parameter makes sure that a texel is not darker than AmbientColor. It is an adjustment.
        \param LightmapsSize: Size of each lightmap texture. By default 512. Consider that the lightmap textures should not
        be too large and not too small. If they are too large the memory costs are higher because the MIP-map sub-textures
        are also generated. If they are too small more surfaces must be created for the resulting 3d model which makes
        the application slower.
        \param DefaultDensity: Density specifies the size for generating the texture space from the world space.
        By this procedure larger triangle faces get larger texture faces in the lightmap. When a calculated texture face is
        larger then "LightmapsSize" it is clamped to it.
        \param TexelBlurRadius: Blur factor for the lightmaps. This is also a balance for none-radiosity technic.
        This reduces hard-shadows and changes it to soft-shadows. The lightmap textures are not only blured completly.
        This technic is a little bit more complicate but can however cause some unbeautiful areas on the lightmaps.
        Mostly in the edges of a room. Be default 2 which causes a nice smooth light scene. If the blur factor is 0
        no bluring computations will proceeded.
        \param Flags: Specifies additional options for the generation process. For more information
        see the ELightmapGenerationsFlags enumeration.
        \return Pointer to an Mesh object where the whole level data is stored. This is a static object which forms the
        whole game level. Dynamic objects - when "StayAlone" in the SGetShadowObject structure is true - has this resulting
        object as parent. 'StayAlone' objects are objects which also got a lightmap but which shall be further movable.
        e.g. doors.
        */
        scene::Mesh* generateLightmaps(
            const std::list<SCastShadowObject> &CastShadowObjects,
            const std::list<SGetShadowObject> &GetShadowObjects,
            const std::list<SLightmapLight> &LightSources,
            const video::color &AmbientColor = DEF_LIGHTMAP_AMBIENT,
            const u32 LightmapsSize = DEF_LIGHTMAP_SIZE,
            const f32 DefaultDensity = DEF_LIGHTMAP_DENSITY,
            const u32 TexelBlurRadius = DEF_LIGHTMAP_BLURRADIUS,
            const s32 Flags = 0
        );
        
        void clearLightmapObjects(); // Delete the generated objects & textures
        
        /**
        Sets the callback function. This function is called several times in during the
        lightmap generation process. With this you can control the progress and maybe cancel.
        Try to process not so much in this function because the time costs will grow!
        \param Callback: Callback function in the form of "bool Callback(f32 Progress)"
        where Progress is between 0.0 and 1.0.
        */
        static void setCallback(const LightmapCallback &Callback);
        
        /* Objects */
        
        inline scene::Mesh* getFinalModel() const
        {
            return FinalModel_;
        }
        inline std::list<scene::Mesh*> getSingleModels() const
        {
            return SingleModels_;
        }
        
        inline std::list<video::Texture*> getLightmapList() const
        {
            return LightmapTextures_;
        }
        
    private:
        
        /* === Declerations === */
        
        struct SVertex;
        struct STriangle;
        struct SFace;
        struct SLightmap;
        struct SModel;
        struct SAxisData;
        
        typedef scene::ImageTreeNode<SLightmap> TRectNode;
        
        /* === Structures === */
        
        struct SVertex
        {
            SVertex();
            SVertex(const SModel* Model, const u32 VertexSurface, const u32 VertexIndex);
            ~SVertex();
            
            /* Functions */
            bool adjacency(const STriangle &OpTriangle) const;
            
            void scaleProj(const dim::size2df &Scale);
            dim::point2df getMapCoord() const;
            
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
            void computeDensityAverage();
            void updateVertexProjection();
            void resizeVertexProjection(const dim::size2di &NewSize);
            
            //! \todo This is incomplete and very slow!
            bool adjacency(const SFace &OpFace) const;
            
            void build(scene::Mesh* Mesh);
            
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
            void completeFaces();
            
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
            void partitionMesh();
            void createAxles();
            void linkAxisTriangles(const s32 Axis);
            void buildFaces(scene::Mesh* Mesh);
            
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
            video::Texture* createTexture();
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
        
        /* === Friends === */
        
        friend void LMapRasterizePixelCallback(
            s32 x, s32 y, const SRasterizerVertex &Vertex, void* UserData
        );
        friend void LMapBlurPixelCallback(s32 x, s32 y, void* UserData);
        
        /* === Functions === */
        
        void createFacesLightmaps(SModel* Model);
        void generateLightTexels(SLight* Light);
        
        void rasterizeTriangle(const SLight* Light, const STriangle &Triangle);
        
        void processTexelLighting(
            SLightmapTexel* Texel, const SLight* Light,
            const dim::vector3df &Position, const dim::vector3df &Normal
        );
        
        void createNewLightmap();
        void putFaceIntoLightmap(SFace* Face);
        
        void buildFinalMesh(SModel* Model);
        
        void blurLightmapTexels(SModel* Model, s32 Factor);
        
        static bool processRunning(bool BoostProgress = true);
        
        /* === Members === */
        
        scene::Mesh* FinalModel_;
        std::list<scene::Mesh*> SingleModels_;
        
        scene::CollisionGraph CollSys_;
        scene::CollisionMesh* CollMesh_;
        
        std::list<SCastShadowObject> CastShadowObjects_;
        std::list<SLight*> LightSources_;
        std::list<SModel*> GetShadowObjects_;
        
        std::list<SLightmap*> Lightmaps_;
        std::list<video::Texture*> LightmapTextures_;
        
        std::map<scene::Mesh*, SModel*> ModelMap_;
        
        SLightmap* CurLightmap_;
        TRectNode* CurRectRoot_;
        
        u32 TexelBlurRadius_;
        
        s32 Flags_;
        
        SFace* CurRasterFace_;
        const SLight* CurRasterLight_;
        SLightmap* CurRasterLightmap_;
        s32 CurRasterBlurFactor_;
        
        // Static members
        static LightmapCallback Callback_;
        static s32 Progress_;
        static s32 ProgressMax_;
        
        static f32 DefaultDensity_;
        static video::color AmbientColor_;
        static dim::size2di LightmapSize_;
        
};


} // /namespace tool

} // /namespace sp


#endif

#endif



// ================================================================================
