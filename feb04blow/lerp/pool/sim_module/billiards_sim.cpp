#include "general.h"
#include "hash_table.h"

#include "parser.h"
#include <stdlib.h>
#include <ctype.h>

#include "interp.h"
#include "schema.h"

#include "parser_private.h"
#include "bytecode_runner.h"
#include "goal_solver.h"
#include "printer.h"
#include "memory_manager_private.h"
#include "profiler_ticks.h"

#include "unicode.h"  // For string stuff
#include "robust_math.h"

#include "module_helper.h"

#include <math.h>




extern "C" {
    DLL_EXPORT void get_module_info(Lerp_Module_Init_Info *info_return);
    DLL_EXPORT Database *instantiate(Lerp_Interp *);
    DLL_EXPORT void init(Lerp_Interp *);
    DLL_EXPORT void shutdown(Lerp_Interp *);
    DLL_EXPORT void enumerate_gc_roots(Lerp_Interp *);
};



const int FRICTION = 1;
const double FRICTION_CONSTANT = 0.1;
const double WALL_COEFFICIENT_OF_RESTITUTION = 0.3;
const double BALL_COEFFICIENT_OF_RESTITUTION = 0.85;

struct Vector2 {
    double x, y;

    Vector2(double x, double y);
    Vector2() {};

    void set(double x, double y);
    void normalize_or_zero();
    const float length() const;
    const float length_squared() const;
};

inline Vector2::Vector2(double _x, double _y) {
    x = _x; 
    y = _y; 
}

inline Vector2 const operator +=(Vector2 &v0, Vector2 const &v1) {
    v0.x += v1.x;
    v0.y += v1.y;
    return v0;
}

inline Vector2 const operator -=(Vector2 &v0, Vector2 const &v1) {
    v0.x -= v1.x;
    v0.y -= v1.y;
    return v0;
}

inline Vector2 const operator +(const Vector2 &v0, const Vector2 &v1) {
    return Vector2(v0.x + v1.x, v0.y + v1.y);
}

inline const Vector2 operator -(const Vector2 &v0, const Vector2 &v1) {
    return Vector2(v0.x-v1.x, v0.y-v1.y);
}

inline const Vector2 operator *(const Vector2 &v0, double factor) {
    return Vector2(v0.x*factor, v0.y*factor);
}

inline const Vector2 operator *(double factor, const Vector2 &v0) {
    return Vector2(v0.x*factor, v0.y*factor);
}


// Find the dot product of 2-vectors v1 and v2.
inline double dot_product(Vector2 &v1, Vector2 &v2) {
    return ((double)v1.x * (double)v2.x + 
	    (double)v1.y * (double)v2.y);
}

inline double dot_product(const Vector2 &v1, const Vector2 &v2) {
    return ((double)v1.x * (double)v2.x + 
            (double)v1.y * (double)v2.y);
}



// Not inline
void Vector2::normalize_or_zero() {
    double dx, dy;

    dx = (double) x;	
    dy = (double) y;

    double sq = sqrt(dx * dx + dy * dy);
    if (sq == 0) return;

	double factor = 1.0 / sq;
    x = dx * factor;
    y = dy * factor;
}

const float Vector2::length() const {
    double dx,dy;

    dx = (double) x;	
    dy = (double) y;

    float sq = (float)sqrt(dx * dx + dy * dy);
    return sq;
}



const int BALLS_MAX = 20;

struct Ball_State {
    Vector2 position;
    Vector2 velocity;
};

struct Ball_Constant_State {
    double r, g, b;
    double radius;
    double mass;
};

struct Pool_State {
    double timestep;
    int num_balls;
    double play_field_width, play_field_height;

    unsigned int last_collision_id;

    Ball_State ball_state[BALLS_MAX];
    Ball_State ball_state_t1[BALLS_MAX];
    Ball_Constant_State ball_constant_state[BALLS_MAX];
};


enum Physics_Event_Type {
    NO_EVENT = 0,
    WALL_COLLISION,
    BALL_COLLISION
};

