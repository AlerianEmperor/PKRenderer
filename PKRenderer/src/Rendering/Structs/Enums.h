#pragma once
#include <PKAssets/PKAsset.h>

namespace PK::Rendering::Structs
{
    typedef PK::Assets::PKDescriptorType ResourceType;
    typedef PK::Assets::PKElementType ElementType;
    typedef PK::Assets::PKShaderStage ShaderStage;
    typedef PK::Assets::PKBlendFactor BlendFactor;
    typedef PK::Assets::PKBlendOp BlendOp;
    typedef PK::Assets::PKComparison Comparison;
    typedef PK::Assets::PKCullMode CullMode;
    typedef PK::Assets::PKRasterMode RasterMode;
    typedef PK::Assets::Shader::Type ShaderType;

    constexpr static const uint32_t PK_DESIRED_SWAP_CHAIN_IMAGE_COUNT = 4;
    constexpr static const uint32_t PK_MAX_FRAMES_IN_FLIGHT = 2;
    constexpr static const uint32_t PK_MAX_RENDER_TARGETS = 8;
    constexpr static const uint32_t PK_SHADOW_CASCADE_COUNT = 4;
    constexpr static const uint32_t PK_MAX_DESCRIPTOR_SETS = PK::Assets::PK_ASSET_MAX_DESCRIPTOR_SETS;
    constexpr static const uint32_t PK_MAX_DESCRIPTORS_PER_SET = PK::Assets::PK_ASSET_MAX_DESCRIPTORS_PER_SET;
    constexpr static const uint32_t PK_MAX_VERTEX_ATTRIBUTES = PK::Assets::PK_ASSET_MAX_VERTEX_ATTRIBUTES;
    constexpr static const uint32_t PK_MAX_UNBOUNDED_SIZE = PK::Assets::PK_ASSET_MAX_UNBOUNDED_SIZE;
    constexpr static const uint32_t PK_MAX_VIEWPORTS = 16;

    constexpr const static char* PK_VS_POSITION = PK::Assets::Mesh::PK_VS_POSITION;
    constexpr const static char* PK_VS_NORMAL = PK::Assets::Mesh::PK_VS_NORMAL;
    constexpr const static char* PK_VS_TANGENT = PK::Assets::Mesh::PK_VS_TANGENT;  
    constexpr const static char* PK_VS_COLOR = PK::Assets::Mesh::PK_VS_COLOR; 
    constexpr const static char* PK_VS_TEXCOORD0 = PK::Assets::Mesh::PK_VS_TEXCOORD0;
    constexpr const static char* PK_VS_TEXCOORD1 = PK::Assets::Mesh::PK_VS_TEXCOORD1;
    constexpr const static char* PK_VS_TEXCOORD2 = PK::Assets::Mesh::PK_VS_TEXCOORD2;
    constexpr const static char* PK_VS_TEXCOORD3 = PK::Assets::Mesh::PK_VS_TEXCOORD3;

    constexpr const static char PK_CUBE_FACE_RIGHT = 0;
    constexpr const static char PK_CUBE_FACE_LEFT = 1;
    constexpr const static char PK_CUBE_FACE_DOWN = 2;
    constexpr const static char PK_CUBE_FACE_UP = 3;
    constexpr const static char PK_CUBE_FACE_FRONT = 4;
    constexpr const static char PK_CUBE_FACE_BACK = 5;

    enum class APIType
    {
        Off,
        Vulkan,
        DX12
    };

    enum class QueueType
    {
        Transfer,
        Graphics,
        Compute,
        Present,
        MaxCount
    };

    enum class SamplerType : uint8_t
    {
        Sampler2D,
        Sampler2DArray,
        Sampler3D,
        Cubemap,
        CubemapArray,
    };

    enum class TextureBindMode : uint8_t
    {
        SampledTexture,
        Image,
        RenderTarget
    };

    enum class FilterMode : uint8_t
    {
        Point,
        Bilinear,
        Trilinear,
        Bicubic
    };

    enum class PolygonMode : uint8_t
    {
        Fill,
        Line,
        Point,
    };

    enum class WrapMode : uint8_t
    {
        Clamp,
        Repeat,
        Mirror,
        MirrorOnce,
        Border
    };

    enum class ColorMask
    {
        NONE = 0,
        R = 0x00000001,
        G = 0x00000002,
        B = 0x00000004,
        A = 0x00000008,
        RG = R | G,
        RGB = RG | B,
        RGBA = RGB | A,
    };

    enum class FrontFace : uint8_t
    {
        CounterClockwise,
        Clockwise,
    };

    enum class LoadOp : uint8_t
    {
        Keep,
        Clear,
        Discard,
    };

    enum class StoreOp : uint8_t
    {
        Store,
        Discard,
    };

    enum class BorderColor : uint8_t
    {
        FloatClear,
        IntClear,
        FloatBlack,
        IntBlack,
        FloatWhite,
        IntWhite
    };

