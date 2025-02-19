#include "imgui/imgui.h"
#include <Engine/Components/SpriteRenderer.hpp>
#include <Engine/Scene.hpp>
#include <iostream>
#include <Editor/GUI/MainGUI.hpp>
#include <icons/icons.h>
#include <imgui/imgui_stdlib.h>

using namespace VaultRenderer;
namespace Engine::Components {
    void SpriteRenderer::Init() {
        transform = &Scene::Main->EntityRegistry.get<Transform>(entity);

        std::vector<Vertex> vertices = {
            Vertex{glm::vec3(-0.5f, -0.5f, 0.0f), glm::vec2(0.0f, 0.0f), glm::vec3(0, 1, 0)},
            Vertex{glm::vec3(0.5f, -0.5f, 0.0f), glm::vec2(1.0f, 0.0f), glm::vec3(0, 1, 0)},
            Vertex{glm::vec3(0.5f, 0.5f, 0.0f), glm::vec2(1.0f, 1.0f), glm::vec3(0, 1, 0)},
            Vertex{glm::vec3(-0.5f, 0.5f, 0.0f), glm::vec2(0.0f, 1.0f), glm::vec3(0, 1, 0)} //
        };
        std::vector<uint32_t> indices = {0, 1, 2, 0, 2, 3};
        mesh = std::make_unique<Mesh>(vertices, indices);
        mesh->material.roughness = 1;
        mesh->material.metallic = 0;
    }

    void SpriteRenderer::Draw(VaultRenderer::Shader &shader) {
        if (mesh)
            mesh->Draw(shader);
    }

    void SpriteRenderer::OnGUI() {
        DrawComponent<SpriteRenderer>(Scene::Main->EntityRegistry, 40, [&] {
            ImGui::ColorEdit4("Color", &mesh->material.color.x);
            ImGui::ColorEdit3("Emission Color", &mesh->material.emissionColor.x, ImGuiColorEditFlags_HDR);

            ImGui::Text("Sprite");
            ImGui::ImageButton((mesh->material.diffuse ? reinterpret_cast<void *>(mesh->material.diffuse->texture_data->ID) : 0), ImVec2(100, 100), ImVec2(0, 1), ImVec2(1, 0));
            if (ImGui::IsItemClicked(1)) {
                mesh->material.diffuse.reset();
            }
            if (ImGui::BeginDragDropTarget()) {
                if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("image_file")) {
                    std::string path = (char *)payload->Data;
                    mesh->material.SetDiffuse(path);
                }
            }

            // ImGui::Button("Drag Custom Shader");
            // if (custom_shader_path != "") ImGui::Text("Custom Shader: %s", custom_shader_path.c_str());
            // if (ImGui::BeginDragDropTarget()) {
            //     if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("shader")) {
            //         custom_shader_path = (char *)payload->Data;
            //         custom_shader = std::make_unique<VaultRenderer::Shader>(custom_shader_path);
            //     }
            // }
        });
    }
} // namespace Engine::Components