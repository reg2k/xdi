#pragma once

#include "f4se/GameForms.h"

class UI;
class Setting;

namespace GameUtils
{
    bool        HasVMScript(TESForm* form, const char* scriptName);
    Setting*    GetINISetting(const char* name);
    const char* GetReferenceName(TESObjectREFR* refr);
    bool HasKeyword(TESForm* form, BGSKeyword* keyword);

    // Function signatures
    typedef bool(*_UI_IsMenuOpen)(UI* ui, BSFixedString* name);
    typedef const char*(*_GetReferenceName)(TESObjectREFR* refr);
}