#include "framework.h"
#include "make_world.h"
#include "seam_database.h"
#include "mesh.h"    // Needed for the range-checking assert; no big deal.
#include "mesh_seam.h"

Seam_Database::Seam_Database() {
    num_seams_added_directly = 0;
    num_seams_created_by_rewrites = 0;
    num_seams_merged = 0;
}

Seam_Database::~Seam_Database() {
}

void Seam_Database::add(Mesh_Seam *seam) {
    assert(seam);
    seams.add(seam);
    num_seams_added_directly++;
}

void Seam_Database::remove(Mesh_Seam *seam) {
    seams.remove(seam);
}

int block_id_difference(Mesh_Seam *seam1, Mesh_Seam *seam2, int index) {
    World_Block *block1 = seam1->block_membership[index];
    World_Block *block2 = seam2->block_membership[index];

    if (block1 == NULL) {
        if (block2 == NULL) return 0;
        return -1;
    }

    if (block2 == NULL) return 1;

    if (block1->block_id < block2->block_id) return -1;
    if (block1->block_id > block2->block_id) return +1;

    return 0;
}

static int compare_seams(const void *b1, const void *b2) {
    Mesh_Seam *seam1 = *(Mesh_Seam **)b1;
    Mesh_Seam *seam2 = *(Mesh_Seam **)b2;

    int diff;
    diff = block_id_difference(seam1, seam2, 0);
    if (diff) return diff;

    diff = block_id_difference(seam1, seam2, 1);
    if (diff) return diff;

    diff = block_id_difference(seam1, seam2, 2);
    return diff;
}

// Concatenate two seams together.
static Mesh_Seam *concatenate_seams(Mesh_Seam *a, Mesh_Seam *b) {
    int num_faces = a->num_faces + b->num_faces;
    Mesh_Seam *result = new Mesh_Seam(num_faces);

    result->block_membership[0] = a->block_membership[0];
    result->block_membership[1] = a->block_membership[1];
    result->block_membership[2] = a->block_membership[2];

    int dest_cursor = 0;

    int i;
    for (i = 0; i < a->num_faces * 3; i++) {
        result->indices[dest_cursor++] = a->indices[i];
    }

    for (i = 0; i < b->num_faces * 3; i++) {
        result->indices[dest_cursor++] = b->indices[i];
    }
    
    return result;
}

int Seam_Database::merge_seams() {
    qsort(seams.data, seams.live_items, sizeof(seams.data[0]),
          compare_seams);

    int num_merges = 0;

    int cursor_left = 0;
    while (cursor_left < seams.live_items) {
        int cursor_right = cursor_left + 1;
        while (cursor_right < seams.live_items) {
            Mesh_Seam *seam_left = seams[cursor_left];
            Mesh_Seam *seam_right = seams[cursor_right];
            if (compare_seams(&seam_left, &seam_right) == 0) {
                Mesh_Seam *new_seam = concatenate_seams(seam_left, seam_right);
                delete seam_left;
                delete seam_right;
                seams[cursor_left] = new_seam;
                seams[cursor_right] = NULL;
                cursor_right++;
            } else {
                break;
            }
        }

        cursor_left = cursor_right;
    }

    // Fill in the NULLs now.
    int i;
    for (i = 0; i < seams.live_items; i++) {
        if (seams[i] == NULL) {
            seams[i] = seams[seams.live_items - 1];
            seams.live_items--;
            i--;

            num_merges++;
            num_seams_merged++;
        }
    }

    return num_merges;
}

void Seam_Database::delete_high_resolution_seams() {
    int i;
    for (i = 0; i < seams.live_items; i++) {
        Mesh_Seam *seam = seams[i];

        if (seam->is_high_res_seam) {
            delete seam;
            seams[i] = seams[seams.live_items - 1];
            seams.live_items--;
        }
    }
}

void Seam_Database::collect_seams(World_Block *block, int n0, int n1,
                                  List *results) {
    int j;
    for (j = n0; j < n1; j++) {
        Mesh_Seam *seam = seams[j];

        int i;
        for (i = 0; i < 3; i++) {
            if (seam->block_membership[i] == block) {
                results->add(seam);
                break;
            }
        }
    }
}

void Seam_Database::collect_fully_marked_seams(Auto_Array <Mesh_Seam *> *results) {
    Mesh_Seam *seam;
    Array_Foreach(&seams, seam) {
        bool add = true;

        int i;
        for (i = 0; i < 3; i++) {
            World_Block *block = seam->block_membership[i];
            if (block == NULL) break;
            if (block->block_search_marker == 0) {
                add = false;
                break;
            }
        }

        if (add) results->add(seam);
    } Endeach;
}

Mesh_Seam *copy_seam(Mesh_Seam *seam) {
    Mesh_Seam *result = new Mesh_Seam(seam->num_faces);
    result->is_high_res_seam = seam->is_high_res_seam;

    int i;
    for (i = 0; i < 3; i++) {
        result->block_membership[i] = seam->block_membership[i];
    }

    for (i = 0; i < seam->num_faces * 3; i++) {
        result->indices[i] = seam->indices[i];
    }
    
    return result;
}

void copy_face(Mesh_Seam *seam, int src_index, int dest_index) {
    int i;
    for (i = 0; i < 3; i++) {
        seam->indices[i+dest_index*3] = seam->indices[i+src_index*3];
    }
}

