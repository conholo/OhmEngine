#include "ohmpch.h"
#include "Ohm/Rendering/Renderer.h"

#include "Ohm/Rendering/VertexArray.h"
#include "Ohm/Rendering/Shader.h"

#include "Ohm/Rendering/Vertex.h"
#include "Ohm/Rendering/RenderCommand.h"
#include "Ohm/Rendering/Texture2D.h"
#include "Ohm/Rendering/UniformBuffer.h"
#include "Ohm/Core/Time.h"

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
		RenderFlag Flags;

		struct GlobalData
		{
			float ElapsedTime;
			float DeltaTime;
		};

		struct CameraData
		{
			glm::mat4 u_ViewProjectionMatrix;
			glm::mat4 u_ModelMatrix;
			glm::mat4 u_ProjectionMatrix;
			glm::mat4 u_ViewMatrix;
			glm::mat4 u_NormalMatrix;
		};

		Ref<UniformBuffer> CameraBuffer;
		Ref<UniformBuffer> GlobalBuffer;
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

		s_RenderData->GlobalBuffer = CreateRef<UniformBuffer>(sizeof(RenderData::GlobalBuffer), 3);

		ShaderLibrary::Load("assets/shaders/Phong.shader");
		ShaderLibrary::Load("assets/shaders/PreDepth.shader");
		ShaderLibrary::Load("assets/shaders/DepthMap.shader");
		ShaderLibrary::Load("assets/shaders/flatcolor.shader");
		ShaderLibrary::Load("assets/shaders/VertexDeformation.shader");
	}

	void Renderer::UploadCameraUniformData(const EditorCamera& camera, const TransformComponent& transform)
	{
		glm::mat4 modelView = camera.GetView() * transform.Transform();
		glm::mat4 normalMatrix = glm::transpose(glm::inverse(modelView));
		glm::mat4 viewProjection = camera.GetProjectionView();

		RenderData::CameraData cameraData{ viewProjection, transform.Transform(), camera.GetProjection(), camera.GetView(), normalMatrix };
		s_RenderData->CameraBuffer->SetData(&cameraData, sizeof(RenderData::CameraData));
	}

	void Renderer::BeginScene()
	{
		float globalTimeValues[2] = { Time::Elapsed(), Time::DeltaTime() };

		s_RenderData->GlobalBuffer->SetData((void*)globalTimeValues, sizeof(float) * 2);
		s_Stats.Clear();
	}

	void Renderer::BeginPass(const Ref<RenderPass>& renderPass)
	{
		RenderCommand::SetFlags(renderPass->GetRenderPassSpecification().Flags);
		auto& specification = renderPass->GetRenderPassSpecification();
		Ref<Framebuffer> passFB = renderPass->GetRenderPassSpecification().TargetFramebuffer;
		passFB->Bind();
		RenderCommand::ClearColor(specification.ClearColor.r, specification.ClearColor.g, specification.ClearColor.b, specification.ClearColor.a);
		RenderCommand::Clear(specification.ColorWrite, specification.DepthRead);
	}

	void Renderer::DrawMesh(const EditorCamera& camera, const Ref<Mesh>& mesh, const Ref<Material>& material, const TransformComponent& transform)
	{
		s_RenderData->VAO->Bind();
		mesh->Bind();
		s_RenderData->VAO->EnableVertexAttributes(mesh->GetVertexBuffer());
		material->UploadStagedUniforms();
		
		UploadCameraUniformData(camera, transform);

		RenderCommand::DrawIndexed(s_RenderData->VAO, mesh->GetIndexBuffer()->GetCount());

		s_RenderData->VAO->Unbind();
		material->GetShader()->Unbind();
		mesh->Unbind();

		s_Stats.TriangleCount += mesh->GetIndices().size() / 3;
		s_Stats.VertexCount += mesh->GetVertices().size();
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