    enum class InputRate : uint8_t
    {
        PerVertex,
        PerInstance
    };

    enum class BufferUsage : uint32_t
    {
        None = 0,

        GPUOnly     = 1,
        CPUOnly     = 2,
        CPUToGPU    = 3,
        GPUToCPU    = 4,
        CPUCopy     = 5,

        TransferDst           = 1 << 4,
        TransferSrc           = 1 << 5,
        Vertex                = 1 << 6,
        Index                 = 1 << 7,
        Constant              = 1 << 8,
        Storage               = 1 << 9,
        Indirect              = 1 << 10,
        Dynamic               = 1 << 11,
        PersistentStage       = 1 << 12,
        Sparse                = 1 << 13,
        AccelerationStructure = 1 << 14,
        InstanceInput         = 1 << 15,
        ShaderBindingTable    = 1 << 16,
        Concurrent            = 1 << 17,

        TypeBits = 7,
        AlignedTypes = Storage | Constant,
        DefaultVertex = GPUOnly | TransferDst | Vertex,
        SparseVertex = DefaultVertex | Sparse,
        DefaultIndex = GPUOnly | TransferDst | Index,
        SparseIndex = DefaultIndex | Sparse,
        DefaultConstant = GPUOnly | TransferDst | Constant,
        DefaultStorage = GPUOnly | TransferDst | Storage,
        PersistentStorage = DefaultStorage | PersistentStage,
        DefaultStaging = CPUOnly | TransferSrc,
        DefaultShaderBindingTable = GPUOnly | TransferDst | ShaderBindingTable
    };

    enum class TextureUsage : uint8_t
    {
        None = 0x0,
        RTColor = 0x1,
        RTDepth = 0x2,
        RTStencil = 0x4,
        Upload = 0x8,
        Sample = 0x10,
        Input = 0x20,
        Storage = 0x40,
        Concurrent = 0x80,
        Default = Upload | Sample,
        DefaultStorage = Upload | Sample | Storage,
        RTColorSample = RTColor | Sample,
        RTDepthSample = RTDepth | Sample,

        ValidRTTypes = RTColor | RTDepth | RTStencil,
        ValidRTColorUsages = Sample | Storage | Input,
        ValidRTDepthUsages = Sample | Input,
    };

    enum class RenderableFlags : uint8_t
    {
        Mesh = 1 << 0,
        Light = 1 << 1,
        Static = 1 << 2,
        CastShadows = 1 << 3,
        Cullable = 1 << 4,
        RayTraceable = 1 << 5,

        // Presets
        DefaultMesh = Mesh | Static | CastShadows | Cullable | RayTraceable,
        DefaultMeshNoShadows = Mesh | Static | Cullable,
    };

    enum class LightType : uint8_t
    {
        Point = 0,
        Spot = 1,
        Directional = 2,
        TypeCount
    };

    enum class TextureFormat : uint16_t 
    {
        Invalid = 0,
        R8, 
        R8_SNORM, 
        R8UI, 
        R8I, 
        Stencil8,
        R16F, 
        R16UI, 
        R16I,
        RG8, 
        RG8_SNORM, 
        RG8UI, 
        RG8I,
        RGB565,
        RGB9E5, // 9995 is actually 32 bpp but it's here for historical reasons.
        RGB5A1,
        RGBA4,
        Depth16,
        RGB8, 
        RGB8_SRGB, 
        RGB8_SNORM, 
        RGB8UI, 
        RGB8I,
        R32F, 
        R32UI, 
        R32I,
        RG16F, 
        RG16UI, 
        RG16I,
        B10G11R11UF,
        RGBA8, 
        RGBA8_SRGB, 
        RGBA8_SNORM, 
        BGRA8_SRGB,
        RGB10A2, 
        RGBA8UI, 
        RGBA8I,
        Depth32F, 
        Depth24_Stencil8, 
        Depth32F_Stencil8,
        RGB16F, 
        RGB16UI, 
        RGB16I,
        RG32F, 
        RG32UI, 
        RG32I,
        RGBA16, 
        RGBA16F, 
        RGBA16UI, 
        RGBA16I,
        RGB32F, 
        RGB32UI, 
        RGB32I,
        RGBA32F, 
        RGBA32UI, 
        RGBA32I,

        // compressed formats

        // Mandatory in GLES 3.0 and GL 4.3
        EAC_R11, 
        EAC_R11_SIGNED, 
        EAC_RG11, 
        EAC_RG11_SIGNED,
        ETC2_RGB8, 
        ETC2_SRGB8,
        ETC2_RGB8_A1, 
        ETC2_SRGB8_A1,
        ETC2_RGBA8, 
        ETC2_SRGBA8, 

        // Available everywhere except Android/iOS
        DXT4, 
        DXT1_RGB, 
        DXT1_RGBA, 
        DXT3_RGBA, 
        DXT5_RGBA,
        DXT1_SRGB, 
        DXT1_SRGBA, 
        DXT3_SRGBA, 
        DXT5_SRGBA,
        
