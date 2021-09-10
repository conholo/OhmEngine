#pragma once

#include "Ohm.h"
#include "Ohm/Rendering/EditorCamera.h"
#include "Ohm/Rendering/FBO.h"

namespace Ohm
{
	class EditorLayer : public Layer
	{
	public:

		EditorLayer();
		virtual ~EditorLayer();

		void OnAttach() override;
		void OnUpdate(Time dt) override;
		void OnDetach() override;
		void OnUIRender() override;
		void OnEvent(Event& event) override;

	private:

		Entity m_Quad;
		glm::vec3 m_QuadPosition{ 0.0f, 8.0f, 0.0f };
		glm::vec3 m_QuadRotationDegrees{ 0.0f };
		glm::vec3 m_QuadRotation{ 0.0f };
		glm::vec3 m_QuadSize{ 3.0f };
		glm::vec4 m_QuadColor{ 0.0f, 0.5f, 0.2f, 1.0f };
		float m_QuadSpecularStrength = 0.8f;
		float m_QuadAmbientStrength = 0.5f;
		float m_QuadDiffuseStrength = 1.0f;
		bool m_QuadIsTextured = false;


		Entity m_Torus;
		glm::vec3 m_TorusPosition{ 0.0f, 3.0f, 0.0f };
		glm::vec3 m_TorusRotationDegrees{ 0.0f };
		glm::vec3 m_TorusRotation{ 0.0f };
		glm::vec3 m_TorusSize{ 5.0f };
		glm::vec4 m_TorusColor{ 0.0f, 0.8f, 0.8f, 1.0f };
		float m_TorusSmoothBlend = 0.1;
		

		Entity m_Sphere;
		glm::vec3 m_SpherePosition{ -5.0f, 3.0f, 0.0f };
		glm::vec3 m_SphereRotationDegrees{ 0.0f };
		glm::vec3 m_SphereRotation{ 0.0f };
		glm::vec3 m_SphereSize{ 2.0f };
		glm::vec4 m_SphereColor{ 0.512f, 0.512f, .772f, 1.0f };
		float m_SphereSpecularStrength = 0.9f;
		float m_SphereAmbientStrength = 0.5f;
		float m_SphereDiffuseStrength = 1.0f;
		bool m_SphereIsTextured = false;


		Entity m_Cube;
		glm::vec3 m_CubePosition{ 5.0f, 3.0f, 0.0f };
		glm::vec3 m_CubeRotationDegrees{ 45.0f };
		glm::vec3 m_CubeRotation{ 0.0f };
		glm::vec3 m_CubeSize{ 3.5f };
		glm::vec4 m_CubeColor{ 0.7f, 0.0f, 0.1f, 1.0f };
		float m_CubeSpecularStrength = 0.8f;
		float m_CubeAmbientStrength = 0.5f;
		float m_CubeDiffuseStrength = 1.0f;
		bool m_CubeIsTextured = false;

		Entity m_Plane;
		glm::vec3 m_PlanePosition{ 0.0f, -2.0f, -30.0f };
		glm::vec3 m_PlaneRotationDegrees{ 0.0f };
		glm::vec3 m_PlaneRotation{ 0.0f };
		glm::vec3 m_PlaneSize{ 100.0f, 0.01f, 100.0f };
		glm::vec4 m_PlaneColor{ .65f, 0.71f, 0.80f, 1.0f };
		float m_PlaneSpecularStrength = 0.8f;
		float m_PlaneAmbientStrength = 0.2f;
		float m_PlaneDiffuseStrength = 1.0f;
		bool m_PlaneIsTextured = false;

		Entity m_DirectionalLight;
		bool m_LightSpin = false;
		glm::vec3 m_LightPosition{ 0.0f, 75.0f, 0.0f };
		glm::vec3 m_LightSize{ 0.5f };
		glm::vec3 m_LightRotationDegrees{ 0.0f };
		glm::vec3 m_LightRotation{ 0.0f };
		glm::vec4 m_LightColor{ 1.0f };


		EditorCamera m_Camera;
		glm::vec2 m_ViewportSize{ 0.0f };
		glm::vec2 m_ViewportBounds[2];
		glm::vec4 m_ClearColor{ 0.1f, 0.1f, 0.1f, 1.0f };


		Ref<Texture2D> m_TestTexture;
		Ref<Scene> m_Scene;
		Ref<Framebuffer> m_Framebuffer;
		Ref<FBO> m_TestFBO;
		float m_ElapsedTime = 0.0f;
	};
}