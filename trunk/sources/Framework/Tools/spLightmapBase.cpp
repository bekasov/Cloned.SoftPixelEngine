/*
 * Lightmap generator base file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Framework/Tools/spLightmapBase.hpp"

#ifdef SP_COMPILE_WITH_LIGHTMAPGENERATOR


#include "SceneGraph/spSceneMesh.hpp"
#include "SceneGraph/spSceneLight.hpp"

#include <boost/foreach.hpp>


namespace sp
{
namespace tool
{


/*
 * SCastShadowObject structure
 */

SCastShadowObject::SCastShadowObject(scene::Mesh* Obj) throw(io::NullPointerException) :
    Mesh(Obj)
{
    if (!Obj)
        throw io::NullPointerException("'SCastShadowObject' class constructor");
}
SCastShadowObject::~SCastShadowObject()
{
}


/*
 * SGetShadowObject structure
 */

SGetShadowObject::SGetShadowObject(scene::Mesh* Obj, bool DefStayAlone) throw(io::NullPointerException) :
    Mesh        (Obj            ),
    StayAlone   (DefStayAlone   )
{
    if (!Obj)
        throw io::NullPointerException("1st 'SGetShadowObject' class constructor");
    
    TrianglesDensity.resize(Mesh->getMeshBufferCount());
    for (u32 s = 0; s < TrianglesDensity.size(); ++s)
        TrianglesDensity[s].resize(Mesh->getMeshBuffer(s)->getTriangleCount());
}
SGetShadowObject::SGetShadowObject(
    scene::Mesh* Obj, const std::vector< std::vector<f32> > &DefTrianglesDensity, bool DefStayAlone) throw(io::NullPointerException) :
    Mesh            (Obj                ),
    StayAlone       (DefStayAlone       ),
    TrianglesDensity(DefTrianglesDensity)
{
    if (!Obj)
        throw io::NullPointerException("2nd 'SGetShadowObject' class constructor");
    
    if (TrianglesDensity.size() != Mesh->getMeshBufferCount())
        TrianglesDensity.resize(Mesh->getMeshBufferCount());
    
    for (u32 s = 0; s < TrianglesDensity.size(); ++s)
    {
        u32 TriCount = Mesh->getMeshBuffer(s)->getTriangleCount();
        if (TrianglesDensity[s].size() != TriCount)
            TrianglesDensity[s].resize(TriCount);
    }
}
SGetShadowObject::~SGetShadowObject()
{
}


/*
 * SLightmapLight structure
 */

SLightmapLight::SLightmapLight() :
    Type            (scene::LIGHT_DIRECTIONAL   ),
    Attn0           (1.0f                       ),
    Attn1           (0.0f                       ),
    Attn2           (0.0f                       ),
    InnerConeAngle  (30.0f                      ),
    OuterConeAngle  (60.0f                      ),
    Visible         (false                      )
{
}
SLightmapLight::SLightmapLight(const scene::Light* Obj) throw(io::NullPointerException) :
    Type            (scene::LIGHT_DIRECTIONAL   ),
    Attn0           (1.0f                       ),
    Attn1           (0.0f                       ),
    Attn2           (0.0f                       ),
    InnerConeAngle  (30.0f                      ),
    OuterConeAngle  (60.0f                      ),
    Visible         (false                      )
{
    if (!Obj)
        throw io::NullPointerException("'SLightmapLight' class constructor");
    
    Type = Obj->getLightModel();
    Matrix = Obj->getTransformMatrix(true);
    
    video::color ClrDiffuse, ClrAmbient, ClrSpecular;
    Obj->getLightingColor(ClrDiffuse, ClrAmbient, ClrSpecular);
    
    Color = ClrDiffuse;
    
    if (Obj->getVolumetric())
    {
        Attn0 = 1.0f;
        Attn1 = 1.0f / Obj->getVolumetricRadius();
        Attn2 = 1.0f / Obj->getVolumetricRadius();
    }
    
    Obj->getSpotCone(InnerConeAngle, OuterConeAngle);
    Visible = Obj->getVisible();
}
SLightmapLight::~SLightmapLight()
{
}


} // /namespace tool

} // /namespace sp


#endif



// ================================================================================
