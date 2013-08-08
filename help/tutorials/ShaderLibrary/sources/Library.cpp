//
// SoftPixel Engine Tutorial: ShaderLibrary - (04/09/2010)
//

#include "Library.hpp"


/**
 * This documentation block is for those who don't know what shaders actually are and what they do.
 * For Direct3D shaders primary exist since version 8. They where assembler like shaders. Since Direct3D9
 * with its new "Shader Model 2.0" the high level shader (HLSL: High Level Shading Language) were introduced.
 * For OpenGL the high level shader (GLSL: OpenGL Shading Language - also glSlang) were introduced with
 * version 2.0. Assembler like shader for OpenGL already exist since version 1.3.
 * But what actually does a shader? A shader is a program which replaces the fixed-function-pipeline (FFP).
 * In the FFP you have some options to configure your 3D model surface. e.g. the color, shininess, texturing etc.
 * But at any time you will see that with the FFP you are really constricted. With shaders you can program
 * the surface in very high detail. With a vertex shader you can manipulate the transformation for each vertex
 * Which included the coordinate, normal vector, color, texture coordinates etc. With a pixel shader - as the name implies -
 * you can manipulate each pixel including basically its color but also its depth.
 * A small disadvantage when using shaders - particular for beginners - is that when you use them you need to
 * assume the whole work. i.e. lighting- and fog calculations, texturing etc.
 * The high level shading languages are inspired by the C programming language. Or rather it's a little bit a mixture
 * of C and C++. e.g. a shader always has a main function. In OpenGL it's always called "void main() { ... }".
 * The shader programs need also to be compiled like normal C programs. But shaders only be compiled
 * when the main program has been started. Advantage is that the shader programs can be changed without that
 * the whole program needs to be compiled again.
 * The learn more how to write the shaders in practice look in side the shader examples in the "shader/" directory.
 */

/* === ShaderExample === */

ShaderExample::ShaderExample() :
    ShdClass_   (0),
    ShdVertex_  (0),
    ShdPixel_   (0),
    Object_     (0)
{
}
ShaderExample::~ShaderExample()
{
    spScene->deleteNode(Object_);
    spRenderer->deleteShaderClass(ShdClass_);
}

bool ShaderExample::Render()
{
    return false;
}

/**
 * This is our main function to load, compile and link the shader programs.
 * Our shader always have the same layout: We specify the name and each shader exist
 * one times for OpenGL and one times for Direct3D9. e.g. for the shader effect "CelShading"
 * the files "CelShading.glvert" (OpenGL Vertex Shader) and "CelShading.hlsl" (Direct3D9 Shader)
 * exist. More over they need to exist in the same directory ("shader/"). If the shader effect
 * also has a pixel shader we set the parameter "IsVertexPixel" to true. A shader program can
 * consist of only a vertex shader but when a pixel shader is used a vertex shader also needs
 * to be defined. For Direct3D9 we need to specify explicit the shader version. Since Direct3D9.0c
 * the "Shader Model 3.0" is available. Some new features are available in this shader model.
 * Amongst others to pass a normal vector to the pixel shader from the vertex shader.
 * Besides much more instructions can be passed to the shader programs. For many shader examples
 * we need to specify a callback procedure for the shader. This procedure (or function, or method)
 * will be called just before the shader programs are bound. A callback is normally always used for
 * Direct3D9 shaders because there you have to set the matrix transformations
 * (world-, view- and projection matrix) by your self. For OpenGL there are several possibilites
 * to disclaim a callback.
 */
bool ShaderExample::LoadShader(
    const io::stringc &Name,
    const video::ShaderObjectCallback &CallbackProcOGL,
    const video::ShaderObjectCallback &CallbackProcD3D9,
    bool IsVertexPixel, bool IsShaderModel3)
{
    Description_ = Name;
    
    /**
     * This creates our shader table. A shader table is used to combine several shader types.
     * e.g. to use a vertex-, pixel- and geometry shader or only use a vertex shader.
     * For Direct3D11 you need at least a vertex- and a pixel shader because fixed-function-pipeline
     * no longer exists. But for OpenGL and Direct3D9 you can disclaim creating a shader table when
     * using only a vertex shader. For this you can just load a shader and setting the first parameter
     * which sepcifies the shader table to zero. Then the shader will create a shader table and link
     * the program self.
     */
    ShdClass_ = spRenderer->createShaderClass();
    
    const io::stringc ShaderPath = BasePath + "shaders/";
    
    if (spRenderer->getRendererType() == video::RENDERER_OPENGL)
    {
        /**
         * The first parameter specifies our shader table.
         * The next one says which kind of shader it is. The first one has to be a vertex shader.
         * The third parameter tells the renderer which version is used. Actually for OpenGL
         * unimportant because the shader version is normally set in the shader (#version 120)
         * but for Direct3D9 it is absolute necessary.
         * The fourth parameter specifies the shader name. For OpenGL each shader needs to be
         * separated. But for Direct3D9 several shaders can defined in one file. In this
         * tutorial we always use the same file (*.hlsl) for Direct3D9.
         * There is a further parameter with the default value ("") but for Direct3D9 shaders
         * we need to set them to the shader main function. We always use "VertexMain" for
         * the vertex shader and "PixelMain" for the pixel shader.
         */
        ShdVertex_ = spRenderer->loadShader(
            ShdClass_,
            video::SHADER_VERTEX,
            video::GLSL_VERSION_1_20,
            ShaderPath + Name + ".glvert"
        );
        
        if (IsVertexPixel)
        {
            // Load the pixel shader. "glfrag" stands for "OpenGL Fragment Shader".
            ShdPixel_ = spRenderer->loadShader(
                ShdClass_,
                video::SHADER_PIXEL,
                video::GLSL_VERSION_1_20,
                ShaderPath + Name + ".glfrag"
            );
        }
        
        /**
         * Set the shader object callback which is called before the whole object will be rendered.
         * This parameter can also be zero. Then the callback is disabled.
         * You can also set a surface callback (setSurfaceCallback) which is called before each surface
         * is rendered. For more information look at the API documentation.
         */
        ShdClass_->setObjectCallback(CallbackProcOGL);
    }
    else if (spRenderer->getRendererType() == video::RENDERER_DIRECT3D9)
    {
        // Load the vertex shader. Differentiate between shader model 3.0 and 2.0.
        ShdVertex_ = spRenderer->loadShader(
            ShdClass_,
            video::SHADER_VERTEX,
            (IsShaderModel3 ? video::HLSL_VERTEX_3_0 : video::HLSL_VERTEX_2_0),
            ShaderPath + Name + ".hlsl",
            "VertexMain"
        );
        
        if (IsVertexPixel)
        {
            // Load the pixel shader.
            ShdPixel_ = spRenderer->loadShader(
                ShdClass_,
                video::SHADER_PIXEL,
                (IsShaderModel3 ? video::HLSL_PIXEL_3_0  : video::HLSL_PIXEL_2_0),
                ShaderPath + Name + ".hlsl",
                "PixelMain"
            );
        }
        
        // Set the shader object callback which is called before the whole object will be rendered.
        ShdClass_->setObjectCallback(CallbackProcD3D9);
    }
    
    /**
     * If everything worked correctly until yet link the shader program.
     * This process combines the shaders to one program. If this does not succeed the shader compilation failed
     * and we have to return with an error message.
     */
    if ( !ShdVertex_ || !ShdVertex_->valid() || ( IsVertexPixel && ( !ShdPixel_ || !ShdPixel_->valid() ) ) ||
         !ShdClass_->link() )
    {
        io::Log::error("Shader compilation failed");
        return false;
    }
    
    return true;
}

