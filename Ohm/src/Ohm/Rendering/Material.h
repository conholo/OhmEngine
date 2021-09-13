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
		void Set(const std::string& name, const T& data)
		{
			const auto* uniform = FindShaderUniform(name);

			if (uniform == nullptr)
			{
				OHM_CORE_ERROR("Could not find uniform with name: {} in shader: {}.", name, m_Shader->GetName());
				return;
			}

			m_UniformStorageBuffer.Write((uint8_t*)&data, uniform->GetSize(), uniform->GetBufferOffset());
		}

		template<typename T>
		T* Get(const std::string& name)
		{
			const auto* uniform = FindShaderUniform(name);

			// TODO:: Add assertion if not found.

			if (uniform == nullptr)
			{
				OHM_CORE_ERROR("Could not find uniform with name: {} in shader: {}.", name, m_Shader->GetName());
				return nullptr;
			}
		
			return m_UniformStorageBuffer.Read<T>(uniform->GetBufferOffset());
		}

		const ShaderUniform* FindShaderUniform(const std::string& name);
		const std::unordered_map<std::string, StagedUniform>& GetAvailableUniforms() { return m_StagedUniforms; }

	private:
		void AllocateStorageBuffer();
		void InitializeStorageBufferWithUniformDefaults();

	private:
		uint32_t m_UniformStagingOffset = 0;
		Buffer m_UniformStorageBuffer;
		std::unordered_map<std::string, GLint> m_Uniforms;
		std::unordered_map<std::string, StagedUniform> m_StagedUniforms;
		Ref<Shader> m_Shader;
		std::string m_Name;
	};
}