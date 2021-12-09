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
		InitializeStorageBufferWithUniformDefaults();

		SetFlag(RenderFlag::DepthTest);
		SetFlag(RenderFlag::Blend);

		CheckShouldReceiveShadows();
	}

	Ref<Material> Material::Clone(const std::string& cloneName)
	{
		Ref<Material> copy = CreateRef<Material>(cloneName, m_Shader);

		return copy;
	}

	// This probably needs to be handled differently.  Is currently called by the SceneRenderer at the time of the Geometry pass.
	// If the material receives shadows, it sets the uniform pointing to the shadowMap as valid, otherwise an invalid tex index.	
	void Material::CheckShouldReceiveShadows()
	{
		if (!Has("sampler_ShadowMap")) return;

		Set<int>("sampler_ShadowMap", m_ReceiveShadows ? 2 : 0);
	}

	void Material::AllocateStorageBuffer()
	{
		const auto& uniforms = m_Shader->GetUniforms();

		if (uniforms.size() <= 0) return;

		uint32_t bufferSize = 0;

		for (auto& [name, uniform] : uniforms)
			bufferSize += uniform.GetSize();

		m_UniformStorageBuffer.Allocate(bufferSize);
		m_UniformStorageBuffer.ZeroInitialize();
	}

	void Material::InitializeStorageBufferWithUniformDefaults()
	{
		const auto& uniforms = m_Shader->GetUniforms();

		if (uniforms.size() <= 0) return;

		for (auto& [name, uniform] : uniforms)
		{
			switch (uniform.GetType())
			{
				case ShaderDataType::Float:
				case ShaderDataType::Float2:
				case ShaderDataType::Float3:
				case ShaderDataType::Float4:
				{
					GLfloat* data = (GLfloat*)m_Shader->GetUniformData(uniform.GetType(), uniform.GetLocation());

					m_UniformStorageBuffer.Write(data, uniform.GetSize(), uniform.GetBufferOffset());
					break;
				}
				case ShaderDataType::Int:
				{
					GLint* data = (GLint*)m_Shader->GetUniformData(uniform.GetType(), uniform.GetLocation());

					m_UniformStorageBuffer.Write(data, uniform.GetSize(), uniform.GetBufferOffset());
					break;
				}
				case ShaderDataType::Mat3:
				case ShaderDataType::Mat4:
				{
					void* data = malloc(ShaderDataTypeSize(uniform.GetType()));

					m_UniformStorageBuffer.Write(data, ShaderDataTypeSize(uniform.GetType()), uniform.GetBufferOffset());
					break;
				}
				case ShaderDataType::Sampler2D:
					break;
			}
		}
	}

	const ShaderUniform* Material::FindShaderUniform(const std::string& name)
	{
		const auto& uniforms = m_Shader->GetUniforms();

		if (uniforms.size() <= 0) return nullptr;

		if (uniforms.find(name) == uniforms.end())
			return nullptr;

		return &uniforms.at(name);
	}

	void Material::UploadStagedUniforms()
	{
		m_Shader->Bind();

		for (auto& [name, uniform] : m_Shader->GetUniforms())
		{
			std::string name = uniform.GetName();

			switch (uniform.GetType())
			{
				case ShaderDataType::Float:
				{
					float* value = Get<float>(name);
					m_Shader->UploadUniformFloat(name, *value);
					break;
				}
				case ShaderDataType::Float2:
				{
					glm::vec2* value = Get<glm::vec2>(name);
					m_Shader->UploadUniformFloat2(name, *value);
					break;
				}
				case ShaderDataType::Float3:
				{
					glm::vec3* value = Get<glm::vec3>(name);
					m_Shader->UploadUniformFloat3(name, *value);
					break;
				}
				case ShaderDataType::Float4:
				{
					glm::vec4* value = Get<glm::vec4>(name);
					m_Shader->UploadUniformFloat4(name, *value);
					break;
				}
				case ShaderDataType::Sampler2D:
				{
					int* value = Get<int>(name);
					m_Shader->UploadUniformInt(name, *value);
					break;
				}
				case ShaderDataType::Int:
				{
					int* value = Get<int>(name);
					m_Shader->UploadUniformInt(name, *value);
					break;
				}
				case ShaderDataType::Mat3:
				{
					glm::mat3* value = Get<glm::mat3>(name);
					m_Shader->UploadUniformMat3(name, *value);
					break;
				}
				case ShaderDataType::Mat4:
				{
					glm::mat4* value = Get<glm::mat4>(name);
					m_Shader->UploadUniformMat4(name, *value);
					break;
				}
			}
		}
	}
}