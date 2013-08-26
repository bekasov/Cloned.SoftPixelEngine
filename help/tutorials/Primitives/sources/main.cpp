//
// SoftPixel Engine Tutorial: Primitives - (21.6.2010)
//

#include <SoftPixelEngine.hpp>

using namespace sp;

#include "../../common.hpp"

/* === Global members === */

SoftPixelDevice* spDevice           = 0;
io::InputControl* spControl         = 0;
video::RenderSystem* spRenderer     = 0;
video::RenderContext* spContext     = 0;
scene::SceneManager* spSceneMngr    = 0;
scene::SceneGraph* spScene          = 0;

const s32 ScrWidth = 800, ScrHeight = 600;

scene::Camera* Cam  = 0;
scene::Light* Light = 0;
scene::Mesh* SkyBox = 0;

scene::NodeAnimation* Anim  = 0;

video::Texture* TexStone    = 0;
video::Texture* TexDetail   = 0;
video::Texture* TexSkyList[6] = { 0 };

video::Font* Font = 0;

// Structures
struct SDemoPrimitive
{
    scene::Mesh* Object;
    scene::Mesh* Platform;
    dim::vector3df Turn;
    io::stringc Description;
    video::color Color;
};

std::list<SDemoPrimitive> PrimitiveList;

// Declarations
bool InitDevice();
void CreateScene();
void UpdateScene();
scene::Mesh* CreatePrimitive(const scene::EBasicMeshes Model, const dim::vector3df &Position);
void AnimatePrimitives();
void DrawCenteredText(const dim::point2di &Pos, const io::stringc &Text, const video::color &Color);
void DrawPrimitiveDescription(SDemoPrimitive &Prim);
void DrawDescriptions();


/* === All function definitions === */

int main()
{
    if (!InitDevice())
    {
        io::Log::pauseConsole();
        return 0;
    }
    
    CreateScene();
    
    while (spDevice->updateEvents() && !spControl->keyDown(io::KEY_ESCAPE))
    {
        // We only need to clear the depth buffer because the skybox masks always the whole screen
        // (by default: BUFFER_COLOR | BUFFER_DEPTH)
        spRenderer->clearBuffers();
        
        // Update the scene
        UpdateScene();
        
        // Render the whole scene (for each camera)
        // If you want to draw the scene for only one special camera use "renderScene(CameraObject)"
        spScene->renderScene();
        
        // Draw the description for each primitive
        DrawDescriptions();
        
        spContext->flipBuffers();
    }
    
    deleteDevice();
    
    return 0;
}

bool InitDevice()
{
    spDevice = createGraphicsDevice(
        ChooseRenderer(), dim::size2di(ScrWidth, ScrHeight), 32, "SoftPixel Engine - Primitives Tutorial"//, false, DEVICEFLAG_HQ
    );
    
    if (!spDevice)
        return false;
    
    spRenderer  = spDevice->getRenderSystem();
    spContext   = spDevice->getRenderContext();
    spControl   = spDevice->getInputControl();
    spSceneMngr = spDevice->getSceneManager();
    
    spScene     = spDevice->createSceneGraph();
    
    spContext->setWindowTitle(
        spContext->getWindowTitle() + " [ " + spRenderer->getVersion() + " ]"
    );
    
    spDevice->setFrameRate(100);
    
    return true;
}

/**
 * This function creates our scene with its camera, light, ground and all the primitive objects what the tutorial is about.
 * We create a skybox using six textures for each point of the compass. To get the sky textures in a mode that they
 * fit perfectly together at each edge we have to clamp the texture coordinates to edges. To achieve that we set the
 * texture generation flag "Wrap mode" to TEXWRAP_CLAMP that we don't need to set the wrap mode for each texture
 * individual and reset the "Wrap mode" after loading the six sky textures. To understand what the wrap mode really
 * does just remove the call of "setTextureGenFlags" and you will see how ugly the sky box's edges will look ;-)
 */
