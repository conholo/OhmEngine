#include "ohmpch.h"
#include "EnvironmentMapPipeline.h"

#include "Renderer.h"
#include "Shader.h"
#include "Texture2D.h"
#include "TextureLibrary.h"

namespace Ohm
{
    namespace EnvironmentUtils
    {
        std::unordered_map<EnvironmentPipelineType, std::string> EnvTypeToStringMap
        {
        	{EnvironmentPipelineType::BlackCube, "Empty"},
            {EnvironmentPipelineType::FromShader, "From Shader"},
            {EnvironmentPipelineType::FromShader, "From File"}
        };

        std::unordered_map<std::string, EnvironmentPipelineType> GetTypeMap()
        {
            std::unordered_map<std::string, EnvironmentPipelineType> NameToTypeMap
            {
        	    {"Empty", EnvironmentPipelineType::BlackCube},
                {"From Shader", EnvironmentPipelineType::FromShader},
                {"From File", EnvironmentPipelineType::FromFile}
            };
            return NameToTypeMap;
        }

        std::string PipelineTypeToString(EnvironmentPipelineType Type)
        {
            return EnvTypeToStringMap[Type];
        }

        EnvironmentPipelineType NameToPipelineType(const std::string& Name)
        {
            if(Name == "Empty") return EnvironmentPipelineType::BlackCube;
            if(Name == "From Shader") return EnvironmentPipelineType::FromShader;
            if(Name == "From File") return EnvironmentPipelineType::FromFile;
            return EnvironmentPipelineType::BlackCube;
        }
    }

    EnvironmentMapPipeline::EnvironmentMapPipeline()
    {
        m_Specification = CreateRef<EnvironmentMapSpecification>(EnvironmentPipelineType::BlackCube);
    }
    EnvironmentMapPipeline::~EnvironmentMapPipeline() = default;

    void EnvironmentMapPipeline::BuildFromBlackTextureCube() const
    {
        m_Specification->PipelineType = EnvironmentPipelineType::BlackCube;
    }

    void EnvironmentMapPipeline::BuildFromShader(const std::string& CreationShaderName) const
    {
        m_Specification->PipelineType = EnvironmentPipelineType::FromShader;
        GenerateFromShader(CreationShaderName);
    }

    void EnvironmentMapPipeline::BuildFromEquirectangularImage(const std::string& FilePath) const
    {
        m_Specification->PipelineType = EnvironmentPipelineType::FromFile;
        GenerateFromFile(FilePath);
    }

