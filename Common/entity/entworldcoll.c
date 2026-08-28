#include "entworldcoll.h"
#include <float.h>
#include <string.h>
#include <utilitieslib/utils/mathutil.h>
#include <utilitieslib/utils/error.h>
#include "entity/entity.h"
#include "gridcoll/gridcoll.h"
#include "cmdparse/cmdcommon.h"
#include "varutils.h"
#include "entity/motion.h"
#include "group/groupscene.h"
#include "group/group.h"
#include <utilitieslib/utils/utils.h>
#include <utilitieslib/utils/timing.h>
#ifdef SERVER
#include "entity/entGameActions.h"
#include "entity/entsend.h"
#else
#include "graphics/font.h"
#include "render/renderprim.h"
#endif
#define DEFAULT_RADIUS 1.0f

#define WEB_MAX_ANCHOR_DIST    150.0f
#define WEB_MIN_ROPE_LENGTH    8.0f
#define WEB_MAX_ROPE_LENGTH    150.0f
#define WEB_ANCHOR_START_HEIGHT 2.0f
#define WEB_FRESH_GROUND_MIN_ANCHOR_HEIGHT 15.0f
#define WEB_FRESH_GROUND_MIN_VERTICALITY    0.45f
#define WEB_FRESH_GROUND_MIN_ALIGNMENT      0.10f
#define WEB_FRESH_AIR_MIN_ANCHOR_HEIGHT      6.0f
#define WEB_FRESH_AIR_MIN_VERTICALITY        0.10f
#define WEB_FRESH_AIR_MIN_ALIGNMENT          0.00f
#define WEB_CHAIN_MIN_ANCHOR_HEIGHT         12.0f
#define WEB_CHAIN_MIN_VERTICALITY            0.20f
#define WEB_CHAIN_MIN_ALIGNMENT              0.25f
#define WEB_CHAIN_MAX_LATERAL_ALIGNMENT      0.80f
#define WEB_CHAIN_MIN_VELOCITY_RETENTION     0.70f
#define WEB_CHAIN_RETENTION_MIN_SPEED        0.75f
#define WEB_ROPE_SLOP          0.75f
#define WEB_ROPE_BIAS_GAIN     0.35f
#define WEB_ROPE_BIAS_MAX_SPEED 0.75f
#define WEB_SWING_DIRECTION_DELTA_THRESHOLD 0.30f
#define WEB_SWING_RADIAL_VELOCITY_THRESHOLD 0.25f
#define WEB_PENDULUM_ACCEL_DESCENT 0.095f
#define WEB_PENDULUM_ACCEL_ASCENT  0.045f
#define WEB_PENDULUM_ACCEL_APEX    0.015f
#define WEB_FORWARD_TANGENT_ACCEL  0.025f
#define WEB_STEER_ACCEL            0.040f
#define WEB_ATTACH_UPWARD_TARGET_SPEED  1.00f
#define WEB_ATTACH_UPWARD_MAX_DELTA     1.25f
#define WEB_ATTACH_FORWARD_TARGET_SPEED 0.75f
#define WEB_ATTACH_FORWARD_MAX_DELTA    0.75f
#define WEB_INTENT_INPUT_THRESHOLD      0.05f
#define WEB_INTENT_MOMENTUM_MIN_SPEED   0.25f
#define WEB_MAX_SPEED              4.50f
#define WEB_CHAIN_ARM_ARC_HEIGHT        0.35f
#define WEB_CHAIN_ARM_DOWN_SPEED       -0.10f
#define WEB_CHAIN_RELEASE_UP_SPEED      0.18f
#define WEB_CHAIN_MIN_FORWARD_SPEED     0.15f
#define WEB_CHAIN_LATE_RELEASE_ARC_HEIGHT 0.72f
#define WEB_CHAIN_APEX_UP_SPEED           0.02f
#define WEB_CHAIN_PREVIOUS_ANCHOR_EXCLUSION 10.0f
#define WEB_CHAIN_CURRENT_ANCHOR_EXCLUSION  10.0f
#define WEB_ANCHOR_VERTICALITY_WEIGHT   12.0f
#define WEB_ANCHOR_LATERAL_PENALTY      18.0f
#define WEB_CHAIN_RETENTION_WEIGHT       45.0f
#define WEB_LATERAL_REPULSION_ACCEL      0.035f
#define WEB_LATERAL_REPULSION_MIN_DIST   6.0f
#define WEB_LATERAL_REPULSION_FULL_DIST 24.0f
#define WEB_GROUND_LAUNCH_CLEARANCE       5.0f
#define WEB_GROUND_LAUNCH_TARGET_SPEED    2.40f
#define WEB_GROUND_LAUNCH_ACCEL           0.24f
#define WEB_GROUND_LAUNCH_MAX_TICKS       90
#define WEB_SKY_ANCHOR_HEIGHT              45.0f
#define WEB_SKY_ANCHOR_FORWARD_LEAD        28.0f
#define WEB_ASSIST_VISUAL_ANCHOR_BLEND       0.35f
#define WEB_ASSIST_LAUNCH_UP_SPEED            3.20f
#define WEB_ASSIST_LAUNCH_FORWARD_SPEED       2.20f
#define WEB_ASSIST_LAUNCH_ACCEL               0.60f
#define WEB_ASSIST_ASCEND_GRAVITY_ASSIST       0.012f
#define WEB_ASSIST_ASCEND_ENERGY_ASSIST        0.006f
#define WEB_ASSIST_APEX_DOWN_ACCEL            0.020f
#define WEB_ASSIST_DESCEND_DOWN_ACCEL         0.040f
#define WEB_ASSIST_DESCEND_ENERGY_ACCEL       0.008f
#define WEB_ASSIST_SWOOP_DEFAULT_RADIUS       105.0f
#define WEB_ASSIST_SWOOP_MIN_RADIUS            24.0f
#define WEB_ASSIST_SWOOP_EXIT_ANGLE             RAD(36.0f)
#define WEB_ASSIST_SWOOP_MAX_ANGULAR_STEP       RAD(15.0f)
#define WEB_ASSIST_SWOOP_MIN_PLANE_SPEED         0.25f
#define WEB_ASSIST_SWOOP_HORIZONTAL_BLEND_START  0.12f
#define WEB_ASSIST_SWOOP_HORIZONTAL_BLEND_FULL   0.35f
#define WEB_ASSIST_SWOOP_EMERGENCY_BASE_ACCEL    0.36f
#define WEB_ASSIST_SWOOP_EMERGENCY_MAX_ACCEL     0.90f
#define WEB_ASSIST_SWOOP_EMERGENCY_MARGIN        0.50f
#define WEB_ASSIST_ALTITUDE_ARC_DEPTH          36.0f
#define WEB_ASSIST_ALTITUDE_TRANSITION_PADDING 2.0f
#define WEB_ASSIST_ALTITUDE_FLOOR_RISE_BASE    1.5f
#define WEB_ASSIST_ALTITUDE_FLOOR_RISE_ENERGY  1.0f
#define WEB_ASSIST_ALTITUDE_MAX_GAIN           24.0f
#define WEB_ASSIST_ALTITUDE_STOP_ACCEL          0.90f
#define WEB_ASSIST_GROUND_PROBE_DISTANCE      32.0f
#define WEB_ASSIST_GROUND_TARGET_CLEARANCE     6.0f
#define WEB_ASSIST_GROUND_TRANSITION_PADDING   3.0f
#define WEB_ASSIST_GROUND_STOP_ACCEL            0.40f
#define WEB_ASSIST_GROUND_LOOKAHEAD_BASE       10.0f
#define WEB_ASSIST_GROUND_LOOKAHEAD_SPEED       3.0f
#define WEB_ASSIST_GROUND_LOOKAHEAD_MAX        24.0f
#define WEB_ASSIST_VISUAL_TETHER_MIN_GAP_TICKS 18
#define WEB_ASSIST_VISUAL_TETHER_RETRACT_RATE   0.25f
#define WEB_ASSIST_VISUAL_TETHER_SHOOT_WINDUP_TIME 12.0f
#define WEB_ASSIST_VISUAL_TETHER_REPEAT_WINDUP_TIME 0.0f
#define WEB_ASSIST_VISUAL_TETHER_EXTEND_RATE    0.25f
#define WEB_ASSIST_ANIM_CATCH_TICKS             12
#define WEB_ASSIST_ANIM_RELEASE_TICKS           18
#define WEB_ASSIST_HORIZONTAL_MIN_SPEED        1.65f
#define WEB_ASSIST_ASCEND_SPEED                2.50f
#define WEB_ASSIST_APEX_SPEED                  2.00f
#define WEB_ASSIST_DESCEND_SPEED               4.00f
#define WEB_ASSIST_BOTTOM_SPEED                5.20f
#define WEB_ASSIST_ASCEND_ENERGY_SPEED         0.30f
#define WEB_ASSIST_APEX_ENERGY_SPEED           0.35f
#define WEB_ASSIST_DESCEND_ENERGY_SPEED        0.85f
#define WEB_ASSIST_BOTTOM_ENERGY_SPEED         1.00f
#define WEB_ASSIST_HORIZONTAL_MAX_SPEED        6.50f
#define WEB_ASSIST_TOTAL_MAX_SPEED             8.00f
#define WEB_ASSIST_ENERGY_PER_CYCLE            0.28f
#define WEB_ASSIST_MAX_ENERGY                  2.00f
#define WEB_ASSIST_REATTACH_ENERGY_PER_SPEED   0.40f
#define WEB_ASSIST_STEER_ASCEND                 0.070f
#define WEB_ASSIST_STEER_APEX                   0.140f
#define WEB_ASSIST_STEER_DESCEND                0.090f
#define WEB_ASSIST_STEER_BOTTOM                 0.060f

#if SERVER
#define WEB_SWING_LOG_SIDE "SERVER"
#else
#define WEB_SWING_LOG_SIDE "CLIENT"
#endif

// Prototype vertical wall crawl canary. Keep this block isolated from the
// established Web Swing controller constants and functions.
#define WEB_CRAWL_MAX_NORMAL_Y                 0.30f
#define WEB_CRAWL_ATTACH_INPUT_MIN             0.20f
#define WEB_CRAWL_SPEED                        1.00f
#define WEB_CRAWL_ADHESION_SPEED               0.12f
#define WEB_CRAWL_JUMP_OUT_SPEED               1.10f
#define WEB_CRAWL_REATTACH_COOLDOWN_TICKS      10
#define WEB_CRAWL_CONTACT_GRACE_TICKS           2
#define WEB_CRAWL_SAME_WALL_DOT                 0.90f
#define WEB_CRAWL_GROUND_NORMAL_Y               0.70f

#if SERVER
#define WEB_CRAWL_LOG_SIDE "SERVER"
#else
#define WEB_CRAWL_LOG_SIDE "CLIENT"
#endif

int coll_is_player; //Unused
int landed_on_ground; //Anytime in the last DoPhysics did you hit the ground? (if so, do done fall)

#define MDBG 0 && CLIENT

static CollInfo    last_surf;

typedef enum WebSwingIntentSource
{
    WEB_SWING_INTENT_NONE = 0,
    WEB_SWING_INTENT_INPUT,
    WEB_SWING_INTENT_FACING,
} WebSwingIntentSource;

typedef enum WebSwingAnchorContext
{
    WEB_SWING_ANCHOR_FRESH_GROUND = 0,
    WEB_SWING_ANCHOR_FRESH_AIR,
    WEB_SWING_ANCHOR_CHAIN_NEXT,
} WebSwingAnchorContext;

typedef enum WebSwingBackend
{
    WEB_SWING_BACKEND_REAL_ANCHOR = 0,
    WEB_SWING_BACKEND_SKY_ASSISTED = 1,
} WebSwingBackend;

static const char *webSwingBackendName(int backend)
{
    return backend == WEB_SWING_BACKEND_SKY_ASSISTED ? "SKY_ASSISTED" : "REAL_ANCHOR";
}

static const char *webSwingAssistPhaseName(WebSwingAssistPhase phase)
{
    switch(phase)
    {
        case WEB_SWING_ASSIST_LAUNCH:  return "LAUNCH";
        case WEB_SWING_ASSIST_ASCEND:  return "ASCEND";
        case WEB_SWING_ASSIST_APEX:    return "APEX";
        case WEB_SWING_ASSIST_DESCEND: return "DESCEND";
        case WEB_SWING_ASSIST_BOTTOM:  return "BOTTOM";
        default:                       return "NONE";
    }
}

static void webSwingLogReleaseState(Entity *e, const char *stage)
{
    MotionState *motion = e->motion;
    F32 horizontal_speed = sqrt(SQR(motion->vel[0]) + SQR(motion->vel[2]));
    F32 pre_horizontal_speed = sqrt(SQR(motion->web_swing_release_pre_velocity[0]) +
                                    SQR(motion->web_swing_release_pre_velocity[2]));

    filelog_printf("webswing.log",
                   "WEB_SWING %s release_state stage=%s tick=%u pos=(%.3f %.3f %.3f) velocity=(%.6f %.6f %.6f) horizontal_speed=%.6f total_speed=%.6f horizontal_retention=%.6f falling=%d on_surf=%d was_on_surf=%d jumping=%d jump_held=%d jump_still_held=%d last_surf_type=%d last_surf_flags=%d last_ground_surf_flags=%d surf_normal=(%.3f %.3f %.3f) last_surf_normal=(%.3f %.3f %.3f)\n",
                   WEB_SWING_LOG_SIDE,
                   stage,
                   motion->tickCounter,
                   vecParamsXYZ(ENTPOS(e)),
                   vecParamsXYZ(motion->vel),
                   horizontal_speed,
                   lengthVec3(motion->vel),
                   pre_horizontal_speed > 0.0001f ? horizontal_speed / pre_horizontal_speed : 1.0f,
                   motion->falling,
                   motion->on_surf,
                   motion->was_on_surf,
                   motion->jumping,
                   motion->jump_held,
                   motion->jump_still_held,
                   motion->last_surf_type,
                   motion->lastSurfFlags,
                   motion->lastGroundSurfFlags,
                   vecParamsXYZ(motion->surf_normal),
                   vecParamsXYZ(motion->last_surf_normal));
}

void entWorldWebSwingLogReleasePostWorld(Entity *e)
{
    MotionState *motion = e->motion;

    if(motion->web_swing_release_diag_stage != 1 ||
       motion->web_swing_release_diag_tick != motion->tickCounter)
        return;

    webSwingLogReleaseState(e, "RELEASE_POST_WORLD");
    motion->web_swing_release_diag_stage = 2;
}

static const char *webSwingAnchorContextName(WebSwingAnchorContext context)
{
    switch(context)
    {
        case WEB_SWING_ANCHOR_FRESH_GROUND: return "FRESH_GROUND";
        case WEB_SWING_ANCHOR_CHAIN_NEXT:   return "CHAIN_NEXT";
        default:                            return "FRESH_AIR";
    }
}

static const char *webSwingIntentSourceName(WebSwingIntentSource source)
{
    switch(source)
    {
        case WEB_SWING_INTENT_INPUT:
            return "INPUT";
        case WEB_SWING_INTENT_FACING:
            return "FACING";
        default:
            return "NONE";
    }
}

/*
 * MotionStateInput.vel is world-space by the time entMotion is called.  The
 * client builds it from control-local input with control_mat in
 * playerRunPhysicsStep(); the server's pmotionSetVel path writes the
 * authoritative world movement vector directly.  Keep this helper as the
 * single definition of horizontal travel intent for Web Swing.
 */
static WebSwingIntentSource webSwingGetTravelIntent(Entity *e, Vec3 intent, F32 *horizontal_input_magnitude)
{
    Vec3 input;
    F32 input_magnitude;

    copyVec3(e->motion->input.vel, input);
    input[1] = 0.0f;
    input_magnitude = lengthVec3(input);
    if(horizontal_input_magnitude)
        *horizontal_input_magnitude = input_magnitude;

    if(input_magnitude > WEB_INTENT_INPUT_THRESHOLD)
    {
        scaleVec3(input, 1.0f / input_magnitude, intent);
        return WEB_SWING_INTENT_INPUT;
    }

    copyVec3(ENTMAT(e)[2], intent);
    intent[1] = 0.0f;
    if(normalVec3(intent))
        return WEB_SWING_INTENT_FACING;

    zeroVec3(intent);
    return WEB_SWING_INTENT_NONE;
}

static int webSwingGetMeaningfulMomentum(Entity *e, Vec3 momentum, F32 *horizontal_speed)
{
    F32 speed;

    copyVec3(e->motion->vel, momentum);
    momentum[1] = 0.0f;
    speed = lengthVec3(momentum);
    if(horizontal_speed)
        *horizontal_speed = speed;
    if(speed <= WEB_INTENT_MOMENTUM_MIN_SPEED)
    {
        zeroVec3(momentum);
        return 0;
    }

    scaleVec3(momentum, 1.0f / speed, momentum);
    return 1;
}

static void printColls(CollInfo* coll,U32 flags)
{
    int        i;

    if (coll->coll_count)
        printf("\n%dclosest   %d\t%f %f %f   %f\n",flags ? 1 : 0,coll->tri_idx,vecParamsXYZ(coll->mat[3]),coll->mat[1][1]);
    for(i=0;i<coll->coll_count;i++)
    {
        F32        d = distance3(coll->mat[3],coll->tri_colls[i].mat[3]);
        F32        d2 = fabs(coll->mat[3][1] - coll->tri_colls[i].mat[3][1]);
        int j;

        printf(    "tri %3d:\t%f %f %f    %f %f   %f\n",
                coll->tri_colls[i].tri_idx,
                vecParamsXYZ(coll->tri_colls[i].mat[3]),
                d,
                d2,
                coll->tri_colls[i].mat[1][1]);

        for(j = 0; j < 3; j++)
        {
            printf("   vert %d:\t%f\t%f\t%f\n", j, vecParamsXYZ(coll->tri_colls[i].verts[j]));
        }
    }
}

static int BaseEditCollCallback(void *nd, int facing)
{
    DefTracker *tracker = nd;

    if(strstr(tracker->def->name, "_Wall_") || strstr(tracker->def->name, "_Ceiling_") 
        || strstr(tracker->def->name, "_Floor_") || strnicmp(tracker->def->name, "grp", 3)==0)
         return 1;
    else
        return 0;
}

static int collide(Vec3 start,Vec3 end,CollInfo *coll,F32 rad,U32 flags)
{
    extern int dump_grid_coll_info;

    int        ret;

    //coll->tri_colls = 0;
    //coll->coll_count = 0;
    //coll->coll_max = 0;
    //flags |= COLL_GATHERTRIS;

    flags |= COLL_NOTSELECTABLE; // wire fences
    //if (!coll_is_player)
    //    flags |= COLL_ENTBLOCKER;

    //if(dump_grid_coll_info)
    //{
    //    printf(    "start:\t(%1.8f,%1.8f,%1.8f)\n"
    //            "end:\t(%1.8f,%1.8f,%1.8f)\n"
    //            "flags: %d, radius: %1.8f\n",
    //            vecParamsXYZ(start),
    //            vecParamsXYZ(end),
    //            flags,
    //            rad);
    //}

    if(global_motion_state.noDetailCollisions)
    {
        coll->node_callback = BaseEditCollCallback;
        ret = collGrid(0,start,end,coll,rad,flags|COLL_NODECALLBACK);
    }
    else
    {
        ret = collGrid(0,start,end,coll,rad,flags);
    }

    //if(dump_grid_coll_info)
    //{
    //    printColls(coll,flags);
    //}

    return ret;
}

typedef struct WebSwingAnchorSearchStats
{
    WebSwingAnchorContext context;
    int probe_count;
    int collision_ray_hits;
    int height_rejects;
    int distance_rejects;
    int verticality_rejects;
    int alignment_rejects;
    int lateral_rejects;
    int retention_rejects;
    int current_anchor_rejects;
    int selected;
    int used_fallback;
    WebSwingIntentSource intent_source;
    F32 horizontal_input_magnitude;
    int meaningful_momentum;
    int momentum_basis;
    F32 facing_travel_dot;
    Vec3 intent;
    Vec3 momentum;
    Vec3 travel;
    Vec3 travel_right;
    Vec3 entity_right;
    F32 intent_momentum_alignment;
    F32 selected_intent_alignment;
    F32 selected_momentum_alignment;
    F32 selected_forward_alignment;
    F32 selected_verticality;
    F32 selected_lateral_alignment;
    F32 selected_velocity_retention;
    F32 selected_radial_speed;
    F32 incoming_speed;
    int previous_anchor_rejects;
    Vec3 selected_anchor;
    F32 selected_rope_length;
} WebSwingAnchorSearchStats;

