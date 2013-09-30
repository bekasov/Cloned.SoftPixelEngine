/*
 * Query interface header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_QUERY_H__
#define __SP_QUERY_H__


#include "Base/spStandard.hpp"


namespace sp
{
namespace video
{


//! Shader resource types. Currently only supported for the Direct3D 11 render system.
enum EQueryTypes
{
    QUERY_SAMPLES_PASSED = 0,   //!< Queries how many fragment (or rather pixel) samples have passed the depth test.
    QUERY_ANY_SAMPLES_PASSED,   //!< Queries if any fragment (or rather pixel) sample has passed the depth test. For Direct3D 9 render system, this is the same as for QUERY_SAMPLES_PASSED.
    QUERY_PRIMITIVES_GENERATED, //!< Records the number of primitives sent to a particular geometry shader output stream. For Direct3D 9 render system, this is the vertex statistic query.
    QUERY_TIMESTAMP,            //!< Records the current timestamp on the GPU.
};


/**
Query object interface. Query objects can be used to query some information
from the graphics device, i.e. how many samples have passed the depth test.
\code
// Create occlusion query
Query* MyQuery = spRenderer->createQuery(video::QUERY_SAMPLES_PASSED);

// ...

// Render mesh only into depth buffer
spRenderer->setColorMask(false);

MyMesh->getMaterial()->setDepthMethod(video::CMPSIZE_LESS);

// Use occlusion query
MyQuery->begin();
{
    MyMesh->render();
}
MyQuery->end();

spRenderer->setDepthMask(false);
spRenderer->setColorMask(true);

MyMesh->getMaterial()->setDepthMethod(video::CMPSIZE_EQUAL);

// Only render mesh into frame buffer, if it is visible
if (MyQuery->result() > 0)
    MyMesh->render();
\endcode
\since Version 3.3
*/
class Query
{
    
    public:
        
        virtual ~Query()
        {
        }
        
        /* === Functions === */
        
        //! Begins the query.
        virtual void begin() = 0;
        //! End the query.
        virtual void end() = 0;
        
        /**
        Returns the query result. Call this after you called the "begin" and "end" functions.
        \return Query result as integer.
        \todo Currently this can can end in a deadlock if the particular resource is not available!
        \see begin
        \see end
        */
        virtual u64 result() const = 0;
        
        /* === Inline functions === */
        
        /**
        Returns the query type.
        \see EQueryTypes
        */
        inline EQueryTypes getType() const
        {
            return Type_;
        }
        
    protected:
        
        /* === Functions === */
        
        Query(const EQueryTypes Type) :
            Type_(Type)
        {
        }
        
        /* === Members === */
        
        EQueryTypes Type_;
        
};


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
