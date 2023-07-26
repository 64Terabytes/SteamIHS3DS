#include <3ds.h>
#include <citro3d.h>
#include <citro2d.h>
#include <vector>

#include "imgui/imgui.h"
#include "imgui/imgui_sw.h"

int main(int argc, char* argv[])
{
    u16 width = 320, height = 240;

    C3D_Tex *tex = (C3D_Tex*)malloc(sizeof(C3D_Tex));
    static const Tex3DS_SubTexture subt3x = { 512, 256, 0.0f, 1.0f, 1.0f, 0.0f };

    C2D_Image image = (C2D_Image){tex, &subt3x };
    C3D_TexInit(image.tex, 512, 256, GPU_RGBA8);
    C3D_TexSetFilter(image.tex, GPU_LINEAR, GPU_LINEAR);
    C3D_TexSetWrap(image.tex, GPU_REPEAT, GPU_REPEAT);
    gfxInitDefault();
    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
    C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
    C2D_Prepare();

    C3D_RenderTarget* top = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);
    consoleInit(GFX_TOP, NULL);

    std::vector<uint32_t> pixel_buffer(width * height, 0);

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2((float)width, (float)height);
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    imgui_sw::bind_imgui_painting();
    imgui_sw::SwOptions sw_options;
    imgui_sw::make_style_fast();

    touchPosition touch;
    bool InMenu = true; // Set to true initially to show the menu on startup
    while (aptMainLoop())
    {
        hidScanInput();
        u32 kHeld = hidKeysHeld();
        io.DeltaTime = 1.0f / 60.0f;

        if (kHeld & KEY_START)
        {
            if (InMenu)
            {
                break; // Exit the application if Start button is pressed and InMenu is true
            }
        }

        ImGui::NewFrame();

        // Set ImGui window size to fill the entire bottom screen
        ImGui::SetNextWindowSize(ImVec2((float)width, (float)height), ImGuiCond_Always);

        // Center the ImGui window on the bottom screen
        ImVec2 window_pos = ImVec2((320 - width) / 2, (240 - height) / 2);
        ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always);

        // Make the ImGui window not movable and non-collapsible, hide title bar and scrollbars
        ImGui::Begin("Main Menu", &InMenu, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar);

        if (ImGui::Button("Button 1"))
        {
            // Handle button click
        }

        if (ImGui::Button("Button 2"))
        {
            // Handle button click
        }

        // Add more ImGui widgets as needed...

        ImGui::End();

        ImGui::Render();

        memset(io.NavInputs, 0, sizeof(io.NavInputs));
        #define MAP_BUTTON(NAV, BUTTON)       { if (kHeld & BUTTON) io.NavInputs[NAV] = 1.0f; }
        MAP_BUTTON(ImGuiNavInput_Activate,    KEY_A);
        MAP_BUTTON(ImGuiNavInput_Cancel,      KEY_B);
        MAP_BUTTON(ImGuiNavInput_Menu,        KEY_Y);
        MAP_BUTTON(ImGuiNavInput_Input,       KEY_X);
        // Add more button mappings as needed...
        #undef MAP_BUTTON

        io.BackendFlags |= ImGuiBackendFlags_HasGamepad;
        hidTouchRead(&touch);
        if (touch.px && touch.py)
        {
            io.MouseDown[0] = true;
            io.MousePos = ImVec2(touch.px, touch.py);
        }
        else
        {
            io.MouseDown[0] = false;
        }

        std::fill_n(pixel_buffer.data(), width * height, 0x19191919u);

        paint_imgui(pixel_buffer.data(), width, height, sw_options);

        for (u32 x = 0; x < width; x++)
        {
            for (u32 y = 0; y < height; y++)
            {
                u32 dstPos = ((((y >> 3) * (512 >> 3) + (x >> 3)) << 6) + ((x & 1) | ((y & 1) << 1) | ((x & 2) << 1) | ((y & 2) << 2) | ((x & 4) << 2) | ((y & 4) << 3))) * 4;
                u32 srcPos = (y * 320 + x) * 4;
                memcpy(&((u8*)image.tex->data)[dstPos], &((u8*)pixel_buffer.data())[srcPos], 4);
            }
        }

        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
        C2D_TargetClear(top, C2D_Color32(32, 38, 100, 0xFF));
        C2D_SceneBegin(top);
        C2D_DrawImageAt(image, 0.0f, 0.0f, 0.0f, NULL, 1.0f, 1.0f);
        C3D_FrameEnd(0);
    }

    imgui_sw::unbind_imgui_painting();
    ImGui::DestroyContext();
    gfxExit();

    return 0;
}
