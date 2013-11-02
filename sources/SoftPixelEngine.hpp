/*
 * SoftPixel Engine main header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

/*
 * ========== LICENSE (zlib/libpng): ==========
 * 
 * Copyright (c) 2008 Lukas Hermanns
 * 
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 * 
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 * 
 *     1. The origin of this software must not be misrepresented; you must not
 *     claim that you wrote the original software. If you use this software
 *     in a product, an acknowledgment in the product documentation would be
 *     appreciated but is not required.
 * 
 *     2. Altered source versions must be plainly marked as such, and must not be
 *     misrepresented as being the original software.
 * 
 *     3. This notice may not be removed or altered from any source
 *     distribution.
 * 
 * Please note that the SoftPixel Engine is based in part on the work of the
 * Independent JPEG Group, the zlib and the libpng. This means that if you use
 * the SoftPixel Engine in your product, you must acknowledge somewhere in your
 * documentation that you've used the IJG code. It would also be nice to mention
 * that you use the SoftPixel Engine, the zlib and libpng. See the README files
 * in the jpeglib, the zlib and libpng for further information.
 * 
 */

#ifndef __cplusplus
#   error The 'SoftPixel Engine' can only be used with C++!
#endif

#include "Base/spStandard.hpp"
#include "Base/spDimension.hpp"
#include "Base/spInputOutput.hpp"
#include "Base/spMath.hpp"
#include "Base/spThreadManager.hpp"
#include "Base/spTimer.hpp"
#include "Base/spMathRasterizer.hpp"
#include "Base/spMathInterpolator.hpp"
#include "Base/spDimensionPolygon.hpp"
#include "Base/spAssetContainer.hpp"

#include "Platform/spSoftPixelDeviceOS.hpp"

#include "GUI/spGUIManager.hpp"

#include "RenderSystem/spRenderSystem.hpp"
#include "RenderSystem/spVideoModeEnumerator.hpp"
#include "RenderSystem/spShaderResource.hpp"
#include "RenderSystem/AdvancedRenderer/spDeferredRenderer.hpp"
#include "RenderSystem/AdvancedRenderer/spForwardRenderer.hpp"
#include "RenderSystem/AdvancedRenderer/spSparseOctreeVoxelizer.hpp"
#include "RenderSystem/spTextureLayerStandard.hpp"
#include "RenderSystem/spTextureLayerRelief.hpp"
#include "RenderSystem/spQuery.hpp"
#include "RenderSystem/PostProcessing/spRadialBlur.hpp"

#include "SceneGraph/spSceneGraph.hpp"
#include "SceneGraph/spSceneGraphPortalBased.hpp"
#include "SceneGraph/spScenePortal.hpp"
#include "SceneGraph/spSceneSector.hpp"

#include "SceneGraph/Collision/spCollisionGraph.hpp"

#include "SceneGraph/Animation/spTextureAnimation.hpp"

#include "SoundSystem/spSoundDevice.hpp"

#include "Framework/Network/spNetworkSystem.hpp"
#include "Framework/Network/spNetworkSessionLogin.hpp"
#include "Framework/Network/spNetworkSessionReception.hpp"
#include "Framework/Physics/spPhysicsSimulator.hpp"
#include "Framework/Tools/spExtendedToolset.hpp"
#include "Framework/Tools/spUtilityAudioPCMPlotter.hpp"
#include "Framework/Tools/ScriptParser/spUtilityTokenParser.hpp"
#include "Framework/Tools/ScriptParser/spUtilityMaterialScriptReader.hpp"


//
// ============== FOR DOXYGEN DOCUMENTATION ==============
//

/*! \mainpage SoftPixel Engine 3.2 API documentation
 * 
 * <div algin="center"><img alt="SoftPixel Engine Logo" src="../media/SoftPixelEngineLogo.png"></div>
 * 
 * \section intro Introduction
 * 
 * Welcome to the SoftPixel Engine API documentation where you can find each function available for the current version.
 * For detailed questions you can joint the <a href="http://softpixelengine.sourceforge.net/forum/">SoftPixel Forum</a>
 * and ask questions in the section of the SoftPixel Engine.
 * <p>
 * You can also download this API docu as ZIP file
 * </p>
 * <p><a href="http://softpixelengine.sourceforge.net/download/SoftPixelEngineDocu%28v.3.2%29.zip">SoftPixelEngineDocu(v.3.2).zip</a></p>
 * <p><a href="http://softpixelengine.sourceforge.net/download/SoftPixelEngineDocu%28v.3.1%29.zip">SoftPixelEngineDocu(v.3.1).zip</a></p>
 * 
 * \section getstart Getting started
 * 
 * This is the "GettingStarted" tutorial for the first step using the SoftPixel Engine.
 * 
 * \code
 * #include <SoftPixelEngine.hpp>
 * 
 * using namespace sp; // Main namespace
 * 
 * int main()
 * {
 *     // The softpixel-device opens the screen (windowed screen or fullscreen is possible)
 *     SoftPixelDevice* spDevice = createGraphicsDevice(
 *         video::RENDERER_AUTODETECT, dim::size2di(640, 480), 32, "GettingStarted"
 *     );
 *     
 *     video::RenderSystem* spRenderer = spDevice->getRenderSystem();   // Get the render system for drawing and rendering operations.
 *     video::RenderContext* spContext = spDevice->getRenderContext();  // Get the render context to flip video buffers and change settings of the main window.
 *     io::InputControl* spControl = spDevice->getInputControl();       // Get the input controller for keyboard- and mouse events.
 *     
 *     scene::SceneGraph* spScene = spDevice->createSceneGraph();       // Create a scene graph where all the 3d objects are stored and handled.
 *     spScene->setLighting(true);                                      // Enable global lighting.
 *     
 *     scene::Camera* Cam = spScene->createCamera();                    // Create a view camera whose field of view is rendered.
 *     scene::Light* Lit = spScene->createLight();                      // Create a light source. By default directional light.
 *     
 *     scene::Mesh* Obj = spScene->createMesh(scene::MESH_TEAPOT);      // Create a renderable mesh object from the standard geometry library.
 *     Obj->setPosition(dim::vector3df(0, 0, 2.5));                     // Set the object's position (x, y, z).
 *     
 *     // The main loop in which the device will be updated
 *     while (spDevice->updateEvents() && !spControl->keyDown(io::KEY_ESCAPE))
 *     {
 *         spRenderer->clearBuffers();                                  // Clear the video buffer (Color and depth buffer).
 *         
 *         spScene->renderScene();                                      // Render the whole scene.
 *         
 *         Obj->turn(dim::vector3df(1, 1, 1));                          // Rotate our object to present it.
 *         
 *         spContext->flipBuffers();                                    // Flip the video buffers (Color and depth buffer). Now we can see the current frame.
 *     }
 *     
 *     deleteDevice();                                                  // Delete the engine device. This deletes all allocated resources and closes the graphics screen.
 *     
 *     return 0;
 * }
 * \endcode
 */

