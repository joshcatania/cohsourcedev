#include "autoResumeInfo.h"
#include "file.h"
#include "uiLogin.h"
#include "uiEditText.h"
#include "cmdgame.h"
#include "utils.h"
#include "win_init.h"
#include "RegistryReader.h"
#include "AppRegCache.h"
#include "sound_sys.h"
#include "timing.h"
#include "textparser.h"
#include "uiOptions.h"
#include "crypt.h"
#include <shlobj.h>
#include <string>

//ResumeInfo is now in the registry
#define REG_STR_BUF_SIZE 256

const std::string kSettingsFilenameGfx = "gfx.parse6";
const std::string kSettingsFilenameMisc = "misc.parse6";
const int kSettingsDefaultValue = 999;

static GfxSettings gfxSettings_temp;

// Load/Save these regardless of safe-mode
TokenizerParseInfo autoResumeRegInfoSafe[] = {
	{ "accountName",			TOK_STRING_X, (int)&gfxSettings_temp.accountName, ARRAY_SIZE_CHECKED(gfxSettings_temp.accountName)},
	{ "gamma",					TOK_F32_X,	(int)&gfxSettings_temp.gamma},
	{ "fieldOfView",			TOK_INT_X,	(int)&gfxSettings_temp.fieldOfView, kSettingsDefaultValue},
	{ "fxSoundVolume",			TOK_F32_X,	(int)&gfxSettings_temp.fxSoundVolume},
	{ "musicSoundVolume",		TOK_F32_X,	(int)&gfxSettings_temp.musicSoundVolume},
	{ "voiceoverSoundVolume",	TOK_F32_X,	(int)&gfxSettings_temp.voSoundVolume},
	{ "dontSaveName",			TOK_INT_X,	(int)&gfxSettings_temp.dontSaveName},
	{ "", 0, 0 }
};