struct Physics_Event {
    Physics_Event_Type type;
    int ball_identifier;
    int other_identifier;
    double fraction;
    Vector2 normal;
};


Atom *the_x_atom;
Atom *the_y_atom;

void brik() {
}


const int COLLISION_ID_NONE = 0xffffffff;
unsigned int make_collision_id(Physics_Event *event) {
    unsigned int result = (event->type << 24) | (event->ball_identifier << 16) | event->other_identifier;
    return result;
}

bool get_vector_float_member(Lerp_Interp *interp, Database *owner, Integer *index, double *result) {
    Decl_Assertion *assertion = owner->assertions->read();

    for (; assertion; assertion = assertion->next->read()) {
        Decl_Expression *expression = assertion->expression->read();
        First_Class *fc_domain = expression->arguments[0]->read();
        if (fc_domain->type != ARG_INTEGER) continue;

        Integer *domain = (Integer *)fc_domain;

        if (domain->value == index->value) {
            if (expression->num_arguments != 2) {
                interp->report_error("Vector has a weird number of arguments in coordinate assertion %d.\n", index->value);
                return false;
            }

            First_Class *fc_range = expression->arguments[1]->read();
            if (fc_range->type != ARG_FLOAT) {
                interp->report_error("Vector has a non-float value in slot 1 of assertion %d.\n", index->value);
                return false;
            }

            Float *range = (Float *)fc_range;
            *result = range->value;
            return true;
        }
    }

    interp->report_error("Could not find an assertion for vector index %d\n", index->value);
    return false;
}

Database *get_database_member(Lerp_Interp *interp, Database *owner, char *name) {
    First_Class *ptr = owner->lookup_named_slot(interp, interp->parser->make_atom(name));

    if (!ptr || (ptr->type != ARG_DATABASE)) {
        interp->report_error("No database member '%s'\n", name);
        return NULL;
    }

    return (Database *)ptr;
}


static void extract_vector(Lerp_Interp *interp, Database *db, char *name,
                           Vector2 *vec_return) {
    First_Class *fc = get_database_member(interp, db, name);
    if (!fc) {
        interp->report_error("Ball didn't have a member '%s'.\n", name);
        return;
    }

    double x, y;
    bool success;

    Database *db_vector = (Database *)fc;
    success = get_vector_float_member(interp, db_vector, interp->parser->make_integer(1), &x);
    if (!success) return;
    success = get_vector_float_member(interp, db_vector, interp->parser->make_integer(2), &y);
    if (!success) return;

    vec_return->x = x;
    vec_return->y = y;
}

static void extract_float(Lerp_Interp *interp, Database *db, char *name,
                          double *result) {
    First_Class *fc = db->lookup_named_slot(interp, interp->parser->make_atom(name));
    if (!fc) {
        interp->report_error("Ball didn't have a member '%s'.\n", name);
        return;
    }

    if (fc->type != ARG_FLOAT) {
        interp->report_error("Ball member '%s' was not a float.\n", name);
        return;
    }

    Float *f_result = (Float *)fc;
    *result = f_result->value;
}

static void extract_ball_state(Lerp_Interp *interp, Database *db, Ball_State *ball) {
    extract_vector(interp, db, "position", &ball->position);
    extract_vector(interp, db, "velocity", &ball->velocity);
}

static void extract_ball_constant_state(Lerp_Interp *interp, Database *db, Ball_Constant_State *ball_constant) {
    extract_float(interp, db, "r", &ball_constant->r);
    extract_float(interp, db, "g", &ball_constant->g);
    extract_float(interp, db, "b", &ball_constant->b);
    extract_float(interp, db, "radius", &ball_constant->radius);
    extract_float(interp, db, "mass", &ball_constant->mass);
}

