/*
 * Compilation optinos header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_COMPILATION_OPTIONS_H__
#define __SP_COMPILATION_OPTIONS_H__


/* === Compilation configurations === */

#define SP_COMPILE_WITH_RENDERSYSTEMS   // Render Systems
#define SP_COMPILE_WITH_SCENEGRAPHS     // Scene Graphs
#define SP_COMPILE_WITH_NETWORKSYSTEM   // Network System
#define SP_COMPILE_WITH_SOUNDSYSTEM     // Sound System (effects each sound device)
#define SP_COMPILE_WITH_UTILITIES       // Tools (effects each tool)
#define SP_COMPILE_WITH_PRIMITIVES      // Standard Primitive Objects (Cube, Sphere etc.)
#define SP_COMPILE_WITH_GUI             // Graphics User Interface (GUI)
#define SP_COMPILE_WITH_FORMATHANDLERS  // File Format Handlers (Image-, Mesh- and Scene loaders/ savers)
//#define SP_COMPILE_WITH_PHYSICS         // Physics systems
//#define SP_COMPILE_WITH_CG              // Cg Toolkit by NVIDIA
//#define SP_COMPILE_WITH_OPENCL          // OpenCL Toolkit for GPGPU

#ifdef SP_COMPILE_WITH_RENDERSYSTEMS
#   define SP_COMPILE_WITH_OPENGL               // OpenGL 1.1 - 4.1
#   define SP_COMPILE_WITH_OPENGLES1            // OpenGL|ES 1.0 - 1.1
#   define SP_COMPILE_WITH_OPENGLES2            // OpenGL|ES 2.0
//#   define SP_COMPILE_WITH_DIRECT3D9            // Direct3D 9
//#   define SP_COMPILE_WITH_DIRECT3D11           // Direct3D 11
#   ifdef SP_COMPILE_WITH_CG
#       define SP_COMPILE_WITH_DEFERREDRENDERER // DeferredRenderer
#   endif
#endif

#ifdef SP_COMPILE_WITH_SCENEGRAPHS
#   define SP_COMPILE_WITH_SCENEGRAPH_SIMPLE        // Simple scene graph
#   define SP_COMPILE_WITH_SCENEGRAPH_SIMPLE_STREAM // Simple scene graph with streaming (for multi-threading)
#   define SP_COMPILE_WITH_SCENEGRAPH_FAMILY_TREE   // Simple scene graph with child tree hierarchy
#endif

#ifdef SP_COMPILE_WITH_SOUNDSYSTEM
#   define SP_COMPILE_WITH_WINMM            // Windows Multi Media system
#   define SP_COMPILE_WITH_OPENAL           // OpenAL sound device
//#   define SP_COMPILE_WITH_XAUDIO2          // DirectX XAudio2 sound device
#   define SP_COMPILE_WITH_OPENSLES         // OpenSL|ES 1.0

#   define SP_COMPILE_WITH_SOUNDLOADER_WAV  // Sound loader WAV (RIFF Wave)
#endif

#ifdef SP_COMPILE_WITH_PHYSICS
//#   define SP_COMPILE_WITH_NEWTON   // Newton Game Dynamics
#   define SP_COMPILE_WITH_PHYSX    // NVIDIA PhysX
#   define SP_COMPILE_WITH_BULLET   // Bullet Physics
#endif

#ifdef SP_COMPILE_WITH_UTILITIES
#   define SP_COMPILE_WITH_LIGHTMAPGENERATOR    // Lightmap generator
#   define SP_COMPILE_WITH_MODELCOMBINER        // Model combiner
#   define SP_COMPILE_WITH_SCRIPTLOADER         // Script loader
#   define SP_COMPILE_WITH_PARTICLEANIMATOR     // Particle animator
#   define SP_COMPILE_WITH_IMAGEMODIFIER        // Image modifier
#   define SP_COMPILE_WITH_XMLPARSER            // XML parser
#   define SP_COMPILE_WITH_PATHFINDER           // Path finder
#   define SP_COMPILE_WITH_INPUTSERVICE         // Input service
#   define SP_COMPILE_WITH_STORYBOARD           // Storyboard
#   define SP_COMPILE_WITH_TOKENPARSER          // Token parser
#   define SP_COMPILE_WITH_COMMANDLINE          // Command line
#   define SP_COMPILE_WITH_WEBPAGERENDERER      // Web page renderer
#endif

