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

		std::unordered_map<Primitive, GeometryData> primitives;

		GeometryData quadData(
			{
				Vertex{ {-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f} },
				Vertex{ { 0.5f, -0.5f, 0.0f}, {1.0f, 0.0f} },
				Vertex{ { 0.5f,  0.5f, 0.0f}, {1.0f, 1.0f} },
				Vertex{ {-0.5f,  0.5f, 0.0f}, {0.0f, 1.0f} },
			},
			{ 0, 1, 2, 2, 3, 0 }
		);

		primitives[Primitive::Quad] = quadData;


		s_RenderData->VAO = CreateRef<VertexArray>();
		s_RenderData->Shader = CreateRef<Shader>("assets/shaders/flatcolor.shader");

		float* basePtr = &(*quadData.Vertices.data()).VertexPosition.x;
		Ref<VertexBuffer> vertexBuffer = CreateRef<VertexBuffer>(basePtr, sizeof(Vertex) * quadData.Vertices.size());

		BufferLayout layout(
			{
				{ "a_Position", ShaderDataType::Float3 },
				{ "a_TexCoord", ShaderDataType::Float2 }
			}
		);

		vertexBuffer->SetLayout(layout);

		uint32_t* indexPtr = quadData.Indices.data();
		Ref<IndexBuffer> indexBuffer = CreateRef<IndexBuffer>(indexPtr, quadData.Indices.size());

		s_RenderData->VAO->AddVertexBuffer(vertexBuffer);
		s_RenderData->VAO->SetIndexBuffer(indexBuffer);

	}

	void Renderer::BeginScene(const EditorCamera& camera, Primitive primitve)
	{
		s_RenderData->Shader->Bind();
		s_RenderData->Shader->UploadUniformMat4("u_PV", camera.GetProjectionView());
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
	}

	void Renderer::Shutdown()
	{
		delete s_RenderData;
	}
}