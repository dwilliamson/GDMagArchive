#include "../framework.h"
#include "../mesh.h"
#include "mesh_reducer.h"
#include "mesh_topology_handler.h"

#include <float.h>
#include <math.h>

void get_vertex_uv(Triangle_List_Mesh *mesh, int index, Vector3 *result);
extern float GetArea(Vector3 v0, Vector3 v1, Vector3 v2);

Mesh_Topology_Handler::~Mesh_Topology_Handler() {
    delete [] faces;
    delete faces_to_check;
    delete [] material_touched_by_vertex;
    delete [] face_membership;
    delete [] vertex_flags;
    delete [] vertex_coincidence_chain;

    delete [] new_index_to_old_index;
    delete [] old_index_to_new_index;
    delete [] vertex_collapse_destination;
    delete [] output_vertices;
    delete [] output_uvs;
}

float GetArea(Vector3 v0, Vector3 v1, Vector3 v2) {
    Vector3 w1 = v1 - v0;
    Vector3 w2 = v2 - v0;
    Vector3 cross = cross_product(w1, w2);
    return cross.length();
}

void Mesh_Topology_Handler::get_listed_faces(Triangle_List_Mesh *lmesh) {
    int i;
    // Count up all the faces.
    for (i = 0; i < lmesh->num_triangle_lists; i++) {
        Triangle_List_Info *info = &lmesh->triangle_list_info[i];

        assert(info->num_vertices > 2);
        assert(info->num_vertices % 3 == 0);

        num_faces_remaining += (info->num_vertices / 3);
    }

    // Allocate face data; walk over all the strips again, and fill out
    // the face data.

    faces = new Reducer_Face[num_faces_remaining];
    int face_cursor = 0;
    for (i = 0; i < lmesh->num_triangle_lists; i++) {
        Triangle_List_Info *info = &lmesh->triangle_list_info[i];

        int k;
        for (k = 0; k < info->num_vertices; k += 3) {
            assert(face_cursor < num_faces_remaining);
            Reducer_Face *face = &faces[face_cursor];
            face->material = info->material_index;
            face->flags = 0;

            face->indices[0] = lmesh->indices[info->start_of_list + k + 0];
            face->indices[1] = lmesh->indices[info->start_of_list + k + 1];
            face->indices[2] = lmesh->indices[info->start_of_list + k + 2];

            material_touched_by_vertex[face->indices[0]] = face->material;
            material_touched_by_vertex[face->indices[1]] = face->material;
            material_touched_by_vertex[face->indices[2]] = face->material;

            face_cursor++;
        }
    }

    assert(face_cursor == num_faces_remaining);
}

void Mesh_Topology_Handler::init(Triangle_List_Mesh *_mesh) {
    mesh = _mesh;
    faces_to_check = new Auto_Array <int>;
    faces = NULL;
    face_membership = NULL;
    vertex_coincidence_chain = NULL;


    num_vertices_remaining = mesh->num_vertices;
    num_vertices_originally = mesh->num_vertices;
    max_vertices = num_vertices_remaining;

    output_vertices = NULL;
    output_uvs = NULL;
    new_index_to_old_index = NULL;
    vertex_flags = NULL;


    material_touched_by_vertex = new int[num_vertices_remaining];
    old_index_to_new_index = new int[num_vertices_remaining];
    vertex_collapse_destination = new int[num_vertices_remaining];

    int i;
    for (i = 0; i < max_vertices; i++) {
        material_touched_by_vertex[i] = -1;
        old_index_to_new_index[i] = -1;
        vertex_collapse_destination[i] = -1;
    }

    build_vertex_coincidence_chain();

    num_faces_remaining = 0;

    get_listed_faces((Triangle_List_Mesh *)mesh);

    face_membership = new Reducer_Face_Membership[num_vertices_remaining];
    vertex_flags = new unsigned char[num_vertices_remaining];
    for (i = 0; i < num_vertices_remaining; i++) vertex_flags[i] = VERTEX_IS_LIVE;

    init_face_membership_info();
}