static void extract_state(Lerp_Interp *interp, Database *db, Pool_State *state) {
    state->num_balls = 0;

    Decl_Assertion *assertion = db->assertions->read();

    for (; assertion; assertion = assertion->next->read()) {
        Decl_Expression *expression = assertion->expression->read();
        if (expression->num_arguments < 1) continue;

        First_Class *fc_type_string = expression->arguments[0]->read();
        if (fc_type_string->type != ARG_STRING) continue;

        String *type_string = (String *)fc_type_string;
        if (Unicode::strings_match(type_string->value, "Entity")) {
            if (expression->num_arguments != 2) continue;

            int index = state->num_balls++;
            Ball_State *ball = &state->ball_state[index];
            Database *entity_db = (Database *) expression->arguments[1]->read();
            if (entity_db->type != ARG_DATABASE) {
                interp->report_error("A non-database was stored with an Entity label!\n");
                return;
            }

            extract_ball_state(interp, entity_db, ball);
            extract_ball_constant_state(interp, entity_db, &state->ball_constant_state[index]);
        }
    }

    extract_float(interp, interp->global_database, "play_field_width", &state->play_field_width);
    extract_float(interp, interp->global_database, "play_field_height", &state->play_field_height);
//    state->play_field_width = 1.6;
//    state->play_field_height = 3.0;

/*
    int i;
    for (i = 0; i < state->num_balls; i++) {
        Ball_State *ball = &state->ball_state[i];
        printf("Ball %d: (%.4f, %.4f); (%.4f, %.4f)\n",
               i, ball->x, ball->y, ball->vx, ball->vy);
    }
*/
}
    

static Database *gen_vector(Lerp_Interp *interp, Schema *schema, const Vector2 &vec) {
    Database *db = GC_NEW(Database);
    db->schema = ToBarrier(schema);

    // @Incomplete: Set type tag here

    db->add_assertion(interp, interp->parser->make_integer(1), interp->parser->make_float(vec.x));
    db->add_assertion(interp, interp->parser->make_integer(2), interp->parser->make_float(vec.y));
    return db;
}

static Database *gen_state(Lerp_Interp *interp, Pool_State *state,
                           Schema *db_schema, Schema *entity_schema, Schema *vector_schema) {
    Atom *member_atom = interp->member_atom;
    Atom *position_atom = interp->parser->make_atom("position");
    Atom *velocity_atom = interp->parser->make_atom("velocity");
    Atom *timestep_atom = interp->parser->make_atom("timestep");
    Atom *cue_ball_atom = interp->parser->make_atom("cue_ball");
    Atom *radius_atom = interp->parser->make_atom("radius");
    Atom *r_atom = interp->parser->make_atom("r");
    Atom *g_atom = interp->parser->make_atom("g");
    Atom *b_atom = interp->parser->make_atom("b");
    Atom *mass_atom = interp->parser->make_atom("mass");

    String *entity_string = interp->parser->make_string("Entity");

    Database *db = GC_NEW(Database);
    db->add_assertion(interp, member_atom, timestep_atom, interp->parser->make_float(state->timestep));
    
    int i;
    // XXX Temporary...
//    for (i = state->num_balls - 1; i >= 0; i--) {
    for (i = 0; i < state->num_balls; i++) {
        Ball_State *ball = &state->ball_state_t1[i];
        Ball_Constant_State *ball_constant = &state->ball_constant_state[i];
        Database *position = gen_vector(interp, vector_schema, ball->position);
        Database *velocity = gen_vector(interp, vector_schema, ball->velocity);

        Database *db_ball = GC_NEW(Database);
        db_ball->schema = ToBarrier(entity_schema);
        db_ball->add_assertion(interp, member_atom, position_atom, position);
        db_ball->add_assertion(interp, member_atom, velocity_atom, velocity);
        db_ball->add_assertion(interp, member_atom, radius_atom, interp->parser->make_float(ball_constant->radius));
        db_ball->add_assertion(interp, member_atom, mass_atom, interp->parser->make_float(ball_constant->mass));
        db_ball->add_assertion(interp, member_atom, r_atom, interp->parser->make_float(ball_constant->r));
        db_ball->add_assertion(interp, member_atom, g_atom, interp->parser->make_float(ball_constant->g));
        db_ball->add_assertion(interp, member_atom, b_atom, interp->parser->make_float(ball_constant->b));

        db->add_assertion(interp, entity_string, db_ball);

        if ((ball_constant->r == ball_constant->g) && (ball_constant->r == ball_constant->b) && (ball_constant->r != 0.0)) {
            db->add_assertion(interp, member_atom, cue_ball_atom, db_ball);
        }
    }

    db->schema = ToBarrier(db_schema);
    return db;
}


