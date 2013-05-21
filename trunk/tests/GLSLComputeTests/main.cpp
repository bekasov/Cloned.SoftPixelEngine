//
// SoftPixel Engine - GLSL Compute Tests
//

#include <SoftPixelEngine.hpp>

using namespace sp;

#include "../common.hpp"

SP_TESTS_DECLARE

int main()
{
    SP_TESTS_INIT("GLSL Compute")
    


    SP_TESTS_MAIN_BEGIN
    {
        spScene->renderScene(Cam);
    }
    SP_TESTS_MAIN_END
}
