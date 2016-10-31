#include "app_shell.h"
#include "profiler_lowlevel.h"
#include "profiler_highlevel.h"

#include <math.h>

#include "os_specific_opengl_headers.h"
#include <gl/glu.h>

Define_Zone(profile_tracker_draw);

const int IMPORTANT_SLOT = 1;
const double SPEEDSTEP_DETECTION_RATIO = 0.08;

bool get_text_color_and_glow_color(float value, float stdev,
                                   Vector3 *text_color_ret,
                                   Vector3 *glow_color_ret,
                                   float *glow_alpha_ret);


void draw_rectangle(float x0, float y0, float x1, float y1) {
    glVertex3f(x0, y0, 0);
    glVertex3f(x1, y0, 0);
    glVertex3f(x1, y1, 0);

    glVertex3f(x0, y0, 0);
    glVertex3f(x1, y1, 0);
    glVertex3f(x0, y1, 0);
}


void Profile_Tracker::draw(float x, float y) {
    Profile_Scope(profile_tracker_draw);

    int slot = IMPORTANT_SLOT;


    // Draw the results.

    extern Font *small_font; // XXXXX

    float sx = x;
    float sy = y;
    float hh = small_font->character_height;




    // Draw the fps counter.

    float avg_frame_time = frame_time.values[slot];
    if (avg_frame_time == 0) avg_frame_time = 0.01f;
    float fps = 1.0f / avg_frame_time;

    char *displayed_quantity_name = "*error*";
    switch (displayed_quantity) {
    case SELF_TIME:
        displayed_quantity_name = "SELF TIME";
        break;
    case SELF_STDEV:
        displayed_quantity_name = "SELF STDEV";
        break;
    case HIERARCHICAL_TIME:
        displayed_quantity_name = "HIERARCHICAL TIME";
        break;
    case HIERARCHICAL_STDEV:
        displayed_quantity_name = "HIERARCHICAL STDEV";
        break;
    }

    static char buf[BUFSIZ];
    sprintf(buf, "fps: %3.2f  (Frame time %3.3fms)  Displaying %s", fps, avg_frame_time * 1000, displayed_quantity_name);
    
    const float header_pad = 4;
    float header_x0 = sx;
    float header_x1 = header_x0 + small_font->get_string_width_in_pixels(buf) + header_pad;

    app_shell->triangle_mode_begin();
    glBegin(GL_TRIANGLES);
    glColor4f(0.1f, 0.3f, 0, 0.85);
    draw_rectangle(header_x0, sy-.5*hh, header_x1, sy+1.5*hh);
    glEnd();
    app_shell->triangle_mode_end();

    app_shell->text_mode_begin(small_font);
    app_shell->draw_text(small_font, sx, sy, buf, 1, 1, 1);
    app_shell->text_mode_end();

    sy -= 2*hh;


    // Detect SpeedStep and warn the user if it is screwing us up.
    sy -= 1.5*hh;

    int ss_slot = 1;
    double ss_val = integer_timestamps_per_second.values[ss_slot];
    double ss_variance = integer_timestamps_per_second.variances[ss_slot] - ss_val*ss_val;
    double ss_stdev = sqrt(fabs(ss_variance));
    float ss_ratio;
    if (ss_val) {
        ss_ratio = ss_stdev / fabs(ss_val);
    } else {
        ss_ratio = 0;
    }

    if (ss_ratio > SPEEDSTEP_DETECTION_RATIO) {
        app_shell->triangle_mode_begin();
        glBegin(GL_TRIANGLES);
        glColor4f(0.9f, 0.1f, 0.1f, 0.85);
        draw_rectangle(header_x0, sy, header_x1, sy+3*hh);
        glEnd();
        app_shell->triangle_mode_end();

        char *warning1 = "WARNING: SpeedStep detected.  Results are unreliable!";
        char *warning2 = "(Try running on a desktop machine instead.)";
        app_shell->text_mode_begin(small_font);
        float len1 = small_font->get_string_width_in_pixels(warning1);
        float len2 = small_font->get_string_width_in_pixels(warning2);
        float offset1 = (header_x1 - header_x0 - len1) * 0.5f;
        float offset2 = (header_x1 - header_x0 - len2) * 0.5f;
        app_shell->draw_text(small_font, sx+offset1, sy +1.5*hh, warning1, 1, 1, 1);
        app_shell->draw_text(small_font, sx+offset2, sy +0.5*hh, warning2, 1, 1, 1);
        app_shell->text_mode_end();


        sy -= 3*hh;
    }


    // Start drawing the actual report.

    int backup_sy = sy;


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
        draw_rectangle(num2_x0, y0, num2_x1, y1);

        sy -= hh;
    }
    glEnd();
    app_shell->triangle_mode_end();

    app_shell->text_mode_begin(small_font);
    sy = backup_sy;

    // Draw the zone data.
    for (i = 0; i < num_active_zones; i++) {
        Profile_Tracker_Data_Record *record = sorted_pointers[i];
        float self_percentage = 100.0 * (record->self_time.values[slot] / frame_time.values[slot]);
        float hier_percentage = 100.0 * (record->hierarchical_time.values[slot] / frame_time.values[slot]);
        assert(hier_percentage > -1.0f);

        float self_ms = 1000 * record->self_time.values[slot];
        float hier_ms = 1000 * record->hierarchical_time.values[slot];

        Program_Zone *zone = Profiling::zone_pointers_by_index[record->index];

        float name_width_used = small_font->get_string_width_in_pixels(zone->name);

        float name_offset = name_column_width - name_width_used - colon_width;

        float value = record->self_time.values[slot];
        float variance = record->self_time.variances[slot];
        variance = variance - value * value;
        if (variance < 0) variance = 0;
        float stdev = sqrt(variance);

        Vector3 text_color, glow_color;
        float glow_alpha;
        bool use_glow = get_text_color_and_glow_color(value, stdev,
                                                      &text_color,
                                                      &glow_color, &glow_alpha);
        if (use_glow) {
            app_shell->draw_text(small_font, name_x0 + name_offset+1, sy-1, zone->name, glow_color.x, glow_color.y, glow_color.z, glow_alpha);
        }

        app_shell->draw_text(small_font, name_x0 + name_offset, sy, zone->name, text_color.x, text_color.y, text_color.z);

        sprintf(buf, "%5.2f", record->displayed_quantity);
        float num1_len = small_font->get_string_width_in_pixels(buf);
        app_shell->draw_text(small_font, num1_x1 - num1_len, sy, buf, text_color.x, text_color.y, text_color.z);

        sprintf(buf, "%.2f", record->entry_count.values[1]);
        float num2_len = small_font->get_string_width_in_pixels(buf);
        app_shell->draw_text(small_font, num2_x1 - num2_len, sy, buf, text_color.x, text_color.y, text_color.z);

        sy -= hh;
    }

    app_shell->text_mode_end();
}
