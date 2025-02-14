#include "PrecompiledHeader.h"
#include "PassVolumeFog.h"
#include "Rendering/HashCache.h"

namespace PK::Rendering::Passes
{
    using namespace Math;
    using namespace Core;
    using namespace Core::Services;
    using namespace Utilities;
    using namespace Structs;
    using namespace Objects;

    constexpr static const uint3 VolumeResolution = { 160u, 90u, 128u };

    PassVolumeFog::PassVolumeFog(AssetDatabase* assetDatabase, const ApplicationConfig* config)
    {
        TextureDescriptor descriptor;
        descriptor.samplerType = SamplerType::Sampler3D;
        descriptor.format = TextureFormat::RGBA16F;
        descriptor.sampler.filterMin = FilterMode::Trilinear;
        descriptor.sampler.filterMag = FilterMode::Trilinear;
        descriptor.sampler.wrap[0] = WrapMode::Clamp;
        descriptor.sampler.wrap[1] = WrapMode::Clamp;
        descriptor.sampler.wrap[2] = WrapMode::Clamp;
        descriptor.resolution = VolumeResolution;
        descriptor.usage = TextureUsage::Sample | TextureUsage::Storage | TextureUsage::Concurrent;

        m_volumeInject = Texture::Create(descriptor, "Fog.InjectVolume");
        m_volumeScatter = Texture::Create(descriptor, "Fog.ScatterVolume");
        m_depthTiles = Buffer::Create(ElementType::Uint, VolumeResolution.x * VolumeResolution.y, BufferUsage::DefaultStorage, "Fog.DepthTiles");

        m_computeInject = assetDatabase->Find<Shader>("CS_VolumeFogLightDensity");
        m_computeScatter = assetDatabase->Find<Shader>("CS_VolumeFogScatter");
        m_computeDepthTiles = assetDatabase->Find<Shader>("CS_VolumeFogDepthMax");
        m_shaderComposite = assetDatabase->Find<Shader>("SH_VS_VolumeFogComposite");

        auto hash = HashCache::Get();

        m_volumeResources = CreateRef<ConstantBuffer>(BufferLayout(
            {
                { ElementType::Float4, hash->pk_Volume_WindDir },
                { ElementType::Float,  hash->pk_Volume_ConstantFog },
                { ElementType::Float,  hash->pk_Volume_HeightFogExponent },
                { ElementType::Float,  hash->pk_Volume_HeightFogOffset },
                { ElementType::Float,  hash->pk_Volume_HeightFogAmount },
                { ElementType::Float,  hash->pk_Volume_Density },
                { ElementType::Float,  hash->pk_Volume_Intensity },
                { ElementType::Float,  hash->pk_Volume_Anisotropy },
                { ElementType::Float,  hash->pk_Volume_NoiseFogAmount },
                { ElementType::Float,  hash->pk_Volume_NoiseFogScale },
                { ElementType::Float,  hash->pk_Volume_WindSpeed },
            }), "Fog.Parameters");

        OnUpdateParameters(config);

        GraphicsAPI::SetImage(hash->pk_Volume_Inject, m_volumeInject.get());
        GraphicsAPI::SetImage(hash->pk_Volume_Scatter, m_volumeScatter.get());
        GraphicsAPI::SetTexture(hash->pk_Volume_InjectRead, m_volumeInject.get());
        GraphicsAPI::SetTexture(hash->pk_Volume_ScatterRead, m_volumeScatter.get());
        GraphicsAPI::SetBuffer(hash->pk_VolumeResources, m_volumeResources->GetBuffer());
        GraphicsAPI::SetBuffer(hash->pk_VolumeMaxDepths, m_depthTiles.get());
    }

    void PassVolumeFog::ComputeDepthTiles(Objects::CommandBuffer* cmd, const Math::uint3& resolution)
    {
        cmd->BeginDebugScope("VolumetricFog.DepthTiles", PK_COLOR_MAGENTA);
        cmd->Clear(m_depthTiles.get(), 0, sizeof(uint32_t) * VolumeResolution.x * VolumeResolution.y, 0u);
        cmd->Dispatch(m_computeDepthTiles, 0, { resolution.x, resolution.y, 1 });
        cmd->EndDebugScope();
    }

    void PassVolumeFog::Compute(Objects::CommandBuffer* cmd)
    {
        cmd->BeginDebugScope("VolumetricFog.Injection", PK_COLOR_MAGENTA);
        cmd->Dispatch(m_computeInject, 0, VolumeResolution);
        cmd->EndDebugScope();
        cmd->BeginDebugScope("VolumetricFog.Scattering", PK_COLOR_MAGENTA);
        cmd->Dispatch(m_computeScatter, 0, { VolumeResolution.x, VolumeResolution.y, 1u });
        cmd->EndDebugScope();
    }

    void PassVolumeFog::Render(CommandBuffer* cmd, RenderTexture* destination)
    {
        cmd->BeginDebugScope("VolumetricFog.Composite", PK_COLOR_MAGENTA);

        cmd->SetRenderTarget(destination, { 0 }, false, true);
        cmd->Blit(m_shaderComposite, 0);

        cmd->EndDebugScope();
    }

    void PassVolumeFog::OnUpdateParameters(const ApplicationConfig* config)
    {
        auto hash = HashCache::Get();
        m_volumeResources->Set<float4>(hash->pk_Volume_WindDir, float4(config->VolumeWindDirection.value, 0.0f));
        m_volumeResources->Set<float>(hash->pk_Volume_ConstantFog, config->VolumeConstantFog);
        m_volumeResources->Set<float>(hash->pk_Volume_HeightFogExponent, config->VolumeHeightFogExponent);
        m_volumeResources->Set<float>(hash->pk_Volume_HeightFogOffset, config->VolumeHeightFogOffset);
        m_volumeResources->Set<float>(hash->pk_Volume_HeightFogAmount, config->VolumeHeightFogAmount);
        m_volumeResources->Set<float>(hash->pk_Volume_Density, config->VolumeDensity);
        m_volumeResources->Set<float>(hash->pk_Volume_Intensity, config->VolumeIntensity);
        m_volumeResources->Set<float>(hash->pk_Volume_Anisotropy, config->VolumeAnisotropy);
        m_volumeResources->Set<float>(hash->pk_Volume_NoiseFogAmount, config->VolumeNoiseFogAmount);
        m_volumeResources->Set<float>(hash->pk_Volume_NoiseFogScale, config->VolumeNoiseFogScale);
        m_volumeResources->Set<float>(hash->pk_Volume_WindSpeed, config->VolumeWindSpeed);
        m_volumeResources->FlushBuffer(QueueType::Transfer);
    }
}