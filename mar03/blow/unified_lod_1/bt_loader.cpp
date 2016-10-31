#include "framework.h"

#include "bt_loader.h"
#include "make_terrain.h"  // For Elevation_Map; unravel?
#include "binary_file_stuff.h"

#include <stdio.h>
#include <float.h>

Bt_Loader::Bt_Loader() {
}

Bt_Loader::~Bt_Loader() {
}

Elevation_Map *Bt_Loader::load(File_Handle *file) {
    const int HEADER_SIZE = 10;
    char header[HEADER_SIZE+1];
    int length = fread(header, HEADER_SIZE, 1, file);
    if (length < 1) return NULL;

    header[HEADER_SIZE] = 0;

    int columns;
    int rows;

    bool error = false;
    get_u4b(file, &columns, &error);
    get_u4b(file, &rows, &error);
    
    int bytes_per_sample;
    int floating_point_flag;
    get_u2b(file, &bytes_per_sample, &error);
    get_u2b(file, &floating_point_flag, &error);

    int horizontal_units, utm_zone, datum;
    get_u2b(file, &horizontal_units, &error);
    get_u2b(file, &utm_zone, &error);
    get_u2b(file, &datum, &error);

    int junk;
    get_u4b(file, &junk, &error);
    get_u4b(file, &junk, &error);
    get_u4b(file, &junk, &error);
    get_u4b(file, &junk, &error);
    get_u4b(file, &junk, &error);
    get_u4b(file, &junk, &error);
    get_u4b(file, &junk, &error);
    get_u4b(file, &junk, &error);

    int external_projection;
    get_u2b(file, &external_projection, &error);

    float vertical_units;
    get_f32(file, &vertical_units, &error);
    if (error) return NULL;
    if ((bytes_per_sample != 2) && (bytes_per_sample != 4)) return NULL;

    int i;
    for (i = 0; i < 190; i++) fgetc(file);

    Elevation_Map *map = new Elevation_Map(columns, rows);


    map->square_size = Vector3(1, 1, 0);
    map->map_size = Vector3(map->num_squares_x, map->num_squares_y, 0);
    map->corner = Vector3(0, 0, 0);

//    int k = 128;
/*
    map->num_squares_x = k;
    map->num_samples_x = k+1;
    map->num_squares_y = k;
    map->num_samples_y = k+1;
*/

    float min_value = FLT_MAX;

    int j;
    for (i = 0; i < columns; i++) {
        for (j = 0; j < rows; j++) {
            int index = map->get_index(i, j);

            float value;
            get_f32(file, &value, &error);
            map->samples[index] = value;
            if (value < min_value) min_value = value;
        }
    }

    for (i = 0; i < columns * rows; i++) {
        map->samples[i] -= min_value;
        map->samples[i] *= 0.05f;
    }

    return map;
}