// Load always, Save these only if not in safe-mode
// ParseTable items need a non-zero param set (kSettingsDefaultValue) else the StructParser writetext
// functions will skip writing zero values. This is a problem with config specifically because zero values
// are perfectly valid, e.g. fullscreen. If the zero value is never saved then it will never be read and 
// never override the default values provided.
TokenizerParseInfo autoResumeRegInfo[] = {
	{ "version",			TOK_INT_X,	(int)&gfxSettings_temp.version, kSettingsDefaultValue},
	{ "screenX",			TOK_INT_X,	(int)&gfxSettings_temp.screenX, kSettingsDefaultValue},
	{ "screenY",			TOK_INT_X,	(int)&gfxSettings_temp.screenY, kSettingsDefaultValue},
	{ "refreshRate",		TOK_INT_X,	(int)&gfxSettings_temp.refreshRate, kSettingsDefaultValue},
	{ "screenX_pos",		TOK_INT_X,	(int)&gfxSettings_temp.screenX_pos, kSettingsDefaultValue},
	{ "screenY_pos",		TOK_INT_X,	(int)&gfxSettings_temp.screenY_pos, kSettingsDefaultValue},
	{ "maximized",			TOK_INT_X,	(int)&gfxSettings_temp.maximized, kSettingsDefaultValue},
	{ "fullScreen",			TOK_INT_X,	(int)&gfxSettings_temp.fullScreen, kSettingsDefaultValue},
	{ "mipLevel",			TOK_INT_X,	(int)&gfxSettings_temp.advanced.mipLevel, kSettingsDefaultValue},
	{ "characterMipLevel",	TOK_INT_X,	(int)&gfxSettings_temp.advanced.entityMipLevel, kSettingsDefaultValue},
	{ "texLodBias",			TOK_INT_X,	(int)&gfxSettings_temp.advanced.texLodBias, kSettingsDefaultValue},
	{ "texAniso",			TOK_INT_X,	(int)&gfxSettings_temp.advanced.texAniso, kSettingsDefaultValue},
	{ "worldDetailLevel",	TOK_F32_X,	(int)&gfxSettings_temp.advanced.worldDetailLevel, kSettingsDefaultValue},
	{ "entityDetailLevel",	TOK_F32_X,	(int)&gfxSettings_temp.advanced.entityDetailLevel, kSettingsDefaultValue},
	{ "shadowMode",			TOK_INT_X,	(int)&gfxSettings_temp.advanced.shadowMode, kSettingsDefaultValue},
	{ "shadowMapShowAdvanced",TOK_INT_X,	(int)&gfxSettings_temp.advanced.shadowMap.showAdvanced, kSettingsDefaultValue},
	{ "shadowMapShader",	TOK_INT_X,	(int)&gfxSettings_temp.advanced.shadowMap.shader, kSettingsDefaultValue},
	{ "shadowMapSize",		TOK_INT_X,	(int)&gfxSettings_temp.advanced.shadowMap.size, kSettingsDefaultValue},
	{ "shadowMapDistance",	TOK_INT_X,	(int)&gfxSettings_temp.advanced.shadowMap.distance, kSettingsDefaultValue},
	{ "cubemapMode",		TOK_INT_X,	(int)&gfxSettings_temp.advanced.cubemapMode, kSettingsDefaultValue},
	{ "buildingPlanarReflections",		TOK_INT_X,	(int)&gfxSettings_temp.advanced.buildingPlanarReflections, kSettingsDefaultValue},
	{ "ageiaOn",			TOK_INT_X,	(int)&gfxSettings_temp.advanced.ageiaOn, kSettingsDefaultValue},
	{ "physicsQuality",		TOK_INT_X,	(int)&gfxSettings_temp.advanced.physicsQuality, kSettingsDefaultValue},
	{ "showAdvanced",		TOK_INT_X,	(int)&gfxSettings_temp.showAdvanced, kSettingsDefaultValue},
	{ "graphicsQuality",	TOK_F32_X,	(int)&gfxSettings_temp.slowUglyScale, kSettingsDefaultValue},
	{ "maxParticles",		TOK_INT_X,	(int)&gfxSettings_temp.advanced.maxParticles, kSettingsDefaultValue},
	{ "maxParticleFill",	TOK_F32_X,	(int)&gfxSettings_temp.advanced.maxParticleFill, kSettingsDefaultValue},
	{ "suppressCloseFx",	TOK_INT_X,	(int)&gfxSettings_temp.advanced.suppressCloseFx, kSettingsDefaultValue},
	{ "suppressCloseFxDist",TOK_F32_X,	(int)&game_state.suppressCloseFxDist, kSettingsDefaultValue},
	{ "forceSoftwareAudio",	TOK_INT_X,	(int)&gfxSettings_temp.forceSoftwareAudio, kSettingsDefaultValue},
	{ "enableVBOs",			TOK_INT_X,	(int)&gfxSettings_temp.advanced.enableVBOs, kSettingsDefaultValue},
	{ "enable3DSound",		TOK_INT_X,	(int)&gfxSettings_temp.enable3DSound, kSettingsDefaultValue},
	{ "renderScaleX",		TOK_F32_X,	(int)&gfxSettings_temp.renderScaleX, kSettingsDefaultValue},
	{ "renderScaleY",		TOK_F32_X,	(int)&gfxSettings_temp.renderScaleY, kSettingsDefaultValue},
	{ "useRenderScale",		TOK_INT_X,	(int)&gfxSettings_temp.useRenderScale, kSettingsDefaultValue},
	{ "shaderDetail",		TOK_INT_X,	(int)&gfxSettings_temp.advanced.shaderDetail, kSettingsDefaultValue},
	{ "useWater",			TOK_INT_X,	(int)&gfxSettings_temp.advanced.useWater, kSettingsDefaultValue},
	{ "useBloom",			TOK_INT_X,	(int)&gfxSettings_temp.advanced.useBloom, kSettingsDefaultValue},
	{ "useDesaturate",		TOK_INT_X,	(int)&gfxSettings_temp.advanced.useDesaturate, kSettingsDefaultValue},
	{ "bloomMagnitude",		TOK_F32_X,	(int)&gfxSettings_temp.advanced.bloomMagnitude, kSettingsDefaultValue},
	{ "useDOF",				TOK_INT_X,	(int)&gfxSettings_temp.advanced.useDOF, kSettingsDefaultValue},
	{ "dofMagnitude",		TOK_F32_X,	(int)&gfxSettings_temp.advanced.dofMagnitude, kSettingsDefaultValue},
	{ "ambientShowAdvanced",TOK_INT_X,	(int)&gfxSettings_temp.advanced.ambient.showAdvanced, kSettingsDefaultValue},
	{ "ambientOptionScale",	TOK_F32_X,	(int)&gfxSettings_temp.advanced.ambient.optionScale, kSettingsDefaultValue},
	{ "ambientStrength",	TOK_INT_X,	(int)&gfxSettings_temp.advanced.ambient.strength, kSettingsDefaultValue},
	{ "ambientResolution",	TOK_INT_X,	(int)&gfxSettings_temp.advanced.ambient.resolution, kSettingsDefaultValue},
	{ "ambientBlur",		TOK_INT_X,	(int)&gfxSettings_temp.advanced.ambient.blur, kSettingsDefaultValue},
	{ "antiAliasing",		TOK_INT_X,	(int)&gfxSettings_temp.antialiasing, kSettingsDefaultValue},
	{ "useVSync",			TOK_INT_X,	(int)&gfxSettings_temp.advanced.useVSync, kSettingsDefaultValue},
	{ "fancyMouseCursor",	TOK_INT_X,	(int)&gfxSettings_temp.advanced.colorMouseCursor, kSettingsDefaultValue},
	{ "ShowLoginDialog",	TOK_INT_X,	(int)&game_state.showLoginDialog, kSettingsDefaultValue},
	{ "enableHWLights",		TOK_INT_X,	(int)&gfxSettings_temp.enableHWLights, kSettingsDefaultValue},
	{ "razerMouseTray",		TOK_INT_X,	(int)&game_state.razerMouseTray, kSettingsDefaultValue},
	{ "", 0, 0 }
};