static int webSwingFindAnchor(Entity *e, WebSwingAnchorContext context, Vec3 anchor,
                              WebSwingAnchorSearchStats *stats)
{
    typedef struct WebSwingProbe
    {
        F32 forward;
        F32 up;
        F32 side;
    } WebSwingProbe;
    static const WebSwingProbe probes[] =
    {
        { 1.00f, 0.20f,  0.00f },
        { 0.95f, 0.40f,  0.45f },
        { 0.95f, 0.40f, -0.45f },
        { 0.85f, 0.70f,  0.85f },
        { 0.85f, 0.70f, -0.85f },
        { 0.70f, 1.00f,  1.20f },
        { 0.70f, 1.00f, -1.20f },
        { 0.35f, 1.25f,  1.50f },
        { 0.35f, 1.25f, -1.50f },
        { 0.00f, 1.40f,  1.70f },
        { 0.00f, 1.40f, -1.70f },
        {-0.25f, 1.20f,  1.60f },
        {-0.25f, 1.20f, -1.60f },
        {-0.25f, 0.75f,  1.40f },
        {-0.25f, 0.75f, -1.40f },
        { 0.00f, 0.00f,  2.00f },
        { 0.00f, 0.00f, -2.00f },
        {-0.35f, 0.00f,  1.80f },
        {-0.35f, 0.00f, -1.80f },
        {-0.55f, 0.35f,  1.70f },
        {-0.55f, 0.35f, -1.70f },
    };
    Vec3 start;
    Vec3 forward;
    Vec3 entity_right;
    Vec3 travel_right;
    Vec3 momentum;
    Vec3 intent;
    Vec3 up = {0.0f, 1.0f, 0.0f};
    Vec3 best_anchor = {0};
    F32 best_score = -1e30f;
    WebSwingIntentSource intent_source;
    F32 horizontal_input_magnitude = 0.0f;
    int has_momentum;
    int i;

    memset(stats, 0, sizeof(*stats));
    stats->context = context;
    stats->incoming_speed = lengthVec3(e->motion->vel);

    copyVec3(ENTPOS(e), start);
    start[1] += WEB_ANCHOR_START_HEIGHT;
    copyVec3(ENTMAT(e)[2], forward);
    forward[1] = 0.0f;
    if(!normalVec3(forward))
        return 0;
    copyVec3(ENTMAT(e)[0], entity_right);
    entity_right[1] = 0.0f;
    if(!normalVec3(entity_right))
        copyVec3(ENTMAT(e)[0], entity_right);
    intent_source = webSwingGetTravelIntent(e, intent, &horizontal_input_magnitude);
    if(intent_source == WEB_SWING_INTENT_NONE)
        return 0;
    has_momentum = webSwingGetMeaningfulMomentum(e, momentum, NULL);

    // The lateral fan is always perpendicular to player travel intent.  The
    // entity-right vector only keeps the established left/right convention
    // stable; it does not shape the fan around stale momentum.
    crossVec3(up, intent, travel_right);
    if(!normalVec3(travel_right))
        return 0;
    if(dotVec3(travel_right, entity_right) < 0.0f)
        scaleVec3(travel_right, -1.0f, travel_right);

    stats->intent_source = intent_source;
    stats->horizontal_input_magnitude = horizontal_input_magnitude;
    stats->meaningful_momentum = has_momentum;
    stats->momentum_basis = has_momentum;
    stats->facing_travel_dot = dotVec3(forward, intent);
    stats->intent_momentum_alignment = has_momentum ? dotVec3(intent, momentum) : 0.0f;
    copyVec3(intent, stats->intent);
    copyVec3(momentum, stats->momentum);
    copyVec3(intent, stats->travel);
    copyVec3(travel_right, stats->travel_right);
    copyVec3(entity_right, stats->entity_right);

    stats->probe_count = ARRAY_SIZE(probes);
    for(i = 0; i < ARRAY_SIZE(probes); ++i)
    {
        Vec3 direction;
        Vec3 end;
        Vec3 delta;
        Vec3 candidate_direction;
        CollInfo coll = {0};
        F32 distance;
        F32 height;
        F32 travel_alignment;
        F32 forward_alignment;
        F32 momentum_alignment = 0.0f;
        F32 verticality;
        F32 lateral_alignment;
        F32 velocity_retention = 1.0f;
        F32 radial_speed = 0.0f;
        F32 incoming_speed = lengthVec3(e->motion->vel);
        F32 min_height;
        F32 min_verticality;
        F32 min_alignment;
        F32 score;

        scaleVec3(intent, probes[i].forward, direction);
        scaleVec3(up, probes[i].up, delta);
        addVec3(direction, delta, direction);
        scaleVec3(travel_right, probes[i].side, delta);
        addVec3(direction, delta, direction);
        if(!normalVec3(direction))
            continue;

        scaleVec3(direction, WEB_MAX_ANCHOR_DIST, delta);
        addVec3(start, delta, end);

        if(!collide(start, end, &coll, 0.0f, COLL_DISTFROMSTART | COLL_BOTHSIDES))
            continue;

        ++stats->collision_ray_hits;
        copyVec3(coll.mat[3], delta);
        subVec3(delta, ENTPOS(e), candidate_direction);
        distance = normalVec3(candidate_direction);
        if(context == WEB_SWING_ANCHOR_CHAIN_NEXT &&
           distance3(coll.mat[3], e->motion->web_swing_anchor) < WEB_CHAIN_CURRENT_ANCHOR_EXCLUSION)
        {
            ++stats->current_anchor_rejects;
            continue;
        }
        if(context != WEB_SWING_ANCHOR_FRESH_GROUND &&
           lengthVec3Squared(e->motion->web_swing_previous_anchor) > 0.001f &&
           distance3(coll.mat[3], e->motion->web_swing_previous_anchor) < WEB_CHAIN_PREVIOUS_ANCHOR_EXCLUSION)
        {
            ++stats->previous_anchor_rejects;
            continue;
        }

        height = coll.mat[3][1] - ENTPOSY(e);
        travel_alignment = dotVec3(candidate_direction, intent);
        forward_alignment = dotVec3(candidate_direction, forward);
        verticality = MAX(0.0f, candidate_direction[1]);
        lateral_alignment = fabs(dotVec3(candidate_direction, travel_right));

        if(context == WEB_SWING_ANCHOR_FRESH_GROUND)
        {
            min_height = WEB_FRESH_GROUND_MIN_ANCHOR_HEIGHT;
            min_verticality = WEB_FRESH_GROUND_MIN_VERTICALITY;
            min_alignment = WEB_FRESH_GROUND_MIN_ALIGNMENT;
        }
        else if(context == WEB_SWING_ANCHOR_CHAIN_NEXT)
        {
            Vec3 rope_direction;
            Vec3 radial_velocity;
            Vec3 tangent_velocity;

            min_height = WEB_CHAIN_MIN_ANCHOR_HEIGHT;
            min_verticality = WEB_CHAIN_MIN_VERTICALITY;
            min_alignment = WEB_CHAIN_MIN_ALIGNMENT;
            subVec3(ENTPOS(e), coll.mat[3], rope_direction);
            if(normalVec3(rope_direction) && incoming_speed > 0.0001f)
            {
                radial_speed = dotVec3(e->motion->vel, rope_direction);
                scaleVec3(rope_direction, radial_speed, radial_velocity);
                subVec3(e->motion->vel, radial_velocity, tangent_velocity);
                velocity_retention = lengthVec3(tangent_velocity) / incoming_speed;
            }
        }
        else
        {
            min_height = WEB_FRESH_AIR_MIN_ANCHOR_HEIGHT;
            min_verticality = WEB_FRESH_AIR_MIN_VERTICALITY;
            min_alignment = WEB_FRESH_AIR_MIN_ALIGNMENT;
        }

        if(distance < WEB_MIN_ROPE_LENGTH)
            ++stats->distance_rejects;
        if(height < min_height)
            ++stats->height_rejects;
        if(verticality < min_verticality)
            ++stats->verticality_rejects;
        if(travel_alignment < min_alignment)
            ++stats->alignment_rejects;
        if(context == WEB_SWING_ANCHOR_CHAIN_NEXT &&
           lateral_alignment > WEB_CHAIN_MAX_LATERAL_ALIGNMENT)
            ++stats->lateral_rejects;
        if(context == WEB_SWING_ANCHOR_CHAIN_NEXT &&
           incoming_speed >= WEB_CHAIN_RETENTION_MIN_SPEED &&
           velocity_retention < WEB_CHAIN_MIN_VELOCITY_RETENTION)
            ++stats->retention_rejects;
        if(distance < WEB_MIN_ROPE_LENGTH || height < min_height ||
           verticality < min_verticality || travel_alignment < min_alignment ||
           (context == WEB_SWING_ANCHOR_CHAIN_NEXT &&
            (lateral_alignment > WEB_CHAIN_MAX_LATERAL_ALIGNMENT ||
             (incoming_speed >= WEB_CHAIN_RETENTION_MIN_SPEED &&
              velocity_retention < WEB_CHAIN_MIN_VELOCITY_RETENTION))))
            continue;

        if(has_momentum)
            momentum_alignment = dotVec3(candidate_direction, momentum);

        // Prefer high, intent-aligned real collision geometry while retaining
        // forward/fallback coverage and using meaningful momentum only as a
        // secondary continuity signal.
        score = height * 1.5f + travel_alignment * 30.0f +
                forward_alignment * 5.0f + momentum_alignment * 8.0f +
                verticality * WEB_ANCHOR_VERTICALITY_WEIGHT -
                lateral_alignment * WEB_ANCHOR_LATERAL_PENALTY -
                distance * 0.20f;
        if(context == WEB_SWING_ANCHOR_CHAIN_NEXT)
            score += velocity_retention * WEB_CHAIN_RETENTION_WEIGHT;
        if(score > best_score)
        {
            best_score = score;
            copyVec3(coll.mat[3], best_anchor);
            stats->used_fallback = i >= 5;
            stats->selected_intent_alignment = travel_alignment;
            stats->selected_momentum_alignment = momentum_alignment;
            stats->selected_forward_alignment = forward_alignment;
            stats->selected_verticality = verticality;
            stats->selected_lateral_alignment = lateral_alignment;
            stats->selected_velocity_retention = velocity_retention;
            stats->selected_radial_speed = radial_speed;
            stats->incoming_speed = incoming_speed;
        }
    }

    if(best_score <= -1e29f)
        return 0;

    copyVec3(best_anchor, anchor);
    stats->selected = 1;
    copyVec3(best_anchor, stats->selected_anchor);
    stats->selected_rope_length = distance3(ENTPOS(e), best_anchor);
    return 1;
}

/*
 * SKY_ASSISTED deliberately has no collision-query dependency.  Its anchor is
 * a repeatable point above and ahead of current travel intent, so the shared
 * pendulum solver remains the A/B control while acquisition and handoff no
 * longer depend on a particular map's authored geometry.
 */
static int webSwingFindSkyAssistedAnchor(Entity *e, WebSwingAnchorContext context,
                                         Vec3 anchor, WebSwingAnchorSearchStats *stats)
{
    MotionState *motion = e->motion;
    Vec3 intent;
    Vec3 momentum;
    Vec3 offset;
    Vec3 candidate_direction;
    Vec3 rope_direction;
    Vec3 radial_velocity;
    Vec3 tangent_velocity;
    WebSwingIntentSource intent_source;
    F32 horizontal_input_magnitude = 0.0f;
    F32 incoming_speed = lengthVec3(motion->vel);
    int has_momentum;

    memset(stats, 0, sizeof(*stats));
    stats->context = context;
    stats->incoming_speed = incoming_speed;
    intent_source = webSwingGetTravelIntent(e, intent, &horizontal_input_magnitude);
    if(intent_source == WEB_SWING_INTENT_NONE)
        return 0;

    has_momentum = webSwingGetMeaningfulMomentum(e, momentum, NULL);
    copyVec3(ENTPOS(e), anchor);
    scaleVec3(intent, WEB_SKY_ANCHOR_FORWARD_LEAD, offset);
    addVec3(anchor, offset, anchor);
    anchor[1] += WEB_SKY_ANCHOR_HEIGHT;

    copyVec3(anchor, candidate_direction);
    subVec3(candidate_direction, ENTPOS(e), candidate_direction);
    normalVec3(candidate_direction);

    stats->selected = 1;
    stats->intent_source = intent_source;
    stats->horizontal_input_magnitude = horizontal_input_magnitude;
    stats->meaningful_momentum = has_momentum;
    stats->momentum_basis = has_momentum;
    copyVec3(intent, stats->intent);
    copyVec3(momentum, stats->momentum);
    copyVec3(intent, stats->travel);
    stats->intent_momentum_alignment = has_momentum ? dotVec3(intent, momentum) : 0.0f;
    stats->selected_intent_alignment = dotVec3(candidate_direction, intent);
    stats->selected_momentum_alignment = has_momentum ? dotVec3(candidate_direction, momentum) : 0.0f;
    stats->selected_forward_alignment = stats->selected_intent_alignment;
    stats->selected_verticality = MAX(0.0f, candidate_direction[1]);
    stats->selected_velocity_retention = 1.0f;
    stats->incoming_speed = incoming_speed;

    if(context == WEB_SWING_ANCHOR_CHAIN_NEXT && incoming_speed > 0.0001f)
    {
        subVec3(ENTPOS(e), anchor, rope_direction);
        if(normalVec3(rope_direction))
        {
            stats->selected_radial_speed = dotVec3(motion->vel, rope_direction);
            scaleVec3(rope_direction, stats->selected_radial_speed, radial_velocity);
            subVec3(motion->vel, radial_velocity, tangent_velocity);
            stats->selected_velocity_retention = lengthVec3(tangent_velocity) / incoming_speed;
        }
    }

    copyVec3(anchor, stats->selected_anchor);
    stats->selected_rope_length = distance3(ENTPOS(e), anchor);
    return 1;
}

static void webSwingLogAttachAttempt(Entity *e, const WebSwingAnchorSearchStats *stats)
{
    MotionState *motion = e->motion;
    Vec3 forward;
    int grounded = !motion->falling && !motion->jumping;

    copyVec3(ENTMAT(e)[2], forward);
    filelog_printf("webswing.log",
                   "WEB_SWING %s attach_attempt backend=%s context=%s enabled=%d web_swing_enabled=%d up=%.3f grounded=%d falling=%d jumping=%d pos=(%.2f %.2f %.2f) forward=(%.3f %.3f %.3f) intent_source=%s intent=(%.3f %.3f %.3f) momentum=(%.3f %.3f %.3f) meaningful_momentum=%d momentum_basis=%d intent_momentum_alignment=%.3f probes=%d ray_hits=%d rejects_distance=%d rejects_height=%d rejects_verticality=%d rejects_alignment=%d rejects_lateral=%d rejects_current=%d rejects_previous=%d rejects_retention=%d selected=%d fallback=%d selected_intent_alignment=%.3f selected_momentum_alignment=%.3f alignment=%.3f forward_alignment=%.3f verticality=%.3f lateral=%.3f incoming_speed=%.3f radial=%.3f retention=%.3f min_retention=%.3f anchor=(%.2f %.2f %.2f) rope=%.2f\n",
                   WEB_SWING_LOG_SIDE,
                   webSwingBackendName(motion->input.web_swing_backend),
                   webSwingAnchorContextName(stats->context),
                   motion->input.web_swing_enabled,
                   motion->input.web_swing_enabled,
                   motion->input.vel[1],
                   grounded,
                   motion->falling,
                   motion->jumping,
                   vecParamsXYZ(ENTPOS(e)),
                   vecParamsXYZ(forward),
                   webSwingIntentSourceName(stats->intent_source),
                   vecParamsXYZ(stats->intent),
                   vecParamsXYZ(stats->momentum),
                   stats->meaningful_momentum,
                   stats->momentum_basis,
                   stats->intent_momentum_alignment,
                   stats->probe_count,
                   stats->collision_ray_hits,
                   stats->distance_rejects,
                   stats->height_rejects,
                   stats->verticality_rejects,
                   stats->alignment_rejects,
                   stats->lateral_rejects,
                   stats->current_anchor_rejects,
                   stats->previous_anchor_rejects,
                   stats->retention_rejects,
                   stats->selected,
                   stats->selected ? stats->used_fallback : 0,
                   stats->selected_intent_alignment,
                   stats->selected_momentum_alignment,
                   stats->selected_intent_alignment,
                   stats->selected_forward_alignment,
                   stats->selected_verticality,
                   stats->selected_lateral_alignment,
                   stats->incoming_speed,
                   stats->selected_radial_speed,
                   stats->selected_velocity_retention,
                   WEB_CHAIN_MIN_VELOCITY_RETENTION,
                   vecParamsXYZ(stats->selected_anchor),
                   stats->selected_rope_length);
}

static void webSwingLogChainArm(Entity *e, F32 arc_height, F32 forward_speed)
{
    MotionState *motion = e->motion;

    printf("WEB_SWING %s chain_arm backend=%s anchor=(%.2f %.2f %.2f) pos=(%.2f %.2f %.2f) arc_height=%.3f velocity=(%.3f %.3f %.3f) forward_speed=%.3f\n",
           WEB_SWING_LOG_SIDE,
           webSwingBackendName(motion->web_swing_active_backend),
           vecParamsXYZ(motion->web_swing_anchor),
           vecParamsXYZ(ENTPOS(e)),
           arc_height,
           vecParamsXYZ(motion->vel),
           forward_speed);
    filelog_printf("webswing.log",
                   "WEB_SWING %s chain_arm backend=%s anchor=(%.2f %.2f %.2f) pos=(%.2f %.2f %.2f) arc_height=%.3f velocity=(%.3f %.3f %.3f) forward_speed=%.3f\n",
                   WEB_SWING_LOG_SIDE,
                   webSwingBackendName(motion->web_swing_active_backend),
                   vecParamsXYZ(motion->web_swing_anchor),
                   vecParamsXYZ(ENTPOS(e)),
                   arc_height,
                   vecParamsXYZ(motion->vel),
                   forward_speed);
}

static void webSwingLogChainPending(Entity *e, const WebSwingAnchorSearchStats *stats)
{
    MotionState *motion = e->motion;
    filelog_printf("webswing.log",
                   "WEB_SWING %s next_anchor_locked backend=%s current_anchor=(%.2f %.2f %.2f) next_anchor=(%.2f %.2f %.2f) pos=(%.2f %.2f %.2f) velocity=(%.3f %.3f %.3f) incoming_speed=%.3f radial=%.3f predicted_retention=%.3f min_retention=%.3f\n",
                   WEB_SWING_LOG_SIDE,
                   webSwingBackendName(motion->web_swing_active_backend),
                   vecParamsXYZ(motion->web_swing_anchor),
                   vecParamsXYZ(motion->web_swing_next_anchor),
                   vecParamsXYZ(ENTPOS(e)),
                   vecParamsXYZ(motion->vel),
                   stats->incoming_speed,
                   stats->selected_radial_speed,
                   stats->selected_velocity_retention,
                   WEB_CHAIN_MIN_VELOCITY_RETENTION);
}

static void webSwingLogChainHandoff(Entity *e, const Vec3 previous_anchor,
                                    const Vec3 incoming_velocity, F32 predicted_retention)
{
    MotionState *motion = e->motion;
    F32 anchor_advance = distance3(previous_anchor, motion->web_swing_anchor);

    printf("WEB_SWING %s chain_handoff backend=%s previous_anchor=(%.2f %.2f %.2f) new_anchor=(%.2f %.2f %.2f) anchor_advance=%.2f incoming_velocity=(%.3f %.3f %.3f) outgoing_velocity=(%.3f %.3f %.3f) predicted_retention=%.3f catch_suppressed=1 attached_gap=0\n",
           WEB_SWING_LOG_SIDE,
           webSwingBackendName(motion->web_swing_active_backend),
           vecParamsXYZ(previous_anchor),
           vecParamsXYZ(motion->web_swing_anchor),
           anchor_advance,
           vecParamsXYZ(incoming_velocity),
           vecParamsXYZ(motion->vel),
           predicted_retention);
    filelog_printf("webswing.log",
                   "WEB_SWING %s chain_handoff backend=%s previous_anchor=(%.2f %.2f %.2f) new_anchor=(%.2f %.2f %.2f) anchor_advance=%.2f incoming_velocity=(%.3f %.3f %.3f) outgoing_velocity=(%.3f %.3f %.3f) predicted_retention=%.3f catch_suppressed=1 attached_gap=0 segment_id=%u\n",
                   WEB_SWING_LOG_SIDE,
                   webSwingBackendName(motion->web_swing_active_backend),
                   vecParamsXYZ(previous_anchor),
                   vecParamsXYZ(motion->web_swing_anchor),
                   anchor_advance,
                   vecParamsXYZ(incoming_velocity),
                   vecParamsXYZ(motion->vel),
                   predicted_retention,
                   motion->web_swing_anim_segment_id);
}

static void webSwingResetConstraintMetrics(MotionState *motion)
{
    motion->web_swing_constraint_samples = 0;
    motion->web_swing_constraint_soft_correction_count = 0;
    motion->web_swing_constraint_correction_count = 0;
    motion->web_swing_constraint_error_sum = 0.0f;
    motion->web_swing_constraint_max_error = 0.0f;
    motion->web_swing_constraint_correction_sum = 0.0f;
    motion->web_swing_constraint_max_correction = 0.0f;
    motion->web_swing_constraint_max_velocity_dir_delta = 0.0f;
    motion->web_swing_constraint_velocity_dir_delta_sum = 0.0f;
    motion->web_swing_constraint_velocity_dir_delta_large_count = 0;
    motion->web_swing_constraint_velocity_dir_delta_large_run = 0;
    motion->web_swing_constraint_max_velocity_dir_delta_large_run = 0;
    motion->web_swing_constraint_radial_velocity_removed_count = 0;
    motion->web_swing_constraint_radial_velocity_large_count = 0;
    motion->web_swing_constraint_radial_velocity_removed_sum = 0.0f;
    motion->web_swing_constraint_max_radial_velocity_removed = 0.0f;
}

static void webSwingAssistedSetPhase(Entity *e, WebSwingAssistPhase phase, const char *reason)
{
    MotionState *motion = e->motion;
    WebSwingAssistPhase previous = (WebSwingAssistPhase)motion->web_swing_assist_phase;

    if(previous == phase)
        return;

    motion->web_swing_assist_phase = phase;
    motion->web_swing_assist_phase_ticks = 0;
    motion->web_swing_ground_launch_active = phase == WEB_SWING_ASSIST_LAUNCH;
    filelog_printf("webswing.log",
                   "WEB_SWING %s assisted_phase previous=%s phase=%s reason=%s cycle_id=%u segment_id=%u pos=(%.2f %.2f %.2f) velocity=(%.3f %.3f %.3f) preserve_velocity=1\n",
                   WEB_SWING_LOG_SIDE,
                   webSwingAssistPhaseName(previous),
                   webSwingAssistPhaseName(phase),
                   reason,
                   motion->web_swing_assist_cycle_id,
                   motion->web_swing_anim_segment_id,
                   vecParamsXYZ(ENTPOS(e)),
                   vecParamsXYZ(motion->vel));
}

