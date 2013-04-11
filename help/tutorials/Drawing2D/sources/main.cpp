//
// SoftPixel Engine Tutorial: Drawing2D - (21.6.2010)
//

#include <SoftPixelEngine.hpp>

using namespace sp;

#include "../../common.hpp"

/* === Global members === */

SoftPixelDevice* spDevice       = 0;
io::InputControl* spControl     = 0;
video::RenderSystem* spRenderer = 0;
video::RenderContext* spContext = 0;

const s32 ScrWidth = 800, ScrHeight = 600;

video::Texture* TexChar     = 0;
video::Texture* TexGround   = 0;

video::Font* Font = 0;

dim::point2di CharPos(ScrWidth/2, ScrHeight/2);
dim::point2di CharClip(0, 0);

dim::point2di WorldPos(-CharPos/2);

dim::point2di WalkClipSet[] = {
    dim::point2di(0, 0), dim::point2di(2, 0), dim::point2di(0, 0), dim::point2di(1, 3), // Up
    dim::point2di(2, 1), dim::point2di(2, 3), dim::point2di(2, 1), dim::point2di(2, 2), // Down
    dim::point2di(1, 0), dim::point2di(1, 2), dim::point2di(1, 0), dim::point2di(1, 1), // Right
    dim::point2di(0, 2), dim::point2di(0, 1), dim::point2di(0, 2), dim::point2di(0, 3), // Left
};

// Enumerations
enum EWalkDirections
{
    WALKDIR_NONE    = 1,
    
    WALKDIR_UP      = 0,
    WALKDIR_DOWN    = 4,
    WALKDIR_RIGHT   = 8,
    WALKDIR_LEFT    = 12,
};

// Declarations
void InitDevice();
void LoadResources();
void DrawCharacter(s32 PosX, s32 PosY, s32 ClipX, s32 ClipY);
void DrawCharacterWalking(s32 PosX, s32 PosY, s32 ClipX, s32 ClipY, const EWalkDirections WalkDirection, bool BoostWalking);
void DrawScene();
void DrawEffects(const s32 X, const s32 Y);


/* === All function definitions === */

int main()
{
    InitDevice();
    LoadResources();
    
    // Loop until the user presses the ESC key
    while (spDevice->updateEvents() && !spControl->keyDown(io::KEY_ESCAPE))
    {
        spRenderer->clearBuffers();
        
        DrawScene();
        
        spContext->flipBuffers();
    }
    
    // Delete all allocated memor (objects, textures etc.) and close the screen
    deleteDevice();
    
    return 0;
}

/**
 * Here we allocate a graphics device where the renderer can be choosen by the user.
 * We also create an input controller to check each user inputs (keyboard, mouse, joystick).
 * First parameters of "createGraphicsDevice" is the type of renderer. There are currently three
 * different renderers available: DRIVER_OPENGL, DRIVER_DIRECT3D9 and DRIVER_SOFTWARE.
 * The inhouse SoftwareRenderer surly does not support each feature like shaders or anti aliasing
 * but if the user do not have the requiered OpenGL/ DirectX version you can switch to this renderer
 * so that you can be sure the user will see something on the screen.
 * Next parameter is the screen resolution; Color bit depth (currenlty only 16, 24 and 32) and
 * window title (with default value of "").
 * There are further parameters (with default values): [bool isFullscreen = false] to enable full screen mode.
 * And the last parameter is a "SDeviceFlags" structure for more detailed configurations. Have a look at the
 * API documentation (http://softpixelengine.sourceforge.net/docu/index.html)
 * to learn more about the extended device flags.
 */
