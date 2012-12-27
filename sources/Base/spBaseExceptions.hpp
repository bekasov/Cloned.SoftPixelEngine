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


//! Render system exceptions will be thrown when an object is going to be instantiated before the global render system has been created.
class RenderSystemException : public std::exception
{
    
    public:
        
        RenderSystemException(const io::stringc &ObjectName) :
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


} // /namespace io

} // /namespace sp


#endif



// ================================================================================