// Load only
TokenizerParseInfo autoResumeRegInfoReadOnly[] = {
	{ "Auth",			TOK_STRING_X,	(int)&game_state.auth_address, ARRAY_SIZE_CHECKED(game_state.auth_address)},
	{ "DbServer",		TOK_STRING_X,	(int)&game_state.cs_address, ARRAY_SIZE_CHECKED(game_state.cs_address)},
	{ "", 0, 0 }
};

void saveAutoResumeInfoToRegistry(void)
{
	gfxGetSettingsForNextTime( &gfxSettings_temp );

	if (!gfxSettings_temp.filledIn)
		return;

	//TO DO Should be params to this function really
	gfxSettings_temp.dontSaveName = g_iDontSaveName;

	if( gfxSettings_temp.dontSaveName )
		Strncpyt(gfxSettings_temp.accountName, "");
	else
		Strncpyt(gfxSettings_temp.accountName, g_achAccountName);
	
	//Kind of a hack: if your vis_scale is silly high, you almost certainly
	//don't want that saved to the registry.  You were probably editing maps
	if( gfxSettings_temp.advanced.worldDetailLevel > WORLD_DETAIL_LIMIT )
		gfxSettings_temp.advanced.worldDetailLevel = 1.0;
	//End hack

	char roamingAppData[MAX_PATH];
	bool writeToFile = SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, CSIDL_FLAG_CREATE, roamingAppData) == S_OK;
	std::string cohAppDataDir, gfxSettingsFile, miscSettingsFile;
	if (writeToFile) {
		cohAppDataDir = std::string(roamingAppData) + '/' + regGetAppName() + '/';
		gfxSettingsFile = cohAppDataDir + kSettingsFilenameGfx;
		miscSettingsFile = cohAppDataDir + kSettingsFilenameMisc;
		writeToFile = gfxSettingsFile.length() <= MAX_PATH
			&& miscSettingsFile.length() <= MAX_PATH;
	}

	// Misc file is saved regardless of whether client is in safemode or not
	if (writeToFile) {
		_mkdir(cohAppDataDir.c_str());
		ParserWriteTextFile(miscSettingsFile.c_str(), autoResumeRegInfoSafe, NULL, 0, 0);
		if (!game_state.safemode || game_state.options_have_been_saved)
		{
			ParserWriteTextFile(gfxSettingsFile.c_str(), autoResumeRegInfo, NULL, 0, 0);
		}
	} else {
		ParserWriteRegistry(regGetAppKey(), autoResumeRegInfoSafe, NULL, 0, 0);
		if (!game_state.safemode || game_state.options_have_been_saved)
		{
			ParserWriteRegistry(regGetAppKey(), autoResumeRegInfo, NULL, 0, 0);
		}
	}
}

