/*
Welcome to the worst game engines codes main file!
In here you will throw up all over your desk cuz the coding is fucking ass
*/

#include <stb_image/stb_image.h>
#include "Engine/Audio.hpp"
#include "Engine/CascadedShadowMap.hpp"
#include "Engine/Components/MeshRenderer.hpp"
#include "Engine/Components/Transform.hpp"
#include "Engine/Discord.hpp"
#include "Engine/GameObject.hpp"
#include "Engine/HDRSkybox.hpp"
#include "Engine/Model.hpp"
#include "Engine/SimpleCalls.hpp"
#include "GLFW/glfw3.h"
#include "HyperScript/HyperScript.hpp"
#include "Renderer/Framebuffer.hpp"
#include "imgui/TextEditor.hpp"
#include "mono/metadata/row-indexes.h"
#include <filesystem>
#include <iostream>
#include <Renderer/Window.hpp>
#include <Renderer/Mesh.hpp>
#include <Renderer/Texture.hpp>
#include <Renderer/Skybox.hpp>
#include <imgui/imgui.h>
#include <Engine/Scene.hpp>
#include <Engine/Components/IncludeComponents.hpp>
#include <Renderer/ShadowMap.hpp>
#include <Renderer/Stats.hpp>
#include <Renderer/Font.hpp>
#include <Engine/Runtime.hpp>
#include <Editor/EditorLayer.hpp>
#include <fstream>
#include <Editor/GUI/MainGUI.hpp>
#include <Engine/Batch.hpp>
#include <Engine/SceneSerialization.hpp>
#include <Engine/Scripting/LoadScripts.hpp>
#include <script_test.hpp>
#include <experimental/filesystem>

namespace fs = std::filesystem;
static VaultRenderer::Shader *default_shader;

using namespace Engine;
using namespace VaultRenderer;
using namespace Engine::Components;

#include <Engine/Mono/CSharp.hpp>

void AspectRatioCameraViewport() {
    const int targetWidth = 1920, targetHeight = 1080;
    float targetAspectRatio = (float)targetWidth / (float)targetHeight;

    int aspectWidth = VaultRenderer::Window::window->width;
    int aspectHeight = (int)((float)aspectWidth / targetAspectRatio);
    if (aspectHeight > VaultRenderer::Window::window->height) {
        aspectHeight = VaultRenderer::Window::window->height;
        aspectWidth = (int)((float)aspectHeight * targetAspectRatio);
    }
    int vpX = (int)(((float)VaultRenderer::Window::window->width / 2.0f) - ((float)aspectWidth / 2.0f));
    int vpY = (int)(((float)VaultRenderer::Window::window->height / 2.0f) - ((float)aspectHeight / 2.0f));

    if (Engine::Scene::Main->main_camera_object) {
        Engine::Scene::Main->main_camera_object->width = 1920;
        Engine::Scene::Main->main_camera_object->height = 1080;
    }

    glViewport(vpX, vpY, aspectWidth, aspectHeight);
}

void OnGUI(uint32_t smID) {
    using namespace VaultRenderer;
    using namespace Engine;
    using namespace Engine::Components;
    // ImGui::Begin("Statistics");
    // ImGui::Text("Draw Calls: %d", Statistics::GetDrawCalls());
    // ImGui::Text("Vendor: %s", Statistics::vendor.c_str());
    // ImGui::Text("Renderer: %s", Statistics::renderer.c_str());
    // ImGui::Text("Version: %s", Statistics::version.c_str());
    // ImGui::Text("Shading Language: %s", Statistics::shading_language.c_str());
    // ImGui::End();
}

using namespace Editor;
void print(const std::string &traki) {
    std::cout << traki << "\n";
}

#undef GAME_BUILD