static void webSwingAssistedBegin(Entity *e, int grounded)
{
    MotionState *motion = e->motion;
    WebSwingAssistPhase initial_phase;
    F32 horizontal_speed = sqrt(SQR(motion->vel[0]) + SQR(motion->vel[2]));

    if(grounded)
        initial_phase = WEB_SWING_ASSIST_LAUNCH;
    else if(motion->vel[1] > 0.20f)
        initial_phase = WEB_SWING_ASSIST_ASCEND;
    else if(motion->vel[1] < -0.20f)
        initial_phase = WEB_SWING_ASSIST_DESCEND;
    else
        initial_phase = WEB_SWING_ASSIST_APEX;

    motion->web_swing_assist_cycle_id = 1;
    motion->web_swing_assist_energy = grounded ? 0.0f :
        MINMAX((horizontal_speed - WEB_ASSIST_HORIZONTAL_MIN_SPEED) *
               WEB_ASSIST_REATTACH_ENERGY_PER_SPEED,
               0.0f, WEB_ASSIST_MAX_ENERGY);
    // SKY_ASSISTED does not use the visible endpoint as a rigid pivot, but
    // the attachment still authors the altitude band for this run.  The
    // fictional pivot and line length define a low point in world space so
    // a rooftop/high-air attachment does not normalize back to street level.
    motion->web_swing_assist_initial_low_point_y =
        motion->web_swing_anchor[1] - motion->web_swing_rope_length -
        WEB_ASSIST_ALTITUDE_ARC_DEPTH;
    motion->web_swing_assist_low_point_y = motion->web_swing_assist_initial_low_point_y;
    motion->web_swing_assist_current_clearance = -1.0f;
    motion->web_swing_assist_ahead_clearance = -1.0f;
    motion->web_swing_assist_lookahead_distance = -1.0f;
    motion->web_swing_assist_altitude_margin = 0.0f;
    motion->web_swing_assist_swoop_active = 0;
    motion->web_swing_assist_swoop_entry_plane_speed = 0.0f;
    motion->web_swing_assist_swoop_entry_angle = 0.0f;
    motion->web_swing_assist_swoop_radius = 0.0f;
    motion->web_swing_assist_swoop_emergency_count = 0;
    motion->web_swing_assist_phase = WEB_SWING_ASSIST_NONE;
    motion->web_swing_visual_tether_visible = 1;
    motion->web_swing_visual_tether_gap_ticks = 0;
    motion->web_swing_visual_tether_shoot_ticks = 0;
    motion->web_swing_visual_tether_shoot_time = 0.0f;
    motion->web_swing_anim_catch_ticks = 0;
    motion->web_swing_anim_release_ticks = 0;
    motion->web_swing_anim_shoot_active = 0;
    motion->web_swing_visual_tether_state = WEB_SWING_VISUAL_TETHER_EXTENDING;
    motion->web_swing_visual_tether_progress = 0.0f;
    filelog_printf("webswing.log",
                   "WEB_SWING %s assisted_altitude_band event=BEGIN attach_y=%.3f anchor_y=%.3f rope=%.3f low_point_y=%.3f preserve_elevation=1\n",
                   WEB_SWING_LOG_SIDE,
                   ENTPOSY(e),
                   motion->web_swing_anchor[1],
                   motion->web_swing_rope_length,
                   motion->web_swing_assist_low_point_y);
    webSwingAssistedSetPhase(e, initial_phase, grounded ? "GROUND_ACTIVATION" : "AIRBORNE_REATTACH");
}

static void webSwingSetAssistedVisualAnchor(Entity *e, const Vec3 travel_intent)
{
    MotionState *motion = e->motion;
    Vec3 lead;
    F32 previous_low_point = motion->web_swing_assist_low_point_y;
    F32 candidate_low_point;
    F32 maximum_rise;
    F32 gain_ceiling;

    copyVec3(ENTPOS(e), motion->web_swing_anchor);
    scaleVec3(travel_intent, WEB_SKY_ANCHOR_FORWARD_LEAD, lead);
    addVec3(motion->web_swing_anchor, lead, motion->web_swing_anchor);
    motion->web_swing_anchor[1] += WEB_SKY_ANCHOR_HEIGHT;
    motion->web_swing_rope_length = distance3(ENTPOS(e), motion->web_swing_anchor);
    candidate_low_point = motion->web_swing_anchor[1] - motion->web_swing_rope_length -
                          WEB_ASSIST_ALTITUDE_ARC_DEPTH;
    maximum_rise = WEB_ASSIST_ALTITUDE_FLOOR_RISE_BASE +
                   motion->web_swing_assist_energy * WEB_ASSIST_ALTITUDE_FLOOR_RISE_ENERGY;
    gain_ceiling = motion->web_swing_assist_initial_low_point_y +
                   WEB_ASSIST_ALTITUDE_MAX_GAIN *
                   MIN(1.0f, motion->web_swing_assist_energy / WEB_ASSIST_MAX_ENERGY);
    motion->web_swing_assist_low_point_y = MAX(previous_low_point,
        MIN(gain_ceiling, MIN(candidate_low_point, previous_low_point + maximum_rise)));
    if(motion->web_swing_assist_low_point_y > previous_low_point + 0.01f)
    {
        filelog_printf("webswing.log",
                       "WEB_SWING %s assisted_altitude_band event=RAISE cycle_id=%u previous_low_point_y=%.3f candidate_low_point_y=%.3f low_point_y=%.3f max_rise=%.3f gain_ceiling_y=%.3f energy=%.3f preserve_elevation=1\n",
                       WEB_SWING_LOG_SIDE,
                       motion->web_swing_assist_cycle_id,
                       previous_low_point,
                       candidate_low_point,
                       motion->web_swing_assist_low_point_y,
                       maximum_rise,
                       gain_ceiling,
                       motion->web_swing_assist_energy);
    }
}

static void webSwingAssistedReleaseVisualTether(Entity *e)
{
    MotionState *motion = e->motion;

    if(motion->web_swing_visual_tether_state != WEB_SWING_VISUAL_TETHER_ATTACHED)
        return;

    motion->web_swing_visual_tether_visible = 1;
    motion->web_swing_visual_tether_state = WEB_SWING_VISUAL_TETHER_RETRACTING;
    motion->web_swing_visual_tether_progress = 1.0f;
    motion->web_swing_visual_tether_gap_ticks = 0;
    motion->web_swing_visual_tether_shoot_ticks = 0;
    motion->web_swing_visual_tether_shoot_time = 0.0f;
    motion->web_swing_anim_catch_ticks = 0;
    filelog_printf("webswing.log",
                   "WEB_SWING %s visual_tether_release cycle_id=%u segment_id=%u phase=APEX anchor=(%.2f %.2f %.2f) retracting=1 physics_continuous=1\n",
                   WEB_SWING_LOG_SIDE,
                   motion->web_swing_assist_cycle_id,
                   motion->web_swing_anim_segment_id,
                   vecParamsXYZ(motion->web_swing_anchor));
}

static void webSwingUpdateAssistedVisualTether(Entity *e, const Vec3 travel_intent)
{
    MotionState *motion = e->motion;
    WebSwingAssistPhase phase = (WebSwingAssistPhase)motion->web_swing_assist_phase;
    F32 shoot_windup_time = motion->web_swing_assist_cycle_id > 1 ?
        WEB_ASSIST_VISUAL_TETHER_REPEAT_WINDUP_TIME :
        WEB_ASSIST_VISUAL_TETHER_SHOOT_WINDUP_TIME;
    switch((WebSwingVisualTetherState)motion->web_swing_visual_tether_state)
    {
        case WEB_SWING_VISUAL_TETHER_RETRACTING:
            motion->web_swing_visual_tether_progress = MAX(0.0f,
                motion->web_swing_visual_tether_progress -
                WEB_ASSIST_VISUAL_TETHER_RETRACT_RATE * e->timestep);
            if(motion->web_swing_visual_tether_progress <= 0.0f)
            {
                motion->web_swing_visual_tether_visible = 0;
                motion->web_swing_visual_tether_state = WEB_SWING_VISUAL_TETHER_GAP;
                motion->web_swing_visual_tether_gap_ticks = 0;
                motion->web_swing_visual_tether_shoot_ticks = 0;
                motion->web_swing_visual_tether_shoot_time = 0.0f;
                motion->web_swing_anim_catch_ticks = 0;
                filelog_printf("webswing.log",
                               "WEB_SWING %s visual_tether_retracted cycle_id=%u segment_id=%u physics_continuous=1\n",
                               WEB_SWING_LOG_SIDE,
                               motion->web_swing_assist_cycle_id,
                               motion->web_swing_anim_segment_id);
            }
            break;

        case WEB_SWING_VISUAL_TETHER_GAP:
            ++motion->web_swing_visual_tether_gap_ticks;
            if(motion->web_swing_visual_tether_gap_ticks >= WEB_ASSIST_VISUAL_TETHER_MIN_GAP_TICKS &&
               (phase == WEB_SWING_ASSIST_APEX || phase == WEB_SWING_ASSIST_DESCEND))
            {
                // Move the fictional endpoint only while the old web is
                // invisible, then visibly shoot the new line outward.
                webSwingSetAssistedVisualAnchor(e, travel_intent);
                motion->web_swing_visual_tether_visible = 1;
                motion->web_swing_visual_tether_state = WEB_SWING_VISUAL_TETHER_EXTENDING;
                motion->web_swing_visual_tether_progress = 0.0f;
                motion->web_swing_visual_tether_shoot_ticks = 0;
                motion->web_swing_visual_tether_shoot_time = 0.0f;
                motion->web_swing_anim_catch_ticks = 0;
                filelog_printf("webswing.log",
                               "WEB_SWING %s visual_tether_attach cycle_id=%u segment_id=%u phase=%s gap_ticks=%u anchor=(%.2f %.2f %.2f) physics_continuous=1\n",
                               WEB_SWING_LOG_SIDE,
                               motion->web_swing_assist_cycle_id,
                               motion->web_swing_anim_segment_id,
                               webSwingAssistPhaseName(phase),
                               motion->web_swing_visual_tether_gap_ticks,
                               vecParamsXYZ(motion->web_swing_anchor));
            }
            break;

        case WEB_SWING_VISUAL_TETHER_EXTENDING:
            ++motion->web_swing_visual_tether_shoot_ticks;
            motion->web_swing_visual_tether_shoot_time += e->timestep;
            // A fresh attach includes the authored wrist preparation.  A
            // repeated cycle already played that preparation while the old
            // line retracted, so its replacement line leaves immediately.
            if(motion->web_swing_visual_tether_shoot_time >
               shoot_windup_time)
            {
                motion->web_swing_visual_tether_progress = MIN(1.0f,
                    motion->web_swing_visual_tether_progress +
                    WEB_ASSIST_VISUAL_TETHER_EXTEND_RATE * e->timestep);
            }
            if(motion->web_swing_visual_tether_progress >= 1.0f)
            {
                motion->web_swing_visual_tether_state = WEB_SWING_VISUAL_TETHER_ATTACHED;
                motion->web_swing_anim_catch_ticks = 1;
                filelog_printf("webswing.log",
                               "WEB_SWING %s visual_tether_extended cycle_id=%u segment_id=%u shoot_ticks=%u shoot_time=%.3f windup_time=%.3f physics_continuous=1\n",
                               WEB_SWING_LOG_SIDE,
                               motion->web_swing_assist_cycle_id,
                               motion->web_swing_anim_segment_id,
                               motion->web_swing_visual_tether_shoot_ticks,
                               motion->web_swing_visual_tether_shoot_time,
                               shoot_windup_time);
            }
            break;

        case WEB_SWING_VISUAL_TETHER_ATTACHED:
            if(motion->web_swing_anim_catch_ticks > 0 &&
               motion->web_swing_anim_catch_ticks <= WEB_ASSIST_ANIM_CATCH_TICKS)
                ++motion->web_swing_anim_catch_ticks;
            break;

        default:
            break;
    }
}

static void webSwingUpdateAssistedVisualAnchor(Entity *e, const Vec3 travel_intent)
{
    MotionState *motion = e->motion;
    Vec3 target;
    Vec3 lead;
    Vec3 delta;
    F32 blend = MIN(1.0f, WEB_ASSIST_VISUAL_ANCHOR_BLEND * e->timestep);

    copyVec3(ENTPOS(e), target);
    scaleVec3(travel_intent, WEB_SKY_ANCHOR_FORWARD_LEAD, lead);
    addVec3(target, lead, target);
    target[1] += WEB_SKY_ANCHOR_HEIGHT;
    subVec3(target, motion->web_swing_anchor, delta);
    scaleVec3(delta, blend, delta);
    addVec3(motion->web_swing_anchor, delta, motion->web_swing_anchor);
    motion->web_swing_rope_length = distance3(ENTPOS(e), motion->web_swing_anchor);
}

static void webSwingApplyAssistedHorizontal(Entity *e, const Vec3 travel_intent,
                                             WebSwingAssistPhase phase,
                                             int preserve_momentum)
{
    MotionState *motion = e->motion;
    Vec3 horizontal_velocity;
    Vec3 target_velocity;
    Vec3 velocity_delta;
    F32 horizontal_speed;
    F32 target_speed;
    F32 steer_accel;
    F32 max_delta;
    F32 delta_length;

    copyVec3(motion->vel, horizontal_velocity);
    horizontal_velocity[1] = 0.0f;
    horizontal_speed = lengthVec3(horizontal_velocity);

    switch(phase)
    {
        case WEB_SWING_ASSIST_LAUNCH:
            target_speed = MAX(horizontal_speed, WEB_ASSIST_LAUNCH_FORWARD_SPEED);
            steer_accel = WEB_ASSIST_LAUNCH_ACCEL;
            break;
        case WEB_SWING_ASSIST_APEX:
            target_speed = WEB_ASSIST_APEX_SPEED +
                           motion->web_swing_assist_energy * WEB_ASSIST_APEX_ENERGY_SPEED;
            steer_accel = WEB_ASSIST_STEER_APEX;
            break;
        case WEB_SWING_ASSIST_DESCEND:
            target_speed = WEB_ASSIST_DESCEND_SPEED +
                           motion->web_swing_assist_energy * WEB_ASSIST_DESCEND_ENERGY_SPEED;
            steer_accel = WEB_ASSIST_STEER_DESCEND;
            break;
        case WEB_SWING_ASSIST_BOTTOM:
            target_speed = WEB_ASSIST_BOTTOM_SPEED +
                           motion->web_swing_assist_energy * WEB_ASSIST_BOTTOM_ENERGY_SPEED;
            steer_accel = WEB_ASSIST_STEER_BOTTOM;
            break;
        default:
            target_speed = WEB_ASSIST_ASCEND_SPEED +
                           motion->web_swing_assist_energy * WEB_ASSIST_ASCEND_ENERGY_SPEED;
            steer_accel = WEB_ASSIST_STEER_ASCEND;
            break;
    }
    target_speed = MINMAX(target_speed, WEB_ASSIST_HORIZONTAL_MIN_SPEED,
                          WEB_ASSIST_HORIZONTAL_MAX_SPEED);
    if(preserve_momentum)
        target_speed = MAX(horizontal_speed, target_speed);

    scaleVec3(travel_intent, target_speed, target_velocity);
    subVec3(target_velocity, horizontal_velocity, velocity_delta);
    delta_length = lengthVec3(velocity_delta);
    max_delta = steer_accel * e->timestep;
    if(delta_length > max_delta && delta_length > 0.0001f)
        scaleVec3(velocity_delta, max_delta / delta_length, velocity_delta);
    motion->vel[0] += velocity_delta[0];
    motion->vel[2] += velocity_delta[2];
}

static void webSwingAssistedBeginSwoop(Entity *e)
{
    MotionState *motion = e->motion;
    F32 horizontal_speed = sqrt(SQR(motion->vel[0]) + SQR(motion->vel[2]));
    F32 plane_speed = sqrt(SQR(horizontal_speed) + SQR(motion->vel[1]));

    motion->web_swing_assist_swoop_active = 1;
    motion->web_swing_assist_swoop_entry_plane_speed =
        MIN(plane_speed, WEB_ASSIST_TOTAL_MAX_SPEED);
    motion->web_swing_assist_swoop_entry_angle =
        atan2(motion->vel[1], MAX(horizontal_speed, 0.0001f));
    motion->web_swing_assist_swoop_radius = WEB_ASSIST_SWOOP_DEFAULT_RADIUS;
    filelog_printf("webswing.log",
                   "WEB_SWING %s assisted_swoop_begin tick=%u cycle_id=%u entry_angle_deg=%.3f entry_plane_speed=%.3f horizontal_speed=%.3f vertical_speed=%.3f default_radius=%.3f\n",
                   WEB_SWING_LOG_SIDE,
                   motion->tickCounter,
                   motion->web_swing_assist_cycle_id,
                   DEG(motion->web_swing_assist_swoop_entry_angle),
                   motion->web_swing_assist_swoop_entry_plane_speed,
                   horizontal_speed,
                   motion->vel[1],
                   WEB_ASSIST_SWOOP_DEFAULT_RADIUS);
}

static int webSwingApplyAssistedSwoop(Entity *e, const Vec3 travel_intent,
                                      WebSwingAssistPhase phase, F32 clearance)
{
    MotionState *motion = e->motion;
    Vec3 horizontal_direction;
    F32 horizontal_speed;
    F32 plane_speed;
    F32 swing_angle;
    F32 target_angle;
    F32 selected_radius = motion->web_swing_assist_swoop_radius;
    F32 safe_radius = WEB_ASSIST_SWOOP_DEFAULT_RADIUS;
    F32 available_drop = FLT_MAX;
    F32 angular_step;
    F32 horizontal_ratio;
    F32 redirect_strength;
    F32 new_angle;
    F32 new_horizontal_speed;
    int terrain_emergency = 0;

    copyVec3(motion->vel, horizontal_direction);
    horizontal_direction[1] = 0.0f;
    horizontal_speed = lengthVec3(horizontal_direction);
    plane_speed = sqrt(SQR(horizontal_speed) + SQR(motion->vel[1]));
    if(plane_speed < WEB_ASSIST_SWOOP_MIN_PLANE_SPEED)
        return 0;

    if(horizontal_speed > 0.0001f)
        scaleVec3(horizontal_direction, 1.0f / horizontal_speed, horizontal_direction);
    else
        copyVec3(travel_intent, horizontal_direction);

    swing_angle = atan2(motion->vel[1], MAX(horizontal_speed, 0.0001f));
    target_angle = phase == WEB_SWING_ASSIST_BOTTOM ? 0.0f :
                   WEB_ASSIST_SWOOP_EXIT_ANGLE;

    if(phase == WEB_SWING_ASSIST_BOTTOM && swing_angle < 0.0f)
    {
        F32 denominator = 1.0f - cos(fabs(swing_angle));
        F32 altitude_available = ENTPOSY(e) -
            motion->web_swing_assist_low_point_y -
            WEB_ASSIST_ALTITUDE_TRANSITION_PADDING;

        available_drop = altitude_available;
        if(clearance >= 0.0f && clearance < WEB_ASSIST_GROUND_PROBE_DISTANCE)
        {
            available_drop = MIN(available_drop,
                clearance - WEB_ASSIST_GROUND_TARGET_CLEARANCE -
                WEB_ASSIST_GROUND_TRANSITION_PADDING);
        }

        if(denominator > 0.0001f)
            safe_radius = available_drop / denominator;
        selected_radius = MIN(WEB_ASSIST_SWOOP_DEFAULT_RADIUS, safe_radius);
        if(selected_radius < WEB_ASSIST_SWOOP_MIN_RADIUS)
        {
            selected_radius = WEB_ASSIST_SWOOP_MIN_RADIUS;
            terrain_emergency = 1;
        }
        motion->web_swing_assist_swoop_radius = selected_radius;
    }

    horizontal_ratio = horizontal_speed / plane_speed;
    redirect_strength = MINMAX(
        (horizontal_ratio - WEB_ASSIST_SWOOP_HORIZONTAL_BLEND_START) /
        (WEB_ASSIST_SWOOP_HORIZONTAL_BLEND_FULL -
         WEB_ASSIST_SWOOP_HORIZONTAL_BLEND_START),
        0.0f, 1.0f);
    redirect_strength = redirect_strength * redirect_strength *
        (3.0f - 2.0f * redirect_strength);

    angular_step = MIN(WEB_ASSIST_SWOOP_MAX_ANGULAR_STEP,
        plane_speed / MAX(selected_radius, WEB_ASSIST_SWOOP_MIN_RADIUS) *
        MAX(e->timestep, 0.0f));
    angular_step *= redirect_strength;
    new_angle = MIN(target_angle, swing_angle + angular_step);

    if(redirect_strength > 0.0f && new_angle > swing_angle)
    {
        new_horizontal_speed = plane_speed * cos(new_angle);
        motion->vel[0] = horizontal_direction[0] * new_horizontal_speed;
        motion->vel[2] = horizontal_direction[2] * new_horizontal_speed;
        motion->vel[1] = plane_speed * sin(new_angle);
    }

    if(phase == WEB_SWING_ASSIST_BOTTOM)
    {
        F32 downward_speed = MAX(0.0f, -motion->vel[1]);
        int inside_unsafe_clearance =
            (clearance >= 0.0f && clearance < WEB_ASSIST_GROUND_PROBE_DISTANCE &&
             clearance <= WEB_ASSIST_GROUND_TARGET_CLEARANCE +
                          WEB_ASSIST_SWOOP_EMERGENCY_MARGIN) ||
            ENTPOSY(e) <= motion->web_swing_assist_low_point_y +
                          WEB_ASSIST_SWOOP_EMERGENCY_MARGIN;

        if(downward_speed > 0.25f &&
           (terrain_emergency || inside_unsafe_clearance))
        {
            F32 emergency_room = MAX(0.5f, available_drop);
            F32 required_accel = downward_speed * downward_speed /
                                 (2.0f * emergency_room);
            F32 emergency_accel = MIN(WEB_ASSIST_SWOOP_EMERGENCY_MAX_ACCEL,
                MAX(WEB_ASSIST_SWOOP_EMERGENCY_BASE_ACCEL, required_accel));
            F32 applied_correction = emergency_accel * e->timestep;

            motion->vel[1] += applied_correction;
            ++motion->web_swing_assist_swoop_emergency_count;
            filelog_printf("webswing.log",
                           "WEB_SWING %s assisted_swoop_emergency tick=%u cycle_id=%u clearance=%.3f angle_deg=%.3f plane_speed=%.3f required_radius=%.3f selected_radius=%.3f available_drop=%.3f applied_correction=%.3f emergency_count=%u\n",
                           WEB_SWING_LOG_SIDE,
                           motion->tickCounter,
                           motion->web_swing_assist_cycle_id,
                           clearance,
                           DEG(swing_angle),
                           plane_speed,
                           safe_radius,
                           selected_radius,
                           available_drop,
                           applied_correction,
                           motion->web_swing_assist_swoop_emergency_count);
        }
    }

    if(new_angle >= target_angle - 0.0001f)
    {
        if(phase == WEB_SWING_ASSIST_ASCEND)
        {
            motion->web_swing_assist_swoop_active = 0;
            filelog_printf("webswing.log",
                           "WEB_SWING %s assisted_swoop_exit tick=%u cycle_id=%u exit_angle_deg=%.3f plane_speed=%.3f horizontal_speed=%.3f radius=%.3f emergency_count=%u\n",
                           WEB_SWING_LOG_SIDE,
                           motion->tickCounter,
                           motion->web_swing_assist_cycle_id,
                           DEG(new_angle),
                           lengthVec3(motion->vel),
                           sqrt(SQR(motion->vel[0]) + SQR(motion->vel[2])),
                           motion->web_swing_assist_swoop_radius,
                           motion->web_swing_assist_swoop_emergency_count);
        }
        return 1;
    }

    return 0;
}

