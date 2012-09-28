/*
 * Standard definition header
 * 
 * This file is part of the "SoftPixel Engine Wrapper" (Copyright (c) 2008 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <SoftPixelEngine.hpp>

/*
 * ======= Compilation settings =======
 */

#define __COMPILE_WITH_STDCALL__

#if defined(SP_PLATFORM_WINDOWS)
#   define SPW_EXPORT       extern "C" __declspec(dllexport)
#   ifdef __COMPILE_WITH_STDCALL__
#       define SPW_BBCALL   _stdcall // _cdecl or _stdcall
#   else
#       define SPW_BBCALL
#   endif
#elif defined(SP_PLATFORM_LINUX)
#   define SPW_EXPORT
#   define SPW_BBCALL
#endif

#define SPW_PROC(t) SPW_EXPORT t SPW_BBCALL

/**
This is the crash-safe level which shall be used for compilation (This macro needs to be defined).
Level 0: No crash safe but fastest (for maximal speed).
Level 1: Some helpful queries will be made for wrong user input (default level).
Level 2: Maximal crash safe level (for debugging).
*/
#define __CRASHSAFE_LEVEL__ 2

/**
 - Each function must begin with "spw" to be exported.
   
 - Each function whcih ends with "_ex" will not be wrapped into the "SoftPixelEngineWrapper.bb" file
   but only int the BlitzBasic delcs file. Those functions may not have default parameters and
   may not exist already in the BlitzBasic command library!
   
 - The functions "spwGraphics3D", "spwFlip" and "extHideWindow" are special cases.
*/

/*
 * ======= Macros =======
 */

#define LP                  int         // Long-Pointer
#define STR                 const char* // String

#define STATIC(c, p)        (static_cast<c>(p))
#define CASTTO(c, p)        (reinterpret_cast<c>(p))
#define CAST(p)             CASTTO(LP, p)

#define NODE(p)             CASTTO(scene::SceneNode*, p)
#define MESH(p)             CASTTO(scene::Mesh*, p)
#define LIGHT(p)            CASTTO(scene::Light*, p)
#define BILLBOARD(p)        CASTTO(scene::Billboard*, p)
#define CAMERA(p)           CASTTO(scene::Camera*, p)
#define TERRAIN(p)          CASTTO(scene::Terrain*, p)
#define COLLISION(p)        CASTTO(scene::Collision*, p)
#define SHADER(p)           CASTTO(video::Shader*, p)
#define SHADERCLASS(p)      CASTTO(video::ShaderClass*, p)
#define FONT(p)             CASTTO(video::Font*, p)
#define TEXTURE(p)          CASTTO(video::Texture*, p)
#define SOUND(p)            CASTTO(audio::Sound*, p)
#define FILE(p)             CASTTO(io::File*, p)
#define MOVIE(p)            CASTTO(video::Movie*, p)
#define ANIM(p)             CASTTO(scene::Animation*, p)
#define ANIM_NODE(p)        CASTTO(scene::AnimationNode*, p)
#define ANIM_MORPHTARGET(p) CASTTO(scene::AnimationMorphTarget*, p)
#define ANIM_SKELETAL(p)    CASTTO(scene::AnimationSkeletal*, p)
#define MATRIX(p)           CASTTO(dim::matrix4f*, p)
#define BUFFER(p)           CASTTO(char*, p)
#define PHYSICSNODE(p)      CASTTO(physics::PhysicsBasicObject*, p)
#define STATICBODY(p)       CASTTO(physics::StaticBody*, p)
#define RIGIDBODY(p)        CASTTO(physics::RigidBody*, p)
#define BODYJOINT(p)        CASTTO(physics::BodyJoint*, p)

#define NODE_PROC_RTN(o, p)             \
    {                                   \
        switch (NODE(o)->getType())     \
        {                               \
            case scene::NODE_MESH:      \
                return MESH(o)->p;      \
            case scene::NODE_CAMERA:    \
                return CAMERA(o)->p;    \
            case scene::NODE_LIGHT:     \
                return LIGHT(o)->p;     \
            case scene::NODE_BILLBOARD: \
                return BILLBOARD(o)->p; \
            case scene::NODE_TERRAIN:   \
                return TERRAIN(o)->p;   \
        }                               \
    }

