#include "ohmpch.h"
#include <string>
#include <vector>
#include "Ohm/Core/Random.h"
#include <glm/glm.hpp>

#include "HelperFunctions.h"

namespace Ohm
{
	std::vector<glm::vec4> CreateWorleyPoints(uint32_t cells, int seed)
	{
		std::vector<glm::vec4> points;
		points.resize(cells * cells * cells);

		Random::Seed(seed);
		float cellSize = 1.0f / cells;

		for (uint32_t x = 0; x < cells; x++)
		{
			for (uint32_t y = 0; y < cells; y++)
			{
				for (uint32_t z = 0; z < cells; z++)
				{
					float randomX = Random::RandomRange(0.0f, 1.0f);
					float randomY = Random::RandomRange(0.0f, 1.0f);
					float randomZ = Random::RandomRange(0.0f, 1.0f);

					glm::vec3 cellCorner = glm::vec3(x, y, z) * cellSize;
					glm::vec3 offset = glm::vec3(randomX, randomY, randomZ) * cellSize;

					uint32_t index = x + cells * (y + z * cells);
					points[index] = glm::vec4(cellCorner + offset, 0.0f);
				}
			}
		}

		return points;
	}

	std::vector<glm::vec4> GeneratePerlinOffsets(int octaves)
	{
		std::vector<glm::vec4> result;
		result.resize(octaves);

		for (int i = 0; i < octaves; i++)
		{
			glm::vec4 random =
			{
				Random::RandomRange(0.0f, 1.0f),
				Random::RandomRange(0.0f, 1.0f),
				Random::RandomRange(0.0f, 1.0f),
				Random::RandomRange(0.0f, 1.0f)
			};
			result[i] = (random * glm::vec4(2.0) - glm::vec4(1.0)) * glm::vec4(1000.0);
		}

		return result;
	}

	std::string NameFromUITabType(UITabTypes tabType)
	{
		switch (tabType)
		{
		case UITabTypes::MainSettings:			return "Main Settings";
		case UITabTypes::CloudSettings:			return "Cloud Settings";
		case UITabTypes::NoiseTextureSettings:	return "Noise Texture Settings";
		}

		return "";
	}

	std::string NameFromUIType(CloudNoiseType type)
	{
		switch (type)
		{
		case CloudNoiseType::BaseShape:		return "Base Shape";
		case CloudNoiseType::DetailShape:		return "Detail Noise";
		case CloudNoiseType::Perlin:			return "Perlin";
		case CloudNoiseType::Curl:				return "Curl";
		}

		return "";
	}

	std::string NameFromWorleyChannel(WorleyChannelMask type)
	{
		switch (type)
		{
			case WorleyChannelMask::R:	return "R";
			case WorleyChannelMask::G:	return "G";
			case WorleyChannelMask::B:	return "B";
			case WorleyChannelMask::A:	return "A";
		}

		return "";
	}

	glm::vec4 ColorFromMask(WorleyChannelMask mask)
	{
		switch (mask)
		{
			case WorleyChannelMask::R: return { 1.0f, 0.0f, 0.0f, 0.0f };
			case WorleyChannelMask::G: return { 0.0f, 1.0f, 0.0f, 0.0f };
			case WorleyChannelMask::B: return { 0.0f, 0.0f, 1.0f, 0.0f };
			case WorleyChannelMask::A: return { 0.0f, 0.0f, 0.0f, 1.0f };
		}

		return { 0.0f, 0.0f, 0.0f, 0.0f };
	}
}
