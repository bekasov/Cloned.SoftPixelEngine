/*
 * Direct3D9 shader file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "RenderSystem/Direct3D9/spDirect3D9Shader.hpp"

#if defined(SP_COMPILE_WITH_DIRECT3D9)


#include "RenderSystem/Direct3D9/spDirect3D9RenderSystem.hpp"
#include "Platform/spSoftPixelDeviceOS.hpp"


namespace sp
{

extern video::RenderSystem* __spVideoDriver;

namespace video
{


/*
 * Internal members
 */

const c8* const d3dVertexShaderVersions[] =
{
    "vs_1_0",
    "vs_2_0",
    "vs_2_a",
    "vs_3_0",
    "vs_4_0",
    "vs_4_1",
    "vs_5_0",
    0
};

const c8* const d3dPixelShaderVersions[] =
{
    "ps_1_0",
    "ps_1_1",
    "ps_1_2",
    "ps_1_3",
    "ps_1_4",
    "ps_2_0",
    "ps_2_a",
    "ps_2_b",
    "ps_3_0",
    "ps_4_0",
    "ps_4_1",
    "ps_5_0",
    0
};


/*
 * Direct3D9Shader class
 */

Direct3D9Shader::Direct3D9Shader(ShaderTable* Table, const EShaderTypes Type, const EShaderVersions Version)
    : Shader(Table, Type, Version), pD3D9Device_(0), VertexShaderObject_(0), PixelShaderObject_(0), ConstantTable_(0)
{
    pD3D9Device_ = static_cast<video::Direct3D9RenderSystem*>(__spVideoDriver)->getDirect3DDevice();
    
    if (!Table_)
        Table_ = new Direct3D9ShaderTable();
    
    updateTable();
}
Direct3D9Shader::~Direct3D9Shader()
{
    if (VertexShaderObject_)
        VertexShaderObject_->Release();
    if (PixelShaderObject_)
        PixelShaderObject_->Release();
    if (ConstantTable_)
        ConstantTable_->Release();
}

/* Shader compilation */

bool Direct3D9Shader::compile(const std::vector<io::stringc> &ShaderBuffer, const io::stringc &EntryPoint)
{
    bool Result = false;
    
    c8* ProgramBuffer = 0;
    createProgramString(ShaderBuffer, ProgramBuffer);
    
    if (Type_ == SHADER_VERTEX_PROGRAM || Type_ == SHADER_PIXEL_PROGRAM)
        Result = compileProgram(ProgramBuffer);
    else if (Type_ == SHADER_VERTEX)
    {
        Result = compileHLSL(
            ProgramBuffer,
            EntryPoint.c_str(),
            d3dVertexShaderVersions[math::MinMax(Version_, HLSL_VERTEX_1_0, HLSL_VERTEX_3_0) - HLSL_VERTEX_1_0]
        );
    }
    else if (Type_ == SHADER_PIXEL)
    {
        Result = compileHLSL(
            ProgramBuffer,
            EntryPoint.c_str(),
            d3dPixelShaderVersions[math::MinMax(Version_, HLSL_PIXEL_1_0, HLSL_PIXEL_3_0) - HLSL_PIXEL_1_0]
        );
    }
    
    MemoryManager::deleteBuffer(ProgramBuffer);
    
    setupShaderConstants();
    
    return CompiledSuccessfully_ = Result;
}

/* Set the constants (by number) */

bool Direct3D9Shader::setConstant(s32 Number, const EConstantTypes Type, const f32 Value)
{
    if (!ConstantTable_)
    {
        io::Log::warning("Invalid constant table of HLSL shader");
        return false;
    }
    
    D3DXHANDLE Handle = ConstantTable_->GetConstant(0, Number);
    
    if (!Handle)
    {
        io::Log::warning("Could not get HLSL constant #" + io::stringc(Number));
        return false;
    }
    
    if (ConstantTable_->SetFloat(pD3D9Device_, Handle, Value))
    {
        io::Log::warning("Setting HLSL constant 'float' failed");
        return false;
    }
    
    return true;
}