static void webSwingApplyAssistedController(Entity *e)
{
    MotionState *motion = e->motion;
    WebSwingAssistPhase phase = (WebSwingAssistPhase)motion->web_swing_assist_phase;
    Vec3 travel_intent;
    F32 horizontal_input_magnitude = 0.0f;
    F32 clearance;
    F32 downward_speed;
    F32 bottom_threshold;
    F32 horizontal_speed;
    F32 current_clearance;
    F32 ahead_clearance;
    F32 lookahead_distance;
    F32 speed;
    F32 altitude_trigger_y;
    F32 stopping_distance;
    F32 altitude_stopping_distance;

    // The controller already computes these values below.  Keep a cheap copy
    // for client-side manual capture instead of repeating terrain probes.
    motion->web_swing_assist_current_clearance = -1.0f;
    motion->web_swing_assist_ahead_clearance = -1.0f;
    motion->web_swing_assist_lookahead_distance = -1.0f;
    motion->web_swing_assist_altitude_margin =
        ENTPOSY(e) - motion->web_swing_assist_low_point_y;

    if(phase == WEB_SWING_ASSIST_NONE)
        webSwingAssistedBegin(e, !motion->falling && !motion->jumping);
    phase = (WebSwingAssistPhase)motion->web_swing_assist_phase;
    motion->web_swing_assist_altitude_margin =
        ENTPOSY(e) - motion->web_swing_assist_low_point_y;
    ++motion->web_swing_assist_phase_ticks;

    webSwingGetTravelIntent(e, travel_intent, &horizontal_input_magnitude);
    if(lengthVec3Squared(travel_intent) < 0.0001f)
        return;

    horizontal_speed = sqrt(SQR(motion->vel[0]) + SQR(motion->vel[2]));
    lookahead_distance = MIN(WEB_ASSIST_GROUND_LOOKAHEAD_MAX,
                             WEB_ASSIST_GROUND_LOOKAHEAD_BASE +
                             horizontal_speed * WEB_ASSIST_GROUND_LOOKAHEAD_SPEED);
    {
        Vec3 ahead_position;
        Vec3 half_ahead_position;

        copyVec3(ENTPOS(e), ahead_position);
        ahead_position[0] += travel_intent[0] * lookahead_distance;
        ahead_position[2] += travel_intent[2] * lookahead_distance;
        copyVec3(ENTPOS(e), half_ahead_position);
        half_ahead_position[0] += travel_intent[0] * lookahead_distance * 0.5f;
        half_ahead_position[2] += travel_intent[2] * lookahead_distance * 0.5f;
        current_clearance = HeightAtLoc(ENTPOS(e), DEFAULT_RADIUS, WEB_ASSIST_GROUND_PROBE_DISTANCE);
        ahead_clearance = MIN(HeightAtLoc(half_ahead_position, DEFAULT_RADIUS, WEB_ASSIST_GROUND_PROBE_DISTANCE),
                              HeightAtLoc(ahead_position, DEFAULT_RADIUS, WEB_ASSIST_GROUND_PROBE_DISTANCE));
        clearance = MIN(current_clearance, ahead_clearance);
    }
    motion->web_swing_assist_current_clearance = current_clearance;
    motion->web_swing_assist_ahead_clearance = ahead_clearance;
    motion->web_swing_assist_lookahead_distance = lookahead_distance;
    motion->web_swing_assist_altitude_margin =
        ENTPOSY(e) - motion->web_swing_assist_low_point_y;
    webSwingUpdateAssistedVisualTether(e, travel_intent);
    if(motion->web_swing_visual_tether_visible)
        webSwingUpdateAssistedVisualAnchor(e, travel_intent);
    motion->web_swing_assist_altitude_margin =
        ENTPOSY(e) - motion->web_swing_assist_low_point_y;

    // Physics starts with the activation while the authored wrist shot and
    // visible tether extension continue concurrently.  LAUNCH still owns one
    // boost only; presentation readiness is not a movement gate.
    if(phase == WEB_SWING_ASSIST_LAUNCH)
    {
        F32 forward_component = motion->vel[0] * travel_intent[0] +
                                motion->vel[2] * travel_intent[2];
        F32 forward_delta = MAX(0.0f, WEB_ASSIST_LAUNCH_FORWARD_SPEED - forward_component);
        motion->vel[1] = MAX(motion->vel[1], WEB_ASSIST_LAUNCH_UP_SPEED);
        motion->vel[0] += travel_intent[0] * forward_delta;
        motion->vel[2] += travel_intent[2] * forward_delta;
        filelog_printf("webswing.log",
                       "WEB_SWING %s ground_boost tick=%u cycle_id=%u activation_latency_ticks=0 up_speed=%.3f forward_speed=%.3f velocity=(%.3f %.3f %.3f) web_progress=%.3f web_state=%u presentation_independent=1 one_shot=1\n",
                       WEB_SWING_LOG_SIDE,
                       motion->tickCounter,
                       motion->web_swing_assist_cycle_id,
                       motion->vel[1],
                       sqrt(SQR(motion->vel[0]) + SQR(motion->vel[2])),
                       vecParamsXYZ(motion->vel),
                       motion->web_swing_visual_tether_progress,
                       motion->web_swing_visual_tether_state);
        webSwingAssistedSetPhase(e, WEB_SWING_ASSIST_ASCEND, "IMMEDIATE_PHYSICS");
        phase = WEB_SWING_ASSIST_ASCEND;
    }
    webSwingApplyAssistedHorizontal(e, travel_intent, phase,
        phase == WEB_SWING_ASSIST_BOTTOM ||
        (phase == WEB_SWING_ASSIST_ASCEND &&
         motion->web_swing_assist_cycle_id > 1));

    switch(phase)
    {
        case WEB_SWING_ASSIST_LAUNCH:
            // LAUNCH always resolves to ASCEND above on its first update.
            break;

        case WEB_SWING_ASSIST_ASCEND:
        {
            if(motion->web_swing_assist_swoop_active)
            {
                webSwingApplyAssistedSwoop(e, travel_intent, phase, clearance);
            }
            else
            {
                motion->vel[1] += (WEB_ASSIST_ASCEND_GRAVITY_ASSIST +
                                   motion->web_swing_assist_energy *
                                   WEB_ASSIST_ASCEND_ENERGY_ASSIST) *
                                  e->timestep;
            }
            if(!motion->web_swing_assist_swoop_active && motion->vel[1] <= 0.20f)
            {
                webSwingAssistedSetPhase(e, WEB_SWING_ASSIST_APEX, "VERTICAL_TURNOVER");
                webSwingAssistedReleaseVisualTether(e);
            }
            break;
        }

        case WEB_SWING_ASSIST_APEX:
            motion->vel[1] -= WEB_ASSIST_APEX_DOWN_ACCEL * e->timestep;
            if(motion->web_swing_assist_phase_ticks >= 5 || motion->vel[1] <= -0.20f)
                webSwingAssistedSetPhase(e, WEB_SWING_ASSIST_DESCEND, "APEX_COMPLETE");
            break;

        case WEB_SWING_ASSIST_DESCEND:
            motion->vel[1] -= (WEB_ASSIST_DESCEND_DOWN_ACCEL +
                               motion->web_swing_assist_energy * WEB_ASSIST_DESCEND_ENERGY_ACCEL) *
                              e->timestep;
            downward_speed = MAX(0.0f, -motion->vel[1]);
            stopping_distance = downward_speed * downward_speed /
                                (2.0f * WEB_ASSIST_GROUND_STOP_ACCEL);
            altitude_stopping_distance = downward_speed * downward_speed /
                                         (2.0f * WEB_ASSIST_ALTITUDE_STOP_ACCEL);
            bottom_threshold = WEB_ASSIST_GROUND_TARGET_CLEARANCE +
                               WEB_ASSIST_GROUND_TRANSITION_PADDING +
                               stopping_distance;
            bottom_threshold = MIN(bottom_threshold, WEB_ASSIST_GROUND_PROBE_DISTANCE - 1.0f);
            altitude_trigger_y = motion->web_swing_assist_low_point_y +
                                 WEB_ASSIST_ALTITUDE_TRANSITION_PADDING +
                                 altitude_stopping_distance;
            if(clearance <= bottom_threshold || ENTPOSY(e) <= altitude_trigger_y)
            {
                const char *bottom_reason = ENTPOSY(e) <= altitude_trigger_y &&
                                            clearance > bottom_threshold ?
                                            "ALTITUDE_BAND" : "GROUND_ANTICIPATION";
                motion->web_swing_assist_energy = MIN(WEB_ASSIST_MAX_ENERGY,
                    motion->web_swing_assist_energy + WEB_ASSIST_ENERGY_PER_CYCLE);
                filelog_printf("webswing.log",
                               "WEB_SWING %s assisted_bottom_guard reason=%s cycle_id=%u pos_y=%.3f low_point_y=%.3f altitude_trigger_y=%.3f altitude_stopping_distance=%.3f clearance=%.3f terrain_threshold=%.3f downward_speed=%.3f\n",
                               WEB_SWING_LOG_SIDE,
                               bottom_reason,
                               motion->web_swing_assist_cycle_id,
                               ENTPOSY(e),
                               motion->web_swing_assist_low_point_y,
                               altitude_trigger_y,
                               altitude_stopping_distance,
                               clearance,
                               bottom_threshold,
                               downward_speed);
                webSwingAssistedBeginSwoop(e);
                webSwingAssistedSetPhase(e, WEB_SWING_ASSIST_BOTTOM, bottom_reason);
            }
            break;

        case WEB_SWING_ASSIST_BOTTOM:
        {
            if(webSwingApplyAssistedSwoop(e, travel_intent, phase, clearance))
            {
                ++motion->web_swing_assist_cycle_id;
                ++motion->web_swing_anim_segment_id;
                filelog_printf("webswing.log",
                               "WEB_SWING %s assisted_cycle cycle_id=%u segment_id=%u energy=%.3f zero_cross=1 swoop_radius=%.3f entry_plane_speed=%.3f clearance=%.3f pos=(%.2f %.2f %.2f) velocity=(%.3f %.3f %.3f) preserve_plane_speed=1 preserve_horizontal=1\n",
                               WEB_SWING_LOG_SIDE,
                               motion->web_swing_assist_cycle_id,
                               motion->web_swing_anim_segment_id,
                               motion->web_swing_assist_energy,
                               motion->web_swing_assist_swoop_radius,
                               motion->web_swing_assist_swoop_entry_plane_speed,
                               clearance,
                               vecParamsXYZ(ENTPOS(e)),
                               vecParamsXYZ(motion->vel));
                webSwingAssistedSetPhase(e, WEB_SWING_ASSIST_ASCEND, "BOTTOM_SWEEP_COMPLETE");
            }
            break;
        }

        default:
            webSwingAssistedSetPhase(e, WEB_SWING_ASSIST_APEX, "INVALID_PHASE_RECOVERY");
            break;
    }

    horizontal_speed = sqrt(SQR(motion->vel[0]) + SQR(motion->vel[2]));
    if(!motion->web_swing_assist_swoop_active &&
       horizontal_speed > WEB_ASSIST_HORIZONTAL_MAX_SPEED)
    {
        F32 horizontal_scale = WEB_ASSIST_HORIZONTAL_MAX_SPEED / horizontal_speed;
        motion->vel[0] *= horizontal_scale;
        motion->vel[2] *= horizontal_scale;
    }
    speed = lengthVec3(motion->vel);
    if(speed > WEB_ASSIST_TOTAL_MAX_SPEED)
        scaleVec3(motion->vel, WEB_ASSIST_TOTAL_MAX_SPEED / speed, motion->vel);

    ++motion->web_swing_log_tick;
    if(motion->web_swing_log_tick >= 15)
    {
        filelog_printf("webswing.log",
                       "WEB_SWING %s assisted_tick phase=%s phase_ticks=%u cycle_id=%u energy=%.3f clearance=%.3f current_clearance=%.3f ahead_clearance=%.3f lookahead=%.3f low_point_y=%.3f altitude_margin=%.3f speed=%.3f horizontal_speed=%.3f pos=(%.2f %.2f %.2f) velocity=(%.3f %.3f %.3f) intent=(%.3f %.3f %.3f) input_magnitude=%.3f anchor=(%.2f %.2f %.2f) visual_tether=%d\n",
                       WEB_SWING_LOG_SIDE,
                       webSwingAssistPhaseName((WebSwingAssistPhase)motion->web_swing_assist_phase),
                       motion->web_swing_assist_phase_ticks,
                       motion->web_swing_assist_cycle_id,
                       motion->web_swing_assist_energy,
                       clearance,
                       current_clearance,
                       ahead_clearance,
                       lookahead_distance,
                       motion->web_swing_assist_low_point_y,
                       ENTPOSY(e) - motion->web_swing_assist_low_point_y,
                       lengthVec3(motion->vel),
                       horizontal_speed,
                       vecParamsXYZ(ENTPOS(e)),
                       vecParamsXYZ(motion->vel),
                       vecParamsXYZ(travel_intent),
                       horizontal_input_magnitude,
                       vecParamsXYZ(motion->web_swing_anchor),
                       motion->web_swing_visual_tether_visible);
        motion->web_swing_log_tick = 0;
    }
}

