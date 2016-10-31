// @Cleanliness: Lightweight_Proximity_Grid should have an
// abstracted find() function.  That would be nice.

#include "../framework.h"
#include "mesh_reducer.h"
#include "lightweight_proximity_grid.h"
#include "priority_queue.h"
#include "../mesh.h"
#include "error_quadric.h"
#include "mesh_topology_handler.h"
#include "mesh_builder.h"

#include <float.h>
#include <math.h>

extern float GetArea(Vector3 v0, Vector3 v1, Vector3 v2);

struct Reducer_Priority_Queue_Data {
    int vertex0;
    int vertex1;
};

inline int go_to_next_coincident_vertex(Mesh_Topology_Handler *handler, int cursor) {
    assert(cursor < handler->mesh->num_vertices);
    int next = handler->vertex_coincidence_chain[cursor];
    return next;
}

void get_vertex_uv(Triangle_List_Mesh *mesh, int index, Vector3 *result) {
    result->x = mesh->uvs[index].x;
    result->y = mesh->uvs[index].y;
    result->z = 0;
}

void get_vertex_uv(Triangle_List_Mesh *mesh, int index, float *u_ret, float *v_ret) {
    *u_ret = mesh->uvs[index].x;
    *v_ret = mesh->uvs[index].y;
}

void get_vertex_position(Triangle_List_Mesh *mesh, int index, Vector3 *result) {
    *result = mesh->vertices[index];
}

void Mesh_Reducer::fill_point(float *point, int vertex_index, int material_index) {
    float uv_weight = tuning.texture_space_importance_factor;

    if (material_index != -1) {
        float specific_material_factor = material_factor_uv_over_xyz[material_index];
        uv_weight *= specific_material_factor;
    }

    point[0] = mesh->vertices[vertex_index].x;
    point[1] = mesh->vertices[vertex_index].y;
    point[2] = mesh->vertices[vertex_index].z;
    point[3] = mesh->uvs[vertex_index].x * uv_weight;
    point[4] = mesh->uvs[vertex_index].y * uv_weight;
}



void Mesh_Reducer::count_material_areas() {
    // Initialize the ratio of UV space to XYZ space for
    // each material.

    // First build the material arrays and initialize them to 0.

    material_area_uv = new float[mesh->num_materials];
    material_area_xyz = new float[mesh->num_materials];
    material_factor_uv_over_xyz = new float[mesh->num_materials];

    int i;
    for (i = 0; i < mesh->num_materials; i++) {
        material_area_uv[i] = 0;
        material_area_xyz[i] = 0;
        material_factor_uv_over_xyz[i] = 0;
    }

    // Now we iterate over all the faces and add up corresponding
    // areas.  Etc.

    for (i = 0; i < mesh->num_faces; i++) {
        Reducer_Face *face = &topology_handler->faces[i];
        int material_index = face->material;

        int N0 = face->indices[0];
        int N1 = face->indices[1];
        int N2 = face->indices[2];

        Vector3 pos0, pos1, pos2;
        Vector3 tex0, tex1, tex2;

        get_vertex_position(mesh, N0, &pos0);
        get_vertex_position(mesh, N1, &pos1);
        get_vertex_position(mesh, N2, &pos2);

        get_vertex_uv(mesh, N0, &tex0);
        get_vertex_uv(mesh, N1, &tex1);
        get_vertex_uv(mesh, N2, &tex2);

        float XYZArea = GetArea(pos0, pos1, pos2);
        float UVArea = GetArea(tex0, tex1, tex2);

        material_area_xyz[material_index] += XYZArea;
        material_area_uv[material_index] += UVArea;
    }

    for (i = 0; i < mesh->num_materials; i++) {
        if (material_area_uv[i]) {
            material_factor_uv_over_xyz[i] = sqrt(material_area_xyz[i] / material_area_uv[i]);
        } else {
            material_factor_uv_over_xyz[i] = 0;
        }
    }
}