bool Direct3D9Shader::setConstant(s32 Number, const EConstantTypes Type, const f32* Buffer, s32 Count)
{
    if (!ConstantTable_)
    {
        io::Log::warning("Invalid constant table of HLSL shader");
        return false;
    }
    
    D3DXHANDLE Handle = ConstantTable_->GetConstant(0, Number);
    
    if (!Handle)
    {
        io::Log::warning("Could not get HLSL constant #" + io::stringc(Number));
        return false;
    }
    
    if (ConstantTable_->SetFloatArray(pD3D9Device_, Handle, Buffer, Count))
    {
        io::Log::warning("Setting HLSL constant 'float array' failed");
        return false;
    }
    
    return true;
}


bool Direct3D9Shader::setConstant(s32 Number, const EConstantTypes Type, const dim::vector3df &Position)
{
    return setConstant(Number, Type, &Position.X, 3);
}

bool Direct3D9Shader::setConstant(s32 Number, const EConstantTypes Type, const video::color &Color)
{
    f32 Buffer[4];
    Color.getFloatArray(Buffer);
    return setConstant(Number, Type, Buffer, 4);
}

bool Direct3D9Shader::setConstant(s32 Number, const EConstantTypes Type, const dim::matrix4f &Matrix)
{
    return setConstant(Number, Type, Matrix.getArray(), 16);
}


/* Set the constants (by name) */

bool Direct3D9Shader::setConstant(const io::stringc &Name, const f32 Value)
{
    if (!ConstantTable_)
    {
        io::Log::warning("Invalid constant table of HLSL shader");
        return false;
    }
    
    D3DXHANDLE Handle = ConstantTable_->GetConstantByName(0, Name.c_str());
    
    if (!Handle)
    {
        io::Log::warning("Could not get HLSL constant \"" + Name + "\"");
        return false;
    }
    
    if (ConstantTable_->SetFloat(pD3D9Device_, Handle, Value))
    {
        io::Log::warning("Setting HLSL constant 'float' failed");
        return false;
    }
    
    return true;
}

bool Direct3D9Shader::setConstant(const io::stringc &Name, const f32* Buffer, s32 Count)
{
    if (!ConstantTable_)
    {
        io::Log::warning("Invalid constant table of HLSL shader");
        return false;
    }
    
    D3DXHANDLE Handle = ConstantTable_->GetConstantByName(0, Name.c_str());
    
    if (!Handle)
    {
        io::Log::warning("Could not get HLSL constant \"" + Name + "\"");
        return false;
    }
    
    if (ConstantTable_->SetFloatArray(pD3D9Device_, Handle, Buffer, Count))
    {
        io::Log::warning("Setting HLSL constant 'float array' failed");
        return false;
    }
    
    return true;
}


bool Direct3D9Shader::setConstant(const io::stringc &Name, const s32 Value)
{
    if (!ConstantTable_)
    {
        io::Log::warning("Invalid constant table of HLSL shader");
        return false;
    }
    
    D3DXHANDLE Handle = ConstantTable_->GetConstantByName(0, Name.c_str());
    
    if (!Handle)
    {
        io::Log::warning("Could not get HLSL constant \"" + Name + "\"");
        return false;
    }
    
    if (ConstantTable_->SetInt(pD3D9Device_, Handle, Value))
    {
        io::Log::warning("Setting HLSL constant 'integer' failed");
        return false;
    }
    
    return true;
}

bool Direct3D9Shader::setConstant(const io::stringc &Name, const s32* Buffer, s32 Count)
{
    return false; // !TODO!
}


bool Direct3D9Shader::setConstant(const io::stringc &Name, const dim::vector3df &Position)
{
    return setConstant(Name, &Position.X, 3);
}

bool Direct3D9Shader::setConstant(const io::stringc &Name, const dim::vector4df &Position)
{
    return setConstant(Name, &Position.X, 4);
}

bool Direct3D9Shader::setConstant(const io::stringc &Name, const video::color &Color)
{
    f32 Buffer[4];
    Color.getFloatArray(Buffer);
    return setConstant(Name, Buffer, 4);
}

