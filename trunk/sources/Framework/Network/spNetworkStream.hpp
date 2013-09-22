/*
 * Network stream header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_NETWORK_STREAM_H__
#define __SP_NETWORK_STREAM_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_NETWORKSYSTEM


#include "Base/spCriticalSection.hpp"
#include "RenderSystem/spTextureFlags.hpp"
#include "FileFormats/Image/spImageFormatInterfaces.hpp"

#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>


namespace sp
{
namespace video
{
    class Texture;
}
namespace network
{



class NetworkStream;

//! Shared array pointer to raw data.
typedef boost::shared_array<c8> BufferPtr;
//! Shared pointer to NetworkStream class.
typedef boost::shared_ptr<NetworkStream> NetworkStreamPtr;


struct SNetworkStreamBlock
{
    SNetworkStreamBlock() :
        Size(0)
    {
    }
    SNetworkStreamBlock(BufferPtr InitBuffer, u32 InitSize) :
        Size    (InitSize   ),
        Buffer  (InitBuffer )
    {
    }
    template <typename T> SNetworkStreamBlock(const T &Data) :
        Size    (T::getBlockSize()  ),
        Buffer  (new c8[Size]       )
    {
        memcpy(Buffer.get(), Data.getBuffer(), Size);
    }
    ~SNetworkStreamBlock()
    {
    }
    
    /* Members */
    u32 Size;
    BufferPtr Buffer;
};


//! Namespace for all pre-defined network stream block structures.
namespace StreamBlocks
{

#define DefineSetterAndGetter(n, t, r)  \
    inline void set##n(const t& Set##n) \
    {                                   \
        n = static_cast<r>(Set##n);     \
    }                                   \
    inline t get##n() const             \
    {                                   \
        return static_cast<t>(n);       \
    }

struct STextureStreamHeader
{
    STextureStreamHeader() :
        Type    (0),
        Format  (0),
        HWFormat(0)
    {
    }
    ~STextureStreamHeader()
    {
    }
    
    /* Inline functions */
    DefineSetterAndGetter(Size,         dim::size2di,               dim::size2di)
    DefineSetterAndGetter(Type,         video::ETextureTypes,       u8          )
    DefineSetterAndGetter(Format,       video::EPixelFormats,       u8          )
    DefineSetterAndGetter(HWFormat,     video::EHWTextureFormats,   u8          )
    DefineSetterAndGetter(BufferType,   video::EImageBufferTypes,   u8          )
    
    inline const c8* getBuffer() const
    {
        return reinterpret_cast<const c8*>(&Size.Width);
    }
    
    inline static u32 getBlockSize()
    {
        return 11;
    }
    
    /* Members */
    dim::size2di Size;
    u8 Type;
    u8 Format;
    u8 HWFormat;
    u8 BufferType;
};

#undef DefineSetterAndGetter

} // /namespace StreamBlocks


/**
NetworkStream is used to stream very large network packets over a UDP/IP network.
\ingroup group_network
\since Version 3.3
\todo Currently unused and incomplete.
*/
class SP_EXPORT NetworkStream
{
    
    public:
        
        NetworkStream();
        ~NetworkStream();
        
        /* === Functions === */
        
        /**
        Pushes the specified buffer at the end of the stream.
        \param[in] Block Specifies the stream block which is to be pushed to the stream.
        \note This can be used for multi-threading.
        \see SNetworkStreamBlock
        */
        void push(const SNetworkStreamBlock &Block);
        /**
        Pops the buffer from the 
        \param[out] Block Resulting output stream block.
        \return True if a buffer could be poped. Otherwise the stream is empty.
        \note This can be used for multi-threading.
        \see SNetworkStreamBlock
        */
        bool pop(SNetworkStreamBlock &Block);
        
        //! Returns true if the stream is empty.
        bool empty() const;
        
        /* === Static functions === */
        
        static NetworkStreamPtr create(const void* RawBuffer, u32 BufferSize);
        static NetworkStreamPtr create(const video::Texture* Tex);
        //static NetworkStreamPtr create(const audio::SAudioBuffer &AudioBuffer);
        
        static u32 getMaxBlockSize();
        
    private:
        
        /* === Members === */
        
        std::list<SNetworkStreamBlock> Stream_;
        CriticalSection Mutex_;
        
};


} // /namespace network

} // /namespace sp


#endif

#endif



// ================================================================================
