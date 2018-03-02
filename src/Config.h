#pragma once
#include "f4se_common/f4se_version.h"

//-----------------------
// Plugin Information
//-----------------------
#define PLUGIN_VERSION              8
#define PLUGIN_VERSION_STRING       "1.15"
#define PLUGIN_NAME_SHORT           "XDI"
#define PLUGIN_NAME_LONG            "Extended Dialogue Interface"
#define SUPPORTED_RUNTIME_VERSION   CURRENT_RELEASE_RUNTIME
#define MINIMUM_RUNTIME_VERSION     RUNTIME_VERSION_1_10_26
#define COMPATIBLE(runtimeVersion)  (runtimeVersion == SUPPORTED_RUNTIME_VERSION)

#define GAME_PLUGIN_NAME            "XDI.esm"
#define GAME_ACTIVATION_KEYWORD_ID  0x800
#define GAME_RESULT_ID              0x801

#define INI_LOCATION_DEFAULTS       "./Data/MCM/Config/XDI/settings.ini"
#define INI_LOCATION_USER           "./Data/MCM/Settings/XDI.ini"

//-------------
// Addresses
//-------------
// [Count]  Location
// [23]     DialogueEx.cpp