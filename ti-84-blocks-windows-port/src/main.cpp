#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <iostream>

#include "theme.h"

#include "game/game.h"
#include "game/textures.h"
#include <sstream>

// Window resize callback
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Set OpenGL version (GL 3.3 Core)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create window
    GLFWwindow* window = glfwCreateWindow(974, 900, "TI-84 cubes port", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Disable imgui.ini
    io.IniFilename = NULL;

    // Init theme
    initTheme();

    init_game();

    while (!glfwWindowShouldClose(window)) {
        // Handle events
        glfwPollEvents();

        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Game tick
        game_tick();

        static int scale = 2;
        static int colorOffset = 0;
        static bool showIDs = false;
        static bool drawViewportBorder = false;

        // Game window
        ImGui::SetNextWindowPos({ 302, 16 }, ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize({ (float)(320 * scale + 16), (float)(240 * scale + 36) }, ImGuiCond_FirstUseEver);
        ImGui::Begin("Game");

        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImVec2 screen_pos = ImGui::GetCursorScreenPos(); // Get starting position

        if (drawViewportBorder)
            draw_list->AddRect(ImVec2(screen_pos.x - 1, screen_pos.y - 1), ImVec2(screen_pos.x + 320 * scale + 1, screen_pos.y + 240 * scale + 1), IM_COL32(255, 0, 0, 255));

        for (int y = 0; y < LCD_HEIGHT; y++)
        {
            for (int x = 0; x < LCD_WIDTH; x++)
            {
                uint8_t pixel_value = (draw_ui) ? ui_buffer[y * LCD_WIDTH + x] : VRAM[y * LCD_WIDTH + x];
                /*if ((ui_buffer[y * LCD_WIDTH + x] & 0x8000) == 0) {
                    pixel_value = ui_buffer[y * LCD_WIDTH + x];
                }*/
                uint32_t color = gfx_1555ToImGuiColor(palette[(colorOffset + pixel_value) % 255]);  // Convert 8-bit value to RGBA

                float px = screen_pos.x + x * scale;
                float py = screen_pos.y + y * scale;
                draw_list->AddRectFilled(ImVec2(px, py), ImVec2(px + scale, py + scale), color);
            }
        }

        ImGui::End();

        // Settings window
        ImGui::SetNextWindowPos({ 16, 16 }, ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize({ 270, 256 }, ImGuiCond_FirstUseEver);
        ImGui::Begin("Settings");

        ImGui::DragInt("Color offset", &colorOffset, 1, 0, 255);
        ImGui::Checkbox("Show palette IDs", &showIDs);
        ImGui::Checkbox("Draw viewport border", &drawViewportBorder);

        ImGui::End();

        // Palette window
        ImGui::SetNextWindowPos({ 16, 288 }, ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize({ 270, 290 }, ImGuiCond_FirstUseEver);

        ImGui::Begin("Color palette");

        draw_list = ImGui::GetWindowDrawList();
        screen_pos = ImGui::GetCursorScreenPos();

        for (int y = 0; y < 16; y++) {
            for (int x = 0; x < 16; x++) {
                uint32_t color = gfx_1555ToImGuiColor(palette[y * 16 + x]);

                float px = screen_pos.x + x * 16;
                float py = screen_pos.y + y * 16;
                draw_list->AddRectFilled(ImVec2(px + 1, py + 1), ImVec2(px + 14, py + 14), color);
                if (showIDs) {
                    std::stringstream strs;
                    strs << y * 16 + x;
                    std::string temp_str = strs.str();
                    char* char_type = (char*)temp_str.c_str();
                    draw_list->AddText({ px, py }, IM_COL32(255, 255, 255, 255), char_type);
                }
            }
        }

        ImGui::End();

        // Debug info
        ImGui::SetNextWindowPos({ 16, 594 }, ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize({ 270, 290 }, ImGuiCond_FirstUseEver);
        ImGui::Begin("Debug info");

        ImGui::Text("Player pos: %d, %d, %d", player.x, player.y, player.z);
        ImGui::Text("Current block: %d", player.current_block);
        ImGui::Text("row: %d    col: %d", row, col);

        ImGui::End();

        // Controls
        ImGui::SetNextWindowPos({ 302, 548 }, ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize({ 656, 336 }, ImGuiCond_FirstUseEver);
        ImGui::Begin("Controls");

        //ImGui::Text("Menu:\n\tNavigate: Arrows\n\tSelect: Enter\n\rRemove: Delete\n\nMovement:\n\tHorizontal:\n\t\tKeypad_1, Keypad_2, Keypad_3, Keypad_4, Keypad_6, Keypad_7, Keypad_8, Keypad_9\n\tVertical:\n\t\tKeypad_*, Keypad_Substract (-)\n\nPlace block:  Numpad_5\nSelect block: Numpad_Enter\nExit to menu: Escape");
        ImVec4 color  { 0.25f, 0.53f, 0.96f, 1.0f };
        ImVec4 color2 { 0.25f, 0.96f, 0.53f, 1.0f };
        ImGui::TextColored(color2, "Menu:");
        ImGui::Text("Navigate    :");
        ImGui::SameLine();
        ImGui::TextColored(color, "Arrows");
        ImGui::Text("Select      :");
        ImGui::SameLine();
        ImGui::TextColored(color, "Enter");
        ImGui::Text("Delete world:");
        ImGui::SameLine();
        ImGui::TextColored(color, "Delete");
        ImGui::NewLine();
        ImGui::TextColored(color2, "Movement:");
        ImGui::Text("Horizontal  :");
        ImGui::SameLine();
        ImGui::TextColored(color, "Keypad_1");
        ImGui::SameLine();
        ImGui::TextColored(color, "Keypad_2");
        ImGui::SameLine();
        ImGui::TextColored(color, "Keypad_3");
        ImGui::SameLine();
        ImGui::TextColored(color, "Keypad_4");
        ImGui::SameLine();
        ImGui::TextColored(color, "Keypad_6");
        ImGui::SameLine();
        ImGui::TextColored(color, "Keypad_7");
        ImGui::SameLine();
        ImGui::TextColored(color, "Keypad_8");
        ImGui::SameLine();
        ImGui::TextColored(color, "Keypad_9");
        ImGui::Text("Vertical    :");
        ImGui::SameLine();
        ImGui::TextColored(color, "Keypad_Multiply \"*\"");
        ImGui::SameLine();
        ImGui::TextColored(color, "Keypad_Substract \"-\"");
        ImGui::NewLine();
        ImGui::TextColored(color2, "Other:");
        ImGui::Text("Place block :");
        ImGui::SameLine();
        ImGui::TextColored(color, "Numpad_5");
        ImGui::Text("Select block:");
        ImGui::SameLine();
        ImGui::TextColored(color, "Numpad_Enter");
        ImGui::Text("Exit to menu:");
        ImGui::SameLine();
        ImGui::TextColored(color, "Escape");


        ImGui::End();

        // Render
        ImGui::Render();
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);

        if (exit_app) break;
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}