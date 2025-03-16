#pragma once

#include "graphx.h"
#include "draw.h"
#include "world.h"
#include "worldgen.h"
#include "world_io.h"
#include "player.h"
#include "ui.h"

#include <iostream>
#include <fstream>
#include <cstdio>
#include <filesystem>

enum GameState {
    GameState_PrepareWorldSelect,
    GameState_WorldSelect,
    GameState_PlayInit,
    GameState_Play
};

GameState gameState;

bool isMenu = false;
bool isInventory = false;
bool isMenuExited = false;
uint8_t menu_selection_value = 0;
Block_t inventory_selection_value = 0;

void show_menu() {
    if (!isMenu && !isMenuExited) {
        isMenu = true;
    }
}

void show_inventory() {
    if (!isInventory && !isMenuExited) {
        isInventory = true;
    }
}

bool is_menu_exited() {
    if (isMenuExited) {
        isMenuExited = false;
        return true;
    }

    return false;
}

void world_select();
void prepare_world_select();
void prepare_play();
bool init_play(uint8_t world_id, world_t* world, player_t& player);
void play();

uint8_t world_id;
world_t world;//(world_t*)0xD05350;
player_t player;

int32_t scroll_goal_x, scroll_goal_y;

void init_game() {
    // Set right face textures to always to be in shadow
    for (int i = 0; i < TEX_CNT; i++) {
        for (int j = 0; j < TEX_SIZE; j++) {
            textures[i][RIGHT_FACE * 2 + 0][j] |= SHADOW;
            textures[i][RIGHT_FACE * 2 + 1][j] |= SHADOW;
        }
    }

    // memset(palette, 0, 2024);
    for (int i = 0; i < 256; i++) {
        palette[i] = 0xffffff;
    }

    init_ui_palette();

    gameState = GameState_PrepareWorldSelect;
}

void game_tick() {
    static int menu_return;

    if (isMenu) {
        ImGui::Text("IsMenu");
        menu_return = menu_tick();
        if (menu_return != -1) {
            menu_selection_value = menu_return;
            isMenu = false;
            isMenuExited = true;
        }
        return;
    } else if (isInventory) {
        menu_return = block_select_tick();
        if (menu_return != -1) {
            inventory_selection_value = menu_return;
            isInventory = false;
            isMenuExited = true;
        }
        return;
    }

    switch (gameState) {
    case GameState_PrepareWorldSelect:
        prepare_world_select();
        break;
    case GameState_WorldSelect:
        world_select();
        break;
    case GameState_PlayInit:
        prepare_play();
        break;
    case GameState_Play:
        play();
        break;
    }
}

uint8_t world_select_selection, world_select_selection_old;

void prepare_world_select() {
    gfx_SetDrawScreen();
    gfx_FillScreen(1);

    gfx_SetColor(4);
    gfx_FillRectangle(UI_BORDER, UI_BORDER, LCD_WIDTH - 2 * UI_BORDER, LCD_HEIGHT - 2 * UI_BORDER);

    gfx_SetColor(0);
    gfx_Rectangle(UI_BORDER, UI_BORDER, LCD_WIDTH - 2 * UI_BORDER, LCD_HEIGHT - 2 * UI_BORDER);

    char name[8] = "World A";
    char filename[14] = "saves\\WORLDA";

    for (uint8_t i = 0; i < SAVE_CNT; i++) {
        name[6] = 'A' + i;
        filename[11] = 'A' + i;

        if (std::filesystem::exists(filename)) {
            gfx_SetTextFGColor(0);
            gfx_PrintStringXY(name, UI_BORDER + 16, UI_BORDER + 16 + i * 32);
            gfx_SetTextFGColor(3);
            gfx_PrintStringXY("48x16x48", UI_BORDER + 24, UI_BORDER + 24 + i * 32);
            
        } else  {
            gfx_SetTextFGColor(3);
            gfx_PrintStringXY(name, UI_BORDER + 16, UI_BORDER + 16 + i * 32);
            gfx_SetTextFGColor(3);
            gfx_PrintStringXY("( Empty )", UI_BORDER + 24, UI_BORDER + 24 + i * 32);
        }

    }

    gfx_SetTextFGColor(0);
    gfx_PrintStringXY("Quit", UI_BORDER + 16, LCD_HEIGHT - UI_BORDER - 16 - 8);

    world_select_selection = 0;
    world_select_selection_old = world_select_selection ^ 1;

    gameState = GameState_WorldSelect;
}

