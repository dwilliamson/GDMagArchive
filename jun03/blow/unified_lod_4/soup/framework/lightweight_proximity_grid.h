#include "fastint.h"
#include "auto_array.h"

struct Lightweight_Proximity_Grid_Square {
    Auto_Array <Vector3 *> vertices;

    void add(Vector3 *position);
    bool remove(Vector3 *position);
};

struct Lightweight_Proximity_Grid {
    ~Lightweight_Proximity_Grid();

    void init(Vector3 corner, Vector3 extents, float grid_square_width);

    void add(Vector3 *position);
    bool remove(Vector3 *position);

    int get_index(Vector3 *position);

    int get_index_x(float real_x);
    int get_index_y(float real_y);
    int get_index_z(float real_z);

    int get_index(int index_x, int index_y, int index_z);


    void sanity_check();  // For debugging!

    Lightweight_Proximity_Grid_Square *grid_squares;

    Vector3 corner;
    Vector3 extents;

    Vector3 square_dimensions;
    Vector3 inverse_square_dimensions;

    int num_squares_x, num_squares_y, num_squares_z;
    int num_items;
};

inline int Lightweight_Proximity_Grid::get_index(int index_x, int index_y, int index_z) {
    int index = index_z * (num_squares_x * num_squares_y)
              + index_y * num_squares_x + index_x;

    assert(index >= 0);
    assert(index < num_squares_x * num_squares_y * num_squares_z);

    return index;
}

inline int Lightweight_Proximity_Grid::get_index_x(float real) {
    int index = FastInt((real - corner.x) * inverse_square_dimensions.x);
    Clamp(index, 0, num_squares_x - 1);
    return index;
}

inline int Lightweight_Proximity_Grid::get_index_y(float real) {
    int index = FastInt((real - corner.y) * inverse_square_dimensions.y);
    Clamp(index, 0, num_squares_y - 1);
    return index;
}

inline int Lightweight_Proximity_Grid::get_index_z(float real) {
    int index = FastInt((real - corner.z) * inverse_square_dimensions.z);
    Clamp(index, 0, num_squares_z - 1);
    return index;
}


inline int Lightweight_Proximity_Grid::get_index(Vector3 *pos) {
    int square_x = get_index_x(pos->x);
    int square_y = get_index_y(pos->y);
    int square_z = get_index_z(pos->z);

    int index = get_index(square_x, square_y, square_z);
    return index;
}




inline void Lightweight_Proximity_Grid::add(Vector3 *pos) {
    int index = get_index(pos);
    grid_squares[index].vertices.add(pos);
    num_items++;
}

inline bool Lightweight_Proximity_Grid::remove(Vector3 *pos) {
    int index = get_index(pos);
    bool found = grid_squares[index].vertices.remove(pos);
    if (found) num_items--;

    return found;
}

