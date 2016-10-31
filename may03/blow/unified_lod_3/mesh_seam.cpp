#include "framework.h"
#include "mesh.h"
#include "mesh_seam.h"

Mesh_Seam::Mesh_Seam(int _num_faces) {
    num_faces = _num_faces;
    indices = new Seam_Index[num_faces * 3];
}

Mesh_Seam::~Mesh_Seam() {
    delete [] indices;
}

inline bool indices_match(Seam_Index *n0, Seam_Index *n1) {
    if (n0->which_mesh != n1->which_mesh) return false;
    if (n0->vertex_index != n1->vertex_index) return false;

    return true;
}

// This version of Mesh_Seam::compute_uv_coordinates is starting
// to get a little silly.  It's my suspicion that we might do better
// just copying the coordinates from one side of the mesh, and 
// giving the seam an area of 0 in uv-space.
void Mesh_Seam::compute_uv_coordinates(Triangle_List_Mesh *mesh) {
    int num_indices = num_faces * 3;

    int last_anchor_point = -1;
    int num_anchor_points = 0;

    Vector3 p0, p1;
    Vector2 u0, u1;

    int i;
    for (i = 0; i < num_indices; i++) {
        if (indices[i].which_mesh == 0) {
            if (num_anchor_points == 0) {
                num_anchor_points++;

                last_anchor_point = i;
                p0 = mesh->vertices[i];
                u0 = mesh->uvs[i];
            } else {
                if (indices[i].vertex_index == indices[last_anchor_point].vertex_index) continue;

                num_anchor_points++;

                p1 = mesh->vertices[i];
                u1 = mesh->uvs[i];
            }
        }
    }

    if (num_anchor_points < 2) {
        Vector2 uv;

        if (num_anchor_points == 0) {
            uv = Vector2(0, 0);
        } else {
            assert(last_anchor_point >= 0);
            int vertex_index = indices[last_anchor_point].vertex_index;
            uv = mesh->uvs[vertex_index];
        }

        int i;
        for (i = 0; i < num_indices; i++) {
            indices[i].uv = uv;
        }
    } else {
        // @Study: Seems like we have to break things into a direction
        // and length here, which is weird.  Can Clifford algebra help us
        // with this?
        Vector3 dp = p1 - p0;
        Vector2 du = u1 - u0;

        float dp_len = dp.length();
        Vector3 dp_dir;

        if (dp_len == 0) {
            dp_len = 1;
            dp_dir = Vector3(0, 0, 0);
        } else {
            dp_dir = dp;
            dp_dir.normalize();
        }

        int i;
        for (i = 0; i < num_indices; i++) {
            Vector3 pos = mesh->vertices[i];
            Vector3 relative = pos - p0;

            float proj_len = dot_product(relative, dp_dir);
            Vector2 uv = u0 + du * (proj_len / dp_len);

            indices[i].uv = uv;
            // Vector3 projection = relative - (dp * dot_product(relative, dp_dir));
        }            
    }
}

void Mesh_Seam::remove_degenerate_faces() {
    int i;
    for (i = 0; i < num_faces; i++) {
        Seam_Index *n0 = &indices[i*3+0];
        Seam_Index *n1 = &indices[i*3+1];
        Seam_Index *n2 = &indices[i*3+2];

        if (indices_match(n0, n1) || indices_match(n0, n2) || indices_match(n1, n2)) {
            int k = num_faces - 1;
            indices[i*3+0] = indices[k*3+0];
            indices[i*3+1] = indices[k*3+1];
            indices[i*3+2] = indices[k*3+2];
            num_faces--;
            i--;
        }
    }
}

