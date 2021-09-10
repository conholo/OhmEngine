#pragma once

#include <glm/glm.hpp>

// TEMPORARY: ShaderDataType needs to move here.
#include <Ohm/Rendering/BufferLayout.h>

namespace Ohm
{
	typedef unsigned int GLenum;
	typedef int GLint;

	class ShaderUniform
	{
	public:
		ShaderUniform() = default;
		ShaderUniform(const std::string& name, ShaderDataType type, uint32_t size, uint32_t count, int32_t offset);

		const std::string& GetName() const { return m_Name; }
		ShaderDataType GetType() const { return m_Type; }
		uint32_t GetCount() const { return m_Count; }
		uint32_t GetSize() const { return m_Size; }
		int32_t GetBlockOffset() const { return m_BlockOffset; }

		uint32_t GetUniformBufferOffset() const { return m_UniformBufferOffset; }

		void SetUniformBufferOffset(uint32_t offset) { m_UniformBufferOffset = offset; }

	private:

		std::string m_Name;
		ShaderDataType m_Type = ShaderDataType::None;
		uint32_t m_Count = 0;
		uint32_t m_Size = 0;
		uint32_t m_BlockOffset = 0;
		uint32_t m_UniformBufferOffset = 0;
	};

	class ShaderBlock
	{
	public:
		ShaderBlock() = default;
		ShaderBlock(const std::string& name, uint32_t size, uint32_t memberCount, const ShaderUniform& uniform);
	};

	struct ShaderBuffer
	{
		std::string Name;
		uint32_t Size = 0;
		std::unordered_map<std::string, ShaderUniform> Uniforms;
	};

	class Shader
	{
	public:
		Shader(const std::string& filePath);
		~Shader();

		void Bind() const;
		void Unbind() const;

		uint32_t GetID() const { return m_ID; }
		std::string GetName() const { return m_Name; }

		const ShaderUniform& GetUniform(const std::string& name) { return m_Uniforms[name]; }
		const std::unordered_map<std::string, ShaderUniform>& GetUniforms() const { return m_Uniforms; }

		GLint UploadUniformFloat(const std::string& name, float value);
		GLint UploadUniformFloat2(const std::string& name, const glm::vec2& value);
		GLint UploadUniformFloat3(const std::string& name, const glm::vec3& value);
		GLint UploadUniformFloat4(const std::string& name, const glm::vec4& value);
		GLint UploadUniformInt(const std::string& name, int value);
		GLint UploadUniformIntArray(const std::string& name, uint32_t count, int* basePtr);
		GLint UploadUniformMat3(const std::string& name, const glm::mat3& matrix);
		GLint UploadUniformMat4(const std::string& name, const glm::mat4& matrix);

	private:
		std::string ReadFile(const std::string& filePath);
		std::unordered_map<GLenum, std::string> PreProcess(const std::string& source);
		void Compile(const std::unordered_map<GLenum, std::string>& shaderSources);
		void Reflect();

	private:
		std::unordered_map<std::string, ShaderUniform> m_Uniforms;
		uint32_t m_ActiveUniformCount = 0;
		std::string m_Name;
		uint32_t m_ID;
	};
}