bool Direct3D9Shader::setConstant(const io::stringc &Name, const dim::matrix4f &Matrix)
{
    return setConstant(Name, Matrix.getArray(), 16);
}


bool Direct3D9Shader::setConstant(const f32* Buffer, s32 StartRegister, s32 ConstAmount)
{
    return pD3D9Device_->SetVertexShaderConstantF(StartRegister, Buffer, ConstAmount) == D3D_OK;
}


/*
 * ======= Private: =======
 */

bool Direct3D9Shader::compileHLSL(const c8* ProgramBuffer, const c8* EntryPoint, const c8* TargetName)
{
    if (!ProgramBuffer)
        return false;
    
    /* Temporary variables */
    LPD3DXBUFFER Buffer = 0;
    LPD3DXBUFFER Errors = 0;
    
    /* Get the shader name */
    io::stringc ShaderName;
    
    switch (Type_)
    {
        case SHADER_VERTEX:
            ShaderName = "vertex"; break;
        case SHADER_PIXEL:
            ShaderName = "pixel"; break;
    }
    
    /* Compile the shader */
    HRESULT Result = d3dCompileShader(
        ProgramBuffer, strlen(ProgramBuffer), 0, 0, EntryPoint, TargetName, 0, &Buffer, &Errors, &ConstantTable_
    );
    
    /* Check for errors */
    if (Result)
    {
        printError("Direct3D9 HLSL " + ShaderName + " shader compilation failed:");
        
        if (Errors)
        {
            printError((c8*)Errors->GetBufferPointer());
            Errors->Release();
        }
        
        if (Buffer)
            Buffer->Release();
        
        return false;
    }
    
    if (Errors)
        Errors->Release();
    
    if (!Buffer)
        return false;
    
    /* Create the shader program */
    switch (Type_)
    {
        case SHADER_VERTEX:
            Result = pD3D9Device_->CreateVertexShader((DWORD*)Buffer->GetBufferPointer(), &VertexShaderObject_); break;
        case SHADER_PIXEL:
            Result = pD3D9Device_->CreatePixelShader((DWORD*)Buffer->GetBufferPointer(), &PixelShaderObject_); break;
        default:
        {
            io::Log::error("Direct3D9 does not support HLSL " + ShaderName + " shader");
            return false;
        }
    }
    
    if (Result)
    {
        io::Log::error("Could not create HLSL " + ShaderName + " shader");
        Buffer->Release();
        return false;
    }
    
    Buffer->Release();
    
    return true;
}

bool Direct3D9Shader::compileProgram(const c8* ProgramBuffer)
{
    if (!ProgramBuffer)
        return false;
    
    /* Temporary variables */
    LPD3DXBUFFER Buffer = 0;
    LPD3DXBUFFER Errors = 0;
    
    /* Get the shader name */
    io::stringc ShaderName;
    
    switch (Type_)
    {
        case SHADER_VERTEX:
            ShaderName = "vertex"; break;
        case SHADER_PIXEL:
            ShaderName = "pixel"; break;
    }
    
    /* Compile the shader */
    d3dAssembleShader(ProgramBuffer, strlen(ProgramBuffer), 0, 0, 0, &Buffer, &Errors);
    
    /* Check for errors */
    if (Errors)
    {
        /* Print the error messages */
        printError("Direct3D9 " + ShaderName + " program compilation failed:");
        printError((c8*)Errors->GetBufferPointer());
        
        /* Release all allocated objects */
        if (Buffer)
            Buffer->Release();
        Errors->Release();
        
        return false;
    }
    
    if (!Buffer)
        return false;
    
    /* Create the vertex shader */
    HRESULT Result = 0;
    
    switch (Type_)
    {
        case SHADER_VERTEX_PROGRAM:
            Result = pD3D9Device_->CreateVertexShader((DWORD*)Buffer->GetBufferPointer(), &VertexShaderObject_); break;
        case SHADER_PIXEL_PROGRAM:
            Result = pD3D9Device_->CreatePixelShader((DWORD*)Buffer->GetBufferPointer(), &PixelShaderObject_); break;
    }
    
    if (Result)
    {
        io::Log::error("Could not create " + ShaderName + " program");
        Buffer->Release();
        return false;
    }
    
    Buffer->Release();
    
    return true;
}