void Mesh_Topology_Handler::build_vertex_coincidence_chain() {
    vertex_coincidence_chain = new int[num_vertices_remaining];

    int *last_vertex_in_chain_from_canonical = new int[num_vertices_remaining];

    int i;
    for (i = 0; i < num_vertices_remaining; i++) {
        last_vertex_in_chain_from_canonical[i] = i;
    }

    for (i = 0; i < num_vertices_remaining; i++) {
        int canonical = mesh->canonical_vertex_map[i];
        assert(canonical == mesh->canonical_vertex_map[canonical]);

        if (canonical != i) {
            assert(canonical < i);
            assert(canonical >= 0);
            int last = last_vertex_in_chain_from_canonical[canonical];
            vertex_coincidence_chain[last] = i;

            last_vertex_in_chain_from_canonical[canonical] = i;
        }

        vertex_coincidence_chain[i] = canonical;
    }

    // Done; clean up.
    delete [] last_vertex_in_chain_from_canonical;
}

void Mesh_Topology_Handler::assert_face_in_membership(Reducer_Face_Membership *membership, Reducer_Face *face) {
    int i;
    for (i = 0; i < membership->faces.live_items; i++) {
        if (&faces[membership->faces.data[i]] == face) return;
    }

    assert(0);
}

void Mesh_Topology_Handler::sanity_membership() {
    int i;
    for (i = 0; i < mesh->num_vertices; i++) {
        if (vertex_flags[i] & VERTEX_IS_LIVE) sanity_membership(i);
    }
}

void Mesh_Topology_Handler::sanity_membership(int orig_vertex_index) {
    int vertex_index = mesh->canonical_vertex_map[orig_vertex_index];
    Reducer_Face_Membership *membership = &face_membership[vertex_index];

    int i;
    for (i = 0; i < membership->faces.live_items; i++) {
        Reducer_Face *face;
        face = &faces[membership->faces.data[i]];
        if (mesh->canonical_vertex_map[face->indices[0]] == vertex_index) continue;
        if (mesh->canonical_vertex_map[face->indices[1]] == vertex_index) continue;
        if (mesh->canonical_vertex_map[face->indices[2]] == vertex_index) continue;

        assert(0);
    }

    for (i = 0; i < num_faces_remaining; i++) {
        Reducer_Face *face = &faces[i];
        if (face->indices[0] == orig_vertex_index) assert_face_in_membership(membership, face);
        if (face->indices[1] == orig_vertex_index) assert_face_in_membership(membership, face);
        if (face->indices[2] == orig_vertex_index) assert_face_in_membership(membership, face);
    }
}

void Mesh_Topology_Handler::remap_face_membership_face_indices_helper(int orig_vertex_index,
                                                                      int old_face_index,
                                                                      int new_face_index) {
    int vertex_index = mesh->canonical_vertex_map[orig_vertex_index];
    Reducer_Face_Membership *membership = &face_membership[vertex_index];

    int num_found = 0;
    int i;
    for (i = 0; i < membership->faces.live_items; i++) {
        if (membership->faces[i] == old_face_index) {
            membership->faces[i] = new_face_index;
            num_found++;
        }
    }

    // We would like to assert here, but if it is the case that two vertices
    // in the face aliased to the same thing, we will have already removed
    // the face from this membership list, so we won't find it.
    //    assert(num_found);
}

void Mesh_Topology_Handler::remap_face_membership_face_indices(int old_index,
                                                               int new_index) {
    Reducer_Face *face = &faces[new_index];
    remap_face_membership_face_indices_helper(face->indices[0], old_index, new_index);
    remap_face_membership_face_indices_helper(face->indices[1], old_index, new_index);
    remap_face_membership_face_indices_helper(face->indices[2], old_index, new_index);
}

void Mesh_Topology_Handler::remap_vertex_face_indices(int old_index, int new_index) {
    vertex_collapse_destination[old_index] = new_index;

    int i;
    for (i = 0; i < num_faces_remaining; i++) {
        Reducer_Face *face = &faces[i];

        int j;
        for (j = 0; j < 3; j++) {
            if (face->indices[j] == old_index) {
                face->indices[j] = new_index;
                add_face_membership_to_vertex(i, new_index);
                add_face_to_check(i);
            }
        }
    }
}

void Mesh_Topology_Handler::remove_face_membership_from_vertex(int orig_vertex_index, int face_index) {
    int vertex_index = mesh->canonical_vertex_map[orig_vertex_index];
    Reducer_Face_Membership *membership = &face_membership[vertex_index];
    membership->faces.remove(face_index);
}

void Mesh_Topology_Handler::eliminate_vertex_from_mesh(int dead_vertex) {
    num_vertices_remaining--;
    vertex_flags[dead_vertex] &= ~VERTEX_IS_LIVE;
}



