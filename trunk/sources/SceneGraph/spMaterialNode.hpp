/*
 * Material node header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_SCENE_MATERIALNODE_H__
#define __SP_SCENE_MATERIALNODE_H__


#include "Base/spStandard.hpp"
#include "Base/spInputOutputString.hpp"
#include "Base/spMaterialStates.hpp"
#include "SceneGraph/spRenderNode.hpp"


namespace sp
{
namespace scene
{


/**
MaterialNode objects are the root of Mesh, Billboard and Terrain objects. The main content of these objects
is the MaterialStates member with all its interface functions. It also has a shader object.
*/
class SP_EXPORT MaterialNode : public RenderNode
{
    
    public:
        
        virtual ~MaterialNode();
        
        /* === Functions === */
        
        virtual void render() = 0;
        
        virtual bool compare(const MaterialNode* Other) const;
        
        /**
        Sets the mesh's material. In materials many configurations are stored.
        One of these are Diffuse or Ambient colors, ZBuffer- or Lighitng enabling and a Shader object.
        The most of these things can be set by special functions but you have more controll of the
        material by setting it directly. The material affects the whole mesh (each surface).
        If you want a mesh with multiple materials you have to created multiple Mesh object and one of them
        has to be the parent object of the others (use "setParent").
        \param[in] Material Pointer to a MaterialStates object which holds the whole material's data.
        */
        void setMaterial(const video::MaterialStates* Material);
        
        //! Returns a pointer to the video::MaterialStates object.
        inline video::MaterialStates* getMaterial()
        {
            return &Material_;
        }
        //! Returns a constant pointer to the video::MaterialStates object.
        inline const video::MaterialStates* getMaterial() const
        {
            return &Material_;
        }
        
        /**
        Sets the shader object. Shader can be used for high performance graphics effects.
        e.g. water reflection and refraction (more about shaders in the Shader class).
        \param ShaderObject: Pointer to a Shader object.
        */
        inline void setShader(video::Shader* ShaderObject)
        {
            ShaderClass_ = (ShaderObject ? ShaderObject->getShaderClass() : 0);
        }
        
        //! Sets the shader class directly.
        inline void setShaderClass(video::ShaderClass* ShaderObject)
        {
            ShaderClass_ = ShaderObject;
        }
        //! Returns pointer to the ShaderClass object.
        inline video::ShaderClass* getShaderClass() const
        {
            return ShaderClass_;
        }
        
    protected:
        
        friend class SceneGraph;
        friend bool cmpMeshesDefault(Mesh* &obj1, Mesh* &obj2);
        friend bool cmpMeshesFast(Mesh* &obj1, Mesh* &obj2);
        
        /* Functions */
        
        MaterialNode(const ENodeTypes Type);
        
        /* Members */
        
        video::MaterialStates Material_;
        video::ShaderClass* ShaderClass_;
        
};


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
