#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#include <utilitieslib/stdtypes.h>
#include "win/hwlight.h"

// Normally I'd put app #includes after system #includes, but Windows headers are order-sensitive.
//  Caveat refactoror.
#include "cmdparse/cmdgame.h"

#include "windows.h"

#include "entity/character_base.h"
#include "clientcomm/clientcomm.h"
#include "entity/entity.h"
#include "entity/EntPlayer.h"
#include "graphics/font.h"
#include "graphics/gfxLoadScreens.h"
#include "player/player.h"
#include "UI/sprite/sprite_base.h"
#include "graphics/textureatlas.h"
#include "UI/uiStatus.h"
#include "win/win_init.h"

#ifdef __cplusplus
}
#endif // __cplusplus

#include "LogitechLEDLib.h"

static bool s_hwlightInitialized = false;
static bool s_colorSet = false;
static F32 s_eventProgress = 0.0f;

static enum {
    UNKNOWN,
    PRESENT,
    ABSENT,
} s_hwlightPresence = UNKNOWN;

// Listed by ascending priority
static enum {
    EVENT_NONE = 0,
    EVENT_MISSIONCOMPLETE = 1,
    EVENT_LEVELUP = 2,
} s_currentEvent;

void setHWColor(U32 targetColor) {
    LogiLedSetLighting((targetColor & 0x00FF0000) >> 16, (targetColor & 0x0000FF00) >> 8, (targetColor & 0x000000FF));
}

static bool doInitialization() {
    writeConsole(OUTPUT_VERBOSE, "Initializing hardware lights");
    bool LedInitialized = LogiLedInitWithName("City of Heroes");
    if (!LedInitialized) {
        writeConsole(OUTPUT_WARNING, "Failed to initialize hardware lights");
        return false;
    }
    writeConsole(OUTPUT_INFO, "Initialized hardware lights");
    LogiLedSetTargetDevice(LOGI_DEVICETYPE_ALL);
    setHWColor(0);
    return true;
}

static bool getLevelUpColor(U32* targetColor) {
    static const U32 COLORS[] =
        { 0xffffffff, 0xffff0000, 0xffffffff, 0xff0000ff, 0xffffffff, 0xff00ff00 };
    int cyclePosition;

    if (s_currentEvent != EVENT_LEVELUP)
        return false;

    if (s_eventProgress > 30)
        s_currentEvent = EVENT_NONE;

    cyclePosition = ((int)(s_eventProgress / 3.0f)) % ARRAY_SIZE(COLORS);
    *targetColor = COLORS[cyclePosition];
    s_eventProgress += TIMESTEP;
    return true;
}

static bool getMissionCompleteColor(U32* targetColor) {
    F32 intensity;
    const F32 LENGTH_IN_FRAMES = 20.0f;

    if (s_currentEvent != EVENT_MISSIONCOMPLETE)
        return false;

    if (s_eventProgress >= LENGTH_IN_FRAMES) {
        s_currentEvent = EVENT_NONE;
        return false;
    }

    intensity = fmod(2.0 * s_eventProgress / LENGTH_IN_FRAMES, 1.0);

    *targetColor = 0xff000000 | (((int)(0xff * intensity)) << 8);
    s_eventProgress += TIMESTEP;
    return true;
}

bool isValidPlayer(Entity * e) {
    return e && e->pchar && (commCheckConnection() == CS_CONNECTED || loadingScreenVisible());
}

static bool getLowHealthColor(U32* targetColor) {
    F32 intensity;
    Entity *e = playerPtr();
    F32 healthPercentage;

    if (!isValidPlayer(e))
        return false;

    healthPercentage = e->pchar->attrCur.fHitPoints / e->pchar->attrMax.fHitPoints;

    if (healthPercentage > MAX_HEARTBEAT_HEALTH_PERCENTAGE)
        return false;

    intensity = 0.5 * getHeartBeatIntensity(healthPercentage, false);
    *targetColor = 0xff000000 | (((int)(0xff * intensity)) << 16);
    return true;
}

enum {
    COLOR_HERO = 0xffffffff,
    COLOR_VIGILANTE = 0xff0000ff,
    COLOR_ROGUE = 0xffff7b00,
    COLOR_VILLAIN = 0xffff0000,
    COLOR_RESISTANCE = 0xff00ffff,
    COLOR_LOYALIST = 0xffffff00,
};

