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
scene::SceneGraph* spScene          = 0;

const s32 ScrWidth = 800, ScrHeight = 600;

scene::SceneNode* CamRoot   = 0;
scene::Camera* Cam          = 0;
scene::Light* Light         = 0;
scene::Mesh* Shape          = 0;

video::Font* TextFont = 0;

dim::point2di MouseSpeed;
f32 CamPitch = 45.0f, CamYaw = 0.0f;

f32 CurRadius = 3.0f;

static const io::stringc ShapeFilename  = "media/TerrainShape.spm";
static const dim::vector3df ShapeSize   = 15;

// Declarations
void InitDevice();
void CreateScene();
void UpdateScene();
void DrawCenteredText(
    s32 PosY, const io::stringc &Text, const video::color &Color = video::color(255, 255, 255, 200)
);
void DrawInformation();
bool GetSceneIntersection(dim::vector3df &Intersection);
void DrawShape(const dim::vector3df &Pos, const f32 Direction, const f32 Radius = 3.0f);
void FlatShape(const dim::vector3df &Pos, const f32 Radius = 3.0f);
void DrawPaint(const dim::vector3df &Pos, const video::color &Color, bool AddColor = true, const f32 Radius = 3.0f);
void FlatPaint(const dim::vector3df &Pos, const video::color &Color, const f32 Radius = 3.0f);


/* === All function definitions === */

int main()
{
    InitDevice();
    CreateScene();
    
    while (spDevice->updateEvent() && !spControl->keyDown(io::KEY_ESCAPE))
    {
        spRenderer->clearBuffers();
        
        UpdateScene();
        
        spScene->renderScene();
        
        DrawInformation();
        
        spContext->flipBuffers();
    }
    
    deleteDevice();
    
    return 0;
}

void InitDevice()
{
    spDevice    = createGraphicsDevice(
        ChooseRenderer(), dim::size2di(ScrWidth, ScrHeight), 32, "SoftPixel Engine - Shaping tutorial"
    );
    
    spControl   = spDevice->getInputControl();
    spRenderer  = spDevice->getRenderSystem();
    spContext   = spDevice->getRenderContext();
    
    spScene     = spDevice->createSceneGraph();
    
    spContext->setWindowTitle(
        spContext->getWindowTitle() + " [ " + spRenderer->getVersion() + " ]"
    );
    
    spDevice->setFrameRate(100);
}

void CreateScene()
{
    // Load the font
    TextFont = spRenderer->loadFont("Arial", 20, video::FONT_BOLD);
    
    //! At first we create a node (none visible object) as the camera root.
    CamRoot = spScene->createNode();
    CamRoot->setRotation(dim::vector3df(45, 0, 0));
    
    // Create a camera
    Cam = spScene->createCamera();
    Cam->setPosition(dim::vector3df(0, 0, -13));
    
    /**
     * We set the CamRoot object as the camera's parent. If you move, turn or scale the parent this will
     * also effect the child (in this case the camera). So it is easier for us to turn the camera around a scene.
     * Only from the visual aspect we could also turn the object but because we want to make intersection tests
     * then we had to turn the plane's normal (dim::plane3d<T>::Normal). So it's easier for us in this way.
     */
    Cam->setParent(CamRoot);
    
    // Create a light
    Light = spScene->createLight(scene::LIGHT_DIRECTIONAL);
    Light->setRotation(dim::vector3df(45, 10, 0));
    
    spScene->setLighting(true);
    
    // Make background bright
    spRenderer->setClearColor(video::color(128, 200, 255));
    
    /**
     * Our Shape object is a flat plane with enough segments (or rather triangles) that we can
     * change its shape by pressing the mouse button. Think at ingame level editors in games like
     * "The Sims" where you can modify the terrain in the build-mode.
     * Our plane will consist of 50 segments (50 * 50 quads -> 50 * 50 * 2 trianlges)
     */
    Shape = spScene->createMesh(scene::MESH_PLANE, 50);
    
    /**
     * A plane has by default always the size of (1 | 1 | 1). That's a little bit
     * too small for your example so we resize it using "setSize".
     */
    Shape->setScale(ShapeSize);
    
    /**
     * We also want the object double faced using "setRenderFace" (or rather culling mode).
     * In the SoftPixel Engine by default the front side of a triangle will be drawn. But you can change this
     * setting by using this function. More over you can invert this setting global by using the
     * "VideoDriver::setFrontFace" function.'
     */
    Shape->getMaterial()->setRenderFace(video::FACE_BOTH);
    
    /**
     * A plane is by default flat shaded. Because when we shape the model manual we have to set it to
     * gouraud shading. With flat shading the normals (vectors needed for lighting calculations) of each vertex
     * will be computed and normalized (normalized vectors have always the length of 1.0) after calling the
     * "Mesh::updateNormals" function. These normals will stand vertical to its triangle area.
     * With gouraud (or rather smooth) shading the normals of the vertices which have the same space coordinate
     * will get an average value. The result is that each of these adjacency vertices will have the same
     * normal. That occurs that the triangles have vertices which different normals and the lighting computations
     * will result different brightnesses and the vertices' colors will be interpolated. That makes the
     * object more smooth and hide a little bit the sharp edges.
     */
    Shape->setShading(video::SHADING_GOURAUD);
}

