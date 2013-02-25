//
// SoftPixel Engine - PolygonClipping Tests
//

#include <SoftPixelEngine.hpp>

using namespace sp;

#include "../common.hpp"

#include <boost/foreach.hpp>

SP_TESTS_DECLARE

scene::SPrimitiveVertex2D Vertices[10];

void DrawPolygon(const dim::polygon3df &Poly, const video::color &Color)
{
    for (u32 i = 0; i < Poly.getCount(); ++i)
    {
        Vertices[i].setPosition(
            dim::point2di(static_cast<s32>(Poly[i].X), static_cast<s32>(Poly[i].Y))
        );
        Vertices[i].setColor(Color);
    }
    
    spRenderer->draw2DPolygon(video::PRIMITIVE_TRIANGLE_FAN, Vertices, Poly.getCount());
}

const s32 BoxColorCount = 10;
video::color BoxColors[BoxColorCount];
s32 BoxColorIndex = 0;
dim::matrix4f TreeNodeTrans;

void DrawKDTreeNode(scene::KDTreeNode* Node, s32 Level)
{
    #ifdef _DEB_NEW_KDTREE_
    
    // Check if max level has reached
    if (Level <= 0 || !Node)
        return;
    
    // Choose color
    if (++BoxColorIndex >= BoxColorCount)
        BoxColorIndex = 0;
    
    //if (Level == 1)
    {
        // Draw bounding box for tree node
        spRenderer->draw3DBox(Node->getBox(), TreeNodeTrans, BoxColors[BoxColorIndex]); 
    }
    
    if (Node->isLeaf())
    {
        if (Level == 1 || spControl->keyDown(io::KEY_SPACE))
        {
            // Draw triangles
            typedef scene::CollisionMesh::TreeNodeDataType NodeDataT;
            
            NodeDataT* NodeData = reinterpret_cast<NodeDataT*>(Node->getUserData());
            
            if (!NodeData)
                return;
            
            s32 ColIndex = 0;
            
            foreach (scene::SCollisionFace &Face, *NodeData)
            {
                if (++ColIndex >= BoxColorCount)
                    ColIndex = 0;
                spRenderer->draw3DTriangle(0, Face.Triangle, BoxColors[ColIndex]);
            }
        }
    }
    else
    {
        // Draw children
        DrawKDTreeNode(static_cast<scene::KDTreeNode*>(Node->getChildNear()), Level - 1);
        DrawKDTreeNode(static_cast<scene::KDTreeNode*>(Node->getChildFar()), Level - 1);
    }
    
    #endif
}

int main()
{
    SP_TESTS_INIT_EX2(
        video::RENDERER_OPENGL, dim::size2di(1024, 768), "PolygonClipping", false, SDeviceFlags()
    )
    
    //#define SHOW_CLIPPING
    #ifdef SHOW_CLIPPING
    
    // Create polygons
    dim::polygon3df Poly, PolyA, PolyB;
    
    Poly.push(dim::vector3df(150, 300, 0));
    Poly.push(dim::vector3df(550, 150, 0));
    Poly.push(dim::vector3df(350, 500, 0));
    
    #else
    
    const io::stringc ResPath("../DeferredRendererTests/");
    
    scene::Mesh* World = spScene->loadMesh(ResPath + "TestScene.spm");
    
    s32 MaxLevel = 8;
    
    scene::CollisionGraph CollSys;
    scene::CollisionMesh* CollMesh = CollSys.createMesh(0, World, static_cast<u8>(MaxLevel));
    
    scene::KDTreeNode* TreeRootNode = CollMesh->getRootTreeNode();
    
    #endif
    
    s32 DrawTreeLevel = 0;
    
    // Initialize random box colors
    for (s32 i = 0; i < BoxColorCount; ++i)
        BoxColors[i] = math::Randomizer::randColor();
    
    TreeNodeTrans = World->getTransformMatrix(true);
    
    // Main loop
    while (spDevice->updateEvents() && !spControl->keyDown(io::KEY_ESCAPE))
    {
        spRenderer->clearBuffers();
        
        #ifndef SHOW_CLIPPING
        
        // Update camera movement
        if (spContext->isWindowActive())
            tool::Toolset::moveCameraFree();
        
        // Render scene
        spScene->renderScene();
        
        // Draw kd-tree nodes
        spRenderer->beginDrawing3D();
        {
            //spRenderer->setRenderState(video::RENDER_DEPTH, false);
            
            BoxColorIndex = 0;
            DrawKDTreeNode(TreeRootNode, DrawTreeLevel);
            
            //spRenderer->setRenderState(video::RENDER_DEPTH, true);
        }
        spRenderer->endDrawing3D();
        
        if (spControl->keyHit(io::KEY_TAB))
        {
            static bool Wire;
            Wire = !Wire;
            spScene->setWireframe(Wire ? video::WIREFRAME_LINES : video::WIREFRAME_SOLID);
        }
        
        // Update and show tree-node draw-level
        DrawTreeLevel += spControl->getMouseWheel();
        math::Clamp(DrawTreeLevel, 0, MaxLevel + 1);
        
        Draw2DText(15, "DrawTreeLevel = " + io::stringc(DrawTreeLevel));
        
        #else
        
        f32 Dist = static_cast<f32>(spControl->getCursorPosition().X);
        
        const dim::plane3df Plane(
            dim::vector3df(1, -0.2f, 0).normalize(), Dist
        );
        
        PolyA.clear();
        PolyB.clear();
        
        math::CollisionLibrary::clipPolygon(Poly, Plane, PolyA, PolyB);
        
        spRenderer->beginDrawing2D();
        {
            DrawPolygon(PolyA, video::color(255, 0, 0));
            DrawPolygon(PolyB, video::color(0, 255, 0));
        }
        spRenderer->endDrawing2D();
        
        Draw2DText(dim::point2di(15, 15), "Dist = " + io::stringc(Dist));
        
        Draw2DText(
            dim::point2di(15, 40),
            "Poly RED Num = " + io::stringc(PolyA.getCount()) + ", Poly GREEN Num = " + io::stringc(PolyB.getCount())
        );
        
        #endif
        
        spContext->flipBuffers();
    }
    
    deleteDevice();
    
    return 0;
}
