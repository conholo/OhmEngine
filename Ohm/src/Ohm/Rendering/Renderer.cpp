#include "ohmpch.h"
#include "Ohm/Rendering/Renderer.h"
#include "Ohm/Rendering/Shader.h"
#include "Ohm/Rendering/RenderCommand.h"
#include "Ohm/Rendering/Texture2D.h"
#include "Ohm/Rendering/UniformBuffer.h"
#include "Ohm/Core/Time.h"


#include "TextureLibrary.h"
#include "Ohm/Core/Application.h"
#include "Ohm/Scene/Entity.h"

namespace Ohm
{
	Renderer::Statistics Renderer::s_Stats;

	struct RenderData
	{
		std::unordered_map<Primitive, Ref<Mesh>> Primitives;

		struct GlobalData
		{
			float ElapsedTime;
			float DeltaTime;
		};

		struct CameraData
		{
			glm::vec4 CameraPosition;
			glm::mat4 ViewProjectionMatrix;
			glm::mat4 ProjectionMatrix;
			glm::mat4 ViewMatrix;
		};

		struct SceneData
		{
			glm::vec3 Radiance {1.0f};
			float Intensity {1.0f};
			glm::vec3 Direction {0.0f, -1.0f, 0.0f};
			float ShadowAmount {1.0f};
		};

		struct EntityData
		{
			glm::mat4 ModelMatrix;
		};
		
		Ref<UniformBuffer> CameraBuffer;
		Ref<UniformBuffer> GlobalBuffer;
		Ref<UniformBuffer> SceneBuffer;
		Ref<UniformBuffer> EntityBuffer;

		std::unordered_map<std::string, uint32_t> s_UniformBufferBindingMap =
		{
			{TypeName<GlobalData>(),				0},
			{TypeName<CameraData>(),				1},
			{TypeName<SceneData>(),				2},
			{TypeName<EntityData>(),				3},
		};
	};

	static RenderData* s_RenderData = nullptr;

	void Renderer::UploadGlobalData()
	{
		const float GlobalTimeValues[2] = { Time::Elapsed(), Time::DeltaTime() };
		s_RenderData->GlobalBuffer->SetData(GlobalTimeValues, sizeof(RenderData::GlobalData));
	}

	void Renderer::UploadCameraData(const EditorCamera& Camera)
	{
		const glm::mat4 ViewProjectionMatrix = Camera.GetViewProjection();
		const glm::mat4 ProjectionMatrix = Camera.GetProjection();
		const glm::mat4 ViewMatrix = Camera.GetView();

		const RenderData::CameraData CameraData
		{
			glm::vec4(Camera.GetPosition(), 1.0f),
			ViewProjectionMatrix,
			ProjectionMatrix,
			ViewMatrix
		};
		s_RenderData->CameraBuffer->SetData(&CameraData, sizeof(RenderData::CameraData));
	}

	void Renderer::UploadSceneData(const Ref<Scene>& Scene)
	{
		Entity dirLightEntity = Scene->GetDirectionalLight();
		const DirectionalLightComponent& dirLight = dirLightEntity.GetComponent<DirectionalLightComponent>();

		const RenderData::SceneData sceneData
		{
			dirLight.Radiance,
			dirLight.Intensity,
			dirLight.LightDirection,
			dirLight.ShadowAmount,
		};

		s_RenderData->SceneBuffer->SetData(&sceneData, sizeof(RenderData::SceneData));
	}

	void Renderer::UploadPerEntityData(const TransformComponent& transform)
	{
		const RenderData::EntityData data
		{
			transform.Transform()
		};
		s_RenderData->EntityBuffer->SetData(&data, sizeof(RenderData::EntityData));
	}

	void Renderer::Initialize()
	{
		s_RenderData = new RenderData();

		uint32_t globalSlot = s_RenderData->s_UniformBufferBindingMap[TypeName<RenderData::GlobalData>()];
		s_RenderData->GlobalBuffer = CreateRef<UniformBuffer>(sizeof(RenderData::GlobalData), globalSlot);
		
		uint32_t cameraSlot = s_RenderData->s_UniformBufferBindingMap[TypeName<RenderData::CameraData>()];
		s_RenderData->CameraBuffer = CreateRef<UniformBuffer>(sizeof(RenderData::CameraData), cameraSlot);

		uint32_t sceneSlot = s_RenderData->s_UniformBufferBindingMap[TypeName<RenderData::SceneData>()];
		s_RenderData->SceneBuffer = CreateRef<UniformBuffer>(sizeof(RenderData::SceneData), sceneSlot);

		uint32_t entitySlot = s_RenderData->s_UniformBufferBindingMap[TypeName<RenderData::EntityData>()];
		s_RenderData->EntityBuffer = CreateRef<UniformBuffer>(sizeof(RenderData::EntityData), entitySlot);

		s_RenderData->Primitives[Primitive::Cube] = MeshFactory::Create(Primitive::Cube);
		s_RenderData->Primitives[Primitive::Quad] = MeshFactory::Create(Primitive::Quad);
		s_RenderData->Primitives[Primitive::FullScreenQuad] = MeshFactory::Create(Primitive::FullScreenQuad);
		s_RenderData->Primitives[Primitive::Sphere] = MeshFactory::Create(Primitive::Sphere);
		s_RenderData->Primitives[Primitive::Plane] = MeshFactory::Create(Primitive::Plane);
		s_RenderData->Primitives[Primitive::Triangle] = MeshFactory::Create(Primitive::Triangle);
		s_RenderData->Primitives[Primitive::TessellatedQuad] = MeshFactory::Create(Primitive::TessellatedQuad);
		s_RenderData->Primitives[Primitive::Skybox] = MeshFactory::Create(Primitive::Skybox);
		s_RenderData->Primitives[Primitive::Icosphere] = MeshFactory::Create(Primitive::Icosphere);

		TextureLibrary::LoadWhiteTexture();
		TextureLibrary::LoadBlackTexture();
		TextureLibrary::LoadBlackTextureCube();

		TextureLibrary::LoadTexture2D( "assets/textures/BRDF_LUT.png");
		TextureLibrary::LoadTexture2D("assets/textures/lava.jpg");
		TextureLibrary::LoadTexture2D("assets/textures/uv.png");
		TextureLibrary::LoadTexture2D("assets/textures/space.jpg");
		TextureLibrary::LoadTexture2D("assets/textures/map.jpg");
		TextureLibrary::LoadTexture2D("assets/textures/ground-blue.jpg");

		ShaderLibrary::Load("assets/shaders/Phong.shader");
		ShaderLibrary::Load("assets/shaders/ShadowMap.shader");
		ShaderLibrary::Load("assets/shaders/DepthMap.shader");
		ShaderLibrary::Load("assets/shaders/LinearDepthVisualizer.shader");
		ShaderLibrary::Load("assets/shaders/Preetham.shader");
		ShaderLibrary::Load("assets/shaders/PBR.shader");
		ShaderLibrary::Load("assets/shaders/Skybox.shader");
		
		ShaderLibrary::Load("assets/shaders/EquirectangularToCubemap.shader");
		ShaderLibrary::Load("assets/shaders/EnvironmentMipFilter.shader");
		ShaderLibrary::Load("assets/shaders/EnvironmentIrradiance.shader");

		ShaderLibrary::Load("assets/shaders/flatcolor.shader");
		ShaderLibrary::Load("assets/shaders/VertexDeformation.shader");
		ShaderLibrary::Load("assets/shaders/SceneComposite.shader");
		ShaderLibrary::Load("assets/shaders/Bloom.shader");
	}

