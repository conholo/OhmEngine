#pragma once
#include "Ohm/Core/Assert.h"
#include "Ohm/Core/Utility.h"
#include "Ohm/UI/PropertyDrawer.h"
#include "Ohm/Rendering/Material.h"

namespace Ohm
{
    namespace UI
    {
        class MaterialInspector
        {
        public:
            MaterialInspector() = default;
            MaterialInspector(const Ref<Material>& Material);
            ~MaterialInspector() = default;
            void Draw() const;

            template<typename UIPropertyType>
            UIPropertyType* Get(const std::string& UniformName)
            {
                if(typeid(UIPropertyType) == typeid(UIFloat))
                    return (UIPropertyType*)&(*m_FloatDrawers[UniformName]);
                if(typeid(UIPropertyType) == typeid(UIInt))
                    return (UIPropertyType*)&(*m_IntDrawers[UniformName]);
                if(typeid(UIPropertyType) == typeid(UIVector2))
                    return (UIPropertyType*)&(*m_Vec2Drawers[UniformName]);
                if(typeid(UIPropertyType) == typeid(UIVector3))
                    return (UIPropertyType*)&(*m_Vec3Drawers[UniformName]);
                if(typeid(UIPropertyType) == typeid(UIVector4))
                    return (UIPropertyType*)&(*m_Vec4Drawers[UniformName]);
                if(typeid(UIPropertyType) == typeid(UIColor))
                    return (UIPropertyType*)&(*m_ColorDrawers[UniformName]);
                if(typeid(UIPropertyType) == typeid(UITexture2D))
                    return (UIPropertyType*)&(*m_TextureDrawers[UniformName]);
                ASSERT(false, "MaterialInspector: Unknown type: {}", TypeName<UIProperty>());
                return nullptr;
            }

            std::string GetMaterialName() const { return m_Material->GetName(); }

        private:
            void CreatePropertyDrawers();

            Ref<Material> m_Material{};
            std::unordered_map<std::string, Ref<UIFloat>>		m_FloatDrawers{};
            std::unordered_map<std::string, Ref<UIInt>>		    m_IntDrawers{};
            std::unordered_map<std::string, Ref<UIVector2>>	    m_Vec2Drawers{};
            std::unordered_map<std::string, Ref<UIVector3>>	    m_Vec3Drawers{};
            std::unordered_map<std::string, Ref<UIVector4>>	    m_Vec4Drawers{};
            std::unordered_map<std::string, Ref<UIColor>>		m_ColorDrawers{};
            std::unordered_map<std::string, Ref<UITexture2D>>	m_TextureDrawers{};
        };
    }
}
