#pragma once

#include "world.h"
#include "player.h"
#include "ui.h"

#include <stdint.h>

#include <iostream>
#include <fstream>
#include <cstdio>
#include <filesystem>

namespace fs = std::filesystem;

// Saves a world and player position details to a set of files.
// (world_id should be 1-5 though that limit is only imposed by the UI)
void save(uint8_t world_id, world_t& world, player_t& player) {
    if (!fs::exists("saves")) {
        fs::create_directory("saves");
    }

    char filename[13] = "saves\\WORLDA";
    filename[11] = 'A' + world_id;

    // Save player position
    {
        std::ofstream file;
        file.open(filename, std::ios::binary | std::ios::trunc);
        if (!file.is_open()) return;

        //ti_PutC((uint8_t)player.x, var);
        file.put(player.x);
        //ti_PutC((uint8_t)player.y, var);
        file.put(player.y);
        //ti_PutC((uint8_t)player.z, var);
        file.put(player.z);
        //ti_PutC((uint8_t)player.current_block, var);
        file.put(player.current_block);

        //ti_PutC((uint8_t)((scroll_x >> 16) & 0xFF), var);
        file.put((uint8_t)((scroll_x >> 16) & 0xFF));
        //ti_PutC((uint8_t)((scroll_x >>  8) & 0xFF), var);
        file.put((uint8_t)((scroll_x >> 8) & 0xFF));
        //ti_PutC((uint8_t)((scroll_x >>  0) & 0xFF), var);
        file.put((uint8_t)((scroll_x >> 0) & 0xFF));

        //ti_PutC((uint8_t)((scroll_y >> 16) & 0xFF), var);
        file.put((uint8_t)((scroll_y >> 16) & 0xFF));
        //ti_PutC((uint8_t)((scroll_y >>  8) & 0xFF), var);
        file.put((uint8_t)((scroll_y >> 8) & 0xFF));
        //ti_PutC((uint8_t)((scroll_y >>  0) & 0xFF), var);
        file.put((uint8_t)((scroll_y >> 0) & 0xFF));

        //ti_SetArchiveStatus(true, var);

        file.close();
    }



    char out_name[15] = "saves\\WORLDA00";
    out_name[11] = 'A' + world_id;

    // To save RAM, worlds are stored and loaded one horizontal slice
    // at a time
    for (uint8_t i = 0; i < WORLD_HEIGHT; i++) {
        fill_progress_bar(i, WORLD_HEIGHT);

        // Set the digit counter to the world level
        out_name[12] = '0' + (i / 10);
        out_name[13] = '0' + (i % 10);

        /* Open a new variable; deleting it if it already exists */
        std::ofstream file;
        file.open(out_name, std::ios::binary | std::ios::trunc);
        if (!file.is_open()) return;

        //ti_Write((char*)&world.blocks[i], WORLD_SIZE * WORLD_SIZE, 1, var);
        file.write(reinterpret_cast<const char*>(world.blocks[i]), WORLD_SIZE * WORLD_SIZE * sizeof(Block_t));

        //ti_SetArchiveStatus(true, var);

        file.close();
    }
}

// Attempts to load a world in from a given ID. Returns true or
// false depending on if this was successful
bool load(uint8_t world_id, world_t& world, player_t& player) {
    char filename[13] = "saves\\WORLDA";
    filename[11] = 'A' + world_id;

    // Load player position
    {
        std::ifstream file;
        file.open(filename);
        if (!file.is_open()) return false;

        //player.x = (int24_t)ti_GetC(var);
        file.read((char*)&player.x, 1);
        //player.y = (int24_t)ti_GetC(var);
        file.read((char*)&player.y, 1);
        //player.z = (int24_t)ti_GetC(var);
        file.read((char*)&player.z, 1);

        //player.current_block = ti_GetC(var);
        file.read((char*)&player.current_block, 1);

        //scroll_x = 0;
        //scroll_x += ti_GetC(var);
        //scroll_x <<= 8; 
        //scroll_x += ti_GetC(var);
        //scroll_x <<= 8; 
        //scroll_x += ti_GetC(var);
        scroll_x = file.get();

        //scroll_y = 0;
        //scroll_y += ti_GetC(var);
        //scroll_y <<= 8; 
        //scroll_y += ti_GetC(var);
        //scroll_y <<= 8; 
        //scroll_y += ti_GetC(var);
        scroll_y = file.get();

        //ti_SetArchiveStatus(true, var);

        file.close();
    }

    char out_name[15] = "saves\\WORLDA00";
    out_name[11] = 'A' + world_id;

    // To save RAM, worlds are stored and loaded one horizontal slice
    // at a time
    for (uint8_t i = 0; i < WORLD_HEIGHT; i++) {
        // Set the digit counter to the world level
        out_name[12] = '0' + (i / 10);
        out_name[13] = '0' + (i % 10);

        /* Open a new variable; deleting it if it already exists */
        std::ifstream file;
        file.open(out_name, std::ios::binary);
        if (!file.is_open()) return false;

        //ti_Read((char*)&world.blocks[i], WORLD_SIZE * WORLD_SIZE, 1, var);
        file.read(reinterpret_cast<char*>(world.blocks[i]), WORLD_SIZE * WORLD_SIZE * sizeof(Block_t));

        file.close();
    }

    return true;
}

// Deletes all related world files for a given ID
void erase(uint8_t world_id) {
    char filename[13] = "saves\\WORLDA";
    filename[11] = 'A' + world_id;

    if (std::remove(filename) != 0) return;

    char out_name[15] = "saves\\WORLDA00";
    out_name[11] = 'A' + world_id;

    for (uint8_t i = 0; i < WORLD_HEIGHT; i++) {
        // Set the digit counter to the world level
        out_name[12] = '0' + (i / 10);
        out_name[13] = '0' + (i % 10);
        std::remove(out_name);
    }
}