#include "ohmpch.h"
#include "Ohm/Core/Random.h"
#include "Ohm/Rendering/Shader.h"

#include "HelperFunctions.h"
#include "CloudDataStructures.h"

namespace Ohm
{
	static Ref<WorleyChannelData> CreateChannelData(WorleyChannelMask mask, const glm::ivec3& defaultCells, float persistence)
	{
		Ref<WorleyChannelData> data = CreateRef<WorleyChannelData>();
		data->Mask = mask;

		data->WorleyLayerPersistence = persistence;
		data->LayerSeeds.x = Random::RandomRange(0.0f, 10e6f);
		data->LayerSeeds.y = Random::RandomRange(0.0f, 10e6f);
		data->LayerSeeds.z = Random::RandomRange(0.0f, 10e6f);
		data->LayerCells = defaultCells;
		data->MinMax.resize(2);
		data->MinMax[0] = INT_MAX;
		data->MinMax[1] = INT_MIN;

		data->PointsA = CreateWorleyPoints(data->LayerCells.x, data->LayerSeeds.x);
		data->ShapePointsBufferA = CreateRef<ShaderStorageBuffer>(data->PointsA.data(), sizeof(glm::vec4) * data->PointsA.size());
		data->PointsB = CreateWorleyPoints(data->LayerCells.y, data->LayerSeeds.y);
		data->ShapePointsBufferB = CreateRef<ShaderStorageBuffer>(data->PointsB.data(), sizeof(glm::vec4) * data->PointsB.size());
		data->PointsC = CreateWorleyPoints(data->LayerCells.z, data->LayerSeeds.z);
		data->ShapePointsBufferC = CreateRef<ShaderStorageBuffer>(data->PointsC.data(), sizeof(glm::vec4) * data->PointsC.size());
		data->MinMaxBuffer = CreateRef<ShaderStorageBuffer>(data->MinMax.data(), sizeof(int) * 2);

		return data;
	}

	BaseShapeWorleySettings::BaseShapeWorleySettings()
	{
		ChannelR = CreateChannelData(WorleyChannelMask::R, DefaultLayerCellsR, DefaultPersistence.r);
		ChannelG = CreateChannelData(WorleyChannelMask::G, DefaultLayerCellsG, DefaultPersistence.g);
		ChannelB = CreateChannelData(WorleyChannelMask::B, DefaultLayerCellsB, DefaultPersistence.b);
		ChannelA = CreateChannelData(WorleyChannelMask::A, DefaultLayerCellsA, DefaultPersistence.a);

		Texture3DSpecification baseShapeTextureSpec =
		{
			TextureUtils::WrapMode::Repeat,
			TextureUtils::WrapMode::Repeat,
			TextureUtils::WrapMode::Repeat,
			TextureUtils::FilterMode::Linear,
			TextureUtils::FilterMode::Linear,
			TextureUtils::ImageInternalFormat::RGBA32F,
			TextureUtils::ImageDataLayout::RGBA,
			TextureUtils::ImageDataType::Float,
			ShapeResolution, ShapeResolution, ShapeResolution
		};

		BaseShapeTexture = CreateRef<Texture3D>(baseShapeTextureSpec);
	}

	void BaseShapeWorleySettings::UpdateChannel(WorleyChannelMask mask, const Ref<Texture2D>& perlinTexture)
	{
		BaseShapeTexture->BindToImageSlot(0, 0, TextureUtils::TextureAccessLevel::ReadWrite, TextureUtils::TextureShaderDataFormat::RGBA32F);

		uint32_t threadGroups = glm::ceil(ShapeResolution / (float)ShapeThreadGroupSize);

		switch (mask)
		{
			case WorleyChannelMask::R: ChannelR->UpdateChannel(perlinTexture, PerlinWorleyMix, threadGroups); break;
			case WorleyChannelMask::G: ChannelG->UpdateChannel(perlinTexture, PerlinWorleyMix, threadGroups); break;
			case WorleyChannelMask::B: ChannelB->UpdateChannel(perlinTexture, PerlinWorleyMix, threadGroups); break;
			case WorleyChannelMask::A: ChannelA->UpdateChannel(perlinTexture, PerlinWorleyMix, threadGroups); break;
			default: break;
		}
	}

