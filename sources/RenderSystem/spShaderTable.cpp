/*
 * Shader table file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "RenderSystem/spShaderTable.hpp"


namespace sp
{
namespace video
{


ShaderTable::ShaderTable()
    : ObjectCallback_(0), SurfaceCallback_(0), VertexShader_(0), PixelShader_(0),
    GeometryShader_(0), HullShader_(0), DomainShader_(0), ComputeShader_(0),
    HighLevel_(true), CompiledSuccessfully_(false)
{
}
ShaderTable::~ShaderTable()
{
}

void ShaderTable::printError(const io::stringc &Message)
{
    io::Log::message(Message, io::LOG_ERROR | io::LOG_TIME | io::LOG_NOTAB);
}
void ShaderTable::printWarning(const io::stringc &Message)
{
    io::Log::message(Message, io::LOG_WARNING | io::LOG_TIME | io::LOG_NOTAB);
}


} // /namespace scene

} // /namespace sp



// ================================================================================
