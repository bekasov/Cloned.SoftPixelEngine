//
// SoftPixel Engine - SoftwareRasterizer Tests
//

#include <SoftPixelEngine.hpp>

using namespace sp;

#include "../common.hpp"

#include <boost/foreach.hpp>

SP_TESTS_DECLARE

/*
 * Global members
 */

video::Texture* OutputImage = 0;

video::color* FrameBuffer = 0;
f32* DepthBuffer = 0;

dim::size2di FrameBufferSize;

scene::Mesh* Obj = 0;

dim::matrix4f ViewProjectionMatrix;
dim::matrix4f WorldViewProjectionMatrix;
dim::matrix3f NormalMatrix;

dim::point2df ViewportOrigin;
dim::size2df ViewportSize;

f32 NearClippingPlane   = 0.0f;
f32 FarClippingPlane    = 1.0f;

f32 NearRange           = 0.1f;
f32 FarRange            = 1000.0f;


/*
 * Classes
 */

class Vertex
{
    
    public:
        
        Vertex()
        {
        }
        ~Vertex()
        {
        }
        
        /* === Operators === */
        
        #define VTX_OP(o)                                   \
            inline Vertex& operator o (const Vertex &Other) \
            {                                               \
                Coord       o Other.Coord;                  \
                Normal      o Other.Normal;                 \
                Color       o Other.Color;                  \
                TexCoord    o Other.TexCoord;               \
                InvZ        o Other.InvZ;                   \
                return *this;                               \
            }
        
        #define VTX_OP_FACTOR(o)                    \
            inline Vertex& operator o (f32 Factor)  \
            {                                       \
                Coord       o Factor;               \
                Normal      o Factor;               \
                Color       o Factor;               \
                TexCoord    o Factor;               \
                InvZ        o Factor;               \
                return *this;                       \
            }
        
        VTX_OP(=)
        
        VTX_OP(+=)
        VTX_OP(-=)
        
        VTX_OP_FACTOR(*=)
        VTX_OP_FACTOR(/=)
        
        #undef VTX_OP
        #undef VTX_OP_FACTOR
        
        /* === Functions === */
        
        inline s32 getScreenCoordX() const
        {
            return ScreenCoord.X;
        }
        inline s32 getScreenCoordY() const
        {
            return ScreenCoord.Y;
        }
        
        /* === Members === */
        
        dim::vector4df Coord;
        dim::vector3df Normal;
        dim::vector3df Color;
        dim::point2df TexCoord;
        f32 InvZ;
        
        dim::point2di ScreenCoord;
        
};



/*
 * Functions
 */

void CreateScene()
{
    #if 0
    
    // Load textures
    video::Texture* Tex = spRenderer->loadTexture(ROOT_PATH + "Media/SoftPixelEngine Logo Small.png");
    
    // Create models
    Obj = spScene->createMesh(scene::MESH_CUBE);
    Obj->addTexture(Tex);
    
    scene::Mesh* Obj2 = spScene->createMesh(scene::MESH_CUBE);
    Obj2->setPosition(dim::vector3df(-0.5f, 0, 0));
    
    #elif 0
    
    spScene->loadScene(
        "D:/SoftwareEntwicklung/C++/HLC/Tools/SoftPixelSandbox/media/Scenes/Trees.spsb"
    );
    
    #else
    
    Obj = spScene->loadMesh(ROOT_PATH + "AnimationTests/dwarf2.b3d");
    Obj->setScale(0.1f);
    Obj->setRotation(dim::vector3df(0, 180, 0));
    Obj->setPosition(dim::vector3df(0, -2.5f, 5));
    
    scene::Animation* Anim = Obj->getFirstAnimation();
    
    if (Anim)
    {
        Anim->play(scene::PLAYBACK_LOOP);
        Anim->setSpeed(0.1f);
    }
    
    #endif
}