void InitDevice()
{
    // Create the graphics device
    spDevice    = createGraphicsDevice(
        ChooseRenderer(), dim::size2di(ScrWidth, ScrHeight), 32, "SoftPixel Engine - Drawing2D Tutorial"
    );
    
    // Create input control and get render system
    spControl   = spDevice->getInputControl();
    spRenderer  = spDevice->getRenderSystem();
    spContext   = spDevice->getRenderContext();
    
    // Make the background white (use "255" or "video::color(255)" or "video::color(255, 255, 255)")
    // Normally pointless because we draw a background image but if you have no image you can use
    // this function to set the background color
    spRenderer->setClearColor(255);
    
    // Update window title
    spContext->setWindowTitle(
        spContext->getWindowTitle() + " [ " + spRenderer->getVersion() + " ]"
    );
    
    // To be sure the programs runs on each PC with the same speed activate the frame rates
    spDevice->setFrameRate(100);
}

/**
 * In this function we load the textures, fonts etc. After loading the character texture
 * we set the color key to blend one color out. In this case (255, 0, 255) -> pink.
 * There are still further functions to set the color key. The wrap mode specifys how the texture coordinates
 * shall be manipulated. By default the wrap mode is "TEXWRAP_REPEAT" which allows texture coordinates
 * greater than 1.0 and less than 0.0. In our case we want to clamp the texture coordinates to edge
 * i.e. in a range of 0.0 to 1.0 that ugly borders cannot occur. Otherwise you maybe could see some
 * Pixels at the border you don't want ;-)
 * You can also set the wrap mode hor each texture coordinate axis individual. There are several "setWrapMode"
 * functions available. For example "setWrapMode(TEXWRAP_REPEAT, TEXWRAP_CLAMP, TEXWRAP_MIRROR)" would set
 * the wrap mode for the texture coordinate axles U, V and W (W for 3D textures).
 */
void LoadResources()
{
    // Resources path
    const io::stringc ResPath = ROOT_PATH + "Drawing2D/media/";
    
    // Load characters texture
    // (Characters picture found at: http://i33.tinypic.com/200tsfa.jpg)
    TexChar = spRenderer->loadTexture(ResPath + "CharactersTileset.png");
    
    // Set color key (255, 0, 255) with transparency 0
    TexChar->setColorKey(video::color(255, 0, 255, 0));
    
    // Set wrap mode to clamp-to-edges so that ugly borders cannot occur
    TexChar->setWrapMode(video::TEXWRAP_CLAMP);
    
    // Load further textures
    TexGround = spRenderer->loadTexture(ResPath + "Ground.jpg");
    
    // Load font with pixel size 25 and bold type
    Font = spRenderer->createFont("Arial", 25, video::FONT_BOLD);
}

/**
 * Here we have the function which draws the character finally.
 * We use the second of the five "draw2DImage" functions which allows use to set
 * the clipping rectangle and the size of the image. The clipping rectangle are
 * not more than the vertex coordinates (in the range [0.0 - 1.0]) for the four
 * vertices.
 */
void DrawCharacter(s32 PosX, s32 PosY, s32 ClipX, s32 ClipY)
{
    static const s32 CLIP_SIZE = 32;
    static const s32 CHAR_SIZE = 100;
    
    // Set the clipping rectangle
    dim::rect2df ClipRect(dim::rect2di(
        ClipX * CLIP_SIZE, ClipY * CLIP_SIZE,
        (ClipX + 1) * CLIP_SIZE, (ClipY + 1) * CLIP_SIZE
    ).cast<f32>());
    
    // Resize rectangle to range [0.0 - 1.0]
    ClipRect.Left   /= TexChar->getSize().Width;
    ClipRect.Top    /= TexChar->getSize().Height;
    ClipRect.Right  /= TexChar->getSize().Width;
    ClipRect.Bottom /= TexChar->getSize().Height;
    
    // Draw the 2d image with a clipping rect
    spRenderer->draw2DImage(
        TexChar,
        dim::rect2di(PosX - CHAR_SIZE/2, PosY - CHAR_SIZE/2, CHAR_SIZE, CHAR_SIZE),
        ClipRect
    );
}

/**
 * This function processes the character animation. It boosts the clipping (or rather walking) index.
 * To make sure the animation runs on all PCs with the same speed we use the "getMilliseconds" function
 * from the SoftPixel-device to determine when the time of 150 milliseconds has been elapsed.
 */
