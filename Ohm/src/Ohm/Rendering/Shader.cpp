#include "ohmpch.h"
#include "Ohm/Rendering/Shader.h"

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

namespace Ohm
{
	ShaderUniform::ShaderUniform(const std::string& name, ShaderDataType type, uint32_t size, uint32_t count, int32_t blockOffset)
		:m_Name(name), m_Type(type), m_Count(count), m_Size(size), m_BlockOffset(blockOffset)
	{

	}

	static ShaderDataType ShaderDataTypeFromGLenum(GLenum value)
	{
		switch (value)
		{
			case 0x1406: return ShaderDataType::Float;
			case 0x8B50: return ShaderDataType::Float2;
			case 0x8B51: return ShaderDataType::Float3;
			case 0x8B52: return ShaderDataType::Float4;
			case 0x1404: return ShaderDataType::Int;
			case 0x8B5B: return ShaderDataType::Mat3;
			case 0x8B5C: return ShaderDataType::Mat4;
				// Sampler2D -> Uploads with glUniform1i(name, int value);
			case 0x8B5E: return ShaderDataType::Int;
			default: 
			{
				OHM_CORE_ERROR("Invalid GLenum.  No matching ShaderDataType found for {}.", value);
				return ShaderDataType::None;
			}
		}
	}


	Shader::Shader(const std::string& filePath)
	{
		size_t shaderLocationOffset = filePath.rfind("\/") + 1;
		size_t extensionOffset = filePath.find_first_of(".", shaderLocationOffset);

		m_Name = filePath.substr(shaderLocationOffset, extensionOffset - shaderLocationOffset);

		std::string source = ReadFile(filePath);
		auto shaderSources = PreProcess(source);
		Compile(shaderSources);

		Reflect();
	}

	Shader::~Shader()
	{
		glDeleteShader(m_ID);
	}

	void Shader::Bind() const
	{
		glUseProgram(m_ID);
	}

	void Shader::Unbind() const
	{
		glUseProgram(0);
	}

	std::string Shader::ReadFile(const std::string& filePath)
	{
		std::string result;
		std::ifstream input(filePath, std::ios::binary | std::ios::in);

		if (input)
		{
			input.seekg(0, std::ios::end);
			size_t size = input.tellg();

			if (size != -1)
			{
				result.resize(size);
				input.seekg(0, std::ios::beg);
				input.read(&result[0], size);
			}
		}

		return result;
	}

	GLenum ShaderTypeFromString(const std::string& type)
	{
		if (type == "vertex")
			return GL_VERTEX_SHADER;
		else if (type == "fragment")
			return GL_FRAGMENT_SHADER;

		return GL_FALSE;
	}

	std::unordered_map<GLenum, std::string> Shader::PreProcess(const std::string& source)
	{
		const char* typeToken = "#type";
		size_t typeTokenLength = strlen(typeToken);
		size_t position = source.find(typeToken, 0);

		std::unordered_map<GLenum, std::string> result;

		while (position != std::string::npos)
		{
			size_t endOfLine = source.find_first_of("\r\n", position);
			size_t beginShaderType = position + typeTokenLength + 1;
			std::string type = source.substr(beginShaderType, endOfLine - beginShaderType);

			size_t nextLinePosition = source.find_first_not_of("\r\n", endOfLine);
			position = source.find(typeToken, nextLinePosition);
			result[ShaderTypeFromString(type)] = 
				(position == std::string::npos) 
				? source.substr(nextLinePosition) 
				: source.substr(nextLinePosition, position - nextLinePosition);
		}

		return result;
	}

	void Shader::Compile(const std::unordered_map<GLenum, std::string>& shaderSources)
	{
		GLuint program = glCreateProgram();
		std::array<GLenum, 2> glShaderIDs;
		int glShaderIDIndex = 0;

		for (auto& kv : shaderSources)
		{
			GLenum type = kv.first;
			const std::string& source = kv.second;

			GLuint shader = glCreateShader(type);

			const GLchar* sourceCStr = source.c_str();
			glShaderSource(shader, 1, &sourceCStr, 0);

			glCompileShader(shader);

			GLint isCompiled = 0;
			glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
			if (isCompiled == GL_FALSE)
			{
				GLint maxLength = 0;
				glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

				std::vector<GLchar> infoLog(maxLength);
				glGetShaderInfoLog(shader, maxLength, &maxLength, &infoLog[0]);
				
				std::cout << infoLog.data() << std::endl;

				glDeleteShader(shader);

				break;
			}

			glAttachShader(program, shader);
			glShaderIDs[glShaderIDIndex++] = shader;
		}

		m_ID = program;

		glLinkProgram(program);

		GLint isLinked = 0;
		glGetProgramiv(program, GL_LINK_STATUS, (int*)&isLinked);
		if (isLinked == GL_FALSE)
		{
			GLint maxLength = 0;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

			std::vector<GLchar> infoLog(maxLength);
			glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);

			std::cout << infoLog.data() << std::endl;

			glDeleteProgram(program);

			for (auto id : glShaderIDs)
				glDeleteShader(id);

			return;
		}

