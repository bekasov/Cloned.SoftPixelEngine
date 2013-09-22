/*
 * Network stream file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Framework/Network/spNetworkStream.hpp"

#ifdef SP_COMPILE_WITH_NETWORKSYSTEM


#include "Framework/Network/spNetworkBaseUDP.hpp"
#include "RenderSystem/spTextureBase.hpp"

#include <boost/make_shared.hpp>


namespace sp
{
namespace network
{


NetworkStream::NetworkStream()
{
}
NetworkStream::~NetworkStream()
{
}

void NetworkStream::push(const SNetworkStreamBlock &Block)
{
    if (Block.Buffer)
    {
        /* Push the buffer at the end of the stream inside the critical section */
        Mutex_.lock();
        Stream_.push_back(Block);
        Mutex_.unlock();
    }
}

bool NetworkStream::pop(SNetworkStreamBlock &Block)
{
    /* Lock critical section */
    Mutex_.lock();
    
    /* Check if stream is already empty */
    if (Stream_.emtpy())
    {
        /* Unlock critical section and return null pointer */
        Mutex_.unlock();
        return false;
    }
    
    /* Pop buffer at the front of the stream */
    Block = Stream_.front();
    Stream_.pop_front();
    
    /* Unlock critical section */
    Mutex_.unlock();
    
    return true;
}

bool NetworkStream::empty() const
{
    /* Check if the stream is empty inside the critical section */
    Mutex_.lock();
    bool Result = Stream_.empty();
    Mutex_.unlock();
    return Result;
}

NetworkStreamPtr NetworkStream::create(const void* RawBuffer, u32 BufferSize)
{
    if (!RawBuffer || !BufferSize)
        return NetworkStreamPtr();
    
    /* Create network stream */
    NetworkStreamPtr Stream = boost::make_shared<NetworkStream>();
    
    u32 BlockSize = getMaxBlockSize();
    const c8* Buffer = reinterpret_cast<const c8*>(RawBuffer);
    
    while (BufferSize > 0)
    {
        /* Create stream block buffer */
        if (BufferSize < BlockSize)
            BlockSize = BufferSize;
        
        SNetworkStreamBlock Block(BufferPtr(new c8[BlockSize]), BlockSize);
        memcpy(Block.Buffer.get(), Buffer, BlockSize);
        
        /* Add block buffer to stream */
        Stream.push(Block);
        
        /* Increment buffer pointer and reduce buffer size value */
        Buffer += BlockSize;
        BufferSize -= BlockSize;
    }
    
    return Stream;
}

NetworkStreamPtr create(const video::Texture* Tex)
{
    if (!Tex)
        return NetworkStreamPtr();
    
    /* Create network stream */
    NetworkStreamPtr Stream = boost::make_shared<NetworkStream>();
    
    u32 BlockSize = getMaxBlockSize();
    
    /* Add texture header block */
    STextureStreamHeader HeaderBlock;
    {
        HeaderBlock.setSize     (Tex->getSize           ());
        HeaderBlock.setType     (Tex->getType           ());
        HeaderBlock.setFormat   (Tex->getFormat         ());
        HeaderBlock.setHWFormat (Tex->getHardwareFormat ());
    }
    Stream.push(HeaderBlock);
    
    /* Add texture image buffer blocks */
    const video::ImageBuffer* ImgBuffer = Tex->getImageBuffer();
    
    //todo ...
    
    return Stream;
}

u32 NetworkStream::getMaxBlockSize()
{
    return NetworkBaseUDP::RECVBUFFER_SIZE;
}


} // /namespace network

} // /namespace sp


#endif



// ================================================================================
