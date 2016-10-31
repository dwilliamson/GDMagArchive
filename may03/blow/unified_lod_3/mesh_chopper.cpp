#include "framework.h"
#include "mesh_chopper.h"

#include "float.h"
#include "mesh.h"
#include "mesh_seam.h"
#include "mesh_builder.h"

#include <math.h>


inline Vertex_Specifier::Vertex_Specifier(int _input_n0, int _input_n1, 
                                          int _plane_index) {
    input_n0 = _input_n0;
    input_n1 = _input_n1;
    plane_index = _plane_index;

    output_index = -1;
}

inline Vertex_Specifier::Vertex_Specifier(Vertex_Specifier *other) {
    input_n0 = other->input_n0;
    input_n1 = other->input_n1;
    plane_index = other->plane_index;

    output_index = other->output_index;
}

int Vertex_Specifier::compare(Hashable *_other) {
    Vertex_Specifier *other = (Vertex_Specifier *)_other;

    if (other->input_n0 != input_n0) return -1;
    if (other->input_n1 != input_n1) return -1;
    if (other->plane_index != plane_index) return -1;

    // We intentionally don't compare 'output_index' since
    // it's an output parameter of the Hashable, not an input
    // parameter!  Well I guess that's sort of confusing, maybe
    // at some point I will try to make that clearer.
 

    return 0;
}
    
Mesh_Chopper::Mesh_Chopper() {
    plane_epsilon = PLANE_EPSILON_DEFAULT;
}

Mesh_Chopper::~Mesh_Chopper() {
}

void Mesh_Chopper::set_plane_epsilon(float epsilon) {
    assert(epsilon >= 0);
    plane_epsilon = epsilon;
}

void Mesh_Chopper::do_an_edge_split(int n0, int n1,
                                    Vertex_Specifier *spec_return,
                                    int *index_a_return, int *index_b_return) {

    if (n0 < n1) {
        spec_return->input_n0 = n0;
        spec_return->input_n1 = n1;
    } else {
        spec_return->input_n0 = n1;
        spec_return->input_n1 = n0;
    }

    spec_return->plane_index = current_plane_index;

    // This is a little slower than it needs to be
    // but hey, it's written for ease.

    Vector3 p0 = input_mesh->vertices[n0];
    Vector3 p1 = input_mesh->vertices[n1];

    Vector2 uv0 = input_mesh->uvs[n0];
    Vector2 uv1 = input_mesh->uvs[n1];

    Quaternion frame0 = input_mesh->tangent_frames[n0];
    Quaternion frame1 = input_mesh->tangent_frames[n1];

    
    float d0 = dot_product(plane_normal, p0) + plane_d;
    float d1 = dot_product(plane_normal, p1) + plane_d;
    
    float denom = d0 - d1;
    float t;
    if (denom) {
        t = d0 / denom;
    } else {
        t = 0;
    }

    assert(t >= 0);
    assert(t <= 1);

    Vector3 clip_point = p0 + (p1 - p0) * t;
    Vector2 clip_uv = uv0 + (uv1 - uv0) * t;
    Quaternion clip_frame = slerp(frame0, frame1, t);


    int index_a = intermediate_result_a.maybe_add_vertex(clip_point, clip_uv, clip_frame, spec_return);
    int index_b = intermediate_result_b.maybe_add_vertex(clip_point, clip_uv, clip_frame, spec_return);

    // XXX Weird: We encode these outputs as negative numbers...
    // they will go to add_triangle() which uses the sign of the
    // input to know whether it's a vertex in the input or output mesh.

    *index_a_return = -(index_a+1);
    *index_b_return = -(index_b+1);
}


int Mesh_Chopper::xref_vertex(Intermediate_Result *i_result, int index) {
    if (index < 0) {
        return (-index) - 1;
    }

    Vertex_Specifier specifier(index, index, -1);
    int output_index = i_result->maybe_add_vertex(input_mesh->vertices[index],
                                                  input_mesh->uvs[index],
                                                  input_mesh->tangent_frames[index],
                                                  &specifier);
    return output_index;
}

int Mesh_Chopper::xref_vertex_no_creation(Intermediate_Result *i_result, int index) {
    if (index < 0) {
        return (-index) - 1;
    }

    Vertex_Specifier specifier(index, index, -1);
    int output_index = i_result->find_vertex(&specifier);
    return output_index;
}

void Mesh_Chopper::add_triangle(int n0, int n1, int n2, Intermediate_Result *dest_result) {
    int output_n0 = xref_vertex(dest_result, n0);
    int output_n1 = xref_vertex(dest_result, n1);
    int output_n2 = xref_vertex(dest_result, n2);
    dest_result->mesh_builder->add_triangle(output_n0, output_n1, output_n2, current_material_index);
}

