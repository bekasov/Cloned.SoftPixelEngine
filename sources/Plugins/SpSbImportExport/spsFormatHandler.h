/*
 * Format handler header
 * 
 * This file is part of the "SoftPixel Scene Importer/Exporter" (Copyright (c) 2012 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __SPS_FORMAT_HANDLER_H__
#define __SPS_FORMAT_HANDLER_H__


#include "spsCore.h"

#include <vector>
#include <list>
#include <cstring>


namespace sps
{


//! 'SoftPixel Scene' format versions.
enum EFormatVersions
{
    FORMATVERSION_1_00      = 100,
    FORMATVERSION_1_01      = 101,
    FORMATVERSION_1_02      = 102,
    FORMATVERSION_1_03      = 103,
    FORMATVERSION_1_04      = 104,
    FORMATVERSION_1_05      = 105,
    FORMATVERSION_1_06      = 106,
    FORMATVERSION_1_07      = 107,
    FORMATVERSION_1_08      = 108,
    
    FORMATVERSION_LATEST    = FORMATVERSION_1_08,
};


/**
Base class for the importer and exporter.
Here you only have to overwrite the "Error" and "Warning" functions.
*/
class SPS_EXPORT SpSceneFormatHandler
{
    
    public:
        
        virtual ~SpSceneFormatHandler();
        
        /* === Functions === */
        
        //! Returns true if the given format version is supported.
        static bool IsVersionSupported(int32 Version);
        
        //! Returns the given format version as string.
        static std::string GetVersionString(int32 Version);
        
        //! Returns the oldest supported format version.
        static EFormatVersions GetOldestVersion();
        //! Returns the latest supported format version.
        static EFormatVersions GetLatestVersion();
        
        //! Returns the header format magic number (first four bytes in file).
        static int32 GetMagicNumber();
        
    protected:
        
        SpSceneFormatHandler();
        
        /* === Enumerations === */
        
        #include "spsFormatEnumerations.h"
        
        /* === Macros === */
        
        static const std::string SpLumpNames[];
        
        /* === Structures === */
        
        #include "spsBaseDataTypes.h"
        #include "spsFormatStructures.h"
        
        /* === Functions === */
        
        #ifdef __SPS_DEBUG__
        //! Dummy function (used for debug callback message).
        virtual void Debug(const std::string &Message);
        #endif
        
        //! Dummy function.
        virtual void Error(const std::string &Message, const EErrorTypes Type = ERROR_DEFAULT);
        //! Dummy function.
        virtual void Warning(const std::string &Message, const EWarningType Type = WARNING_DEFAULT);
        
        /* === Inline functions === */
        
        inline int32 GetVersion() const
        {
            return Header_.Version;
        }
        inline bool AtLeastVersion(const EFormatVersions Version) const
        {
            return Header_.Version >= Version;
        }
        
    private:
        
        friend class SpSceneImporter;
        friend class SpSceneExporter;
        
        /* === Functions === */
        
        uint32 GetLumpCount() const;
        
        /* === Members === */
        
        SpHeader Header_;
        
};


} // /namespace sps


#endif



// ================================================================================
