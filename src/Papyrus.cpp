#include "Papyrus.h"
#include "Config.h"

#include "ExtraTypes.h"
#include "DialogueEx.h"

#include "f4se/PapyrusVM.h"
#include "f4se/PapyrusNativeFunctions.h"

#define SCRIPT_NAME "XDI"

namespace Papyrus
{
    DECLARE_STRUCT(SceneLink, SCRIPT_NAME)

    UInt32 GetVersionCode(StaticFunctionTag* base) {
        return PLUGIN_VERSION;
    }

    SceneLink GetSceneLink(StaticFunctionTag* base, TESTopicInfo* topicInfo) {
        SceneLink result;
        if (auto sceneLink = DialogueEx::GetSceneLink(topicInfo)) {
            result.Set<BGSScene*>("targetScene", sceneLink->scene);
            result.Set<UInt32>("phase", sceneLink->phase);
        } else {
            result.SetNone(true);
        }
        return result;
    }

	void SetSceneLink(StaticFunctionTag* base, TESTopicInfo* topicInfo, BGSScene* scene, UInt32 phase) {
        DialogueEx::SetSceneLink(topicInfo, scene, phase);
	}

    void ResetSaid(StaticFunctionTag* base, TESTopicInfo* topicInfo) {
        topicInfo->infoFlags &= ~TESTopicInfo::kFlag_HasBeenSaid;
        topicInfo->saidOnce = false;
    }
}

bool Papyrus::RegisterPapyrus(VirtualMachine * vm)
{
    RegisterFuncs(vm);
    _MESSAGE("Registered Papyrus native functions.");
    return true;
}

void Papyrus::RegisterFuncs(VirtualMachine* vm) {

	vm->RegisterFunction(
		new NativeFunction0<StaticFunctionTag, UInt32>("GetVersionCode", SCRIPT_NAME, GetVersionCode, vm));

    vm->RegisterFunction(
        new NativeFunction1<StaticFunctionTag, SceneLink, TESTopicInfo*>("GetSceneLink", SCRIPT_NAME, GetSceneLink, vm));

	vm->RegisterFunction(
		new NativeFunction3<StaticFunctionTag, void, TESTopicInfo*, BGSScene*, UInt32>("SetSceneLink", SCRIPT_NAME, SetSceneLink, vm));

    vm->RegisterFunction(
        new NativeFunction1<StaticFunctionTag, void, TESTopicInfo*>("ResetSaid", SCRIPT_NAME, ResetSaid, vm));


	vm->SetFunctionFlags(SCRIPT_NAME, "GetVersionCode", IFunction::kFunctionFlag_NoWait);
}