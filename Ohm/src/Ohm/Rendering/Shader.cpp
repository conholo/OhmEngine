#include "ohmpch.h"
#include "Ohm/Rendering/Shader.h"
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include "Ohm/Rendering/Material.h"

namespace Ohm
{
	ShaderUniform::ShaderUniform(std::string name, GLint location, ShaderDataType type, uint32_t size, uint32_t count, int32_t blockOffset)
		:m_Name(std::move(name)), m_Location(location), m_Type(type), m_Count(count), m_Size(size), m_BlockOffset(blockOffset)
	{

	}

	ShaderBlock::ShaderBlock(std::string name, uint32_t size, uint32_t memberCount, uint32_t binding, uint32_t blockIndex)
		:m_Name(std::move(name)), m_BlockSize(size), m_MemberCount(memberCount), m_Binding(binding), m_BlockIndex(blockIndex)
	{
		m_UBO = CreateRef<UniformBuffer>(size, binding);
	}

	void ShaderBlock::UploadUBO(const void* Data) const
	{
		m_UBO->SetData(Data, m_BlockSize);
	}

	static ShaderDataType ShaderDataTypeFromGLenum(GLenum value)
	{
		switch (value)
		{
		case GL_FLOAT:				return ShaderDataType::Float;
		case GL_FLOAT_VEC2: 		return ShaderDataType::Float2;
		case GL_FLOAT_VEC3: 		return ShaderDataType::Float3;
		case GL_FLOAT_VEC4: 		return ShaderDataType::Float4;
		case GL_INT:				return ShaderDataType::Int;
		case GL_FLOAT_MAT3: 		return ShaderDataType::Mat3;
		case GL_FLOAT_MAT4: 		return ShaderDataType::Mat4;
		case GL_SAMPLER_2D:			return ShaderDataType::Sampler2D;
		case GL_SAMPLER_CUBE:		return ShaderDataType::SamplerCube;
		default:
			return ShaderDataType::None;
		}
	}
	
	Shader::Shader(const std::string& filePath)
	{
		const size_t shaderLocationOffset = filePath.rfind("\/") + 1;
		const size_t extensionOffset = filePath.find_first_of(".", shaderLocationOffset);

		m_Name = filePath.substr(shaderLocationOffset, extensionOffset - shaderLocationOffset);

		std::string source = ReadFile(filePath);
		AddIncludeFiles(source);
		const auto shaderSources = PreProcess(source);
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
		ASSERT(input, "Unable to read shader source file: '{}'", filePath);
		
		input.seekg(0, std::ios::end);
		const size_t size = input.tellg();
		
		if (size != -1)
		{
			result.resize(size);
			input.seekg(0, std::ios::beg);
			input.read(result.data(), size);
		}
		
		return result;
	}

	GLenum ShaderTypeFromString(const std::string& type)
	{
		if (type == "vertex")
			return GL_VERTEX_SHADER;
		if (type == "fragment")
			return GL_FRAGMENT_SHADER;
		if (type == "compute")
			return GL_COMPUTE_SHADER;
		if (type == "geometry")
			return GL_GEOMETRY_SHADER;

		return GL_FALSE;
	}