void Direct3D9Shader::createProgramString(const std::vector<io::stringc> &ShaderBuffer, c8* &ProgramBuffer)
{
    /* Create a single line string */
    io::stringc ProgramString;
    
    for (std::vector<io::stringc>::const_iterator it = ShaderBuffer.begin(); it != ShaderBuffer.end(); ++it)
        ProgramString += *it + "\n";
    
    /* Create the program buffer and copy the data */
    ProgramBuffer = new c8[ProgramString.size() + 1];
    
    memcpy(ProgramBuffer, ProgramString.c_str(), ProgramString.size());
    
    ProgramBuffer[ProgramString.size()] = 0;
}


HRESULT Direct3D9Shader::d3dAssembleShader(
    LPCSTR pSrcData, UINT SrcDataLen, CONST D3DXMACRO* pDefines,
    LPD3DXINCLUDE pInclude, DWORD Flags, LPD3DXBUFFER* ppShader, LPD3DXBUFFER* ppErrorMsgs)
{
    #if D3DX_SDK_VERSION < 24
    
    #ifdef _MSC_VER
    #pragma comment(lib, "d3dx9.lib")
    #endif
    
    return D3DXAssembleShader(
        pSrcData, SrcDataLen, pDefines, pInclude, Flags, ppShader, ppErrorMsgs
    );
    
    #else
    
    typedef HRESULT (WINAPI *PFND3DXASSEMBLESHADER)(
        LPCSTR pSrcData, UINT SrcDataLen, CONST D3DXMACRO* pDefines,
        LPD3DXINCLUDE pInclude, DWORD Flags, LPD3DXBUFFER* ppShader, LPD3DXBUFFER* ppErrorMsgs
    );
    
    static PFND3DXASSEMBLESHADER pFncAssembleShader = 0;
    
    if (!pFncAssembleShader)
    {
        HMODULE hModule = LoadLibrary(d3dDllFileName.c_str());
        
        if (hModule)
            pFncAssembleShader = (PFND3DXASSEMBLESHADER)GetProcAddress(hModule, "D3DXAssembleShader");
        
        if (!pFncAssembleShader)
        {
            io::Log::error(
                "Could not load function \"D3DXAssembleShader\" from Direct3D9 library file: \"" +
                d3dDllFileName + "\""
            );
        }
    }
    
    if (pFncAssembleShader)
    {
        return pFncAssembleShader(
            pSrcData, SrcDataLen, pDefines, pInclude, Flags, ppShader, ppErrorMsgs
        );
    }
    
    #endif
    
    return 0;
}

HRESULT Direct3D9Shader::d3dCompileShader(
    LPCSTR pSrcData, UINT SrcDataLen, CONST D3DXMACRO* pDefines,
    LPD3DXINCLUDE pInclude, LPCSTR pFunctionName,
    LPCSTR pProfile, DWORD Flags, LPD3DXBUFFER* ppShader,
    LPD3DXBUFFER* ppErrorMsgs, LPD3DXCONSTANTTABLE* ppConstantTable)
{
    #if D3DX_SDK_VERSION < 24
    
    #ifdef _MSC_VER
    #pragma comment(lib, "d3dx9.lib")
    #endif
    
    return D3DXCompileShader(
        pSrcData, SrcDataLen, pDefines, pInclude, pFunctionName, pProfile, Flags, ppShader, ppErrorMsgs, ppConstantTable
    );
    
    #else
    
    typedef HRESULT (WINAPI *PFND3DXCOMPILESHADER)(
        LPCSTR pSrcData, UINT SrcDataLen, CONST D3DXMACRO* pDefines,
        LPD3DXINCLUDE pInclude, LPCSTR pFunctionName,
        LPCSTR pProfile, DWORD Flags, LPD3DXBUFFER* ppShader,
        LPD3DXBUFFER* ppErrorMsgs, LPD3DXCONSTANTTABLE* ppConstantTable
    );
    
    static PFND3DXCOMPILESHADER pFncCompileShader = 0;
    
    if (!pFncCompileShader)
    {
        HMODULE hModule = LoadLibrary(d3dDllFileName.c_str());
        
        if (hModule)
            pFncCompileShader = (PFND3DXCOMPILESHADER)GetProcAddress(hModule, "D3DXCompileShader");
        
        if (!pFncCompileShader)
        {
            io::Log::error(
                "Could not load function \"D3DXCompileShader\" from Direct3D9 library file: \"" + 
                d3dDllFileName + "\""
            );
        }
    }
    
    if (pFncCompileShader)
    {
        return pFncCompileShader(
            pSrcData, SrcDataLen, pDefines, pInclude, pFunctionName, pProfile, Flags, ppShader, ppErrorMsgs, ppConstantTable
        );
    }
    
    #endif
    
    return 0;
}

