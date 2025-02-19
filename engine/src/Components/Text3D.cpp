#include <Engine/Components/Text3D.hpp>
#include <Engine/Scene.hpp>
#include <iostream>
#include <Editor/GUI/MainGUI.hpp>
#include <icons/icons.h>
#include <imgui/imgui_stdlib.h>

namespace Engine::Components {
    void Text3D::Draw(VaultRenderer::Shader &shader) {
        if (font) {
            transform->UpdateModel();
            font->Draw(shader, transform->model, text, color, emissionColor, 0, 0, scale, y_offset, (uint32_t)entity);
        }
    }

    void Text3D::ChangeFont(const std::string &font_path) {
        font.reset();
        font = std::make_shared<VaultRenderer::Font>(font_path.c_str(), 64);
    }

    void Text3D::Init() {
        transform = &Scene::Main->EntityRegistry.get<Transform>(entity);
    }

    void Text3D::OnGUI() {
        DrawComponent<Text3D>(Scene::Main->EntityRegistry, 40, [&] {
            ImGui::InputTextMultiline("Text", &text);
            ImGui::DragFloat("Scale", &scale, 0.001f);
            ImGui::DragFloat("Y Offset", &y_offset, 0.01f);
            ImGui::ColorEdit3("Color", &color.x);
            ImGui::ColorEdit3("Emission Color", &emissionColor.x);
            if (font) {
                ImGui::Text("Font: %s", font->font_path.c_str());
            } else {
                ImGui::Text("Drag a Font File here");
            }
            if (ImGui::BeginDragDropTarget()) {
                if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("font_file")) {
                    ChangeFont((char *)payload->Data);
                }
            }
        });
    }
} // namespace Engine::Components