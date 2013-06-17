
// Texture Buffer Object (TBO) Test HLSL Vertex Shader

struct SVertexIn
{
	float3 Position : POSITION;
	float2 TexCoord : TEXCOORD;
};

struct SVertexOut
{
	float4 Position : SV_Position;
	float2 TexCoord : TEXCOORD;
};

cbuffer BufferVertex : register(c0)
{
	float4x4 WorldViewProjectionMatrix;
};

SVertexOut VertexMain(SVertexIn In)
{
	SVertexOut Out = (SVertexOut)0;
	
	Out.Position = mul(WorldViewProjectionMatrix, float4(In.Position, 1.0));
	Out.TexCoord = In.TexCoord;
	
	return Out;
};


// Texture Buffer Object (TBO) Test HLSL Pixel Shader

tbuffer BufTex : register(t0)
{
	float4 BufferColors[100];
};

float4 PixelMain(SVertexOut In) : SV_Target0
{
	float4 Color = (float4)0;
	
	Color = float4(In.TexCoord.x, In.TexCoord.y, BufferColors[0].x, 1.0);
	
	return Color;
};




