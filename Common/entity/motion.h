#ifndef _MOTION_H
#define _MOTION_H

#include <utilitieslib/stdtypes.h>

typedef struct Entity Entity;
typedef struct ControlState ControlState;
typedef struct SeqInst SeqInst;
typedef struct DefTracker DefTracker;

#define NO_BOUNCE 0
#define DO_BOUNCE 1

// Web Swing mode gives a normal ground jump a stronger launch without
// changing the ordinary jump when the mode is disabled.
#define WEB_LAUNCH_JUMP_HEIGHT_SCALE 2.50f

typedef struct GlobalMotionState {
    int        noEntCollisions;
    int        noDynamicCollisions;
    int        noDetailCollisions; // Used for architects of bases
    
    #if CLIENT
        int    doNotSetAnimBits;
    #endif
} GlobalMotionState;

extern GlobalMotionState global_motion_state;

typedef struct SurfaceParams
{
    F32     traction;
    F32     friction;
    F32        bounce;
    F32        gravity;
    F32        max_speed;
} SurfaceParams;

typedef enum SurfParamIndex
{
    SURFPARAM_GROUND,
    SURFPARAM_AIR,
    SURFPARAM_MAX
} SurfParamIndex;

typedef struct
{
    F32        length;
    F32        radius;
    int        dir;
    int        has_offset;
    Vec3    offset;
    F32        max_coll_dist;
} EntityCapsule;

typedef enum
{
    STUCK_SLIDE = 1,
    STUCK_COMPLETELY,
} StuckType;

typedef enum LastSurfType
{
    SURFTYPE_WALL,
    SURFTYPE_GROUND,
} LastSurfType;

/****************************************************************************************************
*
*  IMPORTANT NOTE: MotionState **MUST** be the same size on the client and the server.
*                  This is for debugging purposes.
*
****************************************************************************************************/

typedef struct MotionStateInput {
    Vec3                vel;            // Only valid for last frame. want to go this way

    F32                    max_speed_scale;
    F32                    max_jump_height;

    SurfaceParams        surf_mods[SURFPARAM_MAX];

    U32                    flying            : 1;// am I flying?
    U32                    no_slide_physics: 1;// so monsters have a chance to navigate the terrain
    U32                    no_ent_collision: 1;
    U32                    stunned            : 1;
    U32                    jump_released    : 1;
    U32                    web_swing_enabled: 1;
} MotionStateInput;