bool is_alias_of(Mesh_Topology_Handler *handler, int target, int other) {
    Triangle_List_Mesh *mesh = handler->mesh;
    if (mesh->canonical_vertex_map[target] == mesh->canonical_vertex_map[other]) return true;
    return false;
}


bool face_is_degenerate2(Mesh_Topology_Handler *handler, Reducer_Face *face) {
    if (is_alias_of(handler, face->indices[0], face->indices[1])) return true;
    if (is_alias_of(handler, face->indices[1], face->indices[2])) return true;
    if (is_alias_of(handler, face->indices[2], face->indices[0])) return true;

    return false;
}

void Mesh_Topology_Handler::clear_faces_to_check() {
    faces_to_check->reset();
}

void Mesh_Topology_Handler::add_face_to_check(int index) {
    faces_to_check->add(index);
}

/*
void assert_faces_are_clean(Mesh_Topology_Handler *handler) {
    int i;
    for (i = 0; i < handler->num_faces_remaining; i++) {
        Reducer_Face *face = &handler->faces[i];
        assert(face->indices[0] != face->indices[1]);
        assert(face->indices[0] != face->indices[2]);
        assert(face->indices[1] != face->indices[2]);
    }
}
*/

void Mesh_Topology_Handler::check_degenerate_faces() {
    int num_changes = 0;

    int i;
    for (i = 0; i < faces_to_check->live_items; i++) {
        int index = faces_to_check->data[i];

        Reducer_Face *face = &faces[index];

        if (face_is_degenerate2(this, face)) {
            // Take this guy out of our own local array...
            num_changes++;
            faces_to_check->data[i] = faces_to_check->data[faces_to_check->live_items - 1];
            faces_to_check->live_items--;
            i--;

            // Now do the actual global manipulations...

            remove_face_membership_from_vertex(face->indices[0], index);
            remove_face_membership_from_vertex(face->indices[1], index);
            remove_face_membership_from_vertex(face->indices[2], index);

            int old_index = num_faces_remaining - 1;
            *face = faces[old_index];
            num_faces_remaining--;

            // This Remap must happen after we copy the face.
            if (old_index != index) {
                remap_face_membership_face_indices(old_index, index);

                int j;
                for (j = 0; j < faces_to_check->live_items; j++) {
                    if (faces_to_check->data[j] == old_index) faces_to_check->data[j] = index;
                }
            }
        }
    }

    if (num_changes) {
        // This collapse may have made more faces degenerate, so we need to recurse!
        // Though I am actually not sure if this is totally necessary;
        // @Simplify: investigate this question sometime?
        check_degenerate_faces();
    }

    faces_to_check->reset();
}

Vector3 get_signed_area(int indices[3], Triangle_List_Mesh *mesh) {
    Vector3 *v0, *v1, *v2;
    v0 = &mesh->vertices[indices[0]];
    v1 = &mesh->vertices[indices[1]];
    v2 = &mesh->vertices[indices[2]];

    Vector3 w1 = *v1 - *v0;
    Vector3 w2 = *v2 - *v0;

    return cross_product(w1, w2);
}


bool Mesh_Topology_Handler::face_would_be_icky(int face_index,
                                               int orig_old_vertex_index,
                                               int orig_new_vertex_index) {

    int *canonical = mesh->canonical_vertex_map;
    int old_vertex_index = canonical[orig_old_vertex_index];
    int new_vertex_index = canonical[orig_new_vertex_index];

    int num_changed = 0;

    Reducer_Face *face = &faces[face_index];

    int indices[3];
    int i;
    for (i = 0; i < 3; i++) {
        indices[i] = mesh->canonical_vertex_map[face->indices[i]];
        if (indices[i] == old_vertex_index) {
            indices[i] = new_vertex_index;
            num_changed++;
        }
    }

    if (num_changed == 0) return false;

    // If the face we are looking at is "cleanly" degenerate, 
    // it is perfectly fine, thanks.  It is not icky.

    if ((indices[0] == indices[1]) || (indices[0] == indices[2])
        || (indices[1] == indices[2])) return false;


    // Check the characteristics of the face we are creating,
    // compared to the old one.  If it becomes degenerate, or
    // flips over, that is bad!

    Vector3 W0 = get_signed_area(face->indices, mesh);
    Vector3 W1 = get_signed_area(indices, mesh);

    // If the areas are of opposite signs, that is icky!

    if (dot_product(W0, W1) <= 0) return true;

    // If the area becomes very very small, that is also icky!

    double Area0 = W0.length();
    double Area1 = W1.length();

    const double ICKINESS_EPSILON = 0.00001;
    if (fabs(Area1) <= ICKINESS_EPSILON * fabs(Area0)) return true;

    // XXXXXX debugging
//    if (W1.z < 0) return true;

    // I guess it's not icky!
    return false;
}