/**
 * \page page_credits Credits
 * 
 * \htmlonly
 * <ul>
 * <li>Lukas Hermanns (Creator, developer, project leader)</li>
 * </ul>
 * \endhtmlonly
 */

/**
 * \page page_license License
 * 
 * \htmlonly
 * <pre>
 * Copyright (c) 2008 Lukas Hermanns
 * 
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 * 
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 * 
 *     1. The origin of this software must not be misrepresented; you must not
 *     claim that you wrote the original software. If you use this software
 *     in a product, an acknowledgment in the product documentation would be
 *     appreciated but is not required.
 * 
 *     2. Altered source versions must be plainly marked as such, and must not be
 *     misrepresented as being the original software.
 * 
 *     3. This notice may not be removed or altered from any source
 *     distribution.
 * </pre>
 * \endhtmlonly
 */

/**
 * \page page_changes17_18 Changes from 1.7 to 1.8
 * 
 * \code
 * // === Changes for Movies === //
 * 
 * namespace movie -> has been removed
 * movie::Movie -> video::Movie
 * movie::MovieManager::loadMovie -> video::VideoDriver::loadMovie
 * movie::MovieManager::renderMovieTexture -> video::Movie::renderTexture
 * movie::MovieManager::playMovie -> video::Movie::play
 * 
 * // Example:
 * video::Movie* MyMovieObject = spRenderer->loadMovie("MyMovieFile.avi");
 * 
 * if (!MyMovieObject->valid())
 *     io::printError("The movie could not be loaded");
 * 
 * MyMovieObject->play();
 * MyMovieObject->setSeek(0.5); // To the middle of the movie
 * MyMovieObject->setSpeed(1.5); // Play with 1.5 times the speed, negative values are also possible but much slower!
 * 
 * MyMovieObject->renderTexture(MyTextureObject);
 * 
 * MyMovieObject->reload("MyMovieField2.avi"); // You can also reload an other movie file using the same movie object handle
 * // /Example
 * 
 * 
 * // === Changes for Sounds === //
 * 
 * createListener -> createSoundDevice
 * 
 * // Example:
 * sound::SensitiveSoundDevice* spListener = sound::createSoundDevice();
 * 
 * sound::Sound* MySoundObject = spListener->loadSound("MySoundFile.wav");
 * 
 * if (!MySoundObject->valid())
 *     io::printError("The sound could not be loaded");
 * 
 * MySoundObject->play(); // Remember to the "Movie" class with its simple functions: "play", "pause", "stop"
 * MySoundObject->setVolume(0.5); // 50% volume
 * MySoundObject->setSeek(0.1);
 * MySoundObject->setSpeed(1.5); // 1.5 times the speed, 1.0 is default (in the old version 0.0 was normal, -1.0 was slow, 1.0 was fast)
 * 
 * while (!MySoundObject->playing()) // Check if the sound is currently playing
 *     if (MySoundObject->finish()) // Stupid request within the above code, but just an example :)
 *         io::printMessage("The sound has been finished");
 * // /Example
 * 
 * // New enumerations
 * SOUNDDEVICE_AUTODETECT
 * SOUNDDEVICE_WINMM
 * SOUNDDEVICE_DUMMY
 * 
 * 
 * // === Changes for Networking === //
 * 
 * // Example:
 * network::NetworkSystem* spNetwork = new network::NetworkSystem(); // (going to be changed in "spDevice->getNetworkSystem()")
 * 
 * if (I_Am_The_Server)
 * {
 *     if (!spNetwork->openServer(Port)) // Default port number is 10000
 *         io::printError("The server could not be opened");
 * }
 * else
 * {
 *     // Useful function is "std::vector<io::stringc> spNetwork->getIPAddressesByName("HostName")" to each IP address of the host by its name
 *     if (!spNetwork->joinServer("192.168.0.1", Port)) // The IP address of the host
 *         io::printError("The server could not be opened");
 * }
 * 
 * spNetwork->sendPacket("MyMessage", 0); // instead of 0 you can use an "network::NetworkClient" pointer for a special Acceptor, otherwise (0) the message will be send to all in the network (which joined the SPE server)
 * 
 * network::SNetworkPacket MyPacket;
 * 
 * if (spNetwork->pickPacket(MyPacket))
 *     io::printMessage(io::stringc("I've got a network message: ") + MyPacket.Message); // Received a message from "MyPacket.Sender"
 * 
 * spNetwork->disconnect(); // Disconnect from the server or the server self
 * // /Example
 * 
 * 
 * // === Changes for File system === //
 * 
 * io::FileLoader -> io::FileSystem
 * 
 * // Example:
 * io::FileSystem* spFileSys = spDevice->getFileSystem(); // alternativly "new io::FileSystem();"
 * io::File* MyFileObject = spFileSys->openFile("MyFile.dat");
 * 
 * if (!MyFileObject) // In this case no object will be created when the file could not be opened (e.g. because of missing file)!
 *     io::printError("The file could not be opened");
 * 
 * MyFileObject->writeString("MyTextField"); // Writes just a string
 * MyFileObject->writeStringN("MyTextField"); // Writes a string and a new line character (on Windows: char(13) & char(10), on Linux: char(10))
 * MyFileObject->writeStringC("MyTextField"); // Writes an ANSI C string, terminated by 0
 * MyFileObject->writeStringData("MyTextField"); // Writes an interger value with the string length and the string self
 * 
 * MyFileObject->writeValue<s32>(math::Random(100)); // Writes a random integer: "s32", 32 bit value
 * 
 * while (!MyFileObject->isEOF()) // Check "EndOfFile"
 *     io::printMessage(MyFileObject->readString()); // Reads a string until the line ends (description see "writeStringN")
 * 
 * MyFileObject->readValue<s32>(); // Reads an interger value
 * 
 * FILE* AnsiCFileHandle = MyFileObject->getHandle(); // Old name "getID", return the ANSI C "FILE" pointer/ handle to proceed own ANSI C file operations.
 * s32 FileSizeInBytes = MyFileObject->getSize(); // Old name "getLength"
 * 
 * spFileSys->closeFile(MyFileObject);
 * // /Example
 * 
 * 
 * // === Changes for Entities and Nodes === //
 * 
 * Node::doTranslate -> Node::translate
 * Node::doTransform -> Node::transform
 * Node::doTurn -> Node::turn
 * Node::doMove -> Node::move
 * Entity::doTextureTranslate -> Entity::textureTranslate
 * Entity::doTextureTransform -> Entity::textureTransform
 * Entity::doTextureTurn -> Entity::textureTurn
 * Entity::doMeshTranslate-> Entity::meshTranslate
 * Entity::doMeshTransform -> Entity::meshTranslate
 * Entity::doMeshTurn-> Entity::meshTurn
 * 
 * // Example:
 * scene::Entity* MyMeshObjectA = spSmngr->createModel(scene::ENTITY_TEAPOT);
 * scene::Entity* MyMeshObjectB = spSmngr->createModel(scene::ENTITY_TORUS, 15); // 15 is the count of segments, allowed because first parameter of "SModelConstruct" structure is the segmentation, all parameters have default values.
 * 
 * scene::SModelConstruct MyConstruct;
 * MyConstruct.Segments = 10; // Segmentation
 * MyConstruct.DegreeLength = 360.0f * 2; // two rotations for the spiral
 * MyConstruct.Size = 1.0f; // Distance between the ring of the spiral in one rotation
 * scene::Entity* MyMeshObjectA = spSmngr->createModel(scene::ENTITY_SPIRAL, MyConstruct);
 * // /Example
 * 
 * 
 * // === Changes for Animation === //
 * 
 * +scene::AnimationNode
 * +scene::AnimationMorphTarget
 * +scene::AnimationSkeletal
 * scene::Animation::doAnimate -> play (+pause, stop, setSpeed, getSeek etc.)
 * 
 * 
 * // === Changes for UserControl === //
 * 
 * UserControl::isKeyPressed -> keyDown
 * UserControl::isKeyReleased -> keyReleased
 * UserControl::isKeyHit -> keyHit
 * UserControl::isMouseButtonPressed -> mouseDown
 * UserControl::isMouseButtonReleased -> mouseReleased
 * UserControl::isMouseButtonHit -> mouseHit
 * \endcode
 */

