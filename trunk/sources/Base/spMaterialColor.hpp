/*
 * Color header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_MATERIAL_COLOR_H__
#define __SP_MATERIAL_COLOR_H__


#include "Base/spStandard.hpp"
#include "Base/spDimensionVector3D.hpp"
#include "Base/spDimensionVector4D.hpp"
#include "Base/spInputOutputString.hpp"


namespace sp
{
namespace video
{


inline u32 getAlpha(u32 Color)
{
    return Color >> 24;
}
inline u32 getRed(u32 Color)
{
    return (Color >> 16) & 0xFF;
}
inline u32 getGreen(u32 Color)
{
    return (Color >> 8) & 0xFF;
}
inline u32 getBlue(u32 Color)
{
    return Color & 0xFF;
}

inline u32 getColor(u32 Red, u32 Green, u32 Blue, u32 Alpha)
{
    return ((Alpha & 0xFF) << 24) | ((Red & 0xFF) << 16) | ((Green & 0xFF) << 8) | (Blue & 0xFF);
}


class color
{
    
    public:
        
        color() :
            Red     (255),
            Green   (255),
            Blue    (255),
            Alpha   (255)
        {
        }
        color(u8 Brightness) :
            Red     (Brightness ),
            Green   (Brightness ),
            Blue    (Brightness ),
            Alpha   (255        )
        {
        }
        color(u8 R, u8 G, u8 B, u8 A = 255) :
            Red     (R),
            Green   (G),
            Blue    (B),
            Alpha   (A)
        {
        }
        color(const color &Other) :
            Red     (Other.Red  ),
            Green   (Other.Green),
            Blue    (Other.Blue ),
            Alpha   (Other.Alpha)
        {
        }
        color(const color &Other, u8 A) :
            Red     (Other.Red  ),
            Green   (Other.Green),
            Blue    (Other.Blue ),
            Alpha   (A          )
        {
        }
        color(const dim::vector3df &Color, bool Adjust = true) :
            Red     (255),
            Green   (255),
            Blue    (255),
            Alpha   (255)
        {
            if (Adjust)
            {
                Red     = static_cast<u8>(Color.X * 255);
                Green   = static_cast<u8>(Color.Y * 255);
                Blue    = static_cast<u8>(Color.Z * 255);
                Alpha   = 255;
            }
            else
            {
                Red     = static_cast<u8>(Color.X);
                Green   = static_cast<u8>(Color.Y);
                Blue    = static_cast<u8>(Color.Z);
                Alpha   = 255;
            }
        }
        color(const dim::vector4df &Color, bool Adjust = true) :
            Red     (255),
            Green   (255),
            Blue    (255),
            Alpha   (255)
        {
            if (Adjust)
            {
                Red     = static_cast<u8>(Color.X * 255);
                Green   = static_cast<u8>(Color.Y * 255);
                Blue    = static_cast<u8>(Color.Z * 255);
                Alpha   = static_cast<u8>(Color.W * 255);
            }
            else
            {
                Red     = static_cast<u8>(Color.X);
                Green   = static_cast<u8>(Color.Y);
                Blue    = static_cast<u8>(Color.Z);
                Alpha   = static_cast<u8>(Color.W);
            }
        }
        ~color()
        {
        }
        
        /* === Operators - comparisions === */
        
        inline bool operator == (const color &Other) const
        {
            return Red == Other.Red && Green == Other.Green && Blue == Other.Blue && Alpha == Other.Alpha;
        }
        inline bool operator != (const color &Other) const
        {
            return Red != Other.Red || Green != Other.Green || Blue != Other.Blue || Alpha != Other.Alpha;
        }
        
        /* === Operators - addition, subtraction, division, multiplication === */
        
        inline color operator + (const color &Other) const
        {
            return color(Red + Other.Red, Green + Other.Green, Blue + Other.Blue, Alpha + Other.Alpha);
        }
        inline color& operator += (const color &Other)
        {
            Red += Other.Red; Green += Other.Green; Blue += Other.Blue; Alpha += Other.Alpha; return *this;
        }
        
        inline color operator - (const color &Other) const
        {
            return color(Red - Other.Red, Green - Other.Green, Blue - Other.Blue, Alpha - Other.Alpha);
        }
        inline color& operator -= (const color &Other)
        {
            Red -= Other.Red; Green -= Other.Green; Blue -= Other.Blue; Alpha -= Other.Alpha; return *this;
        }
        
        inline color operator / (const color &Other) const
        {
            return color(Red / Other.Red, Green / Other.Green, Blue / Other.Blue, Alpha / Other.Alpha);
        }
        inline color& operator /= (const color &Other)
        {
            Red /= Other.Red; Green /= Other.Green; Blue /= Other.Blue; Alpha /= Other.Alpha; return *this;
        }
        
        inline color operator / (u8 Factor) const
        {
            return color(Red/Factor, Green/Factor, Blue/Factor, Alpha);
        }
        inline color& operator /= (u8 Factor)
        {
            Red /= Factor; Green /= Factor; Blue /= Factor; return *this;
        }
        
        inline color operator * (const color &Other) const
        {
            return color(Red * Other.Red, Green * Other.Green, Blue * Other.Blue, Alpha * Other.Alpha);
        }
        inline color& operator *= (const color &Other)
        {
            Red *= Other.Red; Green *= Other.Green; Blue *= Other.Blue; Alpha *= Other.Alpha; return *this;
        }
        
        inline color operator * (u8 Factor) const
        {
            return color(Red*Factor, Green*Factor, Blue*Factor, Alpha);
        }
        inline color& operator *= (u8 Factor)
        {
            Red *= Factor; Green *= Factor; Blue *= Factor; return *this;
        }
        
        inline color operator * (f32 Factor) const
        {
            return color(u8(f32(Red)*Factor), u8(f32(Green)*Factor), u8(f32(Blue)*Factor), Alpha);
        }
        inline color& operator *= (f32 Factor)
        {
            Red = u8(f32(Red)*Factor); Green = u8(f32(Green)*Factor); Blue = u8(f32(Blue)*Factor);
            return *this;
        }
        
        /* === Additional operators === */
        
        inline const u8 operator [] (u32 i) const
        {
            switch (i)
            {
                case 0: return Red;
                case 1: return Green;
                case 2: return Blue;
                case 3: return Alpha;
            }
            return 0;
        }
        
        inline u8& operator [] (u32 i)
        {
            return *(&Red + i);
        }
        
        /* === Extra functions === */
        
        inline color getInverse() const
        {
            return color(255 - Red, 255 - Green, 255 - Blue, 255 - Alpha);
        }
        inline color& setInverse()
        {
            Red = 255 - Red, Green = 255 - Green, Blue = 255 - Blue, Alpha = 255 -Alpha;
            return *this;
        }
        
        inline color getInverse(
            bool isRedInverse, bool isGreenInverse, bool isBlueInverse, bool isAlphaInverse = false) const
        {
            return color(
                isRedInverse    ? 255 - Red     : Red,
                isGreenInverse  ? 255 - Green   : Green,
                isBlueInverse   ? 255 - Blue    : Blue,
                isAlphaInverse  ? 255 - Alpha   : Alpha
            );
        }
        inline color& setInverse(
            bool isRedInverse, bool isGreenInverse, bool isBlueInverse, bool isAlphaInverse = false)
        {
            if (isRedInverse    ) Red   = 255 - Red;
            if (isGreenInverse  ) Green = 255 - Green;
            if (isBlueInverse   ) Blue  = 255 - Blue;
            if (isAlphaInverse  ) Alpha = 255 - Alpha;
            return *this;
        }
        
        /**
        Sets the color with the specified float array.
        This is the opposite function for "getFloatArray".
        \see getFloatArray
        */
        inline void setFloatArray(const f32* ColorArray)
        {
            if (ColorArray)
            {
                Red     = static_cast<u8>(ColorArray[0] * 255);
                Green   = static_cast<u8>(ColorArray[1] * 255);
                Blue    = static_cast<u8>(ColorArray[2] * 255);
                Alpha   = static_cast<u8>(ColorArray[3] * 255);
            }
        }
        /**
        Stores the color data into the given float array.
        \param[out] ColorArray Pointer to the float array where the color data is to be stored.
        This must have at least 4 elements! The components are then in the range [0.0 .. 1.0].
        \code
        f32 ColorArray[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
        MyColor.getFloatArray(ColorArray);
        \endcode
        \see setFloatArray
        */
        inline void getFloatArray(f32* ColorArray) const
        {
            if (ColorArray)
            {
                ColorArray[0] = static_cast<f32>(Red    ) / 255;
                ColorArray[1] = static_cast<f32>(Green  ) / 255;
                ColorArray[2] = static_cast<f32>(Blue   ) / 255;
                ColorArray[3] = static_cast<f32>(Alpha  ) / 255;
            }
        }
        
        inline void setIntensity(const color &Other)
        {
            Red     = u8( static_cast<s32>(Red     ) * static_cast<s32>(Other.Red  ) / 255 );
            Green   = u8( static_cast<s32>(Green   ) * static_cast<s32>(Other.Green) / 255 );
            Blue    = u8( static_cast<s32>(Blue    ) * static_cast<s32>(Other.Blue ) / 255 );
            Alpha   = u8( static_cast<s32>(Alpha   ) * static_cast<s32>(Other.Alpha) / 255 );
        }
        
        inline color getIntensity(const color &Other) const
        {
            color clr(*this);
            clr.setIntensity(Other);
            return clr;
        }
        
        inline void setIntensity(f32 Intensity)
        {
            Red     = static_cast<u8>(Intensity * Red   );
            Green   = static_cast<u8>(Intensity * Green );
            Blue    = static_cast<u8>(Intensity * Blue  );
            Alpha   = static_cast<u8>(Intensity * Alpha );
        }
        
        inline color getIntensity(f32 Intensity) const
        {
            color clr(*this);
            clr.setIntensity(Intensity);
            return clr;
        }
        
        inline void set(u8 R, u8 G, u8 B, u8 A)
        {
            Red = R; Green = G; Blue = B; Alpha = A;
        }
        
        inline u32 getSingle() const
        {
            return getColor(Red, Green, Blue, Alpha);
        }
        inline void setSingle(u32 Color)
        {
            Red     = getRed    (Color);
            Green   = getGreen  (Color);
            Blue    = getBlue   (Color);
            Alpha   = getAlpha  (Color);
        }
        
        inline bool equal(const color &Other, bool isIncludingAlpha = true) const
        {
            return Red == Other.Red && Green == Other.Green && Blue == Other.Blue && ( !isIncludingAlpha || Alpha == Other.Alpha );
        }
        
        //! Returns true if the color is gray scaled, i.e. all color components (red, green and blue) are equal.
        inline bool gray() const
        {
            return Red == Green && Green == Blue;
        }
        
        //! \todo Rename to "getVector3"
        dim::vector3df getVector(bool Adjust = false) const
        {
            if (!Adjust)
            {
                return dim::vector3df(
                    static_cast<f32>(Red), static_cast<f32>(Green), static_cast<f32>(Blue)
                );
            }
            return getVector(false) / 255.0f;
        }

        dim::vector4df getVector4(bool Adjust = false) const
        {
            if (!Adjust)
            {
                return dim::vector4df(
                    static_cast<f32>(Red), static_cast<f32>(Green), static_cast<f32>(Blue), static_cast<f32>(Alpha)
                );
            }
            return dim::vector4df(
                static_cast<f32>(Red    ) / 255.0f,
                static_cast<f32>(Green  ) / 255.0f,
                static_cast<f32>(Blue   ) / 255.0f,
                static_cast<f32>(Alpha  ) / 255.0f
            );
        }
        
        template <typename T> inline T getBrightness() const
        {
            return static_cast<T>( ( static_cast<s32>(Red) + static_cast<s32>(Green) + static_cast<s32>(Blue) ) / 3 );
        }
        
        /* === Static members === */
        
        static const color empty;   //!< color(0, 0, 0, 0). \deprecated
        static const color red;     //!< color(255, 0, 0, 255). \deprecated
        static const color green;   //!< color(0, 255, 0, 255). \deprecated
        static const color blue;    //!< color(0, 0, 255, 255). \deprecated
        static const color yellow;  //!< color(255, 255, 0, 255). \deprecated
        static const color pink;    //!< color(255, 0, 255, 255). \deprecated
        static const color black;   //!< color(0, 0, 0, 255). \deprecated
        static const color white;   //!< color(255, 255, 255, 255). \deprecated
        
        static const color EMPTY;   //!< color(0, 0, 0, 0).
        static const color RED;     //!< color(255, 0, 0, 255).
        static const color GREEN;   //!< color(0, 255, 0, 255).
        static const color BLUE;    //!< color(0, 0, 255, 255).
        static const color YELLOW;  //!< color(255, 255, 0, 255).
        static const color PINK;    //!< color(255, 0, 255, 255).
        static const color BLACK;   //!< color(0, 0, 0, 255).
        static const color WHITE;   //!< color(255, 255, 255, 255).
        
        /* === Static functions === */
        
        /**
        Returns a color derived from the given name.
        \param[in] Name Specifies the color name. Valid values are:
        \li \c red video::color(255, 0, 0)
        \li \c green video::color(0, 255, 0)
        \li \c blue video::color(0, 0, 255)
        \li \c yellow video::color(255, 255, 0)
        \li \c black video::color(0)
        \li \c white video::color(255)
        Otherwise a hex-code can be used: e.g. "#ff00e8".
        \return The specified color or transparent-black (video::color(0, 0, 0, 0)) if the name could not be parsed correctly.
        */
        static color fromName(io::stringc Name)
        {
            if (!Name.size())
                return empty;
            
            Name.makeLower();
            
            if (Name[0] == '#')
            {
                video::color Color;
                
                if (Name.size() == 7 || Name.size() == 9)
                {
                    Color.Red = io::getHexNumber<u8>(Name.mid(1, 2));
                    Color.Green = io::getHexNumber<u8>(Name.mid(3, 2));
                    Color.Blue = io::getHexNumber<u8>(Name.mid(5, 2));
                    
                    if (Name.size() == 9)
                        Color.Alpha = io::getHexNumber<u8>(Name.mid(7, 2));
                    
                    return Color;
                }
                
                return empty;
            }
            
            if (Name == "red"   ) return red;
            if (Name == "green" ) return green;
            if (Name == "blue"  ) return blue;
            if (Name == "yellow") return yellow;
            if (Name == "black" ) return black;
            if (Name == "white" ) return white;
            
            return empty;
        }
        
        /* === Members === */
        
        u8 Red, Green, Blue, Alpha;
        
};


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
