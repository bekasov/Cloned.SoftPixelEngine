//
// SoftPixel Engine Tutorial: ShaderLibrary - (04/09/2010)
//

#include "Library.hpp"

#include "../../common.hpp"

/* === Global members === */

SoftPixelDevice* spDevice       = 0;
io::InputControl* spControl     = 0;
video::RenderSystem* spRenderer = 0;
video::RenderContext* spContext = 0;
scene::SceneGraph* spScene      = 0;

const s32 ScrWidth = 800, ScrHeight = 600;

scene::Camera* Cam  = 0;
scene::Light* Light = 0;

video::Font* TextFont   = 0;

video::color ClearColor = video::color(128, 200, 255);

#if defined(SP_PLATFORM_WINDOWS)
const io::stringc BasePath = "";//ROOT_PATH + "ShaderLibrary/";
#elif defined(SP_PLATFORM_LINUX)
const io::stringc BasePath = "";
#endif

bool IsShowHelp = true;

/**
 * These are the objects of our shader example classes declared in "Library.hpp" and defined in "Library.cpp".
 * For more information about them look inside these files.
 */
ShaderExample               * CurrentExample    = 0;
SimpleVertexManipulation    * ExampleVM         = 0;
Billboarding                * ExampleBB         = 0;
RustProcess                 * ExampleRP         = 0;
ProceduralTextures          * ExamplePT         = 0;
FurEffect                   * ExampleFE         = 0;
CelShading                  * ExampleCS         = 0;
DepthOfField                * ExampleDF         = 0;
BumpMapping                 * ExampleBM         = 0;
WaterSimulation             * ExampleWS         = 0;


// Declarations
bool InitDevice();
void CreateScene();
void UpdateScene();
void RenderScene();
void ClearScene();
void DrawCenteredText(const dim::point2di &Pos, const io::stringc &Text, const video::color &Color);


/* === All function definitions === */

int main()
{
    if (!InitDevice())
        return 0;
    
    CreateScene();
    
    while (spDevice->updateEvents() && !spControl->keyDown(io::KEY_ESCAPE))
    {
        spRenderer->clearBuffers();
        
        UpdateScene();
        
        RenderScene();
        
        spContext->flipBuffers();
    }
    
    ClearScene();
    
    deleteDevice();
    
    return 0;
}

/**
 * In this case we only allow OpenGL or Direct3D9 video driver because the shaders for this tutorial
 * are written only for these renderer. But in this tutorial we only allow the user to choose between OpenGL and Direct3D9
 * because we only have shader for these renderer APIs. Any other renderer needs to be disabled using the specified flags.
 */
bool InitDevice()
{
    spDevice    = createGraphicsDevice(
        #if defined(SP_PLATFORM_WINDOWS)
        ChooseRenderer(CHOOSE_RENDERER | CHOOSE_DISABLE_DIRECT3D11 | CHOOSE_DISABLE_DUMMY).Driver,
        #elif defined(SP_PLATFORM_LINUX)
        video::RENDERER_OPENGL,
        #endif
        dim::size2di(ScrWidth, ScrHeight), 32, "SoftPixel Engine - ShaderLibrary Tutorial"
    );
    
    spControl   = spDevice->getInputControl();
    spRenderer  = spDevice->getRenderSystem();
    spContext   = spDevice->getRenderContext();
    
    spScene     = spDevice->createSceneGraph();
    
    // If the valid renderers are not supported (maybe because they are not compiled
    // in the used SoftPixel Engine DLL) exit the program with an error message.
    if (spRenderer->getRendererType() != video::RENDERER_OPENGL && spRenderer->getRendererType() != video::RENDERER_DIRECT3D9)
    {
        io::Log::error("Valid rendering device is not supported");
        return false;
    }
    
    spContext->setWindowTitle(
        spContext->getWindowTitle() + " [ " + spRenderer->getVersion() + " ]"
    );
    
    spDevice->setFrameRate(100);
    
    spRenderer->setClearColor(ClearColor);
    
    math::Randomizer::seedRandom();
    
    return true;
}

/**
 * Create the whole scene. At first we load a font to standard text drawing,
 * create our camera and a light. In this case we need the light basically for the information
 * about the position and orientation to set them in our shader programs.
 * Here the shader example object will be created using the static template function from the "ShaderExample" parent class.
 */
void CreateScene()
{
    // Load the font
    TextFont = spRenderer->createFont("Arial", 20, video::FONT_BOLD);
    
    // Create a camera to see the scene
    Cam = spScene->createCamera();
    Cam->setRange(0.1f, 200.0f);
    
    // Create a point light
    Light = spScene->createLight(scene::LIGHT_DIRECTIONAL);
    
    spScene->setLighting(true);
    
    // Create the shader examples
    ExampleVM = ShaderExample::Create<SimpleVertexManipulation> ();
    ExampleBB = ShaderExample::Create<Billboarding>             ();
    ExampleRP = ShaderExample::Create<RustProcess>              ();
    ExamplePT = ShaderExample::Create<ProceduralTextures>       ();
    ExampleFE = ShaderExample::Create<FurEffect>                ();
    ExampleCS = ShaderExample::Create<CelShading>               ();
    ExampleDF = ShaderExample::Create<DepthOfField>             ();
    ExampleBM = ShaderExample::Create<BumpMapping>              ();
    ExampleWS = ShaderExample::Create<WaterSimulation>          ();
    
    // Set the current example and make it visible
    CurrentExample = ExampleVM;
    
    if (CurrentExample)
        CurrentExample->Show(true);
}