/**
 * \page page_changes18_19 Changes from 1.8 to 1.9
 * 
 * \code
 * // === Function name changes (dim::matrix4) === //
 * matrixReset -> reset
 * matrixTranslate -> translate
 * matrixRotateX -> rotateX
 * matrixRotateY -> rotateY
 * matrixRotateZ -> rotateZ
 * matrixRotateYXZ -> rotateYXZ
 * matrixRotate -> rotate
 * getArray16 -> getArray
 * 
 * // === Function name changes (dim::vector3d) === //
 * isEqual -> equal
 * isEmpty -> empty
 * 
 * // === Function name changes (scene::Entity) === //
 * getBoundingBox -> getMeshBoundingBox
 * getBoundingSphere -> getMeshBoundingSphere
 * getMeshSurfaceCount -> getSurfaceCount
 * getMeshTriangleCount -> getTriangleCount
 * getMeshVerticesCount -> getVertexCount
 * getTriangleVertices -> getTriangleIndices
 * 
 * // When used before "updateModeling(false)" use now: "updateMeshBuffer"
 * // When used before "updateModeling" oder "updateModeling(true)" use now: "updateNormals"
 * updateModeling -> updateMeshBuffer
 * 
 * // === Function name changes (scene::SceneManager) === //
 * createJoint -> createNode
 * 
 * // === Function name changes (video::Texture) === //
 * setTransFilter -> setColorKey
 * setTransFilterAlpha -> setColorKeyAlpha
 * setTransFilterMask -> setColorKeyMask
 * 
 * // === Other === //
 * math::isEqual -> math::equal
 * video::SMeshMaterial::hShader -> video::SMeshMaterial::ShaderObject
 * \endcode
 */