	void BaseShapeWorleySettings::UpdateAllChannels(const Ref<WorleyPerlinSettings>& perlinSettings)
	{
		BaseShapeTexture->BindToImageSlot(0, 0, TextureUtils::TextureAccessLevel::ReadWrite, TextureUtils::TextureShaderDataFormat::RGBA32F);

		uint32_t threadGroups = glm::ceil(ShapeResolution / (float)ShapeThreadGroupSize);

		ChannelR->UpdateChannel(perlinSettings->PerlinTexture, PerlinWorleyMix, threadGroups);
		ChannelG->UpdateChannel(perlinSettings->PerlinTexture, PerlinWorleyMix, threadGroups);
		ChannelB->UpdateChannel(perlinSettings->PerlinTexture, PerlinWorleyMix, threadGroups);
		ChannelA->UpdateChannel(perlinSettings->PerlinTexture, PerlinWorleyMix, threadGroups);
	}

	void WorleyChannelData::UpdatePoints()
	{
		LayerSeeds.x = Random::RandomRange(0.0f, 10e6f);
		LayerSeeds.y = Random::RandomRange(0.0f, 10e6f);
		LayerSeeds.z = Random::RandomRange(0.0f, 10e6f);
		PointsA = CreateWorleyPoints(LayerCells.x, LayerSeeds.x);
		PointsB = CreateWorleyPoints(LayerCells.y, LayerSeeds.y);
		PointsC = CreateWorleyPoints(LayerCells.z, LayerSeeds.z);

		ShapePointsBufferA = CreateRef<ShaderStorageBuffer>(PointsA.data(), sizeof(glm::vec4) * PointsA.size());
		ShapePointsBufferB = CreateRef<ShaderStorageBuffer>(PointsB.data(), sizeof(glm::vec4) * PointsB.size());
		ShapePointsBufferC = CreateRef<ShaderStorageBuffer>(PointsC.data(), sizeof(glm::vec4) * PointsC.size());
	}

	void WorleyChannelData::UpdateChannel(const Ref<Texture2D>& perlinTexture, float perlinWorleyMix, uint32_t threadGroups)
	{
		MinMax[0] = INT_MAX;
		MinMax[1] = INT_MIN;
		MinMaxBuffer = CreateRef<ShaderStorageBuffer>(MinMax.data(), sizeof(int) * 2);

		ShaderLibrary::Get("WorleyGenerator")->Bind();
		perlinTexture->BindToSamplerSlot(0);

		ShapePointsBufferA->BindToComputeShader(0, ShaderLibrary::Get("WorleyGenerator")->GetID());
		ShapePointsBufferB->BindToComputeShader(1, ShaderLibrary::Get("WorleyGenerator")->GetID());
		ShapePointsBufferC->BindToComputeShader(2, ShaderLibrary::Get("WorleyGenerator")->GetID());
		MinMaxBuffer->BindToComputeShader(3, ShaderLibrary::Get("WorleyGenerator")->GetID());
		ShaderLibrary::Get("WorleyGenerator")->UploadUniformBool("u_IsBaseShape", true);
		ShaderLibrary::Get("WorleyGenerator")->UploadUniformBool("u_Invert", InvertWorley);
		ShaderLibrary::Get("WorleyGenerator")->UploadUniformInt("u_CellsA", LayerCells.x);
		ShaderLibrary::Get("WorleyGenerator")->UploadUniformInt("u_CellsB", LayerCells.y);
		ShaderLibrary::Get("WorleyGenerator")->UploadUniformInt("u_CellsC", LayerCells.z);
		ShaderLibrary::Get("WorleyGenerator")->UploadUniformFloat("u_Tiling", WorleyTiling);
		ShaderLibrary::Get("WorleyGenerator")->UploadUniformFloat("u_InversionWeight", InversionWeight);
		ShaderLibrary::Get("WorleyGenerator")->UploadUniformFloat("u_PerlinWorleyMix", perlinWorleyMix);
		ShaderLibrary::Get("WorleyGenerator")->UploadUniformFloat("u_Persistence", WorleyLayerPersistence);
		ShaderLibrary::Get("WorleyGenerator")->UploadUniformFloat4("u_ChannelMask", ColorFromMask(Mask));
		ShaderLibrary::Get("WorleyGenerator")->UploadUniformInt("u_PerlinTexture", 0);
		ShaderLibrary::Get("WorleyGenerator")->DispatchCompute(threadGroups, threadGroups, threadGroups);
		ShaderLibrary::Get("WorleyGenerator")->EnableShaderImageAccessBarrierBit();

		ShaderLibrary::Get("NormalizeWorley")->Bind();
		MinMaxBuffer->BindToComputeShader(0, ShaderLibrary::Get("NormalizeWorley")->GetID());
		ShaderLibrary::Get("NormalizeWorley")->UploadUniformFloat4("u_ChannelMask", ColorFromMask(Mask));
		ShaderLibrary::Get("NormalizeWorley")->DispatchCompute(threadGroups, threadGroups, threadGroups);
		ShaderLibrary::Get("NormalizeWorley")->EnableShaderImageAccessBarrierBit();
	}

