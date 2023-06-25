//doesnt work something wrong with grad function. rest works
//hlsl version of 2D OpenSimplex noise described here https://github.com/KdotJPG/OpenSimplex2/blob/master/java/OpenSimplex2S.java

struct int64
{
    uint high;
    uint low;
};

struct int64_2
{
    int64 x;
    int64 y;
};

int64 int64_(int2 a)
{
    int64 res;
    res.high = 0 | a.x;
    res.low = 0 | a.y;
    return res;
}

int64 int64_(uint a,uint b)
{
    int64 res;
    res.high = a;
    res.low = b;
    return res;
}

int64_2 int64_2_(int64 a, int64 b)
{
    int64_2 res;
    res.x = a;
    res.y = b;
    return res;
}

int64 from(int2 i)
{
    int64 result;
    result.high = 0 | i.x;
    result.low = 0 | i.y;
    return result;
}

int64 add(int64 a,int64 b)
{
    int64 res;
	res.low=(a.low + b.low);
    res.high = (a.high + b.high + (res.low < a.low));

    return res;
}

int64_2 add(int64_2 a,int64_2 b)
{
    int64_2 res;
    res.x = add(a.x, b.x);
    res.y = add(a.y, b.y);
    return res;
}

int64 sub(int64 a,int64 b)
{
    int64 res;
	//negate b
    res.high = ~b.high;
    res.low = ~b.low;
    const uint neg_low = (res.low + 1);

    res.high = (res.high + (neg_low < res.low));
    res.low = neg_low;
	//add
    res.low = (a.low + res.low);
    res.high = (a.high + res.high + (res.low < a.low));

    return res;
}

int64 mul(int64 a,int64 b)
{
    int64 res;

    bool sign_flag = ((a.high & 0x80000000) == (b.high & 0x80000000));

    if (a.high & 0x80000000)
    {
        a.high = ~a.high;
        a.low = ~a.low;
        const uint neg_low = (a.low + 1) & 0xffffffff;
        a.high = (a.high + (neg_low < a.low)) & 0xffffffff;
        a.low = neg_low;
    }
    if (b.high & 0x80000000)
    {
        b.high = ~b.high;
        b.low = ~b.low;
        const uint neg_low = (b.low + 1) & 0xffffffff;
        b.high = (b.high + (neg_low < b.low)) & 0xffffffff;
        b.low = neg_low;
    }

    uint a_prt = a.low & 0xffff;
    uint b_prt = b.low & 0xffff;
    uint mul_res_1 = a_prt * b_prt;
    uint carry = mul_res_1 >> 16;
    mul_res_1 &= 0xffff;

    a_prt = a.low >> 16;
    b_prt = b.low >> 16;
    uint mul_res_2 = a_prt * b_prt + carry;
    bool carry_plus_one = (mul_res_2 < a_prt);
    mul_res_2 -= carry_plus_one;
    carry = mul_res_2 >> 16 + carry_plus_one;
    mul_res_2 &= 0xffff;

    a_prt = a.high & 0xffff;
    b_prt = b.high & 0xffff;
    uint mul_res_3 = a_prt * b_prt;
    carry_plus_one = (mul_res_3 < a_prt);
    mul_res_3 -= carry_plus_one;
    carry = mul_res_3 >> 16 + carry_plus_one;
    mul_res_3 &= 0xffff;

    a_prt = a.high >> 16;
    b_prt = b.high >> 16;
    uint mul_res_4 = a_prt * b_prt + carry;
    mul_res_4 &= 0xffff0000;
    mul_res_4 &= 0xffff;

    res.high = (mul_res_4 << 16) + mul_res_3;
    res.low = (mul_res_2 << 16) + mul_res_1;

    if (!sign_flag)
    {
        res.high = ~res.high;
        res.low = ~res.low;
        const uint neg_low = (res.low + 1) & 0xffffffff;
        res.high = (res.high + (neg_low < res.low)) & 0xffffffff;
        res.low = neg_low;
    }

    return res;
}

int64 xor_int(int64 a,int b)
{
    int64 res={a.high^b,a.low^b};
    return res;
}

int64 xor_i64(int64 a,int64 b)
{
    int64 res = { a.high ^ b.high, a.low ^ b.low };
    return res;
}

RWTexture2D<float> tex : register(u0);


cbuffer constants : register(b0){
    float2 scale = float2(1.0f,1.0f);
    double2 offset = double2(0.0f,0.0f);
    int2 seed;
}

