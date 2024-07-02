#pragma once

namespace Ohm
{
    enum class DebugShapeType { None, BaseShape, DetailNoise };
    enum class CloudNoiseType { BaseShape, DetailShape, Perlin, Curl };
    enum class WorleyChannelMask { R, G, B, A };
    enum class UITabTypes { None = 0, MainSettings, CloudSettings, NoiseTextureSettings };
}
