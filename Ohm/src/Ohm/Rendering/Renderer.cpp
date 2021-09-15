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

	Renderer::Statistics Renderer::s_Stats;

	struct RenderData
	{
		Ref<VertexArray> VAO;
		Ref<Shader> Shader;
		Ref<Texture2D> Texture;
		Ref<Texture2D> WhiteTexture;


		struct CameraData
		{
			glm::mat4 u_ViewProjectionMatrix;
			glm::mat4 u_ModelMatrix;
			glm::mat4 u_ProjectionMatrix;
			glm::mat4 u_ViewMatrix;
			glm::mat4 u_NormalMatrix;
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
		s_RenderData->Texture->Bind(1);
		s_RenderData->CameraBuffer = CreateRef<UniformBuffer>(sizeof(RenderData::CameraData), 0);
	}

	void Renderer::BeginScene()
	{
		s_Stats.Clear();
	}

	void Renderer::BeginPass(const Ref<RenderPass>& renderPass)
	{
		auto& specification = renderPass->GetRenderPassSpecification();
		Ref<Framebuffer> passFB = renderPass->GetRenderPassSpecification().TargetFramebuffer;
		RenderCommand::SetViewport(passFB->GetFrameBufferSpecification().Width, passFB->GetFrameBufferSpecification().Height);
		passFB->Bind();
		RenderCommand::ClearColor(specification.ClearColor.r, specification.ClearColor.g, specification.ClearColor.b, specification.ClearColor.a);
		RenderCommand::Clear(specification.ColorWrite, specification.DepthRead);
	}

	void Renderer::DrawMeshWithMaterial(const EditorCamera& camera, MeshRendererComponent& meshRenderer, const TransformComponent& transform)
	{
		s_RenderData->VAO->Bind();

		meshRenderer.MeshData->Bind();
		s_RenderData->VAO->EnableVertexAttributes(meshRenderer.MeshData->GetVertexBuffer());
		meshRenderer.MaterialInstance->UploadStagedUniforms();
	
		if (meshRenderer.MaterialInstance->GetShader()->GetName() == "Phong")
		{
			meshRenderer.MaterialInstance->GetShader()->UploadUniformInt("sampler_ShadowMap", 2);
		}



		glm::mat4 modelView = camera.GetView() * transform.Transform();
		glm::mat4 normalMatrix = glm::transpose(glm::inverse(modelView));
		glm::mat4 viewProjection = camera.GetProjectionView();

		RenderData::CameraData cameraData{ viewProjection, transform.Transform(), camera.GetProjection(), camera.GetView(), normalMatrix };
		s_RenderData->CameraBuffer->SetData(&cameraData, sizeof(RenderData::CameraData));

		RenderCommand::DrawIndexed(s_RenderData->VAO, meshRenderer.MeshData->GetIndexBuffer()->GetCount());

		s_RenderData->VAO->Unbind();
		meshRenderer.MaterialInstance->GetShader()->Unbind();
		meshRenderer.MeshData->Unbind();

		s_Stats.TriangleCount += meshRenderer.MeshData->GetIndices().size() / 3;
		s_Stats.VertexCount += meshRenderer.MeshData->GetVertices().size();
	}

	void Renderer::DrawGeometry(MeshRendererComponent& meshRenderer)
	{
		s_RenderData->VAO->Bind();
		meshRenderer.MeshData->Bind();
		s_RenderData->VAO->EnableVertexAttributes(meshRenderer.MeshData->GetVertexBuffer());

		RenderCommand::DrawIndexed(s_RenderData->VAO, meshRenderer.MeshData->GetIndexBuffer()->GetCount());

		s_RenderData->VAO->Unbind();
		meshRenderer.MeshData->Unbind();
	}

	void Renderer::DrawFullScreenQuad()
	{
		s_RenderData->VAO->Bind();
		Ref<Mesh> quad(Mesh::CreatePrimitive(Primitive::Quad));
		quad->Bind();
		s_RenderData->VAO->EnableVertexAttributes(quad->GetVertexBuffer());

		RenderCommand::DrawIndexed(s_RenderData->VAO, quad->GetIndexBuffer()->GetCount());

		s_RenderData->VAO->Unbind();
		quad->Unbind();
	}

	void Renderer::EndPass(const Ref<RenderPass>& renderPass)
	{
		renderPass->GetRenderPassSpecification().TargetFramebuffer->Unbind();
	}

	void Renderer::EndScene()
	{

	}

	void Renderer::Shutdown()
	{
		delete s_RenderData;
	}
}