static void swap_time_arrays(Pool_State *state) {
    int i;
    for (i = 0; i < state->num_balls; i++) {
        Ball_State *ball_t0 = &state->ball_state[i];
        Ball_State *ball_t1 = &state->ball_state_t1[i];

        *ball_t0 = *ball_t1;
    }
}


static void interpolate_ball_positions(Pool_State *state, double fraction) {
    int i;
    for (i = 0; i < state->num_balls; i++) {
        Ball_State *ball_t0 = &state->ball_state[i];
        Ball_State *ball_t1 = &state->ball_state_t1[i];

        ball_t0->position += fraction * (ball_t1->position - ball_t0->position);
        ball_t0->velocity += fraction * (ball_t1->velocity - ball_t0->velocity);
    }
}

inline void run_physics(Pool_State *state, int ball_index) {
    Ball_State *ball_t0 = &state->ball_state[ball_index];
    Ball_State *ball_t1 = &state->ball_state_t1[ball_index];

    double dt = state->timestep;

    Vector2 dx = ball_t0->velocity * dt;
    ball_t1->position = ball_t0->position + dx;

    if (FRICTION) {
        double dv_scale = FRICTION_CONSTANT * dt;
        ball_t1->velocity = ball_t0->velocity * (1.0 - dv_scale);
    } else {
        ball_t1->velocity = ball_t0->velocity;
    }
}

inline void event_check_lt(double x0, double x1, double limit, int ball_index,
                           double normal_x, double normal_y, int wall_index,
                           Pool_State *state, Physics_Event *event) {
    assert(x0 >= limit);
    if (x1 >= limit) return;
    
    double fraction = (limit - x0) / (x1 - x0);  // This divide ought to be safe...
    if (fraction < 0.0) fraction = 0.0;
    if (fraction > 1.0) fraction = 1.0;

    if (fraction > event->fraction) return;
    event->type = WALL_COLLISION;
    event->fraction = fraction;
    event->ball_identifier = ball_index;
    event->other_identifier = wall_index;
    event->normal = Vector2(normal_x, normal_y);
}

inline void event_check_gt(double x0, double x1, double limit, int ball_index,
                           double normal_x, double normal_y, int wall_index,
                           Pool_State *state, Physics_Event *event) {
    assert(x0 <= limit);
    if (x1 <= limit) return;
    
    double fraction = (limit - x0) / (x1 - x0);  // This divide ought to be safe...
    if (fraction < 0.0) fraction = 0.0;
    if (fraction > 1.0) fraction = 1.0;

    if (fraction > event->fraction) return;
    event->type = WALL_COLLISION;
    event->fraction = fraction;
    event->ball_identifier = ball_index;
    event->other_identifier = wall_index;
    event->normal = Vector2(normal_x, normal_y);
}

