#version 460
#MaterialProperty float4 _Color
#MaterialProperty float4 _EmissionColor
#define PK_ACTIVE_BRDF BRDF_PBS_DEFAULT_SS
#include includes/SharedSurfaceShading.glsl
#include includes/Noise.glsl

#pragma PROGRAM_VERTEX
float3 GerstnerWave(float4 wave, float3 p, inout float3 tangent, inout float3 binormal)
{
    float steepness = wave.z;
    float wavelength = wave.w;
    float k = 2 * PK_PI / wavelength;
    float c = sqrt(9.8 / k);
    float2 d = normalize(wave.xy);
    float f = k * (dot(d, p.xz) - c * pk_Time.y);
    float a = steepness / k;

    tangent += float3(
        -d.x * d.x * (steepness * sin(f)),
        d.x * (steepness * cos(f)),
        -d.x * d.y * (steepness * sin(f))
        );
    binormal += float3(
        -d.x * d.y * (steepness * sin(f)),
        d.y * (steepness * cos(f)),
        -d.y * d.y * (steepness * sin(f))
        );
    return float3(
        d.x * (a * cos(f)),
        a * sin(f),
        d.y * (a * cos(f))
        );
}

void PK_SURFACE_FUNC_VERT(inout SurfaceFragmentVaryings surf)
{
    float4 wavea = float4(1, 0, 0.4f, 10);
    float4 waveb = float4(1, 1, 0.1f, 3);
    float4 wavec = float4(-1, 1, 0.2f, 5);

    float3 gridPoint = surf.vs_WORLDPOSITION.xyz;
    float3 tangent = float3(1, 0, 0);
    float3 binormal = float3(0, 0, 1);
    float3 p = gridPoint;

    p += GerstnerWave(wavea, gridPoint, tangent, binormal) * 0.8f;
    p += GerstnerWave(waveb, gridPoint, tangent, binormal) * 0.8f;
    p += GerstnerWave(wavec, gridPoint, tangent, binormal) * 0.8f;

    float3 normal = normalize(cross(binormal, tangent));
    normal = normalize(normal);

    surf.vs_WORLDPOSITION.xyz = p;

#if defined(PK_META_PASS_GIVOXELIZE)
    surf.vs_WORLDPOSITION.y -= 1.0f;
#endif

    surf.vs_NORMAL = normal;
}

#pragma PROGRAM_FRAGMENT
void PK_SURFACE_FUNC_FRAG(in SurfaceFragmentVaryings varyings, inout SurfaceData surf)
{
    float2 uv = varyings.vs_TEXCOORD0;

    float yorigin = pk_MATRIX_M[3].y;

    float3 noise;
    noise.xy = NoiseCell(int2(surf.worldpos.xz * 8.0f + surf.worldpos.yy * 30.0f));
    noise.y = 1.0f;

    surf.normal = normalize(PK_SURF_MESH_NORMAL + noise * 0.15f);

    float nv = saturate(1.0f - dot(surf.normal, surf.viewdir));

    nv = pow4(nv);

    float depth = unlerp_sat(yorigin - 1.0f, yorigin + 5.0f, surf.worldpos.y);

    surf.albedo = lerp(float3(0, 0.01f, 0.02f), float3(0.3, 0.6, 1.0f), depth * depth);
    surf.albedo = lerp(surf.albedo, float3(0.25f, 0.5f, 0.8f), nv); //PK_ACCESS_INSTANCED_PROP(_Color).xyz;
    surf.alpha = 1.0f;

    surf.subsurface_distortion = 0.2f;
    surf.subsurface_power = 2.0f;
    surf.subsurface_thickness = 20.5f * depth;

    surf.metallic = max(0.0f, noise.y * 0.5f);
    surf.roughness = max(0.0f, noise.x * 0.1f);
    surf.occlusion = 1.0f;
}