#include "GameUtils.h"

#include "Globals.h"
#include "rva/RVA.h"
#include "Utils.h"

#include "ExtraTypes.h"

#include "f4se/GameSettings.h"
#include "f4se/GameRTTI.h"
#include "f4se/PapyrusVM.h"

namespace GameUtils {
    //--------------------
    // Addresses [2]
    //--------------------
    RVA <_UI_IsMenuOpen>    UI_IsMenuOpen_Internal      ({{RUNTIME_VERSION_1_10_64, 0x02041F70}}, "E8 ? ? ? ? 84 C0 8B 05 ? ? ? ? 74 62", 0, 1, 5);
    RVA <_GetReferenceName> GetReferenceName_Internal   ({{RUNTIME_VERSION_1_10_64, 0x0040B5E0}}, "40 53 55 57 41 55 48 81 EC ? ? ? ?");

    bool HasVMScript(TESForm* form, const char* scriptName) {

        VirtualMachine* vm = (*G::gameVM)->m_virtualMachine;

        IObjectHandlePolicy*  handlePolicy = vm->GetHandlePolicy();
        UInt64                handle       = handlePolicy->Create(form->formType, form);
        VMIdentifier*         identifier   = nullptr;

        if (vm->GetObjectIdentifier(handle, scriptName, 1, &identifier, 0)) {
            // Release reference
            if (identifier) {
                if (!identifier->DecrementLock()) {
                    identifier->Destroy();
                }
            }
            return true;
        }

        return false;
    }

    bool IsMenuOpen(BSFixedString* menuName) {
        return UI_IsMenuOpen_Internal(*G::ui, menuName);
    }

    Setting* GetINISetting(const char* name) {
        Setting* setting = (*G::iniSettings)->Get(name);
        if (!setting)
            setting = (*G::iniPrefSettings)->Get(name);

        return setting;
    }

    const char* GetReferenceName(TESObjectREFR * refr) {
        return GetReferenceName_Internal(refr);
    }

    bool HasKeyword(TESForm* form, BGSKeyword* keyword) {
        IKeywordFormBase* keywordFormBase = DYNAMIC_CAST(form, TESForm, IKeywordFormBase);
        if (keywordFormBase) {
            auto HasKeyword_Internal = Utils::GetVirtualFunction<_IKeywordFormBase_HasKeyword>(keywordFormBase, 1);
            if (HasKeyword_Internal(keywordFormBase, keyword, 0)) {
                return true;
            }
        }
        return false;
    }
}