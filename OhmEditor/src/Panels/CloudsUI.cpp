#include "CloudsUI.h"
#include <string>
#include "Ohm/Rendering/Clouds/HelperFunctions.h"
#include <imgui/imgui.h>

namespace Ohm
{
	const Ref<WorleyChannelData>& CloudsUI::ShapeChannelFromMask(BaseShapeWorleySettings& baseShapeSettings, WorleyChannelMask shapeMask) const
	{
		switch (shapeMask)
		{
		case WorleyChannelMask::R: return baseShapeSettings.ChannelR;
		case WorleyChannelMask::G: return baseShapeSettings.ChannelG;
		case WorleyChannelMask::B: return baseShapeSettings.ChannelB;
		case WorleyChannelMask::A: return baseShapeSettings.ChannelA;
		}

		return nullptr;
	}

	const Ref<WorleyChannelData>& CloudsUI::DetailChannelFromMask(DetailShapeWorleySettings& detailShapeSettings, WorleyChannelMask detailMask) const
	{
		switch (detailMask)
		{
		case WorleyChannelMask::R: return detailShapeSettings.ChannelR;
		case WorleyChannelMask::G: return detailShapeSettings.ChannelG;
		case WorleyChannelMask::B: return detailShapeSettings.ChannelB;
		}

		return nullptr;
	}

	void CloudsUI::DrawBaseShapeSelectionUI(ShapeTextureDebugDisplaySettings& shapeDebugSettings, WorleyChannelMask& inOutMask)
	{
		std::string currentChannel = NameFromWorleyChannel(inOutMask);
		if (ImGui::BeginCombo("Channel Selection", currentChannel.c_str()))
		{
			for (auto channelType : shapeDebugSettings.ChannelTypes)
			{
				bool isSelect = channelType == inOutMask;

				if (ImGui::Selectable(NameFromWorleyChannel(channelType).c_str(), isSelect))
				{
					inOutMask = channelType;

					if (shapeDebugSettings.DisplaySelectedChannelOnly)
					{
						shapeDebugSettings.EnableGreyScale = false;
						shapeDebugSettings.ShowAlpha = false;
						shapeDebugSettings.DrawAllChannels = false;

						glm::vec4 selectedChannel = ColorFromMask(inOutMask);

						if (selectedChannel.a == 1.0)
						{
							shapeDebugSettings.DisplaySelectedChannelOnly = false;
							shapeDebugSettings.ChannelWeights = selectedChannel;
							shapeDebugSettings.ShowAlpha = true;
						}
						else
						{
							shapeDebugSettings.ChannelWeights = selectedChannel;
						}
					}
				}
			}
			ImGui::EndCombo();
		}

		if (ImGui::Checkbox("Show All Channels", &shapeDebugSettings.DrawAllChannels))
		{
			shapeDebugSettings.EnableGreyScale = false;
			shapeDebugSettings.ShowAlpha = false;
			shapeDebugSettings.DisplaySelectedChannelOnly = false;
		}

		if (ImGui::Checkbox("Show Selected Channel Only", &shapeDebugSettings.DisplaySelectedChannelOnly))
		{
			shapeDebugSettings.EnableGreyScale = false;
			shapeDebugSettings.ShowAlpha = false;
			shapeDebugSettings.DrawAllChannels = false;

			glm::vec4 selectedChannel = ColorFromMask(inOutMask);

			if (selectedChannel.a == 1.0)
			{
				shapeDebugSettings.ShowAlpha = true;
				shapeDebugSettings.DisplaySelectedChannelOnly = false;
			}
			else
			{
				shapeDebugSettings.ChannelWeights = selectedChannel;
			}
		}

		if (!shapeDebugSettings.DrawAllChannels && !shapeDebugSettings.DisplaySelectedChannelOnly)
		{
			if (ImGui::Checkbox("Show Alpha", &shapeDebugSettings.ShowAlpha))
			{
				shapeDebugSettings.ChannelWeights *= glm::vec4(0.0);

				if (shapeDebugSettings.ShowAlpha)
					shapeDebugSettings.EnableGreyScale = false;

				shapeDebugSettings.ChannelWeights.a = shapeDebugSettings.ShowAlpha ? 1.0 : 0.0;

			}
			if (ImGui::Checkbox("Enable Grey Scale", &shapeDebugSettings.EnableGreyScale))
			{
				shapeDebugSettings.ChannelWeights *= glm::vec4(0.0);

				if (shapeDebugSettings.EnableGreyScale)
				{
					shapeDebugSettings.ShowAlpha = false;
					shapeDebugSettings.ChannelWeights += glm::vec4(1.0f);
					shapeDebugSettings.ChannelWeights.a = 0.0f;
				}
			}
			ImGui::DragFloat4("Channel Weights", &shapeDebugSettings.ChannelWeights.x, 0.001f, 0.0, 1.0);
		}
		ImGui::DragFloat("Depth Slice", &shapeDebugSettings.DepthSlice, 0.001f, 0.0, 1.0);
	}


