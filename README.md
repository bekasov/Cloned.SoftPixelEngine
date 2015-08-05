#SoftPixelEngine
Cloned from http://sourceforge.net/projects/softpixelengine/, http://softpixelengine.sourceforge.net/

#Description
The SoftPixel Engine is an OpenSource high level realtime 3D engine for C++ (GNU C++ & VisualC++).
It supports Direct3D11, Direct3D9, OpenGL 1.1 - 4.1, OpenGL|ES 1.1 and OpenGL|ES 2.0 and runs on Windows, Linux, Android and iOS.

#Features
	Renderer independent: Direct3D11, Direct3D9, OpenGL 1.1 - 4.1, OpenGL|ES 1.1, OpenGL|ES 2.0
	Platform independent: Windows, Linux, Android, iOS
	Integrated sub-systems: SoundSystem, NetworkSystem
	Wrappers for C#, VisualBasic, BlitzBasic, PureBasic, FreePascal
	Very easy and good for beginners in graphics programming
	Project files for: Code::Blocks (GCC), VisualStudio 2008, VisualStudio 2010 and VisualStudio 2012
	Shader Model 1.0 - 5.0, Cg, GLSL, HLSL, GLSL|ES, Tessellation, Compute Shader (for GPGPU)
	Integrated PhysicsEngine (NewtonGameDynamics)
	DeferredRenderer
	
	
	Getting started with a very small program (C++ & SoftPixel Engine v.3.2):
```c++
// SoftPixel Engine - Getting started
#include <SoftPixelEngine.hpp>
using namespace sp; // Main namespace
int main()
{
// The softpixel-device opens the screen (windowed screen / fullscreen are possible)
SoftPixelDevice* spDevice = createGraphicsDevice(
    video::RENDERER_OPENGL, dim::size2di(640, 480), 32, "Getting Started"
);
video::RenderSystem* spRenderer = spDevice->getRenderSystem(); // Render system for drawing operations
video::RenderContext* spContext = spDevice->getRenderContext(); // Render context for back buffer controll
io::InputControl* spControl = spDevice->getInputControl(); // Controller for keyboard- and the mouse input events
scene::SceneGraph* spScene = spDevice->createSceneGraph(); // Scene graph system to handle a lots of scene objects
spScene->setLighting(true); // Activates global lighting
scene::Camera* Cam = spScene->createCamera(); // Creates a camera to see our scene
scene::Light* Light = spScene->createLight(); // Creates a light (by default Directional light)
scene::Mesh* Obj = spScene->createMesh(scene::MESH_TEAPOT); // Creates one of the 16 standard objects
Obj->setPosition(dim::vector3df(0, 0, 2.5)); // Sets the object position (x, y, z)
// The main loop will update our device
while (spDevice->updateEvents() && !spControl->keyDown(io::KEY_ESCAPE))
{
    spRenderer->clearBuffers(); // Clears the video buffers (pixel- and depth buffer)     
    spScene->renderScene(); // Renders the scene     
    Obj->turn(dim::vector3df(1, 1, 1)); // Rotates our object     
    spContext->flipBuffers(); // Swaps the video buffer, so we can see the current frame
}

deleteDevice(); // Deletes the device context and closes the screen
return 0;
}
// END-OF-LINE 
```