void CreateScene()
{
    // Resources path
    const io::stringc ResPath = ROOT_PATH + "Primitives/media/";
    
    // Load some textures
    TexStone    = spRenderer->loadTexture(ResPath + "Stone.jpg");
    TexDetail   = spRenderer->loadTexture(ResPath + "DetailMap.jpg");
    
    // Load skybox textures
    spRenderer->setTextureGenFlags(video::TEXGEN_WRAP, video::TEXWRAP_CLAMP);
    TexSkyList[0] = spRenderer->loadTexture(ResPath + "SkyboxNorth.jpg");
    TexSkyList[1] = spRenderer->loadTexture(ResPath + "SkyboxSouth.jpg");
    TexSkyList[2] = spRenderer->loadTexture(ResPath + "SkyboxTop.jpg");
    TexSkyList[3] = spRenderer->loadTexture(ResPath + "SkyboxBottom.jpg");
    TexSkyList[4] = spRenderer->loadTexture(ResPath + "SkyboxWest.jpg");
    TexSkyList[5] = spRenderer->loadTexture(ResPath + "SkyboxEast.jpg");
    spRenderer->setTextureGenFlags(video::TEXGEN_WRAP, video::TEXWRAP_REPEAT);
    
    // Load the font
    Font = spRenderer->createFont("Arial", 20, video::FONT_BOLD);
    
    // Create a camera to see the scene
    Cam = spScene->createCamera();
    
    // Set the near- and far clipping plane (or rather range)
    Cam->setRange(0.1f, 1000.0f);
    
    // Create a light (by default: directional)
    Light = spScene->createLight(scene::LIGHT_DIRECTIONAL);
    
    // Turn the light (only useful for directional lights)
    Light->setRotation(dim::vector3df(45, 10, 0));
    
    // Activate global lighting (by default: true)
    // You can also enable or disable lighting for each individual object "MeshObject->setLighting(true)"
    spScene->setLighting(true);
    
    // Create a height field using the loaded height map
    scene::Mesh* HeightField = spScene->createHeightField(
        spRenderer->loadTexture(ResPath + "HeightMap.jpg"), 100
    );
    
    // Set height field's dimension
    HeightField->setScale(dim::vector3df(70, 8, 70));
    HeightField->setPosition(dim::vector3df(0, -10, 0));
    
    // Add a terrain map (grass) and a detail map
    HeightField->addTexture(spRenderer->loadTexture(ResPath + "TerrainMap.jpg"));
    HeightField->addTexture(TexDetail);
    
    // Configure the second texture to let it look like a real detail texture
    HeightField->getMeshBuffer(0)->setTextureEnv(1, video::TEXENV_ADDSIGNED);
    HeightField->getMeshBuffer(0)->textureTransform(1, dim::point2df(10));
    
    // Create the animation for the camera
    Anim = spSceneMngr->createAnimation<scene::NodeAnimation>("CameraAnimation");
    Anim->setSplineTranslation(true);
    
    Cam->addAnimation(Anim);
    
    dim::vector3df Pos;
    scene::Mesh* CurMesh = 0;
    
    for (s32 i = scene::MESH_CUBE; i <= scene::MESH_TEAPOT; ++i)
    {
        // Create the primitive object
        Pos.X = math::Sin(static_cast<f32>(i) * 360 / (scene::MESH_TEAPOT - scene::MESH_CUBE + 1))*25;
        Pos.Y = -15.0f + i;
        Pos.Z = math::Cos(static_cast<f32>(i) * 360 / (scene::MESH_TEAPOT - scene::MESH_CUBE + 1))*25;
        
        CurMesh = CreatePrimitive(static_cast<scene::EBasicMeshes>(i), Pos);
        
        // Add a new sequence to the animation
        Pos.X = math::Sin(static_cast<f32>(i) * 360 / (scene::MESH_TEAPOT - scene::MESH_CUBE + 1))*30;
        Pos.Y = CurMesh->getPosition(true).Y + 0.5f;
        Pos.Z = math::Cos(static_cast<f32>(i) * 360 / (scene::MESH_TEAPOT - scene::MESH_CUBE + 1))*30;
        
        Anim->addKeyframe(scene::Transformation(Pos, dim::quaternion(), 1.0f));
    }
    
    // Play the animation in a loop
    Anim->play(scene::PLAYBACK_LOOP);
    Anim->setSpeed(0.5f);
    
    // Create a sky box with the radius of 100 elements
    SkyBox = spScene->createSkyBox(TexSkyList, 100.0f);
}

/**
 * Here we update the primitives- and camera animation and camera free movement
 * After animating the camera we point it to the middle of the scene and turn it left
 * to let the camera look in the direction of the animation's translation.
 */
void UpdateScene()
{
    static bool FreeMovement;
    
    // Animate all primitives
    AnimatePrimitives();
    
    // Switch between animation and free movement
    if (spControl->keyHit(io::KEY_SPACE))
    {
        FreeMovement = !FreeMovement;
        Anim->pause(FreeMovement);
        spControl->setCursorVisible(!FreeMovement);
    }
    
    // Move the camera free (if enabled)
    if (FreeMovement)
        tool::Toolset::moveCameraFree(Cam);
    
    // Update all animations
    spSceneMngr->updateAnimations();
    
    // Point the camera (after animation because it will also rotate the camera)
    if (Anim->playing())
    {
        Cam->lookAt(dim::vector3df(0, Cam->getPosition().Y, 0));
        Cam->turn(dim::vector3df(0, -35, 0));
        Cam->turn(dim::vector3df(10, 0, 0));
    }
    
    // Locate the skybox
    SkyBox->setPosition(Cam->getPosition(true));
}

