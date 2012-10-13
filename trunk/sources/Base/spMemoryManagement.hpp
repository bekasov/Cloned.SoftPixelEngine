/*
 * Basic memory management header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_MEMORY_MANAGEMENT_H__
#define __SP_MEMORY_MANAGEMENT_H__


#include "Base/spInputOutputLog.hpp"

#include <exception>
#include <vector>
#include <list>
#include <stdarg.h>


namespace sp
{


//! MemeoryManager namespace was particular desinged for safe memory allocation and destruction.
namespace MemoryManager
{

/* === Delete memory === */

//! Deletes the specified memory buffer savely. This function don't 
template <typename T> void deleteMemory(T* &Buffer)
{
    delete Buffer;
    Buffer = 0;
}

//! Deletes the specified memory buffer (array) savely.
template <typename T> void deleteBuffer(T* &Buffer)
{
    delete [] Buffer;
    Buffer = 0;
}

/* === Allocate memory === */

//! Allocates a new memory buffer and prints an error message if not enough memory is available.
template <typename T> T* createMemory(
    const io::stringc &Description = "Unknown") throw (std::bad_alloc)
{
    try
    {
        return new T();
    }
    catch (std::bad_alloc &e)
    {
        io::Log::error("< Bad Allocation > exception thrown for \"" + Description + "\" memory");
        throw e;
    }
    
    return 0;
}

//! Allocates a new memory buffer (array) and prints an error message if not enough memory is available.
template <typename T> T* createBuffer(
    const s32 Count, const io::stringc &Description = "Unknown") throw (std::bad_alloc)
{
    try
    {
        return new T[Count];
    }
    catch (std::bad_alloc &e)
    {
        io::Log::error("< Bad Allocation > exception thrown for \"" + Description + "\" buffer");
        throw e;
    }
    
    return 0;
}

/* === Remove element from list === */

/**
Removes an element from the list.
\param[in,out] List Specifies the list (std::list or std::vector) from which the element is to be removed.
\param[in,out] Object Specifies the object (or rather element) which is to be removed.
\param[in] DeleteMemory Specifies whether the object is to be deleted or not.
\param[in] UniqueObject Specifies whether several objects can be removed or only one.
\return True if the specified element could be removed from the list.
*/
template < class T, template < class TL, class Allocator = std::allocator<TL> > class L > bool removeElement(
    L<T*> &List, T* &Object, bool DeleteMemory = false, bool UniqueObject = true)
{
    if (!Object)
        return false;
    
    bool Result = false;
    
    for (typename L< T*, std::allocator<T*> >::iterator it = List.begin(); it != List.end();)
    {
        if (*it == Object)
        {
            Result = true;
            
            if (DeleteMemory)
                MemoryManager::deleteMemory(Object);
            
            it = List.erase(it);
            
            if (UniqueObject)
                break;
        }
        else
            ++it;
    }
    
    return Result;
}

template <class T> bool removeElement(
    std::vector<T*> &List, T* &Object, bool DeleteMemory = false, bool UniqueObject = true)
{
    return removeElement<T, std::vector>(List, Object, DeleteMemory, UniqueObject);
}

template <class T> bool removeElement(
    std::list<T*> &List, T* &Object, bool DeleteMemory = false, bool UniqueObject = true)
{
    return removeElement<T, std::list>(List, Object, DeleteMemory, UniqueObject);
}

/* === Access elements === */

template <class T> T getElement(std::list<T> &List, u32 Index)
{
    typename std::list< T, std::allocator<T> >::iterator it = List.begin();
    
    for (; it != List.end() && Index > 0; ++it)
        --Index;
    
    return *it;
}

template <class T> const T getElement(const std::list<T> &List, u32 Index)
{
    typename std::list< T, std::allocator<T> >::const_iterator it = List.begin();
    
    for (; it != List.end() && Index > 0; ++it)
        --Index;
    
    return *it;
}

/* === Check for contents === */

template < class T, template < class TL, class Allocator = std::allocator<TL> > class L > bool hasElement(
    const L<T> &List, const T &Object)
{
    for (typename L< T, std::allocator<T> >::const_iterator it = List.begin(); it != List.end(); ++it)
    {
        if (*it == Object)
            return true;
    }
    return false;
}

template <class T> bool hasElement(const std::vector<T> &List, const T &Object)
{
    return hasElement<T, std::vector>(List, Object);
}

template <class T> bool hasElement(const std::list<T> &List, const T &Object)
{
    return hasElement<T, std::list>(List, Object);
}

/* === Clear list === */

/**
Deletes all elements of a list.
\param List: Specifies the list (std::list or std::vector) from which each elements are to be deleted.
*/
template < class T, template < class TL, class Allocator = std::allocator<TL> > class L > void deleteList(L<T*> &List)
{
    for (typename L< T*, std::allocator<T*> >::iterator it = List.begin(); it != List.end(); ++it)
        MemoryManager::deleteMemory(*it);
    List.clear();
}

template <class T> void deleteList(std::vector<T*> &List)
{
    deleteList<T, std::vector>(List);
}

template <class T> void deleteList(std::list<T*> &List)
{
    deleteList<T, std::list>(List);
}

} // /namespace MemoryManager


} // /namespace sp


#endif



// ================================================================================