	void WorleyChannelData::UpdateChannel(uint32_t threadGroups)
	{
		MinMax[0] = INT_MAX;
		MinMax[1] = INT_MIN;
		MinMaxBuffer = CreateRef<ShaderStorageBuffer>(MinMax.data(), sizeof(int) * 2);

		ShaderLibrary::Get("WorleyGenerator")->Bind();

		ShapePointsBufferA->BindToComputeShader(0, ShaderLibrary::Get("WorleyGenerator")->GetID());
		ShapePointsBufferB->BindToComputeShader(1, ShaderLibrary::Get("WorleyGenerator")->GetID());
		ShapePointsBufferC->BindToComputeShader(2, ShaderLibrary::Get("WorleyGenerator")->GetID());
		MinMaxBuffer->BindToComputeShader(3, ShaderLibrary::Get("WorleyGenerator")->GetID());
		ShaderLibrary::Get("WorleyGenerator")->UploadUniformBool("u_IsBaseShape", false);
		ShaderLibrary::Get("WorleyGenerator")->UploadUniformBool("u_Invert", InvertWorley);
		ShaderLibrary::Get("WorleyGenerator")->UploadUniformInt("u_CellsA", LayerCells.x);
		ShaderLibrary::Get("WorleyGenerator")->UploadUniformInt("u_CellsB", LayerCells.y);
		ShaderLibrary::Get("WorleyGenerator")->UploadUniformInt("u_CellsC", LayerCells.z);
		ShaderLibrary::Get("WorleyGenerator")->UploadUniformFloat("u_Tiling", WorleyTiling);
		ShaderLibrary::Get("WorleyGenerator")->UploadUniformFloat("u_InversionWeight", InversionWeight);
		ShaderLibrary::Get("WorleyGenerator")->UploadUniformFloat("u_Persistence", WorleyLayerPersistence);
		ShaderLibrary::Get("WorleyGenerator")->UploadUniformFloat4("u_ChannelMask", ColorFromMask(Mask));
		ShaderLibrary::Get("WorleyGenerator")->UploadUniformInt("u_PerlinTexture", 0);
		ShaderLibrary::Get("WorleyGenerator")->DispatchCompute(threadGroups, threadGroups, threadGroups);
		ShaderLibrary::Get("WorleyGenerator")->EnableShaderImageAccessBarrierBit();

		ShaderLibrary::Get("NormalizeWorley")->Bind();
		MinMaxBuffer->BindToComputeShader(0, ShaderLibrary::Get("NormalizeWorley")->GetID());
		ShaderLibrary::Get("NormalizeWorley")->UploadUniformFloat4("u_ChannelMask", ColorFromMask(Mask));
		ShaderLibrary::Get("NormalizeWorley")->DispatchCompute(threadGroups, threadGroups, threadGroups);
		ShaderLibrary::Get("NormalizeWorley")->EnableShaderImageAccessBarrierBit();
	}

