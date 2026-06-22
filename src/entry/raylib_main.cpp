// NGED Raylib backend: raylib + ImGui_ImplRaylib
// Uses raw ImGui_ImplRaylib API to avoid font-atlas conflicts with NGED's ImGuiResource.

#include "imgui.h"
#include "imgui_impl_raylib.h"
#include "raylib.h"
#include "entry.h"

namespace nged {

static nged::App* theApp = nullptr;

static int startMainLoop()
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(1280, 720, theApp->title());
    SetTargetFPS(60);
    SetExitKey(0);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 0.0f;
    style.Colors[ImGuiCol_WindowBg].w = 1.0f;

    ImGui_ImplRaylib_Init();

    theApp->init();

    // Main loop
    while (!WindowShouldClose())
    {
        ImGui_ImplRaylib_ProcessEvents();
        ImGui_ImplRaylib_NewFrame();
        ImGui::NewFrame();

        theApp->update();

        ImGui::Render();

        BeginDrawing();
        ClearBackground(DARKGRAY);
        ImGui_ImplRaylib_RenderDrawData(ImGui::GetDrawData());
        EndDrawing();
    }

    // ponytail: raylib can't cancel WindowShouldClose; agreeToQuit is informational here.
    // Use raw GLFW backend when unsaved-work confirmation is required.
    theApp->agreeToQuit();
    theApp->quit();

    ImGui_ImplRaylib_Shutdown();
    ImGui::DestroyContext();

    CloseWindow();

    return 0;
}

void startApp(nged::App* app)
{
    theApp = app;
    startMainLoop();
}

} // namespace nged
