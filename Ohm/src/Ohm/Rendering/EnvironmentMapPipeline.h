#pragma once
#include <functional>
#include <string>
#include "TextureCube.h"

namespace Ohm
{
    class Shader;
    using DispatchCreateRadianceMapFn = std::function<void(const Ref<TextureCube>&, const Ref<TextureCube>&)>;
    
    enum class EnvironmentPipelineType { BlackCube, FromShader, FromFile };
    
    namespace EnvironmentUtils
    {
        std::unordered_map<std::string, EnvironmentPipelineType> GetTypeMap();
        std::string PipelineTypeToString(EnvironmentPipelineType Type);
        EnvironmentPipelineType NameToPipelineType(const std::string& Name);
    }
    
    struct EnvironmentMapSpecification
    {

        EnvironmentPipelineType PipelineType = EnvironmentPipelineType::BlackCube;
        std::string EnvironmentMapName = "Empty Environment";
        std::string FromFileFilePath = "";
        uint32_t EnvironmentMapResolution = 1024;
        uint32_t IrradianceMapComputeSamples = 512;
        DispatchCreateRadianceMapFn PreDispatchFn = nullptr;
        DispatchCreateRadianceMapFn PostDispatchFn = nullptr;

        bool IsShader() const { return PipelineType == EnvironmentPipelineType::FromShader; }

        EnvironmentMapSpecification() = default;
        EnvironmentMapSpecification(EnvironmentPipelineType Type, uint32_t EnvironmentMapResolution = 1024, uint32_t IrradianceComputeSamples = 512)
            : PipelineType(Type), EnvironmentMapResolution(EnvironmentMapResolution), IrradianceMapComputeSamples(IrradianceComputeSamples)
        { }
        
        std::string GetUnfilteredCubeName() const { return EnvironmentMapName + EnvironmentRadianceCubeUnfilteredSuffix; }
        std::string GetFilteredCubeName() const { return EnvironmentMapName + EnvironmentRadianceCubeFilteredSuffix; }
        std::string GetIrradianceCubeName() const { return EnvironmentMapName + EnvironmentIrradianceCubeSuffix; }

        uint32_t GetThreadGroupSize() const { return m_ThreadGroupSize; }
        uint32_t GetIrradianceMapSize() const { return m_IrradianceMapSize; }
    
    private:
        std::string EnvironmentRadianceCubeUnfilteredSuffix = "-EnvironmentRadianceCubeUnfiltered";
        std::string EnvironmentRadianceCubeFilteredSuffix = "-EnvironmentRadianceCubeFiltered";
        std::string EnvironmentIrradianceCubeSuffix = "-EnvironmentIrradianceCube";
        uint32_t m_ThreadGroupSize = 32;
        uint32_t m_IrradianceMapSize = 32;
    };
    
    class EnvironmentMapPipeline
    {
    public:
        EnvironmentMapPipeline();
        ~EnvironmentMapPipeline();

        void BuildFromBlackTextureCube() const;
        void BuildFromShader(const std::string& CreationShaderName) const;
        void BuildFromEquirectangularImage(const std::string& FilePath) const;
        
        const EnvironmentMapSpecification& GetSpecification() const { return *m_Specification; }
        EnvironmentMapSpecification& GetSpecification() { return *m_Specification; }
        
    private:
        void GenerateFromFile(const std::string& filePath) const;
        void GenerateFromShader(const std::string& CreationShader) const;
        Ref<EnvironmentMapSpecification> m_Specification;
    };
}

