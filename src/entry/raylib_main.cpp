// NGED Raylib backend: raylib + ImGui_ImplRaylib

#include "imgui.h"
#include "imgui_impl_raylib.h"
#include "raylib.h"
#include "entry.h"

namespace nged {

static nged::App* theApp = nullptr;

static void startMainLoop()
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_HIGHDPI);
    InitWindow(1280, 720, theApp->title());
    SetTargetFPS(60);
    SetExitKey(0);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui_ImplRaylib_Init();

    theApp->init();

    // Auto DPI scaling
    auto dpi = GetWindowScaleDPI();
    ImGui::GetStyle().ScaleAllSizes(dpi.x);

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

    theApp->agreeToQuit();
    theApp->quit();

    ImGui_ImplRaylib_Shutdown();
    ImGui::DestroyContext();

    CloseWindow();
}

void startApp(nged::App* app)
{
    theApp = app;
    startMainLoop();
}

} // namespace nged
