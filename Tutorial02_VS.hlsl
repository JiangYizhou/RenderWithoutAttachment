float4 vec4(float2 x0, float x1, float x2)
{
	return float4(x0, x1, x2);
}
#ifdef ANGLE_ENABLE_LOOP_FLATTEN
#define LOOP [loop]
#define FLATTEN [flatten]
#else
#define LOOP
#define FLATTEN
#endif
// Attributes
static float2 _a_position = { 0, 0 };

static float4 gl_Position = float4(0, 0, 0, 0);

// Varyings

cbuffer DriverConstants : register(b1)
{
	float4 dx_ViewAdjust : packoffset(c1);
	float2 dx_ViewCoords : packoffset(c2);
	float2 dx_ViewScale  : packoffset(c3);
};

;
void gl_main()
{
	(gl_Position = vec4(_a_position, 0.0, 1.0));
}
;

struct VS_INPUT
{
	float2 _a_position : TEXCOORD0;
};

void initAttributes(VS_INPUT input)
{
	_a_position = input._a_position;
}

struct VS_OUTPUT
{
	float4 dx_Position : SV_Position;
	float4 gl_Position : TEXCOORD0;
	float4 gl_FragCoord : TEXCOORD1;
};

VS_OUTPUT VS(VS_INPUT input)
{
	initAttributes(input);

	gl_main();

	VS_OUTPUT output;
	output.gl_Position = gl_Position;
	output.dx_Position.x = gl_Position.x;
	output.dx_Position.y = -gl_Position.y;
	output.dx_Position.z = (gl_Position.z + gl_Position.w) * 0.5;
	output.dx_Position.w = gl_Position.w;
	output.gl_FragCoord = gl_Position;

	return output;
}
