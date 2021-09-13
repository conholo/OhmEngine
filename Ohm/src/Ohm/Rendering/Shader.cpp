#include "ohmpch.h"
#include "Ohm/Rendering/Shader.h"
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

namespace Ohm
{
	ShaderUniform::ShaderUniform(const std::string& name, GLint location, ShaderDataType type, uint32_t size, uint32_t count, int32_t blockOffset)
		:m_Name(name), m_Location(location), m_Type(type), m_Count(count), m_Size(size), m_BlockOffset(blockOffset)
	{

	}

	ShaderBlock::ShaderBlock(const std::string& name, uint32_t size, uint32_t memberCount, uint32_t binding, uint32_t blockIndex)
		:m_Name(name), m_BlockSize(size), m_MemberCount(memberCount), m_Binding(binding), m_BlockIndex(blockIndex)
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

	void Shader::DumpShaderData()
	{
		if (m_ActiveTotalUniformCount <= 0)
		{
			OHM_INFO("{} Shader has no active uniforms.", m_Name);
			return;
		}

		OHM_CORE_INFO("-------------------------------------------------");
		OHM_CORE_INFO("Shader data for: {}", m_Name);
		OHM_CORE_INFO("-------------------------------------------------\n");
		OHM_CORE_INFO("Active uniform count: {}", m_ActiveTotalUniformCount);
		OHM_CORE_INFO("Base block uniforms: {}", m_DefaultBlockUniformCount);
		OHM_CORE_INFO("Named blocks: {}", m_Blocks.size());
		OHM_CORE_INFO("Named block uniforms: {}", m_NamedBlockUniformCount);
		OHM_CORE_INFO("\n");
		OHM_CORE_INFO("-------------------------------------------------");
		OHM_CORE_INFO("Base Block uniforms:");
		OHM_CORE_INFO("-------------------------------------------------");
		for (auto [name, uniform] : m_BaseBlockUniforms)
		{
			OHM_CORE_INFO("-------------------------------------------------");
			OHM_CORE_INFO("Uniform Name: {}", name);
			OHM_CORE_INFO("		Size: {}", uniform.GetSize());
			OHM_CORE_INFO("		Type: {}", ShaderDataTypeToString[uniform.GetType()]);
			OHM_CORE_INFO("		BufferOffset: {}", uniform.GetBufferOffset());
			OHM_CORE_INFO("		Location: {}", uniform.GetLocation());
			OHM_CORE_INFO("-------------------------------------------------");
		}

		OHM_CORE_INFO("\n");
		OHM_CORE_INFO("-------------------------------------------------");
		OHM_CORE_INFO("Named Blocks:");
		OHM_CORE_INFO("-------------------------------------------------");
		for (auto [blockName, block] : m_Blocks)
		{
			OHM_CORE_INFO("Block Name: {}", blockName);
			OHM_CORE_INFO("Block Binding: {}", block.GetBinding());

			for (auto uniform : block.GetUniforms())
			{
				OHM_CORE_INFO("-------------------------------------------------");
				OHM_CORE_INFO("		Uniform Name: {}", uniform.GetName());
				OHM_CORE_INFO("			Size: {}", uniform.GetSize());
				OHM_CORE_INFO("			Type: {}", ShaderDataTypeToString[uniform.GetType()]);
				OHM_CORE_INFO("			BlockOffset: {}", uniform.GetBlockOffset());
				OHM_CORE_INFO("-------------------------------------------------");
			}

			OHM_CORE_INFO("-------------------------------------------------\n");
		}
	}

