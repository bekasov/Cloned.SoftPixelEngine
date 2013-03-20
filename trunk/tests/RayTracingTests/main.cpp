//
// SoftPixel Engine - RayTracing Tests
//

#include <SoftPixelEngine.hpp>

using namespace sp;

#include "../common.hpp"

#include <Framework/OpenCL/spOpenCLDevice.hpp>

SP_TESTS_DECLARE

int main()
{
    const dim::size2di ScrSize(800, 600);
    
    SP_TESTS_INIT_EX2(
        video::RENDERER_OPENGL, ScrSize, "RayTracing", false, SDeviceFlags(false, false)
    )
    
    /* Create OpenCL device */
    video::OpenCLDevice* CLDev = 0;
    
    try
    {
        CLDev = new video::OpenCLDevice();
    }
    catch (const std::string &ErrorStr)
    {
        io::Log::error(ErrorStr);
        io::Log::pauseConsole();
        return 0;
    }
    
    /* Load OpenCL program */
    u64 Time = io::Timer::millisecs();
    
    video::OpenCLProgram* CLShader = CLDev->loadProgram("RayTracingShader.cl");
    
    io::Log::message("Compilation time: " + io::stringc(io::Timer::millisecs() - Time) + " ms.");
    
    const io::stringc KernelName = "RenderRayTracing";
    
    CLShader->addKernel(KernelName);
    
    /* Create OpenCL buffer objects */
    video::STextureCreationFlags CreationFlags;
    {
        CreationFlags.Size      = ScrSize;
        CreationFlags.Format    = video::PIXELFORMAT_RGBA;
        CreationFlags.MipMaps   = false;
        CreationFlags.MinFilter = video::FILTER_LINEAR;
        CreationFlags.MagFilter = video::FILTER_LINEAR;
    }
    video::Texture* ResultImage = spRenderer->createTexture(CreationFlags);
    
    video::OpenCLBuffer* CLBufImage = CLDev->createBuffer(video::OCLBUFFER_WRITE, ResultImage);
    
    /* Setup Kernel parameters */
    const size_t NumExecCores[2] = { 4, 4 };
    const dim::size2di BlockSize(ScrSize.Width / NumExecCores[0], ScrSize.Height / NumExecCores[1]);
    
    /* Upload kernel parameters */
    CLShader->setParameter(KernelName, 0, BlockSize.Width);
    CLShader->setParameter(KernelName, 1, BlockSize.Height);
    
    // Output image buffer
    CLShader->setParameter(KernelName, 2, ScrSize.Width);
    CLShader->setParameter(KernelName, 3, ScrSize.Height);
    CLShader->setParameter(KernelName, 4, CLBufImage);
    
    // View transformation
    CLShader->setParameter(KernelName, 5, dim::matrix4f::IDENTITY);
    
    // Tree node hierarchy
    CLShader->setParameter(KernelName, 6, 0);
    CLShader->setParameter(KernelName, 7, 0);
    
    // Index buffer
    CLShader->setParameter(KernelName, 8, 0);
    CLShader->setParameter(KernelName, 9, 0);
    
    // Vertex buffer
    CLShader->setParameter(KernelName, 10, 0);
    CLShader->setParameter(KernelName, 11, 0);
    
    
    
    /* Main loop */
    while (spDevice->updateEvents() && !spControl->keyDown(io::KEY_ESCAPE))
    {
        spRenderer->clearBuffers();
        
        /* Execute OpenCL shader */
        if (CLShader->valid())
        {
            CLBufImage->lock();
            CLShader->run(KernelName, 2, NumExecCores, NumExecCores);
            CLBufImage->unlock();
        }
        
        /* Draw output image */
        spRenderer->draw2DImage(ResultImage, 0);
        
        DrawFPS();
        
        spContext->flipBuffers();
    }
    
    /* Clean up */
    delete CLDev;
    
    deleteDevice();
    
    return 0;
}