void entWorldWebSwingUpdateAttachment(Entity *e, int web_swing_test_no_attach)
{
    MotionState *motion = e->motion;
    int held = motion->input.web_swing_enabled && motion->input.vel[1] > 0.001f;
    int selected_backend = motion->input.web_swing_backend ?
        WEB_SWING_BACKEND_SKY_ASSISTED : WEB_SWING_BACKEND_REAL_ANCHOR;

    if(motion->web_swing_release_diag_stage == 2 &&
       motion->web_swing_release_diag_tick != motion->tickCounter)
    {
        webSwingLogReleaseState(e, "RELEASE_NEXT_TICK");
        motion->web_swing_release_diag_stage = 0;
    }

    if(!held)
    {
        if(motion->web_swing_attached)
        {
            copyVec3(motion->vel, motion->web_swing_release_pre_velocity);
            motion->web_swing_release_diag_tick = motion->tickCounter;
            motion->web_swing_release_diag_stage = 1;
            webSwingLogReleaseState(e, "RELEASE_PRE");
            motion->web_swing_anim_release_ticks = WEB_ASSIST_ANIM_RELEASE_TICKS;
            ++motion->web_swing_anim_segment_id;
            printf("WEB_SWING %s detach speed=%.3f backend=%s anchor=(%.2f %.2f %.2f) pos=(%.2f %.2f %.2f) input=(%.2f %.2f %.2f)\n",
                   WEB_SWING_LOG_SIDE,
                   lengthVec3(motion->vel),
                   webSwingBackendName(motion->web_swing_active_backend),
                   vecParamsXYZ(motion->web_swing_anchor),
                   vecParamsXYZ(ENTPOS(e)),
                   vecParamsXYZ(motion->input.vel));
            filelog_printf("webswing.log", "WEB_SWING %s detach speed=%.3f backend=%s anchor=(%.2f %.2f %.2f) pos=(%.2f %.2f %.2f) input=(%.2f %.2f %.2f)\n",
                           WEB_SWING_LOG_SIDE,
                           lengthVec3(motion->vel),
                           webSwingBackendName(motion->web_swing_active_backend),
                           vecParamsXYZ(motion->web_swing_anchor),
                           vecParamsXYZ(ENTPOS(e)),
                           vecParamsXYZ(motion->input.vel));
            filelog_printf("webswing.log", "WEB_SWING %s constraint_summary samples=%u soft_corrections=%u radial_corrections=%u hard_corrections=0 max_error=%.4f avg_error=%.4f max_radial_correction=%.4f avg_radial_correction=%.4f max_velocity_dir_delta=%.4f avg_velocity_dir_delta=%.4f velocity_dir_delta_sum=%.4f velocity_dir_delta_large_count=%u velocity_dir_delta_large_pct=%.2f max_consecutive_velocity_dir_delta=%u radial_velocity_removed_count=%u radial_velocity_removed_pct=%.2f avg_radial_velocity_removed=%.4f max_radial_velocity_removed=%.4f radial_velocity_large_count=%u radial_velocity_large_pct=%.2f direction_delta_threshold=%.3f radial_velocity_threshold=%.3f\n",
                           WEB_SWING_LOG_SIDE,
                           motion->web_swing_constraint_samples,
                           motion->web_swing_constraint_soft_correction_count,
                           motion->web_swing_constraint_correction_count,
                           motion->web_swing_constraint_max_error,
                           motion->web_swing_constraint_samples ? motion->web_swing_constraint_error_sum / motion->web_swing_constraint_samples : 0.0f,
                           motion->web_swing_constraint_max_correction,
                           motion->web_swing_constraint_correction_count ? motion->web_swing_constraint_correction_sum / motion->web_swing_constraint_correction_count : 0.0f,
                           motion->web_swing_constraint_max_velocity_dir_delta,
                           motion->web_swing_constraint_samples ? motion->web_swing_constraint_velocity_dir_delta_sum / motion->web_swing_constraint_samples : 0.0f,
                           motion->web_swing_constraint_velocity_dir_delta_sum,
                           motion->web_swing_constraint_velocity_dir_delta_large_count,
                           motion->web_swing_constraint_samples ? 100.0f * motion->web_swing_constraint_velocity_dir_delta_large_count / motion->web_swing_constraint_samples : 0.0f,
                           motion->web_swing_constraint_max_velocity_dir_delta_large_run,
                           motion->web_swing_constraint_radial_velocity_removed_count,
                           motion->web_swing_constraint_samples ? 100.0f * motion->web_swing_constraint_radial_velocity_removed_count / motion->web_swing_constraint_samples : 0.0f,
                           motion->web_swing_constraint_radial_velocity_removed_count ? motion->web_swing_constraint_radial_velocity_removed_sum / motion->web_swing_constraint_radial_velocity_removed_count : 0.0f,
                           motion->web_swing_constraint_max_radial_velocity_removed,
                           motion->web_swing_constraint_radial_velocity_large_count,
                           motion->web_swing_constraint_samples ? 100.0f * motion->web_swing_constraint_radial_velocity_large_count / motion->web_swing_constraint_samples : 0.0f,
                           WEB_SWING_DIRECTION_DELTA_THRESHOLD,
                           WEB_SWING_RADIAL_VELOCITY_THRESHOLD);
        }
        else if(motion->web_swing_anim_release_ticks > 0)
        {
            --motion->web_swing_anim_release_ticks;
        }
        motion->web_swing_attached = 0;
        motion->web_swing_attach_catch_pending = 0;
        motion->web_swing_attach_grounded = 0;
        motion->web_swing_attach_falling = 0;
        motion->web_swing_attach_jumping = 0;
        motion->web_swing_diag_latched = 0;
        motion->web_swing_state_diag_latched = 0;
        motion->web_swing_chain_armed = 0;
        motion->web_swing_chain_ascent_seen = 0;
        motion->web_swing_next_anchor_valid = 0;
        motion->web_swing_ground_launch_active = 0;
        motion->web_swing_ground_launch_ticks = 0;
        motion->web_swing_assist_phase = WEB_SWING_ASSIST_NONE;
        motion->web_swing_assist_phase_ticks = 0;
        motion->web_swing_assist_cycle_id = 0;
        motion->web_swing_assist_energy = 0.0f;
        motion->web_swing_assist_initial_low_point_y = 0.0f;
        motion->web_swing_assist_low_point_y = 0.0f;
        motion->web_swing_assist_current_clearance = -1.0f;
        motion->web_swing_assist_ahead_clearance = -1.0f;
        motion->web_swing_assist_lookahead_distance = -1.0f;
        motion->web_swing_assist_altitude_margin = 0.0f;
        motion->web_swing_assist_swoop_active = 0;
        motion->web_swing_assist_swoop_entry_plane_speed = 0.0f;
        motion->web_swing_assist_swoop_entry_angle = 0.0f;
        motion->web_swing_assist_swoop_radius = 0.0f;
        motion->web_swing_assist_swoop_emergency_count = 0;
        motion->web_swing_visual_tether_visible = 0;
        motion->web_swing_visual_tether_gap_ticks = 0;
        motion->web_swing_visual_tether_shoot_ticks = 0;
        motion->web_swing_visual_tether_shoot_time = 0.0f;
        motion->web_swing_anim_catch_ticks = 0;
        motion->web_swing_anim_shoot_active = 0;
        motion->web_swing_visual_tether_state = WEB_SWING_VISUAL_TETHER_HIDDEN;
        motion->web_swing_visual_tether_progress = 0.0f;
        zeroVec3(motion->web_swing_previous_anchor);
        zeroVec3(motion->web_swing_next_anchor);
        motion->web_swing_log_tick = 0;
        webSwingResetConstraintMetrics(motion);
        if(motion->web_swing_release_diag_stage == 1)
        {
            if(motion->web_swing_active_backend == WEB_SWING_BACKEND_SKY_ASSISTED)
            {
                // The assisted controller was airborne even if a recent floor
                // probe left contact flags set.  Do not let that stale surface
                // classification apply ground friction to the earned release
                // vector on the handoff frame.
                motion->on_surf = 0;
                motion->was_on_surf = 0;
                motion->falling = 1;
                motion->jumping = 0;
            }
            webSwingLogReleaseState(e, "RELEASE_POST_DETACH");
        }
        return;
    }

    if(motion->web_swing_attached && motion->web_swing_active_backend != selected_backend)
    {
        filelog_printf("webswing.log",
                       "WEB_SWING %s backend_switch_release from=%s to=%s speed=%.3f preserve_velocity=1\n",
                       WEB_SWING_LOG_SIDE,
                       webSwingBackendName(motion->web_swing_active_backend),
                       webSwingBackendName(selected_backend),
                       lengthVec3(motion->vel));
        motion->web_swing_attached = 0;
        motion->web_swing_attach_catch_pending = 0;
        motion->web_swing_chain_armed = 0;
        motion->web_swing_chain_ascent_seen = 0;
        motion->web_swing_next_anchor_valid = 0;
        motion->web_swing_ground_launch_active = 0;
        motion->web_swing_assist_phase = WEB_SWING_ASSIST_NONE;
        motion->web_swing_assist_phase_ticks = 0;
        motion->web_swing_assist_cycle_id = 0;
        motion->web_swing_assist_energy = 0.0f;
        motion->web_swing_assist_initial_low_point_y = 0.0f;
        motion->web_swing_assist_low_point_y = 0.0f;
        motion->web_swing_assist_current_clearance = -1.0f;
        motion->web_swing_assist_ahead_clearance = -1.0f;
        motion->web_swing_assist_lookahead_distance = -1.0f;
        motion->web_swing_assist_altitude_margin = 0.0f;
        motion->web_swing_assist_swoop_active = 0;
        motion->web_swing_assist_swoop_entry_plane_speed = 0.0f;
        motion->web_swing_assist_swoop_entry_angle = 0.0f;
        motion->web_swing_assist_swoop_radius = 0.0f;
        motion->web_swing_assist_swoop_emergency_count = 0;
        motion->web_swing_visual_tether_visible = 0;
        motion->web_swing_visual_tether_gap_ticks = 0;
        motion->web_swing_visual_tether_shoot_ticks = 0;
        motion->web_swing_visual_tether_shoot_time = 0.0f;
        motion->web_swing_anim_catch_ticks = 0;
        motion->web_swing_anim_release_ticks = 0;
        motion->web_swing_anim_shoot_active = 0;
        motion->web_swing_visual_tether_state = WEB_SWING_VISUAL_TETHER_HIDDEN;
        motion->web_swing_visual_tether_progress = 0.0f;
        zeroVec3(motion->web_swing_next_anchor);
        zeroVec3(motion->web_swing_previous_anchor);
        motion->web_swing_diag_latched = 0;
        motion->web_swing_state_diag_latched = 0;
        motion->web_swing_log_tick = 0;
        webSwingResetConstraintMetrics(motion);
    }

    // A normal pendulum that hits real ground must not become a repeating
    // collision/constraint oscillator. Ground launch is exempt because it
    // deliberately starts at street level and owns acquisition until clear.
    if(motion->web_swing_attached &&
       motion->web_swing_active_backend == WEB_SWING_BACKEND_REAL_ANCHOR &&
       !motion->web_swing_ground_launch_active &&
       (landed_on_ground || (!motion->falling && !motion->jumping && motion->on_surf)))
    {
        filelog_printf("webswing.log",
                       "WEB_SWING %s ground_strike_release backend=%s anchor=(%.2f %.2f %.2f) pos=(%.2f %.2f %.2f) velocity=(%.3f %.3f %.3f) landed_on_ground=%d on_surf=%d\n",
                       WEB_SWING_LOG_SIDE,
                       webSwingBackendName(motion->web_swing_active_backend),
                       vecParamsXYZ(motion->web_swing_anchor),
                       vecParamsXYZ(ENTPOS(e)),
                       vecParamsXYZ(motion->vel),
                       landed_on_ground, motion->on_surf);
        motion->web_swing_attached = 0;
        motion->web_swing_attach_catch_pending = 0;
        motion->web_swing_chain_armed = 0;
        motion->web_swing_chain_ascent_seen = 0;
        motion->web_swing_next_anchor_valid = 0;
        motion->web_swing_visual_tether_visible = 0;
        motion->web_swing_visual_tether_gap_ticks = 0;
        motion->web_swing_visual_tether_shoot_ticks = 0;
        motion->web_swing_visual_tether_shoot_time = 0.0f;
        motion->web_swing_anim_catch_ticks = 0;
        motion->web_swing_anim_release_ticks = 0;
        motion->web_swing_anim_shoot_active = 0;
        motion->web_swing_visual_tether_state = WEB_SWING_VISUAL_TETHER_HIDDEN;
        motion->web_swing_visual_tether_progress = 0.0f;
        zeroVec3(motion->web_swing_next_anchor);
        zeroVec3(motion->web_swing_previous_anchor);
        motion->web_swing_diag_latched = 0;
        motion->web_swing_state_diag_latched = 0;
    }

    if(!motion->web_swing_attached && !motion->web_swing_state_diag_latched)
    {
        motion->web_swing_state_diag_latched = 1;
        filelog_printf("webswing.log", "WEB_SWING %s state backend=%s web_swing_enabled=%d up=%.3f falling=%d jumping=%d flying=%d on_surf=%d pos=(%.2f %.2f %.2f)\n",
                       WEB_SWING_LOG_SIDE,
                       webSwingBackendName(selected_backend),
                       motion->input.web_swing_enabled,
                       motion->input.vel[1],
                       motion->falling,
                       motion->jumping,
                       motion->input.flying,
                       motion->on_surf,
                       vecParamsXYZ(ENTPOS(e)));
    }

    if(!web_swing_test_no_attach && !motion->web_swing_attached)
    {
        Vec3 anchor;
        F32 rope_length;
        WebSwingAnchorSearchStats search_stats;
        WebSwingAnchorContext context;
        int grounded;
        int found_anchor;

        grounded = !motion->falling && !motion->jumping;
        context = grounded ? WEB_SWING_ANCHOR_FRESH_GROUND : WEB_SWING_ANCHOR_FRESH_AIR;
        found_anchor = selected_backend == WEB_SWING_BACKEND_SKY_ASSISTED ?
            webSwingFindSkyAssistedAnchor(e, context, anchor, &search_stats) :
            webSwingFindAnchor(e, context, anchor, &search_stats);
        if(found_anchor || !motion->web_swing_diag_latched)
        {
            motion->web_swing_diag_latched = 1;
            webSwingLogAttachAttempt(e, &search_stats);
        }

        if(found_anchor)
        {
            motion->web_swing_attached = 1;
            motion->web_swing_active_backend = selected_backend;
            motion->web_swing_visual_tether_visible = 1;
            motion->web_swing_visual_tether_gap_ticks = 0;
            motion->web_swing_visual_tether_shoot_ticks = 0;
            motion->web_swing_visual_tether_shoot_time = 0.0f;
            motion->web_swing_anim_catch_ticks = 0;
            motion->web_swing_anim_release_ticks = 0;
            motion->web_swing_visual_tether_state = WEB_SWING_VISUAL_TETHER_ATTACHED;
            motion->web_swing_visual_tether_progress = 1.0f;
            copyVec3(anchor, motion->web_swing_anchor);
            rope_length = distance3(ENTPOS(e), anchor);
            motion->web_swing_rope_length = MINMAX(rope_length, WEB_MIN_ROPE_LENGTH, WEB_MAX_ROPE_LENGTH);
            motion->web_swing_log_tick = 0;
            webSwingResetConstraintMetrics(motion);
            motion->web_swing_attach_grounded = !motion->falling && !motion->jumping;
            motion->web_swing_attach_falling = motion->falling;
            motion->web_swing_attach_jumping = motion->jumping;
            motion->web_swing_attach_catch_pending =
                selected_backend == WEB_SWING_BACKEND_REAL_ANCHOR && !grounded;
            motion->web_swing_chain_armed = 0;
            motion->web_swing_chain_ascent_seen = 0;
            motion->web_swing_next_anchor_valid = 0;
            zeroVec3(motion->web_swing_next_anchor);
            zeroVec3(motion->web_swing_previous_anchor);
            motion->web_swing_ground_launch_active = grounded;
            motion->web_swing_ground_launch_origin_y = ENTPOSY(e);
            motion->web_swing_ground_launch_ticks = 0;
            ++motion->web_swing_anim_segment_id;
            motion->jumping = 0;
            motion->falling = 1;

            if(selected_backend == WEB_SWING_BACKEND_SKY_ASSISTED)
                webSwingAssistedBegin(e, grounded);

            if(grounded)
            {
                filelog_printf("webswing.log",
                               "WEB_SWING %s ground_launch_begin backend=%s segment_id=%u origin_y=%.3f anchor=(%.2f %.2f %.2f) rope=%.3f clearance_target=%.3f velocity=(%.3f %.3f %.3f) catch_suppressed=1\n",
                               WEB_SWING_LOG_SIDE,
                               webSwingBackendName(motion->web_swing_active_backend),
                               motion->web_swing_anim_segment_id,
                               motion->web_swing_ground_launch_origin_y,
                               vecParamsXYZ(motion->web_swing_anchor),
                               motion->web_swing_rope_length,
                               WEB_GROUND_LAUNCH_CLEARANCE,
                               vecParamsXYZ(motion->vel));
            }

            printf("WEB_SWING %s attach backend=%s anchor=(%.2f %.2f %.2f) rope=%.2f speed=%.3f\n",
                   WEB_SWING_LOG_SIDE,
                   webSwingBackendName(motion->web_swing_active_backend),
                   vecParamsXYZ(motion->web_swing_anchor),
                   motion->web_swing_rope_length,
                   lengthVec3(motion->vel));
            filelog_printf("webswing.log", "WEB_SWING %s attach backend=%s anchor=(%.2f %.2f %.2f) rope=%.2f speed=%.3f\n",
                           WEB_SWING_LOG_SIDE,
                           webSwingBackendName(motion->web_swing_active_backend),
                           vecParamsXYZ(motion->web_swing_anchor),
                           motion->web_swing_rope_length,
                           lengthVec3(motion->vel));
        }
    }

    if(motion->web_swing_attached &&
       motion->web_swing_active_backend == WEB_SWING_BACKEND_SKY_ASSISTED)
    {
        // The authored controller remains airborne while held. World and
        // building collision still run normally in entWorldCollide.
        motion->jumping = 0;
        motion->falling = 1;
    }
}

