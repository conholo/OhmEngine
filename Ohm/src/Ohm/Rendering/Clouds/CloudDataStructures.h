#pragma once

#include "Ohm/Core/Memory.h"
#include "Ohm/Rendering/ShaderStorageBuffer.h"
#include "Ohm/Rendering/Texture2D.h"
#include "Ohm/Rendering/Texture3D.h"
#include "CloudEnums.h"

#include <vector>
#include <glm/glm.hpp>

namespace Ohm
{
	struct CloudAnimationSettings
	{
		bool AnimateClouds = true;
		float AnimationSpeed = 5.5f;
		float TimeScale = 0.011f;
		float CloudScrollOffsetSpeed = 0.00001f;
		glm::vec3 ShapeTextureOffset{ 0.0f, 0.0f, 0.0f };
	};

	struct CloudSettings
	{
		bool DrawClouds = true;
		// Sky / Sun Parameters
		glm::vec3 SkyColorA = { 45.0f / 255.0f, 74.0 / 255.0f, 103.0 / 255.0f};
		glm::vec3 SkyColorB = { 19.0f / 255.0f, 144.0f / 255.0f, 244.0f / 255.0f };
		float SunSize = 1.0f;				// Relative size of the sun.

		// March Parameters
		int DensitySteps = 60;
		int LightSteps = 40;
		float RandomOffsetStrength = 1.0f;

		// Phase Parameters
		float PhaseBlend = 0.5f;			// The amount of blend between forward/back scattering. (0.5 is a good mix)
		float ForwardScattering = 0.25f;	// Amount of light that is scattered towards the view direction when looking towards the sun through the clouds (0.2 is good).
		float BackScattering = 0.6f;		// Amount of light that is scattered away from the view direction when looking towards the sun through the clouds (0.5 is good)
		float BaseBrightness = 0.6f;		// Additive factor associated with the HG Phase Function (1.0 is a good value)
		float PhaseFactor = 0.5f;			// Multiplicative factor associated with the HG Phase Function (1.0 is a good value)

		// Lighting Parameters
		float PowderConstant = 10.0f;		// In Scattering probability factor.  Creases where bulges occur are more likely to produce in-scattered light.
		float SilverLiningConstant = .9f;	// In Scattering probability factor.  When looking towards the sun, light is more likely to scatter towards the viewer around the cloud edges.
		float ExtinctionFactor = .01f;

		// Density Parameters
		float DensityThreshold = .403f;
		float DensityMultiplier = .344f;
		glm::vec4 ShapeNoiseWeights{ .72f, 1.0f, .28f, .36f };
		glm::vec3 BaseShapeTextureOffset = glm::vec3(0.0);

		glm::vec3 DetailNoiseWeights{ .35f, 0.66f, 0.744f };
		float DetailNoiseWeight = .2f;

		glm::vec3 CloudTypeWeights{ 0.5f, 0.38f, 0.63f };
		float CloudTypeWeightStrength = 7.0f;

		float CurlIntensity = 1.0f;

		// Container / Scale Parameters
		glm::vec3 CloudContainerPosition{ 0.0f, 150.0f, 0.0f };
		glm::vec3 CloudContainerScale{ 400.0f, 300.0f, 300.0f };
		float CloudScaleFactor = 1000.0f;
		float CloudScale = 4.0f;
		float ContainerEdgeFadeDistance = 50.0f;
	};

	struct CurlNoiseSettings
	{
		CurlNoiseSettings();

		const uint32_t CurlThreadGroupSize = 8;
		uint32_t CurlResolution = 128;

		float Strength = 0.5f;
		float Tiling = 3.5f;
		glm::vec2 TilingOffset{ 0.0f, 0.0f };

		Ref<Texture2D> CurlTexture;

		void UpdateTexture();
	};

	struct WorleyPerlinSettings
	{
		WorleyPerlinSettings();

		const uint32_t PerlinThreadGroupSize = 8;
		uint32_t PerlinResolution = 512;

		int Octaves = 5;
		float NoiseScale = 2.1f;
		float Lacunarity = 2.0f;
		float Persistence = .65f;
		glm::vec2 TextureOffset{ 0.0f, 0.0f };

		std::vector<glm::vec4> RandomPerlinOffsets;
		Ref<ShaderStorageBuffer> RandomPerlinOffsetsBuffer;
		Ref<Texture2D> PerlinTexture;

		void UpdatePoints();
		void UpdateTexture();
	};