    void EnvironmentMapPipeline::GenerateFromFile(const std::string& filePath) const
    {
        OHM_CORE_TRACE("-----Starting Environment Map Pipeline using file '{}'...-----", filePath);
        const Texture2DSpecification Specification
        {
            TextureUtils::WrapMode::Repeat,
            TextureUtils::WrapMode::Repeat,
            TextureUtils::FilterMode::Linear,
            TextureUtils::FilterMode::Linear,
            TextureUtils::ImageInternalFormat::FromImage,
            TextureUtils::ImageDataLayout::FromImage,
            TextureUtils::ImageDataType::UByte
        };
        
        const Ref<Texture2D> Equirectangular = TextureLibrary::LoadTexture2D(Specification, filePath);
        ASSERT(Equirectangular, "\tEnvironment Pipeline Error: Unable to create Equirectangular Texture from path '{}'.", filePath)

        uint32_t EnvironmentMapResolution = m_Specification->EnvironmentMapResolution;
        uint32_t ThreadGroupSize = m_Specification->GetThreadGroupSize();
        
        TextureCubeSpecification EnvironmentCubeSpec =
        {
            TextureUtils::WrapMode::ClampToEdge,
            TextureUtils::WrapMode::ClampToEdge,
            TextureUtils::WrapMode::ClampToEdge,
            TextureUtils::FilterMode::LinearMipLinear,
            TextureUtils::FilterMode::Linear,
            TextureUtils::ImageInternalFormat::RGBA32F,
            TextureUtils::ImageDataLayout::RGBA,
            TextureUtils::ImageDataType::Float,
            EnvironmentMapResolution
        };
        const glm::ivec3 ThreadGroups = glm::ivec3(
            glm::ceil(EnvironmentCubeSpec.Dimension / ThreadGroupSize),
            glm::ceil(EnvironmentCubeSpec.Dimension / ThreadGroupSize),
            6);

        EnvironmentCubeSpec.Name = m_Specification->EnvironmentMapName + "-EnvironmentRadianceCubeUnfiltered";
        const Ref<TextureCube> EnvironmentCubeUnfiltered = TextureLibrary::LoadTextureCube(EnvironmentCubeSpec, true);
        EnvironmentCubeSpec.Name = m_Specification->EnvironmentMapName + "-EnvironmentRadianceCubeFiltered";
        const Ref<TextureCube> EnvironmentCubeFiltered = TextureLibrary::LoadTextureCube(EnvironmentCubeSpec, true);

        // EquirectangularToCubemap
        {
            OHM_CORE_TRACE("\tDispatching 'EquirectangularToCubemap' Shader...");
            EnvironmentCubeUnfiltered->BindToImageSlot(0, 0, TextureUtils::TextureAccessLevel::WriteOnly, TextureUtils::TextureShaderDataFormat::RGBA32F);
            ShaderLibrary::Get("EquirectangularToCubemap")->Bind();
            ShaderLibrary::Get("EquirectangularToCubemap")->UploadUniformInt("sampler_EquirectangularTexture", 0);
            ShaderLibrary::Get("EquirectangularToCubemap")->DispatchCompute(ThreadGroups.x, ThreadGroups.y, ThreadGroups.z);
        }
        // EquirectangularToCubemap

        // EnvironmentFilter
        {
            const uint32_t MipCount = TextureUtils::CalculateMipLevelCount(EnvironmentMapResolution, EnvironmentMapResolution);
            float deltaRoughness = 1.0f / glm::max(static_cast<float>(MipCount) - 1.0f, 1.0f);
            
            for(uint32_t i = 0, size = EnvironmentMapResolution; i < MipCount; i++, size /= 2)
            {
                OHM_CORE_TRACE("\tDispatching 'EnvironmentFilter' at the {}th mip of the unfiltered environment map...", i);
                const uint32_t ThreadGroup = glm::max(1u, size / 32);
                float Roughness = i * deltaRoughness;
                Roughness = glm::max(Roughness, 0.05f);
                
                EnvironmentCubeFiltered->BindToImageSlot(0, i, TextureUtils::TextureAccessLevel::ReadWrite, TextureUtils::TextureShaderDataFormat::RGBA32F);
                EnvironmentCubeUnfiltered->BindToSamplerSlot(0);
                ShaderLibrary::Get("EnvironmentMipFilter")->Bind();
                ShaderLibrary::Get("EnvironmentMipFilter")->UploadUniformInt("sampler_InputCube", 0);
                ShaderLibrary::Get("EnvironmentMipFilter")->UploadUniformInt("MipOutputWidth", size);
                ShaderLibrary::Get("EnvironmentMipFilter")->UploadUniformInt("MipOutputHeight", size);
                ShaderLibrary::Get("EnvironmentMipFilter")->UploadUniformFloat("Roughness", Roughness);
                ShaderLibrary::Get("EnvironmentMipFilter")->DispatchCompute(ThreadGroup, ThreadGroup, ThreadGroups.z);
            }
        }
        // EnvironmentFilter

        // EnvironmentIrradiance
        {
            uint32_t IrradianceMapSize = m_Specification->GetIrradianceMapSize();
            uint32_t IrradianceMapComputeSamples = m_Specification->IrradianceMapComputeSamples;
            OHM_CORE_TRACE("\tDispatching 'EnvironmentIrradiance'...");

            TextureCubeSpecification IrradianceSpec =
            {
                TextureUtils::WrapMode::ClampToEdge,
                TextureUtils::WrapMode::ClampToEdge,
                TextureUtils::WrapMode::ClampToEdge,
                TextureUtils::FilterMode::LinearMipLinear,
                TextureUtils::FilterMode::Linear,
                TextureUtils::ImageInternalFormat::RGBA32F,
                TextureUtils::ImageDataLayout::RGBA,
                TextureUtils::ImageDataType::Float,
                IrradianceMapSize,
            };

            IrradianceSpec.Name = m_Specification->EnvironmentMapName + "-EnvironmentIrradianceCube";
            
            const glm::ivec3 IrradianceThreadGroups = glm::ivec3(
            glm::ceil(IrradianceSpec.Dimension / ThreadGroupSize),
            glm::ceil(IrradianceSpec.Dimension / ThreadGroupSize),
            6);

            Ref<TextureCube> IrradianceMap = TextureLibrary::LoadTextureCube(IrradianceSpec, true);
            IrradianceMap->BindToImageSlot(0, 0, TextureUtils::TextureAccessLevel::WriteOnly, TextureUtils::TextureShaderDataFormat::RGBA32F);
            EnvironmentCubeFiltered->BindToSamplerSlot(0);
            ShaderLibrary::Get("EnvironmentIrradiance")->Bind();
            ShaderLibrary::Get("EnvironmentIrradiance")->UploadUniformInt("sampler_RadianceMap", 0);
            ShaderLibrary::Get("EnvironmentIrradiance")->UploadUniformInt("EnvironmentSamples", IrradianceMapComputeSamples);
            ShaderLibrary::Get("EnvironmentIrradiance")->DispatchCompute(IrradianceThreadGroups.x, IrradianceThreadGroups.y, IrradianceThreadGroups.z);
        }
        // EnvironmentIrradiance

        OHM_CORE_TRACE("\t-----EnvironmentMapPipeline complete.  Radiance & Irradiance Maps are ready for use.-----");
    }

