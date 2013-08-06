/*
 * Movie header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_VIDEODRIVER_MOVIE_H__
#define __SP_VIDEODRIVER_MOVIE_H__


#include "Base/spStandard.hpp"
#include "Base/spDimensionSize2D.hpp"
#include "Base/spInputOutputString.hpp"
#include "Base/spMemoryManagement.hpp"
#include "RenderSystem/spTextureBase.hpp"


namespace sp
{
namespace video
{


/**
Movie class. Only AVI video files are supported yet.
\deprecated This is out of date. This class should be completely redesigned.
*/
class SP_EXPORT Movie
{
    
    public:
        
        Movie(const io::stringc &Filename, const s32 Resolution = 256);
        ~Movie();
        
        /* Basics */
        
        bool reload(const io::stringc &Filename, const s32 Resolution = 256);
        void close();
        
        void play(bool Looped = false);
        void pause(bool Paused = true);
        void stop();
        
        /* Effects */
        
        void setSeek(f32 Seek);
        f32 getSeek() const;
        
        void setSpeed(f32 Speed);
        f32 getSpeed() const;
        
        /* Status */
        
        dim::size2di getSize() const;
        
        u32 getLength() const;
        
        bool finish() const;
        bool valid() const;
        
        //! Renders the current movie frame to the specified texture.
        void renderTexture(Texture* Tex);
        
        /* Inline functions */
        
        inline io::stringc getFilename() const
        {
            return Filename_;
        }
        
    private:
        
        /* Enumerations */
        
        enum EMovieStateTypes
        {
            MOVIESTATE_STOPED = 0,
            MOVIESTATE_PLAYED,
            MOVIESTATE_PAUSED
        };
        
        /* Functions */
        
        void flipDataBuffer(void* buffer, u32 Len);
        
        bool exitWithError(const io::stringc &ErrorMessage);
        
        /* Members */
        
        void* DataPointer_;
        io::stringc Filename_;
        f32 Speed_;
        
};


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
