#include "entworldcoll.h"
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
#define WEB_MIN_ANCHOR_HEIGHT  6.0f
#define WEB_MIN_ROPE_LENGTH    8.0f
#define WEB_MAX_ROPE_LENGTH    150.0f
#define WEB_ANCHOR_START_HEIGHT 2.0f
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
#define WEB_CHAIN_PREVIOUS_ANCHOR_EXCLUSION 10.0f
#define WEB_ANCHOR_VERTICALITY_WEIGHT   12.0f
#define WEB_ANCHOR_LATERAL_PENALTY      18.0f
#define WEB_LATERAL_REPULSION_ACCEL      0.035f
#define WEB_LATERAL_REPULSION_MIN_DIST   6.0f
#define WEB_LATERAL_REPULSION_FULL_DIST 24.0f

#if SERVER
#define WEB_SWING_LOG_SIDE "SERVER"
#else
#define WEB_SWING_LOG_SIDE "CLIENT"
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
    int probe_count;
    int collision_ray_hits;
    int height_rejects;
    int distance_rejects;
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
    int previous_anchor_rejects;
    Vec3 selected_anchor;
    F32 selected_rope_length;
} WebSwingAnchorSearchStats;

static int webSwingFindAnchor(Entity *e, Vec3 anchor, WebSwingAnchorSearchStats *stats)
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
        if(e->motion->web_swing_chain_reacquire &&
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

        if(distance < WEB_MIN_ROPE_LENGTH)
            ++stats->distance_rejects;
        if(height < WEB_MIN_ANCHOR_HEIGHT)
            ++stats->height_rejects;
        if(distance < WEB_MIN_ROPE_LENGTH || height < WEB_MIN_ANCHOR_HEIGHT)
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

static void webSwingLogAttachAttempt(Entity *e, const WebSwingAnchorSearchStats *stats)
{
    MotionState *motion = e->motion;
    Vec3 forward;
    int grounded = !motion->falling && !motion->jumping;

    copyVec3(ENTMAT(e)[2], forward);
    if(stats->selected)
    {
        filelog_printf("webswing.log",
                        "WEB_SWING %s attach_attempt web_swing_enabled=%d up=%.3f grounded=%d falling=%d jumping=%d pos=(%.2f %.2f %.2f) forward=(%.3f %.3f %.3f) intent_source=%s intent=(%.3f %.3f %.3f) horizontal_input_magnitude=%.3f meaningful_momentum=%d momentum=(%.3f %.3f %.3f) intent_momentum_alignment=%.3f travel=(%.3f %.3f %.3f) travel_right=(%.3f %.3f %.3f) entity_right=(%.3f %.3f %.3f) momentum_basis=%d facing_travel_dot=%.3f probes=%d ray_hits=%d height_rejects=%d distance_rejects=%d previous_anchor_rejects=%d selected=1 fallback=%d selected_intent_alignment=%.3f selected_momentum_alignment=%.3f selected_forward_alignment=%.3f verticality=%.3f lateral_alignment=%.3f anchor=(%.2f %.2f %.2f) rope=%.2f\n",
                       WEB_SWING_LOG_SIDE,
                       motion->input.web_swing_enabled,
                       motion->input.vel[1],
                       grounded,
                       motion->falling,
                       motion->jumping,
                       vecParamsXYZ(ENTPOS(e)),
                       vecParamsXYZ(forward),
                       webSwingIntentSourceName(stats->intent_source),
                       vecParamsXYZ(stats->intent),
                       stats->horizontal_input_magnitude,
                       stats->meaningful_momentum,
                       vecParamsXYZ(stats->momentum),
                       stats->intent_momentum_alignment,
                       vecParamsXYZ(stats->travel),
                       vecParamsXYZ(stats->travel_right),
                       vecParamsXYZ(stats->entity_right),
                       stats->momentum_basis,
                       stats->facing_travel_dot,
                       stats->probe_count,
                       stats->collision_ray_hits,
                       stats->height_rejects,
                       stats->distance_rejects,
                       stats->previous_anchor_rejects,
                       stats->used_fallback,
                       stats->selected_intent_alignment,
                       stats->selected_momentum_alignment,
                       stats->selected_forward_alignment,
                       stats->selected_verticality,
                       stats->selected_lateral_alignment,
                       vecParamsXYZ(stats->selected_anchor),
                       stats->selected_rope_length);
    }
    else
    {
        filelog_printf("webswing.log",
                        "WEB_SWING %s attach_attempt web_swing_enabled=%d up=%.3f grounded=%d falling=%d jumping=%d pos=(%.2f %.2f %.2f) forward=(%.3f %.3f %.3f) intent_source=%s intent=(%.3f %.3f %.3f) horizontal_input_magnitude=%.3f meaningful_momentum=%d momentum=(%.3f %.3f %.3f) intent_momentum_alignment=%.3f travel=(%.3f %.3f %.3f) travel_right=(%.3f %.3f %.3f) entity_right=(%.3f %.3f %.3f) momentum_basis=%d facing_travel_dot=%.3f probes=%d ray_hits=%d height_rejects=%d distance_rejects=%d previous_anchor_rejects=%d selected=0 fallback=0 selected_intent_alignment=0.000 selected_momentum_alignment=0.000 selected_forward_alignment=0.000 verticality=0.000 lateral_alignment=0.000\n",
                       WEB_SWING_LOG_SIDE,
                       motion->input.web_swing_enabled,
                       motion->input.vel[1],
                       grounded,
                       motion->falling,
                       motion->jumping,
                       vecParamsXYZ(ENTPOS(e)),
                       vecParamsXYZ(forward),
                       webSwingIntentSourceName(stats->intent_source),
                       vecParamsXYZ(stats->intent),
                       stats->horizontal_input_magnitude,
                       stats->meaningful_momentum,
                       vecParamsXYZ(stats->momentum),
                       stats->intent_momentum_alignment,
                       vecParamsXYZ(stats->travel),
                       vecParamsXYZ(stats->travel_right),
                       vecParamsXYZ(stats->entity_right),
                       stats->momentum_basis,
                       stats->facing_travel_dot,
                       stats->probe_count,
                       stats->collision_ray_hits,
                       stats->height_rejects,
                       stats->distance_rejects,
                       stats->previous_anchor_rejects);
    }
}

static void webSwingLogChainArm(Entity *e, F32 arc_height, F32 forward_speed)
{
    MotionState *motion = e->motion;

    printf("WEB_SWING %s chain_arm anchor=(%.2f %.2f %.2f) pos=(%.2f %.2f %.2f) arc_height=%.3f velocity=(%.3f %.3f %.3f) forward_speed=%.3f\n",
           WEB_SWING_LOG_SIDE,
           vecParamsXYZ(motion->web_swing_anchor),
           vecParamsXYZ(ENTPOS(e)),
           arc_height,
           vecParamsXYZ(motion->vel),
           forward_speed);
    filelog_printf("webswing.log",
                   "WEB_SWING %s chain_arm anchor=(%.2f %.2f %.2f) pos=(%.2f %.2f %.2f) arc_height=%.3f velocity=(%.3f %.3f %.3f) forward_speed=%.3f\n",
                   WEB_SWING_LOG_SIDE,
                   vecParamsXYZ(motion->web_swing_anchor),
                   vecParamsXYZ(ENTPOS(e)),
                   arc_height,
                   vecParamsXYZ(motion->vel),
                   forward_speed);
}

static void webSwingLogAutoChainRelease(Entity *e, F32 forward_speed)
{
    MotionState *motion = e->motion;
    F32 speed = lengthVec3(motion->vel);

    printf("WEB_SWING %s auto_chain_release previous_anchor=(%.2f %.2f %.2f) pos=(%.2f %.2f %.2f) velocity=(%.3f %.3f %.3f) speed=%.3f forward_speed=%.3f\n",
           WEB_SWING_LOG_SIDE,
           vecParamsXYZ(motion->web_swing_anchor),
           vecParamsXYZ(ENTPOS(e)),
           vecParamsXYZ(motion->vel),
           speed,
           forward_speed);
    filelog_printf("webswing.log",
                   "WEB_SWING %s auto_chain_release previous_anchor=(%.2f %.2f %.2f) pos=(%.2f %.2f %.2f) velocity=(%.3f %.3f %.3f) speed=%.3f forward_speed=%.3f\n",
                   WEB_SWING_LOG_SIDE,
                   vecParamsXYZ(motion->web_swing_anchor),
                   vecParamsXYZ(ENTPOS(e)),
                   vecParamsXYZ(motion->vel),
                   speed,
                   forward_speed);
}

static void webSwingLogChainAttach(Entity *e, const Vec3 previous_anchor, const Vec3 incoming_velocity)
{
    MotionState *motion = e->motion;
    F32 anchor_advance = distance3(previous_anchor, motion->web_swing_anchor);

    printf("WEB_SWING %s chain_attach previous_anchor=(%.2f %.2f %.2f) new_anchor=(%.2f %.2f %.2f) anchor_advance=%.2f incoming_velocity=(%.3f %.3f %.3f) outgoing_velocity=(%.3f %.3f %.3f) catch_suppressed=1\n",
           WEB_SWING_LOG_SIDE,
           vecParamsXYZ(previous_anchor),
           vecParamsXYZ(motion->web_swing_anchor),
           anchor_advance,
           vecParamsXYZ(incoming_velocity),
           vecParamsXYZ(motion->vel));
    filelog_printf("webswing.log",
                   "WEB_SWING %s chain_attach previous_anchor=(%.2f %.2f %.2f) new_anchor=(%.2f %.2f %.2f) anchor_advance=%.2f incoming_velocity=(%.3f %.3f %.3f) outgoing_velocity=(%.3f %.3f %.3f) catch_suppressed=1\n",
                   WEB_SWING_LOG_SIDE,
                   vecParamsXYZ(previous_anchor),
                   vecParamsXYZ(motion->web_swing_anchor),
                   anchor_advance,
                   vecParamsXYZ(incoming_velocity),
                   vecParamsXYZ(motion->vel));
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

void entWorldWebSwingUpdateAttachment(Entity *e, int web_swing_test_no_attach)
{
    MotionState *motion = e->motion;
    int held = motion->input.web_swing_enabled && motion->input.vel[1] > 0.001f;

    if(!held)
    {
        if(motion->web_swing_attached)
        {
            printf("WEB_SWING %s detach speed=%.3f anchor=(%.2f %.2f %.2f) pos=(%.2f %.2f %.2f) input=(%.2f %.2f %.2f)\n",
                   WEB_SWING_LOG_SIDE,
                   lengthVec3(motion->vel),
                   vecParamsXYZ(motion->web_swing_anchor),
                   vecParamsXYZ(ENTPOS(e)),
                   vecParamsXYZ(motion->input.vel));
            filelog_printf("webswing.log", "WEB_SWING %s detach speed=%.3f anchor=(%.2f %.2f %.2f) pos=(%.2f %.2f %.2f) input=(%.2f %.2f %.2f)\n",
                           WEB_SWING_LOG_SIDE,
                           lengthVec3(motion->vel),
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
        motion->web_swing_attached = 0;
        motion->web_swing_attach_catch_pending = 0;
        motion->web_swing_attach_grounded = 0;
        motion->web_swing_attach_falling = 0;
        motion->web_swing_attach_jumping = 0;
        motion->web_swing_diag_latched = 0;
        motion->web_swing_state_diag_latched = 0;
        motion->web_swing_chain_armed = 0;
        motion->web_swing_chain_reacquire = 0;
        zeroVec3(motion->web_swing_previous_anchor);
        motion->web_swing_log_tick = 0;
        webSwingResetConstraintMetrics(motion);
        return;
    }

    if(!motion->web_swing_attached && !motion->web_swing_state_diag_latched)
    {
        motion->web_swing_state_diag_latched = 1;
        filelog_printf("webswing.log", "WEB_SWING %s state web_swing_enabled=%d up=%.3f falling=%d jumping=%d flying=%d on_surf=%d pos=(%.2f %.2f %.2f)\n",
                       WEB_SWING_LOG_SIDE,
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
        Vec3 previous_anchor;
        Vec3 incoming_velocity;
        F32 rope_length;
        WebSwingAnchorSearchStats search_stats;
        int chain_attach;
        int found_anchor;

        chain_attach = motion->web_swing_chain_reacquire;
        if(chain_attach)
        {
            copyVec3(motion->web_swing_previous_anchor, previous_anchor);
            copyVec3(motion->vel, incoming_velocity);
        }

        found_anchor = webSwingFindAnchor(e, anchor, &search_stats);
        if(!motion->web_swing_diag_latched)
        {
            motion->web_swing_diag_latched = 1;
            webSwingLogAttachAttempt(e, &search_stats);
        }

        if(found_anchor)
        {
            motion->web_swing_attached = 1;
            copyVec3(anchor, motion->web_swing_anchor);
            rope_length = distance3(ENTPOS(e), anchor);
            motion->web_swing_rope_length = MINMAX(rope_length, WEB_MIN_ROPE_LENGTH, WEB_MAX_ROPE_LENGTH);
            motion->web_swing_log_tick = 0;
            webSwingResetConstraintMetrics(motion);
            motion->web_swing_attach_grounded = !motion->falling && !motion->jumping;
            motion->web_swing_attach_falling = motion->falling;
            motion->web_swing_attach_jumping = motion->jumping;
            motion->web_swing_attach_catch_pending = !chain_attach;
            motion->web_swing_chain_armed = 0;
            if(chain_attach)
                motion->web_swing_chain_reacquire = 0;
            motion->jumping = 0;
            motion->falling = 1;

            if(chain_attach)
                webSwingLogChainAttach(e, previous_anchor, incoming_velocity);

            printf("WEB_SWING %s attach anchor=(%.2f %.2f %.2f) rope=%.2f speed=%.3f\n",
                   WEB_SWING_LOG_SIDE,
                   vecParamsXYZ(motion->web_swing_anchor),
                   motion->web_swing_rope_length,
                   lengthVec3(motion->vel));
            filelog_printf("webswing.log", "WEB_SWING %s attach anchor=(%.2f %.2f %.2f) rope=%.2f speed=%.3f\n",
                           WEB_SWING_LOG_SIDE,
                           vecParamsXYZ(motion->web_swing_anchor),
                           motion->web_swing_rope_length,
                           lengthVec3(motion->vel));
        }
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
        printf("WEB_SWING %s attach_catch grounded_at_acquisition=%d falling_at_acquisition=%d jumping_at_acquisition=%d velocity_before=(%.3f %.3f %.3f) velocity_after=(%.3f %.3f %.3f) upward_delta=%.3f forward_delta=%.3f forward_speed_before=%.3f intent=(%.3f %.3f %.3f) intent_source=%s input_magnitude=%.3f anchor=(%.2f %.2f %.2f) rope=%.2f\n",
               WEB_SWING_LOG_SIDE,
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
        filelog_printf("webswing.log", "WEB_SWING %s attach_catch grounded_at_acquisition=%d falling_at_acquisition=%d jumping_at_acquisition=%d velocity_before=(%.3f %.3f %.3f) velocity_after=(%.3f %.3f %.3f) upward_delta=%.3f forward_delta=%.3f forward_speed_before=%.3f intent=(%.3f %.3f %.3f) intent_source=%s input_magnitude=%.3f anchor=(%.2f %.2f %.2f) rope=%.2f\n",
                       WEB_SWING_LOG_SIDE,
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
        webSwingLogChainArm(e, arc_height, forward_speed);
    }

    if(motion->web_swing_chain_armed &&
       motion->vel[1] >= WEB_CHAIN_RELEASE_UP_SPEED &&
       forward_speed >= WEB_CHAIN_MIN_FORWARD_SPEED)
    {
        copyVec3(motion->web_swing_anchor, motion->web_swing_previous_anchor);
        webSwingLogAutoChainRelease(e, forward_speed);
        motion->web_swing_attached = 0;
        motion->web_swing_chain_armed = 0;
        motion->web_swing_chain_reacquire = 1;
        motion->web_swing_attach_catch_pending = 0;
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
    if(intent_source != WEB_SWING_INTENT_NONE)
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

    PERFINFO_AUTO_START("entWorldCollideTop", 1);

    copyVec3(motion->surf_normal, last_slope);

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
    if(motion->on_surf)
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

    if(motion->jumping)
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
