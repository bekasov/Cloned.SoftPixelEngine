//
// SoftPixel Engine - PolygonClipping Tests
//

#include <SoftPixelEngine.hpp>

using namespace sp;

#include "../common.hpp"

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

int main()
{
    SP_TESTS_INIT_EX2(
        video::RENDERER_OPENGL, dim::size2di(1024, 768), "PolygonClipping", false, SDeviceFlags()
    )
    
    // Create polygons
    dim::polygon3df Poly, PolyA, PolyB;
    
    Poly.push(dim::vector3df(150, 300, 0));
    Poly.push(dim::vector3df(550, 150, 0));
    Poly.push(dim::vector3df(350, 500, 0));
    
    // Main loop
    while (spDevice->updateEvents() && !spControl->keyDown(io::KEY_ESCAPE))
    {
        spRenderer->clearBuffers();
        
        /*if (spContext->isWindowActive())
            tool::Toolset::moveCameraFree();
        
        spScene->renderScene();*/
        
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
        
        spContext->flipBuffers();
    }
    
    deleteDevice();
    
    return 0;
}
