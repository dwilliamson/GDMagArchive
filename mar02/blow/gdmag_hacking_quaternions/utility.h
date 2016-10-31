void begin_line_mode(float r, float g, float b);
void end_line_mode();
void begin_text_mode();
void end_text_mode();

void begin_bitmap_mode(GLuint texture_handle);
void end_bitmap_mode();

void draw_text_line(int *x, int *y, char *s);
void graph_function_axes();
bool texture_from_file(char *filename, GLuint *result,
                       int *width_result, int *height_result);


const int LETTER_WIDTH = 11;
const int LETTER_HEIGHT = 13;
const int LETTER_PAD = 2;

const double M_PI = 3.14159265358979323846;
const float F_PI = (float)M_PI;



