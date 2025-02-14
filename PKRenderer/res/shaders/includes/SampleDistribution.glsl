#pragma once

float3x3 ComposeTBNFast(const float3 N)
{
    float3 T, B;

    if (N.z < 0.0)
    {
        const float a = 1.0f / (1.0f - N.z);
        const float b = N.x * N.y * a;
        T = float3(1.0f - N.x * N.x * a, -b, N.x);
        B = float3(b, N.y * N.y * a - 1.0f, -N.y);
    }
    else
    {
        const float a = 1.0f / (1.0f + N.z);
        const float b = -N.x * N.y * a;
        T = float3(1.0f - N.x * N.x * a, b, -N.x);
        B = float3(b, 1.0f - N.y * N.y * a, -N.y);
    }
    
    return float3x3(T, B, N);
}

float3x3 ComposeTBN(const float3 N, const float3 U)
{
    float3 T = normalize(cross(U, N));
    float3 B = cross(N, T);
    return float3x3(T, B, N);
}

float3x3 ComposeTBN(const float3 N)
{
    float3 U = abs(N.z) < 0.999 ? float3(0.0, 0.0, 1.0) : float3(1.0, 0.0, 0.0);
    return ComposeTBN(N, U);
}

float RadicalInverse_VdC(uint bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10;
}

float2 Hammersley(uint i, uint N)
{
    return float2(float(i % N) / float(N), RadicalInverse_VdC(i));
}

float3 GetSampleDirectionHammersLey(const float3 Xi, float blur)
{
    float theta = (1.0f - Xi.z) / (1.0f + (blur - 1.0f) * Xi.z);
    float2 sincos = sqrt(float2(1.0f - theta, theta));
    return normalize(float3(Xi.xy * sincos.xx, sincos.y));
}

float3 DistributeGGX(const float2 Xi, float roughness)
{
    float a = roughness * roughness;
    float phi = 2.0 * 3.14159265 * Xi.x;
    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a * a - 1.0) * Xi.y));
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

    float3 H;
    H.x = cos(phi) * sinTheta;
    H.y = sin(phi) * sinTheta;
    H.z = cosTheta;
    return H;
}

float3 ImportanceSampleGGX(const float2 Xi, const float3 N, float roughness)
{
    float3 H = DistributeGGX(Xi, roughness);
    return normalize(ComposeTBN(N) * H);
}

float3 ImportanceSampleGGX(uint i, uint s, const float3 N, const float R)
{
    float2 Xi = Hammersley(i,s);
    return ImportanceSampleGGX(Xi, N, R);
}

// "Sampling the GGX Distribution of Visible Normals", Heitz
float3 ImportanceSampleGGXVNDF(float2 Xi, const vec3 V, float R)
{
    // prevent grazing angles 
    Xi *= 0.98;

    // Section 3.2: transforming the view direction to the hemisphere configuration
    float3 Vh = normalize(float3(R * V.x, R * V.y, V.z));
    
    // Section 4.1: orthonormal basis (with special case if cross product is zero)
    float lensq = Vh.x * Vh.x + Vh.y * Vh.y;
    const float3 T1 = lensq > 0 ? float3(-Vh.y, Vh.x, 0) * inversesqrt(lensq) : float3(1,0,0);
    const float3 T2 = cross(Vh, T1);

    // Section 4.2: parameterization of the projected area
    float r = sqrt(Xi.x);    
    float phi = 2.0 * 3.14159265 * Xi.y;    
    float t1 = r * cos(phi);
    float t2 = r * sin(phi);
    float s = 0.5 * (1.0 + Vh.z);
    t2 = (1.0 - s) * sqrt(1.0 - t1 * t1) + s * t2;

    // Section 4.3: reprojection onto hemisphere
    const float3 Nh = t1 * T1 + t2 * T2 + sqrt(max(0.0, 1.0 - t1 * t1 - t2 * t2)) * Vh;
    
    // Section 3.4: transforming the normal back to the ellipsoid configuration
    return normalize(float3(R * Nh.x, R * Nh.y, max(0.02, Nh.z)));
}

float3 ImportanceSampleSmithGGX(const float2 Xi, const float3 N, const float3 V, const float R)
{
    const float3x3 basis = ComposeTBNFast(N);
    const float3 ve = transpose(basis) * -V;
    const float3 m = ImportanceSampleGGXVNDF(Xi, ve, R);

    // reflect viewer dir by a microfacet
    float3 l = reflect(-ve, m);
    
    return basis * l;
}

// Ray Tracing Gems, Chapter 16 "Sampling Transformations Zoo"
float3 ImportanceSampleLambert(const float2 Xi, const float3 N)
{
    float a = 1.0 - 2.0 * Xi.x;
    float b = sqrt(1.0 - a * a);
    float phi = 2.0 * 3.14159265 * Xi.y;

    a *= 0.98;
    b *= 0.98;

    return normalize(float3(N.x + b * cos(phi), N.y + b * sin(phi), N.z + a));
}