int Mesh_Chopper::classify_vertex(int index) {
    Vector3 position = input_mesh->vertices[index];
    float sum = dot_product(plane_normal, position) + plane_d;

    if (fabs(sum) <= plane_epsilon) return 0;
    if (sum < 0) return -1;
    return +1;
}

void Mesh_Chopper::rotate_indices(int *n0, int *n1, int *n2,
                                  int *c0, int *c1, int *c2) {
    int tmp;
    
    // This routine puts the indices into a sort of canonical
    // ordering based on their relationships to the plane.
    // This simplifies the number of cases we have to deal with
    // in the clipping code.

    while ((*c1 > *c0) || (*c2 > *c0)) {
        tmp = *n2;
        *n2 = *n1;
        *n1 = *n0;
        *n0 = tmp;

        tmp = *c2;
        *c2 = *c1;
        *c1 = *c0;
        *c0 = tmp;
    }

    while (*c2 >= 0) {
        tmp = *n0;
        *n0 = *n1;
        *n1 = *n2;
        *n2 = tmp;

        tmp = *c0;
        *c0 = *c1;
        *c1 = *c2;
        *c2 = tmp;
    }
}

void Mesh_Chopper::add_one_triangle(int n0, int n1, int n2) {
    int c0 = classify_vertex(n0);
    int c1 = classify_vertex(n1);
    int c2 = classify_vertex(n2);

    Intermediate_Result *dest_result = NULL;
    if ((c0 >= 0) && (c1 >= 0) && (c2 >= 0)) {
        dest_result = &intermediate_result_b;
    } else if ((c0 <= 0) && (c1 <= 0) && (c2 <= 0)) {
        dest_result = &intermediate_result_a;
    }

    // @Robustness:  Right now coplanar edges are not being
    // handled in this code... that should be fixed in the future 
    // please!

    if (dest_result) {
        add_triangle(n0, n1, n2, dest_result);
        return;
    }

    rotate_indices(&n0, &n1, &n2, &c0, &c1, &c2);

    Vertex_Specifier spec_0(n0, n0, -1);
    Vertex_Specifier spec_1(n1, n1, -1);
    Vertex_Specifier spec_2(n2, n2, -1);

    if (c0 == 0) {
        assert(c1 != 0);
        assert(c2 != 0);

        Vertex_Specifier clipped_point_spec;

        int index_a, index_b;
        do_an_edge_split(n1, n2, &clipped_point_spec, &index_a, &index_b);

        add_triangle(n0, n1, index_b, &intermediate_result_b);
        add_triangle(n0, index_a, n2, &intermediate_result_a);

        intermediate_result_b.register_seam_edge(index_b, n0, &clipped_point_spec, &spec_0);
        intermediate_result_a.register_seam_edge(index_a, n0, &clipped_point_spec, &spec_0);
    } else {
        int index_first_a, index_first_b;
        int index_second_a, index_second_b;

        if (c1 == c0) {
            // Do c1/c2, c2/c0

            assert(c1 != c2);
            assert(c2 != c0);

            Vertex_Specifier spec_first, spec_second;

            do_an_edge_split(n1, n2, &spec_first, &index_first_a, &index_first_b);
            do_an_edge_split(n2, n0, &spec_second, &index_second_a, &index_second_b);

            add_triangle(n0, n1, index_first_b, &intermediate_result_b);
            add_triangle(n0, index_first_b, index_second_b, &intermediate_result_b);
            add_triangle(index_first_a, n2, index_second_a, &intermediate_result_a);

            intermediate_result_b.register_seam_edge(index_first_b, index_second_b,
                                                     &spec_first, &spec_second);
            intermediate_result_a.register_seam_edge(index_first_a, index_second_a,
                                                     &spec_first, &spec_second);
        } else {
            // Do c0/c1, c2/c0
            assert((c1 == c2) || (c1 == 0));
            assert(c2 != c0);

            Vertex_Specifier spec_first, spec_second;

            if (c1 == 0) {
                index_first_a = n1;
                index_first_b = n1;
                spec_first = spec_1;
            } else {
                do_an_edge_split(n0, n1, &spec_first, &index_first_a, &index_first_b);
            }

            do_an_edge_split(n2, n0, &spec_second, &index_second_a, &index_second_b);

            add_triangle(index_second_b, n0, index_first_b, &intermediate_result_b);
            add_triangle(index_first_a, n1, index_second_a, &intermediate_result_a);
            add_triangle(n1, index_second_a, n2, &intermediate_result_a);

            intermediate_result_b.register_seam_edge(index_first_b, index_second_b,
                                                     &spec_first, &spec_second);
            intermediate_result_a.register_seam_edge(index_first_a, index_second_a,
                                                     &spec_first, &spec_second);
        }
    }
}