void UpdateScene()
{
    /**
     * The first thing we do in this function is to get the mouse (or rather cursor) speed. If you use the "getCursorSpeed"
     * function only if you need it (maybe first when you press a mouse button) but the mouse speed was not determined
     * before the first value (the mouse speed itself) will be very high because you still moved the mouse until you
     * needed this value. So if you need the mouse speed always call this function at least one times.
     */
    MouseSpeed = spControl->getCursorSpeed();
    
    /**
     * Then allow the user to change the wirefame mode. There are three kinds of wireframe: Solid, Lines and Points.
     * By default the wireframe mode is solid. There are several "setWireframe" functions. The function we use
     * is part of the scene manager and global. i.e. it sets the wireframe for each triangle object (Meshes, Terrains and Billboards).
     * This function exist two times for the scene manager and for each of these objects. The first one only expects
     * one parameter which specify the front and back side's wireframe mode. The second one expects two parameters which
     * specify the individual side's wireframe mode ('front' first, 'back' second). Use the second function when
     * the render face is 'back' or 'both' and you want to set their wireframe individual.
     */
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
    
    // Make intersection tests with the scene
    dim::vector3df Intersection;
    if (GetSceneIntersection(Intersection))
    {
        if (spControl->mouseDown(io::MOUSE_LEFT))
        {
            if (spControl->keyDown(io::KEY_CONTROL))
                FlatShape(Intersection, CurRadius);
            else if (spControl->keyDown(io::KEY_INSERT))
                FlatPaint(Intersection, 255, CurRadius);
            else if (spControl->keyDown(io::KEY_DELETE))
                FlatPaint(Intersection, 0, CurRadius);
            else if (spControl->keyDown(io::KEY_R))
                DrawPaint(Intersection, video::color(255, 0, 0), true, CurRadius);
            else if (spControl->keyDown(io::KEY_G))
                DrawPaint(Intersection, video::color(0, 255, 0), true, CurRadius);
            else if (spControl->keyDown(io::KEY_B))
                DrawPaint(Intersection, video::color(0, 0, 255), true, CurRadius);
            else
                DrawShape(Intersection, 1.0f, CurRadius);
        }
        
        if (spControl->mouseDown(io::MOUSE_RIGHT))
        {
            if (spControl->keyDown(io::KEY_R))
                DrawPaint(Intersection, video::color(255, 0, 0), false, CurRadius);
            else if (spControl->keyDown(io::KEY_G))
                DrawPaint(Intersection, video::color(0, 255, 0), false, CurRadius);
            else if (spControl->keyDown(io::KEY_B))
                DrawPaint(Intersection, video::color(0, 0, 255), false, CurRadius);
            else
                DrawShape(Intersection, -1.0f, CurRadius);
        }
    }
    
    /**
     * Turn the camera root (or rather its parent).
     * This is a typical way to turn a camera. We use the mouse speed and clamp the pitch (for X-axis)
     * to the range of [-90 .. 90]. To rotate an object (or rather the object's rotation matrix) using
     * a vector you can use the "setRotation" function. This rotation will rotate the matrix's axles
     * in the following order: Y, X, Z. If you want to rotate an object different use the "setRotationMatrix"
     * function using your own matrix (dim::matrix4f). This object you can rotate using "rotateX", "rotateY",
     * "rotateZ" or "rotateYXZ".
     */
    if (spControl->mouseDown(io::MOUSE_MIDDLE))
    {
        CamPitch    += (f32)MouseSpeed.Y / 2;
        CamYaw      += (f32)MouseSpeed.X / 2;
        
        if (CamPitch < -90) CamPitch = -90;
        if (CamPitch >  90) CamPitch =  90;
        
        CamRoot->setRotation(dim::vector3df(CamPitch, CamYaw, 0));
    }
    
    if (spControl->keyDown(io::KEY_CONTROL))
    {
        // Change the current radius for drawing
        CurRadius += (f32)spControl->getMouseWheel() / 2;
        math::Clamp(CurRadius, 0.5f, 8.0f);
    }
    else
    {
        // Translate camera forwards/ backwards
        Cam->translate(dim::vector3df(0, 0, (f32)spControl->getMouseWheel()));
        
        if (Cam->getPosition().Z > -3)
            Cam->setPosition(dim::vector3df(0, 0, -3));
        else if (Cam->getPosition().Z < -25)
            Cam->setPosition(dim::vector3df(0, 0, -25));
    }
    
    // Save the model when the user hits the F9 key
    if (spControl->keyHit(io::KEY_F9))
        spScene->saveMesh(Shape, ShapeFilename);
    else if (spControl->keyHit(io::KEY_F5))
    {
        // Check if the file does exist
        if (io::FileSystem().findFile(ShapeFilename))
        {
            // Load a new mesh
            scene::Mesh* NewMesh = spScene->loadMesh(ShapeFilename);
            
            if (NewMesh->getTriangleCount() > 0)
            {
                // Delete the old mesh
                spScene->deleteNode(Shape);
                
                // Replace the old mesh with the new one
                Shape = NewMesh;
                
                // Reset mesh settings
                Shape->setScale(ShapeSize);
                Shape->getMaterial()->setRenderFace(video::FACE_BOTH);
                Shape->setShading(video::SHADING_GOURAUD);
            }
            else
            {
                // If the new mesh has no triangles loading must be failed, so delete the new one
                spScene->deleteNode(NewMesh);
                
                io::Log::warning("New shape could not load correctly", io::LOG_MSGBOX);
            }
        }
        else
            io::Log::warning("Your shape has not saved until yet", io::LOG_MSGBOX);
    }
}

