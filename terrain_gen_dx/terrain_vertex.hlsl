struct VSInput
{
    float3 position : POSITION0;
    int2 uv : TEXCOORD0;
};

struct VSOutput
{
    float4 position : SV_Position;
    float3 camera_pos : POSITION0;
    float3 position_actual : POSITION1;
    float height_scale : POSITION2;
};

cbuffer vpmat : register(b0)
{
    column_major matrix vp_mat;
    float3 camera_pos;
    float height_scale;
}

Texture2D<float> htex : register(t0);

VSOutput main(VSInput input)
{
    VSOutput output;

    const float height = floor(input.position.y * htex[input.uv] * height_scale);

    output.height_scale = height_scale;

    output.camera_pos = camera_pos;
    output.position_actual = float3(input.position.x, height, input.position.z);
    
    output.position = mul(vp_mat, float4(input.position.x, height, input.position.z,1.0));

    return output;
}