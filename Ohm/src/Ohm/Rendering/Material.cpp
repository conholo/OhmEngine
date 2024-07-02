#include "ohmpch.h"
#include "Material.h"
#include "Ohm/Rendering/RenderCommand.h"
#include "Ohm/Rendering/TextureLibrary.h"

#include <glad/glad.h>
#include <glm/glm.hpp>

namespace Ohm
{
	Material::Material(std::string name, const Ref<Shader>& shader)
		: m_Shader(shader), m_Name(std::move(name))
	{
		AllocateBaseBlockStorageBuffer();
		InitializeBaseBlockStorageBufferWithUniformDefaults();
	}

	Ref<Material> Material::Clone(const std::string& cloneName)
	{
		Ref<Material> copy = CreateRef<Material>(cloneName, m_Shader);
		return copy;
	}

	void Material::BindSamplerTexturesToRenderContext()
	{
		const std::vector<ShaderUniform> sampler2dUniforms = m_Shader->GetBaseBlockUniformsOfType(ShaderDataType::Sampler2D);
		for(const ShaderUniform& Uniform : sampler2dUniforms)
		{
			const auto* texUniforms = Get<TextureUniform>(Uniform.GetName());
			TextureLibrary::BindTextureToSlot(texUniforms->RendererID, texUniforms->TextureUnit);
		}

		const std::vector<ShaderUniform> sampler3dUniforms = m_Shader->GetBaseBlockUniformsOfType(ShaderDataType::Sampler3D);
		for(const ShaderUniform& Uniform : sampler3dUniforms)
		{
			const auto* texUniforms = Get<TextureUniform>(Uniform.GetName());
			TextureLibrary::BindTextureToSlot(texUniforms->RendererID, texUniforms->TextureUnit);
		}
		
		const std::vector<ShaderUniform> samplerCubeUniforms = m_Shader->GetBaseBlockUniformsOfType(ShaderDataType::SamplerCube);
		for(const ShaderUniform& Uniform : samplerCubeUniforms)
		{
			const auto* texUniform = Get<TextureUniform>(Uniform.GetName());
			TextureLibrary::BindTextureToSlot(texUniform->RendererID, texUniform->TextureUnit);
		}
	}

	MaterialUniformData Material::GetMaterialUniformData()
	{
		MaterialUniformData data;

		for (auto& [name, uniform] : m_Shader->GetBaseBlockUniforms())
		{
			std::string uniformName = uniform.GetName();

			switch (uniform.GetType())
			{
			case ShaderDataType::Float:
				{
					float* value = Get<float>(uniformName);
					data.FloatUniforms[uniformName] = *value;
					break;
				}
			case ShaderDataType::Float2:
				{
					glm::vec2* value = Get<glm::vec2>(uniformName);
					data.Vec2Uniforms[uniformName] = *value;
					break;
				}
			case ShaderDataType::Float3:
				{
					glm::vec3* value = Get<glm::vec3>(uniformName);
					data.Vec3Uniforms[uniformName] = *value;
					break;
				}
			case ShaderDataType::Float4:
				{
					glm::vec4* value = Get<glm::vec4>(uniformName);
					data.Vec4Uniforms[uniformName] = *value;
					break;
				}
			case ShaderDataType::Sampler2D:
			case ShaderDataType::Sampler3D:
			case ShaderDataType::SamplerCube:
				{
					TextureUniform* value = Get<TextureUniform>(uniformName);
					data.TextureUniforms[uniformName] = *value;
					break;
				}
			case ShaderDataType::Int:
				{
					int* value = Get<int>(uniformName);
					data.IntUniforms[uniformName] = *value;
					break;
				}
			case ShaderDataType::Bool:
				{
					bool* value = Get<bool>(uniformName);
					data.BoolUniforms[uniformName] = *value;
					break;
				}
			case ShaderDataType::Mat3:
				{
					glm::mat3* value = Get<glm::mat3>(uniformName);
					break;
				}
			case ShaderDataType::Mat4:
				{
					glm::mat4* value = Get<glm::mat4>(uniformName);
					break;
				}
			}
		}

		return data;
	}

	void Material::AllocateBaseBlockStorageBuffer()
	{
		const auto& uniforms = m_Shader->GetBaseBlockUniforms();

		if (uniforms.empty()) return;

		uint32_t bufferSize = 0;

		for (auto& [name, uniform] : uniforms)
			bufferSize += uniform.GetSize();

		m_BaseBlockStorageBuffer.Allocate(bufferSize);
		m_BaseBlockStorageBuffer.ZeroInitialize();
	}