void DrawCharacterWalking(s32 PosX, s32 PosY, s32 ClipX, s32 ClipY, const EWalkDirections WalkDirection, bool BoostWalking)
{
    static const u64 WALK_DURATION      = 150;
    static const s32 WORLD_MOVESPEED    = 2;
    
    static EWalkDirections LastWalkDir = WALKDIR_DOWN;
    static s32 WalkIndex;
    static u64 WalkTime;
    
    const s32 WorldRange = TexGround->getSize().Width*4;
    
    // Update walking animation
    if ( ( BoostWalking || WalkIndex > 0 ) && io::Timer::millisecs() > WalkTime + WALK_DURATION )
    {
        WalkTime = io::Timer::millisecs();
        if (++WalkIndex > 3)
            WalkIndex = 0;
    }
    
    if (WalkDirection != WALKDIR_NONE)
        LastWalkDir = WalkDirection;
    
    if (BoostWalking || WalkIndex > 0)
    {
        // Update world movement
        switch (LastWalkDir)
        {
            case WALKDIR_UP:
                WorldPos.Y -= WORLD_MOVESPEED; break;
            case WALKDIR_DOWN:
                WorldPos.Y += WORLD_MOVESPEED; break;
            case WALKDIR_RIGHT:
                WorldPos.X += WORLD_MOVESPEED; break;
            case WALKDIR_LEFT:
                WorldPos.X -= WORLD_MOVESPEED; break;
            default:
                break;
        }
        
        // Clamp world position that the player cannot leave this virtual world
        if (WorldPos.X < -WorldRange - CharPos.X/2) WorldPos.X =  WorldRange - CharPos.X/2;
        if (WorldPos.X >  WorldRange - CharPos.X/2) WorldPos.X = -WorldRange - CharPos.X/2;
        if (WorldPos.Y < -WorldRange - CharPos.Y/2) WorldPos.Y =  WorldRange - CharPos.Y/2;
        if (WorldPos.Y >  WorldRange - CharPos.Y/2) WorldPos.Y = -WorldRange - CharPos.Y/2;
    }
    
    // Draw the character with animation
    DrawCharacter(
        PosX,
        PosY,
        ClipX * 3 + WalkClipSet[LastWalkDir + WalkIndex].X,
        ClipY * 4 + WalkClipSet[LastWalkDir + WalkIndex].Y
    );
}

/**
 * This function draws a horizontal centered text. The text is drawn two times to get a good
 * contrast. First pass back as background and second pass with the specified color.
 */
void DrawCenteredText(s32 PosY, const io::stringc &Text, const video::color &Color = video::color(255, 255, 255, 200))
{
    // Get the half with of the specified text
    const s32 TextHalfWidth = Font->getStringSize(Text).Width/2;
    
    // Drawing text first pass
    spRenderer->draw2DText(
        Font,                                                   // Use our loaded font "arial"
        dim::point2di(ScrWidth/2 - TextHalfWidth, PosY) + 2,    // Center the text
        Text,                                                   // Use the text created above
        video::color(0, 0, 0, Color.Alpha)                      // Draw the text with black color
    );
    
    // Drawing text second pass
    spRenderer->draw2DText(
        Font,                                               // Use our loaded font "arial"
        dim::point2di(ScrWidth/2 - TextHalfWidth, PosY),    // Center the text
        Text,                                               // Use the text created above
        Color                                               // Draw the text with the specified color
    );
}

/**
 * Our drawing functions which draws the whole scene (no 3d scene for sure ^^).
 * It also examines the user input for the arrow keys to move the character.
 * At first we draw the background. Different to 3d scenes is that we have to draw images
 * in the correct order (background first, foreground after). In 3d we only need to
 * create 3d objects. Drawing (or rather rendering) 3d is asumed by the SceneManager.
 * More over in 3d there is a depth buffer to make pixels volumetric. in 2d we only have the
 * color buffer to draw images, lines and other primitives.
 */
