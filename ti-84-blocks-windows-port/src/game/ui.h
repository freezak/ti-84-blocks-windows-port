#pragma once
#include "graphx.h"
#include "draw.h"
#include "textures.h"

// Parameters for the block selection UI
#define UI_BORDER 16
#define ICON_SPACING 40
#define ICON_COLS 6
#define ICON_WIDTH 16

#define ICON_BORDER ((LCD_WIDTH - (ICON_SPACING * (ICON_COLS - 1))) / 2)

// Parameters for the world selection UI
#define SAVE_CNT 5

// Some colors pulled from the palette to be used in the UI
#define UI_BORDER_COLOR 1
#define UI_SELECT_COLOR 2
#define UI_BACKGROUND_COLOR 3

// Add the colors needed to draw the UI elements to the palette
void init_ui_palette() { 
    //volatile uint16_t* p = (uint16_t*)0xE30200;
    palette[0] = gfx_RGBTo1555(  0,  0,  0);
    palette[1] = gfx_RGBTo1555(255,255,255);
    palette[2] = gfx_RGBTo1555(255,102, 64);

    palette[3] = gfx_RGBTo1555(128,128,128);
    palette[4] = gfx_RGBTo1555(192,192,192);
    palette[5] = gfx_RGBTo1555(160,160,160);
}

// Draws the outline of a progress bar and the label
void progress_bar(const char *text) {
    gfx_SetColor(0);
    gfx_Rectangle(16, (LCD_HEIGHT - 16) / 2, LCD_WIDTH - 32, 16);

    gfx_SetColor(1);
    gfx_FillRectangle(16, (LCD_HEIGHT - 32) / 2, LCD_WIDTH - 32, 8);

    gfx_SetTextFGColor(0);
    gfx_PrintStringXY(text, 16, (LCD_HEIGHT - 32) / 2);
}

// Fills in the progress bar with the fraction of progress / total
void fill_progress_bar(uint16_t progress, uint16_t total) {
    gfx_SetColor(2);
    gfx_FillRectangle(18, (LCD_HEIGHT - 12) / 2, progress * (LCD_WIDTH - 36) / (total - 1), 12);
}

// Draws the block selection GUI during play-mode.
void draw_block_select() {
    gfx_SetColor(UI_BACKGROUND_COLOR);
    gfx_FillRectangle(UI_BORDER, UI_BORDER, LCD_WIDTH - 2 * UI_BORDER, LCD_HEIGHT - 2 * UI_BORDER);

    gfx_SetColor(UI_BORDER_COLOR);
    gfx_Rectangle(UI_BORDER, UI_BORDER, LCD_WIDTH - 2 * UI_BORDER, LCD_HEIGHT - 2 * UI_BORDER);

    int32_t block_x = ICON_BORDER;
    int32_t block_y = ICON_BORDER - 20;

    // Draw all the textured blocks
    for(int i = 0; i < TEX_CNT; i++) {
        draw_block(block_x, block_y, (uint8_t*)textures[i]);

        block_x += ICON_SPACING;

        if(block_x >= LCD_WIDTH - 48) {
            block_x = ICON_BORDER;
            block_y += ICON_SPACING;
        }
    }

    // Draw the water block last
    draw_left_triangle( block_x,      block_y,      water_masks[2][0]);
    draw_right_triangle(block_x,      block_y,      water_masks[2][0]);

    draw_left_triangle( block_x,      block_y + 16, water_masks[2][0]);
    draw_right_triangle(block_x - 16, block_y +  8, water_masks[2][0]);

    draw_left_triangle( block_x + 16, block_y +  8, water_masks[2][0]);
    draw_right_triangle(block_x,      block_y + 16, water_masks[2][0]);

    //gfx_SwapDraw();
}

// The full block selection GUI. Dims the screen, draws the graphics, handles input
// and then returns the selected block. The previously selected block is passed as
// an argument.

uint8_t row, col, row_old, col_old;
Block_t _block;

void block_select(Block_t block) {
    _block = block;

    dim_screen();
    gfx_SetDrawScreen();
    draw_block_select();

    // Since water should show last on the list, we have to remap its block ID here
    if(block == WATER)
        block = TEX_CNT + STONE;

    row = (block - STONE) / ICON_COLS;
    col = (block - STONE) % ICON_COLS;

    row_old = row ^ 1;
    col_old = 0;
}

