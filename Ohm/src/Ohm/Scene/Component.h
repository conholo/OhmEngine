#pragma once


#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include "Ohm/Core/Memory.h"
#include "Ohm/Core/UUID.h"
#include "Ohm/Rendering/EnvironmentMapPipeline.h"
#include "Ohm/Rendering/Material.h"
#include "Ohm/Rendering/Mesh.h"
#include "Ohm/Rendering/TextureLibrary.h"
#include "Ohm/Rendering/Clouds/CloudDataStructures.h"

namespace Ohm
{
	struct IDComponent
	{
		UUID ID;

		IDComponent() = default;
		IDComponent(const IDComponent&) = default;
	};

	struct TagComponent
	{
		std::string Tag;

		TagComponent() = default;
		TagComponent(const TagComponent&) = default;
		TagComponent(std::string tag)
			: Tag(std::move(tag)){ }
	};


	struct TransformComponent
	{
		TransformComponent() = default;
		TransformComponent(const glm::vec3& Position, const glm::vec3& Degrees, const glm::vec3& Size)
			:Translation(Position), RotationDegrees(Degrees), Scale(Size) { }


		glm::mat4 Transform() const
		{
			return 
				translate(glm::mat4(1.0), Translation) * 
				toMat4(glm::quat(radians(RotationDegrees))) * 
				scale(glm::mat4(1.0), Scale);
		}

		glm::quat Orientation() const
		{
			return glm::quat(radians(RotationDegrees));
		}

		glm::vec3 Up() const
		{
			return rotate(Orientation(), glm::vec3(0.0f, 1.0f, 0.0f));
		}

		glm::vec3 Right() const
		{
			return rotate(Orientation(), glm::vec3(1.0f, 0.0f, 0.0f));
		}

		glm::vec3 Forward() const
		{
			return rotate(Orientation(), glm::vec3(0.0f, 0.0f, -1.0f));
		}

		glm::vec3 Translation = glm::vec3(0.0f);
		glm::vec3 RotationDegrees = glm::vec3(0.0f);
		glm::vec3 Scale = glm::vec3(1.0f);
	};

	struct MeshRendererComponent
	{
		Ref<Material> MaterialInstance;
		Ref<Mesh> MeshData;

		bool IsComplete() const
		{
			return MaterialInstance != nullptr && MeshData != nullptr;
		}

		MeshRendererComponent() = default;
		MeshRendererComponent(const Ref<Material>& material, const Ref<Mesh>& mesh)
			:MaterialInstance(material), MeshData(mesh) { }
	};

	struct PrimitiveRendererComponent
	{
		Primitive PrimitiveType;
		Ref<Material> MaterialInstance;

		PrimitiveRendererComponent() = default;
		PrimitiveRendererComponent(const Primitive Primitive)
			:PrimitiveType(Primitive){ }
		PrimitiveRendererComponent(const Primitive Primitive, const Ref<Material>& Material)
			:PrimitiveType(Primitive), MaterialInstance(Material){ }
		PrimitiveRendererComponent(const Primitive Primitive, const std::string& ShaderName)
			:PrimitiveType(Primitive), MaterialInstance(CreateRef<Material>(ShaderName + " Material", ShaderLibrary::Get(ShaderName)))
		{
			if(ShaderName == "PBR")
			{
				uint32_t whiteTextureId = TextureLibrary::Get2D("White Texture")->GetID();
				MaterialInstance->Set<TextureUniform>("sampler_AlbedoTexture", {whiteTextureId, 0, 0 });
				MaterialInstance->Set<TextureUniform>("sampler_NormalTexture", {whiteTextureId, 1, 0 });
				MaterialInstance->Set<TextureUniform>("sampler_MetalnessTexture", {whiteTextureId, 2, 0 });
				MaterialInstance->Set<TextureUniform>("sampler_RoughnessTexture", {whiteTextureId, 3, 0 });
			}
		}
	};

	struct CameraComponent
	{
		glm::mat4 View;
		glm::mat4 Projection;

		CameraComponent() = default;
		CameraComponent(const CameraComponent&) = default;
		CameraComponent(const glm::mat4& view, const glm::mat4& projection)
			:View(view), Projection(projection) { }
	};

	struct DirectionalLightComponent
	{
		glm::vec3 Radiance { 1.0f};
		float Intensity = 1.0f;
		glm::vec3 LightDirection {0.0f, -1.0f, 0.0f};
		float ShadowAmount = 1.0f;