/**
 * This function moves an individual object. The user can rotate the object
 * by pressing the left mouse button and translate it forwards or backwards by
 * rolling the mouse wheel.
 */
void ShaderExample::Move(scene::SceneNode* Obj)
{
    static dim::point2di LastCursorPos;
    
    const dim::point2di CursorSpeed = spControl->getCursorSpeed();
    
    if (Obj)
    {
        if (spControl->mouseHit(io::MOUSE_LEFT))
            LastCursorPos = spControl->getCursorPosition();
        
        if (spControl->mouseDown(io::MOUSE_LEFT))
        {
            Rotation_.X -= (f32)CursorSpeed.Y * 0.25f;
            Rotation_.Y -= (f32)CursorSpeed.X * 0.25f;
            
            spControl->setCursorPosition(LastCursorPos);
        }
        
        // Make a X-Y matrix rotation. The standard SoftPixel Engine rotation is Y-X-Z.
        dim::matrix4f Mat;
        
        Mat.rotateX(Rotation_.X);
        Mat.rotateY(Rotation_.Y);
        
        // Set the rotation matrix.
        Obj->setRotationMatrix(Mat);
        
        // Translate forwards and backwards
        Obj->translate(
            dim::vector3df(0, 0, (f32)spControl->getMouseWheel() * 0.1f)
        );
    }
}

/**
 * With this function we can turn the camera. This is not a typicall rotation because
 * the orientation of the camera can be arbitrary.
 */
void ShaderExample::TurnCamera()
{
    static dim::point2di LastCursorPos;
    
    const dim::point2di CursorSpeed = spControl->getCursorSpeed();
    
    if (spControl->mouseHit(io::MOUSE_LEFT))
        LastCursorPos = spControl->getCursorPosition();
    
    if (spControl->mouseDown(io::MOUSE_LEFT))
    {
        dim::vector3df Vec;
        
        Vec.X = (f32)CursorSpeed.Y * 0.25f;
        Vec.Y = (f32)CursorSpeed.X * 0.25f;
        
        Cam->turn(Vec);
        
        spControl->setCursorPosition(LastCursorPos);
    }
}

/**
 * This rotates the camera. It's similar to the "TurnCamera" function but it rotates the
 * camera more like the standard SoftPixel Engine transformation.
 */
void ShaderExample::RotateCamera()
{
    static dim::point2di LastCursorPos;
    static f32 Pitch, Yaw;
    
    const dim::point2di CursorSpeed = spControl->getCursorSpeed();
    
    if (spControl->mouseHit(io::MOUSE_LEFT))
        LastCursorPos = spControl->getCursorPosition();
    
    if (spControl->mouseDown(io::MOUSE_LEFT))
    {
        Pitch   += (f32)CursorSpeed.Y * 0.25f;
        Yaw     += (f32)CursorSpeed.X * 0.25f;
        
        // Clamp the vertical orientation to -90 and 90 degrees.
        if (Pitch > 90)         Pitch = 90;
        else if (Pitch < -90)   Pitch = -90;
        
        spControl->setCursorPosition(LastCursorPos);
    }
    
    Cam->setRotation(dim::vector3df(Pitch, Yaw, 0));
}

/**
 * This turns the light arbitrary when the user is pressing the right mouse button.
 */
void ShaderExample::TurnLight()
{
    static dim::point2di LastCursorPos;
    
    const dim::point2di CursorSpeed = spControl->getCursorSpeed();
    
    if (spControl->mouseHit(io::MOUSE_RIGHT))
        LastCursorPos = spControl->getCursorPosition();
    
    if (spControl->mouseDown(io::MOUSE_RIGHT))
    {
        dim::vector3df Vec;
        
        Vec.X = -(f32)CursorSpeed.Y * 0.25f;
        Vec.Y = -(f32)CursorSpeed.X * 0.25f;
        
        Light->turn(Vec);
        Light->setDirection(Light->getRotationMatrix() * dim::vector3df(0, 0, 1));
        
        spControl->setCursorPosition(LastCursorPos);
    }
}


/* === SimpleVertexManipulation === */

SimpleVertexManipulation::SimpleVertexManipulation() :
    Mode_           (SVM_NORMAL ),
    TransformFactor_(0.0f       ),
    Object2_        (0          ),
    MaxHeight_      (0.0f       ),
    MinHeight_      (0.0f       )
{
}
SimpleVertexManipulation::~SimpleVertexManipulation()
{
    // Actually the engine assumes the work of deleting objects.
    // But a good program has to delete all allocated objects self.
    spScene->deleteNode(Object2_);
}

/**
 * This is our first shader callback. These callbacks needs to have two parameters.
 * First one specifies the shader table which holds each shader program which is linked to it
 * and the second one specifies the material. Particular for Direct3D9 shaders the transformation matrices
 * are set in the callbacks. For OpenGL there are several pre-defined constants like "gl_ModelViewProjectionMatrix"
 * which are very useful to disclaim to always pass basic matrices to the shaders.
 * But when setting a world-, view- or projection matrix we normally use the video driver to get the current
 * matrix states. The world matrix is actually the transformation of the current mesh object
 * (or any other material node object like billboard or terrain) in object space.
 * Object space means the transformation only of the object. There are also names like "tangent space" which
 * are explained at the "BumpMapping" shader example.
 * For Direct3D9 it is very important to pass the so called uniforms (or rather shader constants - because
 * inside the shader programs they are constant) to the right shader. You can not pass a constant to the vertex
 * shader if it is used in the pixel shader. For OpenGL it does not really matter but we still use the right ones.
 */
void VMCallbackOGL(video::ShaderClass* ShdTable, const scene::MaterialNode* Object)
{
    ShdTable->getVertexShader()->setConstant(
        "WorldMatrix", spRenderer->getWorldMatrix()
    );
}

/**
 * For Direct3D9 we additionally pass the world-view-projection transformation. But we need to pass it in
 * the reverse order. It has something to do with the internal processing.
 * Very important to know when using Direct3D9 shaders is that all constants need to be set
 * in each frame again! For OpenGL we can set several constants only one times but for Direct3D9
 * the information will get lost.
 */
