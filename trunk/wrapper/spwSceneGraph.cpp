/*
 * Wrapper scene management file
 * 
 * This file is part of the "SoftPixel Engine Wrapper" (Copyright (c) 2008 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "spwStandard.hpp"


/*
 * ======= All wrapper functions =======
 */

SPW_PROC(LP) spwCreateCamera(LP Parent = 0)
{
    scene::Camera* Obj = g_SceneGraph->createCamera();
    Obj->setParent(NODE(Parent));
    Obj->setRange(0.5, 500);
    
    SWrapCamera CamData;
    {
        CamData.Object          = Obj;
        CamData.ClearFlags      = video::BUFFER_COLOR | video::BUFFER_DEPTH;
        CamData.ClearColor      = video::color(0);
        CamData.FogMode         = video::FOG_NONE;
        CamData.FogColor        = video::color(0);
        CamData.FogFallOff      = 0.1f;
        CamData.FogRangeNear    = 1.0f;
        CamData.FogRangeFar     = 1000.0f;
    }
    g_CameraList[CAST(Obj)] = CamData;
    
    return CAST(Obj);
}

SPW_PROC(LP) spwCreateLight(int Type = 1, LP Parent = 0)
{
    scene::ELightModels LightType;
    
    switch (Type)
    {
        case 3:
            LightType = scene::LIGHT_SPOT; break;
        case 2:
            LightType = scene::LIGHT_POINT; break;
        case 1:
        default:
            LightType = scene::LIGHT_DIRECTIONAL; break;
    }
    
    scene::Light* Obj = g_SceneGraph->createLight(LightType);
    Obj->setParent(NODE(Parent));
    Obj->setLightingColor(255, 255, 255);
    
    g_SceneGraph->setLighting(true);
    
    return CAST(Obj);
}

SPW_PROC(LP) spwCopyEntity(LP Entity, LP Parent = 0)
{
    #if __CRASHSAFE_LEVEL__ >= 2
    CHECK_OBJECT_RTN(Entity, "CopyEntity", "Entity", 0)
    #endif
    
    scene::SceneNode* Obj = 0;
    
    switch (NODE(Entity)->getType())
    {
        case scene::NODE_MESH:
        {
            scene::Mesh* Mesh = g_SceneGraph->copyNode(MESH(Entity));
            stcDefaultMeshSettings(Mesh);
            Obj = Mesh;
        }
        break;
        
        case scene::NODE_CAMERA:
            Obj = g_SceneGraph->copyNode(CAMERA(Entity)); break;
        case scene::NODE_LIGHT:
            Obj = g_SceneGraph->copyNode(LIGHT(Entity)); break;
        case scene::NODE_BILLBOARD:
            Obj = g_SceneGraph->copyNode(BILLBOARD(Entity)); break;
        default:
            return 0;
    }
    
    Obj->setParent(NODE(Parent));
    Obj->setVisible(true);
    
    return CAST(Obj);
}

SPW_PROC(LP) spwCreatePivot(LP Parent = 0)
{
    scene::SceneNode* Obj = g_SceneGraph->createNode();
    Obj->setParent(NODE(Parent));
    return CAST(Obj);
}

SPW_PROC(LP) spwCreateMesh(LP Parent = 0)
{
    scene::Mesh* Obj = g_SceneGraph->createMesh();
    Obj->setParent(NODE(Parent));
    stcDefaultMeshSettings(Obj);
    return CAST(Obj);
}

SPW_PROC(LP) spwCreateCube(LP Parent = 0)
{
    scene::Mesh* Obj = g_SceneGraph->createMesh(scene::MESH_CUBE);
    Obj->setParent(NODE(Parent));
    Obj->setShading(video::SHADING_FLAT);
    Obj->meshTransform(2);
    stcDefaultMeshSettings(Obj);
    return CAST(Obj);
}

SPW_PROC(LP) spwCreateCone(int Segments = 8, bool Solid = true, LP Parent = 0)
{
    scene::Mesh* Obj = g_SceneGraph->createMesh(scene::MESH_CONE, scene::SMeshConstruct(Segments, 1.0, 0.5, Solid));
    Obj->setParent(NODE(Parent));
    stcDefaultMeshSettings(Obj);
    return CAST(Obj);
}

SPW_PROC(LP) spwCreateCylinder(int Segments = 8, bool Solid = true, LP Parent = 0)
{
    scene::Mesh* Obj = g_SceneGraph->createMesh(scene::MESH_CYLINDER, scene::SMeshConstruct(Segments, 0.5, 0.25, Solid));
    Obj->setParent(NODE(Parent));
    Obj->meshTransform(2);
    stcDefaultMeshSettings(Obj);
    return CAST(Obj);
}

SPW_PROC(LP) spwCreatePipe(int Segments = 8, float OuterRadius = 1.0, float InnerRadius = 0.5, LP Parent = 0)
{
    scene::Mesh* Obj = g_SceneGraph->createMesh(scene::MESH_PIPE, scene::SMeshConstruct(Segments, OuterRadius, InnerRadius));
    Obj->setParent(NODE(Parent));
    Obj->meshTransform(dim::vector3df(1, 2, 1));
    stcDefaultMeshSettings(Obj);
    return CAST(Obj);
}

SPW_PROC(LP) spwCreateSphere(int Segments = 8, LP Parent = 0)
{
    scene::Mesh* Obj = g_SceneGraph->createMesh(scene::MESH_SPHERE, Segments);
    Obj->setParent(NODE(Parent));
    Obj->meshTransform(2);
    stcDefaultMeshSettings(Obj);
    return CAST(Obj);
}

SPW_PROC(LP) spwCreateIcoSphere(int Segments = 3, LP Parent = 0)
{
    scene::Mesh* Obj = g_SceneGraph->createMesh(scene::MESH_ICOSPHERE, Segments);
    Obj->setParent(NODE(Parent));
    Obj->meshTransform(2);
    stcDefaultMeshSettings(Obj);
    return CAST(Obj);
}

SPW_PROC(LP) spwCreateTorus(int Segments = 8, float OuterRadius = 1.0, float InnerRadius = 0.5, LP Parent = 0)
{
    scene::Mesh* Obj = g_SceneGraph->createMesh(scene::MESH_TORUS, scene::SMeshConstruct(Segments, OuterRadius, InnerRadius));
    Obj->setParent(NODE(Parent));
    Obj->meshTransform(2);
    stcDefaultMeshSettings(Obj);
    return CAST(Obj);
}

SPW_PROC(LP) spwCreateSpiral(
    int Segments = 8, float OuterRadius = 1.0, float InnerRadius = 0.5, float DegreeLength = 360.0, float DegreeHeight = 2.0, bool Solid = true, LP Parent = 0)
{
    scene::Mesh* Obj = g_SceneGraph->createMesh(
        scene::MESH_SPIRAL, scene::SMeshConstruct(Segments, OuterRadius, InnerRadius, DegreeLength, DegreeHeight, Solid)
    );
    Obj->setParent(NODE(Parent));
    Obj->meshTransform(2);
    stcDefaultMeshSettings(Obj);
    return CAST(Obj);
}

SPW_PROC(LP) spwCreatePlane(int Segments = 1, LP Parent = 0, float Radius = 1000.0)
{
    scene::Mesh* Obj = g_SceneGraph->createMesh(scene::MESH_PLANE);
    Obj->setParent(NODE(Parent));
    Obj->setShading(video::SHADING_FLAT);
    Obj->updateNormals();
    Obj->meshTransform(Radius);
    
    for (int i = 0; i < MAX_COUNT_OF_TEXTURES; ++i)
        Obj->getMeshBuffer(0)->textureTransform(i, Radius);
    
    stcDefaultMeshSettings(Obj);
    return CAST(Obj);
}

SPW_PROC(LP) spwCreateTetrahedron(LP Parent = 0)
{
    return stcCreateStaticObject(scene::MESH_TETRAHEDRON, Parent, true);
}

SPW_PROC(LP) spwCreateDodecahedron(LP Parent = 0)
{
    return stcCreateStaticObject(scene::MESH_DODECAHEDRON, Parent, true);
}

SPW_PROC(LP) spwCreateIcosahedron(LP Parent = 0)
{
    return stcCreateStaticObject(scene::MESH_ICOSAHEDRON, Parent, true);
}

SPW_PROC(LP) spwCreateCuboctahedron(LP Parent = 0)
{
    return stcCreateStaticObject(scene::MESH_CUBOCTAHEDRON, Parent, true);
}

SPW_PROC(LP) spwCreateOctahedron(LP Parent = 0)
{
    return stcCreateStaticObject(scene::MESH_OCTAHEDRON, Parent, true);
}

SPW_PROC(LP) spwCreateTeapot(LP Parent = 0)
{
    return stcCreateStaticObject(scene::MESH_TEAPOT, Parent);
}

SPW_PROC(LP) spwCreateSkyBox(
    LP TexBack, LP TexFront, LP TexTop, LP TexBottom, LP TexLeft, LP TexRight, float Radius = 100.0)
{
    #if __CRASHSAFE_LEVEL__ >= 2
    CHECK_OBJECT_RTN(TexBack,   "CreateSkyBox", "TexBack"   , 0)
    CHECK_OBJECT_RTN(TexFront,  "CreateSkyBox", "TexFront"  , 0)
    CHECK_OBJECT_RTN(TexTop,    "CreateSkyBox", "TexTop"    , 0)
    CHECK_OBJECT_RTN(TexBottom, "CreateSkyBox", "TexBottom" , 0)
    CHECK_OBJECT_RTN(TexLeft,   "CreateSkyBox", "TexLeft"   , 0)
    CHECK_OBJECT_RTN(TexRight,  "CreateSkyBox", "TexRight"  , 0)
    #endif
    
    video::Texture* TexList[6] = {
        TEXTURE(TexBack), TEXTURE(TexFront), TEXTURE(TexTop), TEXTURE(TexBottom), TEXTURE(TexLeft), TEXTURE(TexRight)
    };
    scene::Mesh* Obj = g_SceneGraph->createSkyBox(TexList, Radius);
    stcDefaultMeshSettings(Obj);
    return CAST(Obj);
}

SPW_PROC(LP) spwLoadHeightField(STR File, int Segments = -1, LP Parent = 0)
{
    video::Texture* HeightMap = g_RenderSystem->loadTexture(File);
    
    if (Segments == -1)
    {
        Segments = math::MinMax(
            (int)(sqrt((float)(HeightMap->getSize().Width * HeightMap->getSize().Height)) / 10), 1, 100
        );
    }
    
    scene::Mesh* Obj  = g_SceneGraph->createHeightField(HeightMap, Segments);
    
    const float Width   = HeightMap->getSize().Width;
    const float Height  = HeightMap->getSize().Height;
    
    Obj->meshTranslate(dim::vector3df(0.5, 0.0, 0.5));
    Obj->meshTransform(dim::vector3df(Width, 1, Height));
    
    for (int i = 0; i < MAX_COUNT_OF_TEXTURES; ++i)
        Obj->getMeshBuffer(0)->textureTransform(i, dim::point2df(Width, Height));
    
    stcDefaultMeshSettings(Obj);
    return CAST(Obj);
}

SPW_PROC(LP) spwLoadTerrain(STR File, LP Parent = 0)
{
    #if 0
    video::Texture* HeightMap = g_RenderSystem->loadTexture(File);
    scene::Terrain* Obj = g_SceneGraph->createTerrain(HeightMap, 256);
    Obj->setScale(dim::vector3df(HeightMap->getSize().Width, 1, HeightMap->getSize().Height));
    return CAST(Obj);
    #else
    return spwLoadHeightField(File, -1, Parent);
    #endif
}

SPW_PROC(LP) spwLoadMesh(STR File, LP Parent = 0)
{
    scene::Mesh* Obj = g_SceneGraph->loadMesh(File);
    Obj->setParent(NODE(Parent));
    stcDefaultMeshSettings(Obj);
    return CAST(Obj);
}

SPW_PROC(LP) spwLoadAnimMesh(STR File, LP Parent = 0)
{
    return spwLoadMesh(File, Parent);
}

SPW_PROC(LP) spwLoadMD2(STR File, LP Parent = 0)
{
    return spwLoadMesh(File, Parent);
}

SPW_PROC(LP) spwLoadBSP(STR File, float Gamma = 0.0, LP Parent = 0)
{
    scene::Mesh* Obj = g_SceneGraph->loadScene(File);
    Obj->setParent(NODE(Parent));
    Obj->getMaterial()->setLighting(false);
    stcDefaultMeshSettings(Obj);
    return CAST(Obj);
}