void add_a_seam_quad(Mesh_Seam *seam,
                     Intermediate_Result *res_a, Intermediate_Result *res_b,
                     Intermediate_Result::Seam_Edge *edge_a,
                     Intermediate_Result::Seam_Edge *edge_b) {

    int cursor = seam->num_faces * 3;
    Seam_Index *index;

    index = &seam->indices[cursor++];
    index->which_mesh = 1;
    index->vertex_index = edge_b->index_1;
    index->uv = res_b->mesh_builder->vertex_uvs[index->vertex_index];

    index = &seam->indices[cursor++];
    index->which_mesh = 1;
    index->vertex_index = edge_b->index_0;
    index->uv = res_b->mesh_builder->vertex_uvs[index->vertex_index];

    index = &seam->indices[cursor++];
    index->which_mesh = 0;
    index->vertex_index = edge_a->index_1;
    index->uv = res_a->mesh_builder->vertex_uvs[index->vertex_index];


    index = &seam->indices[cursor++];
    index->which_mesh = 0;
    index->vertex_index = edge_a->index_1;
    index->uv = res_a->mesh_builder->vertex_uvs[index->vertex_index];

    index = &seam->indices[cursor++];
    index->which_mesh = 1;
    index->vertex_index = edge_b->index_0;
    index->uv = res_b->mesh_builder->vertex_uvs[index->vertex_index];

    index = &seam->indices[cursor++];
    index->which_mesh = 0;
    index->vertex_index = edge_a->index_0;
    index->uv = res_a->mesh_builder->vertex_uvs[index->vertex_index];


    seam->num_faces += 2;
}

inline bool specs_match(Vertex_Specifier *spec_a, Vertex_Specifier *spec_b) {
    // @Speed: Could speed this up by checking the precomputed hash codes first

    int result = spec_a->compare(spec_b);
    if (result != 0) return false;
    return true;
}

inline bool seam_edges_match(Intermediate_Result::Seam_Edge *edge_a,
                             Intermediate_Result::Seam_Edge *edge_b) {
    if (!specs_match(&edge_a->spec_0, &edge_b->spec_0)) return false;
    if (!specs_match(&edge_a->spec_1, &edge_b->spec_1)) return false;
    return true;
}

Mesh_Seam *Mesh_Chopper::build_seam() {
    Intermediate_Result *res_a = &intermediate_result_a;
    Intermediate_Result *res_b = &intermediate_result_b;

    Intermediate_Result::Seam_Edge *edge_a, *edge_b;

    Mesh_Seam *result = new Mesh_Seam(res_a->seam_edges.items * 2);

    result->num_faces = 0;

    Foreach(&res_a->seam_edges, edge_a) {
        // Find a potentially matching edge in b; if it's there,
        // we will make a seam quad.  This loop is O(n^2) right
        // now though it's easy to make it O(n) if it becomes
        // a drag on execution time.  

        // Actually as this demo is currently written, this whole
        // phase is unnecessary since we can do all the seam binding
        // back when we create the edges.  But so that we can deal
        // with more complex boundary cases in the future, and 
        // allow easy out-of-core LOD generation, we do this here in 
        // a separate phase.

        Foreach(&res_b->seam_edges, edge_b) {
            if (seam_edges_match(edge_a, edge_b)) {
                add_a_seam_quad(result, res_a, res_b, edge_a, edge_b);

                // @Robustness:  This break is not really correct for non-manifold geometry, 
                // since we could have other edges that match also.  If we care enough
                // we should fix this!

                break;
            }
        } Endeach;
    } Endeach;

    result->remove_degenerate_faces();
    return result;
}
                                    