int main() {
    using namespace VaultRenderer;

    // lets pretend i added some code here

    // chdir("/home/poww/Desktop/Projects/Vault-Engine");

    // dlsym test
    // Script *(*create_obj)() = (Script * (*)()) dlsym(dlopen("./sandbox/script.so", RTLD_LAZY), "create_script");
    // Script *script = create_obj();
    // script->Start();

    // HyperScript::ScriptEngine SE;
    // SE.LoadScript("test", "./scripts/test.hyper");
    // SE.JustRunTheScript(SE.GetModule("test"));

    Serializer::LoadConfigFile("./assets/config.yaml");
    std::cout << "its game buiald\n";
#ifdef GAME_BUILD
    std::cout << "its game build\n";
    Window window(1280, 720, Serializer::config.title == "" ? "Vault Engine" : Serializer::config.title.c_str());
#else
    Window window(1280, 720, "Vault Engine");
    window.SetWindowIcon("./editor/icons/icon.png");
#endif
    Statistics::SetStats();

    Shader shader("./shaders/pbr.glsl");
    Shader non_pbr_shader("./shaders/default.glsl");
    Shader skybox_shader("./shaders/skybox.glsl");
    Shader shadow_map_shader("./shaders/shadow_map.glsl");
    Shader shadow_cubemap_shader("./shaders/shadow_map_point.glsl");
    Shader equirectToCubemap("./shaders/equirectToCubemap.glsl");
    Shader hdri_skybox_shader("./shaders/hdri_skybox.glsl");
    Shader irr_shader("./shaders/hdri_irr.glsl");
    Shader prefilter_shader("./shaders/pbr_prefilter.glsl");
    Shader brdf_shader("./shaders/pbr_brdf.glsl");
    Shader collider_gizmo_shader("./shaders/collider_gizmo.glsl");
    // Texture equirect("./assets/skybox/metro_noord_8k.hdr", TEXTURE_HDRI);

    if (fs::exists(".vscode")) {
        fs::copy("./csharp-lib/lib", "./assets/VAULT_API", fs::copy_options::overwrite_existing | fs::copy_options::recursive);
    }

    Font::InitFT();
    default_shader = &non_pbr_shader;

    Skybox skybox;

    shader.Bind();

    float rotation = 0.5f;
    double previousTime = glfwGetTime();

    using namespace Engine;
    auto scene = Scene::New("./main.vault");
    Scene::SetMainScene(scene);
    Scene::MakeSceneCamera();
    Scene::Main->SetMainCameraObject(Scene::StaticGameObjects.back(), true);
    Scene::Main->main_camera_object->far = 550;

    ShadowMap shadow_map;
    CascadedShadowMap c_ShadowMap(4096, 500.f);
    shadow_map.near = Serializer::config.shadow_near;
    shadow_map.far = Serializer::config.shadow_far;
    shadow_map.ortho_size = Serializer::config.shadow_ortho_size;

    default_shader = Runtime::usePBR ? &shader : &non_pbr_shader;

    Runtime runtime(default_shader);

    // default mesh setup
    MeshRenderer::ModelMeshes[MESH_CUBE] = new ModelMesh("./default_models/cube.obj");
    MeshRenderer::ModelMeshes[MESH_CAPSULE] = new ModelMesh("./default_models/capsule.obj");
    MeshRenderer::ModelMeshes[MESH_SPHERE] = new ModelMesh("./default_models/sphere.obj");

    Runtime::default_shader = Runtime::usePBR ? &shader : &non_pbr_shader;
    runtime.shadowMap = &shadow_map;
    runtime.c_ShadowMap = &c_ShadowMap;
    EditorLayer editor;
    editor.ColliderGizmo.shader = &collider_gizmo_shader;

    GUI::framebufferTextureID = window.m_PostProcessingFramebuffer->texture;
    window.use_imgui_size = true;

    // Model m_2fort("./2fort/2fort.obj")
    // 1model("./capsule.obj");

    // Serializer::Deserialize("./scene.vault");
    // Serializer::DeserializeMaterial("./main.material", GameObject::FindGameObjectByName("My GameObject")->GetComponent<MeshRenderer>().mesh->material);
    // sceneSerializer.Serialize("./scene.vault");

    // Scene::SetMainScene(new_scene);
    // Scene::MakeSceneCamera();
    // Scene::Main->SetMainCameraObject(Scene::StaticGameObjects.back());

    // -- SKELETAL ANIMATION EXAMPLE --
    // Model model("./Crouch To Stand.fbx");
    // Animation danceAnimation("./Crouch To Stand.fbx", &model);
    // Animator animator(&danceAnimation);
    // animator.Update(timestep);
    // auto transforms = animator.GetFinalBoneMatrices();
    // shader.Bind();
    // for (int i = 0; i < transforms.size(); ++i)
    //     shader.SetUniformMat4(("finalBonesMatrices[" + std::to_string(i) + "]").c_str(), transforms[i]);
    // -- SKELETAL ANIMATION EXAMPLE --

    // GAME BUILD
    bool GAME_BUILD_called_once = false;

    // GUI
    auto teLangDef = TextEditor::LanguageDefinition::C();
    GUI::InitTextEditor();
    GUI::text_editor->SetLanguageDefinition(teLangDef);
    GUI::text_editor->SetTabSize(4);
    GUI::text_editor->SetPalette(TextEditor::GetDarkPalette());
    GUI::text_editor->SetShowWhitespaces(false);

    bool editingText = false;
    auto *fontCascadia = ImGui::GetIO().Fonts->AddFontFromFileTTF("assets/fonts/CascadiaMono.ttf", 16.0f);
    auto Function_GUI = [&] {
        ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

#ifdef GAME_BUILD
        bool &isStopped = Runtime::instance->isStopped;
        bool &isRunning = Runtime::instance->isRunning;

        if (!GAME_BUILD_called_once) {
            isRunning = true;
            isStopped = false;
            Scene::Main->OnRuntimeStart();

            auto v = Scene::Main->EntityRegistry.view<CXXScriptComponent>();

            for (auto e : v) {
                auto &component = Scene::Main->EntityRegistry.get<CXXScriptComponent>(e);
                component.OnStart();
            }

            for (auto &go : Scene::Main->GameObjects) {
                if (go->HasComponent<Camera>()) {
                    auto &camera = go->GetComponent<Camera>();
                    if (camera.main_camera) {
                        Scene::Main->SetMainCameraObject(go);
                        break;
                    }
                }
            }

            GAME_BUILD_called_once = true;
        }
        editor.GameGUI();
#else
        editor.GUI();

        if (ImGui::Begin(ICON_FA_CODE " Text Editor")) {

            ImVec2 size = ImGui::GetWindowSize();
            ImVec2 pos = ImGui::GetWindowPos();

            ImVec2 cursor = ImGui::GetMousePos();
            cursor.x -= pos.x;
            cursor.y -= pos.y;
            // window.SetMousePos(cursor.x, cursor.y)

            editingText = (cursor.x > -1 && cursor.x <= size.x) && (cursor.y > -1 && cursor.y <= size.y);

            ImGui::PushFont(fontCascadia);

            GUI::text_editor->Render("Text Editor");

            if (editingText && ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && ImGui::IsKeyPressed(ImGuiKey_S)) {
                std::ofstream file(GUI::TextEditor_path);
                file << GUI::text_editor->GetText();
                GUI::LogTick("File \"" + GUI::TextEditor_path + +"\" has been saved.");
            }
            ImGui::PopFont();
        }

        ImGui::End();

#endif

        for (auto &pointer : GameObject::scheduled_deletions) {
            pointer->UNSAFE_DeleteGameObject();
        }
        GameObject::scheduled_deletions.clear();
    };

    auto Function_ShadowMapRendering = [&] {
        // Directional Light  Shadow Mapping
        if (Scene::Main->EntityRegistry.valid(Scene::Main->EntityRegistry.view<DirectionalLight>().back())) {
            if (Scene::Main->EntityRegistry.get<DirectionalLight>(Scene::Main->EntityRegistry.view<DirectionalLight>().back()).enable_shadow_mapping) {
                shadow_map.RenderSpace([&](std::unique_ptr<Shader> &shadow_shader) {
                    // NOTE: shadow_shader is already binded
                    runtime.DrawToShadowMap(shadow_map.GetDepthBuffer(), *shadow_shader);
                });
            }
        }

        // Point Light Shadow Mapping
        auto pointLightView = Scene::Main->EntityRegistry.view<PointLight>();

        for (auto e : pointLightView) {
            auto &light = Scene::Main->EntityRegistry.get<PointLight>(e);
            if (!light.enable_shadow_mapping)
                continue;

            light.DrawToShadowMap(shadow_cubemap_shader);
        }
    };

    auto Function_FramebufferShaderUniforms = [&](Shader &fbShader) {
        // shader is already binded when this gets called btw & also DO NOT UNBIND HERE only set uniforms n shi
        fbShader.SetUniform1f("HDR_Exposure", Serializer::config.HDR.exposure);
        //
    };

    /*
    FUNCTION EXECUTION ORDER:
    1. Shadow Map Rendering Function, the last argument in window.Run
    2. Runtime Function, the first argument in window.Run
    3. Framebuffer Shader Uniforms (the shader is already binded btw)
    4. GUI Function, the second argument in window.Run

    basically:
    shadow_function()
    runtime_function()
    framebuffer_shader_config();
    gui_function()
    */
    float timestep = 0;

    // -- WORKER MESHES --
    // Shader worker_shader("./shaders/worker.glsl");
    // std::vector<Vertex> planeVertices = {
    //     Vertex{glm::vec3(-0.5f, -0.5f, 0.0f), glm::vec2(0.0f, 0.0f),
    //            glm::vec3(0, 1, 0)},
    //     Vertex{glm::vec3(0.5f, -0.5f, 0.0f), glm::vec2(1.0f, 0.0f),
    //            glm::vec3(0, 1, 0)},
    //     Vertex{glm::vec3(0.5f, 0.5f, 0.0f), glm::vec2(1.0f, 1.0f),
    //            glm::vec3(0, 1, 0)},
    //     Vertex{glm::vec3(-0.5f, 0.5f, 0.0f), glm::vec2(0.0f, 1.0f), glm::vec3(0, 1, 0)}
    //     //
    // };

    // std::vector<uint32_t> planeIndices = {0, 1, 2, 0, 2, 3};

    // Material material(glm::vec4(1, 0, 0, 1));
    // bool mesh_BoxCollider2D_active = false;
    // Mesh mesh_BoxCollider2D(planeVertices, planeIndices);
    // mesh_BoxCollider2D.material.color = glm::vec4(0, 1, 0, 1);
    // glm::vec3 bc2dPos = glm::vec3(0, 0, 0);
    // glm::vec3 bc2dScale = glm::vec3(1, 1, 1);
    glm::vec3 bc2dRotation = glm::vec3(0, 0, 0);
    // -- WORKER MESHES --

#ifndef _WIN32
    unsetenv("TERM");
#endif

    CSharp csharp("./mono/lib");
    // csharp.ReloadAssembly();

    // Audio
    Audio2D::InitAudio();

    if (fs::exists(Serializer::config.main_scene)) {
        Serializer::Deserialize(Serializer::config.main_scene);
    }

    // Discord
    DiscordRPC discordRPC("1025522890688442400");
    discordRPC.UpdatePresence("In Editor", "Making a game");

    // HDRI Skybox Setup
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    HDRSkybox hdrSkybox("./assets/skybox/default_skybox.hdr", &equirectToCubemap, &hdri_skybox_shader, &irr_shader, &prefilter_shader, &brdf_shader, 1024);

    auto Function_MousePicking = [&](Framebuffer::ColorAttachement &ca) {
        hdrSkybox.RenderEnvCubemap();

        glBindTexture(GL_TEXTURE_2D, ca.ID);
        glReadBuffer(GL_COLOR_ATTACHMENT2);
        uint32_t entityId;
        glReadPixels(Window::window->mouse_pos.x, Window::window->mouse_pos.y, 1, 1, GL_RED_INTEGER, GL_UNSIGNED_INT, &entityId);

        runtime.MouseEvents((entt::entity)entityId);

        if (!ImGui::IsMouseDoubleClicked(0)) return;
        auto &gameObject = Scene::Main->FindGameObjectByEntity((entt::entity)entityId);
        if (!gameObject) return;

        GUI::selected_gameObject = gameObject.get();
        //
    };
    // HDRI Skybox Setup

    // PBR Binding irradianceMap
    Runtime::instance->HDR_Skybox = &hdrSkybox;
    uint32_t IRR_FREE_TEXTURE_SLOT = 7;       // Change this if we add more texture options in Material
    uint32_t PREFILTER_FREE_TEXTURE_SLOT = 8; // Change this if we add more texture options in Material
    uint32_t BRDF_FREE_TEXTURE_SLOT = 9;      // Change this if we add more texture options in Material
    shader.Bind();
    shader.SetUniform1i("irradianceMap", IRR_FREE_TEXTURE_SLOT);
    Runtime::instance->HDR_Skybox->BindIrradianceMap(IRR_FREE_TEXTURE_SLOT);

    shader.SetUniform1i("prefilterMap", PREFILTER_FREE_TEXTURE_SLOT);
    Runtime::instance->HDR_Skybox->BindPrefilterMap(PREFILTER_FREE_TEXTURE_SLOT);

    shader.SetUniform1i("brdfLUT", BRDF_FREE_TEXTURE_SLOT);
    Runtime::instance->HDR_Skybox->BindBRDFLUT(BRDF_FREE_TEXTURE_SLOT);

    window.Run([&](Shader &framebuffer_shader) {
        static double lastTime = 0;
        double now = glfwGetTime();
        timestep = now - lastTime;
        lastTime = now;
        // Update the Main Camera of a scene
        runtime.UpdateMainCamera(window);
        // Reset the Statistic Draw Calls
        Statistics::ResetDrawCalls();

        // Set the background color to RED
        window.SetClearColor(0xFF0000);

        // animator.UpdateAnimation(timestep);

        // Render the skybox
        if (Scene::Main->main_camera_object) {
            hdrSkybox.RenderSkybox(hdri_skybox_shader, *Scene::Main->main_camera_object);
            // DO NOT REMOVE CODE BELOW
            // skybox.Render(skybox_shader, Scene::Main->main_camera_object->transform->position, Scene::Main->main_camera_object->transform->rotation, Scene::Main->main_camera_object->up);
        }

        // Shadow shenanigans and fuckery
        runtime.ShadowShenanigans(shadow_map);

        // Drawing Meshes and updaing GameObject Components
        glEnable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);

        // Bind all the camera matricies to the Font Shader (Text)
        Scene::Main->main_camera_object->BindToShader(*Font::font_shader);
        Scene::Main->main_camera_object->BindToShader(collider_gizmo_shader);

        // Update all the GameObjects components
        // shader.Bind();

        // if (GUI::selected_gameObject) {
        //     if (GUI::selected_gameObject->HasComponent<BoxCollider2D>()) {
        //         auto &component = GUI::selected_gameObject->GetComponent<BoxCollider2D>();
        //         auto &transform = GUI::selected_gameObject->GetComponent<Transform>();
        //         bc2dPos = transform.position;
        //         bc2dRotation = transform.rotation;
        //         bc2dScale = glm::vec3(component.size.x, component.size.y, 1);

        //         glClear(GL_DEPTH_BUFFER_BIT);
        //         glDepthFunc(GL_LEQUAL);
        //         glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        //         glDisable(GL_CULL_FACE);
        //         glm::mat4 model = glm::mat4(1.0f);
        //         model = glm::translate(model, bc2dPos) *
        //                 glm::toMat4(glm::quat(bc2dRotation)) *
        //                 glm::scale(model, glm::vec3(bc2dScale.x / 2, bc2dScale.y / 2, 1.0f));

        //         worker_shader.Bind();
        //         worker_shader.SetUniformMat4("transformModel", model);
        //         mesh_BoxCollider2D.Draw(worker_shader);
        //         std::cout << "Drawn!\n";
        //         glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        //         glEnable(GL_CULL_FACE);
        //     }
        // }

        // BIND PBR IBL SHIT
        // shader.Bind();
        // shader.SetUniform1i("irradianceMap", 7);
        Runtime::instance->HDR_Skybox->BindIrradianceMap(IRR_FREE_TEXTURE_SLOT);

        // shader.SetUniform1i("prefilterMap", 8);
        Runtime::instance->HDR_Skybox->BindPrefilterMap(PREFILTER_FREE_TEXTURE_SLOT);

        // shader.SetUniform1i("brdfLUT", 9);
        Runtime::instance->HDR_Skybox->BindBRDFLUT(BRDF_FREE_TEXTURE_SLOT);
        // BIND PBR IBL SHIT

        runtime.UpdateGameObjects(window); //

        // Scheduling
        if (Serializer::scheduled_scene_path != "") {
            GUI::selected_gameObject = nullptr;
            Scene::Main->SetSceneCameraAsMain();
            bool runtime_was_running = false;
            if (Runtime::instance->isRunning) runtime_was_running = true;

            if (runtime_was_running) {
                Runtime::instance->isRunning = false;
                Runtime::instance->isStopped = true;

                Scene::Main->OnRuntimeStop();
            }

            Serializer::Deserialize(Serializer::scheduled_scene_path);
            Serializer::scheduled_scene_path = "";

            if (runtime_was_running) {
                Scene::Main->OnRuntimeStart();

                auto v = Scene::Main->EntityRegistry.view<CXXScriptComponent>();

                for (auto e : v) {
                    auto &component = Scene::Main->EntityRegistry.get<CXXScriptComponent>(e);
                    component.OnStart();
                }

                for (auto &go : Scene::Main->GameObjects) {
                    if (go->HasComponent<Camera>()) {
                        auto &camera = go->GetComponent<Camera>();
                        if (camera.main_camera) {
                            Scene::Main->SetMainCameraObject(go);
                            break;
                        }
                    }
                }

                Runtime::instance->isRunning = true;
                Runtime::instance->isStopped = false;
            }

            if (!Scene::Main->main_camera_object) Scene::Main->SetSceneCameraAsMain();
        } // clang-format off

        // Config Updates
        {
            // Shadow Mapping
            Serializer::config.shadow_near = shadow_map.near;
            Serializer::config.shadow_far = shadow_map.far;
            Serializer::config.shadow_ortho_size = shadow_map.ortho_size;

            Runtime::default_shader =Runtime::usePBR ? &shader : &non_pbr_shader;
            default_shader = Runtime::default_shader;
        }


    }, Function_GUI, Function_ShadowMapRendering, Function_FramebufferShaderUniforms, Function_MousePicking);
    // clang-format on

    return 0;
}