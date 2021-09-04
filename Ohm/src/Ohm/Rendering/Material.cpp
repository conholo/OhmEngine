#include "ohmpch.h"
#include "Ohm/Rendering/Material.h"

#include <glad/glad.h>
#include <glm/glm.hpp>

namespace Ohm
{

	Material::Material(const Ref<Shader>& shader)
		:m_Shader(shader)
	{

	}

	float Material::GetFloat(const std::string& name)
	{
		GLint location = m_Uniforms[name];

		float value;
		glGetUniformfv(m_Shader->GetID(), location, &value);
		
		return value;
	}

	glm::vec2 Material::GetFloat2(const std::string& name)
	{
		GLint location = m_Uniforms[name];

		glm::vec2 value;
		glGetUniformfv(m_Shader->GetID(), location, &value.x);

		return value;
	}

	glm::vec3 Material::GetFloat3(const std::string& name)
	{
		GLint location = m_Uniforms[name];

		glm::vec3 value;
		glGetUniformfv(m_Shader->GetID(), location, &value.x);

		return value;
	}

	glm::vec4 Material::GetFloat4(const std::string& name)
	{
		GLint location = m_Uniforms[name];

		glm::vec4 value;
		glGetUniformfv(m_Shader->GetID(), location, &value.x);

		return value;
	}

	int Material::GetInt(const std::string& name)
	{
		GLint location = m_Uniforms[name];

		int value;
		glGetUniformiv(m_Shader->GetID(), location, &value);

		return value;
	}

	int* Material::GetIntArray(const std::string& name, size_t count)
	{
		GLint location = m_Uniforms[name];

		std::vector<int> result;
		result.resize(count);
		
		int offset = 0;
		for (size_t i = 0; i < count; ++i, offset += sizeof(int))
		{
			int value;
			glGetUniformiv(m_Shader->GetID(), location, result.data() + offset);
		}
		
		return result.data();
	}

	glm::mat3 Material::GetMat3(const std::string& name)
	{
		GLint location = m_Uniforms[name];

		glm::mat3 result(1.0f);

		return result;
	}

	glm::mat4 Material::GetMat4(const std::string& name)
	{
		GLint location = m_Uniforms[name];

		glm::mat4 result(1.0f);

		return result;
	}

	void Material::SetFloat(const std::string& name, float value)
	{
		GLint location = m_Shader->UploadUniformFloat(name, value);
		m_Uniforms.emplace(name, location);
	}

	void Material::SetFloat2(const std::string& name, const glm::vec2& value)
	{
		GLint location = m_Shader->UploadUniformFloat2(name, value);
		m_Uniforms.emplace(name, location);
	}

	void Material::SetFloat3(const std::string& name, const glm::vec3& value)
	{
		GLint location = m_Shader->UploadUniformFloat3(name, value);
		m_Uniforms.emplace(name, location);
	}

	void Material::SetFloat4(const std::string& name, const glm::vec4& value)
	{
		GLint location = m_Shader->UploadUniformFloat4(name, value);
		m_Uniforms.emplace(name, location);
	}

	void Material::SetInt(const std::string& name, int value)
	{
		GLint location = m_Shader->UploadUniformInt(name, value);
		m_Uniforms.emplace(name, location);
	}

	void Material::SetIntArray(const std::string& name, uint32_t count, int* basePtr)
	{
		GLint location = m_Shader->UploadUniformIntArray(name, count, basePtr);
		m_Uniforms.emplace(name, location);
	}

	void Material::SetMat3(const std::string& name, const glm::mat3& matrix)
	{
		GLint location = m_Shader->UploadUniformMat3(name, matrix);
		m_Uniforms.emplace(name, location);
	}

	void Material::SetMat4(const std::string& name, const glm::mat4& matrix)
	{
		GLint location = m_Shader->UploadUniformMat4(name, matrix);
		m_Uniforms.emplace(name, location);
	}

}