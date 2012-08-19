/*
 * Universal buffer header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_DIMENSION_UNIVERSALBUFFER_H__
#define __SP_DIMENSION_UNIVERSALBUFFER_H__


#include "Base/spStandard.hpp"

#include <vector>


namespace sp
{
namespace dim
{


/**
Universal buffer class to handle any kind of data in a large container. This is primary used for mesh buffers
to handle blocks of data (e.g. vertices) easier. Also format conversions (e.g. video::MeshBuffer::setVertexFormat)
can be handled easier instead of just using an std::vector. Note that this is an array container. Thus adding new data can be slow.
*/
class UniversalBuffer
{
    
    public:
        
        UniversalBuffer() : Stride_(1)
        {
        }
        ~UniversalBuffer()
        {
        }
        
        //! Basic copy operator
        inline UniversalBuffer& operator = (const UniversalBuffer &other)
        {
            Stride_ = other.Stride_;
            Buffer_ = other.Buffer_;
            return *this;
        }
        
        //! Sets the buffer's stride which represents the size of each element. The initial stride is 1.
        inline void setStride(u32 Stride)
        {
            Stride_ = Stride;
        }
        inline u32 getStride() const
        {
            return Stride_;
        }
        
        /**
        Sets the specified memory at the given position.
        \param Offset: Byte offset which specifies the position.
        \param Value: Memory which is to be compied to the buffer.
        */
        template <typename T> inline void set(u32 Offset, const T &Value)
        {
            if (Offset + sizeof(T) <= Buffer_.size())
                memcpy(&Buffer_[Offset], &Value, sizeof(T));
        }
        template <typename T> inline T get(u32 Offset) const
        {
            if (Offset + sizeof(T) <= Buffer_.size())
                return *((T*)&Buffer_[Offset]);
            return T(0);
        }
        
        /**
        Sets the specified memory at the given position.
        \param Index: Element index. Byte size is Index * (Buffer's stride).
        \param Offset: Byte offset added to the position which is specified by "Index".
        \param Value: Memory which is to be compied to the buffer.
        */
        template <typename T> inline void set(u32 Index, u32 Offset, const T &Value)
        {
            set<T>(Index * Stride_ + Offset, Value);
        }
        template <typename T> inline T get(u32 Index, u32 Offset) const
        {
            return get<T>(Index * Stride_ + Offset);
        }
        
        /**
        Sets the specified memory at the given position and clamps the size.
        \param Index: Element index. Byte size is Index * (Buffer's stride).
        \param Offset: Byte offset added to the position which is specified by "Index".
        \param MaxSize: Maximal memory size (in bytes) copied from "Value" to the buffer.
        \param Value: Memory which is to be compied to the buffer.
        */
        template <typename T> inline void set(u32 Index, u32 Offset, u32 MaxSize, const T &Value)
        {
            const u32 Size = (sizeof(T) > MaxSize ? MaxSize : sizeof(T));
            Offset += Index * Stride_;
            if (Offset + Size <= Buffer_.size())
                memcpy(&Buffer_[Offset], &Value, Size);
        }
        template <typename T> inline T get(u32 Index, u32 Offset, u32 MaxSize) const
        {
            const u32 Size = (sizeof(T) > MaxSize ? MaxSize : sizeof(T));
            Offset += Index * Stride_;
            if (Offset + Size <= Buffer_.size())
                return *((T*)&Buffer_[Offset]);
            return T(0);
        }
        
        /**
        Sets the specified memory at the given position.
        \param Offset: Byte offset which specifies the position.
        \param Buffer: Memory which is to be compied to the buffer.
        \param Size: Buffer size in bytes.
        */
        inline void setBuffer(u32 Offset, const void* Buffer, u32 Size)
        {
            if (Offset + Size <= Buffer_.size())
                memcpy(&Buffer_[Offset], Buffer, Size);
        }
        inline void getBuffer(u32 Offset, void* Buffer, u32 Size) const
        {
            if (Offset + Size <= Buffer_.size())
                memcpy(Buffer, &Buffer_[Offset], Size);
        }
        
