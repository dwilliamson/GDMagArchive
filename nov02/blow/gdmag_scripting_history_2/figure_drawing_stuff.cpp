
void draw_figure3_latitude(float perc) {
    Quaternion q;
    q.set_from_axis_and_angle(1, 0, 0, M_PI * 0.2f);

    if (perc < -1) perc = 0;
    if (perc > 1) perc = 1;

    float z_theta = M_PI * 0.5f * perc;
    float z = sin(z_theta);
    if (z > 1) z = 1;
    if (z < -1) z = -1;

    float r = sqrt(1 - z*z);

    const int LATITUDE_RESOLUTION = 50;
    float dtheta = 2 * M_PI * (1.0 / (double)LATITUDE_RESOLUTION);

    app_shell->line_mode_begin();

    glBegin(GL_LINE_STRIP);

    int i;
    for (i = 0; i <= LATITUDE_RESOLUTION; i++) {
        float theta0 = i * dtheta;
        float theta1 = (i + 1) * dtheta;

        float ct0 = cos(theta0);
        float st0 = sin(theta0);
        float ct1 = cos(theta1);
        float st1 = sin(theta1);

        float x0 = r * ct0;
        float y0 = r * st0;
        float z0 = z;

        float x1 = r * ct1;
        float y1 = r * st1;
        float z1 = z;

        cpoint v(x0, y0, z0);
        v.rotate(q);

        float w = 200;
        v.scale(w);

        glVertex3f(v.x, v.y, v.z);
//        glVertex2f(v.x, v.z);
    }

    glEnd();

    app_shell->line_mode_end();
}

void draw_figure3_longitude(float perc) {
    Quaternion q;
    q.set_from_axis_and_angle(1, 0, 0, M_PI * 0.2f);

    if (perc < 0) perc = 0;
    if (perc > 1) perc = 1;

    float theta = 2 * M_PI * 0.5f * perc;
    float x_equator = cos(theta);
    float y_equator = sin(theta);

    const int LONGITUDE_RESOLUTION = 50;
    float dtheta = 2 * M_PI * (1.0 / (double)LONGITUDE_RESOLUTION);

    app_shell->line_mode_begin();
    glBegin(GL_LINE_STRIP);

    float w = 200;

    int i;
    for (i = 0; i <= LONGITUDE_RESOLUTION; i++) {
        float phi0 = i * dtheta;
        float phi1 = (i + 1) * dtheta;

        float ct0 = cos(phi0);
        float st0 = sin(phi0);
        float ct1 = cos(phi1);
        float st1 = sin(phi1);

        float x0 = x_equator * ct0;
        float y0 = y_equator * ct0;
        float z0 = st0;

        float x1 = x_equator * ct1;
        float y1 = y_equator * ct1;
        float z1 = st1;

        cpoint v(x0, y0, z0);
        v.rotate(q);

        float w = 200;
        v.scale(w);

        glVertex3f(v.x, v.y, v.z);
//        glVertex2f(v.x, v.z);
    }

    glEnd();

    app_shell->line_mode_end();
}

void draw_figure3() {
/*
    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);

    glDisable(GL_BLEND);
*/
    
    glDisable(GL_CULL_FACE);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(-5, 0, 0,  // position
              0, 0, 0,
              0, 0, 1); // up-vector

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(90, 800.0 / 600.0, 0.1f, 1000.0f);





    const int NQUANTA_J = 12;
    const int NQUANTA_I = 12;

    int j;
    for (j = 0; j < NQUANTA_J; j++) {
        draw_figure3_latitude(j / (float)NQUANTA_J);
        draw_figure3_latitude(-j / (float)NQUANTA_J);
    }

    int i;
    for (i = 0; i <= NQUANTA_I; i++) {
        draw_figure3_longitude(i / (float)NQUANTA_I);
    }
}


void draw_circle_at(float x0, float y0, float r) {
    const int NUM_PIECES = 100;
    int i;

    glBegin(GL_LINE_STRIP);

    float dtheta = (1.0 / (float)NUM_PIECES) * 2 * M_PI;
    for (i = 0; i < NUM_PIECES; i++) {
        float perc = i / (float)(NUM_PIECES - 1);
        float theta = dtheta * i;

        float x = cos(theta) * r;
        float y = sin(theta) * r;

        x += x0;
        y += y0;
        glVertex2f(x, y);
    }
    
    glEnd();
}

void draw_arrow(float x0, float y0, float x1, float y1) {
    glBegin(GL_LINE_STRIP);
    glVertex2f(x0, y0);
    glVertex2f(x1, y1);
    glEnd();
}

