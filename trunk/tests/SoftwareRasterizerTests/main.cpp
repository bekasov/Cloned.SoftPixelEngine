//
// SoftPixel Engine - SoftwareRasterizer Tests
//

#include <SoftPixelEngine.hpp>

using namespace sp;

#include "../common.hpp"

#include <boost/foreach.hpp>

SP_TESTS_DECLARE

//#define USE_CUSTOM_RASTERIZER
#define PERSPECTIVE_CORRECTION

/*
 * Global members
 */

video::Texture* OutputImage = 0;
video::ImageBuffer* ActiveImageBuffer = 0;

video::color* FrameBuffer = 0;
f32* DepthBuffer = 0;

dim::size2di FrameBufferSize;

scene::Mesh* Obj = 0;
scene::Animation* Anim = 0;

dim::matrix4f ViewProjectionMatrix;
dim::matrix4f WorldViewProjectionMatrix;
dim::matrix4f WorldMatrix;
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
        Vertex(const Vertex &Other) :
            Coord       (Other.Coord        ),
            Normal      (Other.Normal       ),
            Color       (Other.Color        ),
            TexCoord    (Other.TexCoord     ),
            InvZ        (Other.InvZ         ),
            ScreenCoord (Other.ScreenCoord  )
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
                ScreenCoord = Other.ScreenCoord;            \
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

#ifdef USE_CUSTOM_RASTERIZER

class Edge
{
    
    public:
        
        Edge(const Edge &Other) :
            v1(Other.v1),
            v2(Other.v2)
        {
        }
        Edge(const Vertex &a, const Vertex &b)
        {
            if (a.ScreenCoord.Y < b.ScreenCoord.Y)
            {
                v1 = a;
                v2 = b;
            }
            else
            {
                v1 = b;
                v2 = a;
            }
        }
        ~Edge()
        {
        }
        
        /* Members */
        
        Vertex v1, v2;
        
};

class Span
{
    
    public:
        
        Span(const Vertex &a, const Vertex &b, s32 X1, s32 X2)
        {
            if (X1 < X2)
            {
                v1 = a;
                v2 = b;
                x1 = X1;
                x2 = X2;
            }
            else
            {
                v1 = b;
                v2 = a;
                x1 = X2;
                x2 = X1;
            }
        }
        ~Span()
        {
        }
        
        /* Members */
        
        Vertex v1, v2;
        
        s32 x1, x2;
        
};

#endif


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
    
    //scene::Mesh* Obj2 = spScene->createMesh(scene::MESH_CUBE);
    //Obj2->setPosition(dim::vector3df(-0.5f, 0, 0));
    
    #elif 0
    
    spScene->loadScene(
        "D:/SoftwareEntwicklung/C++/HLC/Tools/SoftPixelSandbox/media/Scenes/Trees.spsb"
    );
    
    #else
    
    Obj = spScene->loadMesh(ROOT_PATH + "AnimationTests/dwarf2.b3d");
    Obj->setScale(0.1f);
    Obj->setRotation(dim::vector3df(0, 180, 0));
    Obj->setPosition(dim::vector3df(0, -2.5f, 5));
    
    Anim = Obj->getFirstAnimation();
    
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
    Coord.X =  Coord.X * ViewportSize.Width  * 0.5f + (ViewportOrigin.X + ViewportSize.Width  * 0.5f);
    Coord.Y = -Coord.Y * ViewportSize.Height * 0.5f + (ViewportOrigin.Y + ViewportSize.Height * 0.5f);
    Coord.Z =  Coord.Z * (FarClippingPlane - NearClippingPlane)*0.5f + (FarClippingPlane + NearClippingPlane)*0.5f;
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
    #ifdef PERSPECTIVE_CORRECTION
    Vert.TexCoord.X *= Vert.InvZ;
    Vert.TexCoord.Y *= Vert.InvZ;
    #endif
    
    return true;
}

void SampleNearestTexImage2D(const video::ImageBuffer* Sampler, video::color &Color, dim::point2df &TexCoord)
{
    // Get buffer and size
    const video::color* TexBuffer = reinterpret_cast<const video::color*>(Sampler->getBuffer());
    
    s32 w = Sampler->getSize().Width;
    s32 h = Sampler->getSize().Height;
    
    // Sample texture image
    TexCoord.X *= w;
    TexCoord.Y *= h;
    
    s32 u = static_cast<s32>(TexCoord.X) % w;
    s32 v = static_cast<s32>(TexCoord.Y) % h;
    
    if (u < 0)
        u += w;
    if (v < 0)
        v += h;
    
    u32 i = v * w + u;
    
    Color = TexBuffer[i];
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
    if (ActiveImageBuffer)
    {
        // Convert perspective texture coordinates
        dim::point2df TexCoord(Vert.TexCoord);
        
        #ifdef PERSPECTIVE_CORRECTION
        TexCoord.X /= Vert.InvZ;
        TexCoord.Y /= Vert.InvZ;
        #endif
        
        Color *= ActiveImageBuffer->getPixelColor(ActiveImageBuffer->getPixelCoord(TexCoord)).getVector(true);
        //SampleNearestTexImage2D(ActiveImageBuffer, Color, TexCoord);
        //Color *= ActiveImageBuffer->getInterpolatedPixel(TexCoord);
    }
    
    // Render into framebuffer and depth buffer
    FrameBuffer[i] = Color;
    DepthBuffer[i] = Vert.Coord.Z;
}

#ifdef USE_CUSTOM_RASTERIZER

