#include "ohmpch.h"
#include "Ohm/Rendering/TextureLibrary.h"

#include <glad/glad.h>

namespace Ohm
{
    bool TextureLibrary::Has2D(const std::string& Name)
	{
		return s_NameToTexture2DLibrary.find(Name) != s_NameToTexture2DLibrary.end();
	}

	void TextureLibrary::AddTexture2D(const Ref<Texture2D>& texture)
	{
		if (Has2D(texture->GetName()))
		{
			OHM_WARN("Texture2D with name '{}' already contained in Texture Library.", texture->GetName());
			return;
		}
		
		s_NameToTexture2DLibrary[texture->GetName()] = texture;
		s_IdToNameLibrary[texture->GetID()] = texture->GetName();
    	s_TextureLibraryIDs[texture->GetID()] = texture;
		OHM_TRACE("Added Texture2D with name: '{}' to the Texture Library.", texture->GetName());
	}

	Ref<Texture2D> TextureLibrary::LoadTexture2D(const Texture2DSpecification& spec, const std::string& filePath)
	{
		if (!filePath.empty())
		{
			Ref<Texture2D> texture = CreateRef<Texture2D>(filePath, spec);
			AddTexture2D(texture);
			return texture;
		}

		Ref<Texture2D> texture = CreateRef<Texture2D>(spec);
		AddTexture2D(texture);
		return texture;
	}

	bool TextureLibrary::HasCube(const std::string& Name)
	{
		return s_NameToTextureCubeLibrary.find(Name) != s_NameToTextureCubeLibrary.end();
	}

	void TextureLibrary::AddTextureCube(const Ref<TextureCube>& texture)
	{
		if (HasCube(texture->GetName()))
		{
			OHM_TRACE("TextureCube with name '{}' already contained in Texture Library.", texture->GetName());
			return;
		}
		
		s_NameToTextureCubeLibrary[texture->GetName()] = texture;
		s_IdToNameLibrary[texture->GetID()] = texture->GetName();
		OHM_TRACE("Added TextureCube with name: '{}' to the Texture Library.", texture->GetName());
	}

	Ref<TextureCube> TextureLibrary::LoadTextureCube(const TextureCubeSpecification& Spec, bool InvalidateIfExists)
	{
		if(HasCube(Spec.Name))
		{
			if(InvalidateIfExists)
				s_NameToTextureCubeLibrary[Spec.Name]->Invalidate(Spec);

			return s_NameToTextureCubeLibrary[Spec.Name];
		}
		Ref<TextureCube> texture = CreateRef<TextureCube>(Spec);
		AddTextureCube(texture);
		return texture;
	}

	void TextureLibrary::InvalidateCube(const TextureCubeSpecification& spec)
	{
		ASSERT(HasCube(spec.Name), "Unable to Invalidate TextureCube with name '{}' - does not exist", spec.Name);
		const Ref<TextureCube> TextureCube = s_NameToTextureCubeLibrary[spec.Name];
		TextureCube->Invalidate(spec);
	}

	Ref<Texture2D> TextureLibrary::Get2DFromID(uint32_t ID)
	{
		ASSERT(s_IdToNameLibrary.find(ID) != s_IdToNameLibrary.end(), "Unable to find Texture2D with ID: {}", ID);
		const auto TextureName = s_IdToNameLibrary[ID];
		ASSERT(s_NameToTexture2DLibrary.find(TextureName) != s_NameToTexture2DLibrary.end(), "Unable to find Texture2D with Name: {}", TextureName);
		return s_NameToTexture2DLibrary[TextureName];
	}

	Ref<TextureCube> TextureLibrary::GetCubeFromID(uint32_t ID)
	{
		ASSERT(s_IdToNameLibrary.find(ID) != s_IdToNameLibrary.end(), "Unable to find TextureCube with ID: {}", ID);
		const auto TextureName = s_IdToNameLibrary[ID];
		ASSERT(s_NameToTexture2DLibrary.find(TextureName) != s_NameToTexture2DLibrary.end(), "Unable to find TextureCube with Name: {}", TextureName);
		return s_NameToTextureCubeLibrary[TextureName];
	}