	void CloudsUI::DrawDetailShapeSelectionUI(DetailTextureDebugDisplaySettings& detailDebugSettings, WorleyChannelMask& inOutMask)
	{
		std::string currentChannel = NameFromWorleyChannel(inOutMask);
		if (ImGui::BeginCombo("Channel Selection", currentChannel.c_str()))
		{
			for (auto channelType : detailDebugSettings.ChannelTypes)
			{
				bool isSelect = channelType == inOutMask;

				if (ImGui::Selectable(NameFromWorleyChannel(channelType).c_str(), isSelect))
				{
					inOutMask = channelType;

					if (detailDebugSettings.DisplaySelectedChannelOnly)
					{
						detailDebugSettings.EnableGreyScale = false;
						detailDebugSettings.DrawAllChannels = false;

						glm::vec4 selectedChannel = ColorFromMask(inOutMask);
						detailDebugSettings.ChannelWeights = selectedChannel;
					}
				}
			}
			ImGui::EndCombo();
		}

		if (ImGui::Checkbox("Show All Channels", &detailDebugSettings.DrawAllChannels))
		{
			detailDebugSettings.EnableGreyScale = false;
			detailDebugSettings.DisplaySelectedChannelOnly = false;
		}

		if (ImGui::Checkbox("Show Selected Channel Only", &detailDebugSettings.DisplaySelectedChannelOnly))
		{
			detailDebugSettings.EnableGreyScale = false;
			detailDebugSettings.DrawAllChannels = false;

			glm::vec4 selectedChannel = ColorFromMask(inOutMask);

			detailDebugSettings.ChannelWeights = selectedChannel;
		}

		if (!detailDebugSettings.DrawAllChannels && !detailDebugSettings.DisplaySelectedChannelOnly)
		{
			if (ImGui::Checkbox("Enable Grey Scale", &detailDebugSettings.EnableGreyScale))
			{
				if (detailDebugSettings.EnableGreyScale)
					detailDebugSettings.ChannelWeights = glm::vec4(1.0f);
			}
			ImGui::DragFloat3("Channel Weights", &detailDebugSettings.ChannelWeights.x, 0.001f, 0.0, 1.0);
		}
		ImGui::DragFloat("Depth Slice", &detailDebugSettings.DepthSlice, 0.001f, 0.0, 1.0);
	}

	void CloudsUI::Draw(const VolumetricCloudComponent* cloudComponentPtr)
	{
		if(cloudComponentPtr == nullptr) return;
		
		if (ImGui::BeginTabBar("Scene Settings"))
		{
			for (auto& tabType : m_AvailableTabTypes)
			{
				if (ImGui::BeginTabItem(NameFromUITabType(tabType).c_str()))
				{
					cloudComponentPtr->DebugSettings->ActiveTabType = tabType;

					if (cloudComponentPtr->DebugSettings->ActiveTabType == UITabTypes::MainSettings)
					{
						DrawMainSettings(*cloudComponentPtr->MainSettings);
					}
					else if (cloudComponentPtr->DebugSettings->ActiveTabType == UITabTypes::CloudSettings)
					{
						DrawCloudSettingsUI(*cloudComponentPtr->MainSettings, *cloudComponentPtr->AnimationSettings);
					}
					else if (cloudComponentPtr->DebugSettings->ActiveTabType == UITabTypes::NoiseTextureSettings)
					{
						DrawNoiseEditorSettings(*cloudComponentPtr->DebugSettings);
						if (cloudComponentPtr->DebugSettings->ActiveNoiseType == CloudNoiseType::BaseShape)
						{
							DrawBaseShapeUI(*cloudComponentPtr->BaseShapeSettings, cloudComponentPtr->DebugSettings->ActiveShapeMask, *cloudComponentPtr->PerlinSettings);
						}
						else if (cloudComponentPtr->DebugSettings->ActiveNoiseType == CloudNoiseType::DetailShape)
						{
							DrawDetailShapeUI(*cloudComponentPtr->DetailShapeSettings, cloudComponentPtr->DebugSettings->ActiveDetailMask);
						}
						else if (cloudComponentPtr->DebugSettings->ActiveNoiseType == CloudNoiseType::Perlin)
						{
							DrawPerlinUI(*cloudComponentPtr->PerlinSettings);
						}
						else if (cloudComponentPtr->DebugSettings->ActiveNoiseType == CloudNoiseType::Curl)
						{
							DrawCurlUI(*cloudComponentPtr->CurlSettings);
						}
					}
					ImGui::EndTabItem();
				}
			}

			ImGui::EndTabBar();
		}
	}