SPW_PROC(void) spwSaveMesh_ex(LP Mesh, STR File)
{
    #if __CRASHSAFE_LEVEL__ >= 2
    CHECK_OBJECT(Mesh, "SaveMesh", "Mesh")
    #endif
    #if __CRASHSAFE_LEVEL__ >= 1
    CHECK_OBJTYPE(Mesh, scene::NODE_MESH, "SaveMesh", "\"Mesh\" is not a mesh")
    #endif
    
    g_SceneGraph->saveMesh(MESH(Mesh), File);
}

SPW_PROC(void) spwLoadSPSB(STR File, int Flags = 1022)
{
    g_SceneGraph->loadScene(File, "", scene::SCENEFORMAT_SPSB, Flags);
}

SPW_PROC(void) spwBSPLighting(LP BSP, bool UseLightmaps)
{
    #if __CRASHSAFE_LEVEL__ >= 2
    CHECK_OBJECT(BSP, "BSPLighting", "BSP")
    #endif
    #if __CRASHSAFE_LEVEL__ >= 1
    CHECK_OBJTYPE(BSP, scene::NODE_MESH, "BSPLighting", "\"BSP\" is not a mesh")
    #endif
    
    scene::Mesh* Mesh = MESH(BSP);
    Mesh->getMaterial()->setLighting(!UseLightmaps);
    
    if (!UseLightmaps)
    {
        for (unsigned int s = 0; s < Mesh->getMeshBufferCount(); ++s)
            Mesh->getMeshBuffer(s)->removeTexture(1);
    }
}

SPW_PROC(LP) spwLoadSprite(STR File, int Flags = 9, LP Parent = 0)
{
    video::Texture* Tex = g_RenderSystem->loadTexture(File);
    stcSetTextureFalgs(Tex, Flags, 0);
    
    scene::Billboard* Obj = g_SceneGraph->createBillboard(Tex);
    
    Obj->setParent(NODE(Parent));
    Obj->getMaterial()->setLighting(false);
    Obj->setScale(0.5);
    
    return CAST(Obj);
}

SPW_PROC(LP) spwCreateSprite(LP Parent = 0)
{
    scene::Billboard* Obj = g_SceneGraph->createBillboard(0);
    
    Obj->setParent(NODE(Parent));
    Obj->getMaterial()->setLighting(false);
    Obj->setScale(0.5);
    
    return CAST(Obj);
}

SPW_PROC(void) spwRotateSprite(LP Sprite, float Angle)
{
    #if __CRASHSAFE_LEVEL__ >= 2
    CHECK_OBJECT(Sprite, "RotateSprite", "Sprite")
    #endif
    #if __CRASHSAFE_LEVEL__ >= 1
    CHECK_OBJTYPE(Sprite, scene::NODE_BILLBOARD, "RotateSprite", "\"Sprite\" is not a sprite")
    #endif
    
    dim::vector3df Vec(BILLBOARD(Sprite)->getRotation());
    Vec.Z = Angle;
    
    BILLBOARD(Sprite)->setRotation(Vec);
}

SPW_PROC(void) spwScaleSprite(LP Sprite, float Width, float Height)
{
    #if __CRASHSAFE_LEVEL__ >= 2
    CHECK_OBJECT(Sprite, "ScaleSprite", "Sprite")
    #endif
    #if __CRASHSAFE_LEVEL__ >= 1
    CHECK_OBJTYPE(Sprite, scene::NODE_BILLBOARD, "ScaleSprite", "\"Sprite\" is not a sprite")
    #endif
    
    dim::vector3df Vec(BILLBOARD(Sprite)->getScale());
    Vec.X = Width * 0.5f;
    Vec.Y = Height * 0.5f;
    
    BILLBOARD(Sprite)->setScale(Vec);
}

SPW_PROC(void) spwHandleSprite(LP Sprite, float X, float Y)
{
    #if __CRASHSAFE_LEVEL__ >= 2
    CHECK_OBJECT(Sprite, "HandleSprite", "Sprite")
    #endif
    #if __CRASHSAFE_LEVEL__ >= 1
    CHECK_OBJTYPE(Sprite, scene::NODE_BILLBOARD, "HandleSprite", "\"Sprite\" is not a sprite")
    #endif
    BILLBOARD(Sprite)->setBasePosition(dim::point2df(X, Y));
}

SPW_PROC(void) spwSpriteViewMode(LP Sprite, int Mode)
{
    #if __CRASHSAFE_LEVEL__ >= 2
    CHECK_OBJECT(Sprite, "SpriteViewMode", "Sprite")
    #endif
    // !TODO!
}

SPW_PROC(void) spwMeshReference(LP Mesh, LP MeshReference, bool CopyLocation = false, bool CopyMaterial = false)
{
    #if __CRASHSAFE_LEVEL__ >= 2
    CHECK_OBJECT(Mesh, "MeshReference", "Mesh")
    CHECK_OBJECT(MeshReference, "MeshReference", "MeshReference")
    #endif
    #if __CRASHSAFE_LEVEL__ >= 1
    CHECK_OBJTYPE(Mesh, scene::NODE_MESH, "MeshReference", "Mesh")
    CHECK_OBJTYPE(MeshReference, scene::NODE_MESH, "MeshReference", "MeshReference")
    #endif
    MESH(Mesh)->setReference(MESH(MeshReference), CopyLocation, CopyMaterial);
}

SPW_PROC(void) spwSurfaceReference(LP Surface, LP SurfaceReference, bool CopyTextures = false)
{
    SWrapSurface SurfaceData    = g_SurfaceList[Surface];
    SWrapSurface SurfaceDataRef = g_SurfaceList[SurfaceReference];
    
    MESH(SurfaceData.Mesh)->getMeshBuffer(SurfaceData.SurfaceNr)->setReference(
        MESH(SurfaceDataRef.Mesh)->getMeshBuffer(SurfaceDataRef.SurfaceNr)
    );
}

SPW_PROC(void) spwEntityBoundingBox_ex(LP Entity)
{
    #if __CRASHSAFE_LEVEL__ >= 2
    CHECK_OBJECT(Entity, "EntityBoundingBox", "Entity")
    #endif
    #if __CRASHSAFE_LEVEL__ >= 1
    CHECK_OBJTYPE(Entity, scene::NODE_MESH, "EntityBoundingBox", "\"Entity\" is not a mesh")
    #endif
    NODE(Entity)->getBoundingVolume().setType(scene::BOUNDING_BOX);
    NODE(Entity)->getBoundingVolume().setBox(MESH(Entity)->getMeshBoundingBox());
}

SPW_PROC(void) spwEntityBoundingSphere_ex(LP Entity, float Radius)
{
    #if __CRASHSAFE_LEVEL__ >= 2
    CHECK_OBJECT(Entity, "EntityBoundingSphere", "Entity")
    #endif
    NODE(Entity)->getBoundingVolume().setType(scene::BOUNDING_SPHERE);
    NODE(Entity)->getBoundingVolume().setRadius(Radius);
}

SPW_PROC(void) spwEntityNoneBounding_ex(LP Entity)
{
    #if __CRASHSAFE_LEVEL__ >= 2
    CHECK_OBJECT(Entity, "EntityNoneBounding", "Entity")
    #endif
    NODE(Entity)->getBoundingVolume().setType(scene::BOUNDING_NONE);
}

SPW_PROC(void) spwAnimateMD2(
    LP Entity, int Mode = 1, float Speed = 1.0, int FirstFrame = 0, int LastFrame = 0, float Transition = 0.0)
{
    #if __CRASHSAFE_LEVEL__ >= 2
    CHECK_OBJECT(Entity, "AnimateMD2", "Entity")
    #endif
    #if __CRASHSAFE_LEVEL__ >= 1
    CHECK_OBJTYPE(Entity, scene::NODE_MESH, "EntityBoundingBox", "\"Entity\" is not a mesh")
    #endif
    
    scene::Mesh* Mesh = MESH(Entity);
    scene::Animation* Anim = Mesh->getAnimation(0);
    
    #if __CRASHSAFE_LEVEL__ >= 1
    CHECK_POINTER(Anim, "AnimateMD2", "Mesh has no animation");
    #endif
    
    if (Anim)
    {
        Anim->setSpeed(Speed);
        
        switch (Mode)
        {
            case 0:
                Anim->stop(); break;
            case 1:
                Anim->play(scene::PLAYBACK_LOOP, FirstFrame - 1, LastFrame - 1); break;
            case 2:
                Anim->play(scene::PLAYBACK_PINGPONG_LOOP, FirstFrame - 1, LastFrame - 1); break;
            case 3:
                Anim->play(scene::PLAYBACK_ONESHOT, FirstFrame - 1, LastFrame - 1); break;
        }
    }
}

SPW_PROC(void) spwAnimate(
    LP Entity, int Mode = 1, float Speed = 1.0, int Sequence = 0, float Transition = 0.0)
{
    s32 FirstFrame = 0, LastFrame = 0;
    
    if (Sequence > 0 && Sequence <= g_AnimSeqList.size())
    {
        FirstFrame  = g_AnimSeqList[Sequence - 1].FirstFrame;
        LastFrame   = g_AnimSeqList[Sequence - 1].LastFrame;
    }
    
    spwAnimateMD2(Entity, Mode, Speed * 2, FirstFrame, LastFrame, Transition);
}

SPW_PROC(bool) spwAnimating(LP Entity)
{
    #if __CRASHSAFE_LEVEL__ >= 2
    CHECK_OBJECT_RTN(Entity, "Animating", "Entity", false)
    #endif
    #if __CRASHSAFE_LEVEL__ >= 1
    CHECK_POINTER_RTN(NODE(Entity)->getAnimation(0), "Animating", "Entity has no animation", false);
    #endif
    return MESH(Entity)->getAnimation(0)->animating();
}

SPW_PROC(int) spwAnimLength(LP Entity)
{
    #if __CRASHSAFE_LEVEL__ >= 2
    CHECK_OBJECT_RTN(Entity, "AnimLength", "Entity", 0)
    #endif
    #if __CRASHSAFE_LEVEL__ >= 1
    CHECK_POINTER_RTN(NODE(Entity)->getAnimation(0), "AnimLength", "Entity has no animation", 0);
    #endif
    return NODE(Entity)->getAnimation(0)->getLength();
}

SPW_PROC(int) spwAnimSeq(LP Entity)
{
    #if __CRASHSAFE_LEVEL__ >= 2
    CHECK_OBJECT_RTN(Entity, "AnimSeq", "Entity", 0)
    #endif
    
    s32 Sequence;
    f32 Interpolation;
    NODE(Entity)->getAnimation(0)->getSeek(Sequence, Interpolation);
    
    return Sequence;
}

SPW_PROC(float) spwAnimTime(LP Entity)
{
    #if __CRASHSAFE_LEVEL__ >= 2
    CHECK_OBJECT_RTN(Entity, "AnimTime", "Entity", 0.0)
    #endif
    return NODE(Entity)->getAnimation(0)->getSeek();
}

SPW_PROC(void) spwSetAnimKey(
    LP Entity, int Frame, bool PosKey = true, bool RotKey = true, bool SclKey = true)
{
    #if __CRASHSAFE_LEVEL__ >= 2
    CHECK_OBJECT(Entity, "SetAnimKey", "Entity")
    #endif
    
    // Create animation if not exist
    scene::SceneNode* Object     = NODE(Entity);
    scene::Animation* Anim  = Object->getAnimation();
    
    if (Anim && Anim->getType() != scene::ANIMATION_NODE)
        return;
    else if (!Anim)
        Anim = Object->addAnimation(scene::ANIMATION_NODE);
    
    scene::AnimationNode* AnimNode = (scene::AnimationNode*)Anim;
    
    // Add animation keyframe
    AnimNode->addSequence(
        Object->getPosition(), Object->getRotationMatrix(), Object->getScale(), 0.1
    );
}

SPW_PROC(void) spwAddAnimSeq(LP Entity, int Length)
{
    #if __CRASHSAFE_LEVEL__ >= 2
    CHECK_OBJECT(Entity, "AddAnimSeq", "Entity")
    #endif
    
    // !TODO!
}

SPW_PROC(void) spwAnimSpline(LP Entity, bool Enable, float Expansion = 1.0)
{
    #if __CRASHSAFE_LEVEL__ >= 2
    CHECK_OBJECT(Entity, "AnimSpline", "Entity")
    #endif
    
    scene::Animation* Anim = NODE(Entity)->getAnimation();
    
    if (Anim && Anim->getType() == scene::ANIMATION_NODE)
    {
        scene::AnimationNode* AnimNode = (scene::AnimationNode*)Anim;
        AnimNode->setSplineTranslation(Enable);
        if (Enable)
            AnimNode->setSplineExpansion(Expansion);
    }
}