	void* Shader::GetUniformData(ShaderDataType type, GLint location)
	{
		switch (type)
		{
			case Ohm::ShaderDataType::None:
			{
				OHM_CORE_ERROR("Unknown shader data type.");
				return nullptr;
			}
			case Ohm::ShaderDataType::Float:
			case Ohm::ShaderDataType::Float2:
			case Ohm::ShaderDataType::Float3:
			case Ohm::ShaderDataType::Float4:
			{
				GLfloat* data = (GLfloat*)malloc(ShaderDataTypeSize(type));
				glGetUniformfv(m_ID, location, data);

				return (void*)data;
			}
			case Ohm::ShaderDataType::Int:
			{
				GLint* data = (GLint*)malloc(ShaderDataTypeSize(type));
				glGetUniformiv(m_ID, location, data);

				return (void*)data;
			}
			case Ohm::ShaderDataType::Mat3:
			case Ohm::ShaderDataType::Mat4:
			case Ohm::ShaderDataType::Sampler2D:
			{
				OHM_CORE_INFO("We're currently unable to retrieve data from uniforms of type Mat3/Mat4/Sampler2D.");
				return nullptr;
			}
		}

		return nullptr;
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

		m_ActiveTotalUniformCount = activeCount;

		if (activeCount == 0)
		{
			OHM_CORE_WARN("No active uniforms were found when reflecting {}.", m_Name);
			return;
		}

		m_ActiveTotalUniformCount = activeCount;

		GLsizei maxUniformLength;
		glGetProgramiv(m_ID, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxUniformLength);

		std::vector<std::string> activeUniformNames(activeCount);

		for (GLuint i = 0; i < m_ActiveTotalUniformCount; ++i)
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
		for (GLsizei i = 0; i < activeCount; i++)
			nameBuffer[i] = (activeUniformNames[i]).c_str();

		// Declare and allocate memory for a vector containing the uniform indices.
		std::vector<GLuint> indices(activeCount);
		// Get the uniform indices.
		glGetUniformIndices(m_ID, activeCount, nameBuffer.data(), indices.data());

		// Get some data about the uniform.
		std::vector<GLint> types(activeCount);
		std::vector<GLint> counts(activeCount);
		std::vector<GLint> blockIndices(activeCount);
		std::vector<GLint> blockOffsets(activeCount);
		glGetActiveUniformsiv(m_ID, activeCount, indices.data(), GL_UNIFORM_TYPE, types.data());
		glGetActiveUniformsiv(m_ID, activeCount, indices.data(), GL_UNIFORM_SIZE, counts.data());
		glGetActiveUniformsiv(m_ID, activeCount, indices.data(), GL_UNIFORM_BLOCK_INDEX, blockIndices.data());
		glGetActiveUniformsiv(m_ID, activeCount, indices.data(), GL_UNIFORM_OFFSET, blockOffsets.data());		

		// Create ShaderUniforms/Blocks

		uint32_t currentOffset = 0;

		std::vector<uint32_t> registeredBlockIndices;

		// Named Blocks
		for (uint32_t i = 0; i < blockIndices.size(); i++)
		{
			// If the block index is -1 (base block) or we've already registered the block, move on.
			if (blockIndices[i] == -1 || std::find(registeredBlockIndices.begin(), registeredBlockIndices.end(), blockIndices[i]) != registeredBlockIndices.end()) continue;

			// Otherwise, query OpenGL for data about the block and construct a new Ohm Block object to store the data.
			GLint binding;
			GLint blockSize;
			GLint activeCount;

			std::vector<GLchar> nameBuffer(maxUniformLength);
			GLint nameLength;

			glGetActiveUniformBlockName(m_ID, blockIndices[i], maxUniformLength, &nameLength, nameBuffer.data());
			glGetActiveUniformBlockiv(m_ID, blockIndices[i], GL_UNIFORM_BLOCK_BINDING, &binding);
			glGetActiveUniformBlockiv(m_ID, blockIndices[i], GL_UNIFORM_BLOCK_DATA_SIZE, &blockSize);
			glGetActiveUniformBlockiv(m_ID, blockIndices[i], GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &activeCount);

			GLint* activeIndices = (GLint*)malloc(sizeof(GLint) * activeCount);
			glGetActiveUniformBlockiv(m_ID, blockIndices[i], GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, activeIndices);

			std::string name(nameBuffer.data(), nameLength);

			m_Blocks[name] = ShaderBlock(name, blockSize, activeCount, binding, blockIndices[i]);

			registeredBlockIndices.push_back(blockIndices[i]);
		}


		m_NamedBlockUniformCount = 0;
		m_DefaultBlockUniformCount = 0;

		// Create uniform objects and store in respective data structures.
		// Named block uniforms -> m_Blocks unordered map
		// Default block uniforms -> m_BaseBlockUniforms unordered map
		for (uint32_t i = 0; i < m_ActiveTotalUniformCount; i++)
		{
			const std::string name = activeUniformNames[i];
			GLenum type = types[i];
			GLint location = glGetUniformLocation(m_ID, name.c_str());
			ShaderDataType shaderDataType = ShaderDataTypeFromGLenum(type);
			const char* shaderTypeToString = ShaderDataTypeToString[shaderDataType];
			GLint blockOffset = blockOffsets[i];


			GLint count = counts[i];
			uint32_t size = count * ShaderDataTypeSize(shaderDataType);

			GLint blockIndex = blockIndices[i];

			ShaderUniform uniform(name, location, shaderDataType, size, count, blockOffset);

			if (blockIndices[i] != -1)
			{
				// Add the uniform to it's respective block.

				std::vector<GLchar> nameBuffer(maxUniformLength);
				GLint blockNameLength;

				glGetActiveUniformBlockName(m_ID, blockIndices[i], maxUniformLength, &blockNameLength, nameBuffer.data());

				std::string name(nameBuffer.data(), blockNameLength);

				m_NamedBlockUniformCount++;
				m_Blocks[name].AddUniform(uniform);
			}
			else
			{

				uint32_t bufferOffset = currentOffset;
				currentOffset += ShaderDataTypeSize(shaderDataType);

				uniform.SetBufferOffsetForDefaultBlockUniform(bufferOffset);

				// Add the uniform to the base block storage.
				m_DefaultBlockUniformCount++;
				m_BaseBlockUniforms[name] = uniform;
			}
		}
	}

