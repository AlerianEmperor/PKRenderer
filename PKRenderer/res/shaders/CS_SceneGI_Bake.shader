#version 460
#pragma PROGRAM_COMPUTE
#include includes/Lighting.glsl
#include includes/SharedSceneGI.glsl
#include includes/Reconstruction.glsl
#include includes/SharedHistogram.glsl

struct SampleIndirect
{
    SH sh;
    float3 direction;
    float luminance;
};

float3 SampleRadiance(const int2 coord, const float3 origin, const float3 direction, const float dist, const float roughness)
{
    const float3 worldpos = origin + direction * dist;
    float3 clipuvw;

    // Try sample previous forward output for better sampling.
    if (TryGetWorldToPrevClipUVW(worldpos, clipuvw))
    {
        float sdepth = SamplePreviousLinearDepth(clipuvw.xy);
        bool isMiss = sdepth > pk_ProjectionParams.z - 1e-4f && dist >= PK_GI_RAY_MAX_DISTANCE - 0.01f;
        
        float rdepth = LinearizeDepth(clipuvw.z);
        float sviewz = -SamplePreviousViewNormal(clipuvw.xy).z + 0.15f;
        bool isDepthValid = abs(sdepth - rdepth) < (rdepth * 0.01f / sviewz);

        float2 deltacoord = abs(coord - (clipuvw.xy * pk_ScreenParams.xy));
        bool isCoordValid = dot(deltacoord, deltacoord) > 2.0f;

        if (isCoordValid && (isMiss || isDepthValid))
        {
            return tex2D(pk_ScreenColorPrevious, clipuvw.xy).rgb;
        }
    }

    const float level = roughness * roughness * log2(max(1.0f, dist) / PK_GI_VOXEL_SIZE);
    const float4 voxel = SampleGI_WS(worldpos, level);

    const float3 env = SampleEnvironment(OctaUV(direction), roughness);
    const float envclip = saturate(PK_GI_RAY_MAX_DISTANCE * (1.0f - (dist / PK_GI_RAY_MAX_DISTANCE)));
    const float alpha = max(voxel.a, 1.0f / (PK_GI_VOXEL_MAX_MIP * PK_GI_VOXEL_MAX_MIP));

    return lerp(env, voxel.rgb / alpha, envclip);
}

SampleIndirect GetSample(const int2 coord, const int layer, const float3 normal)
{
    SampleIndirect s;
    s.sh = SampleGI_SH(coord, layer);
    s.direction = normalize(s.sh.Y.wyz + normal * 0.01f);
    s.luminance = SHToLuminance(s.sh, normal);
    return s;
}

SampleIndirect GetSampleNew(const int2 coord, const float3 origin, const float3 normal, const float3 direction, const float hitDistance, const float vxconeSize)
{
    const float3 radiance = SampleRadiance(coord, origin, direction, hitDistance, vxconeSize);

    SampleIndirect s;
    s.sh = IrradianceToSH(radiance, direction);
    s.direction = direction;
    s.luminance = SHToLuminance(s.sh, normal);
    return s;

}

layout(local_size_x = 16, local_size_y = 4, local_size_z = 1) in;
void main()
{
    int2 size = int2(pk_ScreenSize.xy);
    int2 coord = int2(gl_GlobalInvocationID.xy);

    if (Any_GEqual(coord, size))
    {
        return;
    }

    SceneGIMeta meta = SampleGI_Meta(coord);

    if (meta.isOOB)
    {
        return;
    }

    const float4 NR = SampleWorldNormalRoughness(coord);
    const float3 N = NR.xyz;
    const float3 O = SampleWorldPosition(coord, size);
    const float3 V = normalize(O - pk_WorldSpaceCameraPos.xyz);
    const float2 Xi = GetSampleOffset(coord, pk_FrameIndex);

    const float2 hitDist = imageLoad(pk_ScreenGI_Hits, coord).xy;
    const float exposure = GetAutoExposure();
    const uint history = 1u + meta.history;
    
    SampleIndirect diff = GetSample(coord, PK_GI_DIFF_LVL, N);
    SampleIndirect spec = GetSample(coord, PK_GI_SPEC_LVL, N);
    SampleIndirect sDiff = GetSampleNew(coord, O, N, ImportanceSampleLambert(Xi, N), hitDist.x, 0.5f);
    SampleIndirect sSpec = GetSampleNew(coord, O, N, ImportanceSampleSmithGGX(Xi, N, V, NR.w), hitDist.y, 0.0f);

    const float wHistory = 1.0f / history;
    const float wDiff = max(0.01f, wHistory);
    const float wSpec = max(exp(-NR.w) * 0.1f, wHistory);

    diff.sh = InterpolateSH(diff.sh, sDiff.sh, wDiff);
    spec.sh = InterpolateSH(spec.sh, sSpec.sh, wSpec);

    const float exposedLum = sqrt(sDiff.luminance) * exposure;

    meta.moments = lerp(meta.moments, float2(exposedLum, pow2(exposedLum)), wDiff);
    meta.moments /= lerp(1.0f, 0.25f, 1.0f / history);

    StoreGI_Meta(coord, meta);
    StoreGI_SH(coord, PK_GI_DIFF_LVL, diff.sh);
    StoreGI_SH(coord, PK_GI_SPEC_LVL, spec.sh);
}