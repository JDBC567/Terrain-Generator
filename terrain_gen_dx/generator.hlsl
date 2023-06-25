RWTexture2D<float> output_tex : register(u0);

RWTexture2D<float> input1 : register(u1);
RWTexture2D<float> input2 : register(u2);
RWTexture2D<float> input3 : register(u3);

[numthreads(32, 32, 1)]
void main(int3 dispatch_id:SV_DispatchThreadID)
{
    float val1 = input1[dispatch_id.xy]*2;
    float val2 = input2[dispatch_id.xy]*0.5;

    float val3 = input3[dispatch_id.xy]*10;

    output_tex[dispatch_id.xy] = pow((val1 + val2 + val3) / 12.5, 1);
}