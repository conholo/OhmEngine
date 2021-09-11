#pragma once

#include "Ohm/Rendering/Shader.h"
#include "Ohm/Core/Buffer.h"

namespace Ohm
{
	struct StagedUniform
	{
		StagedUniform() = default;
		StagedUniform(const StagedUniform& other) = default;
		StagedUniform(const ShaderUniform* uniform, uint32_t stagedBufferOffset)
			:Uniform(uniform), StagedBufferOffset(stagedBufferOffset) { }

		const ShaderUniform* Uniform;
		uint32_t StagedBufferOffset;
	};

	class Material
	{
	public:
		Material(const std::string& name, const Ref<Shader>& shader);
		Ref<Material> Clone(const std::string& cloneName);

		Ref<Shader> GetShader() const { return m_Shader; }
		const std::string& GetName() const { return m_Name; }

		void UploadStagedUniforms();

		template<typename T>
		void StageUniform(const std::string& name, const T& data)
		{
			const auto* uniform = FindShaderUniform(name);

			if (uniform == nullptr)
			{
				OHM_CORE_ERROR("Could not find uniform with name: {}.", name);
				return;
			}

			bool uniformNotAllocated = m_StagedUniforms.find(name) == m_StagedUniforms.end();

			uint32_t offset = uniformNotAllocated ? m_UniformStagingOffset : m_StagedUniforms[name].StagedBufferOffset;

			m_UniformStorageBuffer.Write((uint8_t*)&data, uniform->GetSize(), offset);

			if (uniformNotAllocated)
			{
				StagedUniform staged(uniform, m_UniformStagingOffset);
				m_StagedUniforms[name] = staged;
				m_UniformStagingOffset += uniform->GetSize();
			}
		}

		template<typename T>
		T& GetStaged(const std::string& name)
		{
			const auto* uniform = FindShaderUniform(name);

			// TODO:: Add assertion if not found.
		
			return m_UniformStorageBuffer.Read<T>(m_StagedUniforms[name].StagedBufferOffset);
		}

		const ShaderUniform* FindShaderUniform(const std::string& name);
		const std::unordered_map<std::string, StagedUniform>& GetAvailableUniforms() { return m_StagedUniforms; }

	private:
		void AllocateStorageBuffer();

	private:
		uint32_t m_UniformStagingOffset = 0;
		Buffer m_UniformStorageBuffer;
		std::unordered_map<std::string, GLint> m_Uniforms;
		std::unordered_map<std::string, StagedUniform> m_StagedUniforms;
		Ref<Shader> m_Shader;
		std::string m_Name;
	};
}