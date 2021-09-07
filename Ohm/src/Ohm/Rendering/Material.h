#pragma once

#include "Ohm/Rendering/Shader.h"


namespace Ohm
{

	class Material
	{
	public:
		Material(const Ref<Shader>& shader);
		Material(const Ref<Material>& other);

		Ref<Material> Copy();

		Ref<Shader> GetShader() const { return m_Shader; }

		void SetFloat(const std::string& name, float value);
		void SetFloat2(const std::string& name, const glm::vec2& value);
		void SetFloat3(const std::string& name, const glm::vec3& value);
		void SetFloat4(const std::string& name, const glm::vec4& value);
		void SetInt(const std::string& name, int value);
		void SetIntArray(const std::string& name, uint32_t count, int* basePtr);
		void SetMat3(const std::string& name, const glm::mat3& matrix);
		void SetMat4(const std::string& name, const glm::mat4& matrix);
		
		float GetFloat(const std::string& name);
		glm::vec2 GetFloat2(const std::string& name);
		glm::vec3 GetFloat3(const std::string& name);
		glm::vec4 GetFloat4(const std::string& name);
		int GetInt(const std::string& name);
		int* GetIntArray(const std::string& name, size_t count);
		glm::mat3 GetMat3(const std::string& name);
		glm::mat4 GetMat4(const std::string& name);

	private:
		std::unordered_map<std::string, GLint> m_Uniforms;
		Ref<Shader> m_Shader;
	};
}