/**
 * Here the all user activity and shader interactivity will be updated.
 * The user can switch between wireframe modes (solid, lines or points) and between the different
 * shader examples.
 */
void UpdateScene()
{
    // Update the current shader example
    if (CurrentExample)
        CurrentExample->Update();
    
    // Switch between the wireframe modes
    if (spControl->keyHit(io::KEY_W))
    {
        static s32 WireframeMode = 1;
        
        switch (WireframeMode)
        {
            case 0:
                spScene->setWireframe(video::WIREFRAME_SOLID); WireframeMode = 1; break;
            case 1:
                spScene->setWireframe(video::WIREFRAME_LINES); WireframeMode = 2; break;
            case 2:
                spScene->setWireframe(video::WIREFRAME_POINTS); WireframeMode = 0; break;
        }
    }
    
    // Switch between the different shader examples
    static s32 ExampleIndex;
    
    s32 LastExampleIndex = ExampleIndex;
    
    if (spControl->keyHit(io::KEY_PAGEUP))
    {
        if (++ExampleIndex > 8)
            ExampleIndex = 0;
    }
    if (spControl->keyHit(io::KEY_PAGEDOWN))
    {
        if (--ExampleIndex < 0)
            ExampleIndex = 8;
    }
    
    if (LastExampleIndex != ExampleIndex)
    {
        // Hide the old shader example
        if (CurrentExample)
            CurrentExample->Show(false);
        
        switch (ExampleIndex)
        {
            case 0: CurrentExample = ExampleVM; break;
            case 1: CurrentExample = ExampleBB; break;
            case 2: CurrentExample = ExampleRP; break;
            case 3: CurrentExample = ExamplePT; break;
            case 4: CurrentExample = ExampleFE; break;
            case 5: CurrentExample = ExampleCS; break;
            case 6: CurrentExample = ExampleDF; break;
            case 7: CurrentExample = ExampleBM; break;
            case 8: CurrentExample = ExampleWS; break;
        }
        
        // Show the new shader example
        if (CurrentExample)
            CurrentExample->Show(true);
    }
    
    // Show/ hide the help text
    if (spControl->keyHit(io::KEY_F1))
        IsShowHelp = !IsShowHelp;
}

/**
 * In this function we render the scene and draw the text in the forground.
 */
void RenderScene()
{
    // If the current shader example renders the whole scene self don't draw the scene here
    if ( !CurrentExample || ( CurrentExample && !CurrentExample->Render() ) )
        spScene->renderScene();
    
    if (CurrentExample)
        DrawCenteredText(dim::point2di(ScrWidth/2, 15), CurrentExample->GetDescription(), 255);
    
    // Draw the help text
    if (IsShowHelp)
    {
        DrawCenteredText(
            dim::point2di(ScrWidth/2, 45), "Press PageUp/Down to switch between the shader examples", 255
        );
        DrawCenteredText(
            dim::point2di(ScrWidth/2, 75), "Press Enter (Or rather Return) to switch between demonstration modes", 255
        );
        DrawCenteredText(
            dim::point2di(ScrWidth/2, 105), "Press W to switch between wireframe modes", 255
        );
        DrawCenteredText(
            dim::point2di(ScrWidth/2, 135), "Press Left Mouse Button to rotate object/camera", 255
        );
        DrawCenteredText(
            dim::point2di(ScrWidth/2, 165), "Press Right Mouse Button to rotate light", 255
        );
        DrawCenteredText(
            dim::point2di(ScrWidth/2, 195), "Roll mouse wheel to move object/change sate", 255
        );
        DrawCenteredText(
            dim::point2di(ScrWidth/2, 225), "Press F1 to enable/disable help text", 255
        );
    }
}

/**
 * Delete all shader example objects.
 */
void ClearScene()
{
    MemoryManager::deleteMemory(ExampleVM);
    MemoryManager::deleteMemory(ExampleBB);
    MemoryManager::deleteMemory(ExampleRP);
    MemoryManager::deleteMemory(ExamplePT);
    MemoryManager::deleteMemory(ExampleFE);
    MemoryManager::deleteMemory(ExampleCS);
    MemoryManager::deleteMemory(ExampleDF);
    MemoryManager::deleteMemory(ExampleBM);
    MemoryManager::deleteMemory(ExampleWS);
}

/**
 * Function to draw centered text easyly.
 */
void DrawCenteredText(const dim::point2di &Pos, const io::stringc &Text, const video::color &Color)
{
    // Get the text size
    const dim::size2di TextSize(TextFont->getStringSize(Text));
    
    // Draw the text (background black and foreground with specified color)
    spRenderer->draw2DText(
        TextFont, Pos - dim::point2di(TextSize.Width/2, TextSize.Height/2) + 2, Text, video::color(0, 0, 0, Color.Alpha)
    );
    spRenderer->draw2DText(
        TextFont, Pos - dim::point2di(TextSize.Width/2, TextSize.Height/2), Text, Color
    );
}



// ============================================