/**
 * Function to draw centered text easyly.
 */
void DrawCenteredText(s32 PosY, const io::stringc &Text, const video::color &Color)
{
    // Get the text size
    const dim::size2di TextSize(TextFont->getStringSize(Text));
    
    // Draw the text (background black and foreground with specified color)
    spRenderer->draw2DText(
        TextFont, dim::point2di(ScrWidth/2 - TextSize.Width/2 + 2, PosY + 2), Text, video::color(0, 0, 0, Color.Alpha)
    );
    spRenderer->draw2DText(
        TextFont, dim::point2di(ScrWidth/2 - TextSize.Width/2, PosY), Text, Color
    );
}

/**
 * Draw the help information that the user knows which key is to be pressed.
 */
void DrawInformation()
{
    static bool ShowHelp = true;
    
    if (spControl->keyHit(io::KEY_F1))
        ShowHelp = !ShowHelp;
    
    spRenderer->beginDrawing2D();
    
    if (ShowHelp)
    {
        spRenderer->draw2DRectangle(
            dim::rect2di(0, 0, ScrWidth, 315), video::color(0, 0, 0, 64)
        );
        
        DrawCenteredText(5, "F1 -> Help on/off");
        DrawCenteredText(25, "W -> Swtich wireframe mode");
        DrawCenteredText(45, "Mouse wheel -> Move camera");
        DrawCenteredText(65, "Control + MouseWheel -> Change Radius (" + io::stringc(CurRadius) + ")");
        DrawCenteredText(85, "F5 -> Load shape");
        DrawCenteredText(105, "F9 -> Save shape");
        DrawCenteredText(125, "Left mouse button -> Draw shape up");
        DrawCenteredText(145, "Right mouse button -> Draw shape down");
        DrawCenteredText(165, "Control + Left mouse button -> Reset shape");
        DrawCenteredText(185, "R + Left mouse button -> Draw red");
        DrawCenteredText(205, "G + Left mouse button -> Draw green");
        DrawCenteredText(225, "B + Left mouse button -> Draw blue");
        DrawCenteredText(245, "Middle mouse button -> Turn camera");
        DrawCenteredText(265, "Insert + Left mouse button -> Repaint white");
        DrawCenteredText(285, "Delete + Left mouse button -> Repaint black");
    }
    
    spRenderer->endDrawing2D();
}

