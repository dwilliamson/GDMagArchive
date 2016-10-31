#include "app_shell.h"
#include "profiler_lowlevel.h"
#include "profiler_highlevel.h"
#include "feature_map.h"
#include "statistics.h"

#include <math.h>

#include "os_specific_opengl_headers.h"
#include <gl/glu.h>

extern Font *small_font; // XXXXX

Vector3 get_color(float value, float stdev);

// Callbacks into non-render file...
void do_scratch_colors_for_coordinate(Feature_Map *feature_map, Vector3 *scratch_colors, int basis_vector, float scale_r, float scale_g, float scale_b, bool modulate = false);


void draw_rectangle(float x0, float y0, float x1, float y1) {
    glVertex3f(x0, y0, 0);
    glVertex3f(x1, y0, 0);
    glVertex3f(x1, y1, 0);

    glVertex3f(x0, y0, 0);
    glVertex3f(x1, y1, 0);
    glVertex3f(x0, y1, 0);
}



void draw_column_stripes(int num_active_zones, float sx, float sy, float hh,
                         bool do_third_column,
                         float *name_x1_ret, float *num1_x1_ret, 
                         float *num2_x1_ret) {
    const float name_column_width = 250;
    const float column_pad = 4;
    float number_column_width = small_font->get_string_width_in_pixels("000.0000");

    float name_x0 = sx;
    float name_x1 = name_x0 + name_column_width;

    float num1_x0 = name_x1 + column_pad;
    float num1_x1 = num1_x0 + number_column_width;

    float num2_x0 = num1_x1 + column_pad;
    float num2_x1 = num2_x0 + number_column_width;

    float colon_width = small_font->get_string_width_in_pixels(":");


    // Draw the background colors for the zone data.
    app_shell->triangle_mode_begin();
    glBegin(GL_TRIANGLES);
    int i;
    for (i = 0; i < num_active_zones; i++) {
        if (i & 1) {
            glColor4f(0, 0.4f, 0, 0.85);
        } else {
            glColor4f(0.2f, 0.2f, 0, 0.85);
        }

        float y0, y1;

        y0 = sy;
        y1 = sy + hh;

        draw_rectangle(name_x0, y0, name_x1, y1);
        draw_rectangle(num1_x0, y0, num1_x1, y1);
        if (do_third_column) draw_rectangle(num2_x0, y0, num2_x1, y1);

        sy -= hh;
    }
    glEnd();
    app_shell->triangle_mode_end();


    *num1_x1_ret = num1_x1;
    *num2_x1_ret = num2_x1;
    
    *name_x1_ret = name_x1 - colon_width;
}


void draw_colored_feature_map(Feature_Map *feature_map, float x, float y, float w, float h,
                              Vector3 *colors, int *selection_result = NULL) {

    // Draw a black background for the feature map.
    app_shell->triangle_mode_begin();

    glBegin(GL_QUADS);
    glColor3f(0, 0, 0);
    glVertex2f(x, y);
    glVertex2f(x+w, y);
    glVertex2f(x+w, y+h);
    glVertex2f(x, y+h);
    glEnd();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Now loop over everything and 
    // Draw the feature map intensities.
    glBegin(GL_QUADS);

    int stride = feature_map->num_map_buckets_per_side;
    float k = w / (float)stride;

    int i, j;
    for (j = 0; j < stride; j++) {
        for (i = 0; i < stride; i++) {
            int index = j * stride + i;

            float cr = sqrt(colors[index].x);
            float cg = sqrt(colors[index].y);
            float cb = sqrt(colors[index].z);
            glColor4f(cr, cg, cb, 1.0f);

            float x0 = x+i*k;
            float x1 = x0+k;
            float y0 = y+j*k;
            float y1 = y0+k;

            glVertex2f(x0, y0);
            glVertex2f(x1, y0);
            glVertex2f(x1, y1);
            glVertex2f(x0, y1);

            if (app_shell->mouse_button_is_down && selection_result) {
                float mx = app_shell->mouse_pointer_x;
                float my = app_shell->mouse_pointer_y;
                if ((mx >= x0) && (mx <= x1) && (my >= y0) && (my <= y1)) {
                    *selection_result = index;
                }
            }

        }
    }
    glEnd();

    app_shell->triangle_mode_end();
}