/**
 * \page page_changes19_20 Changes from 1.9 to 2.0
 * 
 * \code
 * // === Class name changes === //
 * scene::Entity -> scene::Mesh
 * scene::Sprite -> scene::Billboard
 * 
 * // === Function name changes (scene::SceneManager) === //
 * createPivot -> createMesh
 * createModel -> createMesh
 * loadModel -> loadMesh
 * saveModel -> saveMesh
 * createSprite -> createBillboard
 * getEntityList -> getMeshList
 * getSpriteList -> getBillboardList
 * 
 * // === Function name changes (scene::Mesh) === //
 * selectSurface -> setActiveSurface
 * 
 * // === Enumeration entry name changes === //
 * NODE_ENTITY -> NODE_MESH
 * NODE_SPRITE -> NODE_BILLBOARD
 * 
 * ENTITY_CUBE -> MESH_CUBE
 * ENTITY_SPHERE -> MESH_SPHERE
 * ...
 * 
 * MODEL_SPM -> MESHFORMAT_SPM
 * MODEL_3DS -> MESHFORMAT_3DS
 * ...
 * 
 * SCENE_SPS -> SCENEFORMAT_SPS
 * SCENE_BSP -> SCENEFORMAT_BSP
 * \endcode
 */

/**
 * \page page_changes20_21 Changes from 2.0 to 2.1
 * 
 * \code
 * // === Class & structure name changes === //
 * ModelCreator -> BasicMeshGenerator
 * SMeshMaterial -> SMaterialStates
 * SHeightMapData -> SHeightMapTexture
 * 
 * // === Function name changes (scene::Mesh) === //
 * setShadingType -> setShading
 * flipMesh -> meshFlip
 * fitMesh -> meshFit
 *
 * // === Removed functions (scene::Mesh) === //
 * drawNormals
 * renderCelShading (can be solved using shaders)
 * setVertexCoordArray
 * getTriangleArray
 * getTriangleList
 * 
 * // === Function name changes (other) === //
 * video::color::flipColors -> setInverse (+getInverse)
 * 
 * // === Function design changes (video::VideoDriver) === //
 * loadShader() -> ...
 * 
 * // === Class inheritance changes === //
 * 
 * BasicObject                 BasicObject
 *  |_ scene::Node              |_ scene::Node
 *      |_ scene::Mesh              |_ scene::MaterialNode
 *      |_ scene::Billboard         |   |_ scene::Mesh
 *      |_ scene::Terrain   ->      |   |_ scene::Billboard
 *      |_ scene::Camera            |   |_ scene::Terrain
 *      |_ scene::Light             |_ scene::Camera
 *                                  |_ scene::Light
 * 
 * // === Enumeration entry name changes === //
 * SCENEFORMAT_BSP -> SCENEFORMAT_BSP1 and SCENEFORMAT_BSP3
 * 
 * // === Other changes === //
 * ESkyBoxTypes -> has been removed (was needed because TGA image files where not flipped vertical; this has been solved)
 * \endcode
 */

/**
 * \page page_changes21_30 Changes from 2.1 to 3.0
 * 
 * \code
 * // === Function name changes (video::Texture) === //
 * makeRenderTarget -> setRenderTarget / getRenderTarget
 * make1Dimensional -> setDimension
 * make2Dimensional -> setDimension
 * make3Dimensional -> setDimension
 * makeCubeMap -> setDimension
 * 
 * // === Class inheritance changes === //
 * 
 *                                   BasicObject
 *                                    |_ Node
 * BasicObject                            |_ scene::SceneNode (this is the new Node class)
 *  |_ scene::Node                            |_ scene::Camera
 *      |_ scene::MaterialNode                |_ scene::Light
 *      |   |_ scene::Mesh                    |_ scene::RenderNode
 *      |   |_ scene::Billboard  ->               |_ scene::MaterialNode
 *      |   |_ scene::Terrain                     |   |_ scene::Mesh
 *      |_ scene::Camera                          |   |_ scene::Billboard
 *      |_ scene::Light                           |   |_ scene::Terrain
 *                                                |_ scene::SceneGraph (this is the new SceneManager class)
 *                                                    |_ scene::SceneGraphSimple
 *                                                    |_ scene::SceneGraphSimpleChildTree
 * 
 * // === Function name changes (scene::Mesh) === //
 * setColor -> paint
 * 
 * // === Other changes === //
 * scene::SMeshSurfaceObject -> video::MeshBuffer
 * scene::Mesh::setVertexCounter -> video::MeshBuffer::setIndexOffset
 * scene::Mesh::addVertexCounter -> video::MeshBuffer::addIndexOffset
 * Mesh::setMeshReference -> Mesh::setReference
 * MeshBuffer::setSurfaceReference -> MeshBuffer::setReference
 * \endcode
 */

/**
 * \page page_changes30_31 Changes from 3.0 to 3.1
 * 
 * \code
 * // === Namespace name changes === //
 * sound -> audio (since the sound device has changed also the namespace name has been adjusted)
 * 
 * // === Function name changes (math::) === //
 * Round2 -> RoundPow2
 * SeedRandom -> math::Randomizer::seedRandom
 * Rnd -> math::Randomizer::randInt
 * Random -> math::Randomizer::randInt
 * 
 * createSoundDevice -> SoftPixelDevice::getSoundDevice
 * 
 * // === Other changes === //
 * video::Texture::getDimensionType -> video::Texture::getDimension
 * 
 * scene::SceneGraph::saveScene -> Has been removed, you can create and save scenes with the "SoftPixel Sandbox".
 * 
 * // === Macros === //
 * #define foreach BOOST_FOREACH -> if you don't want this macro define SP_DONT_DEFINE_BOOST_MACROS before including "SoftPixelEngine.hpp".
 * 
 * // === Enumeration entry name changes === //
 * SCENEFORMAT_SPS -> SCENEFORMAT_SPSB
 * \endcode
 */

