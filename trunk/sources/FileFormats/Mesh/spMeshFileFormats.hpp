/*
 * Mesh file formats header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_MESHFILEFORMATS_H__
#define __SP_MESHFILEFORMATS_H__


#include "Base/spStandard.hpp"

/* MeshLoader: */

#include "FileFormats/Mesh/spMeshLoader.hpp"

#include "FileFormats/Mesh/spMeshLoaderSPM.hpp"     // SoftPixel Mesh
#include "FileFormats/Mesh/spMeshLoader3DS.hpp"     // 3D Studio
#include "FileFormats/Mesh/spMeshLoaderMS3D.hpp"    // Milkshape 3D
#include "FileFormats/Mesh/spMeshLoaderX.hpp"       // DirectX
#include "FileFormats/Mesh/spMeshLoaderB3D.hpp"     // Blitz3D
#include "FileFormats/Mesh/spMeshLoaderMD2.hpp"     // Quake II Model
#include "FileFormats/Mesh/spMeshLoaderMD3.hpp"     // Quake III Model
#include "FileFormats/Mesh/spMeshLoaderOBJ.hpp"     // Wavefront Object

/* MeshSaver: */

#include "FileFormats/Mesh/spMeshSaver.hpp"

#include "FileFormats/Mesh/spMeshSaverSPM.hpp"
#include "FileFormats/Mesh/spMeshSaverB3D.hpp"
#include "FileFormats/Mesh/spMeshSaverOBJ.hpp"

/* SceneLoader: */

#include "FileFormats/Scene/spSceneLoaderSPSB.hpp"  // SoftPixel Sandbox Scene
#include "FileFormats/Scene/spSceneLoaderBSP1.hpp"  // Quake III Arena level
#include "FileFormats/Scene/spSceneLoaderBSP3.hpp"  // Quake 1/ Half-Life 1/ Counter-Strike 1.0 - 1.6 level


#endif



// ================================================================================