bool Mesh_Topology_Handler::collapse_creates_icky_face(int from_index,
                                              int to_index) {
    int index0 = mesh->canonical_vertex_map[from_index];
    int index1 = mesh->canonical_vertex_map[to_index];

    Reducer_Face_Membership *ship = &face_membership[index0];

    int i;
    for (i = 0; i < ship->faces.live_items; i++) {
        int face_index = ship->faces.data[i];
        if (face_would_be_icky(face_index, from_index, to_index)) return true;
    }

    return false;
}

bool Mesh_Topology_Handler::collapse_moves_material_boundary(int from_index,
                                                             int to_index) {
    // If every coincident vertex in the From loop has a material to
    // map to in the To loop, then the collapse is happening parallel
    // to a material boundary (or fully within one material), so it's
    // okay.  Otherwise the material boundary will move.

    int first_alias = from_index;
    int cursor = first_alias;
    while (1) {
        int Dest = simple_find_alias_to_map_to(cursor, to_index);
        if (Dest == -1) return true;  // Material boundary detected.

        cursor = vertex_coincidence_chain[cursor];
        if (cursor == first_alias) break;
    }

    return false;
}

bool Mesh_Topology_Handler::collapse_changes_topology(int from_index, int to_index) {
    int index0 = mesh->canonical_vertex_map[from_index];
    int index1 = mesh->canonical_vertex_map[to_index];

    Reducer_Face_Membership *Mem0 = &face_membership[index0];
    int i;
    for (i = 0; i < Mem0->faces.live_items; i++) {
        int face_index0 = Mem0->faces.data[i];

        // Look for a matching face in Mem1.  This is still a little
        // n-squared, but not as bad as before.  We can turn it into
        // O(n) by using a buffer of markers, if this is slow.

        Reducer_Face_Membership *Mem1 = &face_membership[index1];
        int j;
        for (j = 0; j < Mem1->faces.live_items; j++) {
            if (face_index0 == Mem1->faces.data[j]) return false;
        }
    }

    return true;
}

#ifdef OLD_AND_SLOW
bool Mesh_Topology_Handler::collapse_changes_topology(int from_index,
                                          int to_index) {
    int index0 = mesh->vertices[from_index].LowestCoincidentVertex;
    int index1 = mesh->vertices[to_index].LowestCoincidentVertex;

    int i;
    for (i = 0; i < mesh->num_faces; i++) {
        Reducer_Face *face = &faces[i];
        if (!VertexOnFace(mesh, index0, Face)) continue;
        if (!VertexOnFace(mesh, index1, Face)) continue;

        return false;
    }

    return true;
}
#endif // OLD_AND_SLOW


void Mesh_Topology_Handler::add_face_membership_to_vertex(int face_index, int _vertex_index) {
    int vertex_index = mesh->canonical_vertex_map[_vertex_index];
    Vector3 *pos = &mesh->vertices[vertex_index];

    Reducer_Face_Membership *ship = &face_membership[vertex_index];
    ship->faces.add(face_index);
}

void Mesh_Topology_Handler::init_face_membership_info() {
    int i;
    for (i = 0; i < mesh->num_faces; i++) {
        Reducer_Face *face = &faces[i];
        add_face_membership_to_vertex(i, face->indices[0]);
        add_face_membership_to_vertex(i, face->indices[1]);
        add_face_membership_to_vertex(i, face->indices[2]);
    }
}