void InitRasterizer()
{
    OutputImage = spRenderer->createTexture(spContext->getResolution(), video::PIXELFORMAT_RGBA);
    
    video::ImageBuffer* ImgBuffer = OutputImage->getImageBuffer();
    
    FrameBuffer = reinterpret_cast<video::color*>(ImgBuffer->getBuffer());
    
    FrameBufferSize = ImgBuffer->getSize();
    
    DepthBuffer = new f32[FrameBufferSize.getArea()];
}

void CleanRasterizer()
{
    delete DepthBuffer;
}

void ClearColorBuffer()
{
    memset(FrameBuffer, 0, sizeof(video::color) * FrameBufferSize.getArea());
}

void ClearDepthBuffer()
{
    for (u32 i = 0, c = FrameBufferSize.getArea(); i < c; ++i)
        DepthBuffer[i] = 1.0f;
}

void TransformVertexViewport(dim::vector4df &Coord, f32 &InvZ)
{
    // Setup inverse z component (clamp to near and far clipping planes)
    InvZ = 1.0f / (Coord.Z + NearRange);
    
    // Compute normalized device coordinates
    Coord.X /= Coord.W;
    Coord.Y /= Coord.W;
    Coord.Z /= Coord.W;
    
    // Transform vertex to viewport (screen coordinates)
    Coord.X = Coord.X*ViewportSize.Width*0.5f + (ViewportOrigin.X + ViewportSize.Width*0.5f);
    Coord.Y = -Coord.Y*ViewportSize.Height*0.5f + (ViewportOrigin.Y + ViewportSize.Height*0.5f);
    Coord.Z = Coord.Z*(FarClippingPlane - NearClippingPlane)*0.5f + (FarClippingPlane + NearClippingPlane)*0.5f;
}

bool TransformVertex(video::MeshBuffer* Surf, u32 Index, Vertex &Vert)
{
    // Transform vertex
    Vert.Coord = WorldViewProjectionMatrix * dim::vector4df(Surf->getVertexCoord(Index), 1.0f);
    
    if (Vert.Coord.Z < NearRange)
        return false;
    
    Vert.Normal = NormalMatrix * Surf->getVertexNormal(Index);
    Vert.Normal.normalize();
    
    Vert.TexCoord = Surf->getVertexTexCoord(Index);
    
    // Compute lighting
    Vert.Color = math::Max(0.1f, -Vert.Normal.dot(dim::vector3df(0.0f, 0.0f, 1.0f)));
    
    // Setup viewport transformation
    TransformVertexViewport(Vert.Coord, Vert.InvZ);
    
    Vert.ScreenCoord.X = static_cast<s32>(Vert.Coord.X);
    Vert.ScreenCoord.Y = static_cast<s32>(Vert.Coord.Y);
    
    if ( Vert.ScreenCoord.X < 0 || Vert.ScreenCoord.X >= FrameBufferSize.Width ||
         Vert.ScreenCoord.Y < 0 || Vert.ScreenCoord.Y >= FrameBufferSize.Height )
    {
        return false;
    }
    
    // Setup perspective texture coordinates
    Vert.TexCoord.X *= Vert.InvZ;
    Vert.TexCoord.Y *= Vert.InvZ;
    
    return true;
}

void RasterizerCallback(s32 x, s32 y, const Vertex &Vert, void* UserData)
{
    if (x < 0 || x >= FrameBufferSize.Width || y < 0 || y >= FrameBufferSize.Height)
        return;
    
    // Get framebuffer index
    const u32 i = FrameBufferSize.Width * y + x;
    
    // Depth test
    if (Vert.Coord.Z > DepthBuffer[i])
        return;
    
    // Compute pixel color
    dim::vector3df Color = Vert.Color;
    
    // Apply texture sampling
    if (UserData)
    {
        // Convert perspective texture coordinates
        dim::point2df TexCoord(Vert.TexCoord);
        
        TexCoord.X /= Vert.InvZ;
        TexCoord.Y /= Vert.InvZ;
        
        // Sample texture
        video::ImageBuffer* Sampler = reinterpret_cast<video::ImageBuffer*>(UserData);
        
        Color *= Sampler->getPixelColor(Sampler->getPixelCoord(TexCoord)).getVector(true);
        //Color = Sampler->getInterpolatedPixel(TexCoord);
    }
    
    // Render into framebuffer and depth buffer
    FrameBuffer[i] = Color;
    DepthBuffer[i] = Vert.Coord.Z;
}