void Mesh_Reducer::add_face_constraint_to_quadrics(Reducer_Face *face) {
    float point0[ERROR_QUADRIC_DIMENSIONS];
    float point1[ERROR_QUADRIC_DIMENSIONS];
    float point2[ERROR_QUADRIC_DIMENSIONS];

    fill_point(point0, face->indices[0], face->material);
    fill_point(point1, face->indices[1], face->material);
    fill_point(point2, face->indices[2], face->material);

    Error_Quadric *quadric0 = get_quadric(face->indices[0]);
    Error_Quadric *quadric1 = get_quadric(face->indices[1]);
    Error_Quadric *quadric2 = get_quadric(face->indices[2]);

    quadric0->accumulate_plane(point0, point1, point2);
    quadric1->accumulate_plane(point0, point1, point2);
    quadric2->accumulate_plane(point0, point1, point2);
}

void Mesh_Reducer::init_quadrics() {
    count_material_areas();
    error_quadrics = new Error_Quadric[mesh->num_vertices];

    int i;
    for (i = 0; i < mesh->num_vertices; i++) {
        error_quadrics[i].clear();
    }

    error_quadrics[0].init_index_table();  // Set up the static data.

    for (i = 0; i < topology_handler->num_faces_remaining; i++) {
        Reducer_Face *face = &topology_handler->faces[i];

        if (!(face->flags & FACE_IS_A_SEAM_FILL)) {
            add_face_constraint_to_quadrics(&topology_handler->faces[i]);
        }
    }
}

Error_Quadric *Mesh_Reducer::get_quadric(int index) {
    return &error_quadrics[index];
}

Mesh_Reducer::Mesh_Reducer() {
    tuning.material_boundary_penalty = 1000.0f;
    tuning.topology_change_penalty = 2.0f;
    tuning.icky_face_penalty = 1000.0f;
    tuning.texture_space_importance_factor = 2.0f;
    tuning.lonely_edge_constraint_factor = 3.0f;

    mesh = NULL;
    priority_queue = NULL;
    proximity_grid = NULL;
    error_quadrics = NULL;

    num_target_faces = 1000;
    num_lonely_edges_detected = 0;

    topology_handler = NULL;
}

Mesh_Reducer::~Mesh_Reducer() {
    delete proximity_grid;
    delete priority_queue;
    delete topology_handler;
    delete [] error_quadrics;
    delete [] material_area_uv;
    delete [] material_area_xyz;
    delete [] material_factor_uv_over_xyz;
}

float Mesh_Reducer::compute_collapse_error(int index0, int index1) {
    // Step through every alias of index0.  Find best match
    // among aliases of index1.  Compute error due to the move,
    // accumulate to sum.

    // When done, divide sum by number of index0 aliases to
    // compute an average.

    float error_sum = 0;
    float penalty = 1;

    int first_alias = index0;
    int cursor = index0;
    int vertices_considered = 0;

    while (1) {
        int dest = topology_handler->simple_find_alias_to_map_to(cursor, index1);
        if (dest == -1) {
            // Material boundary detected.  There's no right answer
            // here, so let's just collapse to the canonical vertex,
            // and jack this reduction by a big penalty.  
            // (@Improvement: We might
            // in future want to try to find the vertex with the 
            // least amount of error, or something.)
            
            dest = index1;
            penalty = tuning.material_boundary_penalty;
        }

        Error_Quadric *quadric0 = get_quadric(cursor);
        Error_Quadric *quadric1 = get_quadric(dest);
        assert(quadric0 != quadric1);

        float pos_1_array[ERROR_QUADRIC_DIMENSIONS];
        fill_point(pos_1_array, dest, topology_handler->material_touched_by_vertex[dest]);

        float error = quadric0->evaluate_error(pos_1_array);
        error_sum += error;

        vertices_considered++;

        cursor = topology_handler->vertex_coincidence_chain[cursor];
        if (cursor == first_alias) break;
    }

    if (vertices_considered == 0) return 0;

    if (tuning.topology_change_penalty) {
        const float TOPOLOGY_CHANGE_EPSILON = 0.5f; // XXX arbitrary
        if (topology_handler->collapse_changes_topology(index0, index1)) {
            error_sum += TOPOLOGY_CHANGE_EPSILON;
            error_sum *= tuning.topology_change_penalty;
        }
    }

    if (topology_handler->collapse_creates_icky_face(index0, index1)) {
        return FLT_MAX;
    }

    return (error_sum * penalty) / (float)vertices_considered;
//    return (error_sum * penalty);
}