static const float gradients[256] =
{
    6.980896f, 16.853374f, 16.853374f, 6.980896f,
	16.853374f, -6.980896f, 6.980896f, -16.853374f,
	-6.980896f, -16.853374f, -16.853374f, -6.980896f,
	-16.853374f, 6.980896f, -6.980896f, 16.853374f,
	2.381054f, 18.085899f, 11.105002f, 14.472322f,
	14.472322f, 11.105002f, 18.085899f, 2.381054f,
	18.085899f, -2.381054f, 14.472322f, -11.105002f,
	11.105002f, -14.472322f, 2.381054f, -18.085899f,
	-2.381054f, -18.085899f, -11.105002f, -14.472322f,
	-14.472322f, -11.105002f, -18.085899f, -2.381054f,
	-18.085899f, 2.381054f, -14.472322f, 11.105002f,
	-11.105002f, 14.472322f, -2.381054f, 18.085899f,
	6.980896f, 16.853374f, 16.853374f, 6.980896f,
	16.853374f, -6.980896f, 6.980896f, -16.853374f,
	-6.980896f, -16.853374f, -16.853374f, -6.980896f,
	-16.853374f, 6.980896f, -6.980896f, 16.853374f,
	2.381054f, 18.085899f, 11.105002f, 14.472322f,
	14.472322f, 11.105002f, 18.085899f, 2.381054f,
	18.085899f, -2.381054f, 14.472322f, -11.105002f,
	11.105002f, -14.472322f, 2.381054f, -18.085899f,
	-2.381054f, -18.085899f, -11.105002f, -14.472322f,
	-14.472322f, -11.105002f, -18.085899f, -2.381054f,
	-18.085899f, 2.381054f, -14.472322f, 11.105002f,
	-11.105002f, 14.472322f, -2.381054f, 18.085899f,
	6.980896f, 16.853374f, 16.853374f, 6.980896f,
	16.853374f, -6.980896f, 6.980896f, -16.853374f,
	-6.980896f, -16.853374f, -16.853374f, -6.980896f,
	-16.853374f, 6.980896f, -6.980896f, 16.853374f,
	2.381054f, 18.085899f, 11.105002f, 14.472322f,
	14.472322f, 11.105002f, 18.085899f, 2.381054f,
	18.085899f, -2.381054f, 14.472322f, -11.105002f,
	11.105002f, -14.472322f, 2.381054f, -18.085899f,
	-2.381054f, -18.085899f, -11.105002f, -14.472322f,
	-14.472322f, -11.105002f, -18.085899f, -2.381054f,
	-18.085899f, 2.381054f, -14.472322f, 11.105002f,
	-11.105002f, 14.472322f, -2.381054f, 18.085899f,
	6.980896f, 16.853374f, 16.853374f, 6.980896f,
	16.853374f, -6.980896f, 6.980896f, -16.853374f,
	-6.980896f, -16.853374f, -16.853374f, -6.980896f,
	-16.853374f, 6.980896f, -6.980896f, 16.853374f,
	2.381054f, 18.085899f, 11.105002f, 14.472322f,
	14.472322f, 11.105002f, 18.085899f, 2.381054f,
	18.085899f, -2.381054f, 14.472322f, -11.105002f,
	11.105002f, -14.472322f, 2.381054f, -18.085899f,
	-2.381054f, -18.085899f, -11.105002f, -14.472322f,
	-14.472322f, -11.105002f, -18.085899f, -2.381054f,
	-18.085899f, 2.381054f, -14.472322f, 11.105002f,
	-11.105002f, 14.472322f, -2.381054f, 18.085899f,
	6.980896f, 16.853374f, 16.853374f, 6.980896f,
	16.853374f, -6.980896f, 6.980896f, -16.853374f,
	-6.980896f, -16.853374f, -16.853374f, -6.980896f,
	-16.853374f, 6.980896f, -6.980896f, 16.853374f,
	2.381054f, 18.085899f, 11.105002f, 14.472322f,
	14.472322f, 11.105002f, 18.085899f, 2.381054f,
	18.085899f, -2.381054f, 14.472322f, -11.105002f,
	11.105002f, -14.472322f, 2.381054f, -18.085899f,
	-2.381054f, -18.085899f, -11.105002f, -14.472322f,
	-14.472322f, -11.105002f, -18.085899f, -2.381054f,
	-18.085899f, 2.381054f, -14.472322f, 11.105002f,
	-11.105002f, 14.472322f, -2.381054f, 18.085899f,
	6.980896f, 16.853374f, 16.853374f, 6.980896f,
	16.853374f, -6.980896f, 6.980896f, -16.853374f,
	-6.980896f, -16.853374f, -16.853374f, -6.980896f,
	-16.853374f, 6.980896f, -6.980896f, 16.853374f,
};

static const float2 lut1[4] =
{
	1,1,-1,-1,-1,-1,0,0
};

