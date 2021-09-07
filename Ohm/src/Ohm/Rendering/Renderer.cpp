#include "ohmpch.h"
#include "Ohm/Rendering/Renderer.h"

#include "Ohm/Rendering/VertexArray.h"
#include "Ohm/Rendering/Shader.h"

#include "Ohm/Rendering/Vertex.h"
#include "Ohm/Rendering/RenderCommand.h"
#include "Ohm/Rendering/Texture2D.h"
#include "Ohm/Rendering/UniformBuffer.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Ohm
{
	struct RenderData
	{
		Ref<VertexArray> VAO;
		Ref<Shader> Shader;
		Ref<Texture2D> Texture;
		Ref<Texture2D> WhiteTexture;

		struct CameraData
		{
			glm::mat4 ModelViewMatrix;
			glm::mat4 ProjectionMatrix;
			glm::mat4 NormalMatrix;
		};

		Ref<UniformBuffer> CameraBuffer;
	};

	static RenderData* s_RenderData = nullptr;


	void Renderer::Initialize()
	{
		s_RenderData = new RenderData();

		s_RenderData->VAO = CreateRef<VertexArray>();

		s_RenderData->WhiteTexture = CreateRef<Texture2D>(1, 1);
		uint32_t whiteTextureData = 0xffffffff;
		s_RenderData->WhiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));

		s_RenderData->Texture = CreateRef<Texture2D>("assets/textures/lava.jpg");
		s_RenderData->WhiteTexture->Bind(0);
		//s_RenderData->Texture->Bind(0);
		s_RenderData->CameraBuffer = CreateRef<UniformBuffer>(sizeof(RenderData::CameraData), 0);
	}

	void Renderer::DrawMesh(const EditorCamera& camera, const MeshRendererComponent& meshRenderer, const TransformComponent& transform)
	{
		s_RenderData->VAO->Bind();
		meshRenderer.MaterialInstance->GetShader()->Bind();
		meshRenderer.MaterialInstance->GetShader()->UploadUniformFloat4("u_Color", meshRenderer.Color);
		meshRenderer.MaterialInstance->GetShader()->UploadUniformFloat("u_SpecularStrength", 0.8f);
		meshRenderer.MaterialInstance->GetShader()->UploadUniformFloat("u_AmbientStrength", 0.1f);

		meshRenderer.MeshData->Bind();
		s_RenderData->VAO->EnableVertexAttributes(meshRenderer.MeshData->GetVertexBuffer());

		glm::mat4 modelView = camera.GetView() * transform.Transform();
		glm::mat4 normalMatrix = glm::transpose(glm::inverse(modelView));

		RenderData::CameraData cameraData{ modelView, camera.GetProjection(), normalMatrix };
		s_RenderData->CameraBuffer->SetData(&cameraData, sizeof(RenderData::CameraData));


		RenderCommand::DrawIndexed(s_RenderData->VAO, meshRenderer.MeshData->GetIndexBuffer()->GetCount());

		s_RenderData->VAO->Unbind();
		meshRenderer.MaterialInstance->GetShader()->Unbind();
		meshRenderer.MeshData->Unbind();
	}

	void Renderer::Shutdown()
	{
		delete s_RenderData;
	}
}