void Mesh_Reducer::update_best_candidates(int index0,
                                          int index1,
                                          float *best_error_result,
                                          int *best_v1_result) {
    if (index0 == index1) return;

    assert(topology_handler->vertex_flags[index0] & VERTEX_IS_LIVE);
    assert(topology_handler->vertex_flags[index1] & VERTEX_IS_LIVE);

    float Error = compute_collapse_error(index0, index1);

    if (Error < *best_error_result) {
        *best_error_result = Error;
        *best_v1_result = index1;
    }
}

int get_vertex_index(Triangle_List_Mesh *mesh, Vector3 *pos) {
    int index = pos - mesh->vertices;
    assert(index >= 0);
    assert(index < mesh->num_vertices);

    return index;
}

inline int get_index_within_face(Triangle_List_Mesh *mesh, Reducer_Face *face, int vertex_index) {
    int *canonical = mesh->canonical_vertex_map;
    assert(canonical[vertex_index] == vertex_index);

    int where_within_face;

    if (canonical[face->indices[0]] == vertex_index) where_within_face = 0;
    else if (canonical[face->indices[1]] == vertex_index) where_within_face = 1;
    else if (canonical[face->indices[2]] == vertex_index) where_within_face = 2;
    else where_within_face = -1;

    return where_within_face;
}

inline bool Mesh_Reducer::vertex_is_marked(int index) {
    return topology_handler->vertex_flags[index] & VERTEX_IS_MARKED;
}

inline void Mesh_Reducer::mark_vertex(int index) {
    topology_handler->vertex_flags[index] |= VERTEX_IS_MARKED;
}

inline void Mesh_Reducer::unmark_vertex(int index) {
    topology_handler->vertex_flags[index] &= ~VERTEX_IS_MARKED;
}


void Mesh_Reducer::collect_and_mark_vertex_star(int vertex_index, Auto_Array <int> *targets) {
    Reducer_Face_Membership *membership = &topology_handler->face_membership[vertex_index];

    int face_index;
    Array_Foreach(&membership->faces, face_index) {
        Reducer_Face *face = &topology_handler->faces[face_index];
        int within = get_index_within_face(mesh, face, vertex_index);
        assert(within != -1);

        int n1 = face->indices[(within + 1) % 3];
        int n2 = face->indices[(within + 2) % 3];

        n1 = mesh->canonical_vertex_map[n1];
        n2 = mesh->canonical_vertex_map[n2];

        if (!vertex_is_marked(n1)) {
            targets->add(n1);
            mark_vertex(n1);
        }


        if (!vertex_is_marked(n2)) {
            targets->add(n2);
            mark_vertex(n2);
        }
    } Endeach;
}


float Mesh_Reducer::find_hunt_radius_for_vertex(int vertex_index, Auto_Array <int> *targets) {
    float length_sum = 0;
    int length_count = 0;

    int other_index;
    Array_Foreach(targets, other_index) {
        length_count++;
        Vector3 delta1 = mesh->vertices[other_index] - mesh->vertices[vertex_index];
        length_sum += delta1.length();
    } Endeach;

    if (length_sum == 0) return 0; // XXX something nonzero?

    float length_average = length_sum / (float)length_count;

    return length_average;
}


