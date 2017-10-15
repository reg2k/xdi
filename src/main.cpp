#include <shlobj.h>
#include "f4se/PluginAPI.h"

#include "f4se_common/SafeWrite.h"
#include "f4se_common/BranchTrampoline.h"
#include "f4se/GameReferences.h"
#include "f4se/GameRTTI.h"

#include "Config.h"
#include "RVA.h"
#include "Globals.h"

#include "ExtraTypes.h"
#include "Scaleform.h"
#include "Papyrus.h"
#include "DialogueEx.h"

#define DEBUG _DEBUG
#if DEBUG
#include "Debug.h"
#endif

IDebugLog gLog;
PluginHandle g_pluginHandle = kPluginHandle_Invalid;

F4SEScaleformInterface *g_scaleform = NULL;
F4SEPapyrusInterface   *g_papyrus   = NULL;
F4SEMessagingInterface *g_messaging = NULL;

//-------------------------
// Event Handlers
//-------------------------

void OnF4SEMessage(F4SEMessagingInterface::Message* msg) {
    switch (msg->type) {
        case F4SEMessagingInterface::kMessage_GameLoaded:
            DialogueEx::OnGameLoaded();
            break;
    }
}

//-------------------------
// F4SE Init
//-------------------------

extern "C"
{

bool F4SEPlugin_Query(const F4SEInterface * f4se, PluginInfo * info)
{
    char logPath[MAX_PATH];
    sprintf_s(logPath, sizeof(logPath), "\\My Games\\Fallout4\\F4SE\\%s.log", PLUGIN_NAME_SHORT);
    gLog.OpenRelative(CSIDL_MYDOCUMENTS, logPath);

    _MESSAGE("%s v%s", PLUGIN_NAME_SHORT, PLUGIN_VERSION_STRING);
    _MESSAGE("%s query", PLUGIN_NAME_SHORT);

    // populate info structure
    info->infoVersion = PluginInfo::kInfoVersion;
    info->name    = PLUGIN_NAME_SHORT;
    info->version = PLUGIN_VERSION;

    // store plugin handle so we can identify ourselves later
    g_pluginHandle = f4se->GetPluginHandle();

    // Check game version
	if (!COMPATIBLE(f4se->runtimeVersion)) {
		char str[512];
		sprintf_s(str, sizeof(str), "Your game version: v%d.%d.%d.%d\nExpected version: v%d.%d.%d.%d\n%s will be disabled.",
			GET_EXE_VERSION_MAJOR(f4se->runtimeVersion),
			GET_EXE_VERSION_MINOR(f4se->runtimeVersion),
			GET_EXE_VERSION_BUILD(f4se->runtimeVersion),
			GET_EXE_VERSION_SUB(f4se->runtimeVersion),
			GET_EXE_VERSION_MAJOR(SUPPORTED_RUNTIME_VERSION),
			GET_EXE_VERSION_MINOR(SUPPORTED_RUNTIME_VERSION),
			GET_EXE_VERSION_BUILD(SUPPORTED_RUNTIME_VERSION),
			GET_EXE_VERSION_SUB(SUPPORTED_RUNTIME_VERSION),
			PLUGIN_NAME_LONG
		);

		MessageBox(NULL, str, PLUGIN_NAME_LONG, MB_OK | MB_ICONEXCLAMATION);
		return false;
	}

    if (f4se->runtimeVersion > SUPPORTED_RUNTIME_VERSION) {
        _MESSAGE("INFO: Newer game version (%08X) than target (%08X).", f4se->runtimeVersion, SUPPORTED_RUNTIME_VERSION);
    }

    // Get the scaleform interface
    g_scaleform = (F4SEScaleformInterface *)f4se->QueryInterface(kInterface_Scaleform);
    if (!g_scaleform) {
        _MESSAGE("couldn't get scaleform interface");
        return false;
    }

    // Get the papyrus interface
    g_papyrus = (F4SEPapyrusInterface *)f4se->QueryInterface(kInterface_Papyrus);
    if (!g_papyrus) {
        _MESSAGE("couldn't get papyrus interface");
        return false;
    }

    // Get the messaging interface
    g_messaging = (F4SEMessagingInterface *)f4se->QueryInterface(kInterface_Messaging);
    if (!g_messaging) {
        _MESSAGE("couldn't get messaging interface");
        return false;
    }

	return true;
}

bool F4SEPlugin_Load(const F4SEInterface *f4se)
{
    _MESSAGE("%s load", PLUGIN_NAME_SHORT);

    G::Init();
    RVAManager::UpdateAddresses(f4se->runtimeVersion);

    if (!g_localTrampoline.Create(1024 * 64, nullptr)) {
        _ERROR("couldn't create codegen buffer. this is fatal. skipping remainder of init process.");
        return false;
    }

    if (!g_branchTrampoline.Create(1024 * 64)) {
        _ERROR("couldn't create branch trampoline. this is fatal. skipping remainder of init process.");
        return false;
    }

    // Register Scaleform handlers
    g_scaleform->Register(PLUGIN_NAME_SHORT, Scaleform::RegisterScaleform);

    // Register Papyrus native functions
    g_papyrus->Register(Papyrus::RegisterPapyrus);

    // Register for F4SE messages
    g_messaging->RegisterListener(g_pluginHandle, "F4SE", OnF4SEMessage);

    // Patch game memory
    DialogueEx::Init();

    #if DEBUG
        Debug::Init();
    #endif

	return true;
}

};