/**
 * This function creats the primitive objects and configures its animated rotation.
 * We also set color and description and a platform where the object 'stands' onto.
 */
scene::Mesh* CreatePrimitive(const scene::EBasicMeshes Model, const dim::vector3df &Position)
{
    SDemoPrimitive Prim;
    {
        // Create the primitive object
        Prim.Object = spScene->createMesh(Model);
        
        // Create a platform where the object stands on
        Prim.Platform = spScene->createMesh(scene::MESH_CUBE);
        Prim.Platform->meshTransform(dim::vector3df(1.5, 20, 1.5));
        Prim.Platform->addTexture(TexStone);
        
        // Process auto texturing (also used for lightmaps while generation) with density 1.0
        Prim.Platform->textureAutoMap(0, 0.5);
        
        // Add a detail texture
        Prim.Platform->addTexture(TexDetail);
        Prim.Platform->getMeshBuffer(0)->setTextureEnv(1, video::TEXENV_ADDSIGNED);
        Prim.Platform->textureAutoMap(1, 1.0);
        
        // Determine which rotation shall be used
        switch (Model)
        {
            case scene::MESH_CUBE:
            case scene::MESH_DODECAHEDRON:
            case scene::MESH_ICOSAHEDRON:
            case scene::MESH_ICOSPHERE:
            case scene::MESH_TETRAHEDRON:
            case scene::MESH_CUBOCTAHEDRON:
            case scene::MESH_OCTAHEDRON:
            case scene::MESH_PLANE:
                Prim.Turn = dim::vector3df(1, 1, 1); break;
            case scene::MESH_TEAPOT:
            case scene::MESH_TORUSKNOT:
            case scene::MESH_SPIRAL:
                Prim.Turn = dim::vector3df(0, 1, 0); break;
            case scene::MESH_TORUS:
            case scene::MESH_DISK:
                Prim.Turn = dim::vector3df(1, 0, 0); break;
            default:
                break;
        }
        
        // Set color and description
        switch (Model)
        {
            case scene::MESH_CUBE:
                Prim.Color          = video::color(255, 0, 0);
                Prim.Description    = "Cube";
                break;
            case scene::MESH_CONE:
                Prim.Color          = video::color(0, 255, 0);
                Prim.Description    = "Cone";
                break;
            case scene::MESH_CYLINDER:
                Prim.Color          = video::color(0, 0, 255);
                Prim.Description    = "Cylinder";
                break;
            case scene::MESH_SPHERE:
                Prim.Color          = video::color(255, 255, 0);
                Prim.Description    = "Sphere";
                break;
            case scene::MESH_ICOSPHERE:
                Prim.Color          = video::color(0, 255, 255);
                Prim.Description    = "IcoSphere";
                break;
            case scene::MESH_TORUS:
                Prim.Color          = video::color(255, 0, 255);
                Prim.Description    = "Torus (ring)";
                break;
            case scene::MESH_TORUSKNOT:
                Prim.Color          = video::color(255, 255, 255);
                Prim.Description    = "TorusKnot";
                break;
            case scene::MESH_SPIRAL:
                Prim.Color          = video::color(90, 90, 90);
                Prim.Description    = "Spiral";
                break;
            case scene::MESH_PIPE:
                Prim.Color          = video::color(255, 128, 0);
                Prim.Description    = "Pipe";
                break;
            case scene::MESH_PLANE:
                Prim.Color          = video::color(0, 128, 255);
                Prim.Description    = "Plane";
                break;
            case scene::MESH_DISK:
                Prim.Color          = video::color(128, 255, 128);
                Prim.Description    = "Disk";
                break;
            case scene::MESH_CUBOCTAHEDRON:
                Prim.Color          = video::color(255, 128, 255);
                Prim.Description    = "Coboctahedron";
                break;
            case scene::MESH_TETRAHEDRON:
                Prim.Color          = video::color(64, 128, 255);
                Prim.Description    = "Tetrahedron";
                break;
            case scene::MESH_OCTAHEDRON:
                Prim.Color          = video::color(255, 128, 64);
                Prim.Description    = "Octahedron";
                break;
            case scene::MESH_DODECAHEDRON:
                Prim.Color          = video::color(255, 64, 128);
                Prim.Description    = "Dodecahedron";
                break;
            case scene::MESH_ICOSAHEDRON:
                Prim.Color          = video::color(128, 64, 255);
                Prim.Description    = "Icosahedron";
                break;
            case scene::MESH_TEAPOT:
                Prim.Color          = video::color(222, 80, 120);
                Prim.Description    = "Teapot (Utah Teapot)";
                break;
        }
        
        // Special cases
        switch (Model)
        {
            case scene::MESH_ICOSPHERE:
                // Set flat shading and update normals
                Prim.Object->setShading(video::SHADING_FLAT);
                Prim.Object->updateNormals();
                break;
            case scene::MESH_DISK:
            case scene::MESH_PLANE:
                // Set render face to both because the two models normally only have one face side
                Prim.Object->getMaterial()->setRenderFace(video::FACE_BOTH);
                break;
            case scene::MESH_TORUSKNOT:
                Prim.Object->setScale(0.5);
                break;
            default:
                break;
        }
        
        // For Direct3D11 we want per-pixel shading instead of gouraud shading
        if (spRenderer->getRendererType() == video::RENDERER_DIRECT3D11 && Prim.Object->getMaterial()->getShading() == video::SHADING_GOURAUD)
            Prim.Object->setShading(video::SHADING_PERPIXEL);
    }
    PrimitiveList.push_back(Prim);
    
    // Set the object as a parent to the platform so that we can locate the platform and
    // the object is linked to it. The objects will always be together ;-)
    Prim.Object->setParent(Prim.Platform);
    
    // Color the object and locate it a little bit above the platform
    Prim.Object->paint(Prim.Color);
    
    const dim::vector3df Size(Prim.Object->getMeshBoundingBox().getSize() * Prim.Object->getScale());
    
    if (Prim.Turn.empty())
        Prim.Object->setPosition(dim::vector3df(0, 10.5f + Size.Y/2, 0));
    else
        Prim.Object->setPosition(dim::vector3df(0, 10.5f + math::Max(Size.X, Size.Y, Size.Z)/2, 0));
    
    // Locate the platform and its child object
    Prim.Platform->setPosition(Position);
    
    return Prim.Object;
}

