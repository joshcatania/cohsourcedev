//
//
// initCommon.h - shared initialization functions
//-------------------------------------------------------------------------------------------------


#include <utilitieslib/network/netio.h>
#include "varutils.h"
#include "fxinfo.h"
#include "entity/entVarUpdate.h"
#include <utilitieslib/utils/timing.h>
#include "gameData\BodyPart.h"    // for bpReadBodyPartFiles()
#include "gameComm/npc.h"
#include "gameComm/villainDef.h"
#include <utilitieslib/utils/error.h>
#include "fxinfo.h"
#include <utilitieslib/utils/FolderCache.h>
#include <utilitieslib/utils/fileutil.h>
#include "seq/seqtype.h"
#include "cmdparse/cmdcommon.h"
#include "account/AccountData.h"

#if SERVER
    #include "Reward.h"
    #include "TeamReward.h"
    #include "entity/entGameActions.h"
    #include "cmdparse/cmdserver.h"
#endif


#if CLIENT
    #include "gameComm/initClient.h"
    #include "clientcomm/clientcomm.h"
    #include "UI/uidialog.h"
    #include "entity/entclient.h"
    #include "player/player.h"
    #include "UI/uiCostume.h"
    #include "fxbhvr.h"
    #include "graphics/FX/fxfluid.h"
    #include "graphics/FX/fxcapes.h"
    #include "cmdparse/cmdgame.h"
#endif


void cacheRelevantFolders()
{
    FolderCacheRequestTree(folder_cache, "Defs"); // If we're in dynamic mode, this will load this tree for faster file access
    FolderCacheRequestTree(folder_cache, "Menu");
    if (!quickload) {
        FolderCacheRequestTree(folder_cache, "player_library/animations/male");
        FolderCacheRequestTree(folder_cache, "player_library/animations/huge");
        FolderCacheRequestTree(folder_cache, "player_library/animations/fem");
        FolderCacheRequestTree(folder_cache, "player_library/animations/Vahzilok");
    }
}

//
//

void init_menus()
{
#ifdef SERVER
    extern int write_templates;
    if (!write_templates)
#endif
    {
        loadstart_printf("Loading FX info.. ");
            fxPreloadFxInfo();
        loadend_printf("done");

        loadstart_printf("Building FX string handles.. ");
            fxBuildFxStringHandles();
        loadend_printf("done");
    }

    loadstart_printf("Loading Loyalty Rewards.. ");
        accountLoyaltyRewardTreeLoad();
    loadend_printf("done");

#ifdef CLIENT
    loadstart_printf("Loading FX behaviors.. ");
        fxPreloadBhvrInfo();
    loadend_printf("done");

    loadstart_printf("Loading villain defs.. ");
        villainReadDefFiles();
    loadend_printf("done");

#ifdef NOVODEX_FLUIDS
    loadstart_printf("Loading FX fluids.. ");
    fxPreloadFluidInfo();
    loadend_printf("done");
#endif

    if (!STATE_STRUCT.nofx) {
        loadstart_printf("Loading FX capes.. ");
            fxPreloadCapeInfo();
        loadend_printf("done");
    }
#endif

    // FIXME!!!
    //    Move these two somewhere else.
    loadstart_printf("Loading body parts.. ");
        bpReadBodyPartFiles();
    loadend_printf("done");

#ifdef SERVER
    if (!server_state.levelEditor)
#endif

    loadstart_printf("Loading NPC defs.. ");
        npcReadDefFiles();
    loadend_printf("done");


#ifdef SERVER
    if (!server_state.tsr)
#endif
    {
        loadstart_printf("Loading ent_types.. ");
            seqTypeLoadFiles();
        loadend_printf("done");
    }


#ifdef SERVER
    if (!server_state.levelEditor)
    {
        loadstart_printf("Loading villain defs.. ");
            villainReadDefFiles();
        loadend_printf("done");

        loadstart_printf("Loading reward tables.. ");
            rewardReadDefFiles();
        loadend_printf("done");

        loadstart_printf("Loading team reward mods.. ");
            teamRewardReadDefFiles();
        loadend_printf("done");
    }
#endif
}

