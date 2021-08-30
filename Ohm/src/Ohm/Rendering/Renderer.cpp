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

	void Renderer::BeginScene(const EditorCamera& camera, const MeshRendererComponent& meshRenderer)
	{
		s_RenderData->VAO->Bind();
		s_RenderData->Shader = meshRenderer.MaterialShader;

		std::vector<Vertex> vertices = meshRenderer.MeshData->GetVertices();
		float* basePtr = &vertices.data()->VertexPosition.x;


		Ref<VertexBuffer> vertexBuffer = CreateRef<VertexBuffer>(basePtr, sizeof(Vertex) * meshRenderer.MeshData->GetVertices().size());

		BufferLayout layout(
			{
				{ "a_Position", ShaderDataType::Float3 },
				{ "a_TexCoord", ShaderDataType::Float2 }
			}
		);

		vertexBuffer->SetLayout(layout);

		uint32_t* indexPtr = meshRenderer.MeshData->GetIndices().data();
		Ref<IndexBuffer> indexBuffer = CreateRef<IndexBuffer>(indexPtr, meshRenderer.MeshData->GetIndices().size());

		s_RenderData->VAO->AddVertexBuffer(vertexBuffer);
		s_RenderData->VAO->SetIndexBuffer(indexBuffer);

		s_RenderData->Shader->Bind();
		s_RenderData->Shader->UploadUniformFloat4("u_Color", meshRenderer.Color);
		s_RenderData->Shader->UploadUniformMat4("u_ProjectionView", camera.GetProjectionView());
	}

	void Renderer::UploadModelData(const glm::vec3& position, const glm::vec3& size)
	{
		glm::mat4 translation = glm::translate(glm::mat4(1.0f), position);
		glm::mat4 scale = glm::scale(glm::mat4(1.0f), size);

		glm::mat4 transform = translation * scale;

		UploadModelData(transform);
	}

	void Renderer::UploadModelData(const glm::mat4& transform)
	{
		s_RenderData->Shader->UploadUniformMat4("u_Model", transform);
	}

	void Renderer::EndScene()
	{
		RenderCommand::DrawIndexed(s_RenderData->VAO);
		s_RenderData->VAO->Flush();
	}

	void Renderer::Shutdown()
	{
		delete s_RenderData;
	}
}