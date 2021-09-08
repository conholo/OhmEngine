#include "ohmpch.h"
#include "Ohm/Rendering/Material.h"

#include <glad/glad.h>
#include <glm/glm.hpp>

namespace Ohm
{
	Material::Material(const std::string& name, const Ref<Shader>& shader)
		:m_Name(name), m_Shader(shader)
	{
		AllocateStorageBuffer();
	}


	Ref<Material> Material::Clone(const std::string& cloneName)
	{
		Ref<Material> copy = CreateRef<Material>(cloneName, m_Shader);

		return copy;
	}

	void Material::AllocateStorageBuffer()
	{
		const auto& uniforms = m_Shader->GetUniforms();

		if (uniforms.size() <= 0) return;

		uint32_t size = 0;

		for (auto& [name, uniform] : uniforms)
			size += uniform.GetSize();

		m_UniformStorageBuffer.Allocate(size);
		m_UniformStorageBuffer.ZeroInitialize();
		OHM_CORE_TRACE("Allocated and zero initialized {} bytes to uniform storage buffer.", size);
	}

	const ShaderUniform* Material::FindShaderUniform(const std::string& name)
	{
		const auto& uniforms = m_Shader->GetUniforms();

		if (uniforms.size() <= 0) return nullptr;

		if (uniforms.find(name) == uniforms.end())
		{
			OHM_CORE_WARN("Could not locate uniform named: {} in shader: {}", name, m_Shader->GetName());
			return nullptr;
		}

		return &uniforms.at(name);
	}

	void Material::UploadStagedUniforms()
	{
		m_Shader->Bind();

		if (m_StagedUniforms.size() <= 0) return;

		for (auto& [name, stagedUniform] : m_StagedUniforms)
		{
			const auto* uniform = stagedUniform.Uniform;

			std::string name = uniform->GetName();

			switch (uniform->GetType())
			{
				case ShaderDataType::Float:
				{
					float value = GetStaged<float>(name);
					m_Shader->UploadUniformFloat(name, value);
					break;
				}
				case ShaderDataType::Float2:
				{
					glm::vec2 value = GetStaged<glm::vec2>(name);
					m_Shader->UploadUniformFloat2(name, value);
					break;
				}
				case ShaderDataType::Float3:
				{
					glm::vec3 value = GetStaged<glm::vec3>(name);
					m_Shader->UploadUniformFloat3(name, value);
					break;
				}
				case ShaderDataType::Float4:
				{
					glm::vec4 value = GetStaged<glm::vec4>(name);
					m_Shader->UploadUniformFloat4(name, value);
					break;
				}
				case ShaderDataType::Int:
				{
					int value = GetStaged<int>(name);
					m_Shader->UploadUniformInt(name, value);
					break;
				}
				case ShaderDataType::Mat3:
				{
					glm::mat3 value = GetStaged<glm::mat3>(name);
					m_Shader->UploadUniformMat3(name, value);
					break;
				}
				case ShaderDataType::Mat4:
				{
					glm::mat4 value = GetStaged<glm::mat4>(name);
					m_Shader->UploadUniformMat4(name, value);
					break;
				}
			}
		}
	}
}