void draw_hexagon(float x0, float y0, float r) {
    glBegin(GL_LINE_STRIP);

    int i;
    float dtheta = (1.0 / 6.0) * 2 * M_PI;
    for (i = 0; i <= 6; i++) {
        float theta = i * dtheta;
        float x = cos(theta) * r;
        float y = sin(theta) * r;

        x += x0;
        y += y0;
        glVertex2f(x, y);
    }

    glEnd();
}

const float square_width = 160;

void draw_figure2() {
    float hex_r = square_width * sqrt(sqrt(12.0)) / 3.0;
    float hex_s = hex_r * sqrt(3.0) / 2.0;

    float ox = 300;
    float oy = ox;

    app_shell->line_mode_begin();
    glLineWidth(3);
    glColor3f(0, 1, 0);
    draw_hexagon(ox, oy, hex_r);
    draw_hexagon(ox-3*hex_r, oy, hex_r);
    draw_hexagon(ox+3*hex_r, oy, hex_r);
    draw_hexagon(ox+6*hex_r, oy, hex_r);

    draw_hexagon(ox, oy+2*hex_s, hex_r);
    draw_hexagon(ox-3*hex_r, oy+2*hex_s, hex_r);
    draw_hexagon(ox+3*hex_r, oy+2*hex_s, hex_r);
    draw_hexagon(ox+6*hex_r, oy+2*hex_s, hex_r);

    draw_hexagon(ox, oy-2*hex_s, hex_r);
    draw_hexagon(ox-3*hex_r, oy-2*hex_s, hex_r);
    draw_hexagon(ox+3*hex_r, oy-2*hex_s, hex_r);
    draw_hexagon(ox+6*hex_r, oy-2*hex_s, hex_r);

    float ox1 = ox+1.5*hex_r;
    float oy1 = oy+hex_s;
    draw_hexagon(ox1, oy1, hex_r);
    draw_hexagon(ox1-3*hex_r, oy1, hex_r);
    draw_hexagon(ox1+3*hex_r, oy1, hex_r);
    draw_hexagon(ox1+6*hex_r, oy1, hex_r);
    draw_hexagon(ox1, oy1+2*hex_s, hex_r);
    draw_hexagon(ox1-3*hex_r, oy1+2*hex_s, hex_r);
    draw_hexagon(ox1+3*hex_r, oy1+2*hex_s, hex_r);
    draw_hexagon(ox1+6*hex_r, oy1+2*hex_s, hex_r);
    draw_hexagon(ox1, oy1-2*hex_s, hex_r);
    draw_hexagon(ox1-3*hex_r, oy1-2*hex_s, hex_r);
    draw_hexagon(ox1+3*hex_r, oy1-2*hex_s, hex_r);
    draw_hexagon(ox1+6*hex_r, oy1-2*hex_s, hex_r);
    draw_hexagon(ox1, oy1-4*hex_s, hex_r);
    draw_hexagon(ox1-3*hex_r, oy1-4*hex_s, hex_r);
    draw_hexagon(ox1+3*hex_r, oy1-4*hex_s, hex_r);
    draw_hexagon(ox1+6*hex_r, oy1-4*hex_s, hex_r);
    
    glColor3f(1, 1, 1);
    draw_circle_at(ox, oy, square_width * 0.05f);

    float ax = ox + hex_r * cos(M_PI / 3.0);
    float ay = oy + hex_r * sin(M_PI / 3.0);

    glColor3f(1, 1, 0);
    draw_arrow(ox, oy, ax, ay);

    app_shell->line_mode_end();
}

void draw_figure1() {
    app_shell->line_mode_begin();
    glLineWidth(3.0f);
    glColor3f(0, 1, 0);


    float ox = 300;
    float oy = ox;

    int i0 = -2;
    int i1 = 3;
    int j0 = -2;
    int j1 = 3;

    int i;
    for (i = i0; i <= i1; i++) {
        float x = ox - square_width * 0.5f + i * square_width;
        float y0 = oy - square_width * 0.5f + j0 * square_width;
        float y1 = oy - square_width * 0.5f + j1 * square_width;

        glBegin(GL_LINE_STRIP);
        glVertex2f(x, y0);
        glVertex2f(x, y1);
        glEnd();
    }

    int j;
    for (j = j0; j <= j1; j++) {
        float y = oy - square_width * 0.5f + j * square_width;
        float x0 = ox - square_width * 0.5f + i0 * square_width;
        float x1 = ox - square_width * 0.5f + i1 * square_width;

        glBegin(GL_LINE_STRIP);
        glVertex2f(x0, y);
        glVertex2f(x1, y);
        glEnd();
    }


    glColor3f(1, 1, 1);
    draw_circle_at(ox, oy, square_width * 0.05f);

    glColor3f(1, 1, 0);
    draw_arrow(ox, oy, ox + square_width*.5, oy + square_width*.5);

    app_shell->line_mode_end();
}
