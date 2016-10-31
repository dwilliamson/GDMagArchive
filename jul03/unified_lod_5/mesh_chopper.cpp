#include "framework.h"
#include "mesh_chopper.h"

#include "float.h"
#include "mesh.h"
#include "mesh_seam.h"
#include "mesh_builder.h"

#include <math.h>


int Vertex_Specifier::compare(Hashable *_other) {
    Vertex_Specifier *other = (Vertex_Specifier *)_other;

    if (other->input_n0 != input_n0) return -1;
    if (other->input_n1 != input_n1) return -1;

    // We intentionally don't compare 'output_index' since
    // it's an output parameter of the Hashable, not an input
    // parameter!  Well I guess that's sort of confusing, maybe
    // at some point I will try to make that clearer.
 

    return 0;
}
    
inline bool specs_match(Vertex_Specifier *spec_a, Vertex_Specifier *spec_b) {
    // @Speed: Could speed this up by checking the precomputed hash codes first

    int result = spec_a->compare(spec_b);
    if (result != 0) return false;
    return true;
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

void Mesh_Chopper::do_an_edge_split(int n0, int n1, int c0, int c1,
                                    Vertex_Specifier *spec_return) {
    // If either end of the edge is on the plane,
    // we return that vertex instead of splitting the edge.
    // (This is an optimization, produces fewer triangles/vertices.
    // The optimization probably doesn't do much usually, but it
    // helps avoid pathological cases that would happen if you,
    // say, chop a regular grid by a plane that passes through
    // one of the grid lines.

    if (c0 == 0) {
        assert(c1 != 0);
        spec_return->input_n0 = n0;
        spec_return->input_n1 = n0;
        return;
    }

    if (c1 == 0) {
        spec_return->input_n0 = n1;
        spec_return->input_n1 = n1;
        return;
    }

    if (n0 < n1) {
        spec_return->input_n0 = n0;
        spec_return->input_n1 = n1;
    } else {
        spec_return->input_n0 = n1;
        spec_return->input_n1 = n0;
    }

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

    float dot = dot_product(frame0, frame1);
    if (dot < 0) frame1 = frame1 * -1;
    Quaternion clip_frame = slerp(frame0, frame1, t);


    int index_a = intermediate_result_a.maybe_add_vertex(clip_point, clip_uv, clip_frame, spec_return);
    int index_b = intermediate_result_b.maybe_add_vertex(clip_point, clip_uv, clip_frame, spec_return);
}


int Mesh_Chopper::xref_vertex(Intermediate_Result *i_result,
                              Vertex_Specifier *spec) {
    int output_index;
    if (spec->input_n0 == spec->input_n1) {
        int index = spec->input_n0;
        output_index = i_result->maybe_add_vertex(input_mesh->vertices[index],
                                                  input_mesh->uvs[index],
                                                  input_mesh->tangent_frames[index],
                                                  spec);
    } else {
        Vertex_Specifier *found = (Vertex_Specifier *)i_result->vertex_hash_table->find(spec);
        assert(found);
        output_index = found->output_index;
    }

    return output_index;
}
/*
int Mesh_Chopper::xref_vertex(Intermediate_Result *i_result, int index) {
    
    if (index < 0) {
        return (-index) - 1;
    }

    Vertex_Specifier specifier(index, index);
    specifier.debug_condition_code = -42;
    int output_index = i_result->maybe_add_vertex(input_mesh->vertices[index],
                                                  input_mesh->uvs[index],
                                                  input_mesh->tangent_frames[index],
                                                  &specifier);
    return output_index;
}
*/
int Mesh_Chopper::xref_vertex_no_creation(Intermediate_Result *i_result, int index) {
    if (index < 0) {
        return (-index) - 1;
    }

    Vertex_Specifier specifier(index, index);
    int output_index = i_result->find_vertex(&specifier);
    return output_index;
}

void Mesh_Chopper::add_triangle(Vertex_Specifier *spec0,
                                Vertex_Specifier *spec1,
                                Vertex_Specifier *spec2,
                                Intermediate_Result *dest_result) {
    // Scan for degenerate triangles; refuse to add them...
    if (specs_match(spec0, spec1)) return;
    if (specs_match(spec1, spec2)) return;
    if (specs_match(spec2, spec0)) return;

    // Okay, add!

    int output_n0 = xref_vertex(dest_result, spec0);
    int output_n1 = xref_vertex(dest_result, spec1);
    int output_n2 = xref_vertex(dest_result, spec2);
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
        dest_result = &intermediate_result_a;
    } else if ((c0 <= 0) && (c1 <= 0) && (c2 <= 0)) {
        dest_result = &intermediate_result_b;
    }

    // @Robustness:  Right now coplanar edges are not being
    // handled in this code... that should be fixed in the future 
    // please!

    if (dest_result) {
        Vertex_Specifier spec_n0(n0, n0);
        Vertex_Specifier spec_n1(n1, n1);
        Vertex_Specifier spec_n2(n2, n2);

        add_triangle(&spec_n0, &spec_n1, &spec_n2, dest_result);
        return;
    }


    rotate_indices(&n0, &n1, &n2, &c0, &c1, &c2);

    Vertex_Specifier spec_n0(n0, n0);
    Vertex_Specifier spec_n1(n1, n1);
    Vertex_Specifier spec_n2(n2, n2);

    spec_n0.debug_condition_code = c0;
    spec_n1.debug_condition_code = c1;
    spec_n2.debug_condition_code = c2;
    /*
      All the code below would probably get simpler if instead
      of using local indices n0, n1, n2, we just passed spec_0,
      spec_1, spec_2 to the various functions like add_triangle,
      and let those guys hash to look it up.  The downside is
      that would be slower.  Maybe we should make two fields
      on Vertex_Spec just to hold the output indices for
      result_a and result_b, we can pass the specs and not
      do a hash.  This would make the code more debuggable
      under maintenance!
    */

    // Register any coplanar edges.
    // This will do maybe not the right thing if c0 == c1 == c2 == 0...
    // need to think about what should really happen in that case.

    /* XXX Aren't all these double register_seam_edge calls redundant?
       Should just have one seam edge place centrally.  Too tired to
       investigate this right now though. 
    */
    if ((c0 == 0) && (c1 == 0)) {
        intermediate_result_a.register_seam_edge(&spec_n0, &spec_n1);
        intermediate_result_b.register_seam_edge(&spec_n0, &spec_n1);
        return;
    }

    if ((c1 == 0) && (c2 == 0)) {
        intermediate_result_a.register_seam_edge(&spec_n1, &spec_n2);
        intermediate_result_b.register_seam_edge(&spec_n1, &spec_n2);
        return;
    }

    if ((c2 == 0) && (c0 == 0)) {
        intermediate_result_a.register_seam_edge(&spec_n2, &spec_n0);
        intermediate_result_b.register_seam_edge(&spec_n2, &spec_n0);
        return;
    }

    assert(c0 >= 0);
    assert(c2 == -1);

    Vertex_Specifier spec_m1, spec_m2;

    if (c1 >= 0) {
        do_an_edge_split(n1, n2, c1, c2, &spec_m1);
        do_an_edge_split(n2, n0, c2, c0, &spec_m2);

        add_triangle(&spec_n0, &spec_n1, &spec_m1, &intermediate_result_a);
        add_triangle(&spec_m1, &spec_m2, &spec_n0, &intermediate_result_a);
        add_triangle(&spec_m1, &spec_n2, &spec_m2, &intermediate_result_b);
    } else {
        do_an_edge_split(n0, n1, c0, c1, &spec_m1);
        do_an_edge_split(n2, n0, c2, c0, &spec_m2);

        add_triangle(&spec_n0, &spec_m1, &spec_m2, &intermediate_result_a);
        add_triangle(&spec_m2, &spec_m1, &spec_n1, &intermediate_result_b);
        add_triangle(&spec_n1, &spec_n2, &spec_m2, &intermediate_result_b);
    }

    intermediate_result_a.register_seam_edge(&spec_m1, &spec_m2);
    intermediate_result_b.register_seam_edge(&spec_m1, &spec_m2);
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
    index->which_mesh = 0;
    index->vertex_index = edge_a->index_1;
    index->uv = res_a->mesh_builder->vertex_uvs[index->vertex_index];

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
    index->vertex_index = edge_a->index_0;
    index->uv = res_a->mesh_builder->vertex_uvs[index->vertex_index];

    index = &seam->indices[cursor++];
    index->which_mesh = 1;
    index->vertex_index = edge_b->index_0;
    index->uv = res_b->mesh_builder->vertex_uvs[index->vertex_index];


    seam->num_faces += 2;
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
                             Plane3 *plane, int plane_id,
                             Chopped_Result *result_a,
                             Chopped_Result *result_b,
                             Mesh_Seam **seam_a_to_b_return) {

    input_mesh = _input_mesh;

    intermediate_result_a.my_chopper = this;
    intermediate_result_b.my_chopper = this;

    // Init stuff...
    intermediate_result_a.init(input_mesh);
    intermediate_result_b.init(input_mesh);

    // Okay now actually do some work...

    current_plane_index = plane_id;




    int i;
    for (i = 0; i < input_mesh->num_materials; i++) {
        intermediate_result_a.mesh_builder->add_material(&input_mesh->material_info[i]);
        intermediate_result_b.mesh_builder->add_material(&input_mesh->material_info[i]);
    }

    plane_d = plane->d;
    plane_normal = Vector3(plane->a, plane->b, plane->c);

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
    
    result_a->vertex_hash_table = intermediate_result_a.vertex_hash_table;
    result_b->vertex_hash_table = intermediate_result_b.vertex_hash_table;

    // Prevent these guys from deleting their hash tables.
    intermediate_result_a.vertex_hash_table = NULL;
    intermediate_result_b.vertex_hash_table = NULL;

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

void Intermediate_Result::register_seam_edge(Vertex_Specifier *spec_0,
                                             Vertex_Specifier *spec_1) {
    assert(!specs_match(spec_0, spec_1));

    spec_0->debug_condition_code = 0;  // It's a seam edge!
    spec_1->debug_condition_code = 0;  // It's a seam edge!

    Seam_Edge *edge = new Seam_Edge();
    edge->index_0 = find_vertex(spec_0);
    edge->index_1 = find_vertex(spec_1);

    assert(edge->index_0 != -1);
    assert(edge->index_1 != -1);

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
    seam_edges.clean();
}



inline int Vertex_Specifier::get_hash_code() {
    return input_n0 * 5 + input_n1 * 3;  // @Speed: Compute a real hash code here instead of this lame thing!
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