void DrawScene()
{
    // Set the ground clipping rectangle
    dim::rect2df ClipRect(dim::rect2di(
        WorldPos.X, WorldPos.Y,
        WorldPos.X + ScrWidth/2, WorldPos.Y + ScrHeight/2
    ).cast<f32>());
    
    // Resize the clipping rectangle
    const dim::size2di TexGroundSize(TexGround->getSize());
    
    ClipRect.Left   /= TexGroundSize.Width;
    ClipRect.Top    /= TexGroundSize.Height;
    ClipRect.Right  /= TexGroundSize.Width;
    ClipRect.Bottom /= TexGroundSize.Height;
    
    // Draw the ground over the whole screen
    spRenderer->draw2DImage(
        TexGround, dim::rect2di(0, 0, ScrWidth, ScrHeight), ClipRect
    );
    
    // Draw the special effects
    DrawEffects(-WorldPos.X*2, -WorldPos.Y*2);
    
    // Update walking
    EWalkDirections CharWalkDir = WALKDIR_NONE;
    
    if (spControl->keyDown(io::KEY_UP))
        CharWalkDir = WALKDIR_UP;
    if (spControl->keyDown(io::KEY_DOWN))
        CharWalkDir = WALKDIR_DOWN;
    if (spControl->keyDown(io::KEY_RIGHT))
        CharWalkDir = WALKDIR_RIGHT;
    if (spControl->keyDown(io::KEY_LEFT))
        CharWalkDir = WALKDIR_LEFT;
    
    if (spControl->keyHit(io::KEY_PAGEUP))
    {
        if (++CharClip.X > 2)
            CharClip.X = 0;
    }
    if (spControl->keyHit(io::KEY_PAGEDOWN))
    {
        if (--CharClip.X < 0)
            CharClip.X = 2;
    }
    
    // Draw the animated character
    DrawCharacterWalking(CharPos.X, CharPos.Y, CharClip.X, CharClip.Y, CharWalkDir, CharWalkDir != WALKDIR_NONE);
    
    // Draw some text for our tutorial
    DrawCenteredText(15, "Welcome to the new Drawing2D tutorial!");
    DrawCenteredText(50, "Use the arrow keys to move your character");
    DrawCenteredText(75, "Press PageUp/Down to change your character");
}

/**
 * Now let's draw some effects and special features in 2d drawing.
 */
