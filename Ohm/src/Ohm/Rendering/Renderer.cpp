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
		Ref<Mesh> FullScreenQuad;
		RenderFlag Flags;

		struct GlobalData
		{
			float ElapsedTime;
			float DeltaTime;
		};

		struct CameraData
		{
			glm::vec4 CameraPosition;
			glm::mat4 ViewProjectionMatrix;
			glm::mat4 ModelMatrix;
			glm::mat4 ProjectionMatrix;
			glm::mat4 ViewMatrix;
			glm::mat4 NormalMatrix;
		};

		Ref<UniformBuffer> CameraBuffer;
		Ref<UniformBuffer> GlobalBuffer;
	};

	static RenderData* s_RenderData = nullptr;


	void Renderer::Initialize()
	{
		s_RenderData = new RenderData();

		s_RenderData->VAO = CreateRef<VertexArray>();

		Texture2DSpecification whiteSpec =
		{
			"White Texture",
			TextureUtils::WrapMode::Repeat,
			TextureUtils::FilterMode::Linear,
			TextureUtils::FilterMode::Linear,
			TextureUtils::ImageInternalFormat::RGBA8,
			TextureUtils::ImageDataLayout::RGBA,
			TextureUtils::ImageDataType::UByte,
			1, 1
		};

		s_RenderData->FullScreenQuad = Mesh::CreatePrimitive(Primitive::FullScreenQuad);

		s_RenderData->WhiteTexture = TextureLibrary::Load(whiteSpec);
		uint32_t whiteTextureData = 0xffffffff;
		s_RenderData->WhiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));

		s_RenderData->CameraBuffer = CreateRef<UniformBuffer>(sizeof(RenderData::CameraData), 0);
		s_RenderData->GlobalBuffer = CreateRef<UniformBuffer>(sizeof(RenderData::GlobalBuffer), 3);

		ShaderLibrary::Load("assets/shaders/Phong.shader");
		ShaderLibrary::Load("assets/shaders/ShadowMap.shader");
		ShaderLibrary::Load("assets/shaders/DepthMap.shader");
		ShaderLibrary::Load("assets/shaders/flatcolor.shader");
		ShaderLibrary::Load("assets/shaders/VertexDeformation.shader");
		ShaderLibrary::Load("assets/shaders/SceneComposite.shader");
		ShaderLibrary::Load("assets/shaders/Bloom.shader");
	}

	void Renderer::UploadCameraUniformData(const EditorCamera& camera, const TransformComponent& transform)
	{
		glm::mat4 normalMatrix = glm::transpose(glm::inverse(transform.Transform()));
		glm::mat4 viewProjection = camera.GetProjectionView();

		RenderData::CameraData cameraData{ glm::vec4(camera.GetPosition(), 1.0), viewProjection, transform.Transform(), camera.GetProjection(), camera.GetView(), normalMatrix };
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
		s_RenderData->FullScreenQuad->Bind();
		s_RenderData->VAO->EnableVertexAttributes(s_RenderData->FullScreenQuad->GetVertexBuffer());

		RenderCommand::DrawIndexed(s_RenderData->VAO, s_RenderData->FullScreenQuad->GetIndexBuffer()->GetCount());

		s_RenderData->VAO->Unbind();
		s_RenderData->FullScreenQuad->Unbind();
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