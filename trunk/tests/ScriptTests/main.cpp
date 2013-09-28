//
// SoftPixel Engine - Script Tests
//

#include <SoftPixelEngine.hpp>
#include <boost/foreach.hpp>

#include "Framework/Tools/ScriptParser/spUtilityShaderPreProcessor.hpp"

using namespace sp;

#if defined(SP_COMPILE_WITH_SHADER_PREPROCESSOR) && defined(SP_COMPILE_WITH_MATERIAL_SCRIPT)

#include "../common.hpp"

SP_TESTS_DECLARE

int main()
{
    #if 0
    
    tool::TokenParser Parser;
    
    #   if 1
    
    // Read input shader code
    //const io::stringc Filename = "../../sources/Framework/Tools/LightmapGenerator/spLightmapGenerationShader.hlsl";
    //const io::stringc Filename = "../../sources/RenderSystem/AdvancedRenderer/spLightGridShader.hlsl";
    const io::stringc Filename = "../../sources/RenderSystem/AdvancedRenderer/spDeferredShader.hlsl";
    
    io::FileSystem FileSys;
    std::list<io::stringc> ShdBuffer;
    
    video::ShaderClass::loadShaderResourceFile(FileSys, Filename, ShdBuffer);
    
    // Preprocess shader code
    tool::ShaderPreProcessor ShaderPP;
    
    io::stringc InputCode, OutputCode;
    
    foreach (const io::stringc &Str, ShdBuffer)
        InputCode += Str;
    
    ShaderPP.preProcessShader(
        InputCode,
        OutputCode,
        video::SHADER_COMPUTE,
        video::GLSL_VERSION_4_30,
        "PixelMain",//"ComputeMain",//"ComputeDirectIllumination",
        ( tool::SHADER_PREPROCESS_NO_TABS |
          //tool::SHADER_PREPROCESS_SKIP_BLANKS |
          tool::SHADER_PREPROCESS_SOLVE_MACROS )
    );
    
    #   else
    
    // Parse file
    const io::stringc Filename = "../../sources/Framework/Tools/LightmapGenerator/spLightmapGenerationShader.hlsl";
    //const io::stringc Filename = "TestScript.txt";
    
    tool::TokenIteratorPtr TokenIt = Parser.parseFile(Filename, tool::COMMENTSTYLE_ANSI_C);
    
    if (!TokenIt)
    {
        io::Log::pauseConsole();
        return 0;
    }
    
    bool Quit = false;
    
    io::stringc VarName, VarValue;
    
    // Validate brackets
    const tool::SToken* InvalidToken = 0;
    const tool::ETokenValidationErrors Err = TokenIt->validateBrackets(InvalidToken);
    
    if (InvalidToken)
    {
        switch (Err)
        {
            case tool::VALIDATION_ERROR_UNCLOSED:
                io::Log::error("Unclosed bracket token at " + InvalidToken->getRowColumnString());
                break;
            default:
                io::Log::error("Invalid bracket token at " + InvalidToken->getRowColumnString());
                break;
        }
    }
    
    // Print information about parsed file
    while (!Quit)
    {
        tool::SToken Tkn = TokenIt->getNextToken();
        
        if (Tkn.eof())
            Quit = true;
        
        switch (Tkn.Type)
        {
            case tool::TOKEN_NAME:
                io::Log::message("Name: " + io::stringc(Tkn.Str));
                break;
            case tool::TOKEN_STRING:
                io::Log::message("String: " + io::stringc(Tkn.Str));
                break;
            case tool::TOKEN_NUMBER_INT:
                io::Log::message("Int: " + io::stringc(Tkn.Str));
                break;
            case tool::TOKEN_NUMBER_FLOAT:
                io::Log::message("Float: " + io::stringc(Tkn.Str));
                break;
            default:
                io::Log::message(Tkn.Chr);
                break;
        }
    }
    
    #   endif
    
    io::Log::pauseConsole();
    
    #else
    
    SP_TESTS_INIT_EX(
        video::RENDERER_OPENGL,
        //video::RENDERER_DIRECT3D11,
        dim::size2di(640, 480),
        "ScriptTest",
        false
    )
    
    scene::Mesh* Obj = 0;
    
    Cam->setPosition(dim::vector3df(0, 0, -3));
    
    tool::MaterialScriptReader* ScriptReader = new tool::MaterialScriptReader();
    
    if (ScriptReader->loadScript("TestMaterial.material"))
    {
        video::VertexFormatUniversal* VertFmt = ScriptReader->findVertexFormat("MyVertexFormat");
        if (VertFmt)
            scene::SceneManager::setDefaultVertexFormat(VertFmt);
        
        Obj = spScene->createMesh(scene::MESH_CUBE);
        
        video::MaterialStatesPtr Mat = ScriptReader->findMaterial("TestMat0");
        if (Mat)
            Obj->setMaterial(Mat.get());
        
        #if 0
        video::ShaderClass* Shd = ScriptReader->findShader("SimpleColorShader");
        if (Shd)
            Obj->setShaderClass(Shd);
        #endif
        
        video::Texture* Tex = ScriptReader->findTexture("MyTex0");
        if (Tex)
            Obj->addTexture(Tex);
    }
    
    while (spDevice->updateEvents() && !spControl->keyDown(io::KEY_ESCAPE))
    {
        spRenderer->clearBuffers();
        
        tool::Toolset::presentModel(Obj);
        
        spScene->renderScene();
        
        spContext->flipBuffers();
    }
    
    delete ScriptReader;
    
    deleteDevice();
    
    #endif
    
    return 0;
}

#else

int main()
{
    io::Log::error("This engine was not compiled with 'shader pre-processor' and 'material script reader' utility");
    return 0;
}

#endif