void DrawEffects(const s32 X, const s32 Y)
{
    /**
     * Draw a rectangle with several colors. One color for each corner (or rather vertex).
     */
    spRenderer->draw2DRectangle(
        dim::rect2di(X - 250, Y - 250, X - 50, Y - 50),
        video::color(255, 0, 0),
        video::color(0, 255, 0),
        video::color(0, 0, 255),
        video::color(255, 255, 0)
    );
    
    /**
     * Draw a rectangle (in this case an individual 2d polygon) with a nice special effect using the
     * RHW (Reciprocal Homogeneous W) coordinate.
     * First two parameters of the SPrimitiveVertex2D structure are the X and Y screen coordinates.
     * Next two are the texture coordinates U and V. Then comes the color and the
     * RHW coordinate (optional; by default 1.0; may not be 0 and should not be less then 0).
     * The "draw2DPolygon" function allows you to draw any kind of primitives. These are:
     * PRIMITIVE_POINTS, PRIMITIVE_LINES, PRIMITIVE_LINE_STRIP, PRIMITIVE_LINE_LOOP, PRIMITIVE_TRIANGLES,
     * PRIMITIVE_TRIANGLE_STRIP, PRIMITIVE_TRIANGLE_FAN, PRIMITIVE_QUADS, PRIMITIVE_QUAD_STRIP, PRIMITIVE_POLYGON
     * where the last three values and the line-loop are only available using OpenGL.
     * The last parameter of "draw2DPolygon" defines the count of vertices. This value may be less then
     * the count of elements in the array but may not be greater!
     */
    static f32 EffectAngle;
    EffectAngle += 3;
    
    scene::SPrimitiveVertex2D PrimVertices[4] = {
        //                           X        Y     U  V             Color                      RHW
        scene::SPrimitiveVertex2D( 50.0f + X, -250.0f + Y, 0.0f, 0.0f, video::color(255,   0,   0), math::Sin(EffectAngle)*2+3       ),
        scene::SPrimitiveVertex2D(250.0f + X, -250.0f + Y, 1.0f, 0.0f, video::color(  0, 255,   0), math::Cos(EffectAngle)*2+3       ),
        scene::SPrimitiveVertex2D(250.0f + X,  -50.0f + Y, 1.0f, 1.0f, video::color(  0,   0, 255), math::Sin(EffectAngle+180)*2+3   ),
        scene::SPrimitiveVertex2D( 50.0f + X,  -50.0f + Y, 0.0f, 1.0f, video::color(255, 255,   0), math::Cos(EffectAngle+180)*2+3   )
    };
    
    spRenderer->draw2DPolygon(video::PRIMITIVE_TRIANGLE_FAN, PrimVertices, 4);
    
    /**
     * Draw a further 2d polygon to see what the RHW coordinate actual does.
     * We use the same array because we can tell the function we only want to use 3 vertices.
     * In this case we only set the position. The position can be from the type dim::point2di or dim::vector4f.
     * We use a 4 dimensional vector to set additional the RHW coordinate (X, Y, Z, W) or (X, Y, 0.0, RHW).
     * To get an equilateral triangle locate the vertices' position with sine and cosine.
     */
    PrimVertices[0].setPosition(dim::vector4df(math::Sin(  0)*200.0f + X, 200.0f - math::Cos(  0)*200 + Y, 0.0f, 7.0f));
    PrimVertices[1].setPosition(dim::vector4df(math::Sin(120)*200.0f + X, 200.0f - math::Cos(120)*200 + Y, 0.0f, 1.0f));
    PrimVertices[2].setPosition(dim::vector4df(math::Sin(240)*200.0f + X, 200.0f - math::Cos(240)*200 + Y, 0.0f, 1.0f));
    
    spRenderer->draw2DPolygon(video::PRIMITIVE_TRIANGLES, PrimVertices, 3);
    
    /**
     * Draw the ground image again as a rotatable image with the radius of 100 pixels.
     * There are two functions to rotate an image. The second one allows you to set the
     * rotation center (in percent: 0.5 conform to the middle).
     */
    static f32 TexAngle;
    TexAngle += 3;
    
    spRenderer->draw2DImage(
        TexGround,                      // Image (or rather texture) object which is to be drawn
        dim::point2di(X, Y - 500),      // Center point
        TexAngle,                       // Rotation angle
        100,                            // Radius
        video::color(0, 255, 255, 200)  // Draw with a transparentcy turquoise color
    );
    
    // Update info text with a timer
    static io::Timer FPSTimer(true);
    static io::Timer TextTimer(500ul);
    static bool InfoTextSwitch;
    
    if (TextTimer.finish())
    {
        TextTimer.reset();
        InfoTextSwitch = !InfoTextSwitch;
    }
    
    io::stringc InfoText(
        "[ FPS: " + io::stringc::numberFloat(static_cast<f32>(FPSTimer.getFPS()), 1) + " ] Rotatable Text Drawing"
    );
    
    InfoText += (InfoTextSwitch ? ":-)" : ":-D");
    
    // Setup text transformation
    static f32 FontAngle;
    FontAngle += 3.0f;
    
    dim::matrix4f Mat;
    
    Mat.rotateZ(math::Sin(FontAngle)*15.0f);
    Mat.scale(1.5f);
    
    // Draw text on ground with transformation
    spRenderer->setFontTransformation(Mat);
    {
        spRenderer->draw2DText(
            Font, dim::point2di(X, Y + 150), InfoText, video::color(30, 50, 230), video::TEXT_CENTER
        );
    }
    spRenderer->setFontTransformation(dim::matrix4f());
}




// ============================================
