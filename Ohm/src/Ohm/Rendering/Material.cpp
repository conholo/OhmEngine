#include "ohmpch.h"
#include "Ohm/Rendering/Material.h"
#include "Ohm/Rendering/Texture2D.h"

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
	}

	Ref<Material> Material::Clone(const std::string& cloneName)
	{
		Ref<Material> copy = CreateRef<Material>(cloneName, m_Shader);

		return copy;
	}

	void Material::BindActiveTextures()
	{
		auto textureSlots = TextureLibrary::BindAndGetMaterialTextureSlots(m_Textures);

		for (auto [name, slot] : textureSlots)
			Set<TextureUniform>(name, { m_Textures[name], name == "sampler_ShadowMap" ? 1 : 0, slot });
	}

	void Material::UpdateActiveTexture(const std::string& uniformName, uint32_t newID)
	{
		if (m_Textures.find(uniformName) == m_Textures.end())
		{
			OHM_ERROR("Cannot update texture with name {}.  This texture could not be found.", uniformName);
			return;
		}

		m_Textures[uniformName] = newID;
	}

	MaterialUniformData Material::GetMaterialUniformData()
	{
		MaterialUniformData data;

		for (auto& [name, uniform] : m_Shader->GetUniforms())
		{
			std::string name = uniform.GetName();

			switch (uniform.GetType())
			{
			case ShaderDataType::Float:
			{
				float* value = Get<float>(name);
				data.FloatUniforms[name] = *value;
				break;
			}
			case ShaderDataType::Float2:
			{
				glm::vec2* value = Get<glm::vec2>(name);
				data.Vec2Uniforms[name] = *value;
				break;
			}
			case ShaderDataType::Float3:
			{
				glm::vec3* value = Get<glm::vec3>(name);
				data.Vec3Uniforms[name] = *value;
				break;
			}
			case ShaderDataType::Float4:
			{
				glm::vec4* value = Get<glm::vec4>(name);
				data.Vec4Uniforms[name] = *value;
				break;
			}
			case ShaderDataType::Sampler2D:
			{
				TextureUniform* value = Get<TextureUniform>(name);
				data.TextureUniforms[name] = *value;
				break;
			}
			case ShaderDataType::Int:
			{
				int* value = Get<int>(name);
				data.IntUniforms[name] = *value;
				break;
			}
			case ShaderDataType::Mat3:
			{
				glm::mat3* value = Get<glm::mat3>(name);
				break;
			}
			case ShaderDataType::Mat4:
			{
				glm::mat4* value = Get<glm::mat4>(name);
				break;
			}
			}
		}

		return data;
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
				{
					TextureUniform* data = (TextureUniform*)m_Shader->GetUniformData(uniform.GetType(), uniform.GetLocation());
					data->RendererID = TextureLibrary::Get("White Texture")->GetID();
					m_Textures[name] = data->RendererID;

					m_UniformStorageBuffer.Write(data, uniform.GetSize(), uniform.GetBufferOffset());
					break;
				}
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
					TextureUniform* value = Get<TextureUniform>(name);
					m_Shader->UploadUniformInt(name, (*value).TextureUnit);
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