/**
 * \page page_changes31_32 Changes from 3.1 to 3.2
 * 
 * \code
 * // === Function name changes === //
 * io::string::makeNumber -> io::string::number
 * 
 * SoftPixelDevice::getTime -> io::Timer::getTime
 * SoftPixelDevice::getFPS -> io::Timer::getFPS & io::Timer::getElapsedFrames
 * SoftPixelDevice::getSceneGraph -> SoftPixelDevice::createSceneGraph (+ SoftPixelDevice::deleteSceneGraph)
 * SoftPixelDevice::getSoundDevice -> SoftPixelDevice::createSoundDevice (+ SoftPixelDevice::deleteSoundDevice)
 * 
 * audio::Sound::looped -> audio::Sound::getLoop (+ setLoop)
 * 
 * scene::SCENEGRAPH_USERDEFINED -> scene::SCENEGRAPH_CUSTOM
 * scene::SCENEGRAPH_SIMPLE_CHILDTREE -> scene::SCENEGRAPH_TREE
 * 
 * dim::EMatrixProjectionTypes -> dim::EMatrixCoordinateSystmes
 * 
 * io::FileHDD -> io::FilePhysical
 * io::FileRAM -> io::FileVirtual
 * 
 * io::FILE_HDD -> io::FILE_PHYSICAL
 * io::FILE_RAM -> io::FILE_VIRTUAL
 * io::RESOURCE_HDD -> io::RESOURCE_PHYSICAL
 * 
 * BasicObject -> BaseObject
 * 
 * video::RenderSystem::setClippingPlane has been removed. Use the scene::Camera::setRange function instead.
 * 
 * video::RenderSystem::makeNormalMap -> tool::ImageModifier::bakeNormalMap
 * 
 * class TextureManipulator -> namespace ImageModifier
 * 
 * scene::SceneNode no longer inherits from BoundingVolume but has such an object. Get it with the "getBoundingVolume" function.
 * Now the default bounding volume is 0.5 as radius for sphere and box.
 * 
 * math::ViewFrustum -> scene::ViewFrustum
 * 
 * container4 class has been removed. Use vector4d instead.
 * 
 * SceneNode's collision functions has been removed (checkContact, getContact, getNextContact, getContactList) -> use CollisionNode class instead.
 * 
 * SceneNode's transformation has been ported to a separate class called "Transformation3D" which is a template. But the 32-bit-float typedef
 * is called "Transformation". More over returns the "getTransformation" function no longer a 4x4 matrix but such a transformation object.
 * Use "getTransformMatrix" instead to directly receive the transformation's matrix.
 * 
 * scene::Light::set/getLightingType -> set/getLightModel
 * 
 * dim::rect2d::isPointCollided -> dim::rect2d::overlap
 * dim::rect2d::isRectCollided -> dim::rect2d::overlap
 * 
 * video::Font::getID -> video::Font::getBufferRawData
 * 
 * io::InputController::keyDownEx has been removed -> use keyDown instaed which now also works with left/right shift and left/right control.
 * 
 * SoftPixelDevice::getUserCharList has been removed -> use io::InputControl::getEnteredWord instead
 * 
 * class BasicMeshGenerator -> namespace MeshGenerator
 * 
 * scene::CollisionDetector has been removed -> use scene::CollisionGraph instead
 * scene::Collision has been removed -> use the new collision system
 * SoftPixelDevice::getCollisionDetector -> removed
 * 
 * video::emptycolor -> video::color::empty
 * 
 * io::getHex -> io::getHexString (and io::getHexNumber added)
 * 
 * SoftPixelDevice::updateEvent -> SoftPixelDevice::updateEvents (because several events will be updated)
 * 
 * Mesh::optimizeMeshBuffers -> Mesh::mergeMeshBuffers
 * 
 * StencilManager -> removed (the deprecated stencil management has been removed completely)
 * SceneGraph::set/getStencilEffects -> removed
 * SceneGraph::getStencilManager -> removed
 * 
 * SceneGraph::removeTexture -> SceneManager::removeTexture
 * SceneGraph::createAnimation -> SceneManager::createAnimation
 * SceneGraph::clearAnimations -> SceneManager::clearAnimations
 * SceneGraph::updateAnimation -> SceneManager::updateAnimation
 * 
 * SceneGraph::finishRenderScene -> has become a static function
 * 
 * Mesh::getTriangleLink -> removed
 * 
 * Surface shader callback "ShaderSurfaceCallback" declaration changed from:
 * "void (ShaderClass* Table, const std::vector<SMeshSurfaceTexture>* TextureList)" to
 * "void (ShaderClass* Table, const std::vector<SMeshSurfaceTexture> &TextureList)"
 * 
 * scene::SceneGraph::getRenderList and all the other scene-node lists are no
 * longer from the type std::list but from the type std::vector for performance reasons.
 * 
 * scene::SceneNode is no longer using std::list for scene-children and animation lists,
 * but std::vector is sued for performance reasons.
 * 
 * tool::LightmapGenerator::setCallback -> ...setProgressCallback
 * tool::LightmapGenerator::clearLightmapObjects -> ...clearScene
 * tool::LightmapGenerator::getLightmapList -> ...getLightmapTextures
 * 
 * io::EByteTypes -> io::EMemorySizeTypes
 * io::SIZE_KB -> io::MEMORYSIZE_KB
 * io::SIZE_MB -> io::MEMORYSIZE_MB
 * io::SIZE_GB -> io::MEMORYSIZE_GB
 * 
 * dim::plane3d::getLeftPlane (also the other 5 functions: right, top, bottom, front, back) has changed internally (previous planes where wrong).
 * 
 * dim::matrix4::getMatrix3 -> get3x3 (+ get2x2)
 * 
 * Mesh::clipConcatenatedTriangles -> Mesh::seperateTriangles
 * MeshBuffer::clipConcatenatedTriangles -> MeshBuffer::seperateTriangles
 * MeshBuffer::clearTextureList -> MeshBuffer::clearTextureLayers
 * 
 * Mesh::getTextureList -> now returns an 'std::vector' not an 'std::list'
 * MeshBuffer::getTextureList -> now returns an 'std::vector' not an 'std::list'
 * 
 * "typedef boost::function<void (ShaderClass* Table, const std::vector<SMeshSurfaceTexture> &TextureList)> ShaderSurfaceCallback" changed to
 * "typedef boost::function<void (ShaderClass* Table, const std::vector<TextureLayer*> &TexLayers)> ShaderSurfaceCallback"
 * 
 * sp::spWindowCallback -> sp::SpWin32Callback
 * 
 * video::SMeshSurfaceTexture has been removed -> use the new texture-layer system instead (see TextureLayer class for more details).
 * 
 * tool::ModelCombiner -> tool::MeshBooleanOperator
 * 
 * "typedef boost::function<void (scene::Mesh* Obj, std::vector<MeshBuffer*>* LODSurfaceList, u32 LODIndex)> UserRenderCallback" changed to
 * "typedef boost::function<void (scene::Mesh* Obj, const std::vector<MeshBuffer*> &LODSurfaceList, u32 LODIndex)> UserRenderCallback"
 * 
 * video::DEF_SPLINE_DETAIL has been removed -> it's no longer used.
 * 
 * scene::ECollisionTypes has been removed -> use the new collision system instead.
 * scene::EPickingTypes has been removed -> use the new collision system instead.
 * 
 * scene::Camera::setProjectionMatrix has been removed -> see new Projection3D class.
 * scene::Camera::updatePerspective has been removed -> see new Projection3D class.
 * 
 * video::EMeshBufferUsage -> video::EHWBufferUsage
 * video::MESHBUFFER_STATIC -> video::HWBUFFER_STATIC
 * video::MESHBUFFER_DYNAMIC -> video::HWBUFFER_DYNAMIC
 * 
 * video::RenderSystem::drawPrimitiveList has been removed -> use drawMeshBuffer with none-temporary mesh buffers instead.
 * video::RenderSystem::setDefaultAlphaBlending -> setupDefaultBlending
 * video::RenderSystem::disableTriangleListStates has been removed -> lagecy code.
 * video::RenderSystem::disable3DRenderStates has been removed -> lagecy code.
 * video::RenderSystem::disableTexturing has been removed -> lagecy code.
 * video::RenderSystem::enableBlending has been removed -> lagecy code (use "setRenderState" instead).
 * video::RenderSystem::disableBlending has been removed -> lagecy code (use "setRenderState" instead).
 * video::RenderSystem::updateWireframeMode has been removed -> didn't do anything anytime.
 * video::RenderSystem::set/getSolidMode has been removed -> solid mode is always active for 2D drawing.
 * 
 * scene::StencilManager has been removed:
 *  -> the stencil shadow was never complete but deprecated. Use the deferred renderer and shadow mapper for real-time shadow effects.
 * 
 * video::RenderSystem::clearStencilBuffer has been removed
 * video::RenderSystem::drawStencilShadowVolume has been removed
 * video::RenderSystem::drawStencilShadow has been removed
 *  -> use setStencilMask, setStencilMethod, setStencilOperation and setClearStencil instead
 * 
 * video::RenderSystem::setRenderState (changed behaviour) -> see the documentation notes for information
 * 
 * // === Math macros removed === //
 * SIN, COS, TAN, ASIN, ACOS, ATAN
 * Use math::Sin, math::ASin etc. instead
 * 
 * // === Changes in the Texture class === //
 * Normally all image buffer operations (such as 'turnImage', 'setColorKey' etc.) has to be done
 * with the the "video::ImageBuffer" object. This can be get by the "Texture::getImageBuffer()" function.
 * After modifying the image buffer you have to call "Texture::updateImageBuffer".
 * But there are several functions which has been implemented in the Texture class directly such as
 * 'set-/getColorKey', 'set-/getSize', 'set-/getFormat'.
 * 
 * // === Changes in Animation === //
 * scene::AnimationNode -> scene::NodeAnimation
 * scene::AnimationMorphTarget -> scene::MorphTargetAnimation
 * scene::AnimationSkeletal -> scene::SkeletalAnimation
 * scene::Animation::animating -> scene::Animation::playing
 * 
 * // === Changes in SoftPixelDevice === //
 * All window managing functions (e.g. "setWindowPosition" etc.) has been moved to the render context class.
 * Since there can be more than one render context and thus more than one graphics window these functions
 * belong to the video::RenderContext class. And "getWindowHandle" has changed to "getWindowObject".
 * 
 * // === Changes in ShaderTable === //
 * Every function or class name with "ShaderTable" is now called "ShaderClass". There is no longer any
 * Name with "ShaderTable" because "table" is really the wrong name for it ;-)
 * \endcode
 */

