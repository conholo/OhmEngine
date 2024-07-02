#pragma once

#include "CloudEnums.h"

namespace Ohm
{
    std::vector<glm::vec4> CreateWorleyPoints(uint32_t cells, int seed);
    std::vector<glm::vec4> GeneratePerlinOffsets(int octaves);

    std::string NameFromUITabType(UITabTypes tabType);
    std::string NameFromUIType(CloudNoiseType type);

    std::string NameFromWorleyChannel(WorleyChannelMask type);
    glm::vec4 ColorFromMask(WorleyChannelMask mask);
}
