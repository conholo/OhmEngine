#pragma once

#include "Ohm/Rendering/Shader.h"
#include "Ohm/Core/Buffer.h"
#include "Ohm/Rendering/RenderCommand.h"

namespace Ohm
{
	struct TextureUniform
	{
		uint32_t RendererID = 0;
		int32_t HideInInspector = 1;
		int32_t TextureUnit = -1;
	};

	struct MaterialUniformData
	{
		std::unordered_map<std::string, float> FloatUniforms;
		std::unordered_map<std::string, int> IntUniforms;
		std::unordered_map<std::string, TextureUniform> TextureUniforms;
		std::unordered_map<std::string, glm::vec2> Vec2Uniforms;
		std::unordered_map<std::string, glm::vec3> Vec3Uniforms;
		std::unordered_map<std::string, glm::vec4> Vec4Uniforms;
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
				OHM_CORE_WARN("Could not find uniform with name: {} in shader: {}.", name, m_Shader->GetName());
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
				OHM_CORE_WARN("Could not find uniform with name: {} in shader: {}.", name, m_Shader->GetName());
				return nullptr;
			}
		
			return m_UniformStorageBuffer.Read<T>(uniform->GetBufferOffset());
		}

		bool Has(const std::string& name)
		{
			const auto* uniform = FindShaderUniform(name);

			return uniform != nullptr;
		}

		const ShaderUniform* FindShaderUniform(const std::string& name);

		uint32_t GetFlags() const { return m_RenderFlags; }
		bool GetFlag(RenderFlag flag) const { return (uint32_t)flag & m_RenderFlags; }
		void SetFlag(RenderFlag flag, bool value = true)
		{
			m_RenderFlags = value ? m_RenderFlags | (uint32_t)flag : m_RenderFlags & ~(uint32_t)flag;
		}

		void BindActiveTextures();
		void UpdateActiveTexture(const std::string& uniformName, uint32_t newID);

		MaterialUniformData GetMaterialUniformData();

	private:
		void AllocateStorageBuffer();
		void InitializeStorageBufferWithUniformDefaults();


	private:
		uint32_t m_UniformStagingOffset = 0;
		Buffer m_UniformStorageBuffer;
		std::unordered_map<std::string, GLint> m_Uniforms;
		std::unordered_map<std::string, uint32_t> m_Textures;
		Ref<Shader> m_Shader;
		std::string m_Name;
		uint32_t m_RenderFlags;
	};
}