/**
 * \page page_changes32_33 Changes from 3.2 to 3.3
 * 
 * \code
 * // === Function argument list changes === //
 * math::equal -> has no longer the last parameter "Tolerance".
 * 
 * // === Function name changes === //
 * dim::size2d::getClampedSize -> getScaledSize
 * math::SSplinePolynom::calc -> interpolate
 * dim::vector3d::set/getInvert -> set/getInverse
 * dim::vector3d::getAxisType -> getDominantAxis
 * video::Texture::setDimension -> setType
 * video::Texture::getDimension -> getType
 * video::Texture::setFilter -> setMinMagFilter (the new "setFilter" function takes one argument of the type "STextureFilter").
 * video::ShaderClass::link -> compile
 * video::RenderSystem::queryDrawCalls -> getNumDrawCalls
 * video::RenderSystem::queryMeshBufferBindings -> getNumMeshBufferBindings
 * video::RenderSystem::queryTextureLayerBindings -> getNumTextureLayerBindings
 * video::RenderSystem::queryMaterialUpdates -> getNumMaterialUpdates
 * 
 * // === Enumeration name changes === //
 * video::ETextureDimensions -> ETextureTypes
 * video::EShaderVersions::GLSL_VERSION_3_30_6 -> GLSL_VERSION_3_30
 * video::EShaderVersions::GLSL_VERSION_4_00_8 -> GLSL_VERSION_4_00
 * 
 * // === Class name changes === //
 * math::RadianDegreeAngle -> math::Angle
 * dim::point2d -> dim::vector2d (there are still the "point2df" and "point2di" type definitions and they will be used further more!)
 * tool::TokenParser -> tool::TokenScanner
 * 
 * // === Other changes === //
 * dim::vector4d is finally an own class and no longer inherits from vector3d! It still has the X, Y, Z and W components.
 * video::STextureCreationFlags::Dimension -> Type
 * video::STextureCreationFlags::MagFilter, MinFilter, MipMapFilter, MipMaps, Anisotropy, WrapMode -> use the 'Filter' struct member instead.
 * SDeviceFlags::isAntiAlias -> SDeviceFlags::AntiAliasing::Enabled
 * SDeviceFlags::MultiSamples -> SDeviceFlags::AntiAliasing::MultiSamples
 * video::RenderSystem::deleteShaderClass -> Now by default all the attached shaders will be deleted, too.
 * tool::LightmapGenerator::generateLightmaps -> completely new function header.
 * video::EDeferredRenderFlags -> video::EAdvancedRenderFlags
 * video::DEFERREDFLAG_... -> video::RENDERERFLAG_...
 * video::DeferredRenderer::renderScene (third parameter has been removed) -> use 'video::DeferredRenderer::setDefaultGBufferShader' instead.
 * video::EVideoFeatureQueries -> video::EVideoFeatureSupport
 * video::QUERY_... -> video::VIDEOSUPPORT_...
 * video::MeshBuffer::set/getHardwareInstancing -> Value type has changed from s32 to u32.
 * video::Direct3D9RenderSystem::getDirect3DDevice -> getD3DDevice
 * video::Direct3D11RenderSystem::getDirect3DDevice -> getD3DDevice
 * 
 * // === Changed for scene::Billboard objects === //
 * For 'scene::Billboard' objects lighting is now by default disabled.
 * scene::Billboard::set/getHardwareInstancing -> Value type has changed from s32 to u32.
 * scene::Billboard::set/getColor removed -> Use getMaterial()->set/getDiffuseColor instead.
 * scene::Billboard::set/getBasePosition -> Now it's a 3D vector.
 * 
 * // === Math funcitons upper case now lower case === //
 * math::Sgn -> sgn
 * math::Lerp -> lerp
 * math::LerpParabolic -> lerpParabolic
 * math::LerpSin -> lerpSin
 * math::Round -> round
 * math::RoundPow2 -> roundPow2
 * math::Equal -> equal
 * math::Pow2 -> pow2
 * math::Clamp -> clamp
 * math::Increase -> increase
 * math::Decrease -> decrease
 * math::ModularPow -> modularPow
 * math::Log -> logBase
 *
 * // === Removed === //
 * CRITICAL_SECTION macro has been removed because it's already used in the "windows.h" file -> Make your own mutex lock/unlock calls.
 * SoftPixelDevice::updateDeviceSettings -> Use RenderContext::setResolution/ setVsync etc. instead.
 * SoftPixelDeviceWin32::getDropFilename -> Use SoftPixelDevice::setDropFileCallback instead.
 * dim::vector3d::set/getRound -> were never used.
 * dim::vector3d::vector3d(size2d) constructor -> use "vector3d(Size.Width, Size.Height, 1)" instead.
 * dim::quaternion4::set -> Set the components manually instead.
 * dim::matrix[n]::multiplySingleMatrix -> use the matrix/vector multiplication operators instead.
 * dim::matrix4::getProjection -> was never used.
 * math::Swap -> use std::swap instead.
 * math::sortContainerConst -> use std::sort instead.
 * math::sortContainer -> use std::sort instead.
 * math::getVertexInterpolation -> was never used.
 * video::Texture::addAnimFrame, removeAnimFrame, setAnimation, setAnimFrame, getAnimation, getAnimFrameCount, getAnimFrameList -> use scene::TextureAnimation class instead.
 * video::RenderSystem::queryExtensionSupport -> only used internally of OpenGL render system.
 * video::ComputeShaderIO class has been removed -> Use video::ShaderResource class instead.
 * video::QUERY_VERTEX_SHADER_... has been removed -> Use video::VIDEOSUPPORT_HLSL_... instead.
 * video::QUERY_PIXEL_SHADER_... has been removed -> Use video::VIDEOSUPPORT_HLSL_... instead.
 * video::RenderSystem::setColorMatrix -> was never used.
 * scene::Camera::getProjection -> use scene::Camera::projectPoint instead.
 * scene::MaterialNode::set/getMaterialEnable -> Use video::RenderSystem::setGlobalMaterialState instead.
 * scene::SceneNode::getFirstAnimation -> use scene::SceneNode::getAnimation instead.
 * \endcode
 */