void world_select() {
    if (world_select_selection != world_select_selection_old) {
        gfx_SetColor(4);
        gfx_Rectangle(UI_BORDER + 8,
            UI_BORDER + 12 + world_select_selection_old * 32,
            LCD_WIDTH - UI_BORDER - UI_BORDER - 8 - 8, 24);

        gfx_SetColor(3);
        gfx_Rectangle(UI_BORDER + 8,
            UI_BORDER + 12 + world_select_selection * 32,
            LCD_WIDTH - UI_BORDER - UI_BORDER - 8 - 8, 24);
    }

    world_select_selection_old = world_select_selection;

    if (ImGui::IsKeyPressed(ImGuiKey_DownArrow)) {
        if (world_select_selection < SAVE_CNT)
            world_select_selection++;
    } else if (ImGui::IsKeyPressed(ImGuiKey_UpArrow)) {
        if (world_select_selection > 0)
            world_select_selection--;
    } else if (ImGui::IsKeyPressed(ImGuiKey_Enter)) {
        // Break from the program when the last option (quit) is selected
        if (world_select_selection == SAVE_CNT) exit_app = true;
        world_id = world_select_selection;
        gameState = GameState_PlayInit;
    } else if (ImGui::IsKeyPressed(ImGuiKey_Delete)) {
        const char* options[3] = { "Are you sure?", "No", "Yes" };

        menu(options, 2);
        show_menu(); 
    }

    if (is_menu_exited()) {
        if (menu_selection_value)
            erase(world_select_selection);
        gameState = GameState_PrepareWorldSelect;
    }
}

bool init_play(uint8_t world_id, world_t* world, player_t& player) {

    world->clear_world();
    world->init_tri_grid();

    if (is_menu_exited()) {
        switch (menu_selection_value) {
        case 0:
            generate_natural(*world, player);
            break;
        case 1:
            generate_flat(*world, player);
            break;
        case 2:
            generate_demo(*world, player);
            break;
        }
        player.scroll_to_center(scroll_x, scroll_y);
    } else 
    // Try to load the world file, and otherwise generate a new one
    if (!load(world_id, *world, player) && !isMenu) {
        const char* options[4] = { "What type of world?", "Natural", "Flat", "Demo" };
        
        menu(options, 3);
        show_menu();
        return false;
    }

    gfx_FillScreen(1);
    progress_bar("Building world...");

    progress_bar("Initializing shadows...");
    // Initialize the shadow triangle grid with data from the world
    for (int y = 0; y < WORLD_HEIGHT; y++) {
        fill_progress_bar(y, WORLD_HEIGHT + WORLD_HEIGHT);
        for (int z = 0; z < WORLD_SIZE; z++) {
            for (int x = 0; x < WORLD_SIZE; x++) {
                if (world->blocks[y][x][z] > WATER) {
                    world->set_block_shadow(x, y, z);
                }
            }
        }
    }

    progress_bar("Initializing blocks...");
    // Initialize the triangle grid with data from the world
    for (int y = 0; y < WORLD_HEIGHT; y++) {
        fill_progress_bar(y + WORLD_HEIGHT, WORLD_HEIGHT + WORLD_HEIGHT);
        for (int z = WORLD_SIZE - 1; z >= 0; z--) {
            for (int x = WORLD_SIZE - 1; x >= 0; x--) {
                if (world->blocks[y][x][z] == WATER) {
                    world->set_water(x, y, z);
                } else if (world->blocks[y][x][z] != AIR) {
                    world->set_block(x, y, z, world->blocks[y][x][z]);
                }
            }
        }
    }

    init_palette();
    memset(VRAM, SKY, LCD_CNT);

    draw_x0 = 0;
    draw_y0 = 0;
    draw_x1 = LCD_WIDTH;
    draw_y1 = LCD_HEIGHT;

    gfx_SetDrawBuffer();
    draw_tri_grid(*world);

    player.draw();

    return true;
}

void prepare_play() {
    player.current_block = STONE;
    player.world = &world;

    if (!init_play(world_id, &world, player)) return;

    scroll_goal_x = scroll_x;
    scroll_goal_y = scroll_y;

    gameState = GameState_Play;
}

/*
7 -> 8
8 -> 9
9 -> 6 
4 -> 7
6 -> 3
1 -> 4
2 -> 1
3 -> 2
*/