void draw_single_coordinate_feature_map(Feature_Map *feature_map, float x, float y, float w, float h,
                                       int basis_vector, char *name, Vector3 *scratch_colors,
                                       Map_Selection *selection_array) {

    float scale_r, scale_g, scale_b;

    float mx = app_shell->mouse_pointer_x;
    float my = app_shell->mouse_pointer_y;

    Map_Selection *selection = &selection_array[basis_vector];
    if ((mx >= x) && (mx <= x+w) && (my >= y) && (my <= y+w)) {
        scale_r = scale_g = scale_b = 1.0f;

        if (app_shell->mouse_button_is_down) {
            selection->mouse_button_is_down = true;
        } else {
            if (selection->mouse_button_is_down) {
                selection->mouse_button_is_down = false;
                selection->selected = !selection->selected;
            }
        }

    } else {
        selection->mouse_button_is_down = false;
        scale_r = scale_g = scale_b = 0.7f;
    }


    do_scratch_colors_for_coordinate(feature_map, scratch_colors, basis_vector,
                                     scale_r, scale_g, scale_b);



    // If selected, draw a really big background square to serve as outline...
    // if you cared about fill rate you'd make this be 4 rectangles instead
    // of one big one.
    if (selection->selected) {
        app_shell->triangle_mode_begin();
        float m = 4;
        glBegin(GL_QUADS);
        glColor3f(0, 1, 0);
        glVertex2f(x-m, y-m);
        glVertex2f(x+w+m, y-m);
        glVertex2f(x+w+m, y+h+m);
        glVertex2f(x-m, y+h+m);
        glEnd();
        app_shell->triangle_mode_end();
    }


    draw_colored_feature_map(feature_map, x, y, w, h, scratch_colors);


    app_shell->text_mode_begin(small_font);
    float hh = small_font->character_height;

    app_shell->draw_text(small_font, x, y - hh, name, 1, 1, 1, 1, w);
    app_shell->text_mode_end();
}


inline float ratio_from_mean(float dot, Statistics *statistics) {
    float big_dev = fabs(statistics->mean - statistics->min);
    float big_dev2 = fabs(statistics->mean - statistics->max);
    if (big_dev2 > big_dev) big_dev = big_dev2;

    if (big_dev == 0) return 0;

    float result = fabs(dot / big_dev);
    return result;
}

void Profile_Tracker::draw_basis_vector_report(Feature_Map_Vector *vector,
                                               float x, float y, Vector3 *color) {
//    glColor3f(colorx, color.y, color.z);

    char buf[2000]; // XXXX
    buf[0] = 0;

    feature_map->scratch_vector->copy_from(vector);
    vector = feature_map->scratch_vector;

    vector->sort_by_magnitude();

    float factor = 1;
    if (vector->coordinates[0].magnitude < 0) factor = -1;

    float sum_total = 0;
    int i;
    for (i = 0; i < vector->num_dimensions; i++) {
        sum_total += fabs(vector->coordinates[i].magnitude);
    }

    if (sum_total == 0) return;

    for (i = 0; i < 5; i++) {
        if (i >= vector->num_dimensions) break;
        if (i != 0) strcat(buf, ", ");

        float val = vector->coordinates[i].magnitude * factor;
        float perc = val / sum_total;

        int basis_index = vector->coordinates[i].basis_vector;
        strcat(buf, Profiling::zone_pointers_by_index[basis_index]->name);
        char buf2[100];
        sprintf(buf2, " %2.1f%%", perc * 100);
        strcat(buf, buf2);
    }

    app_shell->draw_text(small_font, x, y, buf, color->x, color->y, color->z);
}

