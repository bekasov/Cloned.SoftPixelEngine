/*
 * D3D11 default font shader file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

/* === Macros === */

#define GET_UINT_INDEXED(v, i) (((uint[4])(v))[(i) % 4])
#define GET_FLOAT_INDEXED(v, i) (((float[4])(v))[(i) % 4])

#define GET_UINT_SHIFTED(v, i) (((v) << ((i) % 4)) & 0xFF000000)


/* === Structures === */

struct SQuad2DVertex
{
    float2 Position;
    float2 TexCoord;
};

struct SFontGlyph
{
    SQuad2DVertex Vertices[4];
};

struct SGeometryOutput
{
	float4 Position : SV_Position;
	float2 TexCoord : TEXCOORD0;
};

struct SDummy
{
};


/* === Shader resources === */

Texture2D FontTexture : register(t0);
SamplerState Sampler : register(s0);

cbuffer BufferTransform : register(b0)
{
    float4x4 WVPMatrix;
    float4 Color;
    float2 Position;
};

#define _USE_STRUCT_BUF_
#ifdef _USE_STRUCT_BUF_

StructuredBuffer<SFontGlyph> Glyphs;

Buffer<uint> Text;
Buffer<float> CharOffset;

#else

cbuffer BufferFont : register(b1)
{
    SFontGlyph Glyphs[256];
};

cbuffer BufferText : register(b2)
{
    // Maximum number of characters in the string are 256*16 = 4096
    uint4 Text[256];
    float4 CharOffset[1024];
};

#endif


/* === Functions === */

SDummy VertexMain(void)
{
    return (SDummy)0;
}

[maxvertexcount(4)]
void GeometryMain(
    point SDummy Unused[1],
    uint PrimID : SV_PrimitiveID,
    inout TriangleStream<SGeometryOutput> OutStream)
{
    // Get the glyph index (every entry in "Text" contains four characters)
    #ifdef _USE_STRUCT_BUF_
    uint GlyphIndex = GET_UINT_SHIFTED(Text[PrimID / 4], PrimID);
    #else
    uint GlyphIndex = GET_UINT_SHIFTED(
        GET_UINT_INDEXED(Text[PrimID / 16], PrimID / 4), PrimID
    );
    #endif

    // Get glyph offset
    #ifdef _USE_STRUCT_BUF_
    float Offset = CharOffset[PrimID];
    #else
    float Offset = GET_FLOAT_INDEXED(CharOffset, PrimID);
    #endif

    // Get the current font glyph.
    SFontGlyph Glyph = Glyphs[GlyphIndex];

    // Generate the vertices
	SGeometryOutput Out = (SGeometryOutput)0;
	
	for (uint i = 0; i < 4; ++i)
	{
        SQuad2DVertex Vert = Glyph.Vertices[i];

        // Setup glyph final position
        float4 Pos = float4(
            Position.x + Vert.Position.x + Offset,
            Position.y + Vert.Position.y,
            0.0,
            1.0
        );

        // Setup final vertex output
		Out.Position = mul(WVPMatrix, Pos);
		Out.TexCoord = Vert.TexCoord;
		
		OutStream.Append(Out);
	}
	
	OutStream.RestartStrip();
}

float4 PixelMain(SGeometryOutput In) : SV_Target0
{
    // Simple texture sampling with text color manipulation
    return FontTexture.Sample(Sampler, In.TexCoord) * Color;
}