static void check_ball_collision(Pool_State *state, int index_0, int index_1,
                                 Physics_Event *event) {

    Ball_State *a0 = &state->ball_state[index_0];
    Ball_State *a1 = &state->ball_state_t1[index_0];
    Ball_State *b0 = &state->ball_state[index_1];
    Ball_State *b1 = &state->ball_state_t1[index_1];

    double ra = state->ball_constant_state[index_0].radius;
    double rb = state->ball_constant_state[index_1].radius;

    Vector2 ox = a0->position - b0->position;
    Vector2 sum = a1->position - a0->position - (b1->position - b0->position);

    double gamma = dot_product(ox, ox) - (ra + rb)*(ra + rb);
    double beta = 2 * dot_product(ox, sum);
    double alpha = dot_product(sum, sum);

    Solution_Set solutions;
    solve_quadratic(alpha, beta, gamma, &solutions);
//    printf("Num solutions: %d  (a %.9f, b %.3f, c %.3f)\n", solutions.num_solutions, alpha, beta, gamma);
    if (solutions.num_solutions == 0) return;


    double t;
    if (solutions.num_solutions == 1) {
        t = solutions.solutions[0];
        if (t < 0) return;
        if (t > 1) return;
    } else {
        assert(solutions.solutions[1] > solutions.solutions[0]);

        if (solutions.solutions[1] < 0) return;
        if (solutions.solutions[0] > 1) return;

        if ((solutions.solutions[0] < 0.0) && (solutions.solutions[1] >= 0.0)) {
            t = 0.0;
        } else {
            if (solutions.solutions[0] >= 0.0) t = solutions.solutions[0];
            else t = solutions.solutions[1];

            assert(t >= 0.0);
            assert(t <= 1.0);
        }
// XXXXX Some solutions are insanely out of range.
//        printf("Solution: %.5f ; %.5f\n", solutions.solutions[0], solutions.solutions[1]);  
    }

//    printf("Solution t: %f\n", t);
    if (t > event->fraction) return;

/*

    {
        // if (simulation_call & 1) {
        int tmp = index_0;
        index_0 = index_1;
        index_1 = tmp;
        //}

    Ball_State *a0 = &state->ball_state[index_0];
    Ball_State *a1 = &state->ball_state_t1[index_0];
    Ball_State *b0 = &state->ball_state[index_1];
    Ball_State *b1 = &state->ball_state_t1[index_1];

    double ra = state->ball_constant_state[index_0].radius;
    double rb = state->ball_constant_state[index_1].radius;

    Vector2 ox2 = a0->position - b0->position;
    Vector2 sum2 = a1->position - a0->position + b1->position - b0->position;

    double gamma2 = dot_product(ox2, ox2) - (ra + rb)*(ra + rb);
    double beta2 = 2 * dot_product(ox2, sum2);
    double alpha2 = dot_product(sum2, sum2);

    Solution_Set solutions2;
    solve_quadratic(alpha2, beta2, gamma2, &solutions2);

    assert(0);
    }

*/





    Vector2 velocity = a0->velocity + b0->velocity;
    Vector2 delta = b0->position - a0->position;
    double dot = dot_product(velocity, delta);
    if ((t == 0.0) && (dot <= 0)) return;
//    if (t == 0.0) printf("Dot is: %.5f\n", dot);
//    if (dot < 0) return;
//    const double EPSILON = 0.001;
//    if ((dot <= 0) && (t < EPSILON)) return;


    Physics_Event new_event;
    new_event.type = BALL_COLLISION;
    new_event.fraction = t;
    new_event.ball_identifier = a0 - state->ball_state;  // @Refactor: Unclean
    new_event.other_identifier = b0 - state->ball_state;  // @Refactor: Unclean

    // Compute the collision normal by computing ball centers at time of collision
    // then taking the vector between them.
    Vector2 pa = a0->position + t * (a1->position - a0->position);
    Vector2 pb = b0->position + t * (b1->position - b0->position);
    Vector2 normal = pa - pb;
    normal.normalize_or_zero();
    new_event.normal = normal;

    if (make_collision_id(&new_event) != state->last_collision_id) {
        *event = new_event;
    }
}

static void find_earliest_event(Pool_State *state, int ball_index, Physics_Event *event) {
    Ball_State *ball_t0 = &state->ball_state[ball_index];
    Ball_State *ball_t1 = &state->ball_state_t1[ball_index];
    Ball_Constant_State *ball_constant = &state->ball_constant_state[ball_index];
    double radius = ball_constant->radius;

    double x0 = radius;
    double x1 = state->play_field_width - radius;
    double y0 = radius;
    double y1 = state->play_field_height - radius;

    event_check_lt(ball_t0->position.x, ball_t1->position.x, x0, ball_index, 1.0, 0.0, 0, state, event);
    event_check_lt(ball_t0->position.y, ball_t1->position.y, y0, ball_index, 0.0, 1.0, 1, state, event);
    event_check_gt(ball_t0->position.x, ball_t1->position.x, x1, ball_index, -1.0, 0.0, 2, state, event);
    event_check_gt(ball_t0->position.y, ball_t1->position.y, y1, ball_index, 0.0, -1.0, 3, state, event);

    int i;
    for (i = ball_index + 1; i < state->num_balls; i++) {
        check_ball_collision(state, ball_index, i, event);
    }
}