    void EnvironmentMapPipeline::GenerateFromShader(const std::string& CreationShader) const
    {
        OHM_CORE_TRACE("-----Starting Environment Map Pipeline using shader '{}'...-----", CreationShader);

        const uint32_t EnvironmentMapResolution = m_Specification->EnvironmentMapResolution;
        const uint32_t ThreadGroupSize = m_Specification->GetThreadGroupSize();

        TextureCubeSpecification EnvironmentCubeSpec =
        { 
            TextureUtils::WrapMode::ClampToEdge,
            TextureUtils::WrapMode::ClampToEdge,
            TextureUtils::WrapMode::ClampToEdge,
            TextureUtils::FilterMode::LinearMipLinear,
            TextureUtils::FilterMode::Linear,
            TextureUtils::ImageInternalFormat::RGBA32F,
            TextureUtils::ImageDataLayout::RGBA,
            TextureUtils::ImageDataType::Float,
            EnvironmentMapResolution
        };
        
        const glm::ivec3 ThreadGroups = glm::ivec3(
            glm::ceil(EnvironmentCubeSpec.Dimension / ThreadGroupSize),
            glm::ceil(EnvironmentCubeSpec.Dimension / ThreadGroupSize),
            6);

        // Recreate Cubemap/Skybox specific resources
        EnvironmentCubeSpec.Name = m_Specification->EnvironmentMapName + "-EnvironmentRadianceCubeUnfiltered";
        const Ref<TextureCube> EnvironmentCubeUnfiltered = TextureLibrary::LoadTextureCube(EnvironmentCubeSpec, true);
        EnvironmentCubeSpec.Name = m_Specification->EnvironmentMapName + "-EnvironmentRadianceCubeFiltered";
        const Ref<TextureCube> EnvironmentCubeFiltered = TextureLibrary::LoadTextureCube(EnvironmentCubeSpec, true);

        // User Shader Radiance Cubemap Creation
        {
            ShaderLibrary::Get(CreationShader)->Bind();
            if(m_Specification->PreDispatchFn)
                m_Specification->PreDispatchFn(EnvironmentCubeUnfiltered, EnvironmentCubeFiltered);
            EnvironmentCubeUnfiltered->BindToImageSlot(0, 0, TextureUtils::TextureAccessLevel::WriteOnly, TextureUtils::TextureShaderDataFormat::RGBA32F);
            ShaderLibrary::Get(CreationShader)->DispatchCompute(ThreadGroups.x, ThreadGroups.y, ThreadGroups.z);
            if(m_Specification->PostDispatchFn)
                m_Specification->PostDispatchFn(EnvironmentCubeUnfiltered, EnvironmentCubeFiltered);
        }
        // User Shader Radiance Cubemap Creation

        // EnvironmentFilter
        {
            const uint32_t MaxMipCount = TextureUtils::CalculateMipLevelCount(EnvironmentMapResolution, EnvironmentMapResolution);
            const float DeltaRoughness = 1.0f / glm::max(static_cast<float>(MaxMipCount) - 1.0f, 1.0f);
            
            for(uint32_t Mip = 0; Mip < MaxMipCount; Mip++)
            {
                OHM_CORE_TRACE("\tDispatching 'EnvironmentFilter-{}' at the {}th mip of the unfiltered environment map...", m_Specification->EnvironmentMapName, Mip);
                const int MipMapSize = m_Specification->EnvironmentMapResolution >> Mip;
                const uint32_t ThreadGroupCount = glm::max(1u, MipMapSize / m_Specification->GetThreadGroupSize());
                float Roughness = Mip * DeltaRoughness;
                Roughness = glm::max(Roughness, 0.05f);
                
                EnvironmentCubeUnfiltered->BindToSamplerSlot(0);
                EnvironmentCubeFiltered->BindToImageSlot(0, Mip, TextureUtils::TextureAccessLevel::ReadWrite, TextureUtils::TextureShaderDataFormat::RGBA32F);
                ShaderLibrary::Get("EnvironmentMipFilter")->Bind();
                ShaderLibrary::Get("EnvironmentMipFilter")->UploadUniformInt("sampler_InputCube", 0);
                ShaderLibrary::Get("EnvironmentMipFilter")->UploadUniformInt("MipOutputWidth", MipMapSize);
                ShaderLibrary::Get("EnvironmentMipFilter")->UploadUniformInt("MipOutputHeight", MipMapSize);
                ShaderLibrary::Get("EnvironmentMipFilter")->UploadUniformInt("Mip", Mip);
                ShaderLibrary::Get("EnvironmentMipFilter")->UploadUniformFloat("Roughness", Roughness);
                ShaderLibrary::Get("EnvironmentMipFilter")->DispatchCompute(ThreadGroupCount, ThreadGroupCount, ThreadGroups.z);
            }
        }
        // EnvironmentFilter

        // EnvironmentIrradiance
        {
            const uint32_t IrradianceMapSize = m_Specification->GetIrradianceMapSize();
            const uint32_t IrradianceMapComputeSamples = m_Specification->IrradianceMapComputeSamples;
            OHM_CORE_TRACE("\tDispatching 'EnvironmentIrradiance-{}'...", m_Specification->EnvironmentMapName);

            TextureCubeSpecification IrradianceSpec =
            {
                TextureUtils::WrapMode::ClampToEdge,
                TextureUtils::WrapMode::ClampToEdge,
                TextureUtils::WrapMode::ClampToEdge,
                TextureUtils::FilterMode::LinearMipLinear,
                TextureUtils::FilterMode::Linear,
                TextureUtils::ImageInternalFormat::RGBA32F,
                TextureUtils::ImageDataLayout::RGBA,
                TextureUtils::ImageDataType::Float,
                IrradianceMapSize,
            };
            
            IrradianceSpec.Name = m_Specification->EnvironmentMapName + "-EnvironmentIrradianceCube";
            
            const glm::ivec3 IrradianceThreadGroups = glm::ivec3(
            glm::ceil(IrradianceSpec.Dimension / ThreadGroupSize),
            glm::ceil(IrradianceSpec.Dimension / ThreadGroupSize),
            6);

            const Ref<TextureCube> IrradianceMap = TextureLibrary::LoadTextureCube(IrradianceSpec, true);
            IrradianceMap->BindToImageSlot(0, 0, TextureUtils::TextureAccessLevel::WriteOnly, TextureUtils::TextureShaderDataFormat::RGBA32F);
            EnvironmentCubeFiltered->BindToSamplerSlot(0);
            ShaderLibrary::Get("EnvironmentIrradiance")->Bind();
            ShaderLibrary::Get("EnvironmentIrradiance")->UploadUniformInt("sampler_RadianceMap", 0);
            ShaderLibrary::Get("EnvironmentIrradiance")->UploadUniformInt("EnvironmentSamples", IrradianceMapComputeSamples);
            ShaderLibrary::Get("EnvironmentIrradiance")->DispatchCompute(IrradianceThreadGroups.x, IrradianceThreadGroups.y, IrradianceThreadGroups.z);

            OHM_CORE_TRACE("\t-----EnvironmentMapPipeline complete.  Radiance & Irradiance Maps are ready for use.-----");
        }
        // EnvironmentIrradiance
    }
}