		DirectionalLightComponent() = default;
		DirectionalLightComponent(const DirectionalLightComponent&) = default;
		DirectionalLightComponent(const glm::vec3& Radiance, float Intensity)
			:Radiance(Radiance), Intensity(Intensity)
		{
		}
	};

	struct EnvironmentMapParams
	{
		float Turbidity {3.0f};
		float AzimuthRads;
		float InclinationRads;
	};

	struct EnvironmentLightComponent
	{
		float Intensity = 1.0f;
		glm::vec3 EnvironmentMapSampleLODs { 0.0f };
		glm::vec3 EnvironmentMapSampleIntensities { 1.0f };

		Ref<EnvironmentMapPipeline> Pipeline{};
		EnvironmentMapParams EnvironmentMapParams{};

		bool NeedsUpdate = true;
		EnvironmentLightComponent()
			:Pipeline(CreateRef<EnvironmentMapPipeline>())
		{ }
	};

	struct VolumetricCloudComponent
	{
		Ref<CloudSettings> MainSettings;
		Ref<CloudAnimationSettings> AnimationSettings;
		Ref<BaseShapeWorleySettings> BaseShapeSettings;
		Ref<DetailShapeWorleySettings> DetailShapeSettings;
		Ref<WorleyPerlinSettings> PerlinSettings;
		Ref<CurlNoiseSettings> CurlSettings;

		struct Debug
		{
			Ref<TextureDebugDisplaySettings> MainTextureDebugSettings;
			Ref<DetailTextureDebugDisplaySettings> DetailTextureDisplaySettings;
			Ref<ShapeTextureDebugDisplaySettings> ShapeTextureDisplaySettings;
			
			UITabTypes ActiveTabType = UITabTypes::MainSettings;
			DebugShapeType ActiveDebugShapeType = DebugShapeType::BaseShape;
			WorleyChannelMask ActiveShapeMask = WorleyChannelMask::R;
			WorleyChannelMask ActiveDetailMask = WorleyChannelMask::R;
			CloudNoiseType ActiveNoiseType = CloudNoiseType::BaseShape;

			bool GetDisplayAlpha() const { return ActiveDebugShapeType == DebugShapeType::None || ActiveDebugShapeType == DebugShapeType::DetailNoise ? false : ShapeTextureDisplaySettings->ShowAlpha; }
			bool GetShowAllChannels() const { return ActiveDebugShapeType == DebugShapeType::BaseShape ? ShapeTextureDisplaySettings->DrawAllChannels : DetailTextureDisplaySettings->DrawAllChannels; }
			bool GetEnableGreyScale() const { return ActiveDebugShapeType == DebugShapeType::BaseShape ? ShapeTextureDisplaySettings->EnableGreyScale : DetailTextureDisplaySettings->EnableGreyScale; }
			float GetDepthSlice() const { return ActiveDebugShapeType == DebugShapeType::BaseShape ? ShapeTextureDisplaySettings->DepthSlice : DetailTextureDisplaySettings->DepthSlice; }
			glm::vec4 GetChannelWeights() const { return ActiveDebugShapeType == DebugShapeType::BaseShape ? ShapeTextureDisplaySettings->ChannelWeights : glm::vec4(DetailTextureDisplaySettings->ChannelWeights, 1.0f); }

			const Ref<TextureDebugDisplaySettings>& GetTextureDisplaySettings() const { return MainTextureDebugSettings; }

			Debug()
			{
				MainTextureDebugSettings = CreateRef<TextureDebugDisplaySettings>();
				DetailTextureDisplaySettings = CreateRef<DetailTextureDebugDisplaySettings>();
				ShapeTextureDisplaySettings = CreateRef<ShapeTextureDebugDisplaySettings>();
			}
		};
		Ref<Debug> DebugSettings;

		VolumetricCloudComponent()
		{
			MainSettings = CreateRef<CloudSettings>();
			AnimationSettings = CreateRef<CloudAnimationSettings>();

			PerlinSettings = CreateRef<WorleyPerlinSettings>();
			CurlSettings = CreateRef<CurlNoiseSettings>();
			BaseShapeSettings = CreateRef<BaseShapeWorleySettings>();
			BaseShapeSettings->UpdateAllChannels(PerlinSettings);
			DetailShapeSettings = CreateRef<DetailShapeWorleySettings>();
			DetailShapeSettings->UpdateAllChannels();
			DebugSettings = CreateRef<Debug>();
		}
	};
}