void VMCallbackD3D9(video::ShaderClass* ShdTable, const scene::MaterialNode* Object)
{
    ShdTable->getVertexShader()->setConstant(
        "WorldMatrix", spRenderer->getWorldMatrix()
    );
    ShdTable->getVertexShader()->setConstant(
        "WorldViewProjectionMatrix", spRenderer->getProjectionMatrix() * spRenderer->getViewMatrix() * spRenderer->getWorldMatrix()
    );
}

/**
 * This virtual function needs to be implemented in each child class. Because this is called after
 * creating the shader example using the static template function "ShaderExample::Create".
 * If the shader could not load or its compilation failed we return the function and stop creating this example.
 * If the shader has been compiled successful we create our objects and set the first constants.
 * The "setConstant" function is massively overloaded. You can set floats, integers, 3d vectors,
 * 4d vectors, colors, matrix, float arrays and integer arrays. For more detail look in the online API docu.
 * When setting a color (video::color) it will be converted to a 4d vector where each component is in the range [0.0 .. 1.0].
 * In GLSL vec4 and in HLSL float4.
 * Then the sahder tables need to be set to our objects.
 */
bool SimpleVertexManipulation::Load()
{
    if (!LoadShader("SimpleVertexManipulation", VMCallbackOGL, VMCallbackD3D9))
        return false;
    
    // Create a cube with a 15x15 grid for each face side.
    Object_     = spScene->createMesh(scene::MESH_CUBE, 15);
    Object2_    = spScene->createMesh(scene::MESH_TEAPOT);
    
    // Store the bounding box data.
    const dim::aabbox3df BoundBox = Object_->getMeshBoundingBox();
    
    MinHeight_ = BoundBox.Min.Y;
    MaxHeight_ = BoundBox.Max.Y;
    
    // Set some constants.
    ShdVertex_->setConstant("BoundBoxMinHeight", MinHeight_);
    ShdVertex_->setConstant("BoundBoxMaxHeight", MaxHeight_);
    
    // Set the shader table for the first object.
    Object_->setShaderClass(ShdClass_);
    Object_->setPosition(dim::vector3df(0, 0, 2));
    
    // Set the shader table for the second object.
    Object2_->setShaderClass(ShdClass_);
    Object2_->setPosition(dim::vector3df(0, 0, 2));
    Object2_->setScale(0.7f);
    
    Update();
    Show(false);
    
    return true;
}

/**
 * This virutal function updates the interactive example. The user can make some stuff
 * like moving the object and switching between the demonstrations. For morphing we use
 * the current time that it runs always with the same speed anyhow on which PC.
 */
void SimpleVertexManipulation::Update()
{
    const f32 Morphing = math::Sin(static_cast<f32>(io::Timer::millisecs()) / 25);
    
    if (spControl->keyHit(io::KEY_RETURN))
    {
        if (++Mode_ > SVM_BLOAT)
            Mode_ = SVM_NORMAL;
    }
    
    ShdVertex_->setConstant("DemoMode", Mode_);
    
    Object_->setVisible(Mode_ != SVM_BLOAT);
    Object2_->setVisible(Mode_ == SVM_BLOAT);
    
    switch (Mode_)
    {
        case SVM_NORMAL:
            Description_        = "SimpleVertexManipulation: Normal transformation";
            break;
            
        case SVM_SPHERIFY:
            TransformFactor_    = Morphing * 1.0f;
            Description_        = "SimpleVertexManipulation: Spherify";
            break;
            
        case SVM_TWIST:
            TransformFactor_    = Morphing * 180.0f;
            Description_        = "SimpleVertexManipulation: Twist";
            break;
            
        case SVM_BLOAT:
            TransformFactor_    = Morphing * 0.1f + 0.1f;
            Description_        = "SimpleVertexManipulation: Bloat";
            break;
    }
    
    // Reset the constants.
    ShdVertex_->setConstant("TransformFactor", TransformFactor_);
    ShdVertex_->setConstant("LightDirection", Light->getDirection());
    ShdVertex_->setConstant("BoundBoxMinHeight", MinHeight_);
    ShdVertex_->setConstant("BoundBoxMaxHeight", MaxHeight_);
    
    Move(Object_);
    Move(Object2_);
    TurnLight();
}

void SimpleVertexManipulation::Show(bool Enable)
{
    Object_->setVisible(Enable && Mode_ != SVM_BLOAT);
    Object2_->setVisible(Enable && Mode_ == SVM_BLOAT);
}


/* === Billboarding === */

Billboarding::Billboarding()
{
    memset(ParticleTex_, 0, sizeof(video::Texture*)*3);
}
Billboarding::~Billboarding()
{
}

void BBCallbackD3D9(video::ShaderClass* ShdTable, const scene::MaterialNode* Object)
{
    ShdTable->getVertexShader()->setConstant(
        "WorldViewMatrix", spRenderer->getViewMatrix() * spRenderer->getWorldMatrix()
    );
    ShdTable->getVertexShader()->setConstant(
        "ProjectionMatrix", spRenderer->getProjectionMatrix()
    );
    ShdTable->getVertexShader()->setConstant(
        "DiffuseColor", Object->getMaterial()->getDiffuseColor()
    );
}

bool Billboarding::Load()
{
    if (!LoadShader("Billboarding", 0, BBCallbackD3D9))
        return false;
    
    for (s32 i = 0; i < 3; ++i)
        ParticleTex_[i] = spRenderer->loadTexture(BasePath + "media/Flare" + io::stringc(i + 1) + ".jpg");
    
    for (s32 i = 0; i < 100; ++i)
    {
        Object_ = spScene->createMesh(scene::MESH_PLANE);
        Object_->meshTurn(dim::vector3df(-90, 0, 0));
        
        Object_->addTexture(ParticleTex_[math::Randomizer::randInt(2)]);
        Object_->setPosition(math::Randomizer::randVector() * 10);
        
        Object_->setShaderClass(ShdClass_);
        
        // Set the blending mode to bright to have a mesh which looks like a standard billboard.
        Object_->getMaterial()->setBlendingMode(video::BLEND_BRIGHT);
        
        ObjectList_.push_back(Object_);
    }
    
    Object_ = 0;
    
    Update();
    Show(false);
    
    return true;
}

void Billboarding::Update()
{
    TurnCamera();
}

void Billboarding::Show(bool Enable)
{
    Cam->setRotation(0);
    
    for (std::list<scene::Mesh*>::iterator it = ObjectList_.begin(); it != ObjectList_.end(); ++it)
        (*it)->setVisible(Enable);
    
    spRenderer->setClearColor(Enable ? 0 : ClearColor);
    spRenderer->clearBuffers(video::BUFFER_COLOR);
}


/* === RustProcess === */

RustProcess::RustProcess() :
    MetalTex_           (0      ),
    RustTex_            (0      ),
    FactorTex_          (0      ),
    TransitionForwards_ (true   ),
    TransitionFactor_   (-2.5f  )
{
}
RustProcess::~RustProcess()
{
    spRenderer->deleteTexture(MetalTex_);
    spRenderer->deleteTexture(RustTex_);
    spRenderer->deleteTexture(FactorTex_);
}

