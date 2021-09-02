#include "ohmpch.h"
#include "Ohm/Rendering/Renderer.h"

#include "Ohm/Rendering/VertexArray.h"
#include "Ohm/Rendering/Shader.h"

#include "Ohm/Rendering/Vertex.h"
#include "Ohm/Rendering/RenderCommand.h"
#include "Ohm/Rendering/Texture2D.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Ohm
{
	struct RenderData
	{
		Ref<VertexArray> VAO;
		Ref<Shader> Shader;
		Ref<Texture2D> Texture;
		Ref<Texture2D> WhiteTexture;
	};

	static RenderData* s_RenderData = nullptr;


	void Renderer::Initialize()
	{
		s_RenderData = new RenderData();

		s_RenderData->VAO = CreateRef<VertexArray>();

		s_RenderData->WhiteTexture = CreateRef<Texture2D>(1, 1);
		uint32_t whiteTextureData = 0xffffffff;
		s_RenderData->WhiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));

		s_RenderData->Texture = CreateRef<Texture2D>("assets/textures/tex.jpg");
		s_RenderData->WhiteTexture->Bind(0);
	}

	void Renderer::DrawMesh(const EditorCamera& camera, const MeshRendererComponent& meshRenderer, const TransformComponent& transform, const TransformComponent& lightTransform)
	{
		s_RenderData->VAO->Bind();
		s_RenderData->Shader = meshRenderer.MaterialShader;

		s_RenderData->VAO->EnableVertexAttributes(meshRenderer.MeshData->GetVertexBuffer());

		meshRenderer.MeshData->Bind();
		s_RenderData->Shader->Bind();

		// Light Position needs to be in view space (modelView * local light position)
		glm::vec3 toViewSpaceLightPosition = glm::vec3(camera.GetView() * lightTransform.Transform() * glm::vec4(0.0, 0.0, 0.0, 1.0f));

		glm::mat4 modelView = camera.GetView() * transform.Transform();
		// Need to work with normals in view space to handle non-uniform scaling.
		glm::mat4 normalMatrix = glm::transpose(glm::inverse(modelView));

		s_RenderData->Shader->UploadUniformFloat("u_SpecularStrength", 0.8f);
		s_RenderData->Shader->UploadUniformFloat("u_AmbientStrength", 0.1f);

		s_RenderData->Shader->UploadUniformFloat3("u_LightPosition", toViewSpaceLightPosition);

		s_RenderData->Shader->UploadUniformFloat4("u_Color", meshRenderer.Color);
		s_RenderData->Shader->UploadUniformFloat4("u_LightColor", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
		s_RenderData->Shader->UploadUniformMat4("u_NormalMatrix", normalMatrix);
		s_RenderData->Shader->UploadUniformMat4("u_ModelView", modelView);
		s_RenderData->Shader->UploadUniformMat4("u_ProjectionMatrix", camera.GetProjection());

		RenderCommand::DrawIndexed(s_RenderData->VAO, meshRenderer.MeshData->GetIndexBuffer()->GetCount());

		s_RenderData->VAO->Unbind();
		s_RenderData->Shader->Unbind();
		meshRenderer.MeshData->Unbind();
	}

	void Renderer::Shutdown()
	{
		delete s_RenderData;
	}
}