/**
 * \page page_howto1 How to create a cube-map
 * 
 * \code
 * // Load first cubemap face (+X).
 * video::Texture* CubeMap = spRenderer->loadTexture("CubeMapFace0.jpg");
 * 
 * // Load other 5 cubemap faces and add them at the end of the image buffer.
 * for (s32 i = 1; i < 6; ++i)
 *     CubeMap->addTextureFrame(spRenderer->loadTexture("CubeMapFace" + io::stringc(i) + ".jpg"), video::TEXTURE_FRAME_VERTICAL);
 * 
 * // Configure the cubemap texture.
 * CubeMap->setDimension(video::TEXTURE_CUBEMAP);
 * CubeMap->setWrapMode(video::TEXWRAP_CLAMP);
 * 
 * // Add the cubemap to the object and configure texture coordinate generation.
 * MeshObject->addTexture(CubeMap);
 * MeshObject->getMeshBuffer(0)->setMappingGen(0, video::MAPGEN_REFLECTION_MAP);
 * \endcode
 */

/**
 * \page page_howto2 How to create a dynamic cube-map
 * 
 * \code
 * // Create the texture with six times the height (because of the six cubemap faces).
 * const s32 CubeMapSize = 256;
 * video::Texture* CubeMap = spRenderer->createTexture(dim::size2di(CubeMapSize, CubeMapSize*6));
 * 
 * // Configure the cubemap texture and convert it to a render-target texture.
 * CubeMap->setDimension(video::TEXTURE_CUBEMAP);
 * CubeMap->setWrapMode(video::TEXWRAP_CLAMP);
 * CubeMap->setRenderTarget(true);
 * 
 * // Add the cubemap to the object and configure texture coordinate generation.
 * MeshObject->addTexture(CubeMap);
 * MeshObject->getMeshBuffer(0)->setMappingGen(0, video::MAPGEN_REFLECTION_MAP);
 * 
 * // ...
 * 
 * // Update the cubemap every frame (or only each second frame to save performance).
 * MeshObject->setVisible(false);
 * spRenderer->updateCubeMap(CubeMap, MeshObject->getPosition(true));
 * MeshObject->setVisible(true);
 * \endcode
 */