	struct WorleyChannelData
	{
		WorleyChannelMask Mask;
		bool InvertWorley = true;
		float WorleyTiling = 1.0f;
		float WorleyLayerPersistence = 0.1f;
		float InversionWeight = 1.0f;
		glm::ivec3 LayerCells;
		glm::ivec3 LayerSeeds;

		std::vector<glm::vec4> PointsA;
		std::vector<glm::vec4> PointsB;
		std::vector<glm::vec4> PointsC;
		std::vector<int> MinMax;
		Ref<ShaderStorageBuffer> ShapePointsBufferA;
		Ref<ShaderStorageBuffer> ShapePointsBufferB;
		Ref<ShaderStorageBuffer> ShapePointsBufferC;
		Ref<ShaderStorageBuffer> MinMaxBuffer;

		void UpdatePoints();
		void UpdateChannel(const Ref<Texture2D>& perlinTexture, float perlinWorleyMix, uint32_t threadGroups);
		void UpdateChannel(uint32_t threadGroups);
	};
	
	struct TextureDebugDisplaySettings
	{
		bool EnableTextureViewer = true;
		float PercentScreenTextureDisplay = 0.1f;
		std::vector<CloudNoiseType> EditorTypes = { CloudNoiseType::BaseShape, CloudNoiseType::DetailShape, CloudNoiseType::Perlin, CloudNoiseType::Curl };
	};

	struct ShapeTextureDebugDisplaySettings
	{
		float DepthSlice;
		bool DisplaySelectedChannelOnly = false;
		bool ShowAlpha = false;
		bool DrawAllChannels = true;
		bool EnableGreyScale = false;
		glm::vec4 ChannelWeights{ 0.0f, 0.0f,0.0f, 0.0f };
		std::vector<WorleyChannelMask> ChannelTypes = { WorleyChannelMask::R, WorleyChannelMask::G, WorleyChannelMask::B, WorleyChannelMask::A };
	};

	struct DetailTextureDebugDisplaySettings
	{
		float DepthSlice;
		bool DisplaySelectedChannelOnly = false;
		bool DrawAllChannels = true;
		bool EnableGreyScale = false;
		glm::vec3 ChannelWeights{ 0.0f, 0.0f,0.0f };
		std::vector<WorleyChannelMask> ChannelTypes = { WorleyChannelMask::R, WorleyChannelMask::G, WorleyChannelMask::B };
	};

	struct BaseShapeWorleySettings
	{
		BaseShapeWorleySettings();

		const uint32_t ShapeThreadGroupSize = 8;
		uint32_t ShapeResolution = 128;

		glm::ivec3 DefaultLayerCellsR{ 2, 6, 10 };
		glm::ivec3 DefaultLayerCellsG{ 5, 8, 12 };
		glm::ivec3 DefaultLayerCellsB{ 8, 16, 25 };
		glm::ivec3 DefaultLayerCellsA{ 14, 22, 33 };
		glm::vec4 DefaultPersistence{ 0.1f, 0.3f, 0.5f, 0.8f };

		float PerlinWorleyMix = 0.1f;
		Ref<Texture3D> BaseShapeTexture;
		Ref<WorleyChannelData> ChannelR;
		Ref<WorleyChannelData> ChannelG;
		Ref<WorleyChannelData> ChannelB;
		Ref<WorleyChannelData> ChannelA;

		void UpdateChannel(WorleyChannelMask mask, const Ref<Texture2D>& perlinTexture);
		void UpdateAllChannels(const Ref<WorleyPerlinSettings>& perlinSettings);
	};

	struct DetailShapeWorleySettings
	{
		DetailShapeWorleySettings();

		const uint32_t DetailThreadGroupSize = 8;
		uint32_t DetailResolution = 32;

		glm::ivec3 DefaultLayerCellsR{ 2, 6, 10 };
		glm::ivec3 DefaultLayerCellsG{ 5, 8, 12 };
		glm::ivec3 DefaultLayerCellsB{ 8, 16, 25 };
		glm::vec3 DefaultPersistence{ 0.1f, 0.3f, 0.5f };

		Ref<Texture3D> DetailShapeTexture;
		Ref<WorleyChannelData> ChannelR;
		Ref<WorleyChannelData> ChannelG;
		Ref<WorleyChannelData> ChannelB;

		void UpdateChannel(WorleyChannelMask mask);
		void UpdateAllChannels();
	};

	const Ref<WorleyChannelData>& ShapeChannelFromMask(const Ref<BaseShapeWorleySettings>& baseShapeSettings, WorleyChannelMask channelMask);
	const Ref<WorleyChannelData>& DetailChannelFromMask(const Ref<DetailShapeWorleySettings>& detailShapeSettings, WorleyChannelMask channelMask);
}

