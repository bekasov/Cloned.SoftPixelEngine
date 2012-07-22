/*
 * Secure list header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_DIMENSION_SECURELIST_H__
#define __SP_DIMENSION_SECURELIST_H__


#include "Base/spStandard.hpp"
#include "Base/spCriticalSection.hpp"

#include <list>


namespace sp
{
namespace dim
{


//! Thread safe variant of the std::list class. This class is particular used for the NetworkSystem.
template < class T, class Allocator = std::allocator<T> > class SecureList : private std::list<T, Allocator>
{
    
    public:
        
        #define CALL_LOCKED(f) lock(); f; unlock();
        
        /* Typedefinitions */
        
        typedef typename std::list<T>       L;
        typedef typename L::iterator        iterator;
        typedef typename L::const_iterator  const_iterator;
        typedef typename L::reference       reference;
        typedef typename L::const_reference const_reference;
        typedef typename L::size_type       size_type;
        
        /* Functions */
        
        SecureList()
        {
        }
        ~SecureList()
        {
        }
        
        inline void lock()
        {
            Section_.lock();
        }
        inline void unlock()
        {
            Section_.unlock();
        }
        
        inline const_iterator begin() const
        {
            return L::begin();
        }
        inline const_iterator end() const
        {
            return L::end();
        }
        
        inline iterator begin()
        {
            return L::begin();
        }
        inline iterator end()
        {
            return L::end();
        }
        
        /*inline const_reference front() const
        {
            CALL_LOCKED(const_reference Result = L::front())
            return Result;
        }
        inline const_reference back() const
        {
            CALL_LOCKED(const_reference Result = L::back())
            return Result;
        }*/
        
        inline reference front()
        {
            CALL_LOCKED(reference Result = L::front())
            return Result;
        }
        inline reference back()
        {
            CALL_LOCKED(reference Result = L::back())
            return Result;
        }
        
        inline iterator erase(iterator it)
        {
            CALL_LOCKED(it = L::erase(it))
            return it;
        }
        
        inline void clear()
        {
            CALL_LOCKED(L::clear())
        }
        
        inline void push_back(const T &Element)
        {
            CALL_LOCKED(L::push_back(Element))
        }
        inline void push_front(const T &Element)
        {
            CALL_LOCKED(L::push_front(Element))
        }
        
        inline void pop_back()
        {
            CALL_LOCKED(L::pop_back())
        }
        inline void pop_front()
        {
            CALL_LOCKED(L::pop_front())
        }
        
        inline void remove(const T &Element)
        {
            CALL_LOCKED(L::remove(Element))
        }
        
        inline void sort()
        {
            CALL_LOCKED(L::sort())
        }
        inline void unique()
        {
            CALL_LOCKED(L::unique())
        }
        
        inline size_type size()// const
        {
            CALL_LOCKED(size_type Result = L::size())
            return Result;
        }
        inline bool empty()// const
        {
            CALL_LOCKED(bool Result = L::empty())
            return Result;
        }
        
        inline iterator insert(iterator it, const T &Element)
        {
            CALL_LOCKED(it = L::insert(it, Element))
            return it;
        }
        
        #undef CALL_LOCKED
        
    private:
        
        /* Members */
        
        CriticalSection Section_;
        
};


} // /namespace dim

} // /namespace sp


#endif



// ================================================================================
