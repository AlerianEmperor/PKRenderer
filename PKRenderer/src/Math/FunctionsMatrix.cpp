#include "PrecompiledHeader.h"
#include "FunctionsMatrix.h"
#include "FunctionsIntersect.h"

namespace PK::Math::Functions
{
    float4x4 GetMatrixTRS(const float3& position, const quaternion& rotation, const float3& scale)
    {
        float qxx(rotation.x * rotation.x);
        float qyy(rotation.y * rotation.y);
        float qzz(rotation.z * rotation.z);
        float qxz(rotation.x * rotation.z);
        float qxy(rotation.x * rotation.y);
        float qyz(rotation.y * rotation.z);
        float qwx(rotation.w * rotation.x);
        float qwy(rotation.w * rotation.y);
        float qwz(rotation.w * rotation.z);

        float4x4 m(1.0f);
        m[3].xyz = position;
        m[0][0] = scale[0] * (1.0f - 2.0f * (qyy + qzz));
        m[0][1] = scale[0] * (2.0f * (qxy + qwz));
        m[0][2] = scale[0] * (2.0f * (qxz - qwy));

        m[1][0] = scale[1] * (2.0f * (qxy - qwz));
        m[1][1] = scale[1] * (1.0f - 2.0f * (qxx + qzz));
        m[1][2] = scale[1] * (2.0f * (qyz + qwx));

        m[2][0] = scale[2] * (2.0f * (qxz + qwy));
        m[2][1] = scale[2] * (2.0f * (qyz - qwx));
        m[2][2] = scale[2] * (1.0f - 2.0f * (qxx + qyy));

        return m;
    }

    float4x4 GetMatrixTRS(const float3& position, const float3& euler, const float3& scale)
    {
        return GetMatrixTRS(position, glm::quat(euler), scale);
    }

    float4x4 GetMatrixInvTRS(const float3& position, const quaternion& rotation, const float3& scale)
    {
        return glm::inverse(GetMatrixTRS(position, rotation, scale));
    }

    float4x4 GetMatrixInvTRS(const float3& position, const float3& euler, const float3& scale)
    {
        return GetMatrixInvTRS(position, glm::quat(euler), scale);
    }

    float4x4 GetMatrixTR(const float3& position, const quaternion& rotation)
    {
        float qxx(rotation.x * rotation.x);
        float qyy(rotation.y * rotation.y);
        float qzz(rotation.z * rotation.z);
        float qxz(rotation.x * rotation.z);
        float qxy(rotation.x * rotation.y);
        float qyz(rotation.y * rotation.z);
        float qwx(rotation.w * rotation.x);
        float qwy(rotation.w * rotation.y);
        float qwz(rotation.w * rotation.z);

        float4x4 m(1.0f);
        m[3].xyz = position;
        m[0][0] = 1.0f - 2.0f * (qyy + qzz);
        m[0][1] = 2.0f * (qxy + qwz);
        m[0][2] = 2.0f * (qxz - qwy);

        m[1][0] = 2.0f * (qxy - qwz);
        m[1][1] = 1.0f - 2.0f * (qxx + qzz);
        m[1][2] = 2.0f * (qyz + qwx);

        m[2][0] = 2.0f * (qxz + qwy);
        m[2][1] = 2.0f * (qyz - qwx);
        m[2][2] = 1.0f - 2.0f * (qxx + qyy);

        return m;
    }

    float4x4 GetPerspective(float fov, float aspect, float zNear, float zFar)
    {
        const float tanHalfFovy = tan(fov * PK_FLOAT_DEG2RAD / 2.0f);
        float4x4 proj(0.0f);
        proj[0][0] = 1.0f / (aspect * tanHalfFovy);
        proj[1][1] = 1.0f / (tanHalfFovy);
        proj[2][2] = (zFar + zNear) / (zFar - zNear);
        proj[2][3] = 1.0;
        proj[3][2] = -(2.0f * zFar * zNear) / (zFar - zNear);
        return proj;
    }

    float4x4 GetOrtho(float left, float right, float bottom, float top, float zNear, float zFar)
    {
        const float rcpRL = 1.0f / (right - left);
        const float rcpTB = 1.0f / (top - bottom);
        const float rcpFN = 1.0f / (zFar - zNear);

        float4x4 Result(1);
        Result[0][0] = -2.0f * rcpRL;
        Result[1][1] = -2.0f * rcpTB;
        Result[2][2] = 2.0f * rcpFN;
        Result[3][0] = (right + left) * rcpRL;
        Result[3][1] = (top + bottom) * rcpTB;
        Result[3][2] = -(zFar + zNear) * rcpFN;
        return Result;
    }