	GLint Shader::UploadUniformFloat(const std::string& name, float value)
	{
		GLint location = m_BaseBlockUniforms[name].GetLocation();
		glUniform1f(location, value);
		return location;
	}

	GLint Shader::UploadUniformFloat2(const std::string& name, const glm::vec2& value)
	{
		GLint location = m_BaseBlockUniforms[name].GetLocation();

		glUniform2f(location, value.x, value.y);
		return location;
	}

	GLint Shader::UploadUniformFloat3(const std::string& name, const glm::vec3& value)
	{
		GLint location = m_BaseBlockUniforms[name].GetLocation();
		glUniform3f(location, value.x, value.y, value.z);
		return location;
	}

	GLint Shader::UploadUniformFloat4(const std::string& name, const glm::vec4& value)
	{
		GLint location = m_BaseBlockUniforms[name].GetLocation();
		glUniform4f(location, value.x, value.y, value.z, value.w);
		return location;
	}

	GLint Shader::UploadUniformInt(const std::string& name, int value)
	{
		GLint location = m_BaseBlockUniforms[name].GetLocation();
		glUniform1i(location, value);
		return location;
	}

	GLint Shader::UploadUniformIntArray(const std::string& name, uint32_t count, int* basePtr)
	{
		GLint location = m_BaseBlockUniforms[name].GetLocation();
		glUniform1iv(location, count, basePtr);
		return location;
	}

	GLint Shader::UploadUniformMat3(const std::string& name, const glm::mat3& matrix)
	{
		GLint location = m_BaseBlockUniforms[name].GetLocation();
		glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
		return location;
	}

	GLint Shader::UploadUniformMat4(const std::string& name, const glm::mat4& matrix)
	{
		GLint location = m_BaseBlockUniforms[name].GetLocation();
		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
		return location;
	}
	
}