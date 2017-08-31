Texture2D<float4> TextureF  : register(t0);
Texture2DMS<float4> TextureF_MS: register(t0);
Texture2D<uint4>  TextureUI : register(t0);
Texture2D<int4>   TextureI  : register(t0);

SamplerState Sampler        : register(s0);

void VS_Passthrough2D(in float2  inPosition :    POSITION, in float2  inTexCoord : TEXCOORD0,
	out float4 outPosition : SV_POSITION, out float2 outTexCoord : TEXCOORD0)
{
	outPosition = float4(inPosition, 0.0f, 1.0f);
	outTexCoord = inTexCoord;
}
float4 PS_PassthroughRGBA2D(in float4 inPosition : SV_POSITION, in float2 inTexCoord : TEXCOORD0) : SV_TARGET0
{
	return TextureF.Sample(Sampler, inTexCoord).rgba;
}