#ifdef SP_COMPILE_WITH_PRIMITIVES
#   define SP_COMPILE_WITH_PRIMITIVE_CUBE           // Cube
#   define SP_COMPILE_WITH_PRIMITIVE_CONE           // Cone
#   define SP_COMPILE_WITH_PRIMITIVE_CYLINDER       // Cylinder
#   define SP_COMPILE_WITH_PRIMITIVE_SPHERE         // Sphere
#   define SP_COMPILE_WITH_PRIMITIVE_ICOSPHERE      // Ico Sphere
#   define SP_COMPILE_WITH_PRIMITIVE_TORUS          // Torus
#   define SP_COMPILE_WITH_PRIMITIVE_TORUSKNOT      // Torus Knot
#   define SP_COMPILE_WITH_PRIMITIVE_SPIRAL         // Spiral
#   define SP_COMPILE_WITH_PRIMITIVE_PIPE           // Pipe
#   define SP_COMPILE_WITH_PRIMITIVE_PLANE          // Plane
#   define SP_COMPILE_WITH_PRIMITIVE_DISK           // Disk
#   define SP_COMPILE_WITH_PRIMITIVE_CUBOCTAHEDRON  // Cuboctahedron
#   define SP_COMPILE_WITH_PRIMITIVE_TETRAHEDRON    // Tetrahedron
#   define SP_COMPILE_WITH_PRIMITIVE_OCTAHEDRON     // Octahedron
#   define SP_COMPILE_WITH_PRIMITIVE_DODECAHEDRON   // Dodecahedron
#   define SP_COMPILE_WITH_PRIMITIVE_ICOSAHEDRON    // Icosahedron
#   define SP_COMPILE_WITH_PRIMITIVE_TEAPOT         // Utah Teapot
#   define SP_COMPILE_WITH_PRIMITIVE_WIRE_CUBE      // Wire Cube
#   ifdef SP_COMPILE_WITH_PRIMITIVE_TEAPOT
//#       define SP_COMPILE_WITH_PRIMITIVE_TEAPOT_DYNAMIC // Create the teapot using Bezier patches
#   endif
#   define SP_COMPILE_WITH_PRIMITIVE_SUPERSHAPE     // Super shape
#endif

#ifdef SP_COMPILE_WITH_FORMATHANDLERS
#   define SP_COMPILE_WITH_TEXLOADER_BMP    // Texture loader BMP (Windows Bitmaps)
#   define SP_COMPILE_WITH_TEXLOADER_JPG    // Texture loader JPG (Joint Photographics Experts)
#   define SP_COMPILE_WITH_TEXLOADER_PCX    // Texture loader PCX (Pixture Exchange)
#   define SP_COMPILE_WITH_TEXLOADER_PNG    // Texture loader PNG (Portable Network Graphics)
#   define SP_COMPILE_WITH_TEXLOADER_TGA    // Texture loader TGA (Targa True Image)
#   define SP_COMPILE_WITH_TEXLOADER_DDS    // Texture loader DDS (Direct Draw Surface)
#   define SP_COMPILE_WITH_TEXLOADER_WAD    // Texture loader WAD (Where is All the Data)
#   define SP_COMPILE_WITH_TEXSAVER_BMP     // Texture saver BMP

#   define SP_COMPILE_WITH_MESHLOADER_3DS   // Mesh loader 3DS (3D Studio)
#   define SP_COMPILE_WITH_MESHLOADER_B3D   // Mesh loader B3D (Blitz3D)
//#   define SP_COMPILE_WITH_MESHLOADER_X     // Mesh loader X (DirectX)
#   define SP_COMPILE_WITH_MESHLOADER_MD2   // Mesh loader MD2 (Quake 2)
#   define SP_COMPILE_WITH_MESHLOADER_MD3   // Mesh loader MD3 (Quake III)
#   define SP_COMPILE_WITH_MESHLOADER_MS3D  // Mesh loader MS3D (Milkshape 3D)
#   define SP_COMPILE_WITH_MESHLOADER_OBJ   // Mesh loader OBJ (Wavefront Object)
#   define SP_COMPILE_WITH_MESHLOADER_SPM   // Mesh loader SPM (SoftPixel Mesh)
#   define SP_COMPILE_WITH_MESHSAVER_B3D    // Mesh saver B3D
#   define SP_COMPILE_WITH_MESHSAVER_OBJ    // Mesh saver OBJ
#   define SP_COMPILE_WITH_MESHSAVER_SPM    // Mesh saver SPM

#   define SP_COMPILE_WITH_SCENELOADER_BSP1 // Quake 1 BSP loader
#   define SP_COMPILE_WITH_SCENELOADER_BSP3 // Quake 3 BSP loader
#   define SP_COMPILE_WITH_SCENELOADER_SPSB // Scene loader SPS (SoftPixel Scene)
#endif

/* === Warning deactivation settings === */

#if defined(SP_COMPILER_VC)
#   pragma warning(disable : 4250) // Warning occurs when a class inherits from several base-classes and a virtual inheritance is used.
#   pragma warning(disable : 4251) // Warning occurs when a class has a member of another class which is not declared for dll-export.
#   pragma warning(disable : 4290) // Warning occurs when a function is declared to throw a specified exception.
#endif


#endif



// ================================================================================