static void deflect_ball_from_normal(Ball_State *ball, const Vector2 &normal) {
    double dot = dot_product(ball->velocity, normal);
    ball->velocity -= 2 * dot * normal;
}

static void enact_collision(Pool_State *state, Physics_Event *event) {
    assert(event->type != NO_EVENT);

    int index = event->ball_identifier;
    assert(index >= 0);
    assert(index < state->num_balls);
    
    Ball_State *ball = &state->ball_state[index];

    if (event->type == WALL_COLLISION) {
        // @Sophistication: Do better math for this, model the collision over time?
        
        Vector2 dir = event->normal;
        double va = dot_product(dir, ball->velocity);

        // XXX ma/ima not necessary?!
        double ma = state->ball_constant_state[index].mass;
        double ima = 1.0 / ma;

        double j = -ma * va * (1 + WALL_COEFFICIENT_OF_RESTITUTION);
        Vector2 impulse = dir * j;

        Vector2 delta_v_a = impulse * ima;
        ball->velocity += delta_v_a;
    } else {
        int other_index = event->other_identifier;
        assert(other_index >= 0);
        assert(other_index < state->num_balls);

        Ball_State *other_ball = &state->ball_state[other_index];

        double ma = state->ball_constant_state[index].mass;
        double mb = state->ball_constant_state[other_index].mass;
        double ima = 1.0 / ma;
        double imb = 1.0 / mb;

        Vector2 va = ball->velocity;
        Vector2 vb = other_ball->velocity;
        Vector2 dir = ball->position - other_ball->position;
        dir.normalize_or_zero();  // XXXX 0 would be very bad

        Vector2 dv = va - vb;
        double c0 = dot_product(dir, dv);
        
        double j = -c0 * (1 + BALL_COEFFICIENT_OF_RESTITUTION) / (ima + imb);
        Vector2 impulse = dir * j;

        Vector2 delta_v_a = impulse * ima;
        Vector2 delta_v_b = impulse * imb * -1.0;

        ball->velocity += delta_v_a;
        other_ball->velocity += delta_v_b;

//        double E1 = 0.5 * ma * dot_product(ball->velocity, ball->velocity) + 0.5 * mb * dot_product(other_ball->velocity, other_ball->velocity);

        brik();

//        double E1 = ma * dot_product(ball->velocity, ball->velocity) + mb * dot_product(other_ball->velocity, other_ball->velocity);

        state->last_collision_id = make_collision_id(event);


    }
}

/*
static void maybe_swap(Pool_State *state) {
    int i;
    for (i = 0; i < state->num_balls; i++) {
        int j = state->num_balls - 1 - i;
        if (j <= i) break;  // AAAARGGGHHHH!!!!

        Ball_State tmp = state->ball_state[i];
        state->ball_state[i] = state->ball_state[j];
        state->ball_state[j] = tmp;

        tmp = state->ball_state_t1[i];
        state->ball_state_t1[i] = state->ball_state_t1[j];
        state->ball_state_t1[j] = tmp;

        Ball_Constant_State tmp2 = state->ball_constant_state[i];
        state->ball_constant_state[i] = state->ball_constant_state[j];
        state->ball_constant_state[j] = tmp2;
    }
}
*/