int block_select_tick() {
    if (row != row_old || col != col_old) {
        gfx_SetColor(UI_BACKGROUND_COLOR);
        gfx_Rectangle(ICON_BORDER - ICON_WIDTH - 2 + (col_old * ICON_SPACING),
            ICON_BORDER - ICON_WIDTH - 6 + (row_old * ICON_SPACING),
            36, 36);
        gfx_Rectangle(ICON_BORDER - ICON_WIDTH - 3 + (col_old * ICON_SPACING),
            ICON_BORDER - ICON_WIDTH - 7 + (row_old * ICON_SPACING),
            38, 38);

        gfx_SetColor(UI_SELECT_COLOR);
        gfx_Rectangle(ICON_BORDER - ICON_WIDTH - 2 + (col * ICON_SPACING),
            ICON_BORDER - ICON_WIDTH - 6 + (row * ICON_SPACING),
            36, 36);
        gfx_Rectangle(ICON_BORDER - ICON_WIDTH - 3 + (col * ICON_SPACING),
            ICON_BORDER - ICON_WIDTH - 7 + (row * ICON_SPACING),
            38, 38);
    }

    row_old = row;
    col_old = col;

    if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow)) {
        if (col > 0)
            col--;
    } else if (ImGui::IsKeyPressed(ImGuiKey_RightArrow)) {
        if (col < ICON_COLS - 1)
            col++;
    } else if (ImGui::IsKeyPressed(ImGuiKey_DownArrow)) {
        row++;
    } else if (ImGui::IsKeyPressed(ImGuiKey_UpArrow)) {
        if (row > 0)
            row--;
    } else if (ImGui::IsKeyPressed(ImGuiKey_KeypadEnter)) {
        gfx_SetDrawBuffer();
        _block = (row * ICON_COLS) + col + STONE;

        // Remap the block back to the actual ID
        if (_block == TEX_CNT + STONE)
            _block = WATER;

        return _block;
    }

    if (row * ICON_COLS + col > TEX_CNT) {
        row = row_old;
        col = col_old;
    }

    return -1;
}

uint8_t _option_cnt;

uint8_t menu_selection, menu_selection_old;

// A generic menu implementation used in some of the save management GUI
void menu(const char **options, uint8_t option_cnt) {
    _option_cnt = option_cnt;

    gfx_SetColor(4);
    gfx_FillRectangle(UI_BORDER, UI_BORDER, LCD_WIDTH - 2 * UI_BORDER, LCD_HEIGHT - 2 * UI_BORDER);

    gfx_SetColor(0);
    gfx_Rectangle(UI_BORDER, UI_BORDER, LCD_WIDTH - 2 * UI_BORDER, LCD_HEIGHT - 2 * UI_BORDER);

        
    gfx_SetTextFGColor(0);
    gfx_PrintStringXY(options[0], UI_BORDER + 16, UI_BORDER + 16);

    
    for(uint8_t i = 0; i < option_cnt; i++) {
        gfx_SetTextFGColor(0);
        gfx_PrintStringXY(options[i + 1], 
                          UI_BORDER + 16, 
                          LCD_HEIGHT - UI_BORDER - 16  - 16 * (option_cnt - 1 - i));
    }

    menu_selection = 0;
    menu_selection_old = menu_selection ^ 1;
}

int menu_tick() {
    if (menu_selection != menu_selection_old) {
        gfx_SetColor(4);
        gfx_Rectangle(UI_BORDER + 8,
            LCD_HEIGHT - UI_BORDER - 18 - 16 * (_option_cnt - 1 - menu_selection_old),
            LCD_WIDTH - UI_BORDER - UI_BORDER - 8 - 8, 12);

        gfx_SetColor(3);
        gfx_Rectangle(UI_BORDER + 8,
            LCD_HEIGHT - UI_BORDER - 18 - 16 * (_option_cnt - 1 - menu_selection),
            LCD_WIDTH - UI_BORDER - UI_BORDER - 8 - 8, 12);
    }

    menu_selection_old = menu_selection;

    if (ImGui::IsKeyPressed(ImGuiKey_DownArrow)) {
        if (menu_selection < _option_cnt - 1)
            menu_selection++;
    } else if (ImGui::IsKeyPressed(ImGuiKey_UpArrow)) {
        if (menu_selection > 0)
            menu_selection--;
    } else if (ImGui::IsKeyPressed(ImGuiKey_Enter)) {
        return menu_selection;
    }

    return -1;
}
