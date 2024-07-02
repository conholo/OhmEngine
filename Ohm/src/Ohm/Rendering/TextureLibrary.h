#pragma once
#include "Texture2D.h"
#include "Texture3D.h"
#include "TextureCube.h"


namespace Ohm
{
    class TextureLibrary
    {
    public:
        static Ref<Texture2D> Get2DFromID(uint32_t ID);
        static Ref<TextureCube> GetCubeFromID(uint32_t ID);
		
        static Ref<Texture2D> LoadTexture2D(const std::string& filePath = "");
        static Ref<Texture2D> LoadTexture2D(const Texture2DSpecification& spec, const std::string& filePath = "");
        static Ref<Texture2D> LoadTexture2D(const Texture2DSpecification& Spec, void* Data);
        static void AddTexture2D(const Ref<Texture2D>& texture);
        static const Ref<Texture2D>& Get2D(const std::string& name);
        static void BindTexture2DToSlot(const std::string& TwoDimensionTextureName, uint32_t Slot);
        static bool Has2D(const std::string& Name);
        
        static Ref<TextureCube> LoadTextureCube(const TextureCubeSpecification& Spec, bool InvalidateIfExists = false);
        static void InvalidateCube(const TextureCubeSpecification& spec);
        static void AddTextureCube(const Ref<TextureCube>& texture);
        static const Ref<TextureCube>& GetCube(const std::string& name);
        static void BindTextureCubeToSlot(const std::string& CubeTextureName, uint32_t Slot);
        static bool HasCube(const std::string& Name);

        static Ref<Texture3D> LoadTexture3D(const Texture3DSpecification& spec);
        static void AddTexture3D(const Ref<Texture3D>& texture);
        static const Ref<Texture3D>& Get3D(const std::string& name);
        static void BindTexture3DToSlot(const std::string& name, uint32_t slot);
        static bool Has3D(const std::string& name);

        static std::unordered_map<std::string, Ref<Texture2D>> Get2DLibrary() { return s_NameToTexture2DLibrary; }

        static void BindTextureToSlot(uint32_t TexID, uint32_t Slot);
        static std::string GetNameFromID(uint32_t TextureID);

        static void LoadWhiteTexture();
        static void LoadBlackTexture();
        static void LoadBlackTextureCube();
        static void LoadBlackTexture3D();

    private:
        static std::unordered_map<std::string, Ref<Texture3D>> s_NameToTexture3DLibrary;
        static std::unordered_map<std::string, Ref<Texture2D>> s_NameToTexture2DLibrary;
        static std::unordered_map<std::string, Ref<TextureCube>> s_NameToTextureCubeLibrary;
		static std::unordered_map<uint32_t, Ref<Texture2D>> s_TextureLibraryIDs;
        static std::unordered_map<uint32_t, std::string> s_IdToNameLibrary;
    };
}