void RPCallbackOGL(video::ShaderClass* ShdTable, const scene::MaterialNode* Object)
{
    ShdTable->getVertexShader()->setConstant(
        "WorldMatrix", spRenderer->getWorldMatrix()
    );
    ShdTable->getVertexShader()->setConstant(
        "LightDirection", Light->getDirection()
    );
}

void RPCallbackD3D9(video::ShaderClass* ShdTable, const scene::MaterialNode* Object)
{
    ShdTable->getVertexShader()->setConstant(
        "WorldMatrix", spRenderer->getWorldMatrix()
    );
    ShdTable->getVertexShader()->setConstant(
        "WorldViewProjectionMatrix", spRenderer->getProjectionMatrix() * spRenderer->getViewMatrix() * spRenderer->getWorldMatrix()
    );
    ShdTable->getVertexShader()->setConstant(
        "LightDirection", Light->getDirection()
    );
}

bool RustProcess::Load()
{
    if (!LoadShader("RustProcess", RPCallbackOGL, RPCallbackD3D9, true))
        return false;
    
    MetalTex_   = spRenderer->loadTexture(BasePath + "media/Metal.jpg");
    RustTex_    = spRenderer->loadTexture(BasePath + "media/Rust.jpg");
    FactorTex_  = spRenderer->loadTexture(BasePath + "media/RustFactor.jpg");
    
    Object_ = spScene->createMesh(scene::MESH_CUBE);
    Object_->setPosition(dim::vector3df(0, 0, 2));
    
    Object_->addTexture(MetalTex_);
    Object_->addTexture(RustTex_);
    Object_->addTexture(FactorTex_);
    
    if (spRenderer->getRendererType() == video::RENDERER_OPENGL)
    {
        // The texture layer order needs only to be set in GLSL.
        ShdPixel_->setConstant("ColorMapA", 0);
        ShdPixel_->setConstant("ColorMapB", 1);
        ShdPixel_->setConstant("FactorMap", 2);
    }
    
    Object_->setShaderClass(ShdClass_);
    
    Update();
    Show(false);
    
    return true;
}

void RustProcess::Update()
{
    const f32 TransitionSpeed = 0.01f;
    
    if (TransitionForwards_)
    {
        if ( ( TransitionFactor_ += TransitionSpeed ) > 5.0f)
            TransitionForwards_ = false;
    }
    else
    {
        if ( ( TransitionFactor_ -= TransitionSpeed ) < -2.5f )
            TransitionForwards_ = true;
    }
    
    ShdPixel_->setConstant("TransitionFactor", math::Max(0.0f, TransitionFactor_));
    
    Move(Object_);
    TurnLight();
}

void RustProcess::Show(bool Enable)
{
    Object_->setVisible(Enable);
}


/* === ProceduralTexture === */

ProceduralTextures::ProceduralTextures() :
    Mode_(PTEX_SIMPLE)
{
}
ProceduralTextures::~ProceduralTextures()
{
}

void PTCallbackOGL(video::ShaderClass* ShdTable, const scene::MaterialNode* Object)
{
    ShdTable->getVertexShader()->setConstant(
        "WorldMatrix", spRenderer->getWorldMatrix()
    );
    ShdTable->getPixelShader()->setConstant(
        "LightPosition", Light->getPosition(true)
    );
}

void PTCallbackD3D9(video::ShaderClass* ShdTable, const scene::MaterialNode* Object)
{
    ShdTable->getVertexShader()->setConstant(
        "WorldMatrix", spRenderer->getWorldMatrix()
    );
    ShdTable->getVertexShader()->setConstant(
        "WorldViewProjectionMatrix", spRenderer->getProjectionMatrix() * spRenderer->getViewMatrix() * spRenderer->getWorldMatrix()
    );
    ShdTable->getPixelShader()->setConstant(
        "LightPosition", Light->getPosition(true)
    );
}

bool ProceduralTextures::Load()
{
    if (!LoadShader("ProceduralTextures", PTCallbackOGL, PTCallbackD3D9, true, true))
        return false;
    
    Object_ = spScene->createMesh(scene::MESH_CUBE);
    Object_->setPosition(dim::vector3df(0, 0, 2));
    Object_->addTexture(spRenderer->createTexture(1));
    Object_->getMaterial()->setSpecularColor(128);
    
    Object_->setShaderClass(ShdClass_);
    
    Description_ = "ProceduralTextures: Simple material";
    
    Update();
    Show(false);
    
    return true;
}

void ProceduralTextures::Update()
{
    if (spControl->keyHit(io::KEY_RETURN))
    {
        if (++Mode_ > PTEX_CIRCLES)
            Mode_ = PTEX_SIMPLE;
        
        ShdPixel_->setConstant("DemoMode", Mode_);
        
        switch (Mode_)
        {
            case PTEX_SIMPLE:
                Description_ = "ProceduralTextures: Simple material"; break;
            case PTEX_CHESSBOARD:
                Description_ = "ProceduralTextures: Chessboard"; break;
            case PTEX_BRICKS:
                Description_ = "ProceduralTextures: Bricks"; break;
            case PTEX_CIRCLES:
                Description_ = "ProceduralTextures: Circles"; break;
        }
    }
    
    if (spRenderer->getRendererType() == video::RENDERER_DIRECT3D9)
    {
        video::MaterialStates* Material = Object_->getMaterial();
        
        // This constants will only be set for HLSL because in GLSL we have
        // several pre-defined constants with information about the standard material configration.
        ShdPixel_->setConstant("AmbientColor", Material->getAmbientColor());
        ShdPixel_->setConstant("DiffuseColor", Material->getDiffuseColor());
        ShdPixel_->setConstant("SpecularColor", Material->getSpecularColor());
        ShdPixel_->setConstant("Shininess", (1.0f - Material->getShininess()) * 128.0f);
    }
    
    Move(Object_);
}

void ProceduralTextures::Show(bool Enable)
{
    Object_->setVisible(Enable);
}


/* === FurEffect === */

s32 FurLayerIndex = 0;
s32 FurLayerCount = 0;

FurEffect::FurEffect() :
    SurfaceTex_ (0),
    HairTex_    (0)
{
}
FurEffect::~FurEffect()
{
}

void FECallbackOGL(video::ShaderClass* ShdTable, const scene::MaterialNode* Object)
{
    ++FurLayerIndex;
    
    ShdTable->getVertexShader()->setConstant(
        "WorldMatrix", spRenderer->getWorldMatrix()
    );
    ShdTable->getVertexShader()->setConstant(
        "LightDirection", Light->getDirection()
    );
    ShdTable->getVertexShader()->setConstant(
        "LayerInterpolation", static_cast<f32>(FurLayerIndex) / FurLayerCount
    );
}

