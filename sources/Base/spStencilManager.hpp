/*
 * Stencil manager header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_STENCILMANAGER_H__
#define __SP_STENCILMANAGER_H__


#include "Base/spStandard.hpp"
#include "Base/spMemoryManagement.hpp"
#include "Base/spInputOutputLog.hpp"
#include "Base/spMath.hpp"
#include "SceneGraph/spSceneMesh.hpp"
#include "SceneGraph/spSceneCamera.hpp"
#include "SceneGraph/spSceneLight.hpp"

#include <list>


namespace sp
{
namespace scene
{


/*
 * Macros
 */

#define __SP_STENCIL_POINTS_COUNT   10000
#define __SP_STENCIL_PLANES_COUNT   10000


/*
 * Enumerations
 */

//! \deprecated
enum EMirrorPlaneTypes
{
    MIRROR_X = 0,
    MIRROR_Y,
    MIRROR_Z
};


/*
 * Structures
 */

//! \deprecated
struct SShadowLightSource
{
    SShadowLightSource(
        scene::SceneNode* LightSource, video::color Color = video::color(0, 0, 0, 128),
        f32 InitShadowLength = 100.0f, f32 InitRadius = -1.0f, bool InitVolumetric = false)
        : Object(LightSource), ShadowColor(Color), ShadowLength(InitShadowLength),
        Radius(InitRadius), Volumetric(InitVolumetric), Visible(true)
    {
    }
    ~SShadowLightSource()
    {
    }
    
    /* Members */
    
    scene::SceneNode* Object;
    video::color ShadowColor;
    f32 ShadowLength;
    f32 Radius;
    bool Volumetric;
    bool Visible;
};


/*
 * CastCloudObject class
 */

// Pre-decleration
class StencilManager;
class CastCloudObject;

typedef void (*PFNSHADOWINTENSITYCALLBACK)(f32 &Intensity, const CastCloudObject* Mesh, const SShadowLightSource* LightSource);


//! \deprecated
class SP_EXPORT CastCloudObject
{
    
    public:
        
        CastCloudObject(Mesh* Object);
        ~CastCloudObject();
        
        /* Functions */
        
        /**
        Adds a light source only for this cast-cloud object. Do not create too much lights
        and use it for each object!
        */
        void addLightSource(SShadowLightSource* LightSource);
        void removeLightSource(SShadowLightSource* LightSource = 0);
        
        /* Inline functions */
        
        inline Mesh* getMesh() const
        {
            return Object_;
        }
        
        inline void setShadowIntensity(f32 Intensity)
        {
            ShadowIntensity_ = Intensity;
        }
        inline f32 getShadowIntensity() const
        {
            return ShadowIntensity_;
        }
        
        inline void setShadowIntensityCallback(PFNSHADOWINTENSITYCALLBACK pFunc)
        {
            ShadowIntensityCallback_ = pFunc;
        }
        
    private:
        
        friend class StencilManager;
        
        /* Strutcures */
        
        struct SPlaneEq
        {
            f32 a, b, c, d;
        };
        
        struct SPlane
        {
            u32 p[3], Neigh[3];
            SPlaneEq PlaneEq;
            bool isVisible;
        };
        
        /* Members */
        
        Mesh* Object_;
        
        f32 ShadowIntensity_;
        
        SPlane pPlanes_[__SP_STENCIL_PLANES_COUNT];
        //SPlane* pPlanes_;
        s32 CountOfPlanes_;
        
        dim::vector3df pMeshVertices_[__SP_STENCIL_POINTS_COUNT];
        //dim::vector3df* pMeshVertices_;
        s32 CountOfMeshVertices_;
        
        dim::vector3df* pShadowVertices_;
        s32 CountOfShadowVertices_;
        
        std::vector<SShadowLightSource*> LightSourcesList_;
        
        PFNSHADOWINTENSITYCALLBACK ShadowIntensityCallback_;
        
};


/*
 * StencilManager class
 */

/**
StencilManager is OUT-OF-DATE
\deprecated
*/
class SP_EXPORT StencilManager
{
    
    public:
        
        StencilManager();
        ~StencilManager();
        
        /* === Stencil shadows === */
        
        /**
        Creates a new cast-cloud object. Each ob these objects have their own shadwo mapped on each
        other 3D model in the whole scene because of stencil-shadow-volumes. Be aware of that Sprite
        objects are also affected by these shadows!
        \param Model: 3D model which is to be added as a cast-cloud object.
        \return Pointer to a CastCloudObject. Use this object to add lights which
        generates the shadow for it.
        */
        CastCloudObject* addCastCloudMesh(Mesh* Model);
        void removeCastCloudMesh(Mesh* Model);
        
        /**
        Adds a new light source for all cast-cloud objects which are currently created.
        You can also set the light sources individual for each cast-cloud object.
        \param LightSource: Pointer to a LightSource object.
        */
        void addLightSource(SShadowLightSource* LightSource);
        void removeLightSource(SShadowLightSource* LightSource);
        
        void updateStencilShadow(Mesh* Model);
        
        /**
        Renders the stencil shadows for the specified camera. This function is called internally
        from the SceneManager when stencil-effects are enabled with "setStencilEffects".
        \param hCamera: Camera object for which the stencil shadows are to be rendered.
        */
        void renderStencilShadows(Camera* hCamera);
        
        /* === Stencil clipping mirrors === */
        
        /*void addMirrorPlane(Mesh* Model, u8 Type);
        void removeMirrorPlane(Mesh* Model);
        
        void addMirrorObject(Mesh* Model);
        void removeMirrorObject(Mesh* Model);
        
        void updateMirrorPlane(Mesh* Model);
        
        void renderStencilMirrors(Camera* CameraObj);*/
        
        /* Inline functions */
        
        /**
        Enables or disables multiple shadows.
        \param MultiShadows: If false each shadow also if they overlap have the same color.
        Otherwise two shadows which overlap cause a partially darker shadow because the shadows
        are mixed. By default multi shadows are disabled.
        */
        inline void setMultiShadows(bool MultiShadows)
        {
            MultiShadows_ = MultiShadows;
        }
        inline bool getMultiShadows() const
        {
            return MultiShadows_;
        }
        
        //! Sets the shadow color for single shadows when MultiShadows are disabled.
        inline void setSingleShadowColor(const video::color &ShadowColor)
        {
            SingleShadowColor_ = ShadowColor;
        }
        inline video::color getSingleShadowColor() const
        {
            return SingleShadowColor_;
        }
        
        //! \return Whole cast-cloud-object list
        inline std::vector<CastCloudObject*> getObjectList() const
        {
            return ObjectList_;
        }
        
    private:
        
        /* Functions */
        
        void updateConnectivity(CastCloudObject* Object);
        void updateCalculationPlanes(CastCloudObject* Object);
        void updateShadowVolume(CastCloudObject* Object, const dim::vector3df LightPos, f32 ShadowLength);
        
        /* Members */
        
        std::vector<CastCloudObject*> ObjectList_;
        
        bool MultiShadows_;
        video::color SingleShadowColor_;
        
};


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