int getAutoResumeInfoFromRegistry( GfxSettings * gfxSettings, char * accountName, int * dontSaveName )
{
	// Determine what settings may need to be overwritten
	bool isFirstRun = true;
	bool isFirstRunSinceSlowUglySlider = true;

	ZeroStruct(&gfxSettings_temp);
	gfxSettings_temp.slowUglyScale = -1;

	char roamingAppData[MAX_PATH];
	bool readFromFile = SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, CSIDL_FLAG_CREATE, roamingAppData) == S_OK;

	std::string gfxSettingsFile, miscSettingsFile;
	if (readFromFile) {
		std::string cohAppDataDir = std::string(roamingAppData) + '/' + regGetAppName() + '/';
		gfxSettingsFile = cohAppDataDir + kSettingsFilenameGfx;
		miscSettingsFile = cohAppDataDir + kSettingsFilenameMisc;
		readFromFile = gfxSettingsFile.length() <= MAX_PATH
			&& miscSettingsFile.length() <= MAX_PATH
			&& (GetFileAttributes(gfxSettingsFile.c_str()) != INVALID_FILE_ATTRIBUTES)
			&& (GetFileAttributes(miscSettingsFile.c_str()) != INVALID_FILE_ATTRIBUTES);
	}

	(readFromFile)
		? ParserReadTextFile(gfxSettingsFile.c_str(), autoResumeRegInfo, NULL)
		: ParserReadRegistry(regGetAppKey(), autoResumeRegInfo, NULL);

	if (gfxSettings_temp.screenX != 0)
		isFirstRun = false;
	if (gfxSettings_temp.slowUglyScale != -1)
		isFirstRunSinceSlowUglySlider = false;

	gfxSettings_temp = *gfxSettings; // Fill in defaults 
	gfxSettings_temp.version = 0;

	// Registry is legacy and settings won't be stored there anymore but check anyway 
	// incase client migrating from previous version. TODO Remove in the future.
	if (readFromFile) {
		ParserReadTextFile(gfxSettingsFile.c_str(), autoResumeRegInfo, NULL);
		ParserReadTextFile(miscSettingsFile.c_str(), autoResumeRegInfoSafe, NULL);
	} else {
		ParserReadRegistry(regGetAppKey(), autoResumeRegInfoSafe, NULL);
		ParserReadRegistry(regGetAppKey(), autoResumeRegInfo, NULL);
	}

	// Ignore the auth and dbserver registry entries if we were started by the Launcher
	// TODO: Once CohUpdater is deprecated, we should be able to remove this completely
	// since the Launcher will always use command line arguments and not the registry
	if (!game_state.usedLauncher)
		ParserLoadFromRegistry(regGetAppKey(), autoResumeRegInfoReadOnly, NULL);

	gfxSettings_temp.filledIn = true;

	if (isFirstRunSinceSlowUglySlider && !isFirstRun) {
		// User had previous settings, don't overwrite them!
		gfxSettings_temp.slowUglyScale = 0.6;
		gfxSettings_temp.showAdvanced = 1;
	}

	gfxSettingsFixup(&gfxSettings_temp);

	*gfxSettings = gfxSettings_temp;

	// Sanity checks and copying appropriate data
	if (accountName)
		strcpy(accountName, gfxSettings->accountName);

	if (gfxSettings->advanced.worldDetailLevel < 0.5f)
		gfxSettings->advanced.worldDetailLevel = 0.5f;
	else if (gfxSettings->advanced.worldDetailLevel > 2.0f)
		gfxSettings->advanced.worldDetailLevel = 2.0f;

	if (gfxSettings->advanced.entityDetailLevel < 0.3f)
		gfxSettings->advanced.entityDetailLevel= 0.6f;
	else if (gfxSettings->advanced.entityDetailLevel> 2.0f)
		gfxSettings->advanced.entityDetailLevel = 2.0f;

	g_audio_state.software = gfxSettings->forceSoftwareAudio;

	if (dontSaveName)
		*dontSaveName = gfxSettings->dontSaveName;

	g_audio_state.uisurround = gfxSettings->enable3DSound;

	game_state.enableHardwareLights = gfxSettings->enableHWLights;

	if (gfxSettings->useRenderScale) {
		int effResX, effResY;
		if (gfxSettings->useRenderScale==RENDERSCALE_FIXED) {
			effResX = gfxSettings->renderScaleX;
			effResY = gfxSettings->renderScaleY;
		} else if (gfxSettings->useRenderScale==RENDERSCALE_SCALE) {
			effResX = gfxSettings->renderScaleX*gfxSettings->screenX;
			effResY = gfxSettings->renderScaleY*gfxSettings->screenY;
		}
		if (effResX < 16 || effResX > gfxSettings->screenX ||
			effResY < 16 || effResY > gfxSettings->screenY)
		{
			gfxSettings->useRenderScale=RENDERSCALE_OFF;
		}
	}

	if (gfxSettings->fieldOfView < FIELDOFVIEW_MIN || gfxSettings->fieldOfView > FIELDOFVIEW_MAX) {
		gfxSettings->fieldOfView = FIELDOFVIEW_STD;
	}

	if (gfxSettings->advanced.bloomMagnitude < 0.1 || gfxSettings->advanced.bloomMagnitude > 4.0)
		gfxSettings->advanced.bloomMagnitude = 1.0;
	if (gfxSettings->advanced.dofMagnitude < 0.1 || gfxSettings->advanced.dofMagnitude > 4.0)
		gfxSettings->advanced.dofMagnitude = 1.0;

	if (game_state.safemode)
	{
		g_audio_state.software = gfxSettings->forceSoftwareAudio = 1;
		optionSet(kUO_EnableJoystick, 0, 0);
		g_audio_state.uisurround = gfxSettings->enable3DSound = 0;
		game_state.enableHardwareLights = gfxSettings->enableHWLights = 0;

		gfxGetSafeModeSettings(gfxSettings);
	}

	return 1;
}

