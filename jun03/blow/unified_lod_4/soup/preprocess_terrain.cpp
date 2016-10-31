#include "framework.h"
#include "make_world.h"
#include "terrain_config.h"

#include <stdio.h>
#include <stdlib.h>

#include "seam_database.h"

Seam_Database *seam_database;

void main(int argc, char **argv) {
    Elevation_Map *map;
    char *filename = NULL;

    seam_database = new Seam_Database();

    if (argc > 1) {
        filename = argv[1];
    } else {
        printf("You didn't supply a filename, so I am making a terrain\n");
        printf("out of some sine-wave bumps.  Elevation map is %d samples across,\n", ELEVATION_MAP_SAMPLES);
        printf("leaf blocks are %d samples across.\n", BLOCK_SAMPLES);
        printf("Frequency in the X direction is %.3f, frequency in the Y direction is %.3f\n", BUMP_F1, BUMP_F2);
        printf("Bumps are %.3f units high.\n", BUMP_HEIGHT);
        printf("\nYou can adjust these settings by editing terrain_config.h and recompiling.\n\n");
    }

    if (filename == NULL) {
        map = make_dummy_elevation_map(1, ELEVATION_MAP_SAMPLES,
                                       BUMP_F1, BUMP_F2, BUMP_HEIGHT);
    } else {
        map = load_elevation_map(filename);
    }

    if (map == NULL) {
        assert(filename);
        fprintf(stderr, "Unable to open map file '%s'!\n", filename);
        exit(1);
    }

    char *output_filename = "output.terrain_tree";
    FILE *f = fopen(output_filename, "wb");
    if (f == NULL) {
        fprintf(stderr, "Unable to open output file '%s' for writing!\n",
                output_filename);
        exit(1);
    }

    build_tangent_frames(map);


    World_Block *tree = make_static_terrain_block(map, BLOCK_SAMPLES);

    save_terrain(tree, seam_database, f);
    fclose(f);
    printf("Saved result as '%s'.\n", output_filename);

    // We don't bother deleting seam_database here.
    exit(0);
}