typedef struct MotionState
{
    Vec3                vel;                // currently going this way
    Vec3                last_pos;           // where it was last frame
    U32                 falling            : 1;// am I falling?
    U32                 bigfall            : 1;// am I falling?
    U32                    jumping            : 1;
    U32                    jump_held        : 1;// am I holding the jump button?
    U32                    jump_still_held    : 1;// was jump released yet?
    U32                    jump_not_landed    : 1;// have I landed since initiating the jump?
    U32                    sliding            : 3;// am I sliding right now? plus a time of sorts for how long you've been in hte air
    U32                    bouncing        : 1;// am I colliding with an entity that bounces instead of rigidly collides?
    U32                    on_poly_edge    : 1;// standing on edge (rather than face) of poly, so force slope to be flat
    U32                    on_surf            : 1;// on a surface
    U32                    was_on_surf        : 1;
    U32                    hit_stumble     : 1;//am in hit stumble mode
    U32                    hit_stumble_kill_velocity_on_impact     : 1;//as soon as you hit the ground, kill impact

    StuckType            stuck;              // able to move last frame?
    StuckType            stuck_head;         // able to move up or down last frame?
    F32                 move_time;          // timer used for nocoll motion

    Vec3                surf_normal;        // vector pointing normal to the ground you're standing on. (0,1,0) means flat ground
    F32                    jump_height;
    F32                    highest_height;
    F32                    heightILastTouchedTheGround;
    int                    jump_time;            // last time ent jumped
    int                    lastSurfFlags;        // Triangle this guy was last known to be on.  FOr friction and traction and bounciness
    int                    lastGroundSurfFlags;// Flags before leaving the ground.
    Vec3                addedVel;            // for rubbery entity collisions (could be a local in movement code, is only used in a single frame
    U32                    tickCounter;        //how many physics ticks have been run on this guy
    U32                    lastEntCollidedWith; //svr_idx of the last entity you collided with
    U32                    tickOfLastEntCollision; //when you hit that entity (from tickCounter)
    LastSurfType        last_surf_type;
    Vec3                last_surf_normal;
    Vec3                prev_surf_normal;
    S32                    low_traction_steps_remaining;    // # of phys steps to force low traction.
    F32                    hitStumbleTractionLoss;

    U32                    web_swing_attached : 1;
    U32                    web_swing_attach_catch_pending : 1;
    U32                    web_swing_attach_grounded : 1;
    U32                    web_swing_attach_falling : 1;
    U32                    web_swing_attach_jumping : 1;
    U32                    web_swing_diag_latched : 1;
    U32                    web_swing_state_diag_latched : 1;
    U32                    web_swing_chain_armed : 1;
    U32                    web_swing_chain_ascent_seen : 1;
    U32                    web_swing_next_anchor_valid : 1;
    U32                    web_swing_ground_launch_active : 1;
    U32                    web_swing_anim_phase;
    U32                    web_swing_anim_segment_id;
    U32                    web_swing_anim_phase_segment_id;
    Vec3                   web_swing_anchor;
    Vec3                   web_swing_previous_anchor;
    Vec3                   web_swing_next_anchor;
    F32                    web_swing_next_anchor_retention;
    U32                    web_swing_chain_search_tick;
    F32                    web_swing_ground_launch_origin_y;
    U32                    web_swing_ground_launch_ticks;
    F32                    web_swing_rope_length;
    U32                    web_swing_log_tick;
    U32                    web_swing_constraint_samples;
    U32                    web_swing_constraint_soft_correction_count;
    U32                    web_swing_constraint_correction_count;
    F32                    web_swing_constraint_error_sum;
    F32                    web_swing_constraint_max_error;
    F32                    web_swing_constraint_correction_sum;
    F32                    web_swing_constraint_max_correction;
    F32                    web_swing_constraint_max_velocity_dir_delta;
    F32                    web_swing_constraint_velocity_dir_delta_sum;
    U32                    web_swing_constraint_velocity_dir_delta_large_count;
    U32                    web_swing_constraint_velocity_dir_delta_large_run;
    U32                    web_swing_constraint_max_velocity_dir_delta_large_run;
    U32                    web_swing_constraint_radial_velocity_removed_count;
    U32                    web_swing_constraint_radial_velocity_large_count;
    F32                    web_swing_constraint_radial_velocity_removed_sum;
    F32                    web_swing_constraint_max_radial_velocity_removed;

    EntityCapsule        capsule;

    MotionStateInput    input;
} MotionState;

// Generated by mkproto
MotionState* createMotionState();
void destroyMotionState(MotionState* motion);
void getCapsule( SeqInst * seq, EntityCapsule * cap  );
void positionCapsule( const Mat4 entMat, EntityCapsule * cap, Mat4 collMat );
void setEntCollTimes(int enabled, U32 client_abs, U32 client_abs_slow);
int checkEntColl(Entity *movingEnt, int doApplyBounce, const Mat3 control_mat);
void entMotionFreeColl(Entity *e);
void entMotionUpdateCollGrid(Entity *e);
void entMotionDrawDebugCollisionTrackers();
void entMotionCheckDoorOccluders();
void entMotionClearAllWorldGroupPtr();
void entMotionFreeCollAll();
int velIsZero(Vec3 vel);
int entCanSetPYR( Entity *e );
int entCanSetInpVel( Entity *e );
void entMoveNoColl(Entity *e);
void entMotion(Entity* e, const Mat3 control_mat, int web_swing_test_no_attach);
bool isEntCurrentlyCollidable( Entity * e );
void setNoCollideOnTracker( DefTracker * tracker, int set );
// End mkproto
#endif
