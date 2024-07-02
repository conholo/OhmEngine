#pragma once

#include "Ohm/Rendering/Clouds/CloudDataStructures.h"
#include "Ohm/Scene/Component.h"

namespace Ohm
{
	struct UITab
	{
		int Index = -1;
		std::string TabName;
	};

	class CloudsUI
	{
	public:
		void Draw(const VolumetricCloudComponent* cloudComponentPtr);
	
	private:
		void DrawMainSettings(CloudSettings& cloudSettings);
		void DrawNoiseEditorSettings(VolumetricCloudComponent::Debug& debugSettings);
		void DrawCloudSettingsUI(CloudSettings& cloudSettings, CloudAnimationSettings& animSettings) const;
		void DrawBaseShapeUI(BaseShapeWorleySettings& baseShapeSettings, WorleyChannelMask baseShapeMask, const WorleyPerlinSettings& perlinSettings) const;
		void DrawDetailShapeUI(DetailShapeWorleySettings& detailShapeSettings, WorleyChannelMask detailShapeMask) const;
		void DrawPerlinUI(WorleyPerlinSettings& perlinSettings) const;
		void DrawCurlUI(CurlNoiseSettings& curlSettings) const;

		void DrawBaseShapeSelectionUI(ShapeTextureDebugDisplaySettings& shapeDebugSettings, WorleyChannelMask& inOutMask);
		void DrawDetailShapeSelectionUI(DetailTextureDebugDisplaySettings& detailDebugSettings, WorleyChannelMask& inOutMask);

	private:
		const Ref<WorleyChannelData>& ShapeChannelFromMask(BaseShapeWorleySettings& baseShapeSettings, WorleyChannelMask shapeMask) const;
		const Ref<WorleyChannelData>& DetailChannelFromMask(DetailShapeWorleySettings& detailShapeSettings, WorleyChannelMask shapeMask) const;

	private:
		std::vector<UITabTypes> m_AvailableTabTypes
		{
			UITabTypes::MainSettings,
			UITabTypes::CloudSettings,
			UITabTypes::NoiseTextureSettings
		};
	};
}