#define CHECK_OBJECT(o, f, n)                                       \
    if (!o)                                                         \
    {                                                               \
        WrapperError(f, "\"" + io::stringc(n) + "\" is invalid");   \
        return;                                                     \
    }

#define CHECK_OBJECT_RTN(o, f, n, r)                                \
    if (!o)                                                         \
    {                                                               \
        WrapperError(f, "\"" + io::stringc(n) + "\" is invalid");   \
        return r;                                                   \
    }

#define CHECK_POINTER(p, f, m)  \
    if (!p)                     \
    {                           \
        WrapperError(f, m);     \
        return;                 \
    }

#define CHECK_POINTER_RTN(p, f, m, r)   \
    if (!p)                             \
    {                                   \
        WrapperError(f, m);             \
        return r;                       \
    }

#define CHECK_OBJTYPE(o, t, f, m)   \
    if (NODE(o)->getType() != t)    \
    {                               \
        WrapperError(f, m);         \
        return;                     \
    }

#define CHECK_OBJTYPE_RTN(o, t, f, m, r)    \
    if (NODE(o)->getType() != t)            \
    {                                       \
        WrapperError(f, m);                 \
        return r;                           \
    }


using namespace sp;

/*
 * ======= Global members =======
 */

extern SoftPixelDevice*                             g_Device;
extern io::InputControl*                            g_InputControl;

extern video::RenderSystem*                         g_RenderSystem;
extern video::RenderContext*                        g_RenderContext;

extern scene::SceneGraph*                           g_SceneGraph;
extern scene::CollisionGraph*                       g_CollisionDetector;
extern scene::StencilManager*                       g_StencilManager;

extern io::OSInformator*                            g_OSInformator;

extern tool::ScriptLoader*                          g_ScriptLoader;
extern audio::SoundDevice*                          g_SoundDevice;
extern physics::PhysicsSimulator*                   g_PhysicsSystem;

extern io::FileSystem                               g_FileSystem;
extern io::Timer                                    g_Timer;

extern io::stringc                                  g_AppTitle;
extern io::stringc                                  g_EndMessage;
extern video::ERenderSystems                        g_DriverType;

extern video::color                                 g_Color, g_PixelColor, g_MultiColor[4];
extern dim::point2di                                g_Origin2D;
extern dim::point2di                                g_CameraProject;
extern dim::size2di                                 g_ScreenSize;
extern bool                                         g_AutoMidHandle;

extern video::Font*                                 g_Font;
extern video::Font*                                 g_StdFont;
extern scene::SceneNode*                            g_ListenerParent;

extern int                                          g_ColorDepth;
extern bool                                         g_EnableMultiColor;

extern int                                          g_ScancodeList[237];
extern io::EMouseKeyCodes                           g_MouseKeyList[3];
extern int                                          g_MouseZ;

extern SDeviceFlags                                 g_DeviceFlags;
extern LP                                           g_WholeSurfaceCount;

extern unsigned long int                            g_PickingTime;
extern scene::SPickingContact                       g_PickingContact;
extern std::list<scene::SPickingContact>::iterator  g_PickingIterator;

extern dim::vector3df                               g_PatchAnchorPoints[4][4];
extern int                                          g_PatchAnchorIndexVert, g_PatchAnchorIndexHorz;


/*
 * ======= Structures =======
 */

struct SWrapSurface
{
    video::MeshBuffer* MeshBuffer;
    int SurfaceNr;
    LP Mesh;
    LP Brush;
};

struct SWrapBrush
{
    LP Texture;
};

struct SWrapTexture
{
    SWrapTexture()
    {
        Scale.Width     = 1.0f;
        Scale.Height    = 1.0f;
        CoordsLayer     = 0;
        isMipmapped     = true;
        isSphereMap     = false;
        isEnabled       = true;
        //isMidHandle     = false;
        BlendMode       = video::TEXENV_MODULATE;
    }
    ~SWrapTexture()
    {
    }
    