void Mesh_Reducer::update_best_candidates(int vertex_index,
                                          float *best_error_result,
                                          int *best_v1_result) {
    Vector3 *pos = &mesh->vertices[vertex_index];

    mark_vertex(vertex_index);

    Auto_Array <int> potential_targets;
    collect_and_mark_vertex_star(vertex_index, &potential_targets);

    float hunt_radius = find_hunt_radius_for_vertex(vertex_index, &potential_targets);

    int i0, i1, j0, j1, k0, k1;
    i0 = proximity_grid->get_index_x(pos->x - hunt_radius);
    i1 = proximity_grid->get_index_x(pos->x + hunt_radius);
    j0 = proximity_grid->get_index_y(pos->y - hunt_radius);
    j1 = proximity_grid->get_index_y(pos->y + hunt_radius);
    k0 = proximity_grid->get_index_z(pos->z - hunt_radius);
    k1 = proximity_grid->get_index_z(pos->z + hunt_radius);

    int i, j, k;
    for (k = k0; k <= k1; k++) {
        for (j = j0; j <= j1; j++) {
            for (i = i0; i <= i1; i++) {
                int grid_index = proximity_grid->get_index(i, j, k);
                Lightweight_Proximity_Grid_Square *grid_square = &proximity_grid->grid_squares[grid_index];

                Auto_Array <Vector3 *> *vertices = &grid_square->vertices;

                int n;
                for (n = 0; n < vertices->live_items; n++) {
                    int other_index = get_vertex_index(mesh, vertices->data[n]);
                    if (!vertex_is_marked(other_index)) {
                        mark_vertex(other_index);
                        potential_targets.add(other_index);
                    }
                }
            }
        }
    }


    unmark_vertex(vertex_index);

    int other_index;
    Array_Foreach(&potential_targets, other_index) {
        unmark_vertex(other_index);
        update_best_candidates(vertex_index, other_index,
                               best_error_result, 
                               best_v1_result);
    } Endeach;

    if (*best_v1_result == -1) {
        int other_index;
        Array_Foreach(&potential_targets, other_index) {
            update_best_candidates(vertex_index, other_index,
                                   best_error_result, 
                                   best_v1_result);
        } Endeach;

    }

}

inline void Mesh_Reducer::update_single_vertex_without_queueing(int index,
                                                                float *error_result,
                                                                Reducer_Priority_Queue_Data *data_result) {
    data_result->vertex0 = index;
    data_result->vertex1 = -1;

    float best_error = FLT_MAX;
    update_best_candidates(index, &best_error, 
                           &data_result->vertex1);

    *error_result = best_error;
}

inline void Mesh_Reducer::queue_single_vertex(int index) {
    assert(mesh->canonical_vertex_map[index] == index);

    float best_error;
    Reducer_Priority_Queue_Data _data;
    update_single_vertex_without_queueing(index, &best_error, &_data);
    if ((_data.vertex1 != -1) && (best_error < FLT_MAX)) {
        Reducer_Priority_Queue_Data *data = new Reducer_Priority_Queue_Data;
        
        *data = _data;

        priority_queue->add(best_error, data);
    }
}


void Mesh_Reducer::init_priority_queue() {
    last_v0_examined = -1;
    last_v1_examined = -1;

    int i;
    for (i = 0; i < topology_handler->max_vertices; i++) {
        if (!(topology_handler->vertex_flags[i] & VERTEX_IS_LIVE)) continue;
        if (mesh->canonical_vertex_map[i] != i) continue;

        queue_single_vertex(i);
    }
}


int Mesh_Reducer::find_alias_to_map_to(int from_index, 
                                       int to_index) {
    int first_alias = to_index;
    int cursor = first_alias;

    int best_result = -1;
    float best_distance2 = FLT_MAX;

    float u0, v0;
    get_vertex_uv(mesh, from_index, &u0, &v0);
    
    // If we can find a vertex that represents uv coordinates within
    // the same material, map to that guy.
    while (1) {
        if (topology_handler->material_touched_by_vertex[from_index] ==
            topology_handler->material_touched_by_vertex[cursor]) {

            Vector3 tex0, tex1;
            get_vertex_uv(mesh, from_index, &tex0);
            get_vertex_uv(mesh, cursor, &tex1);

            float distance2 = (tex1 - tex0).length_squared();
            if (distance2 < best_distance2) {
                best_result = cursor;
                best_distance2 = distance2;
            }
        }

        // Go on to the next alias.

        cursor = topology_handler->vertex_coincidence_chain[cursor];
        if (cursor == first_alias) break;
    }

    if (best_result != -1) return best_result;

    // Give up, just give them an arbitrary one.  We could look into
    // creating a new vertex as a future extension, or moving this
    // one to the new position but... well... I just don't know.

    return to_index;
}