	void CloudsUI::DrawMainSettings(CloudSettings& cloudSettings)
	{
		ImGui::Checkbox("Draw Clouds", &cloudSettings.DrawClouds);
	}

	void CloudsUI::DrawNoiseEditorSettings(VolumetricCloudComponent::Debug& debugSettings)
	{
		if (ImGui::Checkbox("Display Texture Viewer", &debugSettings.MainTextureDebugSettings->EnableTextureViewer))
			debugSettings.MainTextureDebugSettings->PercentScreenTextureDisplay = debugSettings.MainTextureDebugSettings->EnableTextureViewer ? 0.1f : 0.0f;

		if (!debugSettings.MainTextureDebugSettings->EnableTextureViewer)
			return;

		ImGui::DragFloat("Percent Screen Display", &debugSettings.MainTextureDebugSettings->PercentScreenTextureDisplay, 0.001f, 0.0f, 1.0f);

		std::string currentTextureEditor = NameFromUIType(debugSettings.ActiveNoiseType);
		if (ImGui::BeginCombo("Texture Editor Selection", currentTextureEditor.c_str()))
		{
			for (auto editorType : debugSettings.MainTextureDebugSettings->EditorTypes)
			{
				bool isSelect = editorType == debugSettings.ActiveNoiseType;

				if (ImGui::Selectable(NameFromUIType(editorType).c_str(), isSelect))
				{
					debugSettings.ActiveNoiseType = editorType;
					debugSettings.ActiveDebugShapeType = debugSettings.ActiveNoiseType == CloudNoiseType::Perlin
						|| debugSettings.ActiveNoiseType == CloudNoiseType::Curl
							? DebugShapeType::None
						: (debugSettings.ActiveNoiseType == CloudNoiseType::BaseShape
							? DebugShapeType::BaseShape
						: DebugShapeType::DetailNoise);
				}
			}
			ImGui::EndCombo();
		}

		if (debugSettings.ActiveNoiseType == CloudNoiseType::BaseShape)
			DrawBaseShapeSelectionUI(*debugSettings.ShapeTextureDisplaySettings, debugSettings.ActiveShapeMask);
		if (debugSettings.ActiveNoiseType == CloudNoiseType::DetailShape)
			DrawDetailShapeSelectionUI(*debugSettings.DetailTextureDisplaySettings, debugSettings.ActiveDetailMask);
	}