    float4x4 GetOffsetPerspective(float left, float right, float bottom, float top, float fovy, float aspect, float zNear, float zFar)
    {
        const float tanHalfFovy = tan((fovy * PK_FLOAT_DEG2RAD) / 2.0f);
        const float rcpRL = 1.0f / (right - left);
        const float rcpTB = 1.0f / (top - bottom);
        const float rcpFN = 1.0f / (zFar - zNear);

        float4x4 Result(0);
        Result[0][0] = (2.0f * rcpRL) / (aspect * tanHalfFovy);
        Result[1][1] = (2.0f * rcpTB) / tanHalfFovy;
        Result[2][0] = -(right + left) * rcpRL;
        Result[2][1] = -(top + bottom) * rcpTB;
        Result[2][2] = -(zFar + zNear) * rcpFN;
        Result[2][3] = 1.0f;
        Result[3][2] = (2.0f * zFar * zNear) * rcpFN;

        return Result;
    }

    float4x4 GetPerspectiveSubdivision(uint32_t index, const int3& gridSize, float fovy, float aspect, float znear, float zfar)
    {
        int3 coord;
        coord.x = index % gridSize.x;
        coord.y = (index / gridSize.x) % gridSize.y;
        coord.z = index / (gridSize.x * gridSize.y);

        float ix = 2.0f / gridSize.x;
        float iy = 2.0f / gridSize.y;
        float iz = 1.0f / gridSize.z;

        float x = -1.0f + (2.0f * (float)coord.x) / gridSize.x;
        float y = -1.0f + (2.0f * (float)coord.y) / gridSize.y;
        float z = (float)coord.z / gridSize.z;

        float zrange = zfar - znear;

        return GetOffsetPerspective(x, x + ix, y, y + iy, fovy, aspect, znear + zrange * z, znear + zrange * (z + iz));
    }

    float4x4 GetFrustumBoundingOrthoMatrix(const float4x4& worldToLocal, const float4x4& inverseViewProjection, const float3& paddingLD, const float3& paddingRU, float* outZNear, float* outZFar)
    {
        auto aabb = GetInverseFrustumBounds(worldToLocal * inverseViewProjection);

        *outZNear = (aabb.min.z + paddingLD.z);
        *outZFar = (aabb.max.z + paddingRU.z);

        return GetOrtho(aabb.min.x + paddingLD.x,
            aabb.max.x + paddingRU.x,
            aabb.min.y + paddingLD.y,
            aabb.max.y + paddingRU.y,
            aabb.min.z + paddingLD.z,
            aabb.max.z + paddingRU.z) * worldToLocal;
    }

    float GetShadowCascadeMatrices(const float4x4& worldToLocal, const float4x4& inverseViewProjection, const float* zPlanes, float zPadding, uint32_t count, float4x4* matrices)
    {
        auto matrix = worldToLocal * inverseViewProjection;
        auto minNear = std::numeric_limits<float>().max();
        auto maxFar = -std::numeric_limits<float>().max();
        auto zrange = zPlanes[count] - zPlanes[0];

        BoundingBox* aabbs = reinterpret_cast<BoundingBox*>(alloca(sizeof(BoundingBox) * count));

        for (auto i = 0u; i < count; ++i)
        {
            auto lnear = zPlanes[i] / zrange;
            auto lfar = zPlanes[i + 1] / zrange;

            aabbs[i] = GetInverseFrustumBounds(matrix, lnear, lfar);

            if (aabbs[i].min.z < minNear)
            {
                minNear = aabbs[i].min.z;
            }

            if (aabbs[i].max.z > maxFar)
            {
                maxFar = aabbs[i].max.z;
            }
        }

        for (auto i = 0u; i < count; ++i)
        {
            matrices[i] = GetOrtho(
                aabbs[i].min.x,
                aabbs[i].max.x,
                aabbs[i].min.y,
                aabbs[i].max.y,
                minNear + zPadding,
                aabbs[i].max.z) * worldToLocal;
        }

        minNear += zPadding;
        return maxFar - minNear;
    }

    float4x4 GetPerspectiveJittered(const float4x4& matrix, const float2& jitter)
    {
        float4x4 returnValue = matrix;
        returnValue[2][0] += jitter.x;
        returnValue[2][1] += jitter.y;
        return returnValue;
    }
}