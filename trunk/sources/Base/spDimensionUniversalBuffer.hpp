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
#include <cstring>


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
        
        UniversalBuffer() :
            Stride_(1)
        {
        }
        UniversalBuffer(const UniversalBuffer &Other) :
            Stride_(Other.Stride_),
            Buffer_(Other.Buffer_)
        {
        }
        ~UniversalBuffer()
        {
        }
        
        //! Basic copy operator
        inline UniversalBuffer& operator = (const UniversalBuffer &Other)
        {
            Stride_ = Other.Stride_;
            Buffer_ = Other.Buffer_;
            return *this;
        }
        
        /**
        Sets the buffer's stride which represents the size of each element. The initial stride is 1.
        \param[in] Stride Specifies the new stride size (in bytes). This must be greater than 0.
        \return True if the stride could be set. Otherwise false.
        */
        inline bool setStride(size_t Stride)
        {
            if (Stride > 0)
            {
                Stride_ = Stride;
                return true;
            }
            return false;
        }
        inline size_t getStride() const
        {
            return Stride_;
        }
        
        /**
        Returns a pointer (or rather reference) to the specified structured element.
        \param[in] Index Element index. Byte size is Index * (Buffer's stride).
        \return Pointer to the typename casted element.
        \since Version 3.3
        */
        template <typename T> inline T* getRef(size_t Index)
        {
            if (Index * Stride_ + sizeof(T) <= Buffer_.size())
                return reinterpret_cast<T*>(&Buffer_[Index * Stride_]);
            return 0;
        }
        /**
        Returns a constant pointer (or rather constant reference) to the specified structured element.
        \param[in] Index Element index. Byte size is Index * (Buffer's stride).
        \return Constant pointer to the typename casted element.
        \since Version 3.3
        */
        template <typename T> inline const T* getRef(size_t Index) const
        {
            if (Index * Stride_ + sizeof(T) <= Buffer_.size())
                return reinterpret_cast<const T*>(&Buffer_[Index * Stride_]);
            return 0;
        }
        
        /**
        Sets the specified memory at the given position.
        \param[in] Offset Byte offset which specifies the position.
        \param[in] Value Memory which is to be compied to the buffer.
        */
        template <typename T> inline void set(size_t Offset, const T &Value)
        {
            if (Offset + sizeof(T) <= Buffer_.size())
                memcpy(&Buffer_[Offset], &Value, sizeof(T));
        }
        template <typename T> inline T get(size_t Offset) const
        {
            if (Offset + sizeof(T) <= Buffer_.size())
                return *((T*)&Buffer_[Offset]);
            return T(0);
        }
        
        /**
        Sets the specified memory at the given position.
        \param[in] Index Element index. Byte size is Index * (Buffer's stride).
        \param[in] Offset Byte offset added to the position which is specified by "Index".
        \param[in] Value Memory which is to be compied to the buffer.
        */
        template <typename T> inline void set(size_t Index, size_t Offset, const T &Value)
        {
            set<T>(Index * Stride_ + Offset, Value);
        }
        template <typename T> inline T get(size_t Index, size_t Offset) const
        {
            return get<T>(Index * Stride_ + Offset);
        }
        
        /**
        Sets the specified memory at the given position and clamps the size.
        \param[in] Index Element index. Byte size is Index * (Buffer's stride).
        \param[in] Ofset: Byte offset added to the position which is specified by "Index".
        \param[in] MaxSize Maximal memory size (in bytes) copied from "Value" to the buffer.
        \param[in] Value Memory which is to be compied to the buffer.
        */
        template <typename T> inline void set(size_t Index, size_t Offset, size_t MaxSize, const T &Value)
        {
            const size_t Size = (sizeof(T) > MaxSize ? MaxSize : sizeof(T));
            Offset += Index * Stride_;
            if (Offset + Size <= Buffer_.size())
                memcpy(&Buffer_[Offset], &Value, Size);
        }
        template <typename T> inline T get(size_t Index, size_t Offset, size_t MaxSize) const
        {
            const size_t Size = (sizeof(T) > MaxSize ? MaxSize : sizeof(T));
            Offset += Index * Stride_;
            if (Offset + Size <= Buffer_.size())
                return *((T*)&Buffer_[Offset]);
            return T(0);
        }
        
        /**
        Sets the specified memory at the given position.
        \param[in] Offset Byte offset which specifies the position.
        \param[in] Buffer Memory which is to be compied to the buffer.
        \param[in] Size Buffer size in bytes.
        */
        inline void setBuffer(size_t Offset, const void* Buffer, size_t Size)
        {
            if (Offset + Size <= Buffer_.size())
                memcpy(&Buffer_[Offset], Buffer, Size);
        }
        inline void getBuffer(size_t Offset, void* Buffer, size_t Size) const
        {
            if (Offset + Size <= Buffer_.size())
                memcpy(Buffer, &Buffer_[Offset], Size);
        }
        
        /**
        Copies the specified memory to the given position.
        \param[in] Index Element index. Byte size is Index * (Buffer's stride).
        \param[in] Offset Byte offset added to the position which is specified by "Index".
        \param[in] Buffer Memory which is to be compied to the buffer.
        \param[in] Size Buffer size in bytes.
        */
        inline void setBuffer(size_t Index, size_t Offset, const void* Buffer, size_t Size)
        {
            setBuffer(Index * Stride_ + Offset, Buffer, Size);
        }
        /**
        Copies the specified memory from the given position.
        \param[in] Index Element index. Byte size is Index * (Buffer's stride).
        \param[in] Offset Byte offset added to the position which is specified by "Index".
        \param[in,out] Buffer Memory which is to be compied to the buffer.
        \param[in] Size Buffer size in bytes.
        */
        inline void getBuffer(size_t Index, size_t Offset, void* Buffer, size_t Size) const
        {
            getBuffer(Index * Stride_ + Offset, Buffer, Size);
        }
        
        //! Adds the specified memory at the end of the array.
        template <typename T> inline void add(const T &Value)
        {
            const size_t Offset = Buffer_.size();
            Buffer_.resize(Buffer_.size() + sizeof(T));
            set(Offset, Value);
        }
        //! Removes memory in the specified range [Offset .. Offset + Size).
        template <typename T> inline void remove(size_t Index, size_t Offset)
        {
            const size_t FinalOffset = Index * Stride_ + Offset;
            Buffer_.erase(Buffer_.begin() + FinalOffset, Buffer_.end() + FinalOffset + sizeof(T));
        }
        
        //! Adds the given buffer to this buffer. This and the given buffer must have the same 'stride' value.
        inline void add(const UniversalBuffer &Other)
        {
            if (Stride_ == Other.Stride_)
            {
                const size_t PrevSize = Buffer_.size();
                Buffer_.resize(PrevSize + Other.Buffer_.size());
                memcpy(&Buffer_[PrevSize], &Other.Buffer_[0], Other.Buffer_.size());
            }
        }
        
        inline void removeBuffer(size_t Offset, size_t Size)
        {
            Buffer_.erase(Buffer_.begin() + Offset, Buffer_.begin() + Offset + Size);
        }
        inline void removeBuffer(size_t Index, size_t Offset, size_t Size)
        {
            const size_t FinalOffset = Index * Stride_ + Offset;
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
        inline s8* getArray(size_t Offset)
        {
            return Buffer_.size() ? &Buffer_[Offset] : 0;
        }
        //! Returns the buffer array at the specified position.
        inline const s8* getArray(size_t Offset) const
        {
            return Buffer_.size() ? &Buffer_[Offset] : 0;
        }
        
        //! Returns the buffer array at the specified position (index * stride + offset).
        inline s8* getArray(size_t Index, size_t Offset)
        {
            return Buffer_.size() ? &Buffer_[Index * Stride_ + Offset] : 0;
        }
        //! Returns the buffer array at the specified position (index * stride + offset).
        inline const s8* getArray(size_t Index, size_t Offset) const
        {
            return Buffer_.size() ? &Buffer_[Index * Stride_ + Offset] : 0;
        }
        
        //! Resizes the buffer (Size in Bytes).
        inline void setSize(size_t Size)
        {
            Buffer_.resize(Size);
        }
        
        //! Returns the buffer's size (Size in Bytes).
        inline size_t getSize() const
        {
            return Buffer_.size();
        }
        
        //! Resizes the buffer (Size in stride). This is equivalent to "setSize(Count * getStride())".
        inline void setCount(size_t Count)
        {
            Buffer_.resize(Count * Stride_);
        }
        
        //! Returns the count of elements in the buffer. This is equivalent to "getSize() / getStride()".
        inline size_t getCount() const
        {
            return Buffer_.size() / Stride_;
        }
        
        /**
        Fills the buffer with 0's.
        \param[in] Offset Specifies the offset in bytes.
        \param[in] Size Specifies the size in bytes.
        */
        inline void fill(size_t Offset, size_t Size)
        {
            if (Buffer_.size() >= Offset + Size)
                memset(&Buffer_[Offset], 0, Size);
        }
        
        //! Clears the whole buffer.
        inline void clear()
        {
            Buffer_.clear();
        }
        
        //! Returns true if this universal buffer is empty.
        inline bool empty() const
        {
            return Buffer_.empty();
        }
        
        //! Returns a reference to the actual container object (std::vector<s8>).
        inline std::vector<s8>& getContainer()
        {
            return Buffer_;
        }
        //! Returns a constant reference to the actual container object (std::vector<s8>).
        inline const std::vector<s8>& getContainer() const
        {
            return Buffer_;
        }
        
    private:
        
        /* Members */
        
        size_t Stride_; // 1 (byte), 2 (short), 4 (int) etc.
        std::vector<s8> Buffer_;
        
};


} // /namespace dim

} // /namespace sp


#endif



// ================================================================================
