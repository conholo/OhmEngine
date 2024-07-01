#include "MaterialInspector.h"
#include <imgui/imgui.h>


namespace Ohm
{
    namespace UI
    {
	    MaterialInspector::MaterialInspector(const Ref<Material>& Material)
		    :m_Material(Material)
	    {
	    	CreatePropertyDrawers();
	    }

	    void MaterialInspector::Draw() const
	    {
	    	for(const auto& [name, FloatDrawer] : m_FloatDrawers)
	    		FloatDrawer->Draw();
	    	for(const auto&  [name, IntDrawer] : m_IntDrawers)
	    		IntDrawer->Draw();
	    	for(const auto& [name, Vec2Drawer] : m_Vec2Drawers)
	    		Vec2Drawer->Draw();
	    	for(const auto& [name, Vec3Drawer] : m_Vec3Drawers)
	    		Vec3Drawer->Draw();
	    	for(const auto& [name, Vec4Drawer] : m_Vec4Drawers)
	    		Vec4Drawer->Draw();
	    	for(const auto& [name, ColorDrawer] : m_ColorDrawers)
	    		ColorDrawer->Draw();
	    	for(const auto& [name, TextureDrawer] : m_TextureDrawers)
	    		TextureDrawer->Draw();

	    	if (ImGui::Button("Log Shader Data"))
	    		m_Material->GetShader()->LogShaderData();
	    }

        void MaterialInspector::CreatePropertyDrawers()
        {
            const auto& uniforms = m_Material->GetShader()->GetBaseBlockUniforms();

			for (auto [name, uniform] : uniforms)
			{
				if(name.substr(0, 4) == "hide") continue;
				
				const size_t Offset = name.find_first_of('_', 0);
				std::string StrippedName = name.substr(Offset + 1, strlen(name.c_str()));

				std::string StrippedToLower = std::string(StrippedName);
				std::transform(StrippedToLower.begin(), StrippedToLower.end(), StrippedToLower.begin(), ::tolower);
				const bool ColorFound = StrippedToLower.find("color") != std::string::npos;

				switch (UIPropertyTypeFromShaderDataType(uniform.GetType(), ColorFound))
				{
				case UIPropertyType::Float:
					{
						float* value = m_Material->Get<float>(name);
						m_FloatDrawers[name] = CreateRef<UIFloat>(StrippedName, value);
						break;
					}
				case UIPropertyType::Int:
					{
						int* value = m_Material->Get<int>(name);
						m_IntDrawers[name] = CreateRef<UIInt>(StrippedName, value);
						break;
					}
				case UIPropertyType::Vec2:
					{
						glm::vec2* value = m_Material->Get<glm::vec2>(name);
						m_Vec2Drawers[name] = CreateRef<UIVector2>(StrippedName, value);
						break;
					}
				case UIPropertyType::Vec3:
					{
						glm::vec3* value = m_Material->Get<glm::vec3>(name);
						m_Vec3Drawers[name] = CreateRef<UIVector3>(StrippedName, value);
						break;
					}
				case UIPropertyType::Vec4:
					{
						glm::vec4* value = m_Material->Get<glm::vec4>(name);
						m_Vec4Drawers[name] = CreateRef<UIVector4>(StrippedName, value);
						break;
					}
				case UIPropertyType::Color:
					{
						glm::vec4* value = m_Material->Get<glm::vec4>(name);
						m_ColorDrawers[name] = CreateRef<UIColor>(StrippedName, value);
						break;
					}
				case UIPropertyType::Texture:
					{
						TextureUniform* value = m_Material->Get<TextureUniform>(name);
						if(value->HideInUI == 1) continue;
						m_TextureDrawers[name] = CreateRef<UITexture2D>(StrippedName, m_Material, value, name);
						break;
					}
				default: ;
				}
            }
        }
    }
}

