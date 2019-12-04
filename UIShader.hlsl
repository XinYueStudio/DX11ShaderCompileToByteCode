// 通过像素着色器传递的每个像素的颜色数据。
cbuffer ConstantBuffer : register(b0)
{
	float4x4 model;
	float4x4 view;
	float4x4 projection;

	float4x4 v;
	float4x4 p;
	float4 ScreenParams;
	float4 TextureParams;
	int id;
	int eyeindex;
	int Multi;
	int Stereo;

	int Flag1;
	int Flag2;
	int Flag3;
	int Flag4;
	float4 point0;
	float4 point1;
	float4 point2;
	float4 point3;

};


struct appdata
{
	float4 vertex : POSITION;
	float2 texcoord : TEXCOORD0;
};

struct v2f
{
	float4 vertex : SV_POSITION;
	float2 uv : TEXCOORD0;
};


Texture2D MainTexture : register(t0);
SamplerState SimpleSampler : register(s0);



v2f vert(appdata v)
{
	v2f o;
	float4 temp = v.vertex;
	temp.w = 1.0f;
	temp = mul(temp, model);
	temp = mul(temp, view);
	temp = mul(temp, projection);
	o.vertex = temp;
	o.uv = v.texcoord;
	return o;
}






float4 frag(v2f i) : SV_TARGET
{

	float4 temp = MainTexture.Sample(SimpleSampler, i.uv);

 

	float distance01 = distance(point0.xyz, point1.xyz);
	float distance03 = distance(point0.xyz, point3.xyz);
	float3 direction01 = normalize(point0.xyz - point1.xyz);
	float3 direction03 = normalize(point0.xyz - point3.xyz);
	float  proportionx = i.vertex.x*(ScreenParams.x / TextureParams.x) / ScreenParams.x;
	float  proportiony = i.vertex.y*(ScreenParams.y / TextureParams.y) / ScreenParams.y;

	if (Multi == 1)
	{
		proportionx = i.vertex.x*(ScreenParams.x / TextureParams.x) / ScreenParams.x * 2;

		if (eyeindex == 1)
		{
			proportionx = i.vertex.x*(ScreenParams.x / TextureParams.x) / ScreenParams.x * 2 - 1;
		}

		proportiony = i.vertex.y*(ScreenParams.y / TextureParams.y) / ScreenParams.y;
	}

	//Room floor
	if (id == 3)
	{
		float x = sin(40 * 0.0174532924F)*distance03 * 2;
		distance01 += x * proportiony;
	}


	float distancex = distance01 * proportionx;
	float distancey = distance03 * proportiony;
	float3 direction01right = direction01 * distancex;
	float3 direction01up = direction03 * distancey;
	float4 wroldPoint = point0;
	wroldPoint.xyz -= direction01right;
	wroldPoint.xyz -= direction01up;
	wroldPoint.w = 1.0f;
	float4 result4 = mul(mul(p, v), wroldPoint);
	float3 pos;
	pos.x = result4.x;
	pos.y = result4.y;
	pos.z = result4.z;

	pos /= -result4.w;
	pos.x = pos.x * 0.5f + 0.5f;
	pos.y = pos.y * 0.5f + 0.5f;
	float2 value;
	value.x = pos.x;
	value.y = pos.y;


	// if (i.vertex.x >= 798.0f)
	// {

	//	 temp.x = 1.0f;
	//	 temp.y = 0.0f;
	//	 temp.z = 0.5f;
	//	 temp.w = 1.0f;

	//}
	 return temp;
}
