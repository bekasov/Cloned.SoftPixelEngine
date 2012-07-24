/*
 * Format enumerations header
 * 
 * This file is part of the "SoftPixel Scene Importer/Exporter" (Copyright (c) 2012 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __SPS_FORMAT_ENUMERATIONS_H__
#define __SPS_FORMAT_ENUMERATIONS_H__


//! Errors should always displayed in any form.
enum EErrorTypes
{
    ERROR_DEFAULT,
    ERROR_FATAL,        //!< This error type results in canceling the import or export.
    ERROR_FILE,         //!< Problems with file reading or writing.
    ERROR_HEADER,       //!< Header is incorrect or corrupt.
    ERROR_BASEOBJECT,   //!< Failure in the base object data.
    ERROR_MATERIAL,     //!< Failure in the material data.
    ERROR_SURFACE,      //!< Failure in the surface data.
    ERROR_COLLISION,    //!< Failure in the collision data.
    ERROR_CONSTRUCTION, //!< Failure in the mesh construction data.
    ERROR_VIEWCULLING,  //!< Failure in the view culling data.
    ERROR_TEXTURE,      //!< Failure in the texture or texture class data.
    ERROR_LIGHTMAP,     //!< Failure in the lightmap or lightmap scene data.
};

//! Warnings can be ignored.
enum EWarningType
{
    WARNING_DEFAULT,
    WARNING_SHADER,     //!< Warning in the shader or shader parameter data.
};

//! Header lump directories.
enum ELumpDirectories
{
    LUMP_SCENECONFIG = 0,
    
    LUMP_MESHES,
    LUMP_LIGHTS,
    LUMP_CAMERAS,
    LUMP_WAYPOINTS,
    
    LUMP_TEXTURES,
    LUMP_TEXTURECLASSES,
    
    LUMP_LIGHTMAPS,
    LUMP_LIGHTMAPSCENE,
    
    LUMP_SHADERCLASSES,
    
    LUMP_BOUNDVOLUMES,
    LUMP_SOUNDS,
    LUMP_SPRITES,
    LUMP_ANIMNODES,
    
    LUMP_COUNT,
};

//! Shader parameter types.
enum EShaderParamTypes
{
    SHADERPARAM_UNKNOWN,    //!< Unknown shader parameter.
    SHADERPARAM_FLOAT,      //!< Floating point.
    SHADERPARAM_INT,        //!< Integer.
    SHADERPARAM_BOOL,       //!< Boolean.
    SHADERPARAM_VEC2,       //!< Vector with 2 components (x, y).
    SHADERPARAM_VEC3,       //!< Vector with 3 components (x, y, z).
    SHADERPARAM_VEC4,       //!< Vector with 4 components (x, y, z, w).
    SHADERPARAM_MATRIX,     //!< Matrix string classification.
};


#endif



// ================================================================================