	const Ref<WorleyChannelData>& DetailChannelFromMask(const Ref<DetailShapeWorleySettings>& detailShapeSettings, WorleyChannelMask channelMask)
	{
		switch (channelMask)
		{
			case WorleyChannelMask::R: return detailShapeSettings->ChannelR;
			case WorleyChannelMask::G: return detailShapeSettings->ChannelG;
			case WorleyChannelMask::B: return detailShapeSettings->ChannelB;
		}

		return nullptr;
	}

	const Ref<WorleyChannelData>& ShapeChannelFromMask(const Ref<BaseShapeWorleySettings>& baseShapeSettings, WorleyChannelMask channelMask)
	{
		switch (channelMask)
		{
			case WorleyChannelMask::R: return baseShapeSettings->ChannelR;
			case WorleyChannelMask::G: return baseShapeSettings->ChannelG;
			case WorleyChannelMask::B: return baseShapeSettings->ChannelB;
			case WorleyChannelMask::A: return baseShapeSettings->ChannelA;
		}
		return nullptr;
	}

	WorleyPerlinSettings::WorleyPerlinSettings()
	{
		RandomPerlinOffsets = GeneratePerlinOffsets(Octaves);
		RandomPerlinOffsetsBuffer = CreateRef<ShaderStorageBuffer>(RandomPerlinOffsets.data(), sizeof(glm::vec4) * RandomPerlinOffsets.size());

		Texture2DSpecification perlinSpec =
		{
			TextureUtils::WrapMode::Repeat,
			TextureUtils::WrapMode::Repeat,
			TextureUtils::FilterMode::Linear,
			TextureUtils::FilterMode::Linear,
			TextureUtils::ImageInternalFormat::RGBA32F,
			TextureUtils::ImageDataLayout::RGBA,
			TextureUtils::ImageDataType::Float,
			PerlinResolution, PerlinResolution
		};

		PerlinTexture = CreateRef<Texture2D>(perlinSpec);
		PerlinTexture->BindToImageSlot(1, 0, TextureUtils::TextureAccessLevel::ReadWrite, TextureUtils::TextureShaderDataFormat::RGBA32F);
		UpdateTexture();
	}

	void WorleyPerlinSettings::UpdatePoints()
	{
		RandomPerlinOffsets = GeneratePerlinOffsets(Octaves);
		RandomPerlinOffsetsBuffer->SetData(RandomPerlinOffsets.data(), 0, RandomPerlinOffsets.size() * sizeof(glm::vec4));
	}

	void WorleyPerlinSettings::UpdateTexture()
	{
		uint32_t threadGroups = glm::ceil(PerlinResolution / (float)PerlinThreadGroupSize);
		RandomPerlinOffsetsBuffer->BindToComputeShader(3, ShaderLibrary::Get("Perlin2D")->GetID());
		ShaderLibrary::Get("Perlin2D")->Bind();
		ShaderLibrary::Get("Perlin2D")->UploadUniformFloat("u_Settings.NoiseScale", NoiseScale);
		ShaderLibrary::Get("Perlin2D")->UploadUniformFloat("u_Settings.Lacunarity", Lacunarity);
		ShaderLibrary::Get("Perlin2D")->UploadUniformFloat("u_Settings.Persistence", Persistence);
		ShaderLibrary::Get("Perlin2D")->UploadUniformInt("u_Settings.Octaves", Octaves);
		ShaderLibrary::Get("Perlin2D")->UploadUniformFloat2("u_Settings.TextureOffset", TextureOffset);
		ShaderLibrary::Get("Perlin2D")->DispatchCompute(threadGroups, threadGroups, 1);
		ShaderLibrary::Get("Perlin2D")->EnableShaderImageAccessBarrierBit();
	}