SPW_PROC(int) spwExtractAnimSeq(LP Entity, int FirstFrame, int LastFrame, int Sequence = 0)
{
    #if __CRASHSAFE_LEVEL__ >= 2
    CHECK_OBJECT_RTN(Entity, "EntityParent", "Entity", 0)
    #endif
    
    SWrapAnimSeq AnimSeq;
    {
        AnimSeq.FirstFrame  = FirstFrame;
        AnimSeq.LastFrame   = LastFrame;
    }
    g_AnimSeqList.push_back(AnimSeq);
    
    return g_AnimSeqList.size();
}

SPW_PROC(void) spwEntityParent(LP Entity, LP Parent, bool Global = true)
{
    #if __CRASHSAFE_LEVEL__ >= 2
    CHECK_OBJECT(Entity, "EntityParent", "Entity")
    #endif
    NODE(Entity)->setParent(NODE(Parent), Global);
}

SPW_PROC(LP) spwGetParent(LP Entity)
{
    #if __CRASHSAFE_LEVEL__ >= 2
    CHECK_OBJECT_RTN(Entity, "GetParent", "Entity", 0)
    #endif
    return CAST(NODE(Entity)->getParent());
}

SPW_PROC(int) spwCountChildren(LP Entity)
{
    #if __CRASHSAFE_LEVEL__ >= 2
    CHECK_OBJECT_RTN(Entity, "CountChildren", "Entity", 0)
    #endif
    return g_SceneGraph->findChildren(NODE(Entity)).size();
}

SPW_PROC(LP) spwGetChild(LP Entity, int Index)
{
    #if __CRASHSAFE_LEVEL__ >= 2
    CHECK_OBJECT_RTN(Entity, "GetChild", "Entity", 0)
    #endif
    
    std::list<scene::SceneNode*> ChildList = g_SceneGraph->findChildren(NODE(Entity));
    
    if (Index > ChildList.size())
        return 0;
    
    std::list<scene::SceneNode*>::iterator it = ChildList.begin();
    
    for (int i = 1; i < Index; ++i)
        ++it;
    
    return CAST(*it);
}

SPW_PROC(void) spwShowEntity(LP Entity)
{
    #if __CRASHSAFE_LEVEL__ >= 2
    CHECK_OBJECT(Entity, "ShowEntity", "Entity")
    #endif
    NODE(Entity)->setVisible(true);
}

SPW_PROC(void) spwHideEntity(LP Entity)
{
    #if __CRASHSAFE_LEVEL__ >= 2
    CHECK_OBJECT(Entity, "FreeEntity", "Entity")
    #endif
    NODE(Entity)->setVisible(false);
}

SPW_PROC(void) spwFreeEntity(LP Entity)
{
    #if __CRASHSAFE_LEVEL__ >= 2
    CHECK_OBJECT(Entity, "FreeEntity", "Entity")
    #endif
    
    switch (NODE(Entity)->getType())
    {
        case scene::NODE_MESH:
            g_SceneGraph->deleteNode(MESH(Entity)); break;
        case scene::NODE_BILLBOARD:
            g_SceneGraph->deleteNode(BILLBOARD(Entity)); break;
        case scene::NODE_TERRAIN:
            g_SceneGraph->deleteNode(TERRAIN(Entity)); break;
        case scene::NODE_CAMERA:
            g_CameraList.erase(Entity);
            g_SceneGraph->deleteNode(CAMERA(Entity));
            break;
        case scene::NODE_LIGHT:
            g_SceneGraph->deleteNode(LIGHT(Entity));
            g_SceneGraph->setLighting(!g_SceneGraph->getLightList().empty());
            break;
    }
}

SPW_PROC(void) spwPositionEntity(LP Entity, float X, float Y, float Z, bool Global = false)
{
    #if __CRASHSAFE_LEVEL__ >= 2
    CHECK_OBJECT(Entity, "PositionEntity", "Entity")
    #endif
    NODE(Entity)->setPosition(dim::vector3df(X, Y, Z), Global);
}

SPW_PROC(void) spwMoveEntity(LP Entity, float X, float Y, float Z)
{
    #if __CRASHSAFE_LEVEL__ >= 2
    CHECK_OBJECT(Entity, "MoveEntity", "Entity")
    #endif
    NODE(Entity)->move(dim::vector3df(X, Y, Z));
}

SPW_PROC(void) spwTranslateEntity(LP Entity, float X, float Y, float Z, bool Global = false)
{
    #if __CRASHSAFE_LEVEL__ >= 2
    CHECK_OBJECT(Entity, "TranslateEntity", "Entity")
    #endif
    NODE(Entity)->setPosition(NODE(Entity)->getPosition(Global) + dim::vector3df(X, Y, Z), Global);
}

SPW_PROC(void) spwRotateEntity(LP Entity, float X, float Y, float Z, bool Global = false)
{
    #if __CRASHSAFE_LEVEL__ >= 2
    CHECK_OBJECT(Entity, "RotateEntity", "Entity")
    #endif
    NODE(Entity)->setRotation(dim::vector3df(X, -Y, Z), Global);
}

SPW_PROC(void) spwTurnEntity(LP Entity, float X, float Y, float Z, bool Global = false)
{
    #if __CRASHSAFE_LEVEL__ >= 2
    CHECK_OBJECT(Entity, "TurnEntity", "Entity")
    #endif
    
    //NODE(Entity)->setRotation(NODE(Entity)->getRotation(Global) + dim::vector3df(X, Y, Z), Global);
    if (Global)
    {
        dim::matrix4f Mat(NODE(Entity)->getRotationMatrix(true));
        Mat.rotateYXZ(dim::vector3df(X, -Y, Z));
        NODE(Entity)->setRotationMatrix(Mat, true);
    }
    else
        NODE(Entity)->turn(dim::vector3df(X, -Y, Z));
}

SPW_PROC(void) spwScaleEntity(LP Entity, float X, float Y, float Z, bool Global = false)
{
    #if __CRASHSAFE_LEVEL__ >= 2
    CHECK_OBJECT(Entity, "ScaleEntity", "Entity")
    #endif
    
    scene::SceneNode* Obj = NODE(Entity);
    if (Obj->getType() == scene::NODE_BILLBOARD)
        Obj->setScale(dim::vector3df(X, Y, Z) * 0.5, Global);
    else
        Obj->setScale(dim::vector3df(X, Y, Z), Global);
}

SPW_PROC(void) spwPointEntity(LP Entity, LP Target, float Roll = 0.0)
{
    #if __CRASHSAFE_LEVEL__ >= 2
    CHECK_OBJECT(Entity, "PointEntity", "Entity")
    #endif
    
    scene::SceneNode* Obj = NODE(Entity);
    Obj->lookAt(NODE(Target)->getPosition(true), true);
    Obj->turn(dim::vector3df(0, 0, Roll));
}

SPW_PROC(void) spwRotateEntityQuat(LP Entity, float X, float Y, float Z, float W, bool Global = false)
{
    #if __CRASHSAFE_LEVEL__ >= 2
    CHECK_OBJECT(Entity, "RotateEntityQuat", "Entity")
    #endif
    NODE(Entity)->setRotationMatrix(dim::quaternion(X, Y, Z, W).getMatrix(), Global);
}

SPW_PROC(void) spwScaleMesh(LP Mesh, float X, float Y, float Z)
{
    #if __CRASHSAFE_LEVEL__ >= 2
    CHECK_OBJECT(Mesh, "ScaleMesh", "Mesh")
    #endif
    MESH(Mesh)->meshTransform(dim::vector3df(X, Y, Z));
}

SPW_PROC(void) spwFlipMesh(LP Mesh)
{
    #if __CRASHSAFE_LEVEL__ >= 2
    CHECK_OBJECT(Mesh, "FlipMesh", "Mesh")
    #endif
    MESH(Mesh)->flipTriangles();
}

SPW_PROC(void) spwFitMesh(
    LP Mesh, float X, float Y, float Z, float Width, float Height, float Depth, bool Uniform = false)
{
    #if __CRASHSAFE_LEVEL__ >= 2
    CHECK_OBJECT(Mesh, "FitMesh", "Mesh")
    #endif
    
    if (Width < 0)
    {
        X += Width;
        Width = -Width;
    }
    if (Height < 0)
    {
        Y += Height;
        Height = -Height;
    }
    if (Depth < 0)
    {
        Z += Depth;
        Depth = -Depth;
    }
    
    MESH(Mesh)->meshFit(dim::vector3df(X, Y, Z), dim::vector3df(Width, Height, Depth));
}

SPW_PROC(void) spwRotateMesh(LP Mesh, float X, float Y, float Z)
{
    #if __CRASHSAFE_LEVEL__ >= 2
    CHECK_OBJECT(Mesh, "RotateMesh", "Mesh")
    #endif
    MESH(Mesh)->meshTurn(dim::vector3df(X, -Y, Z));
}

SPW_PROC(void) spwPositionMesh(LP Mesh, float X, float Y, float Z)
{
    #if __CRASHSAFE_LEVEL__ >= 2
    CHECK_OBJECT(Mesh, "PositionMesh", "Mesh")
    #endif
    MESH(Mesh)->meshTranslate(dim::vector3df(X, Y, Z));
}

SPW_PROC(void) spwUpdateNormals(LP Mesh)
{
    #if __CRASHSAFE_LEVEL__ >= 2
    CHECK_OBJECT(Mesh, "UpdateNormals", "Mesh")
    #endif
    MESH(Mesh)->updateNormals();
}

SPW_PROC(void) spwUpdateTangentSpace_ex(LP Mesh)
{
    #if __CRASHSAFE_LEVEL__ >= 2
    CHECK_OBJECT(Mesh, "UpdateTangentSpace", "Mesh")
    #endif
    MESH(Mesh)->updateTangentSpace();
}

SPW_PROC(void) spwUpdateTangentSpaceTexLayers_ex(LP Mesh, int TangentLayer, int BinormalLayer)
{
    #if __CRASHSAFE_LEVEL__ >= 2
    CHECK_OBJECT(Mesh, "UpdateTangentSpaceTexLayers", "Mesh")
    #endif
    MESH(Mesh)->updateTangentSpace((u8)TangentLayer, (u8)BinormalLayer);
}

SPW_PROC(LP) spwCreateSurface(LP Mesh, LP Brush = 0)
{
    #if __CRASHSAFE_LEVEL__ >= 2
    if (!Mesh)
    {
        WrapperError("CreateSurface", "\"Mesh\" is invalid");
        return 0;
    }
    #endif
    
    SWrapSurface Surface;
    {
        Surface.SurfaceNr   = MESH(Mesh)->getMeshBufferCount();
        Surface.MeshBuffer  = MESH(Mesh)->createMeshBuffer();
        Surface.Mesh        = Mesh;
        Surface.Brush       = Brush;
    }
    g_SurfaceList[++g_WholeSurfaceCount] = Surface;
    
    if (Brush)
        spwPaintSurface(g_WholeSurfaceCount, Brush);
    
    return g_WholeSurfaceCount;
}

SPW_PROC(void) spwFreeSurface_ex(LP Surface)
{
    SWrapSurface SurfaceData = g_SurfaceList[Surface];
    MESH(SurfaceData.Mesh)->deleteMeshBuffer(SurfaceData.SurfaceNr);
}

SPW_PROC(void) spwClearSurface(LP Surface, bool ClearVertices = true, bool ClearTriangles = true)
{
    #if __CRASHSAFE_LEVEL__ >= 2
    if (!Surface)
    {
        WrapperError("ClearSurface", "\"Surface\" is invalid");
        return;
    }
    #endif
    
    std::map<LP, SWrapSurface>::iterator it = g_SurfaceList.find(Surface);
    
    #if __CRASHSAFE_LEVEL__ >= 1
    if (it == g_SurfaceList.end())
    {
        WrapperError("PaintSufrace", "\"Surface\" is invalid");
        return;
    }
    #endif
    
    if (ClearVertices)
        it->second.MeshBuffer->clearVertices();
    if (ClearTriangles)
        it->second.MeshBuffer->clearIndices();
}

SPW_PROC(LP) spwGetSurface(LP Mesh, int Index)
{
    for (std::map<LP, SWrapSurface>::iterator it = g_SurfaceList.begin(); it != g_SurfaceList.end(); ++it)
    {
        if (it->second.Mesh == Mesh && it->second.SurfaceNr + 1 == Index)
            return it->first;
    }
    return 0;
}