    dim::point2df Position;
    dim::size2df Scale;
    f32 Angle;
    dim::matrix4f Matrix;
    s32 CoordsLayer;
    
    bool isMipmapped;
    bool isSphereMap;
    bool isEnabled;
    //bool isMidHandle;
    
    video::ETextureEnvTypes BlendMode;
};

struct SWrapImage
{
    SWrapImage()
    {
        isMidHandle = false;
        Rotation    = 0.0f;
    }
    ~SWrapImage()
    {
    }
    
    bool isMidHandle;
    f32 Rotation;
};

struct SWrapMovie
{
    video::Movie* Movie;
    audio::Sound* Sound;
    video::Texture* Texture;
};

struct SWrapCamera
{
    scene::Camera* Object;
    int ClearFlags;
    video::color ClearColor;
    video::EFogTypes FogMode;
    video::color FogColor;
    float FogFallOff, FogRangeNear, FogRangeFar;
};

struct SWrapCaptureObject
{
    scene::SceneNode* Object;
    dim::vector3df PositionA, PositionB;
    dim::quaternion RotationA, RotationB;
    dim::matrix4f LastRotationMat;
};

struct SWrapAnimation
{
    scene::SceneNode* Object;
    scene::Animation* Anim;
    // ...
};

struct SWrapAnimSeq
{
    s32 FirstFrame, LastFrame;
};

extern std::map<LP, SWrapSurface>               g_SurfaceList;
extern std::map<LP, SWrapTexture>               g_TextureList;
extern std::map<LP, SWrapImage>                 g_ImageList;
extern std::map<LP, SWrapMovie>                 g_MovieList;
extern std::map<LP, SWrapCamera>                g_CameraList;
extern std::map<LP, SWrapAnimation>             g_AnimationList;
extern std::map<int, scene::Collision*>         g_CollisionList;
extern std::map<LP, scene::SShadowLightSource*> g_ShadowLightList;

extern std::vector<SWrapBrush>                  g_BrushList;
extern std::vector<SWrapAnimSeq>                g_AnimSeqList;

extern std::list<scene::SPickingContact>        g_PickingList;
extern std::list<dim::matrix4f*>                g_MatrixList;
extern std::list<char*>                         g_BufferList;
extern std::list<SWrapCaptureObject>            g_CaptureWorldList;


/* === Declerations === */

static void InitScancodeList();
void WrapperError(const io::stringc &Message);
void WrapperError(const io::stringc &FunctionName, const io::stringc &Message);
void defCallbackUserMaterial(scene::Mesh* Obj, bool isBegin);

SPW_PROC(void) spwPaintSurface(LP Surface, LP Brush);


/*
 * ===================== All static inline functions =====================
 */

static inline void stcAttachSurfaces(scene::Mesh* Mesh)
{
    SWrapSurface Surface;
    Surface.Mesh    = CAST(Mesh);
    Surface.Brush   = 0;
    
    for (unsigned int s = 0; s < Mesh->getMeshBufferCount(); ++s)
    {
        Surface.MeshBuffer  = Mesh->getMeshBuffer(s);
        Surface.SurfaceNr   = (int)s;
        g_SurfaceList[++g_WholeSurfaceCount] = Surface;
    }
}

static inline void stcDefaultMeshSettings(scene::Mesh* Mesh)
{
    video::MaterialStates* Material = Mesh->getMaterial();
    Material->setColorMaterial(false);
    Material->setMaterialCallback(defCallbackUserMaterial);
    Material->setSpecularColor(video::color(0));
    Material->setAmbientColor(video::color(128));
    stcAttachSurfaces(Mesh);
}

static inline LP stcCreateStaticObject(const scene::EBasicMeshes Type, LP Parent, bool FlatShaded = false)
{
    scene::Mesh* Obj = g_SceneGraph->createMesh(Type);
    Obj->setParent(NODE(Parent));
    
    if (FlatShaded)
        Obj->setShading(video::SHADING_FLAT);
    
    Obj->meshTransform(2);
    stcDefaultMeshSettings(Obj);
    return CAST(Obj);
}