void DrawSpan(const Span &span, s32 y)
{
    // Compute difference between the X coordinates
    s32 Xdif = span.x2 - span.x1;
    
    if (Xdif == 0)
        return;
    
    // Interpolate final vertex
    Vertex v(span.v2);
    v -= span.v1;
    
    f32 f = 0.0f;
    f32 step = 1.0f / static_cast<f32>(Xdif);
    
    // Draw each pixel in the span
    for (s32 x = span.x1; x < span.x2; ++x)
    {
        // Interpolate final vertex.
        Vertex p(v);
        p *= f;
        p += span.v1;
        
        // Draw final pixel
        RasterizerCallback(x, y, p, 0);
        
        // Increase factor
        f += step;
    }
}

void DrawSpansBetweenEdges(const Edge &e1, const Edge &e2)
{
    // Break if difference between the Y coordinates is 0
    if ( e1.v2.ScreenCoord.Y - e1.v1.ScreenCoord.Y == 0 ||
         e2.v2.ScreenCoord.Y - e2.v1.ScreenCoord.Y == 0 )
    {
        return;
    }
    
    // Compute difference between the Y coordinates of the two edges
    f32 e1Ydif = static_cast<f32>(e1.v2.ScreenCoord.Y - e1.v1.ScreenCoord.Y);
    f32 e2Ydif = static_cast<f32>(e2.v2.ScreenCoord.Y - e2.v1.ScreenCoord.Y);
    
    // Compute difference between the X coordinates and vertices of the two edges
    f32 e1Xdif = static_cast<f32>(e1.v2.ScreenCoord.X - e1.v1.ScreenCoord.X);
    f32 e2Xdif = static_cast<f32>(e2.v2.ScreenCoord.X - e2.v1.ScreenCoord.X);
    
    Vertex e1Vdif = e1.v2;
    e1Vdif -= e1.v1;
    
    Vertex e2Vdif = e2.v2;
    e2Vdif -= e2.v1;
    
    // Compute factors and steps for interpolation
    f32 f1 = static_cast<f32>(e2.v1.ScreenCoord.Y - e1.v1.ScreenCoord.Y) / e1Ydif;
    f32 step1 = 1.0f / e1Ydif;
    f32 f2 = 0.0f;
    f32 step2 = 1.0f / e2Ydif;
    
    // Draw all lines between the edges
    for (s32 y = e2.v1.ScreenCoord.Y; y < e2.v2.ScreenCoord.Y; ++y)
    {
        // Interpolate vertices
        Vertex a(e1Vdif), b(e2Vdif);
        
        a *= f1;
        b *= f2;
        
        a += e1.v1;
        b += e2.v1;
        
        // Create and draw span
        Span span(
            a, b,
            e1.v1.ScreenCoord.X + static_cast<s32>(e1Xdif * f1),
            e2.v1.ScreenCoord.X + static_cast<s32>(e2Xdif * f2)
        );
        
        DrawSpan(span, y);
        
        // Increase factors
        f1 += step1;
        f2 += step2;
    }
}

void DrawTriangle(const Vertex (&Vertices)[3])
{
    // Create edges for triangle
    Edge Edges[3] =
    {
        Edge(Vertices[0], Vertices[1]),
        Edge(Vertices[1], Vertices[2]),
        Edge(Vertices[2], Vertices[0])
    };
    
    // Find edge with largest length along Y axis
    s32 MaxLen = 0;
    s32 LongEdge = 0;
    
    for (s32 i = 0; i < 3; ++i)
    {
        s32 Len = Edges[i].v2.ScreenCoord.Y - Edges[i].v1.ScreenCoord.Y;
        if (Len > MaxLen)
        {
            MaxLen = Len;
            LongEdge = i;
        }
    }
    
    s32 ShortEdge1 = (LongEdge + 1) % 3;
    s32 ShortEdge2 = (LongEdge + 2) % 3;
    
    // Draw spans between edges
    DrawSpansBetweenEdges(Edges[LongEdge], Edges[ShortEdge1]);
    DrawSpansBetweenEdges(Edges[LongEdge], Edges[ShortEdge2]);
}

#endif

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
    #ifndef USE_CUSTOM_RASTERIZER
    math::Rasterizer::rasterizeTriangle<Vertex>(
        RasterizerCallback, Vertices[0], Vertices[1], Vertices[2]
    );
    #else
    DrawTriangle(Vertices);
    #endif
}

void RasterizeMeshBuffer(video::MeshBuffer* Surf)
{
    u32 Indices[3];
    
    // Bind active image buffer
    ActiveImageBuffer = (Surf->getTexture() ? Surf->getTexture()->getImageBuffer() : 0);
    
    // Rasterize all triangles
    for (u32 i = 0, c = Surf->getTriangleCount(); i < c; ++i)
    {
        Surf->getTriangleIndices(i, Indices);
        
        RasterizeTriangle(Surf, Indices);
    }
}

void RasterizeMesh(scene::Mesh* Node)
{
    // Get world matrix
    WorldMatrix = Node->getTransformMatrix(true);
    
    // Setup world-view-projection matrix
    WorldViewProjectionMatrix = ViewProjectionMatrix;
    WorldViewProjectionMatrix *= WorldMatrix;
    
    // Setup normal matrix
    NormalMatrix = WorldMatrix.get3x3();
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
        
        if (Anim && spControl->keyHit(io::KEY_SPACE))
            Anim->pause(Anim->playing());
        
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
