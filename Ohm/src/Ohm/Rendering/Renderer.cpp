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
		glm::vec3 LightPosition = glm::vec3(3.0f, 10.0f, 0.0f);
	};

	static RenderData* s_RenderData = nullptr;


	void Renderer::Initialize()
	{
		s_RenderData = new RenderData();

		s_RenderData->VAO = CreateRef<VertexArray>();
	}

	void Renderer::DrawMesh(const EditorCamera& camera, const MeshRendererComponent& meshRenderer, const TransformComponent& transform, const TransformComponent& lightTransform)
	{
		s_RenderData->VAO->Bind();
		s_RenderData->Shader = meshRenderer.MaterialShader;

		s_RenderData->VAO->EnableVertexAttributes(meshRenderer.MeshData->GetVertexBuffer());
		meshRenderer.MeshData->GetVertexBuffer()->Bind();

		s_RenderData->Shader->Bind();
		s_RenderData->Shader->UploadUniformFloat3("u_CameraPosition", camera.Position());

		glm::vec3 toViewSpaceLightPosition = glm::vec3(camera.GetView() * lightTransform.Transform() * glm::vec4(0.0, 0.0, 0.0, 1.0f));

		glm::mat4 modelView = camera.GetView() * transform.Transform();
		// Need to work with normals in view space to handle non-uniform scaling.
		glm::mat4 normalMatrix = glm::transpose(glm::inverse(modelView));

		s_RenderData->Shader->UploadUniformFloat3("u_LightPosition", toViewSpaceLightPosition);
		s_RenderData->Shader->UploadUniformFloat4("u_Color", meshRenderer.Color);
		s_RenderData->Shader->UploadUniformMat4("u_NormalMatrix", normalMatrix);
		s_RenderData->Shader->UploadUniformMat4("u_ModelView", modelView);
		s_RenderData->Shader->UploadUniformMat4("u_ProjectionMatrix", camera.GetProjection());

		RenderCommand::DrawIndexed(s_RenderData->VAO, meshRenderer.MeshData->GetIndexBuffer()->GetCount());
	}

	void Renderer::Shutdown()
	{
		delete s_RenderData;
	}
}