	void CloudsUI::DrawCloudSettingsUI(CloudSettings& cloudSettings, CloudAnimationSettings& animSettings) const
	{
		if (ImGui::TreeNodeEx("Cloud Scale/Container Settings"))
		{
			ImGui::DragFloat3("Cloud Container Position", &cloudSettings.CloudContainerPosition.x, 0.1f);
			ImGui::DragFloat3("Cloud Container Scale", &cloudSettings.CloudContainerScale.x, 0.1f);
			ImGui::DragFloat("Container Edge Fade Distance", &cloudSettings.ContainerEdgeFadeDistance, 0.1f);
			ImGui::DragFloat("Cloud Scale", &cloudSettings.CloudScale, 0.01f, 0.0);
			ImGui::DragFloat("Cloud Scale Factor", &cloudSettings.CloudScaleFactor, 0.1f);
			ImGui::TreePop();
		}

		if (ImGui::TreeNodeEx("Animation Settings"))
		{
			ImGui::Checkbox("Animate Clouds", &animSettings.AnimateClouds);
			ImGui::DragFloat("Animation Speed", &animSettings.AnimationSpeed, 0.1f);
			ImGui::DragFloat("Time Scale", &animSettings.TimeScale, 0.001f);
			ImGui::DragFloat("Cloud Scroll Offset Speed", &animSettings.CloudScrollOffsetSpeed, 0.1f);
			ImGui::DragFloat3("Shape Texture Offset", &animSettings.ShapeTextureOffset.x, 1.0f);
			ImGui::TreePop();
		}

		if (ImGui::TreeNodeEx("Density Settings"))
		{
			if (ImGui::DragInt("Density Steps", &cloudSettings.DensitySteps, 0.1f, 1))
				if (cloudSettings.DensitySteps < 1) cloudSettings.DensitySteps = 1;
			ImGui::DragFloat("Random Ray Offset", &cloudSettings.RandomOffsetStrength, 0.01f, 0.0);
			ImGui::DragFloat("Curl Intensity", &cloudSettings.CurlIntensity, 0.01f);
			ImGui::DragFloat("Density Multiplier", &cloudSettings.DensityMultiplier, 0.001f, 0.0);
			ImGui::DragFloat("Density Threshold", &cloudSettings.DensityThreshold, 0.001f, 0.0);
			ImGui::DragFloat4("Shape Noise Weights", &cloudSettings.ShapeNoiseWeights.x, 0.01f, 0.0f, 1.0f);
			ImGui::DragFloat3("Detail Noise Weights", &cloudSettings.DetailNoiseWeights.x, 0.01f, 0.0f, 1.0f);
			ImGui::DragFloat("Detail Noise Weight", &cloudSettings.DetailNoiseWeight, 0.01f, 0.0);
			ImGui::DragFloat3("Cloud Type Weights", &cloudSettings.CloudTypeWeights.x, 0.01f, 0.0f, 1.0f);
			ImGui::DragFloat("Cloud Type Weight Strength", &cloudSettings.CloudTypeWeightStrength, 0.01f, 0.0);

			ImGui::TreePop();
		}

		if (ImGui::TreeNodeEx("Lighting Settings"))
		{
			if (ImGui::DragInt("Light Steps", &cloudSettings.LightSteps, 0.1f, 1))
				if (cloudSettings.LightSteps < 1) cloudSettings.LightSteps = 1;
			ImGui::DragFloat("Powder Constant", &cloudSettings.PowderConstant, 0.001f, 0.0);
			ImGui::DragFloat("Silver Lining Constant", &cloudSettings.SilverLiningConstant, 0.001f, 0.0);
			ImGui::DragFloat("Extinction Factor", &cloudSettings.ExtinctionFactor, 0.01f);
			ImGui::DragFloat("Phase Blend", &cloudSettings.PhaseBlend, 0.001f, 0.0);
			ImGui::DragFloat("Forward Scattering", &cloudSettings.ForwardScattering, 0.001f, 0.0, 1.0);
			ImGui::DragFloat("Back Scattering", &cloudSettings.BackScattering, 0.001f, 0.0, 1.0);
			ImGui::DragFloat("Base Brightness", &cloudSettings.BaseBrightness, 0.001f, 0.0);
			ImGui::DragFloat("Phase Factor", &cloudSettings.PhaseFactor, 0.001f, 0.0, 1.0);
			ImGui::TreePop();
		}
	}

	void CloudsUI::DrawBaseShapeUI(BaseShapeWorleySettings& baseShapeSettings, WorleyChannelMask baseShapeMask, const WorleyPerlinSettings& perlinSettings) const
	{
		Ref<WorleyChannelData> activeWorleyChannelData = ShapeChannelFromMask(baseShapeSettings, baseShapeMask);
		if (activeWorleyChannelData == nullptr) return;

		bool updated = false;
		bool updatedPoints = false;

		if (ImGui::Checkbox("Invert", &activeWorleyChannelData->InvertWorley))
			updated = true;

		if (activeWorleyChannelData->InvertWorley)
			if (ImGui::DragFloat("Inversion Weight", &activeWorleyChannelData->InversionWeight, 0.01f, 0.0, 1.0))
				updated = true;

		if (ImGui::DragInt3("Layer Cells", &activeWorleyChannelData->LayerCells.x, 0.1f, 1))
		{
			updated = true;
			updatedPoints = true;
		}
		if (ImGui::DragInt3("Layer Seeds", &activeWorleyChannelData->LayerSeeds.x, 0.1f))
			updated = true;
		if (ImGui::DragFloat("Persistence", &activeWorleyChannelData->WorleyLayerPersistence, 0.001f, 0.01f))
			updated = true;
		if (ImGui::DragFloat("Tiling", &activeWorleyChannelData->WorleyTiling, 0.01f))
			updated = true;

		if (activeWorleyChannelData->Mask == WorleyChannelMask::R)
		{
			if (ImGui::DragFloat("Perlin Worley Mix", &baseShapeSettings.PerlinWorleyMix, 0.01f))
				updated = true;
		}

		if (ImGui::Button("Generate New Points") || updatedPoints)
		{
			activeWorleyChannelData->UpdatePoints();
			updated = true;
		}
		if (ImGui::Button("Update Channel") || updated)
			baseShapeSettings.UpdateChannel(baseShapeMask, perlinSettings.PerlinTexture);
	}