/**
 * This function executes the intersection test. In this example we only want an intersection
 * with a flat plane (not the shape, always a flat plane). For those simple intersection tests
 * we don't need to create a collision detector with picking meshes etc. We only call
 * "dim::plane3d<T>::checkLineIntersection" to check if the picking line intersects with the plane.
 * The picking line only seems to be a point: the cursor position. But picking always can only executed
 * using a line (or rather ray). This picking line starts from the camera position and ends to the
 * picking depth/ length (by default 1000 units). The "getPickingLine" function creates a suitable
 * 3d line (dim::line3df) projected by the camera's projection matrix.
 * The plane gets a normal (in this case [0 | 1 | 0] an upright vector) and the distance from
 * the origin (in this case 0.0). If an intersection has been detected the return value of
 * "checkLineIntersection" is 'true' otherwise 'false.
 */
bool GetSceneIntersection(dim::vector3df &Intersection)
{
    const dim::line3df PickLine(
        Cam->getPickingLine(spControl->getCursorPosition())
    );
    
    return dim::plane3df(dim::vector3df(0, 1, 0), 0.0f).checkLineIntersection(
        PickLine.Start, PickLine.End, Intersection
    );
}

/**
 * This function draws or rather modifies our shape. "Pos" is the position (or rather the
 * intersection point with our picking plane); "Direction" shall only be 1.0 or -1.0 set
 * if we shape a hull up or down; "Radius" specifies the radius of our pencil tool.
 * "getGlobalLocation" gives us the global object's transformation matrix from your Shape.
 * With "getVertexCount" we go through each object's vertex. If you only want the vertices
 * from a special surface you can set the surface's number (beginning with 0);
 * e.g. "Shape->getVertexCount(0)". The default value is scene::IGNORE_SURFACE which occurs
 * that each surface is used.
 * "getVertexCoord" returns the space coordinate of the vertex (number i). By multiplying
 * the coordinate with the global matrix we have the real vertex coordinate how it also used
 * in the graphics card. We need the real coordinate to make further distance checks.
 * Then we compute the distance between the transformed vertex position (TmpPos) and the
 * picking point (Pos). By compiting this distance we ignore the vertex's height.
 * If the distance is near enough we compute how much the vertex height shall be changed
 * and set the new vertex position.
 */
void DrawShape(const dim::vector3df &Pos, const f32 Direction, const f32 Radius)
{
    dim::vector3df Coord, TmpPos;
    f32 Distance;
    const dim::matrix4f Mat(Shape->getTransformation(true));
    
    // Get the mesh buffer
    video::MeshBuffer* Surface = Shape->getMeshBuffer(0);
    
    // Loop for each vertex
    for (u32 i = 0; i < Shape->getVertexCount(); ++i)
    {
        Coord = Surface->getVertexCoord(i);
        
        TmpPos = Mat * Coord;
        
        // Compute the distance between the intersection and the vertex's coordinate (at the floor -> Y = 0)
        Distance = math::getDistance(
            dim::point2df(Pos.X, Pos.Z), dim::point2df(TmpPos.X, TmpPos.Z)
        );
        
        if (Distance < Radius)
        {
            Distance = (Radius - Distance + 0.1f);
            
            Coord.Y += Direction * Distance/1000;
            
            if (Coord.Y < -0.5f) Coord.Y = -0.5f;
            if (Coord.Y >  0.5f) Coord.Y =  0.5f;
            
            Surface->setVertexCoord(i, Coord);
        }
    }
    
    /**
     * After modifying the Shape we update the normals. This occurs automatically a call of
     * "updateMeshBuffer". If you do not want to update the normals because you maybe only changed
     * the color or texture coordinates or several vertices you can use this function (updateMeshBuffer).
     * "updateNormals" is certainly slower because each vertex's normal will be re-compted.
     * If you only change the indices of several triangles you can use the "updateIndexBuffer" function.
     * And if you only want to update a mesh's surface you can specify the surface's number
     * e.g. "Shape->updateMeshBuffer(0)".
     */
    Shape->updateNormals();
}

/**
 * This function resets the vertice's position back to the floor (Y-Axis is 0.0).
 */
void FlatShape(const dim::vector3df &Pos, const f32 Radius)
{
    dim::vector3df Coord, TmpPos;
    f32 Distance;
    const dim::matrix4f Mat(Shape->getTransformation(true));
    
    // Get the mesh buffer
    video::MeshBuffer* Surface = Shape->getMeshBuffer(0);
    
    // Loop for each vertex
    for (u32 i = 0; i < Shape->getVertexCount(); ++i)
    {
        Coord = Surface->getVertexCoord(i);
        
        TmpPos = Mat * Coord;
        
        // Compute the distance between the intersection and the vertex (at the floor -> Y = 0)
        Distance = math::getDistance(
            dim::point2df(Pos.X, Pos.Z), dim::point2df(TmpPos.X, TmpPos.Z)
        );
        
        if (Distance < Radius)
        {
            Coord.Y = 0.0f;
            
            Surface->setVertexCoord(i, Coord);
        }
    }
    
    Shape->updateNormals();
}