bool Direct3D9Shader::setupShaderConstants()
{
    if (!ConstantTable_)
    {
        io::Log::error("Cannot setup shader constants without shader table object");
        return false;
    }
    
    /* Get count of shader constants */
    D3DXCONSTANTTABLE_DESC ConstTableDesc;
    ConstantTable_->GetDesc(&ConstTableDesc);
    
    const u32 Count = ConstTableDesc.Constants;
    
    /* Temporary memory */
    D3DXCONSTANT_DESC ConstDesc;
    u32 ArrayCount;
    SShaderConstant Constant;
    
    for (u32 i = 0; i < Count; ++i)
    {
        /* Get next shader constant */
        if (ConstantTable_->GetConstantDesc(ConstantTable_->GetConstant(0, i), &ConstDesc, &ArrayCount) != D3D_OK)
        {
            io::Log::error("Could not get shader constant #" + io::stringc(i));
            continue;
        }
        
        Constant.Type = CONSTANT_UNKNOWN;
        
        /* Determine constant type */
        switch (ConstDesc.Type)
        {
            case D3DXPT_BOOL:
                Constant.Type = CONSTANT_BOOL; break;
            case D3DXPT_INT:
                Constant.Type = CONSTANT_INT; break;
            case D3DXPT_FLOAT:
            {
                const u32 RowColSize = math::Max(ConstDesc.Rows, ConstDesc.Columns);
                
                switch (ConstDesc.Class)
                {
                    case D3DXPC_SCALAR:
                    {
                        Constant.Type = CONSTANT_FLOAT;
                    }
                    break;
                    
                    case D3DXPC_VECTOR:
                    {
                        switch (RowColSize)
                        {
                            case 2: Constant.Type = CONSTANT_VECTOR2; break;
                            case 3: Constant.Type = CONSTANT_VECTOR3; break;
                            case 4: Constant.Type = CONSTANT_VECTOR4; break;
                        }
                    }
                    break;
                    
                    case D3DXPC_MATRIX_ROWS:
                    case D3DXPC_MATRIX_COLUMNS:
                    {
                        switch (RowColSize)
                        {
                            case 2: Constant.Type = CONSTANT_MATRIX2; break;
                            case 3: Constant.Type = CONSTANT_MATRIX3; break;
                            case 4: Constant.Type = CONSTANT_MATRIX4; break;
                        }
                    }
                    break;
                }
            }
            break;
        }
        
        /* Setup constant name, array size and add constant to the list */
        Constant.Name   = io::stringc(ConstDesc.Name);
        Constant.Count  = ArrayCount;
        
        #if 0 //!!!
        io::Log::message(
            " DEB: [ spDirect3D9Shader.cpp ] New Shader Constant: \"" + io::stringc(Constant.Name) + "\", Count = " + io::stringc(ArrayCount) +
            ", Type = " + io::stringc(static_cast<s32>(Constant.Type))
        );
        #endif
        
        ConstantList_.push_back(Constant);
    }
    
    return true;
}


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