void FECallbackD3D9(video::ShaderClass* ShdTable, const scene::MaterialNode* Object)
{
    ++FurLayerIndex;
    
    ShdTable->getVertexShader()->setConstant(
        "WorldMatrix", spRenderer->getWorldMatrix()
    );
    ShdTable->getVertexShader()->setConstant(
        "WorldViewProjectionMatrix", spRenderer->getProjectionMatrix() * spRenderer->getViewMatrix() * spRenderer->getWorldMatrix()
    );
    ShdTable->getVertexShader()->setConstant(
        "LightDirection", Light->getDirection()
    );
    ShdTable->getVertexShader()->setConstant(
        "LayerInterpolation", static_cast<f32>(FurLayerIndex) / FurLayerCount
    );
}

bool FurEffect::Load()
{
    if (!LoadShader("FurEffect", FECallbackOGL, FECallbackD3D9, spRenderer->getRendererType() == video::RENDERER_DIRECT3D9))
        return false;
    
    Object_ = spScene->createMesh(scene::MESH_TORUS, 15);
    Object_->setOrder(1);
    
    SurfaceTex_ = spRenderer->loadTexture(BasePath + "media/FurSurface.jpg");
    SurfaceTex_->setFormat(video::PIXELFORMAT_RGBA);
    
    HairTex_ = spRenderer->copyTexture(SurfaceTex_);
    MakeHairs(HairTex_);
    
    FurLayerCount = 20;
    
    scene::Mesh* TempObj = 0;
    
    for (s32 i = 0; i < FurLayerCount; ++i)
    {
        if (i)
        {
            TempObj = spScene->createMesh();
            
            // Set a mesh reference (or rather instance) to save video memory (VRAM).
            TempObj->setReference(*SubObjectList_.begin());
        }
        else
        {
            TempObj = spScene->createMesh();
            
            video::MeshBuffer* Surf = TempObj->createMeshBuffer();
            Surf->setReference(Object_->getMeshBuffer(0));
            Surf->textureTransform(0, dim::point2df(2, 1));
            
            TempObj->addTexture(HairTex_);
        }
        
        TempObj->setParent(Object_);
        TempObj->setShaderClass(ShdClass_);
        
        SubObjectList_.push_back(TempObj);
    }
    
    Object_->setPosition(dim::vector3df(0, 0, 1.5));
    Object_->getMeshBuffer(0)->textureTransform(0, dim::point2df(2, 1));
    Object_->addTexture(SurfaceTex_);
    
    Update();
    Show(false);
    
    return true;
}

void FurEffect::Update()
{
    FurLayerIndex = 0;
    
    const f32 Angle = static_cast<f32>(io::Timer::millisecs()) / 15;
    
    // This factor occurs that the hair is blowing in the wind.
    ShdVertex_->setConstant(
        "TexCoordTranslation", dim::vector3df(math::Sin(Angle), math::Cos(Angle), 0) * 0.01f
    );
    
    ShdVertex_->setConstant("MaxLayerHeight", 0.1f);
    
    Move(Object_);
    TurnLight();
}

void FurEffect::Show(bool Enable)
{
    Object_->setVisible(Enable);
    
    for (std::list<scene::Mesh*>::iterator it = SubObjectList_.begin(); it != SubObjectList_.end(); ++it)
        (*it)->setVisible(Enable);
}

/**
 * This function will pierc our hair texture. The idea of the fur effect is
 * to have several layers (in our case 20 pieces) with a texture where the hairs are stored
 * in the form of a field where some pixels have a transparency of 100% (there are the hairs)
 * and some other pixels have a transparency of 0% (there are no hairs).
 * So this function generates by random the hair surface (or rather texture).
 */
void FurEffect::MakeHairs(video::Texture* Tex)
{
    if (!Tex)
        return;
    
    // Make sure that the texture has an alpha channel.
    Tex->setFormat(video::PIXELFORMAT_RGBA);
    
    video::color Color;
    video::ImageBuffer* ImgBuffer = Tex->getImageBuffer();
    
    for (s32 y = 0, x; y < Tex->getSize().Height; ++y)
    {
        for (x = 0; x < Tex->getSize().Width; ++x)
        {
            // This random query occurs that 2 of 3 pixels are complete transparent
            // because the random range is [0 .. 2].
            if (math::Randomizer::randInt(2))
            {
                Color = ImgBuffer->getPixelColor(dim::point2di(x, y));
                Color.Alpha = 0;
                ImgBuffer->setPixelColor(dim::point2di(x, y), Color);
            }
        }
    }
    
    // Update the image buffer after manipulating the pixels.
    Tex->updateImageBuffer();
}


/* === CelShading === */

CelShading::CelShading() :
    Light1DTex_(0),
    WireObject_(0)
{
}
CelShading::~CelShading()
{
    if (WireObject_)
        spScene->deleteNode(WireObject_);
}

void CSCallbackOGL(video::ShaderClass* ShdTable, const scene::MaterialNode* Object)
{
    ShdTable->getVertexShader()->setConstant(
        "WorldMatrix", spRenderer->getWorldMatrix()
    );
    ShdTable->getVertexShader()->setConstant(
        "LightDirection", Light->getDirection()
    );
}

void CSCallbackD3D9(video::ShaderClass* ShdTable, const scene::MaterialNode* Object)
{
    ShdTable->getVertexShader()->setConstant(
        "WorldMatrix", spRenderer->getWorldMatrix()
    );
    ShdTable->getVertexShader()->setConstant(
        "WorldViewProjectionMatrix", spRenderer->getProjectionMatrix() * spRenderer->getViewMatrix() * spRenderer->getWorldMatrix()
    );
    ShdTable->getVertexShader()->setConstant(
        "LightDirection", Light->getDirection()
    );
}

bool CelShading::Load()
{
    if (!LoadShader("CelShading", CSCallbackOGL, CSCallbackD3D9))
        return false;
    
    Object_ = spScene->createMesh(scene::MESH_TEAPOT);
    
    // An additional feature for cel shading is only supported for OpenGL because Direct3D9
    // does not support drawing lines with an individual width. This additional mesh object
    // will represent the outlines which shall look as if it were drawn with pencil.
    if (spRenderer->getRendererType() == video::RENDERER_OPENGL)
    {
        WireObject_ = spScene->copyNode(Object_);
        WireObject_->setParent(Object_);
        WireObject_->getMaterial()->setWireframe(video::WIREFRAME_LINES);
        WireObject_->getMaterial()->setRenderFace(video::FACE_BACK);
        WireObject_->getMaterial()->setLighting(false);
        WireObject_->paint(0);
    }
    
    Object_->setPosition(dim::vector3df(0, 0, 2));
    Object_->setScale(0.7f);
    
    Object_->setShaderClass(ShdClass_);
    
    const s32 LayerCount = 6;
    const video::color LightColor = video::color(255, 255, 50);
    
    // Create a 1 dimensional texture where the shading transition is to be stored.
    // We also set the wrap-mode to "clamp" that the texture coordinates can never leave
    // the range [0.0 .. 1.0]. If you don't do that particular with HLSL it looks a little bit strange.
    Light1DTex_ = spRenderer->createTexture(dim::size2di(LayerCount, 1));
    Light1DTex_->setDimension(video::TEXTURE_1D);
    Light1DTex_->setWrapMode(video::TEXWRAP_CLAMP);
    
    video::ImageBuffer* ImgBuffer = Light1DTex_->getImageBuffer();
    
    for (s32 i = 0; i < LayerCount; ++i)
    {
        ImgBuffer->setPixelColor(
            dim::point2di(i, 0),
            video::color(
                (u8)((s32)((i + 1) * LightColor.Red   ) / LayerCount),
                (u8)((s32)((i + 1) * LightColor.Green ) / LayerCount),
                (u8)((s32)((i + 1) * LightColor.Blue  ) / LayerCount)
            )
        );
    }
    
    Light1DTex_->updateImageBuffer();
    
    // Set the filter to linear that the transition is hard as if it were drawn with bund pins.
    Light1DTex_->setFilter(video::FILTER_LINEAR);
    
    Object_->addTexture(Light1DTex_);
    
    Update();
    Show(false);
    
    return true;
}