void Mesh_Reducer::perform_one_reduction() {
    int kill_v0 = -1;
    int kill_v1 = -1;

    float priority;
    Reducer_Priority_Queue_Data *data;
    while (1) {

        bool success = false;
        data = (Reducer_Priority_Queue_Data *)priority_queue->remove_head(&priority, &success);

        assert(success);  // We must have one entry in queue per vertex
                          // on a face...

        int v0 = data->vertex0;
        int v1 = data->vertex1;

        if ((topology_handler->vertex_flags[v0] & VERTEX_IS_LIVE) && 
            (topology_handler->vertex_flags[v1] & VERTEX_IS_LIVE)) {
            // This is still potentially a valid collapse (both vertices still
            // exist) but we don't know whether other stuff has been collapsed
            // into V0, invalidating the priority computation that caused us
            // to just pull this guy off.  So.  We have this policy where we
            // always re-evaluate guys and throw them back on the queue.  If
            // we get the same guy twice in a row, we really want to process
            // him now, so away he goes.  Otherwise, he can wait.

            // Re-evaluate this guy and throw him back into the pond.

            float new_priority;
            update_single_vertex_without_queueing(v0, &new_priority, data);
            if (data->vertex1 == -1) {
                delete data;
                continue;
            }

            if (new_priority <= priority) { 
                kill_v0 = data->vertex0;
                kill_v1 = data->vertex1;

                delete data;
                break;
            } else {
                priority_queue->add(new_priority, data);
            }

            continue;
        }

        delete data;

        // Hmm, we nuked either the source or destination vertex since
        // that was completed...

        // If it was the source vertex, just continue.

        if (!(topology_handler->vertex_flags[v0] & VERTEX_IS_LIVE)) continue;

        // If on the other hand that guy is still alive, we need to
        // find a new place to tell him to go.  So find one, and
        // put it in the queue, and then keep on keepin' on.

        queue_single_vertex(v0);
    }

    perform_one_reduction(kill_v0, kill_v1);
}

void Mesh_Reducer::perform_one_reduction(int kill_v0, int kill_v1) {
    assert(kill_v1 >= 0);
    int first_alias = kill_v0;


    topology_handler->clear_faces_to_check();

    while (1) {
        int next = go_to_next_coincident_vertex(topology_handler, first_alias);
        assert(next != kill_v1);
        if (next == first_alias) break;

        int map_to = find_alias_to_map_to(next, kill_v1);

        // Accumulate quadrics.
        if (error_quadrics) {
            Error_Quadric *quadric0 = get_quadric(next);
            Error_Quadric *quadric1 = get_quadric(map_to);
            quadric1->accumulate_quadric(quadric0);
        }

        // Switch over the indices on all the faces.
        topology_handler->remap_vertex_face_indices(next, map_to);

        int next_next = go_to_next_coincident_vertex(topology_handler, next);
        topology_handler->vertex_coincidence_chain[first_alias] = next_next;
        topology_handler->vertex_coincidence_chain[next] = -1;

        topology_handler->eliminate_vertex_from_mesh(next);
    }

    topology_handler->vertex_coincidence_chain[first_alias] = -1;
    remove_vertex_from_grid(first_alias);
    int map_to = find_alias_to_map_to(first_alias, kill_v1);

    if (error_quadrics) { // If we are actually doing detail reduction...
        Error_Quadric *quadric0 = get_quadric(first_alias);
        Error_Quadric *quadric1 = get_quadric(map_to);

        quadric1->accumulate_quadric(quadric0);
    }

    topology_handler->remap_vertex_face_indices(first_alias, map_to);
    topology_handler->eliminate_vertex_from_mesh(first_alias);
    topology_handler->check_degenerate_faces();
}