	void Material::InitializeBaseBlockStorageBufferWithUniformDefaults()
	{
		const auto& uniforms = m_Shader->GetBaseBlockUniforms();

		if (uniforms.empty()) return;

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
					m_BaseBlockStorageBuffer.Write<float>(data, uniform.GetSize(), uniform.GetBufferOffset());
					break;
				}
			case ShaderDataType::Bool:
				{
					GLboolean* data = (GLboolean*)m_Shader->GetUniformData(uniform.GetType(), uniform.GetLocation());
					m_BaseBlockStorageBuffer.Write<bool>(data, uniform.GetSize(), uniform.GetBufferOffset());
					break;
				}
			case ShaderDataType::Int:
				{
					GLint* data = (GLint*)m_Shader->GetUniformData(uniform.GetType(), uniform.GetLocation());
					m_BaseBlockStorageBuffer.Write<int>(data, uniform.GetSize(), uniform.GetBufferOffset());
					break;
				}
			case ShaderDataType::Mat3:
			case ShaderDataType::Mat4:
				{
					void* data = malloc(ShaderDataTypeSize(uniform.GetType()));
					m_BaseBlockStorageBuffer.Write<glm::mat4>(data, ShaderDataTypeSize(uniform.GetType()), uniform.GetBufferOffset());
					break;
				}
			case ShaderDataType::SamplerCube:
				{
					TextureUniform* data = (TextureUniform*)m_Shader->GetUniformData(uniform.GetType(), uniform.GetLocation());
					data->RendererID = TextureLibrary::GetCube("Black TextureCube")->GetID();
					m_BaseBlockStorageBuffer.Write<TextureUniform>(data, uniform.GetSize(), uniform.GetBufferOffset());
					break;
				}
			case ShaderDataType::Sampler2D:
				{
					TextureUniform* data = (TextureUniform*)m_Shader->GetUniformData(uniform.GetType(), uniform.GetLocation());
					data->RendererID = TextureLibrary::Get2D("White Texture")->GetID();
					m_BaseBlockStorageBuffer.Write<TextureUniform>(data, uniform.GetSize(), uniform.GetBufferOffset());
					break;
				}
			case ShaderDataType::Sampler3D:
				{
					TextureUniform* data = (TextureUniform*)m_Shader->GetUniformData(uniform.GetType(), uniform.GetLocation());
					data->RendererID = TextureLibrary::Get3D("Black Texture3D")->GetID();
					m_BaseBlockStorageBuffer.Write<TextureUniform>(data, uniform.GetSize(), uniform.GetBufferOffset());
					break;
				}
			default:
			case ShaderDataType::None: break;
			}
		}
	}

	const ShaderUniform* Material::FindBaseBlockShaderUniform(const std::string& name) const
	{
		const auto& uniforms = m_Shader->GetBaseBlockUniforms();

		if (uniforms.empty()) return nullptr;

		if (uniforms.find(name) == uniforms.end())
			return nullptr;

		return &uniforms.at(name);
	}

	void Material::UploadStagedUniforms()
	{
		BindSamplerTexturesToRenderContext();
		m_Shader->Bind();
		for (auto& [name, uniform] : m_Shader->GetBaseBlockUniforms())
		{
			std::string uniformName = uniform.GetName();

			switch (uniform.GetType())
			{
			case ShaderDataType::Float:
				{
					float* value = Get<float>(uniformName);
					m_Shader->UploadUniformFloat(uniformName, *value);
					break;
				}
			case ShaderDataType::Float2:
				{
					glm::vec2* value = Get<glm::vec2>(uniformName);
					m_Shader->UploadUniformFloat2(uniformName, *value);
					break;
				}
			case ShaderDataType::Float3:
				{
					glm::vec3* value = Get<glm::vec3>(uniformName);
					m_Shader->UploadUniformFloat3(uniformName, *value);
					break;
				}
			case ShaderDataType::Float4:
				{
					glm::vec4* value = Get<glm::vec4>(uniformName);
					m_Shader->UploadUniformFloat4(uniformName, *value);
					break;
				}
			case ShaderDataType::Sampler3D:
			case ShaderDataType::SamplerCube:
			case ShaderDataType::Sampler2D:
				{
					const TextureUniform* value = Get<TextureUniform>(uniformName);
					m_Shader->UploadUniformInt(uniformName, value->TextureUnit);
					break;
				}
			case ShaderDataType::Int:
				{
					int* value = Get<int>(uniformName);
					m_Shader->UploadUniformInt(uniformName, *value);
					break;
				}
			case ShaderDataType::Bool:
				{
					bool* value = Get<bool>(uniformName);
					m_Shader->UploadUniformBool(uniformName, *value);
					break;
				}
			case ShaderDataType::Mat3:
				{
					glm::mat3* value = Get<glm::mat3>(uniformName);
					m_Shader->UploadUniformMat3(uniformName, *value);
					break;
				}
			case ShaderDataType::Mat4:
				{
					glm::mat4* value = Get<glm::mat4>(uniformName);
					m_Shader->UploadUniformMat4(uniformName, *value);
					break;
				}
			case ShaderDataType::Image2D:
			case ShaderDataType::ImageCube:
			case ShaderDataType::None:
				break;
			}
		}
	}
}
