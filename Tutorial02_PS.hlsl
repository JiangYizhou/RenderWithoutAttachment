
int2 ivec2(float2 x0)
{
	return int2(x0);
}
// Uniforms

uniform int2 _u_expectedSize : register(c0);
#define ANGLE_USES_DISCARD_REWRITING
#ifdef ANGLE_ENABLE_LOOP_FLATTEN
#define LOOP [loop]
#define FLATTEN [flatten]
#else
#define LOOP
#define FLATTEN
#endif
// Varyings

static float4 out_f_color = { 0, 0, 0, 0 };
static float4 gl_FragCoord = float4(0, 0, 0, 0);

cbuffer DriverConstants : register(b1)
{
	float4 dx_ViewCoords : packoffset(c1);
	float3 dx_DepthFront : packoffset(c2);
	float2 dx_ViewScale : packoffset(c3);
};

#define GL_USES_FRAG_COORD
;
;
void gl_main()
{
	if (!all(ivec2(gl_FragCoord.xy) == _u_expectedSize))
	{
		discard;
		;
	}
	(out_f_color = float4(1.0, 0.5, 0.25, 1.0));
}
;
struct PS_INPUT
{
	float4 dx_Position : SV_Position;
	float4 gl_Position : TEXCOORD0;
	float4 gl_FragCoord : TEXCOORD1;
};

struct PS_OUTPUT
{
	float4 out_f_color : SV_TARGET0;
};

PS_OUTPUT generateOutput()
{
	PS_OUTPUT output;
	output.out_f_color = out_f_color;
	return output;
}

PS_OUTPUT PS(PS_INPUT input)
{
	float rhw = 1.0 / input.gl_FragCoord.w;
	gl_FragCoord.x = input.dx_Position.x;
	gl_FragCoord.y = input.dx_Position.y;
	gl_FragCoord.z = (input.gl_FragCoord.z * rhw) * dx_DepthFront.x + dx_DepthFront.y;
	gl_FragCoord.w = rhw;

	gl_main();

	return generateOutput();
}