/*
Set password if you have the "cryptic" setting.  Getting the account name is redundant with
the registry account name setting so it's backwards compatible with old resume info file
*/
void saveAutoResumeInfoCryptic(void)
{
	FILE* file;

	PERFINFO_AUTO_START("saveAutoResumeInfoCryptic", 1);
	file = fileOpen("c:\\resume_info.txt", "wt");

	if (file)
	{
		U8 password[32];
		assert(sizeof(password) == sizeof(g_achPassword));
		cryptRetrieve(password, g_achPassword, sizeof(g_achPassword));
		fprintf(file, "\"%s\"\n", g_achAccountName);
		fprintf(file, "%s\n", password);
		fileClose(file);
		memset(password, 0, sizeof(password));
	}
	PERFINFO_AUTO_STOP();
}

/*
Get password if you have the "cryptic" setting.  Getting the account name is redundant with
the registry account name getting so it's backwards compatible with old resume info file
*/
int getAutoResumeInfoCryptic(void)
{
	char* s, * mem, * args[10];
	int		count;

	// Encrypt an empty string
	g_achPassword[0] = 0;
	cryptStore(g_achPassword, g_achPassword, sizeof(g_achPassword));
	mem = (char*) fileAlloc("c:\\resume_info.txt", 0);
	if (!mem)
		return 0;

	//Get Account Name
	count = tokenize_line(mem, args, &s);
	if (!s)
	{
		fileFree(mem);
		return 0;
	}

	if (count)
		Strncpyt(g_achAccountName, args[0]);

	//Get Password
	count = tokenize_line(s, args, &s);
	if (!s)
	{
		fileFree(mem);
		return 0;
	}

	if (count)
	{
		// cryptStore() assumes both destination and source are the same size
		strncpy_s((char*)g_achPassword, sizeof(g_achPassword), args[0], _TRUNCATE);
		cryptStore(g_achPassword, g_achPassword, sizeof(g_achPassword));
	}

	fileFree(mem);

	return 1;
}