static inline bool stcGetCollisionContactByIndex(LP Entity, int Index, scene::SCollisionContact &Contact)
{
    std::list<scene::SCollisionContact> ContactList = NODE(Entity)->getContactList();
    
    int i = 1;
    for (std::list<scene::SCollisionContact>::iterator it = ContactList.begin(); it != ContactList.end(); ++it, ++i)
    {
        if (i == Index)
        {
            Contact = *it;
            return true;
        }
    }
    
    return false;
}

static inline void stcTransformTextureMatrix(SWrapTexture &Texture)
{
    Texture.Matrix.reset();
    Texture.Matrix.translate(dim::vector3df(Texture.Position.X, Texture.Position.Y, 0.0));
    Texture.Matrix.rotateZ(Texture.Angle);
    Texture.Matrix.scale(dim::vector3df(Texture.Scale.Width, Texture.Scale.Height, 1.0));
}

static inline void stcSetTextureFalgs(video::Texture* Texture, int Flags, SWrapTexture* Obj)
{
    if (Flags & 0x0004) // Masked
        Texture->setColorKey(video::color(0, 0, 0, 0));
    else if (!(Flags & 0x0002)) // Alpha (1st pass)
        Texture->setColorKey(video::color(0, 0, 0, 255));
    else if ((Flags & 0x0002) && Texture->getFormat() < 4) // Alpha (2nd pass)
        Texture->setColorKeyAlpha(video::BLENDING_BRIGHT);
    
    Texture->setMipMapping((Flags & 0x0008) != 0); // Mipmapped
    
    Texture->setWrapMode(
        (Flags & 0x0010) ? video::TEXWRAP_CLAMP : video::TEXWRAP_REPEAT, // Clamp U
        (Flags & 0x0020) ? video::TEXWRAP_CLAMP : video::TEXWRAP_REPEAT  // Clamp V
    );
    
    if (Obj)
        Obj->isSphereMap = ((Flags & 0x0040) != 0); // Spheremap
}

static inline void stcSetTextureFrame(video::Texture* Texture, int Frame)
{
    if (Texture->getAnimation())
        Texture->setAnimFrame(Frame);
}

static inline void stcSetMeshSurfaceTexture(
    scene::Mesh* Mesh, video::Texture* Texture, int Frame = 0, int Index = 0, int SurfaceNr = -1)
{
    if (SurfaceNr == -1)
    {
        for (unsigned int i = 0; i < Mesh->getMeshBufferCount(); ++i)
            stcSetMeshSurfaceTexture(Mesh, Texture, Frame, Index, i);
    }
    else
    {
        video::MeshBuffer* Surface = Mesh->getMeshBuffer(SurfaceNr);
        
        if (Surface)
        {
            if (Texture)
            {
                stcSetTextureFrame(Texture, Frame);
                
                if (Surface->getTexture(Index))
                    Surface->setTexture(Index, Texture);
                else
                    Surface->addTexture(Texture, Index);
                
                SWrapTexture TexData = g_TextureList[CAST(Texture)];
                
                Surface->setTextureMatrix(Index, TexData.Matrix);
                Surface->setTextureEnv(Index, TexData.BlendMode);
            }
            else
                Surface->removeTexture(Index);
        }
    }
}


/*
 * ===================== All template inline functions =====================
 */

template <class T> inline void tplCaptureObjectList(const std::list<T*> &NodeList)
{
    SWrapCaptureObject CapObjData;
    
    for (typename std::list< T*, std::allocator<T*> >::const_iterator it = NodeList.begin(); it != NodeList.end(); ++it)
    {
        CapObjData.Object       = *it;
        CapObjData.PositionA    = (*it)->getPosition();
        CapObjData.RotationA    = dim::quaternion((*it)->getRotationMatrix());
        
        g_CaptureWorldList.push_back(CapObjData);
    }
}



// ================================================================================