void entWorldWebSwingApplyConstraint(Entity *e)
{
    MotionState *motion = e->motion;
    Vec3 rope;
    Vec3 tangent_velocity;
    Vec3 tangent_direction;
    Vec3 travel_intent;
    Vec3 tangent_intent;
    Vec3 input_world;
    Vec3 tangent_input;
    Vec3 lateral_anchor_offset;
    Vec3 lateral_repulsion_direction = {0};
    Vec3 forward;
    Vec3 right;
    Vec3 projected;
    Vec3 predicted;
    Vec3 step;
    Vec3 velocity_before_constraint;
    Vec3 velocity_after_constraint;
    Vec3 velocity_before_direction;
    Vec3 velocity_after_direction;
    F32 distance;
    F32 radial_error;
    F32 radial_velocity;
    F32 radial_velocity_removed;
    F32 radial_correction;
    F32 velocity_dir_delta;
    F32 speed;
    F32 arc_height;
    F32 phase_accel;
    F32 forward_speed = 0.0f;
    F32 lateral_anchor_distance = 0.0f;
    F32 lateral_repulsion_scale = 0.0f;
    F32 lateral_repulsion_accel = 0.0f;
    F32 horizontal_input_magnitude = 0.0f;
    F32 tangent_intent_alignment = 0.0f;
    F32 upward_delta = 0.0f;
    F32 forward_delta = 0.0f;
    F32 forward_speed_before = 0.0f;
    WebSwingIntentSource intent_source;
    int hard_correction_fired = 0;
    int soft_correction_fired = 0;
    int phase_pump_suppressed = 0;

    if(!motion->web_swing_attached || e->timestep <= 0.0001f)
        return;

    if(motion->web_swing_active_backend == WEB_SWING_BACKEND_SKY_ASSISTED)
    {
        webSwingApplyAssistedController(e);
        return;
    }

    if(motion->web_swing_ground_launch_active)
    {
        Vec3 to_anchor;
        Vec3 desired_velocity;
        Vec3 velocity_delta;
        F32 anchor_distance;
        F32 velocity_delta_length;
        F32 clearance = ENTPOSY(e) - motion->web_swing_ground_launch_origin_y;

        subVec3(motion->web_swing_anchor, motion->last_pos, to_anchor);
        anchor_distance = normalVec3(to_anchor);
        ++motion->web_swing_ground_launch_ticks;

        if(anchor_distance < 0.001f || anchor_distance > WEB_MAX_ROPE_LENGTH + 5.0f ||
           motion->web_swing_anchor[1] <= motion->web_swing_ground_launch_origin_y ||
           motion->web_swing_ground_launch_ticks > WEB_GROUND_LAUNCH_MAX_TICKS)
        {
            filelog_printf("webswing.log",
                           "WEB_SWING %s ground_launch_abort segment_id=%u reason=%s ticks=%u clearance=%.3f anchor_distance=%.3f anchor=(%.2f %.2f %.2f) pos=(%.2f %.2f %.2f)\n",
                           WEB_SWING_LOG_SIDE,
                           motion->web_swing_anim_segment_id,
                           motion->web_swing_ground_launch_ticks > WEB_GROUND_LAUNCH_MAX_TICKS ? "TIMEOUT" : "ANCHOR_INVALID",
                           motion->web_swing_ground_launch_ticks,
                           clearance,
                           anchor_distance,
                           vecParamsXYZ(motion->web_swing_anchor),
                           vecParamsXYZ(ENTPOS(e)));
            motion->web_swing_ground_launch_active = 0;
            motion->web_swing_attached = 0;
            motion->web_swing_attach_catch_pending = 0;
            motion->web_swing_chain_armed = 0;
            motion->web_swing_chain_ascent_seen = 0;
            motion->web_swing_next_anchor_valid = 0;
            zeroVec3(motion->web_swing_next_anchor);
            return;
        }

        if(clearance >= WEB_GROUND_LAUNCH_CLEARANCE)
        {
            motion->web_swing_ground_launch_active = 0;
            motion->web_swing_rope_length = MINMAX(distance3(ENTPOS(e), motion->web_swing_anchor),
                                                    WEB_MIN_ROPE_LENGTH, WEB_MAX_ROPE_LENGTH);
            filelog_printf("webswing.log",
                           "WEB_SWING %s ground_launch_end segment_id=%u reason=CLEARANCE ticks=%u origin_y=%.3f current_y=%.3f clearance=%.3f anchor=(%.2f %.2f %.2f) rope=%.3f velocity=(%.3f %.3f %.3f)\n",
                           WEB_SWING_LOG_SIDE,
                           motion->web_swing_anim_segment_id,
                           motion->web_swing_ground_launch_ticks,
                           motion->web_swing_ground_launch_origin_y,
                           ENTPOSY(e), clearance,
                           vecParamsXYZ(motion->web_swing_anchor),
                           motion->web_swing_rope_length,
                           vecParamsXYZ(motion->vel));
        }
        else
        {
            scaleVec3(to_anchor, WEB_GROUND_LAUNCH_TARGET_SPEED, desired_velocity);
            subVec3(desired_velocity, motion->vel, velocity_delta);
            velocity_delta_length = lengthVec3(velocity_delta);
            if(velocity_delta_length > WEB_GROUND_LAUNCH_ACCEL * e->timestep)
                scaleVec3(velocity_delta,
                          (WEB_GROUND_LAUNCH_ACCEL * e->timestep) / velocity_delta_length,
                          velocity_delta);
            addVec3(motion->vel, velocity_delta, motion->vel);
            if((motion->web_swing_ground_launch_ticks % 15) == 0)
            {
                filelog_printf("webswing.log",
                               "WEB_SWING %s ground_launch_pull segment_id=%u ticks=%u clearance=%.3f anchor_distance=%.3f velocity=(%.3f %.3f %.3f) acceleration_delta=(%.3f %.3f %.3f)\n",
                               WEB_SWING_LOG_SIDE,
                               motion->web_swing_anim_segment_id,
                               motion->web_swing_ground_launch_ticks,
                               clearance, anchor_distance,
                               vecParamsXYZ(motion->vel),
                               vecParamsXYZ(velocity_delta));
            }
            return;
        }
    }

    if(motion->web_swing_attach_catch_pending)
    {
        Vec3 velocity_before_catch;
        Vec3 velocity_after_catch;

        intent_source = webSwingGetTravelIntent(e, travel_intent, &horizontal_input_magnitude);
        copyVec3(motion->vel, velocity_before_catch);

        if(motion->vel[1] < WEB_ATTACH_UPWARD_TARGET_SPEED)
        {
            upward_delta = MIN(WEB_ATTACH_UPWARD_TARGET_SPEED - motion->vel[1],
                               WEB_ATTACH_UPWARD_MAX_DELTA);
            motion->vel[1] += upward_delta;
        }

        if(intent_source != WEB_SWING_INTENT_NONE)
        {
            forward_speed_before = dotVec3(motion->vel, travel_intent);
            if(forward_speed_before < WEB_ATTACH_FORWARD_TARGET_SPEED)
            {
                forward_delta = MIN(WEB_ATTACH_FORWARD_TARGET_SPEED - forward_speed_before,
                                    WEB_ATTACH_FORWARD_MAX_DELTA);
                scaleVec3(travel_intent, forward_delta, projected);
                addVec3(motion->vel, projected, motion->vel);
            }
        }

        copyVec3(motion->vel, velocity_after_catch);
        printf("WEB_SWING %s attach_catch backend=%s grounded_at_acquisition=%d falling_at_acquisition=%d jumping_at_acquisition=%d velocity_before=(%.3f %.3f %.3f) velocity_after=(%.3f %.3f %.3f) upward_delta=%.3f forward_delta=%.3f forward_speed_before=%.3f intent=(%.3f %.3f %.3f) intent_source=%s input_magnitude=%.3f anchor=(%.2f %.2f %.2f) rope=%.2f\n",
               WEB_SWING_LOG_SIDE,
               webSwingBackendName(motion->web_swing_active_backend),
               motion->web_swing_attach_grounded,
               motion->web_swing_attach_falling,
               motion->web_swing_attach_jumping,
               vecParamsXYZ(velocity_before_catch),
               vecParamsXYZ(velocity_after_catch),
               upward_delta,
               forward_delta,
               forward_speed_before,
               vecParamsXYZ(travel_intent),
               webSwingIntentSourceName(intent_source),
               horizontal_input_magnitude,
               vecParamsXYZ(motion->web_swing_anchor),
               motion->web_swing_rope_length);
        filelog_printf("webswing.log", "WEB_SWING %s attach_catch backend=%s grounded_at_acquisition=%d falling_at_acquisition=%d jumping_at_acquisition=%d velocity_before=(%.3f %.3f %.3f) velocity_after=(%.3f %.3f %.3f) upward_delta=%.3f forward_delta=%.3f forward_speed_before=%.3f intent=(%.3f %.3f %.3f) intent_source=%s input_magnitude=%.3f anchor=(%.2f %.2f %.2f) rope=%.2f\n",
                       WEB_SWING_LOG_SIDE,
                       webSwingBackendName(motion->web_swing_active_backend),
                       motion->web_swing_attach_grounded,
                       motion->web_swing_attach_falling,
                       motion->web_swing_attach_jumping,
                       vecParamsXYZ(velocity_before_catch),
                       vecParamsXYZ(velocity_after_catch),
                       upward_delta,
                       forward_delta,
                       forward_speed_before,
                       vecParamsXYZ(travel_intent),
                       webSwingIntentSourceName(intent_source),
                       horizontal_input_magnitude,
                       vecParamsXYZ(motion->web_swing_anchor),
                       motion->web_swing_rope_length);
        motion->web_swing_attach_catch_pending = 0;
    }

    subVec3(motion->last_pos, motion->web_swing_anchor, rope);
    if(!normalVec3(rope))
        return;

    // Carry existing tangential momentum first.  The assist direction never
    // reverses that momentum just because the player is not pressing a key.
    scaleVec3(rope, dotVec3(motion->vel, rope), projected);
    subVec3(motion->vel, projected, tangent_velocity);
    copyVec3(tangent_velocity, tangent_direction);
    if(!normalVec3(tangent_direction))
    {
        copyVec3(ENTMAT(e)[2], tangent_direction);
        tangent_direction[1] = 0.0f;
        scaleVec3(rope, dotVec3(tangent_direction, rope), projected);
        subVec3(tangent_direction, projected, tangent_direction);
        normalVec3(tangent_direction);
    }

    copyVec3(ENTMAT(e)[2], forward);
    forward[1] = 0.0f;
    normalVec3(forward);
    copyVec3(ENTMAT(e)[0], right);
    right[1] = 0.0f;
    normalVec3(right);

    intent_source = webSwingGetTravelIntent(e, travel_intent, &horizontal_input_magnitude);
    if(intent_source != WEB_SWING_INTENT_NONE)
        forward_speed = dotVec3(motion->vel, travel_intent);
    zeroVec3(tangent_intent);
    if(intent_source != WEB_SWING_INTENT_NONE)
    {
        scaleVec3(rope, dotVec3(travel_intent, rope), projected);
        subVec3(travel_intent, projected, tangent_intent);
        if(normalVec3(tangent_intent))
            tangent_intent_alignment = dotVec3(tangent_direction, tangent_intent);
    }

    // Explicit input that opposes the current pendulum direction must not
    // continue pumping the unwanted phase.  Passive facing intent does not
    // suppress the normal energy assist.
    if(intent_source == WEB_SWING_INTENT_INPUT &&
       lengthVec3Squared(tangent_intent) > 0.0001f &&
       tangent_intent_alignment < 0.0f)
    {
        phase_pump_suppressed = 1;
    }

    arc_height = (ENTPOSY(e) - (motion->web_swing_anchor[1] - motion->web_swing_rope_length)) /
                 MAX(motion->web_swing_rope_length, WEB_MIN_ROPE_LENGTH);
    arc_height = MINMAX(arc_height, 0.0f, 1.0f);

    if(!motion->web_swing_chain_armed &&
       arc_height <= WEB_CHAIN_ARM_ARC_HEIGHT &&
       motion->vel[1] <= WEB_CHAIN_ARM_DOWN_SPEED)
    {
        motion->web_swing_chain_armed = 1;
        motion->web_swing_chain_ascent_seen = 0;
        motion->web_swing_next_anchor_valid = 0;
        zeroVec3(motion->web_swing_next_anchor);
        motion->web_swing_diag_latched = 0;
        webSwingLogChainArm(e, arc_height, forward_speed);
    }

    if(motion->web_swing_chain_armed && !motion->web_swing_next_anchor_valid &&
       motion->web_swing_chain_search_tick != motion->tickCounter)
    {
        Vec3 next_anchor;
        WebSwingAnchorSearchStats next_search_stats;

        motion->web_swing_chain_search_tick = motion->tickCounter;
        if((motion->web_swing_active_backend == WEB_SWING_BACKEND_SKY_ASSISTED &&
            webSwingFindSkyAssistedAnchor(e, WEB_SWING_ANCHOR_CHAIN_NEXT, next_anchor, &next_search_stats)) ||
           (motion->web_swing_active_backend == WEB_SWING_BACKEND_REAL_ANCHOR &&
            webSwingFindAnchor(e, WEB_SWING_ANCHOR_CHAIN_NEXT, next_anchor, &next_search_stats)))
        {
            copyVec3(next_anchor, motion->web_swing_next_anchor);
            motion->web_swing_next_anchor_retention = next_search_stats.selected_velocity_retention;
            motion->web_swing_next_anchor_valid = 1;
            webSwingLogAttachAttempt(e, &next_search_stats);
            webSwingLogChainPending(e, &next_search_stats);
        }
        else if(!motion->web_swing_diag_latched)
        {
            motion->web_swing_diag_latched = 1;
            webSwingLogAttachAttempt(e, &next_search_stats);
        }
    }

    if(motion->web_swing_chain_armed &&
       motion->vel[1] >= WEB_CHAIN_RELEASE_UP_SPEED &&
       forward_speed >= WEB_CHAIN_MIN_FORWARD_SPEED)
    {
        motion->web_swing_chain_ascent_seen = 1;
        if(motion->web_swing_next_anchor_valid)
        {
            Vec3 previous_anchor;
            Vec3 incoming_velocity;
            F32 predicted_retention = motion->web_swing_next_anchor_retention;

            copyVec3(motion->web_swing_anchor, previous_anchor);
            copyVec3(motion->vel, incoming_velocity);
            copyVec3(previous_anchor, motion->web_swing_previous_anchor);
            copyVec3(motion->web_swing_next_anchor, motion->web_swing_anchor);
            motion->web_swing_rope_length = MINMAX(distance3(ENTPOS(e), motion->web_swing_anchor),
                                                    WEB_MIN_ROPE_LENGTH, WEB_MAX_ROPE_LENGTH);
            motion->web_swing_attach_catch_pending = 0;
            motion->web_swing_chain_armed = 0;
            motion->web_swing_chain_ascent_seen = 0;
            motion->web_swing_next_anchor_valid = 0;
            zeroVec3(motion->web_swing_next_anchor);
            ++motion->web_swing_anim_segment_id;
            motion->web_swing_diag_latched = 0;
            motion->web_swing_log_tick = 0;
            webSwingResetConstraintMetrics(motion);
            webSwingLogChainHandoff(e, previous_anchor, incoming_velocity, predicted_retention);
            return;
        }
    }

    if(motion->web_swing_chain_armed && motion->web_swing_chain_ascent_seen &&
       !motion->web_swing_next_anchor_valid &&
       (arc_height >= WEB_CHAIN_LATE_RELEASE_ARC_HEIGHT ||
        motion->vel[1] <= WEB_CHAIN_APEX_UP_SPEED))
    {
        filelog_printf("webswing.log",
                       "WEB_SWING %s chain_late_release backend=%s reason=NO_GOOD_NEXT_ANCHOR anchor=(%.2f %.2f %.2f) pos=(%.2f %.2f %.2f) velocity=(%.3f %.3f %.3f) arc_height=%.3f forward_speed=%.3f preserve_velocity=1\n",
                       WEB_SWING_LOG_SIDE,
                       webSwingBackendName(motion->web_swing_active_backend),
                       vecParamsXYZ(motion->web_swing_anchor),
                       vecParamsXYZ(ENTPOS(e)),
                       vecParamsXYZ(motion->vel),
                       arc_height, forward_speed);
        copyVec3(motion->web_swing_anchor, motion->web_swing_previous_anchor);
        motion->web_swing_attached = 0;
        motion->web_swing_attach_catch_pending = 0;
        motion->web_swing_chain_armed = 0;
        motion->web_swing_chain_ascent_seen = 0;
        motion->web_swing_next_anchor_valid = 0;
        zeroVec3(motion->web_swing_next_anchor);
        motion->web_swing_diag_latched = 0;
        motion->web_swing_state_diag_latched = 0;
        motion->web_swing_log_tick = 0;
        return;
    }

    if(arc_height < 0.55f)
    {
        phase_accel = WEB_PENDULUM_ACCEL_DESCENT +
                      (WEB_PENDULUM_ACCEL_ASCENT - WEB_PENDULUM_ACCEL_DESCENT) * (arc_height / 0.55f);
    }
    else
    {
        phase_accel = WEB_PENDULUM_ACCEL_ASCENT +
                      (WEB_PENDULUM_ACCEL_APEX - WEB_PENDULUM_ACCEL_ASCENT) * ((arc_height - 0.55f) / 0.45f);
    }

    if(!phase_pump_suppressed)
    {
        scaleVec3(tangent_direction, phase_accel * e->timestep, projected);
        addVec3(motion->vel, projected, motion->vel);
    }

    if(lengthVec3Squared(tangent_intent) > 0.0001f)
    {
        scaleVec3(tangent_intent, WEB_FORWARD_TANGENT_ACCEL * e->timestep, projected);
        addVec3(motion->vel, projected, motion->vel);
    }

    // motion->input.vel is already world-space here.  W/A/D are steering
    // input, not the source of swing motion; remove vertical jump input and
    // project the world-space steering vector directly onto the rope tangent.
    copyVec3(motion->input.vel, input_world);
    input_world[1] = 0.0f;
    zeroVec3(tangent_input);
    scaleVec3(rope, dotVec3(input_world, rope), projected);
    subVec3(input_world, projected, tangent_input);
    if(normalVec3(tangent_input))
    {
        scaleVec3(tangent_input, WEB_STEER_ACCEL * e->timestep, projected);
        addVec3(motion->vel, projected, motion->vel);
    }

    // A side-mounted real anchor can pull the player toward a building.  Only
    // the anchor offset perpendicular to current travel intent is corrected;
    // an anchor directly ahead never receives a backward push.
    if(motion->web_swing_active_backend == WEB_SWING_BACKEND_REAL_ANCHOR &&
       intent_source != WEB_SWING_INTENT_NONE)
    {
        Vec3 anchor_offset;
        F32 anchor_along_intent;

        subVec3(motion->web_swing_anchor, ENTPOS(e), anchor_offset);
        anchor_offset[1] = 0.0f;
        anchor_along_intent = dotVec3(anchor_offset, travel_intent);
        scaleVec3(travel_intent, anchor_along_intent, projected);
        subVec3(anchor_offset, projected, lateral_anchor_offset);
        lateral_anchor_distance = lengthVec3(lateral_anchor_offset);
        if(lateral_anchor_distance > WEB_LATERAL_REPULSION_MIN_DIST)
        {
            F32 ramp = MINMAX((lateral_anchor_distance - WEB_LATERAL_REPULSION_MIN_DIST) /
                              (WEB_LATERAL_REPULSION_FULL_DIST - WEB_LATERAL_REPULSION_MIN_DIST),
                              0.0f, 1.0f);

            // Smoothstep keeps the correction gentle as a side anchor enters
            // the active range while reaching the bounded maximum at full
            // lateral separation.
            ramp = ramp * ramp * (3.0f - 2.0f * ramp);
            scaleVec3(lateral_anchor_offset, -1.0f / lateral_anchor_distance,
                      lateral_repulsion_direction);
            lateral_repulsion_scale = ramp;
            lateral_repulsion_accel = WEB_LATERAL_REPULSION_ACCEL * ramp;
            scaleVec3(lateral_repulsion_direction,
                      lateral_repulsion_accel * e->timestep, projected);
            addVec3(motion->vel, projected, motion->vel);
        }
    }

    speed = lengthVec3(motion->vel);
    if(speed > WEB_MAX_SPEED)
        scaleVec3(motion->vel, WEB_MAX_SPEED / speed, motion->vel);

    copyVec3(motion->vel, velocity_before_constraint);
    scaleVec3(motion->vel, e->timestep, step);
    addVec3(motion->last_pos, step, predicted);
    subVec3(predicted, motion->web_swing_anchor, rope);
    distance = normalVec3(rope);
    radial_error = distance - motion->web_swing_rope_length;
    radial_velocity = dotVec3(velocity_before_constraint, rope);
    radial_velocity_removed = 0.0f;
    radial_correction = 0.0f;

    // Keep the boundary forgiving: preserve tangential momentum and remove
    // only outward radial velocity while the predicted point is taut.
    if(distance >= motion->web_swing_rope_length - WEB_ROPE_SLOP && radial_velocity > 0.0f)
    {
        radial_velocity_removed = radial_velocity;
        scaleVec3(rope, radial_velocity, projected);
        subVec3(motion->vel, projected, motion->vel);
        soft_correction_fired = 1;
    }

    scaleVec3(motion->vel, e->timestep, step);
    addVec3(motion->last_pos, step, predicted);
    subVec3(predicted, motion->web_swing_anchor, rope);
    distance = normalVec3(rope);
    if(distance > motion->web_swing_rope_length + WEB_ROPE_SLOP)
    {
        F32 bias_distance = MIN(distance - motion->web_swing_rope_length - WEB_ROPE_SLOP,
                                WEB_ROPE_BIAS_MAX_SPEED * e->timestep);
        F32 bias_speed = MIN(bias_distance * WEB_ROPE_BIAS_GAIN / e->timestep,
                             WEB_ROPE_BIAS_MAX_SPEED);
        scaleVec3(rope, -bias_speed, projected);
        addVec3(motion->vel, projected, motion->vel);
        radial_correction = bias_speed * e->timestep;
        soft_correction_fired = 1;
    }

    copyVec3(motion->vel, velocity_after_constraint);
    velocity_dir_delta = 0.0f;
    copyVec3(velocity_before_constraint, velocity_before_direction);
    copyVec3(velocity_after_constraint, velocity_after_direction);
    if(normalVec3(velocity_before_direction) && normalVec3(velocity_after_direction))
        velocity_dir_delta = 1.0f - MAX(-1.0f, MIN(1.0f, dotVec3(velocity_before_direction, velocity_after_direction)));

    ++motion->web_swing_constraint_samples;
    motion->web_swing_constraint_velocity_dir_delta_sum += velocity_dir_delta;
    if(velocity_dir_delta >= WEB_SWING_DIRECTION_DELTA_THRESHOLD)
    {
        ++motion->web_swing_constraint_velocity_dir_delta_large_count;
        ++motion->web_swing_constraint_velocity_dir_delta_large_run;
        motion->web_swing_constraint_max_velocity_dir_delta_large_run = MAX(motion->web_swing_constraint_max_velocity_dir_delta_large_run,
                                                                              motion->web_swing_constraint_velocity_dir_delta_large_run);
    }
    else
    {
        motion->web_swing_constraint_velocity_dir_delta_large_run = 0;
    }
    if(soft_correction_fired)
        ++motion->web_swing_constraint_soft_correction_count;
    if(radial_error > 0.0f)
    {
        motion->web_swing_constraint_error_sum += radial_error;
        motion->web_swing_constraint_max_error = MAX(motion->web_swing_constraint_max_error, radial_error);
    }
    if(radial_correction > 0.0f)
    {
        ++motion->web_swing_constraint_correction_count;
        motion->web_swing_constraint_correction_sum += radial_correction;
        motion->web_swing_constraint_max_correction = MAX(motion->web_swing_constraint_max_correction, radial_correction);
    }
    motion->web_swing_constraint_max_velocity_dir_delta = MAX(motion->web_swing_constraint_max_velocity_dir_delta, velocity_dir_delta);
    if(radial_velocity_removed > 0.0f)
    {
        ++motion->web_swing_constraint_radial_velocity_removed_count;
        motion->web_swing_constraint_radial_velocity_removed_sum += radial_velocity_removed;
        motion->web_swing_constraint_max_radial_velocity_removed = MAX(motion->web_swing_constraint_max_radial_velocity_removed,
                                                                        radial_velocity_removed);
        if(radial_velocity_removed >= WEB_SWING_RADIAL_VELOCITY_THRESHOLD)
            ++motion->web_swing_constraint_radial_velocity_large_count;
    }

    speed = lengthVec3(motion->vel);
    if(speed > WEB_MAX_SPEED)
        scaleVec3(motion->vel, WEB_MAX_SPEED / speed, motion->vel);

    ++motion->web_swing_log_tick;
    if(motion->web_swing_log_tick >= 15)
    {
        printf("WEB_SWING %s swing speed=%.3f rope=%.2f pos=(%.2f %.2f %.2f) input=(%.2f %.2f %.2f)\n",
               WEB_SWING_LOG_SIDE,
               lengthVec3(motion->vel), motion->web_swing_rope_length,
               vecParamsXYZ(ENTPOS(e)),
               vecParamsXYZ(motion->input.vel));
        filelog_printf("webswing.log", "WEB_SWING %s swing speed=%.3f rope=%.2f pos=(%.2f %.2f %.2f) input=(%.2f %.2f %.2f)\n",
                       WEB_SWING_LOG_SIDE,
                       lengthVec3(motion->vel), motion->web_swing_rope_length,
                       vecParamsXYZ(ENTPOS(e)),
                       vecParamsXYZ(motion->input.vel));
        filelog_printf("webswing.log", "WEB_SWING %s constraint pre_distance=%.3f rope=%.3f radial_error=%.3f radial_velocity=%.3f radial_velocity_removed=%.3f vel_before=(%.3f %.3f %.3f) vel_after=(%.3f %.3f %.3f) hard_correction=%d soft_correction=%d radial_correction=%.4f velocity_dir_delta=%.4f\n",
                       WEB_SWING_LOG_SIDE,
                       radial_error + motion->web_swing_rope_length,
                       motion->web_swing_rope_length,
                       radial_error,
                       radial_velocity,
                       radial_velocity_removed,
                       vecParamsXYZ(velocity_before_constraint),
                       vecParamsXYZ(velocity_after_constraint),
                       hard_correction_fired,
                       soft_correction_fired,
                       radial_correction,
                       velocity_dir_delta);
        if(lengthVec3Squared(input_world) > 0.0001f)
        {
            printf("WEB_SWING %s steering intent_source=%s forward=(%.3f %.3f %.3f) right=(%.3f %.3f %.3f) input_world=(%.3f %.3f %.3f) tangent_input=(%.3f %.3f %.3f) tangent_direction=(%.3f %.3f %.3f) tangent_intent=(%.3f %.3f %.3f) tangent_intent_alignment=%.3f phase_pump_suppressed=%d\n",
                   WEB_SWING_LOG_SIDE,
                   webSwingIntentSourceName(intent_source),
                   vecParamsXYZ(forward), vecParamsXYZ(right),
                   vecParamsXYZ(input_world), vecParamsXYZ(tangent_input),
                   vecParamsXYZ(tangent_direction), vecParamsXYZ(tangent_intent),
                   tangent_intent_alignment, phase_pump_suppressed);
            filelog_printf("webswing.log", "WEB_SWING %s steering intent_source=%s forward=(%.3f %.3f %.3f) right=(%.3f %.3f %.3f) input_world=(%.3f %.3f %.3f) tangent_input=(%.3f %.3f %.3f) tangent_direction=(%.3f %.3f %.3f) tangent_intent=(%.3f %.3f %.3f) tangent_intent_alignment=%.3f phase_pump_suppressed=%d\n",
                           WEB_SWING_LOG_SIDE,
                           webSwingIntentSourceName(intent_source),
                           vecParamsXYZ(forward), vecParamsXYZ(right),
                           vecParamsXYZ(input_world), vecParamsXYZ(tangent_input),
                           vecParamsXYZ(tangent_direction), vecParamsXYZ(tangent_intent),
                           tangent_intent_alignment, phase_pump_suppressed);
        }
        if(global_state.webswing_dev)
        {
            printf("WEB_SWING %s lateral_repulsion lateral_anchor_distance=%.3f repulsion_scale=%.3f repulsion_accel=%.4f direction=(%.3f %.3f %.3f)\n",
                   WEB_SWING_LOG_SIDE,
                   lateral_anchor_distance,
                   lateral_repulsion_scale,
                   lateral_repulsion_accel,
                   vecParamsXYZ(lateral_repulsion_direction));
            filelog_printf("webswing.log", "WEB_SWING %s lateral_repulsion lateral_anchor_distance=%.3f repulsion_scale=%.3f repulsion_accel=%.4f direction=(%.3f %.3f %.3f)\n",
                           WEB_SWING_LOG_SIDE,
                           lateral_anchor_distance,
                           lateral_repulsion_scale,
                           lateral_repulsion_accel,
                           vecParamsXYZ(lateral_repulsion_direction));
        }
        motion->web_swing_log_tick = 0;
    }
}

static int webCrawlNormalizeVerticalNormal(const Vec3 source, Vec3 normalized)
{
    copyVec3(source, normalized);

    if(normalVec3(normalized) < 0.001f)
        return 0;

    if(fabs(normalized[1]) > WEB_CRAWL_MAX_NORMAL_Y)
        return 0;

    return validateVec3(normalized);
}

static void webCrawlRecordWallContact(
    Entity *e,
    const Vec3 collision_normal,
    const Vec3 collision_point,
    const Vec3 attempted_position)
{
    MotionState *motion = e->motion;
    Vec3 normal;
    Vec3 to_player;
    int new_contact_episode;

    if(!motion->input.web_crawl_enabled && !motion->web_crawl_attached)
        return;

    if(!webCrawlNormalizeVerticalNormal(collision_normal, normal))
        return;

    subVec3(attempted_position, collision_point, to_player);
    to_player[1] = 0.0f;

    if(normalVec3(to_player) > 0.001f &&
       dotVec3(normal, to_player) < 0.0f)
    {
        scaleVec3(normal, -1.0f, normal);
    }

    /* Flat-wall canary only; do not silently turn corners into traversal. */
    if(motion->web_crawl_attached &&
       dotVec3(normal, motion->web_crawl_wall_normal) < WEB_CRAWL_SAME_WALL_DOT)
    {
        return;
    }

    new_contact_episode =
        motion->web_crawl_last_contact_tick == 0 ||
        motion->tickCounter < motion->web_crawl_last_contact_tick ||
        motion->tickCounter - motion->web_crawl_last_contact_tick >
            WEB_CRAWL_CONTACT_GRACE_TICKS;

    copyVec3(normal, motion->web_crawl_wall_normal);
    motion->web_crawl_last_contact_tick = motion->tickCounter;

    if(new_contact_episode)
    {
        filelog_printf(
            "webcrawl.log",
            "WEB_CRAWL %s contact tick=%u normal=(%.3f %.3f %.3f) hit=(%.3f %.3f %.3f) attempted=(%.3f %.3f %.3f)\n",
            WEB_CRAWL_LOG_SIDE,
            motion->tickCounter,
            vecParamsXYZ(normal),
            vecParamsXYZ(collision_point),
            vecParamsXYZ(attempted_position));
    }
}

static int webCrawlHasRecentContact(const MotionState *motion)
{
    if(!motion->web_crawl_last_contact_tick)
        return 0;

    if(motion->tickCounter < motion->web_crawl_last_contact_tick)
        return 0;

    if(motion->tickCounter - motion->web_crawl_last_contact_tick >
       WEB_CRAWL_CONTACT_GRACE_TICKS)
    {
        return 0;
    }

    return fabs(motion->web_crawl_wall_normal[1]) <= WEB_CRAWL_MAX_NORMAL_Y &&
           lengthVec3Squared(motion->web_crawl_wall_normal) > 0.5f;
}

static int webCrawlBuildBasis(
    const Vec3 wall_normal,
    Vec3 wall_up,
    Vec3 wall_right)
{
    Vec3 world_up = {0.0f, 1.0f, 0.0f};
    Vec3 projected;

    scaleVec3(wall_normal, dotVec3(world_up, wall_normal), projected);
    subVec3(world_up, projected, wall_up);

    if(normalVec3(wall_up) < 0.001f)
        return 0;

    /* N x UP is character-right for the player facing -N. */
    crossVec3(wall_normal, wall_up, wall_right);

    if(normalVec3(wall_right) < 0.001f)
        return 0;

    return 1;
}

static void webCrawlDetach(Entity *e, const char *reason, U32 cooldown_ticks)
{
    MotionState *motion = e->motion;

    if(motion->web_crawl_attached)
    {
        filelog_printf(
            "webcrawl.log",
            "WEB_CRAWL %s detach reason=%s tick=%u pos=(%.3f %.3f %.3f) velocity=(%.3f %.3f %.3f) normal=(%.3f %.3f %.3f)\n",
            WEB_CRAWL_LOG_SIDE,
            reason,
            motion->tickCounter,
            vecParamsXYZ(ENTPOS(e)),
            vecParamsXYZ(motion->vel),
            vecParamsXYZ(motion->web_crawl_wall_normal));
    }

    motion->web_crawl_attached = 0;

    if(cooldown_ticks > motion->web_crawl_detach_cooldown_ticks)
        motion->web_crawl_detach_cooldown_ticks = cooldown_ticks;
}