static bool getAlignmentColor(U32* targetColor) {
    Entity *e = playerPtr();

    if (!e || !e->pl)
        return false;

    switch (e->pl->praetorianProgress) {
        case kPraetorianProgress_PrimalBorn: case kPraetorianProgress_PrimalEarth: case kPraetorianProgress_NeutralInPrimalTutorial:
            if (e->pl->playerType == kPlayerType_Hero)
                *targetColor = (e->pl->playerSubType == kPlayerSubType_Rogue) ? COLOR_VIGILANTE : COLOR_HERO;
            else
                *targetColor = (e->pl->playerSubType == kPlayerSubType_Rogue) ? COLOR_ROGUE : COLOR_VILLAIN;
            break;

        case kPraetorianProgress_Tutorial: case kPraetorianProgress_Praetoria:
            *targetColor = (e->pl->playerType == kPlayerType_Hero) ? COLOR_RESISTANCE : COLOR_LOYALIST;
            break;

        default:
            // It's bad if we hit this, but not so bad that we should crash in the wild.
#ifndef FINAL
            assert(!"Unrecognized praetorianProgress in getLightColor.");
#else
            return false;
#endif
    }

    return true;
}

static bool getLightColor(U32 *targetColor) {
    if (getLevelUpColor(targetColor))
        return true;

    if (getMissionCompleteColor(targetColor))
        return true;

    if (getLowHealthColor(targetColor))
        return true;

    return getAlignmentColor(targetColor);
}

static void displayEmulatedLights(U32 targetColor) {
    unsigned int rgba = (targetColor << 8) | (targetColor >> 24);
    int screenWidth, screenHeight;
    windowClientSize(&screenWidth, &screenHeight);
    display_sprite(white_tex_atlas, 0, screenHeight - 100, 100000, 100.0f / white_tex_atlas->width, 
        100.0f / white_tex_atlas->height, rgba);
}

void updatePhysicalLightsIfEnabled(U32 targetColor) {
    if (game_state.enableHardwareLights && s_hwlightPresence == PRESENT) {
        setHWColor(targetColor);
    }
}

static void updateLights() {
    static U32 s_currentColor;
    static F32 s_secondsSinceLastColorChange = 1000.0f;
    U32 targetColor;

    s_secondsSinceLastColorChange += TIMESTEP / 30;

    if (!getLightColor(&targetColor))
        return;

    if (game_state.emulateHardwareLights)
        displayEmulatedLights(targetColor);

    if (targetColor != s_currentColor || !s_colorSet) {
        updatePhysicalLightsIfEnabled(targetColor);
        s_secondsSinceLastColorChange = 0.0f;
        s_currentColor = targetColor;
        s_colorSet = true;
    }
}

void hwlightInitialize() {
    // Do not unconditionally check whether the hardware is available as this consumes too much memory
    if (s_hwlightInitialized || (s_hwlightPresence == ABSENT))
        return;

    s_hwlightInitialized = doInitialization();
    s_hwlightPresence = s_hwlightInitialized ? PRESENT : ABSENT;
}

void hwlightTick() {
    if (s_hwlightPresence == ABSENT && !game_state.emulateHardwareLights) {
        game_state.enableHardwareLights = false;
        return;
    }

    if (game_state.enableHardwareLights || game_state.emulateHardwareLights) {
        if (!s_hwlightInitialized)
            hwlightInitialize();

        if (s_hwlightPresence == PRESENT || game_state.emulateHardwareLights)
            updateLights();
    } else {
        if (s_hwlightInitialized)
            hwlightShutdown();
    }
}

void hwlightSignalLevelUp() {
    s_currentEvent = max(s_currentEvent, EVENT_LEVELUP);
    s_eventProgress = 0.0f;
}

void hwlightSignalMissionComplete() {
    s_currentEvent = max(s_currentEvent, EVENT_MISSIONCOMPLETE);
    s_eventProgress = 0.0f;
}

void hwlightShutdown() {

    if (s_hwlightInitialized) {
        LogiLedShutdown();
        writeConsole(OUTPUT_VERBOSE, "Shut down hardware lights");
    }

    s_hwlightInitialized = false;
    s_colorSet = false;
    game_state.enableHardwareLights = false;
}

bool hwlightIsPresent() {
    return s_hwlightPresence == PRESENT;
}