/**
 * This function only animates (or rather rotates) the primitive objects
 */
void AnimatePrimitives()
{
    for (std::list<SDemoPrimitive>::iterator it = PrimitiveList.begin(); it != PrimitiveList.end(); ++it)
        it->Object->turn(it->Turn);
}

/**
 * Function to draw centered text easyly.
 */
void DrawCenteredText(const dim::point2di &Pos, const io::stringc &Text, const video::color &Color)
{
    // Get the text size
    const s32 TextWidth(Font->getStringWidth(Text));
    
    // Draw the text (background black and foreground with specified color)
    spRenderer->draw2DText(
        Font, Pos - dim::point2di(TextWidth/2, 0) + 1, Text, video::color(0, 0, 0, Color.Alpha)
    );
    spRenderer->draw2DText(
        Font, Pos - dim::point2di(TextWidth/2, 0), Text, Color
    );
}

/**
 * This function draws the description text for the specified primitive object.
 * It calculates the distance between the object and the camera to get the alpha channel.
 * For far distanced objects we do not want a descritpion text. Only for near distanced objects.
 */
void DrawPrimitiveDescription(SDemoPrimitive &Prim)
{
    // Get the object's 3d and 2d (projection) position
    const dim::vector3df Pos3D(Prim.Object->getPosition(true) + dim::vector3df(0, 1, 0));
    const dim::point2di Pos2D(Cam->getProjectedPoint(Pos3D));
    
    // Compute distance and clamp it to the range [0.0 - 1.0]
    f32 Distance = ( math::getDistance(Cam->getPosition(), Pos3D) + 5 ) / 25;
    math::clamp(Distance, 0.0f, 1.0f);
    
    // Get the color
    video::color Color(255);//Prim.Color);
    Color.Alpha = static_cast<u8>((1.0f - Distance) * 255);
    
    // Draw the text centered (if alpha is high enough that we can see something)
    if (Color.Alpha > 5)
        DrawCenteredText(Pos2D, Prim.Description, Color);
}

/**
 * Here we just go through each primitive object to draw its description.
 * Further more we draw a tutorial information text at the screen's top.
 */
void DrawDescriptions()
{
    spRenderer->beginDrawing2D();
    
    // Draw a tutorial information text
    DrawCenteredText(
        dim::point2di(ScrWidth/2, 15), "Press space to switch between animation and free movement", video::color(255, 255, 255, 200)
    );
    
    // Draw the primitive descriptions
    for (std::list<SDemoPrimitive>::iterator it = PrimitiveList.begin(); it != PrimitiveList.end(); ++it)
        DrawPrimitiveDescription(*it);
    
    spRenderer->endDrawing2D();
}





// ============================================
