/*
 * Base object header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_BASEOBJECT_H__
#define __SP_BASEOBJECT_H__


#include "Base/spStandard.hpp"
#include "Base/spInputOutputString.hpp"


namespace sp
{


//! This is the base object class. It only stores the object's name and a user data pointer for general purpose.
class SP_EXPORT BaseObject
{
    
    public:
        
        virtual ~BaseObject()
        {
        }
        
        /* === Inline functions === */
        
        /**
        Sets the user data.
        \param Data: Pointer to an individual data field which can be defined by the user.
        By default 0 when the pointer is unused.
        */
        inline void setUserData(void* Data)
        {
            UserData_ = Data;
        }
        /**
        \return Pointer to an individual data field which can be defined by the user.
        Use ANSI-C casting e.g. like: "MyDataStructure* MyData = (MyDataStructure*)MyNodeObject->getUserData();".
        The pointer type must be the same which has been set before with "setUserData".
        */
        inline void* getUserData() const
        {
            return UserData_;
        }
        
        /**
        Sets the user data flags. Use this to determine from which type the user data is.
        \see setUserData
        \since Version 3.2
        */
        inline void setUserDataFlags(s32 Flags)
        {
            UserDataFlags_ = Flags;
        }
        /**
        Returns the user data flags. By default 0.
        \see getUserData
        \since Version 3.2
        */
        inline s32 getUserDataFlags() const
        {
            return UserDataFlags_;
        }
        
        /* === Identification === */
        
        //! Sets the objects's name.
        inline void setName(const io::stringc &Name)
        {
            Name_ = Name;
        }
        //! Returns the objects's name.
        inline io::stringc getName() const
        {
            return Name_;
        }
        
    protected:
        
        BaseObject() :
            UserData_       (0),
            UserDataFlags_  (0)
        {
        }
        BaseObject(const io::stringc &Name) :
            UserData_       (0      ),
            UserDataFlags_  (0      ),
            Name_           (Name   )
        {
        }
        
        /* Members */
        
        void* UserData_;
        s32 UserDataFlags_;
        io::stringc Name_;
        
};


} // /namespace sp


#endif



// ================================================================================