void Profile_Tracker::draw_feature_map(float x, float y, float w, float h) {
    int stride = feature_map->num_map_buckets_per_side;

    float k = w / (float)stride;

    app_shell->triangle_mode_begin();

    const float PIP_THRESHOLD_MIN = 0.01f;

    // Draw a black background for the feature map.
    glBegin(GL_QUADS);
    glColor3f(0, 0, 0);
    glVertex2f(x, y);
    glVertex2f(x+w, y);
    glVertex2f(x+w, y+h);
    glVertex2f(x, y+h);
    glEnd();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    // Compute the feature map intensities.
    // First loop over everything and find the statistics.
    int i, j;
    Statistics statistics;
    for (j = 0; j < stride; j++) {
        for (i = 0; i < stride; i++) {
            int index = j * stride + i;
            statistics.add(feature_map->hit_counts[index].values[0]);
        }
    }

    statistics.finish();
    
    float max = statistics.max;
    if (max == 0) max = 1;
    float imax = 1.0 / fabs(max);


//    if (!app_shell->mouse_button_is_down) selected_map_index = -1;

    // Now loop over everything and compute feature map blueness thingy.
    for (j = 0; j < stride; j++) {
        for (i = 0; i < stride; i++) {
            int index = j * stride + i;

            float val;
            val = feature_map->hit_counts[index].values[0];
            val = val * imax;

            scratch_colors[index] = Vector3(0, 0, 0.75f * val);
            if (selected_map_index == index) scratch_colors[index] = Vector3(0.9f*val, 0, 0.75f*val);

            if ((i == feature_map_cursor_x) && (j == feature_map_cursor_y)) {
                scratch_colors[index] = Vector3(1, 0, 0);
                if (selected_map_index == index) scratch_colors[index] = Vector3(1, 0, 0.9f);
            }
        }
    }

    draw_colored_feature_map(feature_map, x, y, w, h, scratch_colors, &selected_map_index);


    // Draw the modification pips.

    glBegin(GL_QUADS);
    for (j = 0; j < stride; j++) {
        for (i = 0; i < stride; i++) {
            int index = j * stride + i;

            float val;
            val = feature_map->mod_times[index].values[2];

            glColor4f(1, 1, 1, val);

            float x0 = x+i*k;
            float x1 = x0+k;
            float y0 = y+j*k;
            float y1 = y0+k;

            float b = k * 0.2f;
            x0 += b;
            x1 -= b;
            y0 += b;
            y1 -= b;



            // Draw the hardness.
            float adjusted_hardness = sqrt(feature_map->hardnesses[index]);
            glColor4f(0.5f, 0, 0, adjusted_hardness);
            glVertex2f(x0+b, y0-b);
            glVertex2f(x1+b, y0-b);
            glVertex2f(x1+b, y1-b);
            glVertex2f(x0+b, y1-b);

            // Draw the pip.
            if (val >= PIP_THRESHOLD_MIN) {
                glColor4f(1, 1, 1, val);
                glVertex2f(x0, y0);
                glVertex2f(x1, y0);
                glVertex2f(x1, y1);
                glVertex2f(x0, y1);
            }
        }
    }
    glEnd();

    glDisable(GL_BLEND);

    float original_x = x;

    x += w;
    if (selected_map_index != -1) {
        Feature_Map_Vector *center = feature_map->cluster_vectors[selected_map_index];

        Statistics statistics;

        // Draw a map that is shaded by difference (bright = different, dark = similar)
        int num_vectors = stride * stride;

        int i;
        for (i = 0; i < num_vectors; i++) {
            float dist = distance(center, feature_map->cluster_vectors[i]);
            statistics.add(dist);
        }
        statistics.finish();

        if (statistics.max > statistics.min) {
            float idenom = 1.0 / (statistics.max - statistics.min);

            for (i = 0; i < num_vectors; i++) {
                // Maybe store distance in a tmp vector instead of computing it twice like this.
                float dist = distance(center, feature_map->cluster_vectors[i]);
                float val = (dist - statistics.min) * idenom;
                scratch_colors[i] = Vector3(val, val, val);
            }

            draw_colored_feature_map(feature_map, x, y, w, h, scratch_colors);
        }

        // Draw a map that shows the top 3 directions of difference.

        const int MAX_BASES = 3;
        feature_map->find_spread_vectors(feature_map->cluster_vectors[selected_map_index],
                                         feature_map->cluster_vectors, num_vectors, MAX_BASES);

        float red, green, blue;
        Feature_Map *map = feature_map;
        Feature_Map_Vector *scratch = map->scratch_vector;

        Statistics stat0, stat1, stat2;

        // I am confused about why I need to do this, but let's give it a shot.
        for (i = 0; i < num_vectors; i++) {
            vector_add(map->cluster_vectors[i], map->cluster_vectors[selected_map_index], scratch, -1);
            float dot0 = dot_product(scratch, map->spread_basis_vectors[0]);
            float dot1 = dot_product(scratch, map->spread_basis_vectors[1]);
            float dot2 = dot_product(scratch, map->spread_basis_vectors[2]);
            stat0.add(dot0);
            stat1.add(dot1);
            stat2.add(dot2);
        }

        stat0.finish();
        stat1.finish();
        stat2.finish();

        for (i = 0; i < num_vectors; i++) {
            vector_add(map->cluster_vectors[i], map->cluster_vectors[selected_map_index], scratch, -1);

            float dot0 = dot_product(scratch, map->spread_basis_vectors[0]);
            float dot1 = dot_product(scratch, map->spread_basis_vectors[1]);
            float dot2 = dot_product(scratch, map->spread_basis_vectors[2]);
            red = ratio_from_mean(dot0, &stat0);
            green = ratio_from_mean(dot1, &stat1);
            blue = ratio_from_mean(dot2, &stat2);
            
            // These should not happen!!
            if (red > 1) red = 1;
            if (green > 1) green = 1;
            if (blue > 1) blue = 1; 

            scratch_colors[i] = Vector3(red, green, blue);
        }

        x += w;
        draw_colored_feature_map(feature_map, x, y, w, h, scratch_colors);

        app_shell->text_mode_begin(small_font);

        for (i = 0; i < map->num_spread_basis_vectors; i++) {
            y -= small_font->character_height;

            Vector3 color(1, 1, 1);
            if (i == 0) color = Vector3(1, 0, 0);
            if (i == 1) color = Vector3(0, 1, 0);
            if (i == 2) color = Vector3(0, 0, 1);

            draw_basis_vector_report(map->spread_basis_vectors[i], x-2*w, y, &color);
        }

        app_shell->text_mode_end();
    }

    if (!draw_feature_map_report) return;

    app_shell->triangle_mode_begin();

    x = original_x;

    int index = feature_map->get_index(feature_map_cursor_x, feature_map_cursor_y);
    if (index == -1) index = selected_map_index;
    if (index == -1) return;

    int num_dimensions = feature_map->num_vector_dimensions;
    float hh = small_font->character_height;
    float sx = x;
    float sy = y - 2*hh;
    float name_x1, num1_x1, num2_x1;
    draw_column_stripes(num_active_zones, sx, sy, hh, false,
                        &name_x1, &num1_x1, &num2_x1);

    // Draw usage statistics of this vector (what percentage
    // of the time it is hit, as opposed to the rest).
    
    float hit_total = feature_map->total_hits;
    if (hit_total == 0) hit_total = 1;
    float hit_count = feature_map->hit_counts[index].values[0];
    float hit_percentage = 100.0 * (hit_count / hit_total);

    char buf[1000]; // XXX
    sprintf(buf, "Occurred %3.2f%% of the time.", hit_percentage);
    app_shell->text_mode_begin(small_font);
    app_shell->draw_text(small_font, sx, sy + hh, buf, 1, 1, 1);
    app_shell->text_mode_end();

    // Now actually draw the stats for this vector.

    input_vector->copy_from(feature_map->cluster_vectors[index]);
    input_vector->sort_by_magnitude();

    glColor3f(1, 1, 1);
    app_shell->text_mode_begin(small_font);
    for (i = 0; i < num_dimensions; i++) {
        int basis = input_vector->coordinates[i].basis_vector;
        float mag = input_vector->coordinates[i].magnitude;

        Profile_Tracker_Data_Record *record = &data_records[basis];
        Program_Zone *zone = Profiling::zone_pointers_by_index[record->index];

        Vector3 vcolor(1, 1, 1);
        float name_len = small_font->get_string_width_in_pixels(zone->name);
        app_shell->draw_text(small_font, name_x1 - name_len, sy, zone->name, 1, 1, 1);
        char buf[128]; // XXX!
        sprintf(buf, "%3.2fms", mag * 1000.0f);

        float num1_len = small_font->get_string_width_in_pixels(buf);
        app_shell->draw_text(small_font, num1_x1 - num1_len, sy, buf, vcolor.x, vcolor.y, vcolor.z);

        sy -= hh;
    }        
    app_shell->text_mode_end();


    // Draw the single-coordinate breakdowns.

    float pad = 4;
    sx = pad;
    sy = app_shell->screen_height - h - pad;
    for (i = 0; i < 6; i++) {
        int xref = input_vector->coordinates[i].basis_vector;
        Profile_Tracker_Data_Record *record = &data_records[xref];
        Program_Zone *zone = Profiling::zone_pointers_by_index[record->index];
        draw_single_coordinate_feature_map(feature_map, sx, sy, w, h, xref, zone->name, scratch_colors, map_selection_array);
        sx += w + 3*pad;
    }

    // If we have any single-coordinate maps selected, draw the AND of
    // all of them.
    static char desc_buf[2000]; // XXXXX
    desc_buf[0] = 0;
    int maps_counted = 0;
    for (i = 0; i < feature_map->num_vector_dimensions; i++) {
        Map_Selection *selection = &map_selection_array[i];
        if (selection->selected) {
            char *name = Profiling::zone_pointers_by_index[i]->name;

            bool modulate;
            if (maps_counted == 0) {
                strcat(desc_buf, name);
                modulate = false;
            } else {
                strcat(desc_buf, " AND ");
                strcat(desc_buf, name);
                modulate = true;
            }

            do_scratch_colors_for_coordinate(feature_map, scratch_colors, i, 1, 1, 1, modulate);

            maps_counted++;
        }
    }

    sx = pad;
    sy = app_shell->screen_height - h - 2*pad - 2*hh;
    if (maps_counted) {
        app_shell->draw_text(small_font, sx, sy, desc_buf, 1, 1, 1);

        sy -= h + pad;
        draw_colored_feature_map(feature_map, sx, sy, w, h, scratch_colors);
    }
}

void Profile_Tracker::draw_text(float x, float y, char *buf, float r, float g, float b, float a, float max_width) {
    if (app_shell->draw_mode != App_Shell::MODE_TEXT) app_shell->text_mode_begin(small_font);
    app_shell->draw_text(small_font, x, y, buf, r, g, b, a, max_width);
}

void Profile_Tracker::set_draw_color(float r, float g, float b, float a) {
    if (app_shell->draw_mode != App_Shell::MODE_TRIANGLE) app_shell->triangle_mode_begin();
    glColor4f(r, g, b, a);
    draw_color_r = r;
    draw_color_g = g;
    draw_color_b = b;
    draw_color_a = a;
}

void Profile_Tracker::draw_colored_rectangle(float x0, float y0, float x1, float y1) {
    if (app_shell->draw_mode != App_Shell::MODE_TRIANGLE) {
        app_shell->triangle_mode_begin();
        glColor4f(draw_color_r, draw_color_g, draw_color_b, draw_color_a);
    }

    glBegin(GL_TRIANGLES);
    draw_rectangle(x0, y0, x1, y1);
    glEnd();
}