		for (auto id : glShaderIDs)
		{
			glDetachShader(program, id);
			glDeleteShader(id);
		}
	}


	void Shader::Reflect()
	{
		GLsizei activeCount = 0;
		glGetProgramiv(m_ID, GL_ACTIVE_UNIFORMS, &activeCount);


		if (activeCount == 0)
		{
			OHM_CORE_WARN("No active uniforms were found when reflecting {}.", m_Name);
			return;
		}

		m_ActiveUniformCount = activeCount;

		OHM_CORE_INFO("Active uniform count for: {} - {}", m_Name, m_ActiveUniformCount);

		GLsizei maxUniformLength;
		glGetProgramiv(m_ID, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxUniformLength);

		std::vector<std::string> activeUniformNames(activeCount);

		for (GLuint i = 0; i < m_ActiveUniformCount; ++i)
		{
			GLsizei length;
			std::vector<GLchar> nameBuffer(maxUniformLength);

			glGetActiveUniformName(m_ID, i, maxUniformLength, &length, nameBuffer.data());

			// Write to member vector to store names.
			std::string name(nameBuffer.data(), length);

			// Store name
			activeUniformNames[i] = name;
		}


		// Vector of buffers for each uniform name.
		std::vector<const char*> nameBuffer(activeCount);

		// Convert from string to const char* for glGetUniformIndices -- it needs a const char**.
		for (uint32_t i = 0; i < activeCount; i++)
			nameBuffer[i] = (activeUniformNames[i]).c_str();

		// Declare and allocate memory for a vector containing the uniform indices.
		std::vector<GLuint> indices(activeCount);
		// Get the uniform indices.
		glGetUniformIndices(m_ID, activeCount, nameBuffer.data(), indices.data());

		// Get some data about the uniform.
		std::vector<GLint> types(activeCount);
		std::vector<GLint> counts(activeCount);
		std::vector<GLint> blockIndices(activeCount);
		std::vector<GLint> offsets(activeCount);
		glGetActiveUniformsiv(m_ID, activeCount, indices.data(), GL_UNIFORM_TYPE, types.data());
		glGetActiveUniformsiv(m_ID, activeCount, indices.data(), GL_UNIFORM_SIZE, counts.data());
		glGetActiveUniformsiv(m_ID, activeCount, indices.data(), GL_UNIFORM_BLOCK_INDEX, blockIndices.data());
		glGetActiveUniformsiv(m_ID, activeCount, indices.data(), GL_UNIFORM_OFFSET, offsets.data());

		OHM_CORE_INFO("Active uniform data for: {}\n", m_Name);

		// Create ShaderUniforms


		for (uint32_t i = 0; i < activeCount; i++)
		{
			if (blockIndices[i] != -1)
			{
				// Non default block uniform
			}
			else
			{
				const std::string name = activeUniformNames[i];
				GLenum type = types[i];
				ShaderDataType shaderDataType = ShaderDataTypeFromGLenum(type);
				const char* shaderTypeToString = ShaderDataTypeToString[shaderDataType];
				GLint offset = offsets[i];

				GLint count = counts[i];
				uint32_t size = count * ShaderDataTypeSize(shaderDataType);

				GLint blockIndex = blockIndices[i];

				m_Uniforms[name] = ShaderUniform(name, shaderDataType, size, count, offset);
			}
		}

		for (auto [name, uniform] : m_Uniforms)
		{
			OHM_CORE_INFO("Name: {}, Type: {}, Count: {}, Size: {}, Block Offset: {}\n", uniform.GetName(), ShaderDataTypeToString[uniform.GetType()], uniform.GetCount(), uniform.GetSize(), uniform.GetBlockOffset());
		}
	}

	GLint Shader::UploadUniformFloat(const std::string& name, float value)
	{
		GLint location = glGetUniformLocation(m_ID, name.c_str());
		glUniform1f(location, value);
		return location;
	}

	GLint Shader::UploadUniformFloat2(const std::string& name, const glm::vec2& value)
	{
		GLint location = glGetUniformLocation(m_ID, name.c_str());
		glUniform2f(location, value.x, value.y);
		return location;
	}

	GLint Shader::UploadUniformFloat3(const std::string& name, const glm::vec3& value)
	{
		GLint location = glGetUniformLocation(m_ID, name.c_str());
		glUniform3f(location, value.x, value.y, value.z);
		return location;
	}

	GLint Shader::UploadUniformFloat4(const std::string& name, const glm::vec4& value)
	{
		GLint location = glGetUniformLocation(m_ID, name.c_str());
		glUniform4f(location, value.x, value.y, value.z, value.w);
		return location;
	}

	GLint Shader::UploadUniformInt(const std::string& name, int value)
	{
		GLint location = glGetUniformLocation(m_ID, name.c_str());
		glUniform1i(location, value);
		return location;
	}

	GLint Shader::UploadUniformIntArray(const std::string& name, uint32_t count, int* basePtr)
	{
		GLint location = glGetUniformLocation(m_ID, name.c_str());
		glUniform1iv(location, count, basePtr);
		return location;
	}

	GLint Shader::UploadUniformMat3(const std::string& name, const glm::mat3& matrix)
	{
		GLint location = glGetUniformLocation(m_ID, name.c_str());
		glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
		return location;
	}

	GLint Shader::UploadUniformMat4(const std::string& name, const glm::mat4& matrix)
	{
		GLint location = glGetUniformLocation(m_ID, name.c_str());
		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
		return location;
	}


}