Mesh_Seam *Seam_Database::rewrite_seam(Mesh_Seam *old_seam,
                                       World_Block *source_block,
                                       World_Block *dest_block,
                                       int *index_map,
                                       int max_hierarchy_distance) {
    // Check to see if the resulting seam will be within the
    // proper hierarchy distance.  If it won't, we bail!
    int leaf_distance_min, leaf_distance_max;
    leaf_distance_min = leaf_distance_max = dest_block->leaf_distance;
    
    int i;
    for (i = 0; i < 3; i++) {
        World_Block *other = old_seam->block_membership[i];
        if (!other) break;
        if (other == source_block) continue;
        if (other->leaf_distance < leaf_distance_min) leaf_distance_min = other->leaf_distance;
        if (other->leaf_distance > leaf_distance_max) leaf_distance_max = other->leaf_distance;
    }

    int stride = leaf_distance_max - leaf_distance_min;
    if (stride > max_hierarchy_distance) return NULL;


    // Well it looks okay; let's make a new seam.

    Mesh_Seam *new_seam = copy_seam(old_seam);
    new_seam->is_high_res_seam = false;

    // Find the index for the source block
    int source_index = -1;
    for (i = 0; i < 3; i++) {
        if (new_seam->block_membership[i] == source_block) {
            source_index = i;
            new_seam->block_membership[i] = dest_block;
            break;
        }
    }

    assert(source_index != -1);

    // Now rewrite the mofo.

    for (i = 0; i < new_seam->num_faces * 3; i++) {
        Seam_Index *index = &new_seam->indices[i];
        if (index->which_mesh == source_index) {
            int old_vertex_index = index->vertex_index;
            index->vertex_index = index_map[index->vertex_index];

            assert(index->vertex_index >= -1);
            assert(index->vertex_index < dest_block->mesh->num_vertices);
        }
    }

    // (For now) Eradicate any triangles that now go to nowhere.
    for (i = 0; i < new_seam->num_faces; i++) {
        int n0 = i*3+0;
        int n1 = i*3+1;
        int n2 = i*3+2;

        if ((new_seam->indices[n0].vertex_index == -1) ||
            (new_seam->indices[n1].vertex_index == -1) ||
            (new_seam->indices[n2].vertex_index == -1)) {

            copy_face(new_seam, new_seam->num_faces - 1, i);
            new_seam->num_faces--;
            i--;
        }
    }

    
    // Clean up.

    new_seam->simplify_block_membership();
    if (new_seam->block_membership[1] == NULL) {
        // We have created a seam belonging only to one block...
        // nobody in the system will want to use it, so we just
        // delete it here rather than bloat the database.
        delete new_seam;
        return NULL;
    }

    new_seam->remove_degenerate_faces();

    if (new_seam->num_faces == 0) {
        // No point in keeping an empty seam around!
        delete new_seam;
        return NULL;
    }

    return new_seam;
}

void Seam_Database::perform_rewrite_rule(Rewrite_Rule *rule,
                                         int max_hierarchy_distance) {

    List seams_to_rewrite;
    collect_seams(rule->source_block, 0, seams.live_items, &seams_to_rewrite);

    Mesh_Seam *old_seam;
    Foreach(&seams_to_rewrite, old_seam) {
        Mesh_Seam *new_seam = rewrite_seam(old_seam, rule->source_block,
                                           rule->dest_block, 
                                           rule->index_map, max_hierarchy_distance);
        if (new_seam) {
            num_seams_created_by_rewrites++;
            seams.add(new_seam);
        }
    } Endeach;

    // Now we'll coalesce seams...
    while (1) {
        int num_changes = merge_seams();
        if (num_changes == 0) break;
    }
}


void Seam_Database::find_seams_containing_these_blocks(World_Block **blocks, int num_blocks, 
                                                       Auto_Array <Mesh_Seam *> *results) {

    // Mark the blocks...
    int i;
    for (i = 0; i < num_blocks; i++) blocks[i]->block_search_marker = 1;

    // Find seams whose membership consists of all-marked blocks.
    Mesh_Seam *seam;
    Array_Foreach(&seams, seam) {
        bool add = true;

        for (i = 0; i < 3; i++) {
            World_Block *block = seam->block_membership[i];
            if (block == NULL) break;
            if (block->block_search_marker == 0) {
                add = false;
                break;
            }
        }

        if (add) results->add(seam);
    } Endeach;

    // Unmark all the blocks.
    for (i = 0; i < num_blocks; i++) blocks[i]->block_search_marker = 0;
}

void Seam_Database::find_seams_containing_at_least_this_block(World_Block *target_block,
                                                             Auto_Array <Mesh_Seam *> *results) {

    Mesh_Seam *seam;
    Array_Foreach(&seams, seam) {
        bool add = false;

        int i;
        for (i = 0; i < 3; i++) {
            World_Block *block = seam->block_membership[i];
            if (block == target_block) add = true;
        }

        if (add) results->add(seam);
    } Endeach;
}

void Seam_Database::delete_seams_that_touch_marked_blocks() {
    int i;
    for (i = 0; i < seams.live_items; i++) {
        Mesh_Seam *seam = seams[i];
        bool remove = false;
        
        int j;
        for (j = 0; j < 3; j++) {
            World_Block *block = seam->block_membership[j];
            if (block && block->block_search_marker) {
                remove = true;
                break;
            }
        }

        if (remove) {
            delete seam;
            seams[i] = seams[seams.live_items - 1];
            seams.live_items--;
            i--;
        }
    }
}