/**
 * This function does not shape our object but it paints it. We use temporary floating-point
 * values to make more precised interpolations between the multiple colors.
 */
void DrawPaint(const dim::vector3df &Pos, const video::color &Color, bool AddColor, const f32 Radius)
{
    dim::vector3df Coord, TmpPos;
    video::color VertexColor;
    f32 Distance;
    f32 TmpColor[3], TmpVertexColor[3], FinalColor[3];
    const dim::matrix4f Mat(Shape->getTransformation(true));
    
    TmpColor[0] = static_cast<f32>(Color.Red    ) / 255;
    TmpColor[1] = static_cast<f32>(Color.Green  ) / 255;
    TmpColor[2] = static_cast<f32>(Color.Blue   ) / 255;
    
    // Get the mesh buffer
    video::MeshBuffer* Surface = Shape->getMeshBuffer(0);
    
    // Loop for each vertex
    for (u32 i = 0; i < Shape->getVertexCount(); ++i)
    {
        Coord = Surface->getVertexCoord(i);
        
        TmpPos = Mat * Coord;
        
        // Compute the distance between the intersection and the vertex (at the floor -> Y = 0)
        Distance = math::getDistance(
            dim::point2df(Pos.X, Pos.Z), dim::point2df(TmpPos.X, TmpPos.Z)
        );
        
        if (Distance < Radius)
        {
            Distance = (Radius - Distance + 0.1f) / Radius / 15;
            
            VertexColor = Surface->getVertexColor(i);
            
            TmpVertexColor[0] = static_cast<f32>(VertexColor.Red    ) / 255;
            TmpVertexColor[1] = static_cast<f32>(VertexColor.Green  ) / 255;
            TmpVertexColor[2] = static_cast<f32>(VertexColor.Blue   ) / 255;
            
            if (AddColor)
            {
                FinalColor[0] = TmpColor[0] * Distance + TmpVertexColor[0];
                FinalColor[1] = TmpColor[1] * Distance + TmpVertexColor[1];
                FinalColor[2] = TmpColor[2] * Distance + TmpVertexColor[2];
            }
            else
            {
                FinalColor[0] = TmpVertexColor[0] - TmpColor[0] * Distance;
                FinalColor[1] = TmpVertexColor[1] - TmpColor[1] * Distance;
                FinalColor[2] = TmpVertexColor[2] - TmpColor[2] * Distance;
            }
            
            math::Clamp(FinalColor[0], 0.0f, 1.0f);
            math::Clamp(FinalColor[1], 0.0f, 1.0f);
            math::Clamp(FinalColor[2], 0.0f, 1.0f);
            
            Surface->setVertexColor(
                i, video::color((u8)(FinalColor[0] * 255), (u8)(FinalColor[1] * 255), (u8)(FinalColor[2] * 255))
            );
        }
    }
    
    /**
     * Only update the mesh buffer because we only changed the vertice's color.
     * No normals need to be re-computed.
     */
    Shape->updateMeshBuffer();
}

/**
 * This function only resets the vertices' color.
 */
void FlatPaint(const dim::vector3df &Pos, const video::color &Color, const f32 Radius)
{
    dim::vector3df Coord, TmpPos;
    f32 Distance;
    const dim::matrix4f Mat(Shape->getTransformation(true));
    
    // Get the mesh buffer
    video::MeshBuffer* Surface = Shape->getMeshBuffer(0);
    
    // Loop for each vertex
    for (u32 i = 0; i < Shape->getVertexCount(); ++i)
    {
        Coord = Surface->getVertexCoord(i);
        
        TmpPos = Mat * Coord;
        
        // Compute the distance between the intersection and the vertex (at the floor -> Y = 0)
        Distance = math::getDistance(
            dim::point2df(Pos.X, Pos.Z), dim::point2df(TmpPos.X, TmpPos.Z)
        );
        
        if (Distance < Radius)
            Surface->setVertexColor(i, Color);
    }
    
    Shape->updateMeshBuffer();
}






// ============================================