/**
 * \page page_howto3 How to create a tracking shot
 * 
 * \code
 * // Move with "tool::moveCameraFree" your camera where you want a node in your animation sequence and capture its location.
 * if (spControl->mouseHit(io::MOUSE_LEFT))
 * {
 *     const dim::vector3df Pos(Cam->getPosition(true));
 *     const dim::quaternion Rot(dim::quaternion(Cam->getRotationMatrix(true)).getInverse());
 *     
 *     // Store it in a file where you can copy and paste the code.
 *     File->writeStringN(
 *         "Anim->addSequence(dim::vector3df(" + io::stringc(Pos.X) + ", " + io::stringc(Pos.Y) + ", " + io::stringc(Pos.Z) +
 *         "), dim::quaternion(" + io::stringc(Rot.X) + ", " + io::stringc(Rot.Y) + ", " + io::stringc(Rot.Z) + ", " + io::stringc(Rot.W) + "));"
 *     );
 * }
 * 
 * // ...
 * 
 * // After creating your sequence copy and paste the generated code to create your animation and do some configurations.
 * Anim->setSplineTranslation(true);
 * Anim->setSpeed(0.01f);
 * \endcode
 */

/**
 * \page page_threading Multi-Threading with the SoftPixel Engine
 * 
 * Since version 3.2 the SoftPixel Engine was new designed to support multi-threading in some parts.
 * These are the currently supported features which can be used in combination with multi-threading:
 * 
 * Loading Textures:
 * \code
 * THREAD_PROC(MyThreadProc)
 * {
 *     // Activate shared render context
 *     MySharedContext->activate();
 *     
 *     // Load some textures:
 *     spRenderer->loadTexture(...);
 *     ...
 *     
 *     // Deactivate shared render context that the loaded
 *     // textures can be used in the main thread
 *     MySharedContext->deactivate();
 *     
 *     return 0;
 * }
 * 
 * ...
 * 
 * // Create the shared render context
 * MySharedContext = spContext->createSharedContext();
 * 
 * // Create and start my thread function
 * ThreadManager MyThread(MyThreadProc);
 * 
 * ...
 * \endcode
 */

//! Main namespace in which everything can be found.
namespace sp
{

//! Basic classes such as vectors, planes, matrix etc. can be found in this namespace.
namespace dim { }

//! Namespace for each GUI (Graphics User Interface) objects (e.g. Windows, Buttons etc.) for developing level editors or the like.
namespace gui { }

//! All simple text output and string class are in this namespace.
namespace io { }

//! This namespace contains all mathematical basic functions such as sine, cosine, distance calculations etc.
namespace math { }

//! This namespace contains all network functions such as connecting, sending and receiving messages.
namespace network { }

//! Namespace for physics simulations using NewtonGameDynamics library.
namespace physics { }

//! Add 3D object management functions and scene rendering can be found in this namespace.
namespace scene { }

//! All audio, sound and music operation functions can be found here. Also volumetric 3D sound effects are in this namespace.
namespace audio { }

//! This namespace contains all extendet tools for particular game development which should be used in a level editor.
namespace tool { }

//! All rendering and drawing operations for 2D and 3D can be found in this namespace.
namespace video { }

} // /namespace sp

/**
\defgroup group_animation Animation System
\defgroup group_collision Collision System
\defgroup group_audio Sound System
\defgroup group_network Network System
\defgroup group_gui Built in GUI
\defgroup group_gpgpu GPGPU
\defgroup group_shader Shaders
\defgroup group_script Scripting
\defgroup group_pathfinding Pathfinding
\defgroup group_scenegraph Scene Management
\defgroup group_texture Texture Mapping
\defgroup group_data_types Base Data Types
\defgroup group_arithmetic Vector and Matrix Arithmetic
*/

/**
\todo MeshBuffer -> store information when the buffer has been modified (addVertex, addTriangle etc.) and make the buffer un-renderable!
*/



// ================================================================================