SPW_PROC(LP) spwFindSurface(LP Mesh, LP Brush)
{
    for (std::map<LP, SWrapSurface>::iterator it = g_SurfaceList.begin(); it != g_SurfaceList.end(); ++it)
    {
        if (it->second.Mesh == Mesh && it->second.Brush == Brush)
            return it->first;
    }
    return 0;
}

SPW_PROC(int) spwAddVertex(LP Surface, float X, float Y, float Z, float U = 0.0, float V = 0.0, float W = 0.0)
{
    s32 Index = -1;
    SWrapSurface SurfaceData = g_SurfaceList[Surface];
    scene::Mesh* Mesh = MESH(SurfaceData.Mesh);
    
    if (Mesh)
    {
        Index = static_cast<s32>(
            SurfaceData.MeshBuffer->addVertex(dim::vector3df(X, Y, Z), dim::vector3df(U, V, W))
        );
        SurfaceData.MeshBuffer->updateVertexBuffer();
    }
    
    return Index;
}

SPW_PROC(int) spwAddTriangle(LP Surface, int v0, int v1, int v2)
{
    s32 Index = -1;
    SWrapSurface SurfaceData = g_SurfaceList[Surface];
    scene::Mesh* Mesh = MESH(SurfaceData.Mesh);
    
    if (Mesh)
    {
        Index = (s32)SurfaceData.MeshBuffer->addTriangle(v0, v1, v2);
        SurfaceData.MeshBuffer->updateIndexBuffer();
    }
    
    return Index;
}

SPW_PROC(void) spwPatchAnchorPoint_ex(int IndexVert, int IndexHorz, float X, float Y, float Z)
{
    if (IndexVert >= 0 && IndexVert < 4 && IndexHorz >= 0 && IndexHorz < 4)
        g_PatchAnchorPoints[IndexVert][IndexHorz] = dim::vector3df(X, Y, Z);
}

SPW_PROC(void) spwAddPatchAnchorPoint_ex(float X, float Y, float Z)
{
    if (g_PatchAnchorIndexVert < 4 && g_PatchAnchorIndexHorz < 4)
        g_PatchAnchorPoints[g_PatchAnchorIndexVert][g_PatchAnchorIndexHorz] = dim::vector3df(X, Y, Z);
    
    if (++g_PatchAnchorIndexHorz > 3)
    {
        g_PatchAnchorIndexHorz = 0;
        ++g_PatchAnchorIndexVert;
    }
}

SPW_PROC(void) spwAddBezierPatch(LP Surface, int Segments = 10)
{
    SWrapSurface SurfaceData = g_SurfaceList[Surface];
    scene::BasicMeshGenerator().createBezierPatchFace(
        MESH(SurfaceData.Mesh), SurfaceData.SurfaceNr, g_PatchAnchorPoints, Segments
    );
    g_PatchAnchorIndexVert = 0;
    g_PatchAnchorIndexHorz = 0;
}

SPW_PROC(int) spwCountSurfaces(LP Mesh)
{
    return MESH(Mesh)->getMeshBufferCount();
}

SPW_PROC(int) spwCountVertices(LP Surface)
{
    return g_SurfaceList[Surface].MeshBuffer->getVertexCount();
}

SPW_PROC(int) spwCountTriangles(LP Surface)
{
    return g_SurfaceList[Surface].MeshBuffer->getTriangleCount();
}

SPW_PROC(void) spwVertexCoords(LP Surface, int Index, float X, float Y, float Z)
{
    std::map<LP, SWrapSurface>::iterator it = g_SurfaceList.find(Surface);
    
    #if __CRASHSAFE_LEVEL__ >= 2
    if (it == g_SurfaceList.end())
    {
        WrapperError("VertexCoords", "\"Surface\" is invalid");
        return;
    }
    #endif
    
    SWrapSurface SurfaceData = it->second;
    
    if (SurfaceData.MeshBuffer)
    {
        SurfaceData.MeshBuffer->setVertexCoord(Index, dim::vector3df(X, Y, Z));
        SurfaceData.MeshBuffer->updateVertexBufferElement(Index);
    }
}

SPW_PROC(void) spwVertexNormal(LP Surface, int Index, float X, float Y, float Z)
{
    std::map<LP, SWrapSurface>::iterator it = g_SurfaceList.find(Surface);
    
    #if __CRASHSAFE_LEVEL__ >= 2
    if (it == g_SurfaceList.end())
    {
        WrapperError("VertexCoords", "\"Surface\" is invalid");
        return;
    }
    #endif
    
    SWrapSurface SurfaceData = it->second;
    
    if (SurfaceData.MeshBuffer)
    {
        SurfaceData.MeshBuffer->setVertexNormal(Index, dim::vector3df(X, Y, Z));
        SurfaceData.MeshBuffer->updateVertexBufferElement(Index);
    }
}

SPW_PROC(void) spwVertexBinormal(LP Surface, int Index, float X, float Y, float Z)
{
    std::map<LP, SWrapSurface>::iterator it = g_SurfaceList.find(Surface);
    
    #if __CRASHSAFE_LEVEL__ >= 2
    if (it == g_SurfaceList.end())
    {
        WrapperError("VertexCoords", "\"Surface\" is invalid");
        return;
    }
    #endif
    
    SWrapSurface SurfaceData = it->second;
    
    if (SurfaceData.MeshBuffer)
    {
        SurfaceData.MeshBuffer->setVertexBinormal(Index, dim::vector3df(X, Y, Z));
        SurfaceData.MeshBuffer->updateVertexBufferElement(Index);
    }
}

SPW_PROC(void) spwVertexTangent(LP Surface, int Index, float X, float Y, float Z)
{
    std::map<LP, SWrapSurface>::iterator it = g_SurfaceList.find(Surface);
    
    #if __CRASHSAFE_LEVEL__ >= 2
    if (it == g_SurfaceList.end())
    {
        WrapperError("VertexCoords", "\"Surface\" is invalid");
        return;
    }
    #endif
    
    SWrapSurface SurfaceData = it->second;
    
    if (SurfaceData.MeshBuffer)
    {
        SurfaceData.MeshBuffer->setVertexTangent(Index, dim::vector3df(X, Y, Z));
        SurfaceData.MeshBuffer->updateVertexBufferElement(Index);
    }
}

SPW_PROC(void) spwVertexColor(LP Surface, int Index, int Red, int Green, int Blue, float Alpha = 1.0)
{
    #if __CRASHSAFE_LEVEL__ >= 2
    CHECK_OBJECT(Surface, "VertexCoords", "Surface")
    #endif
    
    std::map<LP, SWrapSurface>::iterator it = g_SurfaceList.find(Surface);
    
    #if __CRASHSAFE_LEVEL__ >= 2
    if (it == g_SurfaceList.end())
    {
        WrapperError("VertexCoords", "\"Surface\" is corrupt");
        return;
    }
    #endif
    
    SWrapSurface SurfaceData = it->second;
    
    if (SurfaceData.MeshBuffer)
    {
        SurfaceData.MeshBuffer->setVertexColor(Index, video::color(Red, Green, Blue, (int)(Alpha*255)));
        SurfaceData.MeshBuffer->updateVertexBufferElement(Index);
    }
}

SPW_PROC(void) spwVertexTexCoords(LP Surface, int Index, float U, float V, float W = 0.0, int Layer = 0)
{
    #if __CRASHSAFE_LEVEL__ >= 2
    CHECK_OBJECT(Surface, "VertexCoords", "Surface")
    #endif
    
    std::map<LP, SWrapSurface>::iterator it = g_SurfaceList.find(Surface);
    
    #if __CRASHSAFE_LEVEL__ >= 2
    if (it == g_SurfaceList.end())
    {
        WrapperError("VertexCoords", "\"Surface\" is corrupt");
        return;
    }
    #endif
    
    SWrapSurface SurfaceData = it->second;
    
    if (SurfaceData.MeshBuffer)
    {
        SurfaceData.MeshBuffer->setVertexTexCoord(Index, dim::vector3df(U, V, W), Layer);
        SurfaceData.MeshBuffer->updateVertexBufferElement(Index);
    }
}

SPW_PROC(void) spwUpdateMeshImmediate_ex(LP Mesh, bool Enable)
{
    #if __CRASHSAFE_LEVEL__ >= 2
    CHECK_OBJECT(Mesh, "UpdateMeshImmediate", "Mesh")
    #endif
    #if __CRASHSAFE_LEVEL__ >= 1
    CHECK_OBJTYPE(Mesh, scene::NODE_MESH, "UpdateMeshImmediate", "\"Mesh\" is not a mesh")
    #endif
    
    for (u32 s = 0; s < MESH(Mesh)->getMeshBufferCount(); ++s)
        MESH(Mesh)->getMeshBuffer(s)->setUpdateImmediate(Enable);
}

SPW_PROC(void) spwUpdateMeshBuffer(LP Mesh, LP Surface = 0)
{
    #if __CRASHSAFE_LEVEL__ >= 2
    CHECK_OBJECT(Mesh, "UpdateMeshImmediate", "Mesh")
    CHECK_OBJECT(Surface, "UpdateMeshImmediate", "Surface")
    #endif
    #if __CRASHSAFE_LEVEL__ >= 1
    CHECK_OBJTYPE(Mesh, scene::NODE_MESH, "UpdateMeshImmediate", "\"Mesh\" is not a mesh")
    #endif
    
    if (Surface)
        g_SurfaceList[Surface].MeshBuffer->updateMeshBuffer();
    else
        MESH(Mesh)->updateMeshBuffer();
}

SPW_PROC(float) spwVertexX(LP Surface, int Index)
{
    #if __CRASHSAFE_LEVEL__ >= 2
    CHECK_OBJECT_RTN(Surface, "VertexX", "Surface", 0.0)
    #endif
    
    SWrapSurface SurfaceData = g_SurfaceList[Surface];
    
    #if __CRASHSAFE_LEVEL__ >= 2
    CHECK_POINTER_RTN(SurfaceData.Mesh, "VertexX", "Surface refers to invalid mesh", 0.0)
    CHECK_OBJTYPE_RTN(SurfaceData.Mesh, scene::NODE_MESH, "VertexX", "Surface refers to an object which is not a mesh", 0.0)
    #endif
    
    return SurfaceData.MeshBuffer->getVertexCoord(Index).X;
}

SPW_PROC(float) spwVertexY(LP Surface, int Index)
{
    #if __CRASHSAFE_LEVEL__ >= 2
    CHECK_OBJECT_RTN(Surface, "VertexY", "Surface", 0.0)
    #endif
    
    SWrapSurface SurfaceData = g_SurfaceList[Surface];
    
    #if __CRASHSAFE_LEVEL__ >= 2
    CHECK_POINTER_RTN(SurfaceData.Mesh, "VertexY", "Surface refers to invalid mesh", 0.0)
    CHECK_OBJTYPE_RTN(SurfaceData.Mesh, scene::NODE_MESH, "VertexY", "Surface refers to an object which is not a mesh", 0.0)
    #endif
    
    return SurfaceData.MeshBuffer->getVertexCoord(Index).Y;
}

SPW_PROC(float) spwVertexZ(LP Surface, int Index)
{
    #if __CRASHSAFE_LEVEL__ >= 2
    CHECK_OBJECT_RTN(Surface, "VertexZ", "Surface", 0.0)
    #endif
    
    SWrapSurface SurfaceData = g_SurfaceList[Surface];
    
    #if __CRASHSAFE_LEVEL__ >= 2
    CHECK_POINTER_RTN(SurfaceData.Mesh, "VertexZ", "Surface refers to invalid mesh", 0.0)
    CHECK_OBJTYPE_RTN(SurfaceData.Mesh, scene::NODE_MESH, "VertexZ", "Surface refers to an object which is not a mesh", 0.0)
    #endif
    
    return SurfaceData.MeshBuffer->getVertexCoord(Index).Z;
}

SPW_PROC(float) spwVertexNX(LP Surface, int Index)
{
    #if __CRASHSAFE_LEVEL__ >= 2
    CHECK_OBJECT_RTN(Surface, "VertexNX", "Surface", 0.0)
    #endif
    
    SWrapSurface SurfaceData = g_SurfaceList[Surface];
    
    #if __CRASHSAFE_LEVEL__ >= 2
    CHECK_POINTER_RTN(SurfaceData.Mesh, "VertexNX", "Surface refers to invalid mesh", 0.0)
    CHECK_OBJTYPE_RTN(SurfaceData.Mesh, scene::NODE_MESH, "VertexNX", "Surface refers to an object which is not a mesh", 0.0)
    #endif
    
    return SurfaceData.MeshBuffer->getVertexNormal(Index).X;
}

