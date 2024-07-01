#include "ohmpch.h"
#include "Ohm/Scene/SceneSerializer.h"
#include "Ohm/Scene/Entity.h"
#include "Ohm/Rendering/Mesh.h"
#include "Ohm/Rendering/Material.h"

#include <glm/glm.hpp>
#include <yaml-cpp/yaml.h>


namespace YAML
{
	template<>
	struct convert<Ohm::TextureUniform>
	{
		static Node encode(const Ohm::TextureUniform texUniform)
		{
			Node node;
			node.push_back(texUniform.RendererID);
			node.push_back(texUniform.HideInUI);
			node.push_back(texUniform.TextureUnit);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, Ohm::TextureUniform& texUniform)
		{
			if (!node.IsSequence() || node.size() != 3)
				return false;

			texUniform.RendererID = node[0].as<uint32_t>();
			texUniform.HideInUI = node[1].as<int32_t>();
			texUniform.TextureUnit = node[2].as<int32_t>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec2>
	{
		static Node encode(const glm::vec2& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, glm::vec2& rhs)
		{
			if (!node.IsSequence() || node.size() != 2)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec3>
	{
		static Node encode(const glm::vec3& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, glm::vec3& rhs)
		{
			if (!node.IsSequence() || node.size() != 3)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec4>
	{
		static Node encode(const glm::vec4& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.push_back(rhs.w);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, glm::vec4& rhs)
		{
			if (!node.IsSequence() || node.size() != 4)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			rhs.w = node[3].as<float>();
			return true;
		}
	};

}

namespace Ohm
{
	SceneSerializer::SceneSerializer(const Ref<Scene>& scene)
		:m_Scene(scene)
	{

	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const TextureUniform& texUniform)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << texUniform.RendererID << texUniform.HideInUI << texUniform.TextureUnit << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec2& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec3& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec4& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
		return out;
	}

	static void SerializeEntity(YAML::Emitter& out, Entity entity)
	{
		auto& idComponent = entity.GetComponent<IDComponent>();

		out << YAML::BeginMap;
		out << YAML::Key << "Entity" << YAML::Value << std::to_string(idComponent.ID);

		if (entity.HasComponent<IDComponent>())
		{
			out << YAML::Key << "IDComponent";
			out << YAML::BeginMap;
			out << YAML::Key << "ID" << YAML::Value << idComponent.ID;
			out << YAML::EndMap;
		}
		
		if (entity.HasComponent<TagComponent>())
		{
			out << YAML::Key << "TagComponent";
			out << YAML::BeginMap;

			auto& tagComponent = entity.GetComponent<TagComponent>();
			out << YAML::Key << "Tag" << YAML::Value << tagComponent.Tag;

			out << YAML::EndMap;
		}

		if (entity.HasComponent<TransformComponent>())
		{
			out << YAML::Key << "TransformComponent";
			out << YAML::BeginMap;

			auto& tc = entity.GetComponent<TransformComponent>();
			out << YAML::Key << "Translation" << YAML::Value << tc.Translation;
			out << YAML::Key << "Rotation" << YAML::Value << tc.RotationDegrees;
			out << YAML::Key << "Scale" << YAML::Value << tc.Scale;

			out << YAML::EndMap;
		}

		if (entity.HasComponent<MeshRendererComponent>())
		{
			out << YAML::Key << "MeshRendererComponent";
			out << YAML::BeginMap;

			auto& meshRenderer = entity.GetComponent<MeshRendererComponent>();
			const uint32_t primitiveType = (uint32_t)meshRenderer.MeshData->GetPrimitiveType();
			out << YAML::Key << "PrimitiveType" << YAML::Value << primitiveType;

			MaterialUniformData materialData = meshRenderer.MaterialInstance->GetMaterialUniformData();

				out << YAML::Key << "Shader Name" << YAML::Value << meshRenderer.MaterialInstance->GetShader()->GetName();
				out << YAML::Key << "Material Name" << YAML::Value << meshRenderer.MaterialInstance->GetName();
				out << YAML::Key << "Material Uniforms";
				out << YAML::BeginMap;

					out << YAML::Key << "FloatUniforms";
					out << YAML::BeginMap;
					for (auto [name, floatValue] : materialData.FloatUniforms)
						out << YAML::Key << name << YAML::Value << floatValue;
					out << YAML::EndMap;

					out << YAML::Key << "IntUniforms";
					out << YAML::BeginMap;
					for (auto [name, intValue] : materialData.IntUniforms)
						out << YAML::Key << name << YAML::Value << intValue;
					out << YAML::EndMap;

					out << YAML::Key << "Vec2Uniforms";
					out << YAML::BeginMap;
					for (auto [name, vec2Value] : materialData.Vec2Uniforms)
						out << YAML::Key << name << YAML::Value << vec2Value;
					out << YAML::EndMap;

					out << YAML::Key << "Vec3Uniforms";
					out << YAML::BeginMap;
					for (auto [name, vec3Value] : materialData.Vec3Uniforms)
						out << YAML::Key << name << YAML::Value << vec3Value;
					out << YAML::EndMap;

					out << YAML::Key << "Vec4Uniforms";
					out << YAML::BeginMap;
					for (auto [name, vec4Value] : materialData.Vec4Uniforms)
						out << YAML::Key << name << YAML::Value << vec4Value;
					out << YAML::EndMap;

					out << YAML::Key << "TextureUniforms";
					out << YAML::BeginMap;
					for (auto [name, textureValue] : materialData.TextureUniforms)
						out << YAML::Key << name << YAML::Value << textureValue;
					out << YAML::EndMap;

				out << YAML::EndMap;

			out << YAML::EndMap;
		}

		if (entity.HasComponent<DirectionalLightComponent>())
		{
			out << YAML::Key << "DirectionalLightComponent";
			out << YAML::BeginMap;

			auto& light = entity.GetComponent<DirectionalLightComponent>();
			out << YAML::Key << "Radiance" << YAML::Value << light.Radiance;
			out << YAML::Key << "Intensity" << YAML::Value << light.Intensity;
			out << YAML::Key << "LightDirection" << YAML::Value << light.LightDirection;
			out << YAML::Key << "ShadowAmount" << YAML::Value << light.ShadowAmount;

			out << YAML::EndMap;
		}

		out << YAML::EndMap;
	}

	void SceneSerializer::Serialize(const std::string& filePath)
	{
		YAML::Emitter out;
		out << YAML::BeginMap;
		
		out << YAML::Key << "Scene" << YAML::Value << "Untitled";
		out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;

		m_Scene->m_Registry.each([&](auto entityID)
			{
				Entity entity = { entityID, m_Scene.get() };
				if (!entity) return;

				SerializeEntity(out, entity);
			});

		out << YAML::EndSeq;
		out << YAML::EndMap;

		std::ofstream fout(filePath);
		fout << out.c_str();
	}

	bool SceneSerializer::Deserialize(const std::string& filePath)
	{
		YAML::Node data;
		try
		{
			data = YAML::LoadFile(filePath);
		}
		catch (YAML::ParserException e)
		{
			return false;
		}

		if (!data["Scene"])
			return false;

		std::string sceneName = data["Scene"].as<std::string>();
		OHM_CORE_TRACE("Deserializing scene '{0}'", sceneName);

		if (auto entities = data["Entities"])
		{
			for (auto entity : entities)
			{
				uint32_t uuid = entity["Entity"].as<uint64_t>();

				std::string tag;
				if (auto tagData = entity["TagComponent"])
					tag = tagData["Tag"].as<std::string>();

				OHM_CORE_TRACE("Deserialized Entity with ID: {0}, Name: {1}", uuid, tag);

				Entity deserializedEntity = m_Scene->CreateEntity(tag);

				if (auto transformData = entity["TransformComponent"])
				{
					auto& tc = deserializedEntity.GetComponent<TransformComponent>();
					tc.Translation = transformData["Translation"].as<glm::vec3>();
					tc.RotationDegrees = transformData["Rotation"].as<glm::vec3>();
					tc.Scale = transformData["Scale"].as<glm::vec3>();
				}

				if (auto meshRendererData = entity["MeshRendererComponent"])
				{
					auto& meshRendererComponent = deserializedEntity.AddComponent<MeshRendererComponent>();
					uint32_t primitiveType = meshRendererData["PrimitiveType"].as<uint32_t>();

					meshRendererComponent.MeshData = MeshFactory::Create(static_cast<Primitive>(primitiveType));

					if (auto materialUniformData = meshRendererData["Material Uniforms"])
					{
						std::string shaderName = meshRendererData["Shader Name"].as<std::string>();
						std::string materialName = meshRendererData["Material Name"].as<std::string>();
						
						Ref<Material> deserializedMaterial = CreateRef<Material>(materialName, ShaderLibrary::Get(shaderName));

						if (auto intUniformData = materialUniformData["IntUniforms"])
						{
							for (YAML::const_iterator it = intUniformData.begin(); it != intUniformData.end(); ++it)
							{
								std::string key = it->first.as<std::string>();
								int value = it->second.as<int>();
								deserializedMaterial->Set<int>(key, value);
							}
						}
						if (auto floatUniformData = materialUniformData["FloatUniforms"])
						{
							for (YAML::const_iterator it = floatUniformData.begin(); it != floatUniformData.end(); ++it)
							{
								std::string key = it->first.as<std::string>();
								float value = it->second.as<float>();
								deserializedMaterial->Set<float>(key, value);
							}
						}
						if (auto vec2UniformData = materialUniformData["Vec2Uniforms"])
						{
							for (YAML::const_iterator it = vec2UniformData.begin(); it != vec2UniformData.end(); ++it)
							{
								std::string key = it->first.as<std::string>();
								glm::vec2 value = it->second.as <glm::vec2>();
								deserializedMaterial->Set<glm::vec2>(key, value);
							}
						}
						if (auto vec3UniformData = materialUniformData["Vec3Uniforms"])
						{
							for (YAML::const_iterator it = vec3UniformData.begin(); it != vec3UniformData.end(); ++it)
							{
								std::string key = it->first.as<std::string>();
								glm::vec3 value = it->second.as <glm::vec3>();
								deserializedMaterial->Set<glm::vec3>(key, value);
							}
						}
						if (auto vec4UniformData = materialUniformData["Vec4Uniforms"])
						{
							for (YAML::const_iterator it = vec4UniformData.begin(); it != vec4UniformData.end(); ++it)
							{
								std::string key = it->first.as<std::string>();
								glm::vec4 value = it->second.as <glm::vec4>();
								deserializedMaterial->Set<glm::vec4>(key, value);
							}
						}
						if (auto textureUniformData = materialUniformData["TextureUniforms"])
						{
							for (YAML::const_iterator it = textureUniformData.begin(); it != textureUniformData.end(); ++it)
							{
								std::string key = it->first.as<std::string>();
								TextureUniform value = it->second.as<TextureUniform>();
								deserializedMaterial->Set<TextureUniform>(key, value);
							}
						}

						meshRendererComponent.MaterialInstance = deserializedMaterial;
					}
				}

				if (auto lightData = entity["LightComponent"])
				{
					auto& lightComponent = deserializedEntity.AddComponent<DirectionalLightComponent>();
					lightComponent.Radiance = lightData["Radiance"].as<glm::vec4>();
					lightComponent.Intensity = lightData["Intensity"].as<float>();
					lightComponent.LightDirection = lightData["LightDirection"].as<glm::vec4>();
					lightComponent.ShadowAmount = lightData["ShadowAmount"].as<float>();
				}
			}
		}

		return true;
	}
}