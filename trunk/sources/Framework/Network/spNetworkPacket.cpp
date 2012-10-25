/*
 * Network packet file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Framework/Network/spNetworkSystem.hpp"

#ifdef SP_COMPILE_WITH_NETWORKSYSTEM


namespace sp
{
namespace network
{


NetworkPacket::NetworkPacket(u32 BufferSize) :
    LogicalBufferSize_(BufferSize)
{
    createBuffer(BufferSize);
}
NetworkPacket::NetworkPacket(const EReservedDescriptors Descriptor) :
    LogicalBufferSize_(0)
{
    createBuffer(0);
    setDescriptor(static_cast<u32>(Descriptor));
}
NetworkPacket::NetworkPacket(
    const c8* Buffer, u32 BufferSize, bool isSetLowLevel) :
    LogicalBufferSize_(0)
{
    if (isSetLowLevel)
    {
        createBufferNonOffset(BufferSize);
        memcpy(Buffer_.get(), Buffer, BufferSize);
    }
    else
    {
        createBuffer(BufferSize);
        memcpy(getBuffer(), Buffer, BufferSize);
    }
}
NetworkPacket::NetworkPacket(const NetworkPacket &Other) :
    Buffer_             (Other.Buffer_              ),
    LogicalBufferSize_  (Other.LogicalBufferSize_   )
{
}
NetworkPacket::NetworkPacket(const io::stringc &String) :
    LogicalBufferSize_(0)
{
    createBuffer(String.size() + 1);
    memcpy(getBuffer(), String.c_str(), String.size() + 1);
}
NetworkPacket::~NetworkPacket()
{
}

io::stringc NetworkPacket::getString() const
{
    if (*(getBuffer() + LogicalBufferSize_ - 1) == '\0')
        return io::stringc(getBuffer());
    return "";
}
bool NetworkPacket::compareString(const io::stringc &Str) const
{
    if (LogicalBufferSize_ == Str.size() + 1)
        return memcmp(getBuffer(), Str.c_str(), LogicalBufferSize_) == 0;
    return false;
}

void NetworkPacket::setDescriptor(u32 Descriptor)
{
    if (Buffer_.get())
        memcpy(Buffer_.get(), &Descriptor, NetworkPacket::RESERVED_OFFSET);
}
u32 NetworkPacket::getDescriptor() const
{
    u32 Descriptor = 0;
    
    if (Buffer_.get())
        memcpy(&Descriptor, Buffer_.get(), NetworkPacket::RESERVED_OFFSET);
    
    return Descriptor;
}

void NetworkPacket::setReceiverAddress(u32 IPAddress)
{
    //todo
}
u32 NetworkPacket::getReceiverAddress() const
{
    return 0; //todo
}



/*
 * ======= Private: =======
 */

void NetworkPacket::createBuffer(u32 Size)
{
    LogicalBufferSize_ = Size;
    Buffer_ = boost::shared_array<c8>(new c8[Size + NetworkPacket::RESERVED_OFFSET]);
}
void NetworkPacket::createBufferNonOffset(u32 Size)
{
    LogicalBufferSize_ = Size - NetworkPacket::RESERVED_OFFSET;
    Buffer_ = boost::shared_array<c8>(new c8[Size]);
}


} // /namespace network

} // /namespace sp


#endif



// ================================================================================