	void CloudsUI::DrawDetailShapeUI(DetailShapeWorleySettings& detailShapeSettings, WorleyChannelMask detailShapeMask) const
	{
		Ref<WorleyChannelData> activeDetailChannelData = DetailChannelFromMask(detailShapeSettings, detailShapeMask);
		if (activeDetailChannelData == nullptr) return;

		bool updated = false;
		bool updatedPoints = false;

		if (ImGui::Checkbox("Invert", &activeDetailChannelData->InvertWorley))
			updated = true;
		if (activeDetailChannelData->InvertWorley)
			if (ImGui::DragFloat("Inversion Weight", &activeDetailChannelData->InversionWeight, 0.01f, 0.0, 1.0))
				updated = true;
		if (ImGui::DragInt3("Layer Cells", &activeDetailChannelData->LayerCells.x, 0.1f, 1))
		{
			updated = true;
			updatedPoints = true;
		}
		if (ImGui::DragInt3("Layer Seeds", &activeDetailChannelData->LayerSeeds.x, 0.1f))
			updated = true;
		if (ImGui::DragFloat("Persistence", &activeDetailChannelData->WorleyLayerPersistence, 0.001f, 0.01f))
			updated = true;
		if (ImGui::DragFloat("Tiling", &activeDetailChannelData->WorleyTiling, 0.01f))
			updated = true;

		if (ImGui::Button("Generate New Points") || updatedPoints)
		{
			activeDetailChannelData->UpdatePoints();
			updated = true;
		}
		if (ImGui::Button("Update Channel") || updated)
			detailShapeSettings.UpdateChannel(detailShapeMask);
	}

	void CloudsUI::DrawPerlinUI(WorleyPerlinSettings& perlinSettings) const
	{
		bool updated = false;
		if (ImGui::DragInt("Octaves", &perlinSettings.Octaves, 0.1f, 0, 8))
			updated = true;
		if (ImGui::DragFloat("Perlin Texture Scale", &perlinSettings.NoiseScale, 0.001f, 0.01f))
			updated = true;
		if (ImGui::DragFloat("Perlin Persistence", &perlinSettings.Persistence, 0.001f, 0.01f, 1.0f))
			updated = true;
		if (ImGui::DragFloat("Perlin Lacunarity", &perlinSettings.Lacunarity, 0.25f, 1.0f))
			updated = true;
		if (ImGui::DragFloat2("Perlin Texture Offset", &perlinSettings.TextureOffset.x, 10.0f))
			updated = true;

		if (ImGui::Button("Generate New Offsets"))
		{
			perlinSettings.UpdatePoints();
			updated = true;
		}
		if (ImGui::Button("Update Noise"))
			perlinSettings.UpdateTexture();

		if (updated)
			perlinSettings.UpdateTexture();
	}

	void CloudsUI::DrawCurlUI(CurlNoiseSettings& curlSettings) const
	{
		bool updated = false;
		if (ImGui::DragFloat("Strength", &curlSettings.Strength, 0.001f))
			updated = true;
		if (ImGui::DragFloat("Tiling", &curlSettings.Tiling, 0.001f, 0.01f))
			updated = true;
		if (ImGui::DragFloat2("Texture Offset", &curlSettings.TilingOffset.x, 1.0f))
			updated = true;
		if (ImGui::Button("Update Noise") || updated)
			curlSettings.UpdateTexture();
	}
}