	DetailShapeWorleySettings::DetailShapeWorleySettings()
	{
		ChannelR = CreateChannelData(WorleyChannelMask::R, DefaultLayerCellsR, DefaultPersistence.r);
		ChannelG = CreateChannelData(WorleyChannelMask::G, DefaultLayerCellsG, DefaultPersistence.g);
		ChannelB = CreateChannelData(WorleyChannelMask::B, DefaultLayerCellsB, DefaultPersistence.b);

		ChannelR->InvertWorley = false;
		ChannelR->InvertWorley = false;
		ChannelR->InvertWorley = false;

		Texture3DSpecification detailShapeTextureSpec =
		{
			TextureUtils::WrapMode::Repeat,
			TextureUtils::WrapMode::Repeat,
			TextureUtils::WrapMode::Repeat,
			TextureUtils::FilterMode::Linear,
			TextureUtils::FilterMode::Linear,
			TextureUtils::ImageInternalFormat::RGBA32F,
			TextureUtils::ImageDataLayout::RGBA,
			TextureUtils::ImageDataType::Float,
			DetailResolution, DetailResolution, DetailResolution
		};

		DetailShapeTexture = CreateRef<Texture3D>(detailShapeTextureSpec);
	}

	void DetailShapeWorleySettings::UpdateChannel(WorleyChannelMask mask)
	{
		DetailShapeTexture->BindToImageSlot(0, 0, TextureUtils::TextureAccessLevel::ReadWrite, TextureUtils::TextureShaderDataFormat::RGBA32F);

		uint32_t threadGroups = glm::ceil(DetailResolution / (float)DetailThreadGroupSize);

		switch (mask)
		{
		case WorleyChannelMask::R: ChannelR->UpdateChannel(threadGroups); break;
		case WorleyChannelMask::G: ChannelG->UpdateChannel(threadGroups); break;
		case WorleyChannelMask::B: ChannelB->UpdateChannel(threadGroups); break;
		default: break;
		}
	}

	void DetailShapeWorleySettings::UpdateAllChannels()
	{
		DetailShapeTexture->BindToImageSlot(0, 0, TextureUtils::TextureAccessLevel::ReadWrite, TextureUtils::TextureShaderDataFormat::RGBA32F);

		uint32_t threadGroups = glm::ceil(DetailResolution / (float)DetailThreadGroupSize);

		ChannelR->UpdateChannel(threadGroups);
		ChannelG->UpdateChannel(threadGroups);
		ChannelB->UpdateChannel(threadGroups);
	}

	CurlNoiseSettings::CurlNoiseSettings()
	{
		Texture2DSpecification curlSpec =
		{
			TextureUtils::WrapMode::Repeat,
			TextureUtils::WrapMode::Repeat,
			TextureUtils::FilterMode::Linear,
			TextureUtils::FilterMode::Linear,
			TextureUtils::ImageInternalFormat::RGBA32F,
			TextureUtils::ImageDataLayout::RGBA,
			TextureUtils::ImageDataType::Float,
			CurlResolution, CurlResolution
		};

		CurlTexture = CreateRef<Texture2D>(curlSpec);

		UpdateTexture();
	}

	void CurlNoiseSettings::UpdateTexture()
	{
		CurlTexture->BindToImageSlot(0, 0, TextureUtils::TextureAccessLevel::ReadWrite, TextureUtils::TextureShaderDataFormat::RGBA32F);
		uint32_t threadGroups = glm::ceil(CurlResolution / (float)CurlThreadGroupSize);
		ShaderLibrary::Get("Curl")->Bind();
		ShaderLibrary::Get("Curl")->UploadUniformFloat("u_Strength", Strength);
		ShaderLibrary::Get("Curl")->UploadUniformFloat("u_Tiling", Tiling);
		ShaderLibrary::Get("Curl")->UploadUniformFloat2("u_TilingOffset", TilingOffset);
		ShaderLibrary::Get("Curl")->DispatchCompute(threadGroups, threadGroups, 1);
		ShaderLibrary::Get("Curl")->EnableShaderImageAccessBarrierBit();
	}
}