SPW_PROC(float) spwVertexNY(LP Surface, int Index)
{
    #if __CRASHSAFE_LEVEL__ >= 2
    CHECK_OBJECT_RTN(Surface, "VertexNY", "Surface", 0.0)
    #endif
    
    SWrapSurface SurfaceData = g_SurfaceList[Surface];
    
    #if __CRASHSAFE_LEVEL__ >= 2
    CHECK_POINTER_RTN(SurfaceData.Mesh, "VertexNY", "Surface refers to invalid mesh", 0.0)
    CHECK_OBJTYPE_RTN(SurfaceData.Mesh, scene::NODE_MESH, "VertexNY", "Surface refers to an object which is not a mesh", 0.0)
    #endif
    
    return SurfaceData.MeshBuffer->getVertexNormal(Index).Y;
}

SPW_PROC(float) spwVertexNZ(LP Surface, int Index)
{
    #if __CRASHSAFE_LEVEL__ >= 2
    CHECK_OBJECT_RTN(Surface, "VertexNZ", "Surface", 0.0)
    #endif
    
    SWrapSurface SurfaceData = g_SurfaceList[Surface];
    
    #if __CRASHSAFE_LEVEL__ >= 2
    CHECK_POINTER_RTN(SurfaceData.Mesh, "VertexNZ", "Surface refers to invalid mesh", 0.0)
    CHECK_OBJTYPE_RTN(SurfaceData.Mesh, scene::NODE_MESH, "VertexNZ", "Surface refers to an object which is not a mesh", 0.0)
    #endif
    
    return SurfaceData.MeshBuffer->getVertexNormal(Index).Z;
}

SPW_PROC(float) spwVertexBX(LP Surface, int Index)
{
    #if __CRASHSAFE_LEVEL__ >= 2
    CHECK_OBJECT_RTN(Surface, "VertexBX", "Surface", 0.0)
    #endif
    
    SWrapSurface SurfaceData = g_SurfaceList[Surface];
    
    #if __CRASHSAFE_LEVEL__ >= 2
    CHECK_POINTER_RTN(SurfaceData.Mesh, "VertexBX", "Surface refers to invalid mesh", 0.0)
    CHECK_OBJTYPE_RTN(SurfaceData.Mesh, scene::NODE_MESH, "VertexBX", "Surface refers to an object which is not a mesh", 0.0)
    #endif
    
    return SurfaceData.MeshBuffer->getVertexBinormal(Index).X;
}

SPW_PROC(float) spwVertexBY(LP Surface, int Index)
{
    #if __CRASHSAFE_LEVEL__ >= 2
    CHECK_OBJECT_RTN(Surface, "VertexBY", "Surface", 0.0)
    #endif
    
    SWrapSurface SurfaceData = g_SurfaceList[Surface];
    
    #if __CRASHSAFE_LEVEL__ >= 2
    CHECK_POINTER_RTN(SurfaceData.Mesh, "VertexBY", "Surface refers to invalid mesh", 0.0)
    CHECK_OBJTYPE_RTN(SurfaceData.Mesh, scene::NODE_MESH, "VertexBY", "Surface refers to an object which is not a mesh", 0.0)
    #endif
    
    return SurfaceData.MeshBuffer->getVertexBinormal(Index).Y;
}

SPW_PROC(float) spwVertexBZ(LP Surface, int Index)
{
    #if __CRASHSAFE_LEVEL__ >= 2
    CHECK_OBJECT_RTN(Surface, "VertexBZ", "Surface", 0.0)
    #endif
    
    SWrapSurface SurfaceData = g_SurfaceList[Surface];
    
    #if __CRASHSAFE_LEVEL__ >= 2
    CHECK_POINTER_RTN(SurfaceData.Mesh, "VertexBZ", "Surface refers to invalid mesh", 0.0)
    CHECK_OBJTYPE_RTN(SurfaceData.Mesh, scene::NODE_MESH, "VertexBZ", "Surface refers to an object which is not a mesh", 0.0)
    #endif
    
    return SurfaceData.MeshBuffer->getVertexBinormal(Index).Z;
}

SPW_PROC(float) spwVertexTX(LP Surface, int Index)
{
    #if __CRASHSAFE_LEVEL__ >= 2
    CHECK_OBJECT_RTN(Surface, "VertexTX", "Surface", 0.0)
    #endif
    
    SWrapSurface SurfaceData = g_SurfaceList[Surface];
    
    #if __CRASHSAFE_LEVEL__ >= 2
    CHECK_POINTER_RTN(SurfaceData.Mesh, "VertexTX", "Surface refers to invalid mesh", 0.0)
    CHECK_OBJTYPE_RTN(SurfaceData.Mesh, scene::NODE_MESH, "VertexTX", "Surface refers to an object which is not a mesh", 0.0)
    #endif
    
    return SurfaceData.MeshBuffer->getVertexTangent(Index).X;
}

SPW_PROC(float) spwVertexTY(LP Surface, int Index)
{
    #if __CRASHSAFE_LEVEL__ >= 2
    CHECK_OBJECT_RTN(Surface, "VertexTY", "Surface", 0.0)
    #endif
    
    SWrapSurface SurfaceData = g_SurfaceList[Surface];
    
    #if __CRASHSAFE_LEVEL__ >= 2
    CHECK_POINTER_RTN(SurfaceData.Mesh, "VertexTY", "Surface refers to invalid mesh", 0.0)
    CHECK_OBJTYPE_RTN(SurfaceData.Mesh, scene::NODE_MESH, "VertexTY", "Surface refers to an object which is not a mesh", 0.0)
    #endif
    
    return SurfaceData.MeshBuffer->getVertexTangent(Index).Y;
}

SPW_PROC(float) spwVertexTZ(LP Surface, int Index)
{
    #if __CRASHSAFE_LEVEL__ >= 2
    CHECK_OBJECT_RTN(Surface, "VertexTZ", "Surface", 0.0)
    #endif
    
    SWrapSurface SurfaceData = g_SurfaceList[Surface];
    
    #if __CRASHSAFE_LEVEL__ >= 2
    CHECK_POINTER_RTN(SurfaceData.Mesh, "VertexTZ", "Surface refers to invalid mesh", 0.0)
    CHECK_OBJTYPE_RTN(SurfaceData.Mesh, scene::NODE_MESH, "VertexTZ", "Surface refers to an object which is not a mesh", 0.0)
    #endif
    
    return SurfaceData.MeshBuffer->getVertexTangent(Index).Z;
}

SPW_PROC(int) spwVertexRed(LP Surface, int Index)
{
    #if __CRASHSAFE_LEVEL__ >= 2
    CHECK_OBJECT_RTN(Surface, "VertexRed", "Surface", 0)
    #endif
    
    SWrapSurface SurfaceData = g_SurfaceList[Surface];
    
    #if __CRASHSAFE_LEVEL__ >= 2
    CHECK_POINTER_RTN(SurfaceData.Mesh, "VertexRed", "Surface refers to invalid mesh", 0)
    CHECK_OBJTYPE_RTN(SurfaceData.Mesh, scene::NODE_MESH, "VertexRed", "Surface refers to an object which is not a mesh", 0)
    #endif
    
    return SurfaceData.MeshBuffer->getVertexColor(Index).Red;
}

SPW_PROC(int) spwVertexGreen(LP Surface, int Index)
{
    #if __CRASHSAFE_LEVEL__ >= 2
    CHECK_OBJECT_RTN(Surface, "VertexGreen", "Surface", 0)
    #endif
    
    SWrapSurface SurfaceData = g_SurfaceList[Surface];
    
    #if __CRASHSAFE_LEVEL__ >= 2
    CHECK_POINTER_RTN(SurfaceData.Mesh, "VertexGreen", "Surface refers to invalid mesh", 0)
    CHECK_OBJTYPE_RTN(SurfaceData.Mesh, scene::NODE_MESH, "VertexGreen", "Surface refers to an object which is not a mesh", 0)
    #endif
    
    return SurfaceData.MeshBuffer->getVertexColor(Index).Green;
}

SPW_PROC(int) spwVertexBlue(LP Surface, int Index)
{
    #if __CRASHSAFE_LEVEL__ >= 2
    CHECK_OBJECT_RTN(Surface, "VertexBlue", "Surface", 0)
    #endif
    
    SWrapSurface SurfaceData = g_SurfaceList[Surface];
    
    #if __CRASHSAFE_LEVEL__ >= 2
    CHECK_POINTER_RTN(SurfaceData.Mesh, "VertexBlue", "Surface refers to invalid mesh", 0)
    CHECK_OBJTYPE_RTN(SurfaceData.Mesh, scene::NODE_MESH, "VertexBlue", "Surface refers to an object which is not a mesh", 0)
    #endif
    
    return SurfaceData.MeshBuffer->getVertexColor(Index).Blue;
}

SPW_PROC(float) spwVertexAlpha(LP Surface, int Index)
{
    #if __CRASHSAFE_LEVEL__ >= 2
    CHECK_OBJECT_RTN(Surface, "VertexAlpha", "Surface", 0)
    #endif
    
    SWrapSurface SurfaceData = g_SurfaceList[Surface];
    
    #if __CRASHSAFE_LEVEL__ >= 2
    CHECK_POINTER_RTN(SurfaceData.Mesh, "VertexAlpha", "Surface refers to invalid mesh", 0)
    CHECK_OBJTYPE_RTN(SurfaceData.Mesh, scene::NODE_MESH, "VertexAlpha", "Surface refers to an object which is not a mesh", 0)
    #endif
    
    return (float)(SurfaceData.MeshBuffer->getVertexColor(Index).Alpha) / 255;
}

SPW_PROC(float) spwVertexU(LP Surface, int Index)
{
    #if __CRASHSAFE_LEVEL__ >= 2
    CHECK_OBJECT_RTN(Surface, "VertexU", "Surface", 0.0)
    #endif
    
    SWrapSurface SurfaceData = g_SurfaceList[Surface];
    
    #if __CRASHSAFE_LEVEL__ >= 2
    CHECK_POINTER_RTN(SurfaceData.Mesh, "VertexU", "Surface refers to invalid mesh", 0.0)
    CHECK_OBJTYPE_RTN(SurfaceData.Mesh, scene::NODE_MESH, "VertexU", "Surface refers to an object which is not a mesh", 0.0)
    #endif
    
    return SurfaceData.MeshBuffer->getVertexTexCoord(Index).X;
}

SPW_PROC(float) spwVertexV(LP Surface, int Index)
{
    #if __CRASHSAFE_LEVEL__ >= 2
    CHECK_OBJECT_RTN(Surface, "VertexV", "Surface", 0.0)
    #endif
    
    SWrapSurface SurfaceData = g_SurfaceList[Surface];
    
    #if __CRASHSAFE_LEVEL__ >= 2
    CHECK_POINTER_RTN(SurfaceData.Mesh, "VertexV", "Surface refers to invalid mesh", 0.0)
    CHECK_OBJTYPE_RTN(SurfaceData.Mesh, scene::NODE_MESH, "VertexV", "Surface refers to an object which is not a mesh", 0.0)
    #endif
    
    return SurfaceData.MeshBuffer->getVertexTexCoord(Index).Y;
}

SPW_PROC(float) spwVertexW(LP Surface, int Index)
{
    #if __CRASHSAFE_LEVEL__ >= 2
    CHECK_OBJECT_RTN(Surface, "VertexW", "Surface", 0.0)
    #endif
    
    SWrapSurface SurfaceData = g_SurfaceList[Surface];
    
    #if __CRASHSAFE_LEVEL__ >= 2
    CHECK_POINTER_RTN(SurfaceData.Mesh, "VertexW", "Surface refers to invalid mesh", 0.0)
    CHECK_OBJTYPE_RTN(SurfaceData.Mesh, scene::NODE_MESH, "VertexW", "Surface refers to an object which is not a mesh", 0.0)
    #endif
    
    return SurfaceData.MeshBuffer->getVertexTexCoord(Index).Z;
}

SPW_PROC(int) spwTriangleVertex(LP Surface, int Index, int Corner)
{
    SWrapSurface SurfaceData = g_SurfaceList[Surface];
    
    unsigned int Indices[3] = { 0 };
    
    SurfaceData.MeshBuffer->getTriangleIndices(Index, Indices);
    
    math::Clamp(Corner, 0, 2);
    
    return Indices[Corner];
}

