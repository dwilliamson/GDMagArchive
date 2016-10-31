#include "../framework.h"
#include "lightweight_proximity_grid.h"
#include <math.h>

Lightweight_Proximity_Grid::~Lightweight_Proximity_Grid() {
    if (grid_squares) delete [] grid_squares;
}

void Lightweight_Proximity_Grid::init(Vector3 _corner, Vector3 _extents, 
                          float _grid_square_width) {
    assert(_grid_square_width > 0);

    float w = _grid_square_width;
    float iw = 1.0 / w;

    square_dimensions.set(w, w, w);
    inverse_square_dimensions.set(iw, iw, iw);

    corner = _corner;
    extents = _extents;


    num_squares_x = (int)ceil(extents.x * iw);
    num_squares_y = (int)ceil(extents.y * iw);
    num_squares_z = (int)ceil(extents.z * iw);

    if (num_squares_x < 1) num_squares_x = 1;
    if (num_squares_y < 1) num_squares_y = 1;
    if (num_squares_z < 1) num_squares_z = 1;

    grid_squares = new Lightweight_Proximity_Grid_Square[num_squares_x * num_squares_y * num_squares_z];

    num_items = 0;
}


void Lightweight_Proximity_Grid::sanity_check() {
    int num_squares = num_squares_x * num_squares_y * num_squares_z;
    assert(num_squares > 0);

    int sum = 0;

    int i;
    for (i = 0; i < num_squares; i++) {
        Lightweight_Proximity_Grid_Square *square = &grid_squares[i];
        sum += square->vertices.live_items;
    }

    assert(sum == num_items);
}
