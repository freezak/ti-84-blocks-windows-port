#pragma once

#include <string.h>
#include "textures.h"
#include "world.h"

#define LCD_WIDTH   320
#define LCD_HEIGHT  240

#define BLOCK_WIDTH  32
#define BLOCK_HALF_WIDTH  (BLOCK_WIDTH / 2)
#define BLOCK_HEIGHT 31

#define SCROLL_SPEED 8

#define LCD_CNT (LCD_WIDTH * LCD_HEIGHT)

extern uint8_t* buffer1;
extern uint8_t* buffer2;

extern uint8_t* ui_buffer;
extern bool draw_ui;

extern bool exit_app;

#define BUFFER_SWP ((uint64_t)buffer1 ^ (uint64_t)buffer2)

extern uint8_t* VRAM;

extern int32_t scroll_x;
extern int32_t scroll_y;

// Bounds to draw within
extern uint16_t draw_x0;
extern uint16_t draw_y0;
extern uint16_t draw_x1;
extern uint16_t draw_y1;

// Draws a left facing triangle and checks every pixel to ensure nothing gets drawn out of bounds
void draw_left_triangle_clipped(int32_t x0, int32_t y0, uint8_t *tex, uint8_t *shadow_mask, uint8_t *water_mask);

// Draws a right facing triangle and checks every pixel to ensure nothing gets drawn out of bounds
void draw_right_triangle_clipped(int32_t x0, int32_t y0, uint8_t *tex, uint8_t *shadow_mask, uint8_t *water_mask);

// Draws a left facing triangle
void draw_left_triangle(int32_t x0, int32_t y0, uint8_t *tex, uint8_t *shadow_mask, uint8_t *water_mask);

// Draws a right facing triangle
void draw_right_triangle(int32_t x0, int32_t y0, uint8_t *tex, uint8_t *shadow_mask, uint8_t *water_mask);


// Draws a left facing filled triangle and checks every pixel to ensure nothing gets drawn out of bounds
void draw_left_triangle_clipped(int32_t x0, int32_t y0, uint8_t *water_mask);

// Draws a right facing filled triangle and checks every pixel to ensure nothing gets drawn out of bounds
void draw_right_triangle_clipped(int32_t x0, int32_t y0, uint8_t *water_mask);

// Draws a left facing filled triangle
void draw_left_triangle(int32_t x0, int32_t y0, uint8_t *water_mask);

// Draws a right facing filled triangle
void draw_right_triangle(int32_t x0, int32_t y0, uint8_t *water_mask);


// Draws a left facing triangle
void draw_left_triangle(int32_t x0, int32_t y0, uint8_t *tex, uint8_t flags);

// Draws a right facing triangle
void draw_right_triangle(int32_t x0, int32_t y0, uint8_t *tex, uint8_t flags);


void draw_block(int32_t x, int32_t y, uint8_t *tex);

void draw_block(uint8_t x, uint8_t y, uint8_t z, uint8_t *tex);

void draw_tri_grid(world_t &world);

void scroll_view(world_t &world, int32_t x, int32_t y);

void dim_screen();

void draw_num(int32_t x, int32_t y, uint8_t n);

void empty_draw_region(void);

void expand_draw_region(uint8_t x, uint8_t y, uint8_t z);