	Ref<Texture2D> TextureLibrary::LoadTexture2D(const std::string& filePath)
	{
		Texture2DSpecification defaultFromFileSpec =
		{
			TextureUtils::WrapMode::Repeat,
			TextureUtils::WrapMode::Repeat,
			TextureUtils::FilterMode::LinearMipLinear,
			TextureUtils::FilterMode::Linear,
			TextureUtils::ImageInternalFormat::FromImage,
			TextureUtils::ImageDataLayout::FromImage,
			TextureUtils::ImageDataType::UByte,
		};

		Ref<Texture2D> texture = CreateRef<Texture2D>(filePath, defaultFromFileSpec);
		AddTexture2D(texture);

		return texture;
	}

	Ref<Texture2D> TextureLibrary::LoadTexture2D(const Texture2DSpecification& Spec, void* Data)
	{
		if(Has2D(Spec.Name))
			return s_NameToTexture2DLibrary[Spec.Name];

		Texture2DSpecification defaultFromFileSpec =
		{
			TextureUtils::WrapMode::Repeat,
			TextureUtils::WrapMode::Repeat,
			TextureUtils::FilterMode::LinearMipLinear,
			TextureUtils::FilterMode::Linear,
			TextureUtils::ImageInternalFormat::FromImage,
			TextureUtils::ImageDataLayout::FromImage,
			TextureUtils::ImageDataType::UByte,
		};

		Ref<Texture2D> texture = CreateRef<Texture2D>(Spec, Data);
		AddTexture2D(texture);

		return texture;
	}

	const Ref<Texture2D>& TextureLibrary::Get2D(const std::string& name)
	{
		ASSERT(Has2D(name), "No Texture2D with name '{}' found in Texture Library.", name)
		return s_NameToTexture2DLibrary.at(name);
	}

	const Ref<TextureCube>& TextureLibrary::GetCube(const std::string& name)
	{
		ASSERT(HasCube(name), "No TextureCube with name '{}' found in Texture Library.", name)
		return s_NameToTextureCubeLibrary.at(name);
	}

	void TextureLibrary::BindTexture2DToSlot(const std::string& TwoDimensionTextureName, uint32_t Slot)
	{
		ASSERT(Has2D(TwoDimensionTextureName), "TextureLibrary: Unable to bind Texture2D with name '{}' to slot '{}'.  This texture has not been registered.", TwoDimensionTextureName, Slot);
		const auto& Texture2D = s_NameToTexture2DLibrary[TwoDimensionTextureName];
		glBindTextureUnit(Slot, Texture2D->GetID());
	}

	void TextureLibrary::BindTextureCubeToSlot(const std::string& CubeTextureName, uint32_t Slot)
	{
		ASSERT(HasCube(CubeTextureName), "TextureLibrary: Unable to bind TextureCube with name '{}' to slot '{}'.  This texture has not been registered.", CubeTextureName, Slot);
		const auto& TextureCube = s_NameToTextureCubeLibrary[CubeTextureName];
		glBindTextureUnit(Slot, TextureCube->GetID());
	}

	void TextureLibrary::BindTextureToSlot(uint32_t TexID, uint32_t Slot)
	{
		glBindTextureUnit(Slot, TexID);
	}

	std::string TextureLibrary::GetNameFromID(uint32_t TextureID)
	{
		ASSERT(s_IdToNameLibrary.find(TextureID) != s_IdToNameLibrary.end(), "TextureLibrary: Unable to find texture with ID '{}'.", TextureID);
		return s_IdToNameLibrary[TextureID];
	}

