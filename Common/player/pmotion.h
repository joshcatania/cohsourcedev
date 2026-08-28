#ifndef _PMOTION_H
#define _PMOTION_H

#include "cmdparse/cmdcommon.h"
#include "group/groupgrid.h"
#include "entity/motion.h"

typedef struct StashTableImp *StashTable;
typedef const struct StashTableImp *cStashTable;
typedef struct Entity                Entity;
typedef struct FuturePush            FuturePush;
typedef struct MotionState            MotionState;
typedef struct PhysicsRecordingStep    PhysicsRecordingStep;

typedef struct PhysicsRecordingStep {
    U16                        csc_net_id;
    
    #if CLIENT
        U32                    pak_id;
    #endif
    
    struct {
        Vec3                pos;
        MotionState            motion;
    } before, after;
} PhysicsRecordingStep;

typedef struct PhysicsRecording {
    char*                    name;

    struct {
        PhysicsRecordingStep*    step;
        int                        maxCount;
        int                        count;
    } steps;
} PhysicsRecording;

typedef struct GlobalPlayerMotionState {
    StashTable htPhysicsRecording;
} GlobalPlayerMotionState;

extern GlobalPlayerMotionState global_pmotion_state;
extern StashTable g_htAttackVolumes;
#ifdef CLIENT
extern char *g_activeMaterialTrackerName;
typedef struct WebSwingAnimationSyncTelemetry
{
    char move_name[64];
    F32 frame;
    F32 first_frame;
    F32 last_frame;
    F32 normalized_progress;
    int sync_active;
    U32 physical_tick;
    char physical_phase[16];
    F32 physical_angle_degrees;
    F32 physical_plane_speed;
    F32 physical_horizontal_speed;
    F32 target_frame;
    F32 target_normalized_progress;
    F32 sync_error_frames;
    U32 core_restart_count;
    U32 backwards_target_steps;
} WebSwingAnimationSyncTelemetry;

void pmotionWebSwingCaptureSetAnimationSyncTelemetry(
    const WebSwingAnimationSyncTelemetry *telemetry);
void pmotionWebSwingCaptureRenderHud(void);
#endif

#define entMode(e,mode_num) ( (e)->state.mode & ( 1 << (mode_num) ) )

const char* pmotionGetBinaryControlName(ControlId id);
void pmotionUpdateControlsPrePhysics(Entity* player, ControlState* controls, int curTime);
void pmotionUpdateControlsPostPhysics(ControlState* controls, int curTime);
void pmotionSetState(Entity* e, ControlState* controls);
void pmotionSetVel(Entity* e, ControlState* controls,float inp_vel_scale);
F32 pmotionGetMaxJumpHeight(Entity* e, ControlState* controls);
void pmotionVolumeTrackersInvalidate();
void pmotionCheckVolumeTrigger(Entity* e);
int pmotionGetNeighborhoodProperties(Entity* e,char **namep,char **musicp);
int pmotionGetNeighborhoodFromPos(const Vec3 pos, char **namep, char **musicp, int *villainMinp, int *villainMaxp, int *minTeamp, int *maxTeamp);
int pmotionGetNeighborhoodPropertiesInternal(DefTracker *neighborhood,char **namep,char **musicp, int *villainMinp, int *villainMaxp, int *minTeamp, int *maxTeamp);
void pmotionIJustEnteredANewVolume(Entity * e, Volume * volume );
void pmotionIJustExitedAVolume(Entity * e, Volume * volume );
void pmotionApplyFuturePush(MotionState* motion, FuturePush* fp);


PhysicsRecordingStep* createPhysicsRecordingStep(void);
void pmotionRecordStateBeforePhysics(Entity* e, const char* trackName, ControlStateChange* csc);
void pmotionRecordStateAfterPhysics(Entity* e, const char* trackName);

#if SERVER
    void pmotionSendPhysicsRecording(Packet* pak, Entity* e, const char* trackName);
#else
    void pmotionReceivePhysicsRecording(Packet* pak, Entity* e);
    void pmotionUpdateNetID(ControlStateChange* csc);
    void pmotionClearUpdateNetIDs();
#endif



#endif
