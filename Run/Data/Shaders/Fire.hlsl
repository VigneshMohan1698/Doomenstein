//------------------------------------------------------------------------------------------------
struct vs_input_t
{
	float3 localPosition : POSITION;
	float4 color : COLOR;
	float2 uv : TEXCOORD;
};

//------------------------------------------------------------------------------------------------
struct v2p_t
{
	float4 position : SV_Position;
	float4 color : COLOR;
	float2 uv0 : TEXCOORD0;
	float2 uv1 : TEXCOORD1;
	float2 uv2 : TEXCOORD2;
	float2 uv3 : TEXCOORD3;
};

//------------------------------------------------------------------------------------------------
cbuffer CameraConstants : register(b2)
{
	float4x4 ProjectionMatrix;
	float4x4 ViewMatrix;
};


//------------------------------------------------------------------------------------------------
cbuffer ModelConstants : register(b3)
{
	float4x4 ModelMatrix;
	float4 ModelColor;
};

//-------------------------------------------------------------------------------------------------
cbuffer NoiseConstants : register(b4)
{
	float frameTime;
	float3 scrollSpeeds;
	float3 scales;
	float padding;
};

//-------------------------------------------------------------------------------------------------
cbuffer DistortionConstants: register(b5)
{
	float2 distortion1;
	float2 distortion2;
	float2 distortion3;
	float distortionScale;
	float distortionBias;
};

//------------------------------------------------------------------------------------------------
Texture2D diffuseTexture : register(t0);
Texture2D noiseTexture : register(t1);
Texture2D alphaTexture : register(t2);

SamplerState SampleType : register(s0);
SamplerState SampleType2 : register(s1);

//------------------------------------------------------------------------------------------------
v2p_t VertexMain(vs_input_t input)
{
	float4 localPosition = float4(input.localPosition, 1);
	float4 worldPosition = mul(ModelMatrix, localPosition);
	float4 viewPosition = mul(ViewMatrix, worldPosition);
	float4 clipPosition = mul(ProjectionMatrix, viewPosition);

	v2p_t v2p;
	v2p.position = clipPosition;
	v2p.color = input.color;
	v2p.uv0 = input.uv;

	v2p.uv1 = (input.uv * scales.x);
	v2p.uv1.y = v2p.uv1.y + (frameTime * scrollSpeeds.x);

	v2p.uv2 = (input.uv * scales.y);
	v2p.uv2.y = v2p.uv2.y + (frameTime * scrollSpeeds.y);

	v2p.uv3 = (input.uv * scales.z);
	v2p.uv3.y = v2p.uv3.y + (frameTime * scrollSpeeds.z);
	return v2p;
}

//------------------------------------------------------------------------------------------------
float4 PixelMain(v2p_t input) : SV_Target0
{
	float4 noise1;
	float4 noise2;
	float4 noise3;
	float4 finalNoise;
	float perturb;
	float2 noiseCoords;
	float4 fireColor;
	float4 alphaColor;

	noise1 = noiseTexture.Sample(SampleType, input.uv1);
	noise2 = noiseTexture.Sample(SampleType, input.uv2);
	noise3 = noiseTexture.Sample(SampleType, input.uv3);

	noise1 = (noise1 - 0.5f) * 2.0f;
	noise2 = (noise2 - 0.5f) * 2.0f;
	noise3 = (noise3 - 0.5f) * 2.0f;

	noise1.xy = noise1.xy * distortion1.xy;
	noise2.xy = noise2.xy * distortion2.xy;
	noise3.xy = noise3.xy * distortion3.xy;

	finalNoise = noise1 + noise2 + noise3;

	perturb = ((1.0f-input.uv0.y) * distortionScale) + distortionBias;
	noiseCoords.xy = (finalNoise.xy * perturb) + input.uv0.xy;
	fireColor = diffuseTexture.Sample(SampleType2, noiseCoords.xy);

	alphaColor = alphaTexture.Sample(SampleType2, noiseCoords.xy);
	fireColor.a = alphaColor;

	//float4 diffuseColor = diffuseTexture.Sample(diffuseSampler, input.uv);
	//float4 color = diffuseColor * input.color;
	//clip(fireColor.a - 0.5f);
	return fireColor;
}
