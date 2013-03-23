/*
 * Format handler file
 * 
 * This file is part of the "SoftPixel Scene Importer/Exporter" (Copyright (c) 2012 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "spsFormatHandler.h"

#include <sstream>


namespace sps
{


#include "spsFormatMacros.h"

SpSceneFormatHandler::SpSceneFormatHandler()
{
}
SpSceneFormatHandler::~SpSceneFormatHandler()
{
}

bool SpSceneFormatHandler::IsVersionSupported(int32 Version)
{
    return Version >= SpSceneFormatHandler::GetOldestVersion() && Version <= SpSceneFormatHandler::GetLatestVersion();
}

std::string SpSceneFormatHandler::GetVersionString(int32 Version)
{
    if (!IsVersionSupported(Version))
        return "";
    
    /* Get major and minor version numbers */
    int32 Major = Version / 100;
    int32 Minor = Version % 100;
    
    /* Convert to string */
    std::stringstream SStr;
    SStr << Major << ".";
    
    if (Minor < 10)
        SStr << "0";
    
    SStr << Minor;
    
    return SStr.str();
}

EFormatVersions SpSceneFormatHandler::GetOldestVersion()
{
    return FORMATVERSION_1_00;
}
EFormatVersions SpSceneFormatHandler::GetLatestVersion()
{
    return FORMATVERSION_1_09;
}

int32 SpSceneFormatHandler::GetMagicNumber()
{
    return *((int32*)"SpSb");
}


/*
 * ======= Protected: =======
 */

#ifdef __SPS_DEBUG__
void SpSceneFormatHandler::Debug(const std::string &Message)
{
    // dummy
}
#endif

void SpSceneFormatHandler::Error(const std::string &Message, const EErrorTypes Type)
{
    // dummy
}
void SpSceneFormatHandler::Warning(const std::string &Message, const EWarningType Type)
{
    // dummy
}


/*
 * ======= Private: =======
 */

uint32 SpSceneFormatHandler::GetLumpCount() const
{
    /* Get count of used lump directories */
    uint32 LumpCount;
    
    if (AtLeastVersion(FORMATVERSION_1_09))
        LumpCount = 15;
    else if (AtLeastVersion(FORMATVERSION_1_08))
        LumpCount = 14;
    else if (AtLeastVersion(FORMATVERSION_1_06))
        LumpCount = 13;
    else if (AtLeastVersion(FORMATVERSION_1_02))
        LumpCount = 10;
    else
        LumpCount = 9;
    
    return LumpCount;
}


} // /namespace sps



// ================================================================================
