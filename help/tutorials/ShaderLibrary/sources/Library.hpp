//
// SoftPixel Engine Tutorial: ShaderLibrary - (04/09/2010)
//

#include <SoftPixelEngine.hpp>

using namespace sp;

/* === Declarations === */

extern SoftPixelDevice* spDevice;
extern io::InputControl* spControl;
extern video::RenderSystem* spRenderer;
extern scene::SceneGraph* spScene;

extern const s32 ScrWidth, ScrHeight;

extern scene::Camera* Cam;
extern scene::Light* Light;

extern video::Font* TextFont;

extern video::color ClearColor;

extern const io::stringc BasePath;


/* === Classes === */

/**
 * Base shader example class.
 */
class ShaderExample
{
    
    public:
        
        ShaderExample();
        virtual ~ShaderExample();
        
        virtual bool Render();
        
        virtual bool Load() = 0;
        virtual void Update() = 0;
        virtual void Show(bool Enable) = 0;
        
        inline io::stringc GetDescription() const
        {
            return Description_;
        }
        
        template <class T> static T* Create()
        {
            T* NewExample = new T();
            
            if (!NewExample->Load())
            {
                delete NewExample;
                return 0;
            }
            
            return NewExample;
        }
        
    protected:
        
        bool LoadShader(
            const io::stringc &Name,
            const video::ShaderObjectCallback &CallbackProcOGL = 0,
            const video::ShaderObjectCallback &CallbackProcD3D9 = 0,
            bool IsVertexPixel = false, bool IsShaderModel3 = false
        );
        
        void Move(scene::SceneNode* Obj);
        void TurnCamera();
        void RotateCamera();
        void TurnLight();
        
        video::ShaderTable* ShdTable_;
        
        video::Shader* ShdVertex_;
        video::Shader* ShdPixel_;
        
        scene::Mesh* Object_;
        
        io::stringc Description_;
        dim::point2df Rotation_;
        
};

/**
 * Simple vertex manipulation: Normal, Spherify, Twist, Bloat.
 */
class SimpleVertexManipulation : public ShaderExample
{
    
    public:
        
        SimpleVertexManipulation();
        ~SimpleVertexManipulation();
        
        bool Load();
        void Update();
        void Show(bool Enable);
        
    private:
        
        enum EDemoModes
        {
            SVM_NORMAL = 0,
            SVM_SPHERIFY,
            SVM_TWIST,
            SVM_BLOAT,
        };
        
        s32 Mode_;
        f32 TransformFactor_;
        
        scene::Mesh* Object2_;
        
        f32 MaxHeight_, MinHeight_;
        
};

/**
 * Hardware accelerated alternativ to standard billboarding.
 */
class Billboarding : public ShaderExample
{
    
    public:
        
        Billboarding();
        ~Billboarding();
        
        bool Load();
        void Update();
        void Show(bool Enable);
        
    private:
        
        std::list<scene::Mesh*> ObjectList_;
        
        video::Texture* ParticleTex_[3];
        
};

/**
 * Rusting process.
 */
class RustProcess : public ShaderExample
{
    
    public:
        
        RustProcess();
        ~RustProcess();
        
        bool Load();
        void Update();
        void Show(bool Enable);
        
    private:
        
        video::Texture* MetalTex_;
        video::Texture* RustTex_;
        video::Texture* FactorTex_;
        
        bool TransitionForwards_;
        f32 TransitionFactor_;
        
};

/**
 * Realtime calculated procedural textures.
 */
class ProceduralTextures : public ShaderExample
{
    
    public:
        
        ProceduralTextures();
        ~ProceduralTextures();
        
        bool Load();
        void Update();
        void Show(bool Enable);
        
    private:
        
        enum EDemoModes
        {
            PTEX_SIMPLE = 0,
            PTEX_CHESSBOARD,
            PTEX_BRICKS,
            PTEX_CIRCLES,
        };
        
        s32 Mode_;
        
};

/**
 * Fur (or rather hair-) effect.
 */
class FurEffect : public ShaderExample
{
    
    public:
        
        FurEffect();
        ~FurEffect();
        
        bool Load();
        void Update();
        void Show(bool Enable);
        
    private:
        
        void MakeHairs(video::Texture* Tex);
        
        std::list<scene::Mesh*> SubObjectList_;
        
        video::Texture* SurfaceTex_;
        video::Texture* HairTex_;
        
};

/**
 * Cel (or rather toon-) shading.
 */
class CelShading : public ShaderExample
{
    
    public:
        
        CelShading();
        ~CelShading();
        
        bool Load();
        void Update();
        void Show(bool Enable);
        
    private:
        
        video::Texture* Light1DTex_;
        
        scene::Mesh* WireObject_;
        
};

/**
 * Depth of field (DOF) effect.
 */
class DepthOfField : public ShaderExample
{
    
    public:
        
        DepthOfField();
        ~DepthOfField();
        
        bool Render();
        
        bool Load();
        void Update();
        void Show(bool Enable);
        
    private:
        
        f32 DepthBias_;
        
        scene::Mesh* Object2_;
        scene::Mesh* Object3_;
        
        video::Texture* ScreenTex_;
        video::Texture* SurfaceTex_;
        
};

/**
 * Bump mapping: Per-pixel shading method.
 */
class BumpMapping : public ShaderExample
{
    
    public:
        
        BumpMapping();
        ~BumpMapping();
        
        bool Load();
        void Update();
        void Show(bool Enable);
        
    private:
        
        enum EDemoModes
        {
            BM_ROCKS = 0,
            BM_ROCKS_NOBUMPS,
            BM_STONE,
            BM_STONE_NOBUMPS,
        };
        
        s32 Mode_;
        
        video::Texture* ColorMap_[2];
        video::Texture* NormalMap_[2];
        video::Texture* TempTex_;
        video::Texture* FlareTex_;
        
        scene::Mesh* Object2_;
        scene::Billboard* LightObj_;
        
        video::color LightColor_;
        bool EnableBumps_;
        
};

/**
 * Water simulation with refractions and reflections.
 */
class WaterSimulation : public ShaderExample
{
    
    public:
        
        WaterSimulation();
        ~WaterSimulation();
        
        bool Render();
        
        bool Load();
        void Update();
        void Show(bool Enable);
        
    private:
        
        scene::Mesh* HeightField_;
        scene::Mesh* WaterPlane_;
        
        video::Texture* GrassTex_;
        video::Texture* HeightMapTex_;
        
        video::Texture* WaterColorMap_;
        video::Texture* WaterNormalMap_;
        
        video::Texture* RefractionMap_;
        video::Texture* ReflectionMap_;
        
};






// ============================================