void Mesh_Reducer::compensate_for_lonely_edge(Reducer_Face *face,
                                              int vertex_index0,
                                              int vertex_index1) {
    // Make some fake vertices that stick out from this face,
    // so's we like can pass them to the quadric thingy to
    // impose the constraint.

    // We start by getting the XYZ normal of the face.
    Vector3 position0;
    Vector3 position1;
    Vector3 position2;
    get_vertex_position(mesh, face->indices[0], &position0);
    get_vertex_position(mesh, face->indices[1], &position1);
    get_vertex_position(mesh, face->indices[2], &position2);

    Vector3 side1 = position1 - position0;
    Vector3 side2 = position2 - position0;
    Vector3 normal = cross_product(side1, side2);

    int can0 = mesh->canonical_vertex_map[vertex_index0];
    int can1 = mesh->canonical_vertex_map[vertex_index1];
    assert(topology_handler->vertex_flags[can0] & VERTEX_IS_ON_LONELY_EDGE);
    assert(topology_handler->vertex_flags[can1] & VERTEX_IS_ON_LONELY_EDGE);

    // So that we don't do anything numerically bizarre, we will scale
    // this normal vector so that it's the same length as the distance
    // between vertex_index0 and vertex_index1.

    Vector3 vertex_position_0, vertex_position_1;
    get_vertex_position(mesh, vertex_index0, &vertex_position_0);
    get_vertex_position(mesh, vertex_index1, &vertex_position_1);
    float length = distance(vertex_position_0, vertex_position_1);

    normal.normalize();
    normal.scale(length);


    // Now let's gather the 2 points representing the edge.  Then
    // we'll manufacture a 3rd point, then tell the quadric guys
    // to go deal.  Right now we don't do anything with UV (unclear
    // whether we should).
    float point0[ERROR_QUADRIC_DIMENSIONS];
    float point1[ERROR_QUADRIC_DIMENSIONS];

    fill_point(point0, vertex_index0, face->material);
    fill_point(point1, vertex_index1, face->material);

    float point2[ERROR_QUADRIC_DIMENSIONS];

    assert(ERROR_QUADRIC_DIMENSIONS == 5);
    point2[0] = point1[0] + normal.x;
    point2[1] = point1[1] + normal.y;
    point2[2] = point1[2] + normal.z;
    point2[3] = point1[3];
    point2[4] = point1[4];

    Error_Quadric *quadric0 = get_quadric(vertex_index0);
    Error_Quadric *quadric1 = get_quadric(vertex_index1);
    
    quadric0->accumulate_plane(point0, point1, point2, tuning.lonely_edge_constraint_factor);
    quadric1->accumulate_plane(point0, point1, point2, tuning.lonely_edge_constraint_factor); 

    num_lonely_edges_detected++;
}

void Mesh_Reducer::handle_lonely_edges() {
    // Look for all lonely edges on this mesh.  For any that
    // we find, add a constraint to keep that edge from moving
    // much.

    int i;
    for (i = 0; i < mesh->num_vertices; i++) {
        if (!(topology_handler->vertex_flags[i] & VERTEX_IS_LIVE)) continue;

        int vertex_0_index = i;
        int canonical0 = mesh->canonical_vertex_map[vertex_0_index];

        Reducer_Face_Membership *membership = &topology_handler->face_membership[i];

        // For each face this vertex is on, take this vertex
        // and the 'forward' vertex along the face's winding
        // order to constitute the edge we are looking at now.
        // (We don't have to consider the guy behind us in the
        // winding order, because we will process that edge when
        // we are looking at that vertex, and he sees us in front
        // of him.)

        int j;
        for (j = 0; j < membership->faces.live_items; j++) {
            int face_index = membership->faces.data[j];
            Reducer_Face *face = &topology_handler->faces[face_index];

            int where_within_face = get_index_within_face(mesh, face, canonical0);
            assert(where_within_face != -1);


            int vertex_1_index = face->indices[(where_within_face + 1) % 3];
            int canonical1 = mesh->canonical_vertex_map[vertex_1_index];
            
            // Now... we search again through this vertex's membership
            // list, to find ANOTHER face containing this vertex,
            // which contains the edge appearing in the opposite winding
            // order (canonical1 followed by canonical0).  If we can find
            // no such face, then this edge is lonely.
            
            bool edge_is_lonely = true;

            Reducer_Face_Membership *mem1 = &topology_handler->face_membership[canonical1];

            int k;
            for (k = 0; k < mem1->faces.live_items; k++) {
                int face_index1 = mem1->faces.data[k];
                if (face_index1 == face_index) continue;

                Reducer_Face *face1 = &topology_handler->faces[face_index1];
                int where_within_face = get_index_within_face(mesh, face1, canonical0);
                if (where_within_face == -1) continue;

                int vertex_2_index = face1->indices[(where_within_face + 2) % 3];
                int canonical2 = mesh->canonical_vertex_map[vertex_2_index];
                if (canonical2 == canonical1) {
                    edge_is_lonely = false;
                    break;
                }
            }

            if (edge_is_lonely) {
                topology_handler->vertex_flags[canonical0] |= VERTEX_IS_ON_LONELY_EDGE;
                topology_handler->vertex_flags[canonical1] |= VERTEX_IS_ON_LONELY_EDGE;
                compensate_for_lonely_edge(face, canonical0, canonical1);
            }
        }
    }
}