static int webCrawlApplyController(Entity *e)
{
    MotionState *motion = e->motion;
    Vec3 input;
    Vec3 input_dir;
    Vec3 wall_up;
    Vec3 wall_right;
    Vec3 desired;
    Vec3 temp;
    Vec3 adhesion;
    F32 input_mag;
    F32 input_scale;
    F32 climb_input;
    F32 side_input;
    F32 desired_mag;
    int recent_contact;

    if(motion->web_crawl_detach_cooldown_ticks)
        --motion->web_crawl_detach_cooldown_ticks;

    /* Disable means completely return control to stock physics. */
    if(!motion->input.web_crawl_enabled)
    {
        if(motion->web_crawl_attached)
            webCrawlDetach(e, "DISABLED", 0);

        motion->web_crawl_last_contact_tick = 0;

        if(!motion->web_crawl_attached)
            zeroVec3(motion->web_crawl_wall_normal);

        return 0;
    }

    /* Web Swing wins traversal ownership. */
    if(motion->web_swing_attached)
    {
        if(motion->web_crawl_attached)
        {
            webCrawlDetach(
                e,
                "WEB_SWING",
                WEB_CRAWL_REATTACH_COOLDOWN_TICKS);
        }

        return 0;
    }

    recent_contact = webCrawlHasRecentContact(motion);

    /* Jump is an explicit outward detach; normal jump handling remains stock. */
    if(motion->web_crawl_attached && motion->input.vel[1] > 0.001f)
    {
        Vec3 normal;

        copyVec3(motion->web_crawl_wall_normal, normal);
        webCrawlDetach(
            e,
            "JUMP",
            WEB_CRAWL_REATTACH_COOLDOWN_TICKS);

        motion->on_surf = 0;
        motion->was_on_surf = 0;
        motion->falling = 1;
        motion->vel[0] = normal[0] * WEB_CRAWL_JUMP_OUT_SPEED;
        motion->vel[2] = normal[2] * WEB_CRAWL_JUMP_OUT_SPEED;

        return 0;
    }

    if(motion->web_crawl_attached && !recent_contact)
    {
        webCrawlDetach(e, "CONTACT_LOST", 0);
        return 0;
    }

    /* Horizontal user command only; vertical input remains jump/detach. */
    copyVec3(motion->input.vel, input);
    input[1] = 0.0f;

    input_mag = lengthVec3(input);
    input_scale = MIN(input_mag, 1.0f);

    if(input_mag > 0.001f)
    {
        scaleVec3(input, 1.0f / input_mag, input_dir);
    }
    else
    {
        zeroVec3(input_dir);
    }

    /* Initial acquisition requires native wall contact and input into it. */
    if(!motion->web_crawl_attached)
    {
        F32 into_wall;

        if(motion->web_crawl_detach_cooldown_ticks ||
           !recent_contact ||
           motion->input.vel[1] > 0.001f ||
           input_mag <= 0.001f)
        {
            return 0;
        }

        into_wall = -dotVec3(input_dir, motion->web_crawl_wall_normal) * input_scale;
        if(into_wall < WEB_CRAWL_ATTACH_INPUT_MIN)
            return 0;

        motion->web_crawl_attached = 1;
        filelog_printf(
            "webcrawl.log",
            "WEB_CRAWL %s attach tick=%u pos=(%.3f %.3f %.3f) normal=(%.3f %.3f %.3f) into=%.3f\n",
            WEB_CRAWL_LOG_SIDE,
            motion->tickCounter,
            vecParamsXYZ(ENTPOS(e)),
            vecParamsXYZ(motion->web_crawl_wall_normal),
            into_wall);
    }

    if(!webCrawlBuildBasis(motion->web_crawl_wall_normal, wall_up, wall_right))
    {
        webCrawlDetach(e, "INVALID_BASIS", 0);
        return 0;
    }

    /* W into wall -> +wall_up; S away -> -wall_up; A/D -> wall_right. */
    climb_input = -dotVec3(input_dir, motion->web_crawl_wall_normal) * input_scale;
    side_input = dotVec3(input_dir, wall_right) * input_scale;

    /* Ground exit uses actual ground state, not last_surf_type. */
    if(!motion->falling && motion->surf_normal[1] > WEB_CRAWL_GROUND_NORMAL_Y &&
       climb_input <= 0.05f)
    {
        webCrawlDetach(e, "GROUND", 0);
        return 0;
    }

    zeroVec3(desired);
    scaleVec3(wall_up, climb_input, temp);
    addVec3(desired, temp, desired);
    scaleVec3(wall_right, side_input, temp);
    addVec3(desired, temp, desired);

    desired_mag = lengthVec3(desired);
    if(desired_mag > 1.0f)
        scaleVec3(desired, 1.0f / desired_mag, desired);

    scaleVec3(desired, WEB_CRAWL_SPEED, motion->vel);

    /* Inward bias lets the native SlideWall solver retain contact. */
    scaleVec3(motion->web_crawl_wall_normal, -WEB_CRAWL_ADHESION_SPEED, adhesion);
    addVec3(motion->vel, adhesion, motion->vel);

    return 1;
}

F32 HeightAtLoc(const Vec3 vec, F32 radius, F32 dist)
{
    CollInfo coll;
    Vec3 top;
    Vec3 bot;

    // Move the top up a tiny bit in case the point is coincident with the ground.
    copyVec3(vec, top);
    top[1] += 0.1f;

    copyVec3(vec, bot);
    bot[1] -= dist;

    if (!collide(top,bot,&coll,radius,COLL_DISTFROMSTART|COLL_CYLINDER))
        copyVec3(bot,coll.mat[3]);
    return top[1] - coll.mat[3][1];
}

F32 entHeight(Entity *e, F32 dist)
{
    return HeightAtLoc(ENTPOS(e), 1.0f, dist);
}

static int SlideWall(Entity *e,Vec3 top,Vec3 bot,Vec3 pos)
{
    int            i;
    CollInfo    coll;
    Vec3        dv,pt1,pt2;
    F32            d,h,rad = DEFAULT_RADIUS;

    //if (0)
    //{
    //    copyVec3(bot,pt1);
    //    copyVec3(bot,pt2);
    //    pt1[1] += 6.0;
    //    pt2[1] += 1.5;
    //    coll.tri_colls = 0;
    //    coll.coll_count = 0;
    //    coll.coll_max = 0;
    //    collide(pt1,pt2,&coll,rad,COLL_DISTFROMCENTER | COLL_CYLINDER | COLL_BOTHSIDES | COLL_GATHERTRIS);
    //    free(coll.tri_colls);
    //    if (coll.coll_count)
    //        printf("\n");
    //}

    h = bot[1];
    for(i=0;i<3;i++)
    {
        copyVec3(bot,pt1);
        copyVec3(bot,pt2);
        pt1[1] += 6.0;
        pt2[1] += 1.5;
        if (!collide(pt1,pt2,&coll,rad,COLL_DISTFROMCENTER | COLL_CYLINDER | COLL_BOTHSIDES))
        {
            #if MDBG
                if (!i)
                    xyprintf(5,13,"NOSLIDE");
            #endif
            copyVec3(top,pt1);
            copyVec3(bot,pt2);
            pt1[1] += 3.5;
            pt2[1] += 3.5;

            subVec3(top,bot,dv);
            if (lengthVec3Squared(dv) < 1.f || !collGrid(0,pt1,pt2,&coll,rad * 0.667,COLL_DISTFROMCENTER))
            {
                last_surf.ctri = 0;
                copyVec3(bot,top);
                return 0;
            }
            else
            {
                #if MDBG
                    xyprintf(5,19,"STUCK %f %f %f",coll.mat[3][0],coll.mat[3][1],coll.mat[3][2]);
                    xyprintf(5,20,"  PT2 %f %f %f",bot[0],bot[1],bot[2]);
                #endif

                return 1;
            }
        }
        if (1)
        {
            last_surf = coll;
            e->motion->last_surf_type = SURFTYPE_WALL;
            webCrawlRecordWallContact(e, coll.mat[1], coll.mat[3], bot);
        }
        subVec3(coll.mat[3],bot,dv);
        dv[1] = 0;
        d = rad + 0.05 - normalVec3(dv);
        scaleVec3(dv,d,dv);
        subVec3(bot,dv,bot);

        #if MDBG
        {
            DefTracker *tracker;

            tracker = coll.node;
            xyprintf(5,14+i,"%08x %s  %d SLIDE: %f %f %f (%f %f %f) amt %f",
                tracker,tracker->def->name,i,coll.mat[3][0],coll.mat[3][1],coll.mat[3][2],
                -dv[0],-dv[1],-dv[2],d);
        }
        #endif
        //copyVec3(dv,e->last_slide);
    }
    if (e->motion->vel[1] > 0)
        e->motion->vel[1] = 0;

    return 2;
}

void testcoll(U32 flags)
{
    int        ret;
    Vec3    top = {584.92419,2.5+0.25978798,380.55411};
    Vec3    bot = {584.92419,0.25978798,380.55411};
    CollInfo    coll;

    coll.tri_colls = 0;
    coll.coll_count = 0;
    coll.coll_max = 0;

    ret = collide(top,bot,&coll,DEFAULT_RADIUS,COLL_DISTFROMSTART | COLL_GATHERTRIS | flags);
    {
        int        i;

        if (coll.coll_count)
            printf("\n%dclosest   %f %f %f   %f\n",flags ? 1 : 0,coll.mat[3][0],coll.mat[3][1],coll.mat[3][2],coll.mat[1][1]);
        for(i=0;i<coll.coll_count;i++)
        {
            F32        d = distance3(coll.mat[3],coll.tri_colls[i].mat[3]), d2 = fabs(coll.mat[3][1] - coll.tri_colls[i].mat[3][1]);

            printf("%f %f %f    %f %f   %f\n",coll.tri_colls[i].mat[3][0],coll.tri_colls[i].mat[3][1],coll.tri_colls[i].mat[3][2],d,d2,coll.tri_colls[i].mat[1][1]);
        }
        if (0 && coll.mat[1][1] < 0.4)
        {
            coll.tri_colls = 0;
            coll.coll_count = 0;
            coll.coll_max = 0;
            ret = collide(top,bot,&coll,DEFAULT_RADIUS,COLL_DISTFROMSTART | COLL_GATHERTRIS);
            coll.tri_colls = 0;
            coll.coll_count = 0;
            coll.coll_max = 0;
            ret = collide(top,bot,&coll,DEFAULT_RADIUS,COLL_DISTFROMSTART | COLL_GATHERTRIS);
        }
    }
}

static int GroundHeight(const Vec3 pos,CollInfo *coll,Entity *e,SurfaceParams *surf)
{
    Vec3            top;
    Vec3            bot;
    int                ret;
    MotionState*    motion = e->motion;

#if 0
    testcoll(COLL_DISTFROMSTARTEXACT);
    testcoll(0);
#endif

    copyVec3(pos,top);
    copyVec3(pos,bot);
    top[1] += 2.5 - motion->vel[1] * e->timestep;
    bot[1] += motion->vel[1] * e->timestep - 0.5 * surf->gravity * SQR(e->timestep);
    coll->tri_colls = 0;
    coll->coll_count = 0;
    coll->coll_max = 0;
    ret = collide(top,bot,coll,DEFAULT_RADIUS,COLL_DISTFROMSTART);// | COLL_GATHERTRIS);
    if (ret)
    {
        last_surf = *coll;
        motion->last_surf_type = SURFTYPE_GROUND;
    }
    if (1 && coll->mat[1][1] < 0.7)
    {
        ret = collide(top,bot,coll,DEFAULT_RADIUS,COLL_DISTFROMSTART | COLL_DISTFROMSTARTEXACT);// | COLL_GATHERTRIS);
        if (ret)
        {
            last_surf = *coll;
        }
    }

#if 0 && CLIENT
    {
        int        i;

        if (coll->coll_count)
            printf("\nclosest   %f %f %f   %f\n",coll->mat[3][0],coll->mat[3][1],coll->mat[3][2],coll->mat[1][1]);
        for(i=0;i<coll->coll_count;i++)
        {
            F32        d = distance3(coll->mat[3],coll->tri_colls[i].mat[3]), d2 = fabs(coll->mat[3][1] - coll->tri_colls[i].mat[3][1]);

            if (1 || d2 < 0.2)
                printf("%f %f %f    %f %f   %f\n",coll->tri_colls[i].mat[3][0],coll->tri_colls[i].mat[3][1],coll->tri_colls[i].mat[3][2],d,d2,coll->tri_colls[i].mat[1][1]);
        }
        if (coll->mat[1][1] < 0.4)
        {
            coll->tri_colls = 0;
            coll->coll_count = 0;
            coll->coll_max = 0;
            ret = collide(top,bot,coll,DEFAULT_RADIUS,COLL_DISTFROMSTART | COLL_GATHERTRIS);
            coll->tri_colls = 0;
            coll->coll_count = 0;
            coll->coll_max = 0;
            ret = collide(top,bot,coll,DEFAULT_RADIUS,COLL_DISTFROMSTART | COLL_GATHERTRIS);
        }
    }
#endif
    if(coll->tri_colls)
    {
        free(coll->tri_colls);
    }
    if (!ret)
    {
        F32 minHeight = min(-2000.0f, scene_info.minHeight);
        
        coll->mat[3][1] = minHeight;
        
        if (bot[1] < minHeight){
            static CTri bottom_tri;
    
            coll->mat[1][0] = 0;
            coll->mat[1][1] = 1;
            coll->mat[1][2] = 0;

            bottom_tri.norm[0] = 0;
            bottom_tri.norm[1] = 1;
            bottom_tri.norm[2] = 0;
            
            coll->ctri = &bottom_tri;
            last_surf = *coll;
            motion->last_surf_type = SURFTYPE_GROUND;

            ret = 1;
        }
    }
    return ret;
}

static void reflectOffPlane(Vec3 incident,Vec3 plane_norm,Vec3 reflect)
{
    Vec3    N;
    Vec3    nI;
    Vec3    I = {0};
    F32        dot;
    F32        mag;

    copyVec3(incident,I);
    normalVec3(I);
    subVec3(zerovec3,I,nI);
    mag = lengthVec3(incident);

    copyVec3(plane_norm,N);

    dot = dotVec3(nI,N);
    scaleVec3(N,2*dot,N);
    addVec3(N,I,reflect);
    scaleVec3(reflect,mag,reflect);
}

static void CheckFeet(Entity *e,SurfaceParams *surf)
{
    F32                new_height;
    F32                plr_height;
    CollInfo        coll;
    CollInfo        savecoll;
    int                first = 1;
    Vec3            dv;
    int                hit;
    MotionState*    motion = e->motion;

    savecoll = last_surf;
retry:
    hit = GroundHeight(ENTPOS(e),&coll,e,surf);

    new_height = coll.mat[3][1];
    plr_height = ENTPOSY(e);

    if(hit)
    {
        motion->heightILastTouchedTheGround = ENTPOSY(e);
    }

    // Is the entity more than 0.05 feet above the ground?
    
    if (/*motion->flying ||*/ (new_height - plr_height < -0.3 * e->timestep))
    {
        if(motion->input.flying || (plr_height > motion->highest_height))
            motion->highest_height = plr_height;

        // It should be considered as "falling".
        
        //printf("falling!\n");        
        
          motion->falling = !motion->input.flying;
        zeroVec3(motion->surf_normal);
        last_surf = savecoll;

        // Don't have to do anything else if the entity is somewhere
        // in the air.
        return;
    }

    // The entity is less than 0.3 feet above the ground.  The entity
    // should be considered as "not falling."
    // If the entity thinks it's falling, it means it's time to make the entity
    // land.

    copyVec3(coll.mat[1],motion->surf_normal);

    if(!motion->input.flying && motion->falling && motion->surf_normal[1] > 0.3)
    {
        // Don't say the entity is falling anymore.
        
        //printf("not falling: %f\n", new_height - plr_height);
        
        motion->falling = 0;
        motion->jump_not_landed = 0;

        landed_on_ground = 1;
    }

    //t = -motion->vel[1] * e->timestep;

    if (new_height - plr_height > 1.5)
    {
        if (first)
        {
            Vec3 newpos;
            last_surf.ctri = 0;
            subVec3(ENTPOS(e),motion->last_pos,dv);
            scaleVec3(dv,0.5,dv);
            addVec3(motion->last_pos,dv,newpos);
            if (new_height - plr_height > 40)
                vecY(newpos) = new_height;
                //e->mat[3][1] -= 1.5;
            entUpdatePosInterpolated(e, newpos);
            first = 0;
            goto retry;
        }

#if MDBG
        xyprintf(5,12,"feet stuck: old height %f new height %f",plr_height,new_height);
#endif
        entUpdatePosInterpolated(e, motion->last_pos);

        return;
    }
    if (!motion->input.flying)
    {
        if (new_height - plr_height < 1.5)
        {
            Vec3 newpos;
            copyVec3(ENTPOS(e), newpos);
            vecY(newpos) = new_height;
            entUpdatePosInterpolated(e, newpos);
        }
    }
    else
    {
        Vec3 newpos;
        copyVec3(ENTPOS(e), newpos);
        vecY(newpos) = MAX(new_height, plr_height);
        entUpdatePosInterpolated(e, newpos);
    }
}

static int CheckHead(Entity *e,int ok_to_move)
{
    Vec3            top;
    Vec3            bot;
    int                ret;
    CollInfo        coll;
    F32                diff;
    MotionState*    motion = e->motion;

    copyVec3(ENTPOS(e),top);
    copyVec3(ENTPOS(e),bot);
    top[1] += 6.0;
    bot[1] += 1.5;
    ret = collide(bot,top,&coll,DEFAULT_RADIUS,COLL_DISTFROMSTART | COLL_CYLINDER);
    //if (0 && ret)
    //{
    //    last_surf = coll;
    //    last_surf_type = 11;
    //}
    if (ret)
    {
        if (!ok_to_move)
            return 1;
        diff = ENTPOSY(e) + 6 - coll.mat[3][1];
        if (diff > 1.5)
        {
            entUpdatePosInterpolated(e, motion->last_pos);
        }
        else
        {
            Vec3 newpos;
            copyVec3(ENTPOS(e), newpos);
            vecY(newpos) += diff;
            entUpdatePosInterpolated(e, newpos);
        }
        return 1;
    }
    return 0;
}

static void makeSteepSlopesSlippery(MotionState *motion,SurfaceParams *surf)
{
    F32        slope = 1 - motion->surf_normal[1];

    if (motion->on_poly_edge)
        return;
        
    // surf_normal[1]=0 means vertical
    #define SLOPE_START_SLIDE 0.3
    #define SLOPE_FULL_SLIDE 0.6
    if (slope >= SLOPE_START_SLIDE)
    {
        F32        ratio;

        ratio = (slope - SLOPE_START_SLIDE) / (SLOPE_FULL_SLIDE - SLOPE_START_SLIDE);
        if (ratio > 1)
            ratio = 1;
        if (!motion->input.no_slide_physics)
             surf->traction = (1-ratio)*surf->traction + ratio * 0.001;
        surf->friction = (1-ratio)*surf->friction + ratio * 0.001;
        //surf->bounce = (1-ratio)*surf->bounce + ratio * 0.6;
    }
}

#define SURFACE_GROUND    0
#define SURFACE_AIR        1
#define SURFACE_ICE        2

void entWorldGetSurface(Entity *e, SurfaceParams *surf)
{
    #define GRAVITY 0.065
    static SurfaceParams defaultSurfs[] =
    {
        { 1.00f,    0.45f,    0.01,    GRAVITY,    1.00f }, // default SURFACE_GROUND
        { 0.02f,    0.01f,    0,        GRAVITY,    1.00f }, // default SURFACE_AIR
    };
    
    if (e->motion->falling || e->motion->input.flying)
        *surf = defaultSurfs[SURFACE_AIR];
    else
        *surf = defaultSurfs[SURFACE_GROUND];

    if (!e->motion->falling && !e->motion->input.flying)
        makeSteepSlopesSlippery(e->motion,surf);
}

SurfaceParams *entWorldGetSurfaceModifier(Entity *e)
{
    static SurfaceParams surf;
    
    MotionState* motion = e->motion;
    int inAir = motion->falling || motion->input.flying;

    surf = motion->input.surf_mods[inAir];

    if(motion->jumping || motion->input.flying)
    {
        surf.gravity = 0;
    }
    else if(inAir)
    {
        surf.friction = 1.0f;

        if(!motion->falling)
        {
            surf.traction = 1.0f;
        }
    }

    return &surf;
}

void entWorldApplySurfMods(SurfaceParams *surf_mod,SurfaceParams *surf)
{
    surf->friction    *= surf_mod->friction;
    surf->traction    *= surf_mod->traction;
    surf->bounce    *= surf_mod->bounce;
    surf->gravity    *= surf_mod->gravity;
    surf->max_speed *= surf_mod->max_speed;
}                                                              

void entWorldApplyTextureOverrides( SurfaceParams *    surf, int lastSurfFlags )
{
    if( lastSurfFlags && surf )  
    {
        if( lastSurfFlags & COLL_SURFACESLICK )
        {
            surf->friction = 0.01;
            surf->traction = 0.2;
        }
        if( lastSurfFlags & COLL_SURFACEBOUNCY )
        {
            surf->bounce = 0.5;
        }
        if( lastSurfFlags & COLL_SURFACEICY )
        {
            surf->friction = 0.0;
            surf->traction = 0.05;
        }
    }
}

static void checkMaxHeight(Entity *e, SurfaceParams* surf)
{
    F32        max_fly_height = scene_info.maxHeight;

    if(ENTPOSY(e) > max_fly_height)
    {
        Vec3 newpos;

        copyVec3(ENTPOS(e), newpos);
        vecY(newpos) = max_fly_height;
        entUpdatePosInterpolated(e, newpos);
        
        if (e->motion->vel[1] > 0)
            e->motion->vel[1] = 0;

        if(e->motion->jumping)
        {
            e->motion->jumping = 0; // hit max world height, turn gravity on.
            e->motion->falling = 1;
        }
    }
}

