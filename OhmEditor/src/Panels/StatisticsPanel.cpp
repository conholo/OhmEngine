#include "StatisticsPanel.h"
#include "Ohm/Rendering/Renderer.h"
#include "imgui/imgui.h"

namespace Ohm
{
    namespace UI
    {
        void StatisticsPanel::Draw()
        {
            ImGui::Begin("Statistics");

            const Renderer::Statistics RenderStats = Renderer::GetStats();

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::Text("Vertex Count: %d", RenderStats.VertexCount);
            ImGui::Text("Triangle Count: %d", RenderStats.TriangleCount);
            ImGui::End();
        }
    }
}

