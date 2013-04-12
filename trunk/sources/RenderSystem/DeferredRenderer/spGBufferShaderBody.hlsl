/*
 * GBuffer D3D11 shader body file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

    Out.DiffuseAndSpecular = DiffuseAndSpecular;
    Out.NormalAndDepth = NormalAndDepth;

    #if 0//!!!
	WorldPos += 0.01;
    Out.DiffuseAndSpecular.rgb = WorldPos - floor(WorldPos);
    #endif

    return Out;
}