void Mesh_Chopper::chop_mesh(Triangle_List_Mesh *_input_mesh,
                             int *_input_flag_array,
                             Chop_Direction direction,
                             Chopped_Result *result_a,
                             Chopped_Result *result_b,
                             Mesh_Seam **seam_a_to_b_return) {

    input_mesh = _input_mesh;
    input_flag_array = _input_flag_array;


    intermediate_result_a.my_chopper = this;
    intermediate_result_b.my_chopper = this;

    // Init stuff...
    intermediate_result_a.init(input_mesh);
    intermediate_result_b.init(input_mesh);

    // Okay now actually do some work...

    Vector3 plane_point(-0.65f, 0, 0);

    switch (direction) {
    case CHOP_X:
        plane_normal = Vector3(1, 0, 0);
        break;
    case CHOP_Y:
        plane_normal = Vector3(0, 1, 0);
        break;
    case CHOP_Z:
        plane_normal = Vector3(0, 0, 1);
        break;
    default:
        assert(0);
        break;
    }

    current_plane_index = (int)direction;
    plane_d = -dot_product(plane_normal, plane_point);


    float d_min = FLT_MAX;
    float d_max = -FLT_MAX;
    
    assert(input_mesh->num_vertices > 0);

    int i;
    for (i = 0; i < input_mesh->num_vertices; i++) {
        float d = dot_product(plane_normal, input_mesh->vertices[i]);
        if (d < d_min) d_min = d;
        if (d > d_max) d_max = d;
    }

    float d = 0.5f * (d_min + d_max);


    for (i = 0; i < input_mesh->num_materials; i++) {
        intermediate_result_a.mesh_builder->add_material(&input_mesh->material_info[i]);
        intermediate_result_b.mesh_builder->add_material(&input_mesh->material_info[i]);
    }

    int k;
    for (k = 0; k < input_mesh->num_triangle_lists; k++) {
        Triangle_List_Info *info = &input_mesh->triangle_list_info[k];
        current_material_index = info->material_index;

        int i;
        for (i = 0; i < info->num_vertices; i += 3) {
            int n0 = input_mesh->indices[info->start_of_list + i + 0];
            int n1 = input_mesh->indices[info->start_of_list + i + 1];
            int n2 = input_mesh->indices[info->start_of_list + i + 2];

            add_one_triangle(n0, n1, n2);
        }
    }

    result_a->result_mesh = intermediate_result_a.mesh_builder->build_mesh();
    result_b->result_mesh = intermediate_result_b.mesh_builder->build_mesh();

    Mesh_Seam *seam = build_seam();
    *seam_a_to_b_return = seam;

    // Cleanup stuff...
    intermediate_result_a.clean();
    intermediate_result_b.clean();
}


Intermediate_Result::Intermediate_Result() {
    mesh_builder = NULL;
}

Intermediate_Result::~Intermediate_Result() {
    clean();
}

void Intermediate_Result::init(Triangle_List_Mesh *mesh) {
    mesh_builder = new Mesh_Builder(mesh->num_vertices * 2,  // This is overconservative
                                            mesh->num_faces * 2);

    vertex_hash_table = new Hash_Table(mesh->num_vertices / 2);
}

void Intermediate_Result::register_seam_edge(int index_0, int index_1, 
                                             Vertex_Specifier *spec_0,
                                             Vertex_Specifier *spec_1) {
    assert(!specs_match(spec_0, spec_1));

    Seam_Edge *edge = new Seam_Edge();

    edge->index_0 = my_chopper->xref_vertex_no_creation(this, index_0);
    edge->index_1 = my_chopper->xref_vertex_no_creation(this, index_1);

    edge->spec_0 = *spec_0;
    edge->spec_1 = *spec_1;

    assert(spec_0->input_n0 >= 0);
    assert(spec_0->input_n1 >= 0);
    assert(spec_1->input_n0 >= 0);
    assert(spec_1->input_n1 >= 0);

    seam_edges.add(edge);
}

void Intermediate_Result::clean() {
    delete mesh_builder;
    mesh_builder = NULL;

    delete vertex_hash_table;
    vertex_hash_table = NULL;

    Seam_Edge *edge;
    Foreach(&seam_edges, edge) { 
        delete edge;
    } Endeach;
}



inline int Vertex_Specifier::get_hash_code() {
    // The +1 is so that, if plane_index is -1, we are guaranteed
    // to have a number >= 0
    return input_n0 * 5 + input_n1 * 3 + plane_index + 1;  // @Speed: Compute a real hash code here instead of this lame thing!
}

int Intermediate_Result::add_vertex(Vector3 position, Vector2 uv, 
                                    Quaternion tangent_frame,
                                    Vertex_Specifier *specifier) {
    int index = mesh_builder->add_vertex(position, uv, tangent_frame);
    specifier->output_index = index;

    vertex_hash_table->add(new Vertex_Specifier(specifier));
    return index;
}

int Intermediate_Result::find_vertex(Vertex_Specifier *specifier) {
    Vertex_Specifier *already_defined = (Vertex_Specifier *)vertex_hash_table->find(specifier);
    if (already_defined) {
        return already_defined->output_index;
    }

    return -1;
}

int Intermediate_Result::maybe_add_vertex(Vector3 position, Vector2 uv,
                                          Quaternion tangent_frame,
                                          Vertex_Specifier *specifier) {
    int index = find_vertex(specifier);
    if (index != -1) return index;

    return add_vertex(position, uv, tangent_frame, specifier);
}
