/*
 * Light grid header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_LIGHT_GRID_H__
#define __SP_LIGHT_GRID_H__


#include "Base/spStandard.hpp"

#if defined(SP_COMPILE_WITH_DEFERREDRENDERER)


#include "RenderSystem/spTextureBase.hpp"


namespace sp
{

namespace scene { class Light; }

namespace video
{


/**
The light grid is used by the deferred renderer for tiled shading.
\since Version 3.3
*/
class SP_EXPORT LightGrid
{
    
    public:
        
        LightGrid();
        ~LightGrid();
        
        /* === Functions === */
        
        /**
        Creates the light grid.
        \param[in] Resolution Specifies the resolution. This should be the same as specified for the engine's graphics device.
        \param[in] GridSize Specifies the grid size or rather the count of tiles on the X and Y axes.
        \return True if the grid could be created successful.
        */
        bool createGrid(const dim::size2di &Resolution, const dim::size2di &GridSize);
        void deleteGrid();
        
        /**
        Builds the light grid by filling the TLI buffer texture with indices of each light,
        which intersects the respective grid tiles.
        \see fillLightIntoGrid
        */
        template <typename T> void buildGrid(T itBegin, T itEnd)
        {
            for (typename T it = itBegin; it != itEnd; ++it)
                fillLightIntoGrid(*it);
        }
        
        /**
        Fills the given light source into the grid.
        \see buildGrid
        */
        void fillLightIntoGrid(scene::Light* Obj);
        
        s32 bind(s32 TexLayerBase);
        s32 unbind(s32 TexLayerBase);

        /* === Inline functions === */
        
        //! Returns the TLI (Tile Light Index List) texture object. This is an signed integer texture buffer.
        inline Texture* getTLITexture() const
        {
            return TLITexture_;
        }
        
    private:
        
        /* === Functions === */
        

        
        /* === Members === */
        
        Texture* TLITexture_;   //!< This is a texture buffer storing the light indcies.
        
};


} // /namespace video

} // /namespace sp


#endif

#endif



// ================================================================================