void entWorldMoveMe(Entity* e, SurfaceParams* surf)
{
    MotionState*    motion = e->motion;
    int                try_count = 0;
    Vec3            top;
    Vec3            bot;
    Vec3            test;
    int                ret;
    
    checkMaxHeight(e, surf);

    retry:
    try_count++;
    copyVec3(motion->last_pos, top);
    copyVec3(ENTPOS(e), bot);
    ret = SlideWall(e, top, bot, top);
    if(ret)
    {
        motion->stuck_head = STUCK_COMPLETELY;
        entUpdatePosInterpolated(e, motion->last_pos);
    }
    else
    {
        motion->stuck_head = 0;
        entUpdatePosInterpolated(e, top);
    }

    copyVec3(ENTPOS(e), test);

    CheckFeet(e, surf);
    
    if (fabs(ENTPOSY(e) - test[1]) > 0.00001)
    {
        if (CheckHead(e,0))
        {
            if (!surf->gravity && try_count == 1)
            {
                Vec3 newpos;

                copyVec3(ENTPOS(e), newpos);
                vecY(newpos) = motion->last_pos[1];
                entUpdatePosInterpolated(e, newpos);
                goto retry;
            }
            else
            {
                 if (motion->vel[1] > 0)
                 {
                    motion->vel[1] = 0;
                }
                motion->stuck_head = STUCK_SLIDE;
            }
            entUpdatePosInterpolated(e, motion->last_pos);
        }
    }
    else if (motion->stuck_head == STUCK_COMPLETELY)
    {
        if (CheckHead(e,1))
        {
            copyVec3(ENTPOS(e),test);

            CheckFeet(e,surf);

            if (fabs(ENTPOSY(e) - test[1]) > 0.00001)
            {
                entUpdatePosInterpolated(e, motion->last_pos);
            }
        }
    }
}

void entWorldCollide(Entity* e, const Mat3 control_mat)
{
    Vec3            gravity_vec = {0};
    Vec3            last_slope;
    MotionState*    motion = e->motion;
    SurfaceParams    surf;
    SurfaceParams*    surf_mod;
    F32                max_speed;
    F32                friction_scale;
    F32                traction_scale;
    int                web_crawl_owns_velocity;

    PERFINFO_AUTO_START("entWorldCollideTop", 1);

    copyVec3(motion->surf_normal, last_slope);

    web_crawl_owns_velocity = webCrawlApplyController(e);

    entWorldGetSurface(e, &surf);
    surf_mod = entWorldGetSurfaceModifier(e);
    entWorldApplySurfMods(surf_mod, &surf);
    entWorldApplyTextureOverrides(&surf, motion->lastSurfFlags);
    
    if(surf.friction > 1)
        surf.friction = 1;

    if(surf.traction > 1)
        surf.traction = 1;
        
    // Check for forced low-traction.
    
    if(motion->low_traction_steps_remaining > 0)
    {
        if(surf.traction > 0.1)
        {
            surf.traction = 0.1;
        }

        if(surf.friction > 0.3)
        {
            surf.friction = 0.3;
        }
    }

    //TL! entWorldCollide Apply per frame traction loss and gravity gain
    if( motion->hitStumbleTractionLoss )      
    {
        F32 t = 1.0 - motion->hitStumbleTractionLoss;
        t = MINMAX( t, 0.0 , 1.0 );
        surf.traction *= t;

        if( !surf.gravity ) 
        {
            surf.gravity = 0.02 * motion->hitStumbleTractionLoss ;
        } 
    }
    if( !motion->falling &&  motion->hit_stumble_kill_velocity_on_impact )        
    {
        //FLYING OR RUNNING 
        scaleVec3( motion->vel, 0.2 * motion->hitStumbleTractionLoss, motion->vel );//TO DO base on strength of hit
        //zeroVec3( motion->vel );
        motion->hit_stumble_kill_velocity_on_impact = 0; //TO DO does this slow me down
    }
    


    // Integrate friction and traction over time.
    
    friction_scale = 1 - powf(1 - surf.friction, e->timestep);
    traction_scale = 1 - powf(1 - surf.traction, e->timestep);

    // Get the max speed.

    max_speed = surf.max_speed;
    
    if(*(int*)&motion->input.max_speed_scale)
    {
        max_speed *= motion->input.max_speed_scale;
    }

    // Get the gravity vector for this timestep.

    if(surf.gravity)
    {
        F32 gravity = 1.0 * (motion->vel[1] <= 0 ? surf.gravity * 1.5 : surf.gravity);
        F32    fall = 0.5 * gravity * SQR(e->timestep);

        //e->mat[3][1] -= fall;
        gravity_vec[1] = -gravity * e->timestep;
    }
    PERFINFO_AUTO_STOP();
    if(web_crawl_owns_velocity)
    {
        /* Crawl owns motion->vel for this tick; skip stock gravity/friction. */
    }
    else if(motion->on_surf)
    {
        Vec3    dv;
        Vec3    inp_vel = {0,0,0};
        F32*    surf_norm = motion->last_surf_normal;
        
        PERFINFO_AUTO_START("entWorldCollide if(motion->on_surf", 1);
        if (motion->last_surf_type == SURFTYPE_GROUND && surf_norm[1] < last_slope[1] && motion->vel[1] <= 0)
        {
            surf_norm = last_slope;
        }
        
        if(dotVec3(motion->prev_surf_normal, surf_norm) < -0.7)
        {
            // Make me able to jump out of the wedge.
            
            motion->jump_held = 1;
            motion->falling = 0;
            
            traction_scale = 1;
        }
        
        copyVec3(surf_norm, motion->prev_surf_normal);

        // Calculate the gravity component vector parallel to the surface.
        
        reflectOffPlane(gravity_vec, surf_norm, dv);
        addVec3(dv, gravity_vec, gravity_vec);
        
        if(    !motion->input.flying &&
            surf_norm[1] > 0.0f)
        {
            F32 scale;
            
            // Vertically project the input velocity onto the surface plane,
            //   scaled by the surface normal y-component.
            
            inp_vel[0] = motion->input.vel[0] * surf_norm[1];
            inp_vel[1] = -(surf_norm[0] * motion->input.vel[0] + surf_norm[2] * motion->input.vel[2]);
            inp_vel[2] = motion->input.vel[2] * surf_norm[1];
            
            scale = lengthVec3XZ(motion->input.vel) * traction_scale;
            
            if(inp_vel[1] < 0.0f)
            {
                F32 inp_vel_len = lengthVec3(inp_vel);
                F32 input_vel_len = lengthVec3XZ(motion->input.vel);
                
                // Going downhill, so scale the inp_vel to the length of the input.
                
                scale *= input_vel_len / inp_vel_len;

                if(    !vec3IsZero(inp_vel) &&
                    !vec3IsZero(gravity_vec))
                {
                    // Speed up when going downhill.
                    
                    F32 diff = 1 - surf_norm[1];
                    
                    diff *= dotVec3(gravity_vec, inp_vel) / (lengthVec3(gravity_vec) * inp_vel_len);
                    
                    max_speed /= max(0.1, 1 - diff);
                }
            }

            scaleVec3(inp_vel, scale, inp_vel);
        }
        else
        {
            F32 scale = lengthVec3(motion->input.vel) * traction_scale;

            scaleVec3(motion->input.vel, scale, inp_vel);
        }

        scaleVec3(gravity_vec, 0.5 * min(1, 4 * (1 - traction_scale)), gravity_vec);
        
        // Project the current velocity onto the surface, and multiply by bounce.
        
        {
            Mat3 surf_mat;
            Mat3 surf_mat_inv;
            Vec3 xvel;
            Vec3 dvx;
            Vec3 up = {0,0,1};

            // Create matrix with z pointing out of the surface and x-vector on the xz-plane.
            
            camLookAt(surf_norm, surf_mat);
            
            // Create the transpose to convert from world-space to surf_mat-space.
            
            transposeMat3Copy(surf_mat, surf_mat_inv);
            
            // Get the velocity in surf_mat space.
            
            mulVecMat3(motion->vel, surf_mat_inv, xvel);
            
            if(xvel[2] <= 0)
            {
                // Reflect the velocity off the surface.
                
                reflectOffPlane(xvel, up, dvx);
                
                // Multiply by the bounce.
                dvx[2] *= surf.bounce; 

                //If friction is low, don't lose the velocity.  I doubt (1.0 - friction) is the right rate of loss, but
                //it will do for now...
                if( 0 && !nearSameVec3( unitmat[3], motion->vel ) )
                {
                    F32 oldVel, newVel, lostVel, velToRestore, scaleToRestore;
 
                    oldVel = lengthVec3( motion->vel );
                    newVel = lengthVec3( dvx );
                    lostVel = oldVel - newVel;
                    velToRestore = lostVel * (1.0 - surf.friction);
                    scaleToRestore = (newVel + velToRestore) / newVel;
                    scaleVec3( dvx, scaleToRestore, dvx );
                }

                // Convert the reflected velocity back to world space.
                
                mulVecMat3(dvx, surf_mat, dv);
            }
            else
            {
                copyVec3(motion->vel, dv);
            }
        }
        
        // Calculate the friction and traction.
        
        {
            S32        no_input = vec3IsZero(inp_vel);
            F32        scale = no_input ? 0 : dotVec3(inp_vel, dv);
            Vec3    friction_dir;
            
            if(!motion->falling && !motion->input.flying)
            {
                motion->lastGroundSurfFlags = motion->lastSurfFlags;
                motion->was_on_surf = 1;
            }

            if(!no_input && scale >= 0)
            {
                F32        inp_vel_mag_SQR = lengthVec3Squared(inp_vel);
                Vec3    projected;
                Vec3    perp_friction_dir;
                int        clamp_max;
                F32        clamp_max_value;
                F32        perp_friction_scale;
                
                // Get the perpedicular friction direction.
                
                perp_friction_scale = friction_scale + traction_scale * 0.5;
                if(perp_friction_scale > 1)
                    perp_friction_scale = 1;
                
                scaleVec3(dv, -perp_friction_scale, friction_dir);
                scale = dotVec3(inp_vel, friction_dir) / inp_vel_mag_SQR;
                scaleVec3(inp_vel, scale, projected);
                subVec3(friction_dir, projected, perp_friction_dir);
                addVec3(dv, perp_friction_dir, dv);
                
                // Get the projected current velocity after friction was applied.
                
                scale = dotVec3(inp_vel, dv) / inp_vel_mag_SQR;
                scaleVec3(inp_vel, scale, projected);
                
                // If projected current velocity is faster than max_speed, slow me down.
                
                if(lengthVec3Squared(projected) <= SQR(max_speed))
                {
                    clamp_max = 1;
                }
                else
                {
                    clamp_max = 0;
                    clamp_max_value = lengthVec3(projected);
                }
                
                // Add in the input velocity.
                
                addVec3(dv, inp_vel, dv);

                // Get the projected current velocity again.
                
                scale = dotVec3(inp_vel, dv) / inp_vel_mag_SQR;
                scaleVec3(inp_vel, scale, projected);
                
                scale = lengthVec3Squared(projected);
                
                if(scale > SQR(max_speed))
                {
                    // Remove the current projected velocity.
                    
                    subVec3(dv, projected, dv);
                    
                    scale = sqrt(scale);

                    if(clamp_max)
                    {
                        // Wasn't going fast enough before, so clamp speed.
                        
                        scale = max_speed / scale;
                    }
                    else
                    {
                        // Reduce the current overflow-velocity by the friction coefficient.
                        
                        scale = ((clamp_max_value - max_speed) * (1 - friction_scale) + max_speed) / scale;
                    }

                    // Add the projected velocity back in.
                    
                    scaleVec3(projected, scale, projected);
                    addVec3(dv, projected, dv);
                }
            }
            else
            {
                if(!no_input)
                {
                    friction_scale += traction_scale * 0.5;
                    if(friction_scale > 1)
                        friction_scale = 1;
                }
                
                scaleVec3(dv, -friction_scale, friction_dir);
                addVec3(dv, inp_vel, dv);
                addVec3(dv, friction_dir, dv);
            }
            
            addVec3(dv, gravity_vec, dv);
        }

        //#if CLIENT
        //{
        //    // Draw some stuff.
        //    
        //    Vec3 target1;
        //    addVec3(motion->last_pos, gravity_vec, target1);
        //    entDebugAddLine(motion->last_pos, 0xffffffff, target1, 0xffff7700);
        //    addVec3(motion->last_pos, inp_vel, target1);
        //    entDebugAddLine(motion->last_pos, 0xffffffff, target1, 0xff0077ff);
        //    addVec3(motion->last_pos, motion->inp_vel, target1);
        //    entDebugAddLine(motion->last_pos, 0xffffffff, target1, 0xff00ff77);
        //    addVec3(motion->last_pos, motion->vel, target1);
        //    entDebugAddLine(motion->last_pos, 0xffffffff, target1, 0xffff0000);
        //}
        //#endif

        copyVec3(dv, motion->vel);

        //#if MDBG
        //    xyprintf(5,17,"vel %f %f %f",motion->vel[0],motion->vel[1],motion->vel[2]);
        //#endif

        //if(0){
        //    int moved = fabs(e->mat[3][1] - motion->last_pos[1]) > 0.00001;

        //    if (last_slope[1] == surf_norm[1] && (1-last_slope[1]) >= SLOPE_START_SLIDE && !moved && motion->vel[1] < -0.001)
        //    {
        //        motion->on_poly_edge = 1;
        //    }
        //    else if ((1-last_slope[1]) < SLOPE_START_SLIDE)
        //    {
        //        motion->on_poly_edge = 0;
        //    }
        //}
        PERFINFO_AUTO_STOP();
    }
    else
    {
        #define IGNORED_SURFACES (COLL_SURFACESLICK | COLL_SURFACEBOUNCY | COLL_SURFACEICY)
        
        static int debugBranchBitfield; // temporary debugging information to track down the crazy velocity bug.
        Vec3    dv;
        F32        scale;
        Vec3    friction_dir;
        F32        dv1;
        Vec3    inp_vel = {0,0,0};

        debugBranchBitfield = 0;

        PERFINFO_AUTO_START("entWorldCollide else", 1);
        if(motion->was_on_surf && !(motion->lastGroundSurfFlags & IGNORED_SURFACES) && motion->vel[1] > 0)
        {
            debugBranchBitfield |= 1 << 0;

            if (!motion->jump_held)
            {
                debugBranchBitfield |= 1 << 1;
                motion->vel[1] = 0;
            }
            else if (motion->vel[1] > 1)
            {
                debugBranchBitfield |= 1 << 2;
                motion->vel[1] = 1;
            }
        }
        
        motion->was_on_surf = 0;

        zeroVec3(motion->prev_surf_normal);
        
        motion->on_poly_edge = 0;
        
        if(motion->input.flying)
        {
            debugBranchBitfield |= 1 << 3;
            scale = lengthVec3(motion->input.vel) * traction_scale;
            scaleVec3(motion->input.vel, scale, inp_vel);

            copyVec3(motion->vel, dv);
            dv1 = 0;
        }
        else
        {
            debugBranchBitfield |= 1 << 4;
            scale = lengthVec3XZ(motion->input.vel) * traction_scale;
            scaleVec3(motion->input.vel, scale, inp_vel);

            dv1 = motion->vel[1];

            dv[0] = motion->vel[0];
            dv[1] = 0;
            dv[2] = motion->vel[2];
            
            inp_vel[1] = 0;
        }
        
        // Calculate the friction and traction.
        
        {
            S32        no_input = vec3IsZero(inp_vel);
            
            scale = no_input ? 0 : dotVec3(inp_vel, dv);
        
            if(!no_input && scale >= 0)
            {
                F32        inp_vel_mag_SQR = lengthVec3Squared(inp_vel);
                Vec3    projected;
                Vec3    perp_friction_dir;
                int        clamp_max;
                F32        clamp_max_value;
                F32        perp_friction_scale;
                
                debugBranchBitfield |= 1 << 5;

                // Get the perpedicular friction direction.
                
                perp_friction_scale = friction_scale + traction_scale * 0.5;
                if (perp_friction_scale > 1)
                {
                    debugBranchBitfield |= 1 << 6;
                    perp_friction_scale = 1;
                }

                scaleVec3(dv, -perp_friction_scale, friction_dir);
                scale = dotVec3(inp_vel, friction_dir) / inp_vel_mag_SQR;
                scaleVec3(inp_vel, scale, projected);
                subVec3(friction_dir, projected, perp_friction_dir);
                addVec3(dv, perp_friction_dir, dv);
                
                // Get the projected current velocity after friction was applied.
                
                scale = dotVec3(inp_vel, dv) / inp_vel_mag_SQR;
                scaleVec3(inp_vel, scale, projected);
                
                // If projected current velocity is faster than max_speed, slow me down.
                
                if(lengthVec3Squared(projected) <= SQR(max_speed))
                {
                    debugBranchBitfield |= 1 << 7;
                    clamp_max = 1;
                }
                else
                {
                    debugBranchBitfield |= 1 << 8;
                    clamp_max = 0;
                    clamp_max_value = lengthVec3(projected);
                }
                
                // Add in the input velocity.
                
                addVec3(dv, inp_vel, dv);

                // Get the projected current velocity again.
                
                scale = dotVec3(inp_vel, dv) / inp_vel_mag_SQR;
                scaleVec3(inp_vel, scale, projected);
                
                scale = lengthVec3Squared(projected);
                
                if(scale > SQR(max_speed))
                {
                    // Remove the current projected velocity.
                    
                    debugBranchBitfield |= 1 << 9;

                    subVec3(dv, projected, dv);
                    
                    scale = sqrt(scale);

                    if(clamp_max)
                    {
                        // Wasn't going fast enough before, so clamp speed.
                        
                        debugBranchBitfield |= 1 << 10;
                        scale = max_speed / scale;
                    }
                    else
                    {
                        // Reduce the current overflow-velocity by the friction coefficient.
                        
                        debugBranchBitfield |= 1 << 11;
                        scale = ((clamp_max_value - max_speed) * (1 - friction_scale) + max_speed) / scale;
                    }

                    // Add the projected velocity back in.
                    
                    scaleVec3(projected, scale, projected);
                    addVec3(dv, projected, dv);
                }
            }
            else
            {
                debugBranchBitfield |= 1 << 12;
                if(!no_input)
                {
                    debugBranchBitfield |= 1 << 13;
                    friction_scale += traction_scale * 0.5;
                    if (friction_scale > 1)
                    {
                        debugBranchBitfield |= 1 << 14;
                        friction_scale = 1;
                    }
                }
                
                scaleVec3(dv, -friction_scale, friction_dir);
                addVec3(dv, inp_vel, dv);
                addVec3(dv, friction_dir, dv);
            }
        }
                
        // Add in the gravity vector.

        addVec3(dv, gravity_vec, dv);
        
        // Copy back to vel.

        dv[1] += dv1;
        copyVec3(dv, motion->vel);
        PERFINFO_AUTO_STOP();
    }
    PERFINFO_AUTO_START("entWorldCollideBottom", 1);
    //printf("%f %f speed %f slope %f  type %d  d %f\n",e->mat[3][0],e->mat[3][1],lengthVec3(motion->vel),last_surf.ctri ? last_surf.mat[1][1] : 2,last_surf_type,distance3(e->motion->last_pos,e->mat[3]));
    //printf("gravity_vec[1]: %f: %f + %f = %f\n", e->timestep, gravity_vec[1], motion->vel[1], motion->vel[1] + gravity_vec[1]);

    #if CLIENT
    {
        Vec3 target1;
        addVec3(motion->last_pos, gravity_vec, target1);
        //entDebugAddLine(motion->last_pos, 0xffffffff, target1, 0xff0077ff);
    }
    #endif

    if(motion->jumping && !web_crawl_owns_velocity)
    {
        motion->vel[1] = motion->input.vel[1];
    }

    entWorldWebSwingApplyConstraint(e);

    // Do entity collisions.

    {
        Vec3 add_vel, newpos;
        scaleVec3(motion->vel, e->timestep, add_vel);
        addVec3(motion->last_pos, add_vel, newpos);
        entUpdatePosInterpolated(e, newpos);
    }

    PERFINFO_AUTO_START("checkEntColl",1);
    checkEntColl(e, 1, control_mat);
    PERFINFO_AUTO_STOP();
    //printf("moving: %f\t(%1.3f\t%1.3f\t%1.3f)\n", lengthVec3(add_vel), vecParamsXYZ(add_vel));
    PERFINFO_AUTO_START("entWorldMoveMe",1);
    entWorldMoveMe(e, &surf);
    PERFINFO_AUTO_STOP();
    if(!motion->input.flying && motion->vel[1] < 0 && sameVec3(ENTPOS(e), motion->last_pos) && motion->last_surf_normal[1] < 0.5)
    {
        motion->jump_held = 0;
        setVec3(motion->surf_normal, 0, 1, 0);
        motion->on_surf = 1;
        motion->falling = 0;
        motion->vel[1] = 0;
    }
    else
    {
        motion->on_surf = last_surf.ctri ? 1 : 0;
        copyVec3(last_surf.mat[1], motion->last_surf_normal);
    }
        
    copyVec3(ENTPOS(e), motion->last_pos);

    //if(!motion->jump_not_landed && motion->vel[1] > 0)
    //    motion->vel[1] = 0;

    if(last_surf.ctri)
    {
        motion->lastSurfFlags = last_surf.ctri->flags;
        
        //printf(    "tri: (%f,%f,%f), (%f,%f), (%f,%f)\n",
        //        vecParamsXYZ(last_surf.ctri->V1),
        //        last_surf.ctri->v2[0], last_surf.ctri->v2[1], 
        //        last_surf.ctri->v3[0], last_surf.ctri->v3[1]);
    }
    else
    {
        motion->lastSurfFlags = 0;
    }

    #if MDBG
        xyprintf(4,18,"fall %d gravity_vec %f %f %f",e->motion->falling,gravity_vec[0],gravity_vec[1],gravity_vec[2]);
    #endif

    PERFINFO_AUTO_STOP();
}