SPW_PROC(void) spwEntityColor(LP Entity, int Red, int Green, int Blue)
{
    scene::ENodeTypes Type = NODE(Entity)->getType();
    
    if (Type == scene::NODE_MESH)
    {
        video::color* Color = &MESH(Entity)->getMaterial()->getDiffuseColor();
        Color->Red      = Red;
        Color->Green    = Green;
        Color->Blue     = Blue;
        
        Color = &MESH(Entity)->getMaterial()->getAmbientColor();
        Color->Red      = Red / 2;
        Color->Green    = Green / 2;
        Color->Blue     = Blue / 2;
    }
    else if (Type == scene::NODE_BILLBOARD)
    {
        video::color Color(BILLBOARD(Entity)->getColor());
        Color.Red   = Red;
        Color.Green = Green;
        Color.Blue  = Blue;
        BILLBOARD(Entity)->setColor(Color);
    }
}

SPW_PROC(void) spwEntityAlpha(LP Entity, float Alpha)
{
    scene::ENodeTypes Type = NODE(Entity)->getType();
    
    if (Type == scene::NODE_MESH)
        MESH(Entity)->getMaterial()->getDiffuseColor().Alpha = u8(Alpha * 255);
    else if (Type == scene::NODE_BILLBOARD)
    {
        video::color Color(BILLBOARD(Entity)->getColor());
        Color.Alpha = u8(Alpha * 255);
        BILLBOARD(Entity)->setColor(Color);
    }
}

SPW_PROC(void) spwEntityShininess(LP Entity, float Shininess)
{
    MESH(Entity)->getMaterial()->setShininess(Shininess);
    MESH(Entity)->getMaterial()->setSpecularColor(video::color(u8(Shininess * 255)));
}

SPW_PROC(void) spwEntityFX(LP Entity, int Mode)
{
    video::MaterialStates* Material = MESH(Entity)->getMaterial();
    
    Material->setLighting(!(Mode & 0x0001));
    Material->setColorMaterial(Mode & 0x0002);
    Material->setShading((Mode & 0x0004) ? video::SHADING_FLAT : video::SHADING_GOURAUD);
    Material->setFog(!(Mode & 0x0008));
    Material->setRenderFace((Mode & 0x0010) ? video::FACE_BOTH : video::FACE_FRONT);
    // !TODO: 0x0020 -> VertexAlpha enable
}

SPW_PROC(void) spwEntityBlend(LP Entity, int Mode)
{
    video::EBlendingTypes BlendSource, BlendTarget;
    
    switch (Mode)
    {
        case 0: // disable texture
            BlendSource = video::BLEND_ONE;
            BlendTarget = video::BLEND_ZERO;
            break;
        case 1: // alpha
            BlendSource = video::BLEND_SRCCOLOR;
            BlendTarget = video::BLEND_INVSRCCOLOR;
            break;
        case 2: // multiply (default)
            BlendSource = video::BLEND_SRCALPHA;
            BlendTarget = video::BLEND_INVSRCALPHA;
            break;
        case 3: // add
            BlendSource = video::BLEND_SRCALPHA;
            BlendTarget = video::BLEND_ONE;
            break;
    }
    
    switch (NODE(Entity)->getType())
    {
        case scene::NODE_MESH:
            MESH(Entity)->getMaterial()->setBlendingMode(BlendSource, BlendTarget); break;
        case scene::NODE_BILLBOARD:
            BILLBOARD(Entity)->getMaterial()->setBlendingMode(BlendSource, BlendTarget); break;
    }
}

SPW_PROC(void) spwEntityOrder(LP Entity, int Order)
{
    MESH(Entity)->setOrder((scene::ENodeOrderTypes)Order);
}

SPW_PROC(void) spwMeshCullMode_ex(LP Entity, int Mode)
{
    MESH(Entity)->getMaterial()->setRenderFace((video::EFaceTypes)Mode);
}

SPW_PROC(float) spwEntityX(LP Entity, bool Global = false)
{
    return NODE(Entity)->getPosition(Global).X;
}

SPW_PROC(float) spwEntityY(LP Entity, bool Global = false)
{
    return NODE(Entity)->getPosition(Global).Y;
}

SPW_PROC(float) spwEntityZ(LP Entity, bool Global = false)
{
    return NODE(Entity)->getPosition(Global).Z;
}

SPW_PROC(float) spwEntityPitch(LP Entity, bool Global = false)
{
    return NODE(Entity)->getRotation(Global).X;
}

SPW_PROC(float) spwEntityYaw(LP Entity, bool Global = false)
{
    return -NODE(Entity)->getRotation(Global).Y;
}

SPW_PROC(float) spwEntityRoll(LP Entity, bool Global = false)
{
    return NODE(Entity)->getRotation(Global).Z;
}

SPW_PROC(void) spwEntityMatrix(LP Entity, LP MatrixOut, bool Global = true)
{
    #if __CRASHSAFE_LEVEL__ >= 2
    if (!Entity)
    {
        WrapperError("EntityMatrix", "\"Entity\" is invalid");
        return;
    }
    if (!MatrixOut)
    {
        WrapperError("EntityMatrix", "\"MatrixOut\" is invalid");
        return;
    }
    #endif
    
    if (Global)
        (*MATRIX(MatrixOut)) = NODE(Entity)->getGlobalLocation();
    else
    {
        (*MATRIX(MatrixOut)) = NODE(Entity)->getPositionMatrix();
        (*MATRIX(MatrixOut)) *= NODE(Entity)->getRotationMatrix();
        (*MATRIX(MatrixOut)) *= NODE(Entity)->getScaleMatrix();
    }
}

SPW_PROC(float) spwMeshWidth(LP Mesh)
{
    return MESH(Mesh)->getMeshBoundingBox().getSize().X;
}

SPW_PROC(float) spwMeshHeight(LP Mesh)
{
    return MESH(Mesh)->getMeshBoundingBox().getSize().Y;
}

SPW_PROC(float) spwMeshDepth(LP Mesh)
{
    return MESH(Mesh)->getMeshBoundingBox().getSize().Z;
}

SPW_PROC(STR) spwEntityName(LP Entity)
{
    return NODE(Entity)->getName().c_str();
}

SPW_PROC(void) spwNameEntity(LP Entity, STR Name)
{
    NODE(Entity)->setName(Name);
}

SPW_PROC(float) spwEntityDistance(LP SrcEntity, LP DestEntity)
{
    return math::getDistance(NODE(SrcEntity)->getPosition(true), NODE(DestEntity)->getPosition(true));
}

SPW_PROC(void) spwCameraViewport(LP Camera, int X, int Y, int Width, int Height)
{
    CAMERA(Camera)->setViewport(dim::rect2di(X, Y, Width, Height));
}

SPW_PROC(void) spwCameraRange(LP Camera, float Near, float Far)
{
    CAMERA(Camera)->setRange(Near * 0.5, Far);
}

SPW_PROC(void) spwCameraZoom(LP Camera, float Zoom)
{
    CAMERA(Camera)->setZoom(Zoom);
}

SPW_PROC(void) spwCameraFOV_ex(LP Camera, float FOV)
{
    CAMERA(Camera)->setFOV(FOV);
}

SPW_PROC(void) spwCameraProjMode(LP Camera, int Mode)
{
    switch (Mode)
    {
        case 0:
            CAMERA(Camera)->setVisible(false); break;
        case 1:
            CAMERA(Camera)->setOrtho(false); break;
        case 2:
            CAMERA(Camera)->setOrtho(true); break;
    }
    CAMERA(Camera)->updatePerspective();
}

SPW_PROC(void) spwCameraMirror_ex(LP Camera, bool Enable)
{
    CAMERA(Camera)->setMirror(Enable);
}

SPW_PROC(void) spwCameraMirrorMatrix_ex(LP Camera, LP Matrix)
{
    CAMERA(Camera)->setMirrorMatrix(*MATRIX(Matrix));
}

SPW_PROC(void) spwLightColor(LP Light, int Red, int Green, int Blue)
{
    LIGHT(Light)->setLightingColor(video::color(Red, Green, Blue));
}

SPW_PROC(void) spwLightRange(LP Light, float Range)
{
    LIGHT(Light)->setVolumetric(!math::Equal(Range, -1.0f));
    LIGHT(Light)->setVolumetricRadius(Range);
}

SPW_PROC(void) spwLightConeAngles(LP Light, float InnerAngle, float OuterAngle)
{
    // !TODO!
}

SPW_PROC(void) spwAmbientLight(int Red, int Green, int Blue)
{
    // !TODO!
}

SPW_PROC(void) spwCaptureWorld()
{
    g_CaptureWorldList.clear();
    
    tplCaptureObjectList<scene::Mesh>(g_SceneGraph->getMeshList());
    tplCaptureObjectList<scene::Light>(g_SceneGraph->getLightList());
    tplCaptureObjectList<scene::Terrain>(g_SceneGraph->getTerrainList());
}

SPW_PROC(void) spwRenderWorld(float Tween = 1.0)
{
    // Update capture world animation
    if (Tween < 1.0 && !g_CaptureWorldList.empty())
    {
        dim::quaternion Rotation;
        
        for (std::list<SWrapCaptureObject>::iterator it = g_CaptureWorldList.begin(); it != g_CaptureWorldList.end(); ++it)
        {
            it->PositionB       = it->Object->getPosition();
            it->LastRotationMat = it->Object->getRotationMatrix();
            it->RotationB       = dim::quaternion(it->LastRotationMat);
            
            Rotation.slerp(it->RotationA, it->RotationB, Tween);
            
            it->Object->setPosition(it->PositionA + (it->PositionB - it->PositionA) * Tween);
            it->Object->setRotationMatrix(Rotation.getMatrix());
        }
    }
    
    // Update mirror planes
    // !TODO: for each plane ...
    
    // Render the whole scene for each camera
    for (std::map<LP, SWrapCamera>::iterator it = g_CameraList.begin(); it != g_CameraList.end(); ++it)
    {
        if (it->second.Object->getVisible())
        {
            // Individual camera settings
            g_RenderSystem->setFog(it->second.FogMode);
            g_RenderSystem->setFogRange(it->second.FogFallOff, it->second.FogRangeNear, it->second.FogRangeFar);
            g_RenderSystem->setFogColor(it->second.FogColor);
            g_RenderSystem->setClearColor(it->second.ClearColor);
            
            // Render the whole scene for the current camera
            g_SceneGraph->renderScene(it->second.Object);
        }
    }
    
    // Reset captured world
    if (Tween < 1.0 && !g_CaptureWorldList.empty())
    {
        for (std::list<SWrapCaptureObject>::iterator it = g_CaptureWorldList.begin(); it != g_CaptureWorldList.end(); ++it)
        {
            it->Object->setPosition(it->PositionB);
            it->Object->setRotationMatrix(it->LastRotationMat);
        }
    }
}

SPW_PROC(void) spwRenderWorldStereo(LP Camera, float CamDegree, float CamDist = 0.25)
{
    g_SceneGraph->renderSceneStereoImage(CAMERA(Camera), CamDegree, CamDist);
}

SPW_PROC(void) spwUpdateWorld()
{
    g_SceneGraph->updateAnimations();
    g_CollisionDetector->updateScene();
    
    if (g_SoundDevice && g_ListenerParent)
        g_SoundDevice->setListenerPosition(g_ListenerParent->getPosition(true));
}

SPW_PROC(void) spwWireFrame(int Mode)
{
    switch (Mode)
    {
        case 0:
            g_SceneGraph->setWireframe(video::WIREFRAME_SOLID); break;
        case 1:
            g_SceneGraph->setWireframe(video::WIREFRAME_LINES); break;
        case 2:
            g_SceneGraph->setWireframe(video::WIREFRAME_POINTS); break;
    }
}

SPW_PROC(void) spwMeshWireFrame_ex(LP Entity, int Mode)
{
    video::MaterialStates* Material = MESH(Entity)->getMaterial();
    
    switch (Mode)
    {
        case 0:
            Material->setWireframe(video::WIREFRAME_SOLID); break;
        case 1:
            Material->setWireframe(video::WIREFRAME_LINES); break;
        case 2:
            Material->setWireframe(video::WIREFRAME_POINTS); break;
    }
}

