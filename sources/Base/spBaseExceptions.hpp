/*
 * Base exceptions header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_BASE_EXCEPTIONS_H__
#define __SP_BASE_EXCEPTIONS_H__


#include "Base/spStandard.hpp"
#include "Base/spInputOutputString.hpp"


namespace sp
{
namespace io
{


//! Default exception holding a message describing the error.
class SP_EXPORT DefaultException : public std::exception
{
    
    public:
        
        DefaultException(const io::stringc &Message) throw() :
            Message_(Message)
        {
        }
        ~DefaultException() throw()
        {
        }
        
        const c8* what() const throw()
        {
            return Message_.c_str();
        }
        
    private:
        
        io::stringc Message_;
        
};


//! Render-system-exceptions will be thrown when an object is going to be instantiated before the global render system has been created.
class SP_EXPORT RenderSystemException : public std::exception
{
    
    public:
        
        RenderSystemException(const io::stringc &ObjectName) throw() :
            ObjectName_(ObjectName)
        {
        }
        ~RenderSystemException() throw()
        {
        }
        
        const c8* what() const throw()
        {
            return ("\"" + ObjectName_ + "\" has been instantiated before global render system has been created").c_str();
        }
        
    private:
        
        io::stringc ObjectName_;
        
};


//! Null-pointer-exceptions will be thrown when an invalid object (or rather a null pointer) is passed to a class constructor which requires a valid object.
class SP_EXPORT NullPointerException : public std::exception
{
    
    public:
        
        NullPointerException(const io::stringc &Origin) throw() :
            Origin_(Origin)
        {
        }
        ~NullPointerException() throw()
        {
        }
        
        const c8* what() const throw()
        {
            return ("Null pointer has been passed to \"" + Origin_ + "\" where it is not allowed").c_str();
        }
        
    private:
        
        io::stringc Origin_;
        
};


} // /namespace io

} // /namespace sp


#endif



// ================================================================================