void Mesh_Reducer::mark_face_as_seam(int index) {
    assert(index >= 0);
    assert(index < topology_handler->num_faces_remaining);
    topology_handler->faces[index].flags |= FACE_IS_A_SEAM_FILL;
}

void Mesh_Reducer::collapse_similar_vertices(float threshold) {
    double t2 = threshold * threshold;
    float hunt_radius = threshold;

    Auto_Array <int> to_collapse;
    
    int n;
    for (n = 0; n < mesh->num_vertices; n++) {
        if (n != mesh->canonical_vertex_map[n]) continue;
        if (!(topology_handler->vertex_flags[n] & VERTEX_IS_LIVE)) continue;

        Vector3 *pos = &mesh->vertices[n];

        int i0, i1, j0, j1, k0, k1;
        i0 = proximity_grid->get_index_x(pos->x - hunt_radius);
        i1 = proximity_grid->get_index_x(pos->x + hunt_radius);
        j0 = proximity_grid->get_index_y(pos->y - hunt_radius);
        j1 = proximity_grid->get_index_y(pos->y + hunt_radius);
        k0 = proximity_grid->get_index_z(pos->z - hunt_radius);
        k1 = proximity_grid->get_index_z(pos->z + hunt_radius);

        to_collapse.reset();

        int i, j, k;
        for (k = k0; k <= k1; k++) {
            for (j = j0; j <= j1; j++) {
                for (i = i0; i <= i1; i++) {
                    int grid_index = proximity_grid->get_index(i, j, k);
                    Lightweight_Proximity_Grid_Square *grid_square = &proximity_grid->grid_squares[grid_index];

                    Auto_Array <Vector3 *> *vertices = &grid_square->vertices;

                    int k;
                    for (k = 0; k < vertices->live_items; k++) {
                        int other_index = get_vertex_index(mesh, vertices->data[k]);
                        if (other_index == n) continue;
                        assert(topology_handler->vertex_flags[other_index] & VERTEX_IS_LIVE);

                        double d2 = distance_squared(mesh->vertices[other_index], *pos);
                        if (d2 <= t2) to_collapse.add(other_index);
                    }
                }
            }
        }

        int target;
        Array_Foreach(&to_collapse, target) {
            perform_one_reduction(target, n);
            topology_handler->check_degenerate_faces();
        } Endeach;
    }

}

void Mesh_Reducer::reduce(int _num_target_faces) {
    // Do setup regarding quadrics, etc.

    init_quadrics();
    handle_lonely_edges();

    priority_queue = new Priority_Queue(mesh->num_vertices * 10);
    init_priority_queue();


    // Actually do the reduction part now.


    //    AssertFacesAreSane(mesh);
    initial_num_vertices = mesh->num_vertices;
    num_target_faces = _num_target_faces;

    while (topology_handler->num_faces_remaining > num_target_faces) {
        perform_one_reduction();
    }
}

