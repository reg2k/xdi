#include "GameUtils.h"

#include "Globals.h"
#include "rva/RVA.h"
#include "Utils.h"

#include "ExtraTypes.h"

#include "f4se/GameData.h"
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

    TESForm * GetFormFromIdentifier(const std::string & identifier) {
        auto delimiter = identifier.find('|');
        if (delimiter != std::string::npos) {
            std::string modName = identifier.substr(0, delimiter);
            std::string modForm = identifier.substr(delimiter + 1);

            const ModInfo* mod = (*G::dataHandler)->LookupModByName(modName.c_str());
            if (mod && mod->modIndex != 0xFF) {
                UInt32 formID = strtoul(modForm.c_str(), nullptr, 16) & 0xFFFFFF;
                UInt32 flags = Utils::GetOffset<UInt32>(mod, 0x334);
                if (flags & (1 << 9)) {
                    // ESL
                    formID &= 0xFFF;
                    formID |= 0xFE << 24;
                    formID |= Utils::GetOffset<UInt16>(mod, 0x372) << 12;	// ESL load order
                } else {
                    formID |= (mod->modIndex) << 24;
                }
                return LookupFormByID(formID);
            }
        }
        return nullptr;
    }

    TESForm* GetFormFromFile(const char* pluginName, UInt32 formID) {
        const ModInfo* mod = (*G::dataHandler)->LookupModByName(pluginName);
        if (mod && mod->modIndex != 0xFF) {
            UInt32 flags = Utils::GetOffset<UInt32>(mod, 0x334);
            if (flags & (1 << 9)) {
                // ESL
                formID &= 0xFFF;
                formID |= 0xFE << 24;
                formID |= Utils::GetOffset<UInt16>(mod, 0x372) << 12;	// ESL load order
            } else {
                formID |= (mod->modIndex) << 24;
            }
            return LookupFormByID(formID);
        }
        return nullptr;
    }
}