        // ASTC formats are available with a GLES extension
        RGBA_ASTC_4x4,
        RGBA_ASTC_5x4,
        RGBA_ASTC_5x5,
        RGBA_ASTC_6x5,
        RGBA_ASTC_6x6,
        RGBA_ASTC_8x5,
        RGBA_ASTC_8x6,
        RGBA_ASTC_8x8,
        RGBA_ASTC_10x5,
        RGBA_ASTC_10x6,
        RGBA_ASTC_10x8,
        RGBA_ASTC_10x10,
        RGBA_ASTC_12x10,
        RGBA_ASTC_12x12,
        SRGB8_ALPHA8_ASTC_4x4,
        SRGB8_ALPHA8_ASTC_5x4,
        SRGB8_ALPHA8_ASTC_5x5,
        SRGB8_ALPHA8_ASTC_6x5,
        SRGB8_ALPHA8_ASTC_6x6,
        SRGB8_ALPHA8_ASTC_8x5,
        SRGB8_ALPHA8_ASTC_8x6,
        SRGB8_ALPHA8_ASTC_8x8,
        SRGB8_ALPHA8_ASTC_10x5,
        SRGB8_ALPHA8_ASTC_10x6,
        SRGB8_ALPHA8_ASTC_10x8,
        SRGB8_ALPHA8_ASTC_10x10,
        SRGB8_ALPHA8_ASTC_12x10,
        SRGB8_ALPHA8_ASTC_12x12,
    };

    enum class Cookie : uint8_t
    {
        Circle0,
        Circle1,
        Circle2,
        Square0,
        Square1,
        Square2,
        Triangle,
        Star,
    };


    enum class RayTracingShaderGroup
    {
        RayGeneration,
        Miss,
        Hit,
        Callable, //@TODO Add callable support
        MaxCount
    };

    enum class RayTracingShaderGroupStageMask
    {
        RayGeneration = 1 << (uint32_t)ShaderStage::RayGeneration,
        Miss = 1 << (uint32_t)ShaderStage::RayMiss,
        Hit = 1 << (uint32_t)ShaderStage::RayClosestHit | 1 << (uint32_t)ShaderStage::RayAnyHit | 1 << (uint32_t)ShaderStage::RayIntersection,
        Callable = 0, 
        MaxCount
    };

    constexpr const static RayTracingShaderGroup PK_SHADER_STAGE_RAYTRACING_GROUP[(uint32_t)ShaderStage::MaxCount] =
    {
        RayTracingShaderGroup::MaxCount,        //Vertex,
        RayTracingShaderGroup::MaxCount,        //TesselationControl,
        RayTracingShaderGroup::MaxCount,        //TesselationEvaluation,
        RayTracingShaderGroup::MaxCount,        //Geometry,
        RayTracingShaderGroup::MaxCount,        //Fragment,
        RayTracingShaderGroup::MaxCount,        //Compute,
        RayTracingShaderGroup::RayGeneration,   //RayGeneration,
        RayTracingShaderGroup::Miss,            //RayMiss,
        RayTracingShaderGroup::Hit,             //RayClosestHit,
        RayTracingShaderGroup::Hit,             //RayAnyHit,
        RayTracingShaderGroup::Hit,             //RayIntersection,
    };


    #define PK_DECLARE_ENUM_OPERATORS(Type) \
    static constexpr Type operator | (const Type& a, const Type& b) { return (Type)((uint32_t)a | (uint32_t)b); } \
    static constexpr Type operator |= (const Type& a, const Type& b) { return a | b; } \
    static constexpr Type operator & (const Type& a, const Type& b) { return (Type)((uint32_t)a & (uint32_t)b); } \
    static constexpr Type operator & (const Type& a, const int& b) { return (Type)((uint32_t)a & (uint32_t)b); } \
    static constexpr bool operator == (const Type& a, const int& b) { return (uint32_t)a == b; } \
    static constexpr bool operator != (const Type& a, const int& b) { return (uint32_t)a != b; } \

    PK_DECLARE_ENUM_OPERATORS(ColorMask)
    PK_DECLARE_ENUM_OPERATORS(BufferUsage)
    PK_DECLARE_ENUM_OPERATORS(TextureUsage)
    PK_DECLARE_ENUM_OPERATORS(RenderableFlags)
    PK_DECLARE_ENUM_OPERATORS(RayTracingShaderGroupStageMask)

    #undef PK_DECLARE_ENUM_OPERATORS

    namespace ElementConvert
    {
        inline uint16_t Size(ElementType format) { return (uint16_t)PK::Assets::GetElementSize(format); }
        inline uint16_t Alignment(ElementType format) { return (uint16_t)PK::Assets::GetElementAlignment(format); }
        inline uint16_t Components(ElementType format) { return (uint16_t)PK::Assets::GetElementComponents(format); }
    }
}