static void run_timestep(Pool_State *state) {
    int i;
    for (i = 0; i < state->num_balls; i++) {
        // We just run physics to compute the endpoints of each object's motion
        // without collision detecting; then afterward, we collision detect,
        // and just linearize to interpolate between the endpoints.
        run_physics(state, i);
    }

    double dt = state->timestep;

    Physics_Event event;
    event.type = NO_EVENT;
    event.fraction = 2.0;

    for (i = 0; i < state->num_balls; i++) {
        find_earliest_event(state, i, &event);  // Only finds events earlier than event.time
    }
    
    if (event.fraction < 1.0) {
        assert(event.type != NO_EVENT);
        assert(dt > 0.0);
        assert(event.fraction >= 0.0);

        double fraction = event.fraction;
        interpolate_ball_positions(state, fraction);

        dt *= fraction;
        state->timestep -= dt;

        enact_collision(state, &event);
        
        if (state->timestep > 0.0) run_timestep(state);
        return;
    } else {
        swap_time_arrays(state);
        return;
    }
}

static Schema *get_schema(Lerp_Interp *interp, char *name) {
    Schema *schema = (Schema *)interp->global_database->lookup_named_slot(interp, interp->parser->make_atom(name));
    if ((schema == NULL) || (schema->type != ARG_SCHEMA)) {
        interp->report_error("Unable to find a schema called '%s'\n", name);
        if (schema) interp->report_error("(Because its type is not SCHEMA.)\n");
        return NULL;
    }

    return schema;
}

static void gen_state_and_return(Lerp_Interp *interp, Lerp_Call_Record *record,
                                 Pool_State *state, Database *db) {
    Schema *db_schema = db->schema->read();
    Schema *entity_schema = get_schema(interp, "Entity");
    Schema *vector_schema = get_schema(interp, "Vector3");
    
    if (!entity_schema) return;
    if (!vector_schema) return;

    Database *result = gen_state(interp, state, db_schema, entity_schema, vector_schema);

    Return(result);
}

void proc_simulate(Lerp_Interp *interp, Lerp_Call_Record *record) {
    bool success = argument_count(interp, record, 2);
    if (!success) return;

    First_Class *fc_db = record->registers[1]->read();
    if (fc_db->type != ARG_DATABASE) {
        interp->report_error("Type mismatch; error 1 must be a database.\n");
        return;
    }

    double timestep;
    success = coerce_to_float(interp, record, 1, &timestep);
    if (!success) return;

    Pool_State state;
    state.timestep = timestep;
    extract_state(interp, (Database *)fc_db, &state);


    state.last_collision_id = COLLISION_ID_NONE;
    run_timestep(&state);

    gen_state_and_return(interp, record, &state, (Database *)fc_db);
}

void proc_simulate_until_everything_is_at_rest(Lerp_Interp *interp, Lerp_Call_Record *record) {
    bool success = argument_count(interp, record, 2);
    if (!success) return;

    First_Class *fc_db = record->registers[1]->read();
    if (fc_db->type != ARG_DATABASE) {
        interp->report_error("Type mismatch; error 1 must be a database.\n");
        return;
    }

    double timestep;
    success = coerce_to_float(interp, record, 1, &timestep);
    if (!success) return;

    Pool_State state;
    extract_state(interp, (Database *)fc_db, &state);

    const double LENGTH_TO_STOP = 0.01;

    while (1) {
        bool can_stop = true;

        state.timestep = timestep;
        state.last_collision_id = COLLISION_ID_NONE;

        run_timestep(&state);

        int i;
        for (i = 0; i < state.num_balls; i++) {
            Ball_State *ball = &state.ball_state[i];
            if (ball->velocity.length() > LENGTH_TO_STOP) can_stop = false;
        }

        if (can_stop) break;
    }

    gen_state_and_return(interp, record, &state, (Database *)fc_db);
}





void get_module_info(Lerp_Module_Init_Info *info) {
    info->system_version = LERP_SYSTEM_VERSION;
    info->default_name = "Billiards_Sim";
};

void init(Lerp_Interp *interp) {
    the_x_atom = interp->parser->make_atom("x");
    the_y_atom = interp->parser->make_atom("y");
}

Database *instantiate(Lerp_Interp *interp) {
    Database *db = GC_NEW(Database);

    Register(simulate);
    Register(simulate_until_everything_is_at_rest);

    return db;
}

void shutdown(Lerp_Interp *) {
}

void enumerate_gc_roots(Lerp_Interp *) {
}