	void Shader::LogShaderData()
	{
		if (m_ActiveTotalUniformCount <= 0)
		{
			OHM_CORE_INFO("{} Shader has no active uniforms.", m_Name);
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
			OHM_CORE_INFO("Block Size: {}", block.GetBlockSize());
			OHM_CORE_INFO("Member Count: {}", block.GetMemberCount());

			for (auto [Name, uniform] : block.GetUniforms())
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

	void Shader::ClearBinding()
	{
		glUseProgram(0);
	}

	std::unordered_map<GLenum, std::string> Shader::PreProcess(const std::string& source)
	{
		const char* typeToken = "#type";
		const size_t typeTokenLength = strlen(typeToken);
		size_t position = source.find(typeToken, 0);

		std::unordered_map<GLenum, std::string> result;

		while (position != std::string::npos)
		{
			const size_t endOfLine = source.find_first_of("\r\n", position);
			const size_t beginShaderType = position + typeTokenLength + 1;
			std::string type = source.substr(beginShaderType, endOfLine - beginShaderType);

			const size_t nextLinePosition = source.find_first_not_of("\r\n", endOfLine);
			position = source.find(typeToken, nextLinePosition);
			result[ShaderTypeFromString(type)] =
				(position == std::string::npos)
				? source.substr(nextLinePosition)
				: source.substr(nextLinePosition, position - nextLinePosition);
		}

		return result;
	}

	void Shader::AddIncludeFiles(std::string& outSource)
	{
		const char* typeToken = "//include";
		const size_t typeTokenLength = strlen(typeToken);
		size_t position = outSource.find(typeToken, 0);
		const std::string pathTo = "assets/shaders/"; 

		while (position != std::string::npos)
		{
			const size_t endOfLine = outSource.find_first_of("\r\n", position);
			size_t beginIncludeName = position + typeTokenLength + 1;
			std::string includeFileName = outSource.substr(beginIncludeName, endOfLine - beginIncludeName);

			includeFileName.erase(remove(includeFileName.begin(),includeFileName.end(), '\"'), includeFileName.end());
			const std::string newPath = pathTo + includeFileName;
			
			size_t nextLinePosition = outSource.find_first_not_of("\r\n", endOfLine);
			std::string includeContents = ReadFile(newPath);
			outSource.insert(nextLinePosition, includeContents + "\n");
			position = outSource.find(typeToken, nextLinePosition);
		}
	}

	void* Shader::GetUniformData(ShaderDataType type, GLint location)
	{
		switch (type)
		{
		case ShaderDataType::None:
			{
				return nullptr;
			}
		case ShaderDataType::Float:
		case ShaderDataType::Float2:
		case ShaderDataType::Float3:
		case ShaderDataType::Float4:
			{
				GLfloat* data = (GLfloat*)malloc(ShaderDataTypeSize(type));
				glGetUniformfv(m_ID, location, data);

				return data;
			}
		case ShaderDataType::SamplerCube:
		case ShaderDataType::Sampler2D:
			{
				GLint* data = (GLint*)malloc(ShaderDataTypeSize(type));
				glGetUniformiv(m_ID, location, data);
				(TextureUniform*)data;

				return data;
			}
		case ShaderDataType::Int:
			{
				GLint* data = (GLint*)malloc(ShaderDataTypeSize(type));
				glGetUniformiv(m_ID, location, data);
				return data;
			}
		case ShaderDataType::Mat3:
		case ShaderDataType::Mat4:
			{
				OHM_CORE_INFO("Currently unable to retrieve uniform data for uniforms of type Mat3/Mat4");
				return nullptr;
			}
		}
		
		return nullptr;
	}

	void Shader::Compile(const std::unordered_map<GLenum, std::string>& shaderSources)
	{
		const GLuint program = glCreateProgram();
		std::vector<GLuint> shaderIDs;

		for (auto& kv : shaderSources)
		{
			const GLenum shaderType = kv.first;
			m_IsCompute = shaderType == GL_COMPUTE_SHADER;
			const std::string& source = kv.second;

			GLuint shader = glCreateShader(shaderType);

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
				std::string shaderLog (infoLog.begin(), infoLog.end());
				std::string shaderTypeName;

				if (shaderType == GL_FRAGMENT_SHADER)
					shaderTypeName = "FRAGMENT COMPILATION ERROR";
				else if (shaderType == GL_VERTEX_SHADER)
					shaderTypeName = "VERTEX COMPILATION ERROR";
				else if (shaderType == GL_GEOMETRY_SHADER)
					shaderTypeName = "GEOMETRY COMPILATION ERROR";
				else if (shaderType == GL_COMPUTE_SHADER)
					shaderTypeName = "COMPUTE COMPILATION ERROR";

				OHM_CORE_ERROR(m_Name);
				OHM_CORE_ERROR("Error Type: {}", shaderTypeName);
				OHM_CORE_ERROR("Error Log:\n{}", shaderLog);
				
				glDeleteShader(shader);

				break;
			}

			glAttachShader(program, shader);
			shaderIDs.push_back(shader);
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

			std::stringstream ss;

			const std::string messageHeader = m_Name + ": LINKING ERROR: ";
			ss << messageHeader << infoLog.data();
			OHM_CORE_ERROR(ss.str());

			glDeleteProgram(program);

			for (auto id : shaderIDs)
				glDeleteShader(id);

			return;
		}

		for (auto id : shaderIDs)
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

			// Otherwise, query OpenGL for data about the block and construct a new Block object to store the data.
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
			if(name.substr(0, 3) == "gl_")
				continue;
			GLenum type = types[i];
			GLint location = glGetUniformLocation(m_ID, name.c_str());
			ShaderDataType shaderDataType = ShaderDataTypeFromGLenum(type);
			GLint blockOffset = blockOffsets[i];

			GLint count = counts[i];
			uint32_t size = count * ShaderDataTypeSize(shaderDataType);

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

	GLint Shader::UploadUniformBool(const std::string& name, bool value)
	{
		GLint location = glGetUniformLocation(m_ID, name.c_str());
		glUniform1f(location, value);
		return location;
	}

	const std::unordered_map<std::string, ShaderUniform>& Shader::GetAllUniformsFromBlock(const std::string& BlockName)
	{
		ASSERT(m_Blocks.find(BlockName) != m_Blocks.end(), "Unable to retrieve uniforms with unknown block name: {}", BlockName);
		return m_Blocks[BlockName].GetUniforms();
	}

	std::vector<ShaderUniform> Shader::GetBaseBlockUniformsOfType(ShaderDataType Type)
	{
		std::vector<ShaderUniform> Match;
		for(auto [UniformName, Uniform] : m_BaseBlockUniforms)
		{
			if(Uniform.GetType() == Type)
				Match.push_back(Uniform);
		}

		return Match;
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

	GLint Shader::UploadUniformFloat3Array(const std::string& name, uint32_t count, glm::vec3* value)
	{
		GLint location = glGetUniformLocation(m_ID, name.c_str());
		glUniform3fv(location, count, glm::value_ptr(value[0]));
		return location;
	}

	GLint Shader::UploadUniformFloat2Array(const std::string& name, uint32_t count, glm::vec2* value)
	{
		GLint location = glGetUniformLocation(m_ID, name.c_str());
		glUniform2fv(location, count, glm::value_ptr(value[0]));
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

	void Shader::EnableAllBarriersBits()
	{
		glMemoryBarrier(GL_ALL_BARRIER_BITS);
	}

	void Shader::EnableShaderImageAccessBarrierBit()
	{
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
	}

	void Shader::EnableTextureFetchBarrierBit()
	{
		glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
	}

	void Shader::EnableShaderStorageBarrierBit()
	{
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	}

	void Shader::EnableAtomicCounterBarrierBit()
	{
		glMemoryBarrier(GL_ATOMIC_COUNTER_BARRIER_BIT);
	}

	void Shader::EnableBufferUpdateBarrierBit()
	{
		glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);
	}
	
	void Shader::DispatchCompute(uint32_t groupX, uint32_t groupY, uint32_t groupZ)
	{
		if (!m_IsCompute)
		{
			OHM_CORE_WARN("Shader: '{}' is not of type Compute Shader.  Cannot dispatch.", m_Name);
			return;
		}

		glUseProgram(m_ID);
		glDispatchCompute(groupX, groupY, groupZ);
	}

	std::unordered_map<std::string, Ref<Shader>> ShaderLibrary::s_ShaderLibrary;

	void ShaderLibrary::Add(const Ref<Shader>& shader)
	{
		if (s_ShaderLibrary.find(shader->GetName()) == s_ShaderLibrary.end())
		{
			s_ShaderLibrary[shader->GetName()] = shader;
		}
		else
		{
			OHM_CORE_WARN("Shader already contained in Shader Library.  Attempted to add '{}' Shader to Library.", shader->GetName());
		}
	}

	void ShaderLibrary::Load(const std::string& filePath)
	{
		Ref<Shader> shader = CreateRef<Shader>(filePath);
		Add(shader);
	}

	const Ref<Shader>& ShaderLibrary::Get(const std::string& name)
	{
		ASSERT(s_ShaderLibrary.find(name) != s_ShaderLibrary.end(), "No shader with name: '{}' found in Shader Library.", name);
		return s_ShaderLibrary.at(name);
	}
}