        /**
        Sets the specified memory at the given position.
        \param Index: Element index. Byte size is Index * (Buffer's stride).
        \param Offset: Byte offset added to the position which is specified by "Index".
        \param Buffer: Memory which is to be compied to the buffer.
        \param Size: Buffer size in bytes.
        */
        inline void setBuffer(u32 Index, u32 Offset, const void* Buffer, u32 Size)
        {
            setBuffer(Index * Stride_ + Offset, Buffer, Size);
        }
        inline void getBuffer(u32 Index, u32 Offset, void* Buffer, u32 Size) const
        {
            getBuffer(Index * Stride_ + Offset, Buffer, Size);
        }
        
        //! Adds the specified memory at the end of the array.
        template <typename T> inline void add(const T &Value)
        {
            const u32 Offset = Buffer_.size();
            Buffer_.resize(Buffer_.size() + sizeof(T));
            set(Offset, Value);
        }
        //! Removes memory in the specified range [Offset .. Offset + Size).
        template <typename T> inline void remove(u32 Index, u32 Offset)
        {
            const u32 FinalOffset = Index * Stride_ + Offset;
            Buffer_.erase(Buffer_.begin() + FinalOffset, Buffer_.end() + FinalOffset + sizeof(T));
        }
        
        inline void removeBuffer(u32 Offset, u32 Size)
        {
            Buffer_.erase(Buffer_.begin() + Offset, Buffer_.begin() + Offset + Size);
        }
        inline void removeBuffer(u32 Index, u32 Offset, u32 Size)
        {
            const u32 FinalOffset = Index * Stride_ + Offset;
            Buffer_.erase(Buffer_.begin() + FinalOffset, Buffer_.begin() + FinalOffset + Size);
        }
        
        //! Returns the buffer array. This points to the first element in the array.
        inline s8* getArray()
        {
            return Buffer_.size() ? &Buffer_[0] : 0;
        }
        //! Returns the buffer array. This points to the first element in the array.
        inline const s8* getArray() const
        {
            return Buffer_.size() ? &Buffer_[0] : 0;
        }
        
        //! Returns the buffer array at the specified position.
        inline s8* getArray(u32 Offset)
        {
            return Buffer_.size() ? &Buffer_[Offset] : 0;
        }
        //! Returns the buffer array at the specified position.
        inline const s8* getArray(u32 Offset) const
        {
            return Buffer_.size() ? &Buffer_[Offset] : 0;
        }
        
        //! Returns the buffer array at the specified position (index * stride + offset).
        inline s8* getArray(u32 Index, u32 Offset)
        {
            return Buffer_.size() ? &Buffer_[Index * Stride_ + Offset] : 0;
        }
        //! Returns the buffer array at the specified position (index * stride + offset).
        inline const s8* getArray(u32 Index, u32 Offset) const
        {
            return Buffer_.size() ? &Buffer_[Index * Stride_ + Offset] : 0;
        }
        
        //! Resizes the buffer (Size in Bytes).
        inline void setSize(u32 Size)
        {
            Buffer_.resize(Size);
        }
        
        //! Returns the buffer's size (Size in Bytes).
        inline u32 getSize() const
        {
            return Buffer_.size();
        }
        
        //! Resizes the buffer (Size in stride). This is equivalent to "setSize(Count * getStride())".
        inline void setCount(u32 Count)
        {
            Buffer_.resize(Count * Stride_);
        }
        
        //! Returns the count of elements in the buffer. This is equivalent to "getSize() / getStride()".
        inline u32 getCount() const
        {
            return Buffer_.size() / Stride_;
        }
        
        /**
        Fills the buffer with 0's.
        \param Offset: Specifies the offset in bytes.
        \param Size: Specifies the size in bytes.
        */
        inline void fill(u32 Offset, u32 Size)
        {
            if (Buffer_.size() >= Offset + Size)
                memset(&Buffer_[Offset], 0, Size);
        }
        
        //! Clears the whole buffer.
        inline void clear()
        {
            Buffer_.clear();
        }
        
    private:
        
        /* Members */
        
        u32 Stride_; // 1 (byte), 2 (short), 4 (int) etc.
        std::vector<s8> Buffer_;
        
};


} // /namespace dim

} // /namespace sp


#endif



// ================================================================================