static const float2 lut2[8] =
{
    -1.3660254, -0.36602542,
	0.21132487, -0.7886751,
	-0.366025, -1.366025,
	-0.788675, 0.211325,
	0.788675, -0.211325,
	-0.788675, 0.211325,
	-0.211325, 0.788675,
	0.211325, -0.788675,
};

static const int64_2 lut3[8] =
{
    {
        { 2752151639, 618762462 },
        { 1502401319, 3676085 }
    },
    {
        { 0, 0 },
        { 1502401319, 3676085 }
    },
    {
        { 1376075819, 2456864879 },
        { 3004802638, 7352170 }
    },
    {
        { 1376075819, 2456864879 },
        { 0, 0 }
    },
    {
        { 2918891476, 1838102417 },
        { 0, 0 }
    },
    {
        { 1376075819, 2456864879 },
        { 0, 0 }
    },
    {
        { 0, 0 },
        { 2792565976, 4291291211 }
    },
    {
        { 0, 0 },
        { 1502401319, 3676085 }
    }
};

float grad(int64 seed_64,int64_2 xysvp, float2 dxy)
{
    int64 hash = xor_i64(seed_64, xor_i64(xysvp.x, xysvp.y));
    return asfloat(seed_64.high);
    const int64 hash_mul ={1403254573 ,4005906165};
    hash = mul(hash, hash_mul);

    const uint shifted = hash.high >> 26;
    hash = xor_i64(hash, int64_(0, shifted));
    int gi = int(hash.low | (hash.high & 2147483648)) & 254;

	float2 grad_val;

    if (gi == (gi | 1))
    {
        grad_val =  dxy * gradients[gi];
    }
    else
    {
        grad_val.x = gradients[gi];
        grad_val.y = gradients[gi | 1];

    }
    grad_val *= dxy;
    return grad_val.x + grad_val.y;
}

float noise2(int64 seed_64,double2 xy)
{
    double2 s = (0.366025403784439L * (xy.x + xy.y)).xx;
	s += xy;

    int2 xysb = int2(floor(s.x), floor(s.y));
    float2 xyi = float2(s - double2(xysb));

    int64_2 xysbp=int64_2_(int64_(0,0),int64_(0,0));
    static const int64_2 xysbp_c = int64_2_(int64_(1376075819, 2456864879), int64_(1502401319, 3676085));
    xysbp.x = mul(from(xysb), xysbp_c.x);
    xysbp.y = mul(from(xysb), xysbp_c.y);

    float t = float(double(xyi.x + xyi.y) * -0.21132486540518713L);

    float2 dxy0 = xyi + t.xx;

    float2 dxysq = dxy0 * dxy0;

    float a = 0.666667 - dxysq.x - dxysq.y;

    float asq = a * a;

    float value = asq * asq * grad(seed_64,xysbp, dxy0);

    return grad(seed_64,xysbp, dxy0);

    a = -3.1547005 * t + (-0.6666667 + a);
    const float2 dxy_ = dxy0 - (0.57735026).xx;
    asq = a * a;
    value += asq * asq * grad(seed_64,add(xysbp, int64_2_(int64_(1376075819, 2456864879), int64_(1502401319, 3676085))), dxy_);
	
    int comp = int(t > (-0.21132486540518713L));

    t = xyi.x - xyi.y;
    float2 comp2_lhs = xyi + float2(t, -t);

    comp2_lhs = comp2_lhs * lut1[comp * 2] + lut1[comp * 2 + 1];

    comp = comp << 2;

    int2 lut2_3_index = int2(comp + int(comp2_lhs.x < 0), comp + int(comp2_lhs.y < 0) + 2);

    float4 dxy = dxy0.xyxy + float4(lut2[lut2_3_index.x], lut2[lut2_3_index.y]);

    float4 dxysq_ = dxy * dxy;

    comp2_lhs = float2(0.666667, 0.666667) + float2(-dxysq_.x - dxysq_.y, -dxysq_.z - dxysq_.w);

    if (comp2_lhs.x > 0)
    {
        asq = comp2_lhs.x * comp2_lhs.x;
        value += asq * asq * grad(seed_64,add(xysbp, lut3[lut2_3_index.x]), dxy.xy);
    }

    if (comp2_lhs.y > 0)
    {
        asq = comp2_lhs.y * comp2_lhs.y;
        value += asq * asq * grad(seed_64,add(xysbp, lut3[lut2_3_index.y]), dxy.zw);
    }
	
    value += 1.0;
    value *= 0.5;

    return value;
}

[numthreads(8,8,1)]
void main(int3 dispatch_thread_id:SV_DispatchThreadID)
{
	const int64 seed_64 = int64_(seed);
    tex[dispatch_thread_id.xy] = noise2(seed_64,(double2(dispatch_thread_id.xy) + offset) * scale);
}