void CelShading::Update()
{
    // Set the wire mesh object's wireframe mode always to lines.
    if (WireObject_)
        WireObject_->getMaterial()->setWireframe(video::WIREFRAME_LINES);
    
    Move(Object_);
    TurnLight();
}

void CelShading::Show(bool Enable)
{
    if (WireObject_)
    {
        // For the wire mesh object the lines need to be drawn with a larger size (or rather width).
        spRenderer->setLineSize(Enable ? 5 : 1);
        WireObject_->setVisible(Enable);
    }
    
    Object_->setVisible(Enable);
}


/* === DepthOfField === */

DepthOfField::DepthOfField() :
    DepthBias_  (0.0f   ),
    Object2_    (0      ),
    Object3_    (0      ),
    ScreenTex_  (0      ),
    SurfaceTex_ (0      )
{
}
DepthOfField::~DepthOfField()
{
    spRenderer->deleteTexture(SurfaceTex_);
    spRenderer->deleteTexture(ScreenTex_);
    
    spScene->deleteNode(Object2_);
    spScene->deleteNode(Object3_);
}

void DFCallbackD3D9(video::ShaderClass* ShdTable, const scene::MaterialNode* Object)
{
    ShdTable->getVertexShader()->setConstant(
        "WorldViewProjectionMatrix", spRenderer->getProjectionMatrix() * spRenderer->getViewMatrix() * spRenderer->getWorldMatrix()
    );
}

bool DepthOfField::Load()
{
    if (!LoadShader("DepthOfField", 0, DFCallbackD3D9, true, true))
        return false;
    
    // Create a render target texture. For those texture which are NPOT (none-power-of-two) textures
    // - i.e. the size is not 2^n - MIP mapping has to be disabled. Otherwise updating the render target
    // can be very slow.
    ScreenTex_  = spRenderer->createTexture(dim::size2di(ScrWidth, ScrHeight));
    ScreenTex_->setMipMapping(false);
    ScreenTex_->setRenderTarget(true);
    
    SurfaceTex_ = spRenderer->loadTexture(BasePath + "media/Metal.jpg");
    
    Object_ = spScene->createMesh(scene::MESH_CUBE);
    Object_->setPosition(dim::vector3df(-0.7f, 0, 2));
    Object_->setRotation(dim::vector3df(0, 8, 0));
    Object_->addTexture(SurfaceTex_);
    
    Object2_ = spScene->createMesh(scene::MESH_CUBE);
    Object2_->setPosition(dim::vector3df(2, 0, 4));
    Object2_->setRotation(dim::vector3df(0, -15, 0));
    Object2_->addTexture(SurfaceTex_);
    
    Object3_ = spScene->createMesh(scene::MESH_CUBE);
    Object3_->setPosition(dim::vector3df(0, 0, 7));
    Object3_->setRotation(dim::vector3df(0, 25, 0));
    Object3_->addTexture(SurfaceTex_);
    
    Update();
    Show(false);
    
    return true;
}

/**
 * For this example we render the scene in an other way. This way is called "Deferred Rendering".
 * At first we render the whole scene in the render target texture without using shaders.
 * Then we render the scene again with the use of shaders and only the render target texture
 * which is used in the shader.
 */
bool DepthOfField::Render()
{
    spRenderer->setRenderTarget(ScreenTex_);
    spRenderer->clearBuffers();
    
    spScene->renderScene();
    
    spRenderer->setRenderTarget(0);
    spRenderer->clearBuffers();
    
    Object_->setShaderClass(ShdClass_);
    Object2_->setShaderClass(ShdClass_);
    Object3_->setShaderClass(ShdClass_);
    
    spRenderer->setRenderState(video::RENDER_TEXTURE, false);
    ScreenTex_->bind(0);
    
    spScene->renderScene();
    
    ScreenTex_->unbind(0);
    spRenderer->setRenderState(video::RENDER_TEXTURE, true);
    
    Object_->setShaderClass(0);
    Object2_->setShaderClass(0);
    Object3_->setShaderClass(0);
    
    return true;
}

void DepthOfField::Update()
{
    DepthBias_ += (f32)spControl->getMouseWheel() / 2;
    math::clamp(DepthBias_, -10.0f, 20.0f);
    
    ShdPixel_->setConstant("DepthBias", DepthBias_);
    ShdPixel_->setConstant("ScreenWidth", ScrWidth);
    ShdPixel_->setConstant("ScreenHeight", ScrHeight);
    ShdPixel_->setConstant("DepthRange", 7.0f);
    
    Description_ = "Depth of Field (DepthBias: " + io::stringc(DepthBias_) + ")";
    
    TurnLight();
}

void DepthOfField::Show(bool Enable)
{
    Object_->setVisible(Enable);
    Object2_->setVisible(Enable);
    Object3_->setVisible(Enable);
}


/* === BumpMapping ===*/

BumpMapping::BumpMapping() :
    Mode_       (BM_ROCKS   ),
    Object2_    (0          ),
    LightObj_   (0          ),
    EnableBumps_(true       )
{
    memset(ColorMap_, 0, sizeof(video::Texture*)*2);
    memset(NormalMap_, 0, sizeof(video::Texture*)*2);
}
BumpMapping::~BumpMapping()
{
    spScene->deleteNode(Object2_);
}

void BMCallbackOGL(video::ShaderClass* ShdTable, const scene::MaterialNode* Object)
{
    ShdTable->getVertexShader()->setConstant(
        "WorldMatrix", spRenderer->getWorldMatrix()
    );
}

void BMCallbackD3D9(video::ShaderClass* ShdTable, const scene::MaterialNode* Object)
{
    ShdTable->getVertexShader()->setConstant(
        "WorldMatrix", spRenderer->getWorldMatrix()
    );
    ShdTable->getVertexShader()->setConstant(
        "WorldViewProjectionMatrix", spRenderer->getProjectionMatrix() * spRenderer->getViewMatrix() * spRenderer->getWorldMatrix()
    );
}