	void Renderer::BeginScene(const Ref<Scene>& scene, const EditorCamera& camera)
	{
		s_Stats.Clear();

		UploadGlobalData();
		UploadCameraData(camera);
		UploadSceneData(scene);
	}

	void Renderer::BeginPass(const Ref<RenderPass>& renderPass)
	{
		if (renderPass->GetRenderPassSpecification().Type != PassType::DefaultFBO)
		{
			const Ref<Framebuffer> PassFB = renderPass->GetRenderPassSpecification().TargetFramebuffer;
			PassFB->Bind();
		}
		else
			RenderCommand::SetViewport(Application::GetApplication().GetWindow().GetWidth(), Application::GetApplication().GetWindow().GetHeight());

		auto& specification = renderPass->GetRenderPassSpecification();
		RenderCommand::ClearColor(specification.ClearColor);
		RenderCommand::Clear(specification.ClearColorFlag, specification.ClearDepthFlag);
	}

	void Renderer::DrawPrimitive(const PrimitiveRendererComponent& primitive)
	{
		const auto& primitiveMesh = s_RenderData->Primitives[primitive.PrimitiveType];
		primitiveMesh->Bind();
		primitive.MaterialInstance->UploadStagedUniforms();
		RenderCommand::DrawIndexed(primitiveMesh->GetVAO());
		primitiveMesh->Unbind();
		s_Stats.TriangleCount += primitiveMesh->GetIndices().size() / 3;
		s_Stats.VertexCount += primitiveMesh->GetVertices().size();
	}

	void Renderer::DrawPrimitive(const PrimitiveRendererComponent& primitive, const Ref<Material>& material)
	{
		const auto& primitiveMesh = s_RenderData->Primitives[primitive.PrimitiveType];
		primitiveMesh->Bind();
		material->UploadStagedUniforms();
		RenderCommand::DrawIndexed(primitiveMesh->GetVAO());
		primitiveMesh->Unbind();
		s_Stats.TriangleCount += primitiveMesh->GetIndices().size() / 3;
		s_Stats.VertexCount += primitiveMesh->GetVertices().size();
	}

	void Renderer::DrawFullScreenQuad(const Ref<Material>& Material)
	{
		s_RenderData->Primitives[Primitive::FullScreenQuad]->Bind();
		Material->UploadStagedUniforms();
		RenderCommand::DrawIndexed(s_RenderData->Primitives[Primitive::FullScreenQuad]->GetVAO());
		s_RenderData->Primitives[Primitive::FullScreenQuad]->Unbind();
		s_Stats.TriangleCount += s_RenderData->Primitives[Primitive::FullScreenQuad]->GetIndices().size() / 3;
		s_Stats.VertexCount += s_RenderData->Primitives[Primitive::FullScreenQuad]->GetVertices().size();
	}

	void Renderer::DrawSkybox(const Ref<Material>& SkyboxMaterial)
	{
		s_RenderData->Primitives[Primitive::Skybox]->Bind();
		SkyboxMaterial->UploadStagedUniforms();

		RenderCommand::SetDepthFlag(DepthFlag::LEqual);
		RenderCommand::DrawIndexed(s_RenderData->Primitives[Primitive::Skybox]->GetVAO());
		RenderCommand::SetDepthFlag(DepthFlag::Less);

		s_RenderData->Primitives[Primitive::Skybox]->Unbind();
		s_Stats.TriangleCount += s_RenderData->Primitives[Primitive::Skybox]->GetIndices().size() / 3;
		s_Stats.VertexCount += s_RenderData->Primitives[Primitive::Skybox]->GetVertices().size();
	}

	void Renderer::EndPass(const Ref<RenderPass>& renderPass)
	{
		if (renderPass->GetRenderPassSpecification().Type == PassType::DefaultFBO) return;
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