SPW_PROC(void) spwEntityType(LP Entity, int CollisionType, bool Recursive = false)
{
    scene::Collision* Coll = g_CollisionList[CollisionType];
    
    if (!Coll)
        Coll = g_CollisionList[CollisionType] = g_CollisionDetector->createCollision();
    
    scene::SCollisionObject* CollObj = 0;
    
    if (NODE(Entity)->getType() == scene::NODE_MESH)
    {
        scene::Mesh* Mesh = MESH(Entity);
        Mesh->createOctTree();
        CollObj = g_CollisionDetector->addCollisionMesh(Mesh, Coll);
    }
    else
        CollObj = g_CollisionDetector->addCollisionObject(NODE(Entity), Coll, 1.0, dim::aabbox3df(-1.0, 1.0));
    
    if (CollObj)
        CollObj->isAutoFreeze = true;
}

SPW_PROC(void) spwEntityRadius(LP Entity, float RadiusX, float RadiusY = -1.0)
{
    if (RadiusY < 0)
        RadiusY = RadiusX;
    
    scene::SCollisionObject* CollObj = g_CollisionDetector->getCollisionObject(NODE(Entity));
    if (CollObj)
        CollObj->Radius = RadiusX;
    
    scene::SPickingObject* PickObj = g_CollisionDetector->getPickingObject(NODE(Entity));
    if (PickObj)
        PickObj->Radius = RadiusX;
}

SPW_PROC(void) spwResetEntity(LP Entity)
{
    if (NODE(Entity)->getType() == scene::NODE_MESH)
        MESH(Entity)->deleteOctTree();
    
    scene::SCollisionObject* Obj = g_CollisionDetector->getCollisionObject(NODE(Entity));
    if (Obj)
        g_CollisionDetector->removeCollisionObject(Obj);
}

SPW_PROC(void) spwCollisions(int SrcType, int DestType, int Method, int Response)
{
    scene::Collision* SrcColl = g_CollisionList[SrcType];
    scene::Collision* DestColl = g_CollisionList[DestType];
    
    if (!SrcColl)
        SrcColl = g_CollisionList[SrcType] = g_CollisionDetector->createCollision();
    if (!DestColl)
        DestColl = g_CollisionList[DestType] = g_CollisionDetector->createCollision();
    
    scene::ECollisionTypes Type;
    
    switch (Method)
    {
        case 1:
            Type = scene::COLLISION_SPHERE_TO_SPHERE; break;
        case 2:
            Type = scene::COLLISION_SPHERE_TO_POLYGON; break;
        case 3:
            Type = scene::COLLISION_SPHERE_TO_BOX; break;
        default:
            return;
    }
    
    SrcColl->addCollisionMaterial(DestColl, Type);
}

SPW_PROC(int) spwCountCollisions(LP Entity)
{
    return NODE(Entity)->getContactList().size();
}

SPW_PROC(LP) spwCollisionEntity(LP Entity, int Index)
{
    scene::SCollisionContact Contact;
    return stcGetCollisionContactByIndex(Entity, Index, Contact) ? CAST(Contact.Object) : 0;
}

SPW_PROC(LP) spwCollisionSurface(LP Entity, int Index)
{
    scene::SCollisionContact Contact;
    if (stcGetCollisionContactByIndex(Entity, Index, Contact) && Contact.Object->getType() == scene::NODE_MESH)
        return spwGetSurface(CAST(Contact.Object), Contact.SurfaceIndex + 1);
    return 0;
}

SPW_PROC(int) spwCollisionTriangle(LP Entity, int Index)
{
    scene::SCollisionContact Contact;
    return stcGetCollisionContactByIndex(Entity, Index, Contact) ? Contact.TriangleIndex : 0;
}

SPW_PROC(float) spwCollisionX(LP Entity, int Index)
{
    scene::SCollisionContact Contact;
    return stcGetCollisionContactByIndex(Entity, Index, Contact) ? Contact.Point.X : 0.0;
}

SPW_PROC(float) spwCollisionY(LP Entity, int Index)
{
    scene::SCollisionContact Contact;
    return stcGetCollisionContactByIndex(Entity, Index, Contact) ? Contact.Point.Y : 0.0;
}

SPW_PROC(float) spwCollisionZ(LP Entity, int Index)
{
    scene::SCollisionContact Contact;
    return stcGetCollisionContactByIndex(Entity, Index, Contact) ? Contact.Point.Z : 0.0;
}

SPW_PROC(float) spwCollisionNX(LP Entity, int Index)
{
    scene::SCollisionContact Contact;
    return stcGetCollisionContactByIndex(Entity, Index, Contact) ? Contact.Normal.X : 0.0;
}

SPW_PROC(float) spwCollisionNY(LP Entity, int Index)
{
    scene::SCollisionContact Contact;
    return stcGetCollisionContactByIndex(Entity, Index, Contact) ? Contact.Normal.Y : 0.0;
}

SPW_PROC(float) spwCollisionNZ(LP Entity, int Index)
{
    scene::SCollisionContact Contact;
    return stcGetCollisionContactByIndex(Entity, Index, Contact) ? Contact.Normal.Z : 0.0;
}

SPW_PROC(bool) spwEntityCollided(LP Entity, int Type)
{
    return NODE(Entity)->checkContact(g_CollisionList[Type]);
}

SPW_PROC(bool) spwMeshesIntersect(LP MeshA, LP MeshB)
{
    return g_CollisionDetector->meshIntersection(MESH(MeshA), MESH(MeshB));
}

SPW_PROC(void) spwEntityPickMode(LP Entity, int Geometry = 0, bool Obscurer = true)
{
    if (Geometry == 2 && NODE(Entity)->getType() == scene::NODE_MESH)
    {
        scene::Mesh* Mesh = MESH(Entity);
        Mesh->createOctTree();
        g_CollisionDetector->addPickingMesh(Mesh);
    }
    else
    {
        scene::EPickingTypes Type;
        
        switch (Geometry)
        {
            case 0:
                Type = scene::PICKMODE_NONE; break;
            case 1:
                Type = scene::PICKMODE_SPHERE; break;
            case 2:
                Type = scene::PICKMODE_POLYGON; break;
            case 3:
                Type = scene::PICKMODE_BOX; break;
            default:
                return;
        }
        
        g_CollisionDetector->addPickingObject(NODE(Entity), Type);
    }
}

SPW_PROC(LP) spwCameraPick(LP Camera, int X, int Y, float PickLength = -1.0)
{
    g_PickingTime = g_Device->getMilliseconds();
    
    g_PickingList = g_CollisionDetector->pickIntersection(
        CAMERA(Camera)->getPickingLine(dim::point2di(X, Y), PickLength)
    );
    
    if (!g_PickingList.empty())
    {
        g_PickingIterator   = g_PickingList.begin();
        g_PickingContact    = *g_PickingIterator;
        return CAST(g_PickingContact.Object);
    }
    
    return 0;
}

SPW_PROC(LP) spwLinePick(float X, float Y, float Z, float DX, float DY, float DZ, float Radius = 0.0)
{
    g_PickingTime = g_Device->getMilliseconds();
    
    g_PickingList = g_CollisionDetector->pickIntersection(
        dim::line3df(dim::vector3df(X, Y, Z), dim::vector3df(X + DX, Y + DY, Z + DZ))
    );
    
    if (!g_PickingList.empty())
    {
        g_PickingIterator   = g_PickingList.begin();
        g_PickingContact    = *g_PickingIterator;
        return CAST(g_PickingContact.Object);
    }
    
    return 0;
}

SPW_PROC(LP) spwEntityPick(LP Entity, float Range)
{
    return 0; // !TODO!
}

SPW_PROC(int) spwCountPicked_ex()
{
    return g_PickingList.size();
}

SPW_PROC(bool) spwNextPicked_ex()
{
    if (!g_PickingList.empty())
    {
        ++g_PickingIterator;
        
        if (g_PickingIterator != g_PickingList.end())
        {
            g_PickingContact = *g_PickingIterator;
            return true;
        }
        else
            --g_PickingIterator;
    }
    return false;
}

SPW_PROC(bool) spwPreviousPicked_ex()
{
    if (!g_PickingList.empty() && g_PickingIterator != g_PickingList.begin())
    {
        --g_PickingIterator;
        g_PickingContact = *g_PickingIterator;
        return true;
    }
    return false;
}

SPW_PROC(LP) spwPickedEntity()
{
    return CAST(g_PickingContact.Object);
}

SPW_PROC(int) spwPickedSurface()
{
    return g_PickingContact.SurfaceIndex;
}

SPW_PROC(int) spwPickedTriangle()
{
    return g_PickingContact.TriangleIndex;
}

SPW_PROC(float) spwPickedX()
{
    return g_PickingContact.Point.X;
}

SPW_PROC(float) spwPickedY()
{
    return g_PickingContact.Point.Y;
}

SPW_PROC(float) spwPickedZ()
{
    return g_PickingContact.Point.Z;
}

SPW_PROC(float) spwPickedNX()
{
    return g_PickingContact.Normal.X;
}

SPW_PROC(float) spwPickedNY()
{
    return g_PickingContact.Normal.Y;
}

SPW_PROC(float) spwPickedNZ()
{
    return g_PickingContact.Normal.Z;
}

SPW_PROC(float) spwPickedTime()
{
    return static_cast<f32>(io::Timer::millisecs() - g_PickingTime) / 1000.0f;
}

SPW_PROC(void) spwCameraProject(LP Camera, float X, float Y, float Z)
{
    g_CameraProject = CAMERA(Camera)->getProjection(dim::vector3df(X, Y, Z));
}

SPW_PROC(int) spwProjectedX()
{
    return g_CameraProject.X;
}

SPW_PROC(int) spwProjectedY()
{
    return g_CameraProject.Y;
}

SPW_PROC(int) spwProjectedZ()
{
    return 0; // !TODO!
}

SPW_PROC(void) spwMeshShadow_ex(LP Entity, bool Enable)
{
    if (Enable)
        g_StencilManager->addCastCloudMesh(MESH(Entity));
    else
        g_StencilManager->removeCastCloudMesh(MESH(Entity));
    
    g_SceneGraph->setStencilEffects(!g_StencilManager->getObjectList().empty());
}

SPW_PROC(void) spwLightShadow_ex(LP Light, bool Enable)
{
    if (Enable)
    {
        if (!g_ShadowLightList[Light])
        {
            scene::SShadowLightSource* LightSource = new scene::SShadowLightSource(LIGHT(Light));
            g_ShadowLightList[Light] = LightSource;
            g_StencilManager->addLightSource(LightSource);
        }
    }
    else
    {
        scene::SShadowLightSource* LightSource = g_ShadowLightList[Light];
        
        if (LightSource)
        {
            g_StencilManager->removeLightSource(LightSource);
            g_ShadowLightList.erase(Light);
            delete LightSource;
        }
    }
}

SPW_PROC(void) spwEntityBoundBox(
    LP Entity, float X, float Y, float Z, float Width, float Height, float Depth, bool Enable = true)
{
    NODE(Entity)->getBoundingVolume().setType(Enable ? scene::BOUNDING_BOX : scene::BOUNDING_NONE);
    NODE(Entity)->getBoundingVolume().setBox(
        dim::aabbox3df(dim::vector3df(X, Y, Z), dim::vector3df(X + Width, Y + Height, Z + Depth))
    );
}

SPW_PROC(void) spwEntityBoundSphere(LP Entity, float Radius, bool Enable = true)
{
    NODE(Entity)->getBoundingVolume().setType(Enable ? scene::BOUNDING_SPHERE : scene::BOUNDING_NONE);
    NODE(Entity)->getBoundingVolume().setRadius(Radius);
}

SPW_PROC(void) spwMeshBoundBox(LP Mesh, bool Enable = true)
{
    MESH(Mesh)->getBoundingVolume().setType(Enable ? scene::BOUNDING_BOX : scene::BOUNDING_NONE);
    MESH(Mesh)->getBoundingVolume().setBox(MESH(Mesh)->getMeshBoundingBox());
}

SPW_PROC(LP) spwCreateMatrix_ex()
{
    dim::matrix4f* Obj = new dim::matrix4f();
    g_MatrixList.push_back(Obj);
    return CAST(Obj);
}

SPW_PROC(void) spwFreeMatrix_ex(LP Matrix)
{
    dim::matrix4f* Obj = MATRIX(Matrix);
    
    for (std::list<dim::matrix4f*>::iterator it = g_MatrixList.begin(); it != g_MatrixList.end(); ++it)
    {
        if (*it == Obj)
        {
            MemoryManager::deleteMemory(*it);
            g_MatrixList.erase(it);
            break;
        }
    }
}

SPW_PROC(void) spwWorldMatrix_ex(LP MatrixOut)
{
    (*MATRIX(MatrixOut)) = g_RenderSystem->getWorldMatrix();
}