	std::unordered_map<std::string, int32_t> TextureLibrary::BindAndGetMaterialTextureSlots(const std::unordered_map<std::string, uint32_t>& textureIDs)
    {
    	std::unordered_map<std::string, int32_t> nameToSlotMap;
    	uint32_t currentSlot = 1;
    	for (auto [name, id] : textureIDs)
    	{
    		if (s_TextureLibraryIDs.find(id) == s_TextureLibraryIDs.end())
    		{
    			// THIS NEEDS TO BE UPDATED TO HANDLE TEXTURES THAT HAVEN'T BEEN REGISTERED!
    			//OHM_WARN("Texture with name: '{}' and id: '{}' could not be found in the Texture Library.  Could not bind this texture.  Check that it has been added to the Texture Library.", name, id);
    			//continue;
    		}

    		if (name == "sampler_ShadowMap")
    		{
    			nameToSlotMap[name] = 0;
    			glBindTextureUnit(0, id);
    		}
    		else
    		{
    			nameToSlotMap[name] = currentSlot;
    			glBindTextureUnit(currentSlot++, s_TextureLibraryIDs[id]->GetID());
    		}
    	}

    	return nameToSlotMap;
    }
	
	
	void TextureLibrary::LoadWhiteTexture()
	{
		Texture2DSpecification whiteTextureSpec =
		{	
			TextureUtils::WrapMode::Repeat,
			TextureUtils::WrapMode::Repeat,
			TextureUtils::FilterMode::Linear,
			TextureUtils::FilterMode::Linear,
			TextureUtils::ImageInternalFormat::RGBA8,
			TextureUtils::ImageDataLayout::RGBA,
			TextureUtils::ImageDataType::UByte,
			1, 1,
			"White Texture"
		};

		Ref<Texture2D> WhiteTexture = CreateRef<Texture2D>(whiteTextureSpec);
		uint32_t whiteTextureData = 0xffffffff;
		WhiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));
		AddTexture2D(WhiteTexture);
	}

	void TextureLibrary::LoadBlackTexture()
	{
		Texture2DSpecification whiteTextureSpec =
		{	
			TextureUtils::WrapMode::Repeat,
			TextureUtils::WrapMode::Repeat,
			TextureUtils::FilterMode::Linear,
			TextureUtils::FilterMode::Linear,
			TextureUtils::ImageInternalFormat::RGBA8,
			TextureUtils::ImageDataLayout::RGBA,
			TextureUtils::ImageDataType::UByte,
			1, 1,
			"Black Texture"
		};

		const Ref<Texture2D> BlackTexture = CreateRef<Texture2D>(whiteTextureSpec);
		uint32_t blackTextureData = 0xff000000;
		BlackTexture->SetData(&blackTextureData, sizeof(uint32_t));
		AddTexture2D(BlackTexture);
	}

	void TextureLibrary::LoadBlackTextureCube()
	{
		const TextureCubeSpecification BlackCubeTextureSpec =
		{
			TextureUtils::WrapMode::ClampToEdge,
			TextureUtils::WrapMode::ClampToEdge,
			TextureUtils::WrapMode::ClampToEdge,
			TextureUtils::FilterMode::LinearMipLinear,
			TextureUtils::FilterMode::Linear,
			TextureUtils::ImageInternalFormat::RGBA8,
			TextureUtils::ImageDataLayout::RGBA,
			TextureUtils::ImageDataType::UByte,
			1,
			"Black TextureCube"
		};

		const Ref<TextureCube> BlackTextureCube = CreateRef<TextureCube>(BlackCubeTextureSpec);
		uint32_t blackTextureData = 0xff000000;
		BlackTextureCube->SetData(&blackTextureData, sizeof(uint32_t));
		AddTextureCube(BlackTextureCube);
	}

	std::unordered_map<std::string, Ref<Texture2D>> TextureLibrary::s_NameToTexture2DLibrary;
	std::unordered_map<std::string, Ref<TextureCube>> TextureLibrary::s_NameToTextureCubeLibrary;
	std::unordered_map<uint32_t, Ref<Texture2D>> TextureLibrary::s_TextureLibraryIDs;
	std::unordered_map<uint32_t, std::string> TextureLibrary::s_IdToNameLibrary;
}
