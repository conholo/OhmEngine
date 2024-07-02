#pragma once

#include <stdint.h>
#include <vector>
#include <string>
#include <unordered_map>

namespace Ohm
{
	enum class ShaderDataType
	{
		None = 0, Float, Float2, Float3, Float4, Int, Bool, Mat3, Mat4, Sampler2D, Sampler3D, SamplerCube, Image2D, ImageCube
	};

	static std::unordered_map<ShaderDataType, const char*> ShaderDataTypeToString =
	{
		{ShaderDataType::Float,		    "float"},
		{ShaderDataType::Float2,	    	"vec2"},
		{ShaderDataType::Float3,	    	"vec3"},
		{ShaderDataType::Float4,	    	"vec4"},
		{ShaderDataType::Int,		    "int"},
		{ShaderDataType::Bool,		    "bool"},
		{ShaderDataType::Mat3,		    "mat3"},
		{ShaderDataType::Mat4,		    "mat4"},
		{ShaderDataType::Sampler2D,		"sampler2D"},
		{ShaderDataType::Sampler3D,		"sampler3D"},
		{ShaderDataType::SamplerCube,	"samplerCube"},
		{ShaderDataType::Image2D,		"image2D"},
		{ShaderDataType::ImageCube,		"imageCube"},
	};

	static uint32_t ShaderDataTypeSize(ShaderDataType type)
	{
		switch (type)
		{
			case ShaderDataType::Float:		return 1 * 4;
			case ShaderDataType::Float2:	return 2 * 4;
			case ShaderDataType::Float3:	return 3 * 4;
			case ShaderDataType::Float4:	return 4 * 4;
			case ShaderDataType::Bool:
			case ShaderDataType::Int:		return 1 * 4;
			case ShaderDataType::Mat3:		return 3 * 3 * 4;
			case ShaderDataType::Mat4:		return 4 * 4 * 4;
			// 3 Ints, third int represents bool for hide in inspector, made int to keep better alignment
			case ShaderDataType::Image2D:
			case ShaderDataType::ImageCube:
			case ShaderDataType::SamplerCube:
			case ShaderDataType::Sampler3D:
			case ShaderDataType::Sampler2D:	return 3 * 4;
			default:						return 0;
		}
	}

	struct BufferElement
	{
		std::string Name;
		ShaderDataType Type;
		uint32_t Size;
		uint32_t Offset;
		bool Normalized;

		BufferElement() = default;

		BufferElement(std::string name, ShaderDataType type, bool normalized = false)
			:Name(std::move(name)), Type(type), Size(ShaderDataTypeSize(type)), Offset(0), Normalized(normalized) { }

		uint32_t GetComponentCount() const
		{
			switch (Type)
			{
				case ShaderDataType::Float:		return 1;
				case ShaderDataType::Float2:	return 2;
				case ShaderDataType::Float3:	return 3;
				case ShaderDataType::Float4:	return 4;
				case ShaderDataType::Bool:		return 1;
				case ShaderDataType::Int:		return 1;
				case ShaderDataType::Mat3:		return 3 * 3;
				case ShaderDataType::Mat4:		return 4 * 4;
				default:						return 0;
			}
		}
	};


	class BufferLayout
	{
	public:
		BufferLayout() = default;

		BufferLayout(const std::initializer_list<BufferElement> elements)
			:m_Elements(elements)
		{
			CalculateStrideAndOffsets();
		}

		uint32_t GetStride() const { return m_Stride; }

		std::vector<BufferElement>& GetElements() { return m_Elements; }
		const std::vector<BufferElement>& GetElements() const { return m_Elements; }

		std::vector<BufferElement>::iterator begin() { return m_Elements.begin(); }
		std::vector<BufferElement>::iterator end() { return m_Elements.end(); }

		std::vector<BufferElement>::const_iterator begin() const { return m_Elements.begin(); }
		std::vector<BufferElement>::const_iterator end() const { return m_Elements.end(); }

	private:
		void CalculateStrideAndOffsets()
		{
			m_Stride = 0;
			uint32_t offset = 0;

			for (auto& element : m_Elements)
			{
				element.Offset = offset;
				offset += element.Size;
				m_Stride += element.Size;
			}
		}

	private:
		uint32_t m_Stride = 0;
		std::vector<BufferElement> m_Elements;
	};
}