bool BumpMapping::Load()
{
    if (!LoadShader("BumpMapping", BMCallbackOGL, BMCallbackD3D9, true, true))
        return false;
    
    ColorMap_[0]    = spRenderer->loadTexture(BasePath + "media/RocksColorMap.jpg");
    NormalMap_[0]   = spRenderer->loadTexture(BasePath + "media/RocksNormalMap.jpg");
    
    ColorMap_[1]    = spRenderer->loadTexture(BasePath + "media/StoneColorMap.jpg");
    NormalMap_[1]   = spRenderer->loadTexture(BasePath + "media/StoneNormalMap.jpg");
    
    FlareTex_       = spRenderer->loadTexture(BasePath + "media/Flare3.jpg");
    
    Object_ = spScene->createMesh(scene::MESH_PLANE);
    
    /**
     * Change the vertex format to 'extended' because we want to store the tangent space
     * (tangent-, binormal- and normal vectors) in the texture coordinates. Thus we need
     * Texture coordinates with 3 components. But the 'default' vertex format only use
     * 2 component texture coordinates.
     */
    Object_->getMeshBuffer(0)->setVertexFormat(spRenderer->getVertexFormatExtended());
    
    Object_->setPosition(dim::vector3df(0, -2, 0));
    Object_->setScale(50);
    Object_->getMeshBuffer(0)->textureTransform(0, 25.0f);
    
    /**
     * This updates the tangent space or rather the tangent- and binormal vectors for each vertex of
     * our mesh object. Because only when using the Direct3D11 video driver tangent- and binormal vectors
     * are actually supported we store the vectors in the second and third texture coordinates.
     * 1 is the second one for the tangent (0 would be the first one) and 2 is the third one for the binormal.
     * Tangent space is used for normal mapping effects light "BumpMapping", "ParallaxMapping",
     * "ParallaxOcclusionMapping" etc. The tangent and binormal vectors are important for the correct
     * normal orientation in the pixel shader. Normally each normal mapping effect is pixel based
     * i.e. the normal vector needs to be computed for each pixel. With the tangent, binormal and normal
     * a new rotation matrix can be created to transform the final normal vector stored in the normal map.
     */
    Object_->updateTangentSpace(1, 2);
    
    Object_->addTexture(ColorMap_[Mode_]);
    Object_->addTexture(NormalMap_[Mode_]);
    
    Object_->setShaderClass(ShdClass_);
    
    Object2_ = spScene->createMesh(scene::MESH_CUBE);
    Object2_->getMeshBuffer(0)->setVertexFormat(spRenderer->getVertexFormatExtended());
    
    Object2_->setPosition(dim::vector3df(0, 0, 2));
    Object2_->updateTangentSpace(1, 2);
    
    Object2_->addTexture(ColorMap_[Mode_]);
    Object2_->addTexture(NormalMap_[Mode_]);
    
    Object2_->setShaderClass(ShdClass_);
    
    LightColor_ = video::color(255);
    
    LightObj_ = spScene->createBillboard(FlareTex_);
    LightObj_->getMaterial()->setLighting(false);
    LightObj_->setColor(LightColor_);
    LightObj_->setScale(0.5);
    
    if (spRenderer->getRendererType() == video::RENDERER_OPENGL)
    {
        // The texture layer order needs only to be set in GLSL.
        ShdPixel_->setConstant("ColorMap", 0);
        ShdPixel_->setConstant("NormalMap", 1);
    }
    
    Description_ = "BumpMapping: Rocks";
    EnableBumps_ = true;
    
    Update();
    Show(false);
    
    return true;
}

void BumpMapping::Update()
{
    RotateCamera();
    
    const f32 Angle = static_cast<f32>(io::Timer::millisecs()) / 25;
    
    dim::matrix4f Mat;
    Mat.rotateY(Angle);
    const dim::vector3df LightPos = Mat * dim::vector3df(0, 0, 3);
    
    LightObj_->setPosition(LightPos);
    
    Object2_->turn(1);
    
    if (spControl->keyHit(io::KEY_RETURN))
    {
        if (++Mode_ > BM_STONE_NOBUMPS)
            Mode_ = BM_ROCKS;
        
        s32 TexIndex = 0;
        
        switch (Mode_)
        {
            case BM_ROCKS:
                Description_    = "BumpMapping: Rocks";
                TexIndex        = 0;
                EnableBumps_    = true;
                break;
                
            case BM_ROCKS_NOBUMPS:
                Description_    = "BumpMapping: Rocks (no bumps)";
                TexIndex        = 0;
                EnableBumps_    = false;
                break;
                
            case BM_STONE:
                Description_    = "BumpMapping: Stone";
                TexIndex        = 1;
                EnableBumps_    = true;
                break;
                
            case BM_STONE_NOBUMPS:
                Description_    = "BumpMapping: Stone (no bumps)";
                TexIndex        = 1;
                EnableBumps_    = false;
                break;
        }
        
        Object_->getMeshBuffer(0)->setTexture(0, ColorMap_[TexIndex]);
        Object_->getMeshBuffer(0)->setTexture(1, NormalMap_[TexIndex]);
        
        Object2_->getMeshBuffer(0)->setTexture(0, ColorMap_[TexIndex]);
        Object2_->getMeshBuffer(0)->setTexture(1, NormalMap_[TexIndex]);
    }
    
    ShdPixel_->setConstant("EnableBumps", EnableBumps_);
    ShdPixel_->setConstant("Light1_Position", LightPos);
    ShdPixel_->setConstant("Light1_Diffuse", LightColor_);
    ShdPixel_->setConstant("Light1_Specular", video::color(70));
    ShdPixel_->setConstant("Light1_Radius", 50.0f);
    ShdPixel_->setConstant("Light1_Shininess", 90.0f);
}

void BumpMapping::Show(bool Enable)
{
    if (Enable)
        RotateCamera();
    else
        Cam->setRotation(0);
    
    Object_->setVisible(Enable);
    Object2_->setVisible(Enable);
    LightObj_->setVisible(Enable);
}


/* === WaterSimulation === */

WaterSimulation::WaterSimulation() :
    HeightField_    (0),
    WaterPlane_     (0),
    GrassTex_       (0),
    HeightMapTex_   (0),
    WaterColorMap_  (0),
    WaterNormalMap_ (0),
    RefractionMap_  (0),
    ReflectionMap_  (0)
{
}
WaterSimulation::~WaterSimulation()
{
    spRenderer->deleteTexture(GrassTex_);
    spRenderer->deleteTexture(HeightMapTex_);
    spRenderer->deleteTexture(WaterColorMap_);
    spRenderer->deleteTexture(WaterNormalMap_);
    spRenderer->deleteTexture(RefractionMap_);
    spRenderer->deleteTexture(ReflectionMap_);
    
    spScene->deleteNode(HeightField_);
    spScene->deleteNode(WaterPlane_);
}

