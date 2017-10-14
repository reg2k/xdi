#include "GameUtils.h"

#include "Globals.h"

#include "f4se/PapyrusVM.h"

namespace GameUtils
{
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
}