void Mesh_Topology_Handler::compact_vertices() {
    int i;

    // Eliminate any faces with area that is way too small.
    // This is because the LOD process can generate faces
    // where all 3 vertices are colinear (it happens pretty
    // easily, actually).

    const double AREA_FRACTION = 0.0003;
    double dx = 2.0f; // XXX
    double AREA_EPSILON = (dx * AREA_FRACTION) * (dx * AREA_FRACTION);

    Vector3 *v0, *v1, *v2;

    for (i = 0; i < num_faces_remaining; i++) {
        Reducer_Face *face = &faces[i];

        int n0 = face->indices[0];
        int n1 = face->indices[1];
        int n2 = face->indices[2];

        v0 = &mesh->vertices[n0];
        v1 = &mesh->vertices[n1];
        v2 = &mesh->vertices[n2];

        double area = GetArea(*v0, *v1, *v2);
        if (area < AREA_EPSILON) {
            *face = faces[num_faces_remaining - 1];
            num_faces_remaining--;
            i--;
        }
    }

    // Kill any vertices that don't belong to a face.

    int *vertex_ref_count = new int[max_vertices];

    for (i = 0; i < max_vertices; i++) vertex_ref_count[i] = 0;

    for (i = 0; i < num_faces_remaining; i++) {
        Reducer_Face *face = &faces[i];
        vertex_ref_count[face->indices[0]]++;
        vertex_ref_count[face->indices[1]]++;
        vertex_ref_count[face->indices[2]]++;
    }

    for (i = 0; i < max_vertices; i++) {
        if (!(vertex_flags[i] & VERTEX_IS_LIVE)) {
            assert(vertex_ref_count[i] == 0);
            continue;
        }

        if (vertex_ref_count[i]) continue;

        vertex_flags[i] &= ~VERTEX_IS_LIVE;
        num_vertices_remaining--;
    }

    delete [] vertex_ref_count;


    // Okay, let's do the actual compaction now.

    int num_vertices_compacted = 0;

    new_index_to_old_index = new int[max_vertices];

    for (i = 0; i < max_vertices; i++) {
        new_index_to_old_index[i] = -1;
    }

    output_vertices = new Vector3[num_vertices_remaining];
    output_uvs = new Vector2[num_vertices_remaining];
    output_tangent_frames = new Quaternion[num_vertices_remaining];

    for (i = 0; i < max_vertices; i++) {
        if (vertex_flags[i] & VERTEX_IS_LIVE) {
            output_vertices[num_vertices_compacted] = mesh->vertices[i];
            output_uvs[num_vertices_compacted] = mesh->uvs[i];
            if (mesh->tangent_frames) output_tangent_frames[num_vertices_compacted] = mesh->tangent_frames[i];
            new_index_to_old_index[num_vertices_compacted] = i;
            old_index_to_new_index[i] = num_vertices_compacted;

            num_vertices_compacted++;
        }
    }

    for (i = 0; i < max_vertices; i++) {
        int xref = i;
        while (!(vertex_flags[xref] & VERTEX_IS_LIVE)) {
            xref = vertex_collapse_destination[xref];
            assert(xref != -1);
        }

        int remap = old_index_to_new_index[xref];
        assert(remap != -1);
        old_index_to_new_index[i] = remap;
    }


    for (i = 0; i < num_faces_remaining; i++) {
        Reducer_Face *face = &faces[i];
        int j;
        for (j = 0; j < 3; j++) {
            int xref = old_index_to_new_index[face->indices[j]];
            assert(xref >= 0);
            assert(xref < num_vertices_compacted);

            face->indices[j] = xref;
        }
    }

    assert(num_vertices_compacted == num_vertices_remaining);
}
// XXX num_vertices_remaining is the number of CANONICAL guys, not total


int Mesh_Topology_Handler::simple_find_alias_to_map_to(int from_index, int to_index) {
    int first_alias = to_index;
    int cursor = first_alias;

    int best_result = -1;
    float best_distance2 = FLT_MAX;

    // If we can find a vertex that represents uv coordinates within
    // the same material, map to that guy.  There could be multiple
    // guys with different uv coordinates; if there are, find the
    // one closest in uv-space.  (XXX Not necessarily right because
    // uv space can wrap!  Maybe we compensate for the wrapping here.
    // Whatever.)

    while (1) {
        if (material_touched_by_vertex[from_index] ==
            material_touched_by_vertex[cursor]) {

            Vector3 Tex0, Tex1;
            get_vertex_uv(mesh, from_index, &Tex0);
            get_vertex_uv(mesh, cursor, &Tex1);

            float distance2 = (Tex1 - Tex0).length_squared();
            if (distance2 < best_distance2) {
                best_result = cursor;
                best_distance2 = distance2;
            }
        }

        // Go on to the next alias.

        cursor = vertex_coincidence_chain[cursor];
        if (cursor == first_alias) break;
    }

    return best_result;
}