void WSCallbackD3D9(video::ShaderClass* ShdTable, const scene::MaterialNode* Object)
{
    ShdTable->getVertexShader()->setConstant(
        "WorldViewProjectionMatrix", spRenderer->getProjectionMatrix() * spRenderer->getViewMatrix() * spRenderer->getWorldMatrix()
    );
}

bool WaterSimulation::Load()
{
    if (!LoadShader("WaterSimulation", 0, WSCallbackD3D9, true, true))
        return false;
    
    GrassTex_       = spRenderer->loadTexture(BasePath + "media/Grass.jpg");
    HeightMapTex_   = spRenderer->loadTexture(BasePath + "media/HeightMap.jpg");
    WaterColorMap_  = spRenderer->loadTexture(BasePath + "media/WaterColorMap.jpg");
    WaterNormalMap_ = spRenderer->loadTexture(BasePath + "media/WaterNormalMap.jpg");
    
    // Create two render target textures: the refraction- and reflection map.
    // This water simulation will also be achieved with deferred rendering.
    RefractionMap_  = spRenderer->createTexture(dim::size2di(ScrWidth, ScrHeight));
    ReflectionMap_  = spRenderer->createTexture(dim::size2di(ScrWidth, ScrHeight));
    
    RefractionMap_->setMipMapping(false);
    ReflectionMap_->setMipMapping(false);
    
    RefractionMap_->setFilter(video::FILTER_LINEAR);
    ReflectionMap_->setFilter(video::FILTER_LINEAR);
    
    RefractionMap_->setWrapMode(video::TEXWRAP_CLAMP);
    ReflectionMap_->setWrapMode(video::TEXWRAP_CLAMP);
    
    RefractionMap_->setRenderTarget(true);
    ReflectionMap_->setRenderTarget(true);
    
    // Create a height field for the environment.
    HeightField_ = spScene->createHeightField(HeightMapTex_, 100);
    HeightField_->setPosition(dim::vector3df(0, -3, 0));
    HeightField_->setScale(dim::vector3df(50, 4, 50));
    
    HeightField_->addTexture(GrassTex_);
    HeightField_->getMeshBuffer(0)->textureTransform(0, 30.0f);
    
    // Create a water plane which gets the shader.
    WaterPlane_ = spScene->createMesh(scene::MESH_PLANE);
    WaterPlane_->setPosition(dim::vector3df(0, -1.5, 0));
    WaterPlane_->setScale(50);
    WaterPlane_->getMaterial()->setColorMaterial(false);
    WaterPlane_->getMaterial()->setDiffuseColor(video::color(255, 255, 255, 64));
    
    WaterPlane_->addTexture(RefractionMap_);
    WaterPlane_->addTexture(ReflectionMap_);
    WaterPlane_->addTexture(WaterColorMap_);
    WaterPlane_->addTexture(WaterNormalMap_);
    WaterPlane_->addTexture(HeightMapTex_);
    
    WaterPlane_->getMeshBuffer(0)->textureTransform(0, 25.0f);
    WaterPlane_->getMeshBuffer(0)->textureTransform(1, 15.0f);
    
    WaterPlane_->setShaderClass(ShdClass_);
    
    if (spRenderer->getRendererType() == video::RENDERER_OPENGL)
    {
        // The texture layer order needs only to be set in GLSL.
        ShdPixel_->setConstant("RefractionMap", 0);
        ShdPixel_->setConstant("ReflectionMap", 1);
        ShdPixel_->setConstant("WaterSurfaceMap", 2);
        ShdPixel_->setConstant("NormalMap", 3);
        ShdPixel_->setConstant("HeightMap", 4);
    }
    
    Update();
    Show(false);
    
    return true;
}

/**
 * This function renders the whole scene three times. The first time the scene is rendered
 * into the refraction map without any mirror effect. The second time the scene is rendered
 * into the reflection map where the whole view is fliped on Y axis that a realistic reflection
 * is simulated. And the third time the scene is finally rendered onto the screen with the use of
 * our water simulation shader where the two render target texture are needed.
 * For the refraction and reflection a clipping plane is used that reflected obects can not
 * leave the reflection area.
 */
bool WaterSimulation::Render()
{
    const f32 WaterPlaneHeight = WaterPlane_->getPosition().Y;
    
    WaterPlane_->setVisible(false);
    
    // Render the refraction map.
    spRenderer->setClipPlane(0, dim::plane3df(dim::vector3df(0, -1.0f, 0), WaterPlaneHeight), true);
    
    spRenderer->setRenderTarget(RefractionMap_);
    spRenderer->clearBuffers();
    
    spScene->renderScene();
    
    spRenderer->setRenderTarget(0);
    
    // Render the reflection map.
    dim::matrix4f MirrorMat;
    MirrorMat.translate(dim::vector3df(0, WaterPlaneHeight*2, 0));
    MirrorMat.scale(dim::vector3df(1, -1, 1));
    Cam->setMirrorMatrix(MirrorMat);
    
    Cam->setMirror(true);
    spRenderer->setFrontFace(false);
    
    spRenderer->setClipPlane(0, dim::plane3df(dim::vector3df(0, 1, 0), -WaterPlaneHeight), true);
    
    spRenderer->setRenderTarget(ReflectionMap_);
    spRenderer->clearBuffers();
    spScene->renderScene();
    spRenderer->setRenderTarget(0);
    
    Cam->setMirror(false);
    spRenderer->setFrontFace(true);
    
    spRenderer->setClipPlane(0, dim::plane3df(), false);
    WaterPlane_->setVisible(true);
    
    // Render the final scene.
    spScene->renderScene();
    
    return true;
}

void WaterSimulation::Update()
{
    WaterPlane_->getMeshBuffer(0)->textureTranslate(0, dim::point2df(0, 0.005f));
    WaterPlane_->getMeshBuffer(0)->textureTranslate(1, dim::point2df(0, 0.005f));
    
    if (spControl->keyDown(io::KEY_UP))
        WaterPlane_->translate(dim::vector3df(0, 0.01f, 0));
    if (spControl->keyDown(io::KEY_DOWN))
        WaterPlane_->translate(dim::vector3df(0, -0.01f, 0));
    
    ShdPixel_->setConstant("ScreenWidth", ScrWidth);
    ShdPixel_->setConstant("ScreenHeight", ScrHeight);
    ShdPixel_->setConstant("IsUnderWater", false);
    ShdPixel_->setConstant("WaterPlaneHeight", WaterPlane_->getPosition().Y);
    ShdPixel_->setConstant("ObjectHeight", HeightField_->getScale().Y);
    
    TurnLight();
    
    RotateCamera();
}

void WaterSimulation::Show(bool Enable)
{
    if (Enable)
        RotateCamera();
    else
        Cam->setRotation(0);
    
    HeightField_->setVisible(Enable);
    WaterPlane_->setVisible(Enable);
}





// ============================================