void play() {
    // TODO: add input implementation
    if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow)) {
        scroll_goal_x += SCROLL_SPEED;
    } else if (ImGui::IsKeyPressed(ImGuiKey_RightArrow)) {
        scroll_goal_x -= SCROLL_SPEED;
    } else if(ImGui::IsKeyPressed(ImGuiKey_DownArrow)) {
        scroll_goal_y -= SCROLL_SPEED;
    } else if(ImGui::IsKeyPressed(ImGuiKey_UpArrow)) {
        scroll_goal_y += SCROLL_SPEED;
    } else if (ImGui::IsKeyPressed(ImGuiKey_Keypad8)) {
        player.move(0, 0, 1);
        player.scroll_to_contain(scroll_goal_x, scroll_goal_y);
    } else if (ImGui::IsKeyPressed(ImGuiKey_Keypad9)) {
        player.move(1, 0, 1);
        player.scroll_to_contain(scroll_goal_x, scroll_goal_y);
    } else if (ImGui::IsKeyPressed(ImGuiKey_Keypad6)) {
        player.move(1, 0, 0);
        player.scroll_to_contain(scroll_goal_x, scroll_goal_y);
    } else if (ImGui::IsKeyPressed(ImGuiKey_Keypad7)) {
        player.move(-1, 0, 1);
        player.scroll_to_contain(scroll_goal_x, scroll_goal_y);
    } else if (ImGui::IsKeyPressed(ImGuiKey_Keypad3)) {
        player.move(1, 0, -1);
        player.scroll_to_contain(scroll_goal_x, scroll_goal_y);
    } else if (ImGui::IsKeyPressed(ImGuiKey_Keypad4)) {
        player.move(-1, 0, 0);
        player.scroll_to_contain(scroll_goal_x, scroll_goal_y);
    } else if (ImGui::IsKeyPressed(ImGuiKey_Keypad1)) {
        player.move(-1, 0, -1);
        player.scroll_to_contain(scroll_goal_x, scroll_goal_y);
    } else if (ImGui::IsKeyPressed(ImGuiKey_Keypad2)) {
        player.move(0, 0, -1);
        player.scroll_to_contain(scroll_goal_x, scroll_goal_y);
    } else if (ImGui::IsKeyPressed(ImGuiKey_KeypadMultiply)) {
        player.move(0, 1, 0);
        player.scroll_to_contain(scroll_goal_x, scroll_goal_y);
    } else if (ImGui::IsKeyPressed(ImGuiKey_KeypadSubtract)) {
        player.move(0, -1, 0);
        player.scroll_to_contain(scroll_goal_x, scroll_goal_y);
    } else if (ImGui::IsKeyPressed(ImGuiKey_Keypad5)) {
        // Initialize our update region to be empty
        empty_draw_region();

        // Place or remove block will expand the update region to contain all updated
        // blocks (where a shadow is cast or uncast)
        if (player.current_block != WATER) {
            if (world.blocks[player.y][player.x][player.z] == AIR) {
                world.place_block(player.x, player.y, player.z, player.current_block);
            }
            // Just replace water with solid blocks when placing (double tap 5 to replace water with air)
            else if (world.blocks[player.y][player.x][player.z] == WATER) {
                world.remove_block(player.x, player.y, player.z);
                world.place_block(player.x, player.y, player.z, player.current_block);
            } else {
                world.remove_block(player.x, player.y, player.z);
            }
        } else
        {
            if (world.blocks[player.y][player.x][player.z] == AIR) {
                world.set_water(player.x, player.y, player.z);
                expand_draw_region(player.x, player.y, player.z);
            } else {
                world.remove_block(player.x, player.y, player.z);
            }
        }

        // Redraw the section of the screen where updates occurred and the cursor on top of that
        draw_tri_grid(world);
        player.draw();
    } else if (ImGui::IsKeyPressed(ImGuiKey_KeypadEnter)) {
        // TODO: block select
        block_select(player.current_block);
        show_inventory();
    } else if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
        init_ui_palette();
        gfx_SetDrawScreen();

        gfx_FillScreen(1);
        progress_bar("Saving...");

        save(world_id, world, player);

        gameState = GameState_PrepareWorldSelect;
    }

    if (is_menu_exited()) {
        player.current_block = inventory_selection_value;
    }

    // Compute the motion needed to reach our scroll target
    int32_t scroll_step_x = scroll_goal_x - scroll_x;
    int32_t scroll_step_y = scroll_goal_y - scroll_y;

    // Clamp it to the max scroll speed
    if (scroll_step_x > SCROLL_SPEED)
        scroll_step_x = SCROLL_SPEED;
    if (scroll_step_x < -SCROLL_SPEED)
        scroll_step_x = -SCROLL_SPEED;

    if (scroll_step_y > SCROLL_SPEED)
        scroll_step_y = SCROLL_SPEED;
    if (scroll_step_y < -SCROLL_SPEED)
        scroll_step_y = -SCROLL_SPEED;

    if (scroll_step_x != 0 || scroll_step_y != 0) {
        scroll_view(world, scroll_step_x, scroll_step_y);
        player.draw();
    }
}