SPW_PROC(void) spwViewMatrix_ex(LP MatrixOut)
{
    (*MATRIX(MatrixOut)) = g_RenderSystem->getViewMatrix();
}

SPW_PROC(void) spwProjectionMatrix_ex(LP MatrixOut)
{
    (*MATRIX(MatrixOut)) = g_RenderSystem->getProjectionMatrix();
}

SPW_PROC(float) spwMatrixGetValue_ex(LP Matrix, int Index)
{
    if (Index >= 0 && Index < 16)
        return (*MATRIX(Matrix))[Index];
    return 0.0f;
}

SPW_PROC(void) spwMatrixSetValue_ex(LP Matrix, int Index, float Value)
{
    if (Index >= 0 && Index < 16)
        (*MATRIX(Matrix))[Index] = Value;
}

SPW_PROC(void) spwMatrixReset_ex(LP Matrix)
{
    MATRIX(Matrix)->reset();
}

SPW_PROC(void) spwMatrixTranslate_ex(LP Matrix, float X, float Y, float Z)
{
    MATRIX(Matrix)->translate(dim::vector3df(X, Y, Z));
}

SPW_PROC(void) spwMatrixScale_ex(LP Matrix, float X, float Y, float Z)
{
    MATRIX(Matrix)->scale(dim::vector3df(X, Y, Z));
}

SPW_PROC(void) spwMatrixRotate_ex(LP Matrix, float Angle, float NX, float NY, float NZ)
{
    MATRIX(Matrix)->rotate(Angle, dim::vector3df(NX, NY, NZ));
}

SPW_PROC(void) spwMatrixRotateX_ex(LP Matrix, float Angle)
{
    MATRIX(Matrix)->rotateX(Angle);
}

SPW_PROC(void) spwMatrixRotateY_ex(LP Matrix, float Angle)
{
    MATRIX(Matrix)->rotateY(Angle);
}

SPW_PROC(void) spwMatrixRotateZ_ex(LP Matrix, float Angle)
{
    MATRIX(Matrix)->rotateZ(Angle);
}

SPW_PROC(void) spwMultMatrix_ex(LP MatrixOut, LP MatrixA, LP MatrixB)
{
    (*MATRIX(MatrixOut)) = (*MATRIX(MatrixA)) * (*MATRIX(MatrixB));
}

SPW_PROC(void) spwInverseMatrix_ex(LP Matrix)
{
    MATRIX(Matrix)->setInverse();
}

SPW_PROC(LP) spwCreateBuffer_ex(int Size)
{
    char* Obj = new char[Size];
    g_BufferList.push_back(Obj);
    return CAST(Obj);
}

SPW_PROC(void) spwFreeBuffer_ex(LP Buffer)
{
    char* Obj = BUFFER(Buffer);
    
    for (std::list<char*>::iterator it = g_BufferList.begin(); it != g_BufferList.end(); ++it)
    {
        if (*it == Obj)
        {
            MemoryManager::deleteBuffer(*it);
            g_BufferList.erase(it);
            break;
        }
    }
}


/*
 * ======= Physics system: =======
 */

SPW_PROC(void) spwUpdateSimulation(float StepTime = 0.017)
{
    g_PhysicsSystem->updateSimulation(StepTime);
}

SPW_PROC(LP) spwCreateStaticBody_ex(LP MeshGeom)
{
    return CAST(g_PhysicsSystem->createStaticBody(MESH(MeshGeom)));
}

SPW_PROC(void) spwDeleteStaticBody_ex(LP PhysicsObject)
{
    g_PhysicsSystem->deleteStaticBody(STATICBODY(PhysicsObject));
}

SPW_PROC(LP) spwCreateRigidBodyNode(LP RootNode, int Type, float X = 0.5, float Y = 0.5, float Z = 0.5, float Radius = 0.5, float Height = 0.5, float Mass = 25.0)
{
    return CAST(g_PhysicsSystem->createRigidBody(
        NODE(RootNode), (physics::ERigidBodies)Type, physics::SRigidBodyConstruction(dim::vector3df(X, Y, Z), Radius, Height), Mass
    ));
}

SPW_PROC(LP) spwCreateRigidBodyMesh(LP MeshGeom, float Mass = 25.0)
{
    return CAST(g_PhysicsSystem->createRigidBody(MESH(MeshGeom)));
}

SPW_PROC(void) spwDeleteRigidBody_ex(LP PhysicsObject)
{
    g_PhysicsSystem->deleteRigidBody(RIGIDBODY(PhysicsObject));
}

SPW_PROC(void) spwSetPhysicsSolverModel_ex(int Model)
{
    g_PhysicsSystem->setSolverModel(Model);
}

SPW_PROC(void) spwSetPhysicsGravity_ex(float X, float Y, float Z)
{
    g_PhysicsSystem->setGravity(dim::vector3df(X, Y, Z));
}

SPW_PROC(float) spwGetPhysicsGravityX_ex()
{
    return g_PhysicsSystem->getGravity().X;
}

SPW_PROC(float) spwGetPhysicsGravityY_ex()
{
    return g_PhysicsSystem->getGravity().Y;
}

SPW_PROC(float) spwGetPhysicsGravityZ_ex()
{
    return g_PhysicsSystem->getGravity().Z;
}

SPW_PROC(void) spwSetPhysicsThreadCount_ex(int Count)
{
    g_PhysicsSystem->setThreadCount(Count);
}

SPW_PROC(float) spwGetPhysicsThreadCount_ex()
{
    return g_PhysicsSystem->getThreadCount();
}

SPW_PROC(STR) spwGetPhysicsVersion_ex()
{
    return g_PhysicsSystem->getVersion().c_str();
}

SPW_PROC(void) spwSetPhysicsObjectPosition_ex(LP PhysicsObject, float X, float Y, float Z)
{
    if (PhysicsObject)
        PHYSICSNODE(PhysicsObject)->setPosition(dim::vector3df(X, Y, Z));
}

SPW_PROC(float) spwGetPhysicsObjectPositionX_ex(LP PhysicsObject)
{
    return PhysicsObject ? PHYSICSNODE(PhysicsObject)->getPosition().X : 0.0f;
}

SPW_PROC(float) spwGetPhysicsObjectPositionY_ex(LP PhysicsObject)
{
    return PhysicsObject ? PHYSICSNODE(PhysicsObject)->getPosition().Y : 0.0f;
}

SPW_PROC(float) spwGetPhysicsObjectPositionZ_ex(LP PhysicsObject)
{
    return PhysicsObject ? PHYSICSNODE(PhysicsObject)->getPosition().Z : 0.0f;
}

SPW_PROC(void) spwSetPhysicsObjectRotation_ex(LP PhysicsObject, LP Matrix)
{
    if (PhysicsObject && Matrix)
        PHYSICSNODE(PhysicsObject)->setRotation(*MATRIX(Matrix));
}

SPW_PROC(void) spwGetPhysicsObjectRotation_ex(LP PhysicsObject, LP Matrix)
{
    if (PhysicsObject && Matrix)
        *MATRIX(Matrix) = PHYSICSNODE(PhysicsObject)->getRotation();
}

SPW_PROC(void) spwSetRigidBodyMass_ex(LP PhysicsObject, float Mass)
{
    if (PhysicsObject)
        RIGIDBODY(PhysicsObject)->setMass(Mass);
}

SPW_PROC(float) spwGetRigidBodyMass_ex(LP PhysicsObject)
{
    return PhysicsObject ? RIGIDBODY(PhysicsObject)->getMass() : 0.0f;
}

SPW_PROC(void) spwSetRigidBodyAutoSleep_ex(LP PhysicsObject, bool Enable)
{
    if (PhysicsObject)
        RIGIDBODY(PhysicsObject)->setAutoSleep(Enable);
}

SPW_PROC(bool) spwGetRigidBodyAutoSleep_ex(LP PhysicsObject)
{
    return PhysicsObject ? RIGIDBODY(PhysicsObject)->getAutoSleep() : false;
}

SPW_PROC(void) spwSetRigidBodyBalancePoint_ex(LP PhysicsObject, float X, float Y, float Z)
{
    if (PhysicsObject)
        RIGIDBODY(PhysicsObject)->setBalancePoint(dim::vector3df(X, Y, Z));
}

SPW_PROC(float) spwGetRigidBodyBalancePointX_ex(LP PhysicsObject)
{
    return PhysicsObject ? RIGIDBODY(PhysicsObject)->getBalancePoint().X : 0.0f;
}

SPW_PROC(float) spwGetRigidBodyBalancePointY_ex(LP PhysicsObject)
{
    return PhysicsObject ? RIGIDBODY(PhysicsObject)->getBalancePoint().Y : 0.0f;
}

SPW_PROC(float) spwGetRigidBodyBalancePointZ_ex(LP PhysicsObject)
{
    return PhysicsObject ? RIGIDBODY(PhysicsObject)->getBalancePoint().Z : 0.0f;
}

SPW_PROC(void) spwAddRigidBodyVelocity_ex(LP PhysicsObject, float X, float Y, float Z)
{
    if (PhysicsObject)
        RIGIDBODY(PhysicsObject)->addVelocity(dim::vector3df(X, Y, Z));
}

SPW_PROC(void) spwSetRigidBodyVelocity_ex(LP PhysicsObject, float X, float Y, float Z)
{
    if (PhysicsObject)
        RIGIDBODY(PhysicsObject)->setVelocity(dim::vector3df(X, Y, Z));
}

SPW_PROC(float) spwGetRigidBodyVelocityX_ex(LP PhysicsObject)
{
    return PhysicsObject ? RIGIDBODY(PhysicsObject)->getVelocity().X : 0.0f;
}

SPW_PROC(float) spwGetRigidBodyVelocityY_ex(LP PhysicsObject)
{
    return PhysicsObject ? RIGIDBODY(PhysicsObject)->getVelocity().Y : 0.0f;
}

SPW_PROC(float) spwGetRigidBodyVelocityZ_ex(LP PhysicsObject)
{
    return PhysicsObject ? RIGIDBODY(PhysicsObject)->getVelocity().Z : 0.0f;
}

SPW_PROC(void) spwAddRigidBodyImpulse_ex(LP PhysicsObject, float DirX, float DirY, float DirZ, float PointX, float PointY, float PointZ)
{
    if (PhysicsObject)
    {
        RIGIDBODY(PhysicsObject)->addImpulse(
            dim::vector3df(DirX, DirY, DirZ), dim::vector3df(PointX, PointY, PointZ)
        );
    }
}

SPW_PROC(LP) spwAddRigidBodyJoint(
    LP PhysicsObject, LP Child, int Type, float PointX, float PointY, float PointZ,
    float DirX = 0.0, float DirY = 1.0, float DirZ = 0.0, float Dir2X = 0.0, float Dir2Y = 1.0, float Dir2Z = 0.0)
{
    if (PhysicsObject && Child)
    {
        return CAST(g_PhysicsSystem->createJoint(
            RIGIDBODY(PhysicsObject), RIGIDBODY(Child),
            static_cast<physics::EPhysicsJoints>(Type),
            dim::vector3df(PointX, PointY, PointZ),
            dim::vector3df(DirX, DirY, DirZ),
            dim::vector3df(Dir2X, Dir2Y, Dir2Z)
        ));
    }
    return 0;
}

SPW_PROC(void) spwRemoveRigidBodyJoint_ex(LP PhysicsObject, LP Joint)
{
     if (PhysicsObject && Joint)
        RIGIDBODY(PhysicsObject)->removeJoint(BODYJOINT(Joint));
}

SPW_PROC(void) spwSetRigidBodyGravity_ex(LP PhysicsObject, float X, float Y, float Z)
{
    if (PhysicsObject)
        RIGIDBODY(PhysicsObject)->setGravity(dim::vector3df(X, Y, Z));
}

SPW_PROC(float) spwGetRigidBodyGravityX_ex(LP PhysicsObject)
{
    return PhysicsObject ? RIGIDBODY(PhysicsObject)->getGravity().X : 0.0f;
}

SPW_PROC(float) spwGetRigidBodyGravityY_ex(LP PhysicsObject)
{
    return PhysicsObject ? RIGIDBODY(PhysicsObject)->getGravity().Y : 0.0f;
}

SPW_PROC(float) spwGetRigidBodyGravityZ_ex(LP PhysicsObject)
{
    return PhysicsObject ? RIGIDBODY(PhysicsObject)->getGravity().Z : 0.0f;
}



// ================================================================================
