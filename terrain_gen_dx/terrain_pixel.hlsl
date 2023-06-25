struct PSInput
{
    float4 position : SV_Position;
    float3 camera_pos : POSITION0;
    float3 position_actual : POSITION1;
    float height_scale : POSITION2;
};

struct PSOutput
{
    float4 color : SV_Target0;
};

PSOutput main(PSInput input)
{
    PSOutput output;

    output.color.xyz = floor(input.position_actual.yyy-0.001) / input.height_scale;

    return output;
}