void RasterizeTriangle(video::MeshBuffer* Surf, const u32 (&Indices)[3])
{
    // Vertex processing
    Vertex Vertices[3];
    
    for (u32 i = 0; i < 3; ++i)
    {
        if (!TransformVertex(Surf, Indices[i], Vertices[i]))
            return;
    }
    
    // Compute culling
    const dim::triangle3df Face(Vertices[0].Coord, Vertices[1].Coord, Vertices[2].Coord);
    
    if (!Face.isFrontSide())
        return;
    
    // Rasterize triangle
    math::Rasterizer::rasterizeTriangle<Vertex>(
        RasterizerCallback, Vertices[0], Vertices[1], Vertices[2],
        Surf->getTexture() ? Surf->getTexture()->getImageBuffer() : 0
    );
}

void RasterizeMeshBuffer(video::MeshBuffer* Surf)
{
    u32 Indices[3];
    
    // Rasterize all triangles
    for (u32 i = 0, c = Surf->getTriangleCount(); i < c; ++i)
    {
        Surf->getTriangleIndices(i, Indices);
        
        RasterizeTriangle(Surf, Indices);
    }
}

void RasterizeMesh(scene::Mesh* Node)
{
    // Setup world-view-projection matrix
    WorldViewProjectionMatrix = ViewProjectionMatrix;
    WorldViewProjectionMatrix *= Node->getTransformMatrix(true);
    
    // Setup normal matrix
    NormalMatrix = Node->getTransformMatrix(true).get3x3();
    NormalMatrix.normalize();
    
    // Rasterize all mesh buffers
    foreach (video::MeshBuffer* Surf, Node->getMeshBufferList())
        RasterizeMeshBuffer(Surf);
}

void RasterizeScene()
{
    // Setup view-projection matrix
    ViewProjectionMatrix = Cam->getProjection().getMatrixLH();
    ViewProjectionMatrix *= Cam->getTransformation(true).getInverseMatrix();
    
    // Setup viewport
    ViewportSize = FrameBufferSize.cast<f32>();
    
    // Setup view range
    NearRange = Cam->getRangeNear();
    FarRange = Cam->getRangeFar();
    
    // Rasterize all models
    foreach (scene::RenderNode* Node, spScene->getRenderList())
    {
        if (Node->getVisible() && Node->getType() == scene::NODE_MESH)
            RasterizeMesh(static_cast<scene::Mesh*>(Node));
    }
}


/*
 * Main functions
 */

int main()
{
    SP_TESTS_INIT_EX(
        video::RENDERER_OPENGL, dim::size2di(800, 600), "SoftwareRasterizer", false
    )
    
    Cam->setPosition(dim::vector3df(0, 0, -3));
    
    InitRasterizer();
    
    CreateScene();
    
    while (spDevice->updateEvents() && !spControl->keyDown(io::KEY_ESCAPE))
    {
        spRenderer->clearBuffers();
        
        // Update scene
        if (Obj)
            tool::Toolset::presentModel(Obj);
        else if (spContext->isWindowActive())
            tool::Toolset::moveCameraFree(Cam, 0.25f * io::Timer::getGlobalSpeed());
        
        spSceneMngr->updateAnimations();
        
        // Draw scene
        #if 1
        
        ClearColorBuffer();
        ClearDepthBuffer();
        
        RasterizeScene();
        
        OutputImage->updateImageBuffer();
        
        spRenderer->draw2DImage(OutputImage, 0);
        
        #else
        
        spScene->renderScene();
        
        #endif
        
        DrawFPS();
        
        spContext->flipBuffers();
    }
    
    CleanRasterizer();
    
    deleteDevice();
    
    return 0;
}