Mesh_Builder *Mesh_Reducer::prepare_result(int **remap) {
    assert(mesh);
    assert(topology_handler);

    topology_handler->compact_vertices();
    int num_vertices = topology_handler->num_vertices_remaining;
    int num_faces = topology_handler->num_faces_remaining;

    Mesh_Builder *result = new Mesh_Builder(num_vertices, num_faces);

    Vector2 *output_uvs = (Vector2 *)topology_handler->output_uvs;
    Quaternion *output_tangent_frames = topology_handler->output_tangent_frames;
    int *xref = new int[num_vertices];
    int i;
    for (i = 0; i < num_vertices; i++) {
        int index = result->add_vertex(topology_handler->output_vertices[i],
                                       output_uvs[i], 
                                       output_tangent_frames[i]);
        assert(index == i);
        xref[i] = index;
    }


    for (i = 0; i < num_faces; i++) {
        Reducer_Face *face = &topology_handler->faces[i];
        result->add_triangle(xref[face->indices[0]], xref[face->indices[1]],
                             xref[face->indices[2]], face->material);
    }

    if (remap) {
        *remap = xref;
    } else {
        delete [] xref;
    }
        
    for (i = 0; i < mesh->num_materials; i++) {
        // @Feature We don't check for no-longer-used materials and compact them.
        // This probably doesn't happen very often anyway, BUT we probably
        // we probably want to intentionally compact things into lower-res
        // textures at some point, for which we allocate one material for
        // the whole dude maybe?  Oh that would change the material shaders
        // and cause popping so it might actually suck.
        // Hmm, who the hell knows.
        result->add_material(&mesh->material_info[i]);
    }

    return result;
}

void Mesh_Reducer::get_result(Triangle_List_Mesh **result_return) {
    int *remap;
    Mesh_Builder *builder = prepare_result(&remap);

    // @Robustness: 0.01f threshold, or adjustable distance
    // @Robustness: integer remapper
    Triangle_List_Mesh *result = builder->build_mesh();

    int i;
    for (i = 0; i < topology_handler->num_vertices_originally; i++) {
        int xref = topology_handler->old_index_to_new_index[i];
        topology_handler->old_index_to_new_index[i] = remap[xref];
    }

    delete [] remap;

    *result_return = result;
}

// Mesh_Reducer looks at the dimensions of the model, then cooks up a
// roughly uniform 3D grid.  Allocates that grid, iterates over
// the model and plunks every vertex into the grid square where
// it goes.  At runtime we do a hunt_radius search through this grid,
// find everyone within a certain distance, and check them to see
// if they are a good collapse candidate.  (This way we are
// checking non-edges as vertex collapse candidates, which seems
// like a really good idea given the input data I've seen.)

void Mesh_Reducer::init(Triangle_List_Mesh *_mesh) {
    assert(mesh == NULL);
    mesh = _mesh;

    // Collect the dimensions of the mesh.

    Vector3 v0, v1;
    v0.x = v0.y = v0.z = FLT_MAX;
    v1.x = v1.y = v1.z = -FLT_MAX;

    int i;
    for (i = 0; i < mesh->num_vertices; i++) {
        Vector3 *v = &mesh->vertices[i];
        if (v->x < v0.x) v0.x = v->x;
        if (v->y < v0.y) v0.y = v->y;
        if (v->z < v0.z) v0.z = v->z;
        if (v->x > v1.x) v1.x = v->x;
        if (v->y > v1.y) v1.y = v->y;
        if (v->z > v1.z) v1.z = v->z;
    }

    Vector3 dv = v1 - v0;
    float widest = Max(dv.x, Max(dv.y, dv.z));
    const int RESOLUTION = 12;
    float ds = widest / (float)RESOLUTION;
    assert(ds != 0);

    proximity_grid = new Lightweight_Proximity_Grid;
    proximity_grid->init(v0, dv, ds);

    for (i = 0; i < mesh->num_vertices; i++) {
        if (mesh->canonical_vertex_map[i] == i) {
            Vector3 *pos = &mesh->vertices[i];
            proximity_grid->add(pos);
        }
    }

    topology_handler = new Mesh_Topology_Handler();
    topology_handler->init(mesh);

}

void Mesh_Reducer::remove_vertex_from_grid(int vertex_index) {
    if (vertex_index != mesh->canonical_vertex_map[vertex_index]) return;
    Vector3 *pos = &mesh->vertices[vertex_index];
    bool success = proximity_grid->remove(pos);
    assert(success);
}



