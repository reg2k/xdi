#include "GameUtils.h"

#include "Globals.h"
#include "rva/RVA.h"
#include "Utils.h"

#include "ExtraTypes.h"

#include "f4se/GameReferences.h"
#include "f4se/GameMenus.h"
#include "f4se/GameSettings.h"
#include "f4se/GameRTTI.h"
#include "f4se/PapyrusVM.h"

namespace GameUtils {
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

    Setting* GetINISetting(const char* name) {
        Setting* setting = (*G::iniSettings)->Get(name);
        if (!setting)
            setting = (*G::iniPrefSettings)->Get(name);

        return setting;
    }

    const char* GetReferenceName(TESObjectREFR * refr) {
        return CALL_MEMBER_FN(refr, GetReferenceName)();
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