#include "ohmpch.h"
#include "Ohm/Rendering/Renderer.h"

#include "Ohm/Rendering/VertexArray.h"
#include "Ohm/Rendering/Shader.h"

#include "Ohm/Rendering/Vertex.h"
#include "Ohm/Rendering/RenderCommand.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Ohm
{
	struct RenderData
	{
		Ref<VertexArray> VAO;
		Ref<Shader> Shader;
	};

	static RenderData* s_RenderData = nullptr;


	void Renderer::Initialize()
	{
		s_RenderData = new RenderData();

		s_RenderData->VAO = CreateRef<VertexArray>();
	}

	void Renderer::DrawMesh(const EditorCamera& camera, const MeshRendererComponent& meshRenderer, const TransformComponent& transform)
	{
		s_RenderData->VAO->Bind();
		s_RenderData->Shader = meshRenderer.MaterialShader;

		s_RenderData->VAO->EnableVertexAttributes(meshRenderer.MeshData->GetVertexBuffer());
		meshRenderer.MeshData->GetVertexBuffer()->Bind();

		s_RenderData->Shader->Bind();
		s_RenderData->Shader->UploadUniformFloat3("u_CameraPosition", camera.Position());

		// Light position needs to be in view space.
		glm::vec3 lightPosition = glm::vec3(2.0f, 5.0f, 0.0f);
		glm::vec3 lightRotation = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::vec3 lightScale = glm::vec3(1.0f);

		glm::mat4 lightTransform =
			glm::translate(glm::mat4(1.0f), lightPosition) *
			glm::toMat4(glm::quat(lightRotation)) *
			glm::scale(glm::mat4(1.0f), lightScale);

		glm::vec3 toViewSpaceLightPosition = glm::vec3(camera.GetView() * lightTransform * glm::vec4(0.0, 0.0, 0.0, 1.0f));

		glm::mat4 modelView = transform.Transform() * camera.GetView();
		// Need to work with normals in view space to handle non-uniform scaling.
		glm::mat4 normalMatrix = glm::transpose(glm::inverse(modelView));

		s_RenderData->Shader->UploadUniformFloat3("u_LightPosition", toViewSpaceLightPosition);
		s_RenderData->Shader->UploadUniformFloat4("u_Color", meshRenderer.Color);
		s_RenderData->Shader->UploadUniformMat4("u_NormalMatrix", normalMatrix);
		s_RenderData->Shader->UploadUniformMat4("u_PVM", camera.GetProjectionView() * transform.Transform());

		RenderCommand::DrawIndexed(s_RenderData->VAO, meshRenderer.MeshData->GetIndexBuffer()->GetCount());
	}

	void Renderer::Shutdown()
	{
		delete s_RenderData;
	}
}