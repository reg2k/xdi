#include "Scaleform.h"

#include "f4se/ScaleformValue.h"
#include "f4se/ScaleformMovie.h"
#include "f4se/ScaleformCallbacks.h"
#include "f4se/PapyrusScaleformAdapter.h"

#include "f4se/GameMenus.h"
#include "f4se/GameReferences.h"
#include "f4se/GameSettings.h"

#include "Globals.h"

#include "Settings.h"
#include "DialogueEx.h"

namespace Scaleform {
    using namespace DialogueEx;

    //---------------------
    // Scaleform Functions
    //---------------------

    // function IsFrameworkActive():Boolean;
    // Returns true if the player is currently making a dialogue selection and:
    // - The XDI keyword is on the scene.
    class IsFrameworkActive : public GFxFunctionHandler {
    public:
        virtual void Invoke(Args* args) {
            args->result->SetBool(false);

            if (DialogueEx::IsFrameworkActive())
                args->result->SetBool(true);
        }
    };

    // function GetDialogueOptions():Array;
    // Returns: [{optionID:int, prompt:String, response:String, enabled:Boolean, said:Boolean, challengeLevel:int, challengeResult:int, linkedToSelf:Boolean}, ...]
    // Prompt contains short prompt text. Response contains full dialogue text.
    // Enabled reflects whether the dialogue option has passed condition checks.
    // Said reflects whether the dialogue option has already been said by the player.
	class GetDialogueOptions : public GFxFunctionHandler {
	public:
		virtual void Invoke(Args* args) {
            GetDialogueGFXValue(args->movie->movieRoot, args->result);
		}
	};

    // function SelectDialogueOption(option:int):Boolean;
    class SelectDialogueOption : public GFxFunctionHandler {
    public:
        virtual void Invoke(Args* args) {
            if (args->numArgs < 1) return;
            if (args->args[0].GetType() != GFxValue::kType_Int) return;
            int option = args->args[0].GetInt();

            args->result->SetBool(DialogueEx::SelectDialogueOption(option));
        }
    };

    // function GetTargetName():String;
    // Returns the name of the target of the current player dialogue action.
    class GetTargetName : public GFxFunctionHandler {
    public:
        virtual void Invoke(Args* args) {
            const char* result = "";
            if (TESObjectREFR* target = GetCurrentPlayerDialogueTarget()) {
                // TODO: note this is not version-independent
                result = CALL_MEMBER_FN(target, GetReferenceName)();
            }
            args->result->SetString(result);
        }
    };

    // function GetINISetting(setting:String):*;
    // Returns the value of the given INI setting.
    class GetINISetting : public GFxFunctionHandler {
    public:
        virtual void Invoke(Args* args) {
            args->result->SetNull();
            if (args->numArgs < 1) return;
            if (args->args[0].GetType() != GFxValue::kType_String) return;

            // TODO: This is not address-independent.
            Setting* setting = ::GetINISetting(args->args[0].GetString());

            if (setting) {
                switch (setting->GetType()) {
                    case Setting::kType_Integer:
                        args->result->SetInt(setting->data.s32);
                        break;
                    case Setting::kType_Bool:
                        args->result->SetBool(setting->data.u8 != 0);
                        break;
                    case Setting::kType_Float:
                        args->result->SetNumber(setting->data.f32);
                        break;
                    case Setting::kType_String:
                        args->result->SetString(setting->data.s);
                        break;
                    default:
                        _MESSAGE("Warning: Unknown INI type %d.", setting->GetType());
                        break;
                }
            }
        }
    };

    // function GetModSetting(setting:String):*;
    // Returns the value of the given mod setting.
    class GetModSetting : public GFxFunctionHandler {
    public:
        virtual void Invoke(Args* args) {
            args->result->SetNull();
            if (args->numArgs < 1) return;
            if (args->args[0].GetType() != GFxValue::kType_String) return;

            const char* settingName = args->args[0].GetString();
            if (strlen(settingName) == 0) return;

            switch (tolower(settingName[0])) {
                case 'i':
                    args->result->SetInt(Settings::GetInt(settingName));
                    break;
                case 'b':
                    args->result->SetBool(Settings::GetBool(settingName));
                    break;
                case 'f':
                    args->result->SetNumber(Settings::GetFloat(settingName));
                    break;
                case 's':
                    args->result->SetString(Settings::GetString(settingName).c_str());
                    break;
                default:
                    _MESSAGE("WARNING: Cannot retrieve setting %s with invalid type.", settingName);
                    break;
            }
        }
    };

    // function GetSubtitlePosition():Array;
    // Returns: [xPos:Number, yPos:Number]
    // Returns null if the HUDMenu is not open.
    // TODO: This function is not version-independent. (IsMenuOpen)
    class GetSubtitlePosition_GFX : public GFxFunctionHandler {
    public:
        virtual void Invoke(Args* args) {
            args->result->SetNull();

            BSFixedString menuStr("HUDMenu");
            if ((*G::ui)->IsMenuOpen(&menuStr)) {
                IMenu* menu = (*G::ui)->GetMenu(&menuStr);
                GFxMovieRoot* movieRoot = menu->movie->movieRoot;

                movieRoot->CreateArray(args->result);

                GFxValue subtitleX; movieRoot->GetVariable(&subtitleX, "root.BottomCenterGroup_mc.SubtitleText_mc.x");
                GFxValue subtitleY; movieRoot->GetVariable(&subtitleY, "root.BottomCenterGroup_mc.SubtitleText_mc.y");

                args->result->PushBack(&subtitleX);
                args->result->PushBack(&subtitleY);
            }
        }
    };

    // function SetSubtitlePosition(xPos:Number, yPos:Number):Boolean;
    // Returns true if the HUDMenu is open and the subtitle position was set. False otherwise.
    class SetSubtitlePosition_GFX : public GFxFunctionHandler {
    public:
        virtual void Invoke(Args* args) {
            args->result->SetBool(false);

            if (args->numArgs < 2) return;
            if (args->args[0].GetType() != GFxValue::kType_Number) return;
            if (args->args[1].GetType() != GFxValue::kType_Number) return;

            BSFixedString menuStr("HUDMenu");
            if ((*G::ui)->IsMenuOpen(&menuStr)) {
                IMenu* menu = (*G::ui)->GetMenu(&menuStr);
                GFxMovieRoot* movieRoot = menu->movie->movieRoot;

                GFxValue subtitle; movieRoot->GetVariable(&subtitle, "root.BottomCenterGroup_mc.SubtitleText_mc");
                subtitle.SetMember("x", &args->args[0]);
                subtitle.SetMember("y", &args->args[1]);

                args->result->SetBool(true);
            }
        }
    };

    // function SetWheelZoomEnabled(enabled:Boolean);
    class SetWheelZoomEnabled : public GFxFunctionHandler {
    public:
        virtual void Invoke(Args* args) {
            if (args->numArgs < 1) return;
            if (args->args[0].GetType() != GFxValue::kType_Bool) return;

            bool enabled = args->args[0].GetBool();
            DialogueEx::SetWheelZoomEnabled(enabled);
        }
    };

    // function SetFavoritesEnabled(enabled:Boolean);
    class SetFavoritesEnabled : public GFxFunctionHandler {
    public:
        virtual void Invoke(Args* args) {
            if (args->numArgs < 1) return;
            if (args->args[0].GetType() != GFxValue::kType_Bool) return;

            bool enabled = args->args[0].GetBool();
            DialogueEx::SetFavoritesEnabled(enabled);
        }
    };

    // function SetMovementEnabled(enabled:Boolean);
    class SetMovementEnabled : public GFxFunctionHandler {
    public:
        virtual void Invoke(Args* args) {
            if (args->numArgs < 1) return;
            if (args->args[0].GetType() != GFxValue::kType_Bool) return;

            bool enabled = args->args[0].GetBool();
            DialogueEx::SetMovementEnabled(enabled);
        }
    };

    // function SetPlayerControls(type:int, flags:int, enabled:Boolean);
    class SetPlayerControls : public GFxFunctionHandler {
    public:
        virtual void Invoke(Args* args) {
            if (args->numArgs < 3) return;
            if (args->args[0].GetType() != GFxValue::kType_Int) return;
            if (args->args[1].GetType() != GFxValue::kType_Int) return;
            if (args->args[2].GetType() != GFxValue::kType_Bool) return;

            int type        = args->args[0].GetInt();
            int flags       = args->args[1].GetInt();
            bool enabled    = args->args[2].GetBool();
            DialogueEx::SetInputEnableFlags(type, flags, enabled);
        }
    };

    // function SetXDIResult(value:Number);
    class SetXDIResult : public GFxFunctionHandler {
    public:
        virtual void Invoke(Args* args) {
            if (args->numArgs < 1) return;
            if (args->args[0].GetType() != GFxValue::kType_Number) return;

            float value = args->args[0].GetNumber();
            DialogueEx::SetXDIResult(value);
        }
    };

    //-------------------
    // General Functions
    //-------------------

    std::pair<float, float> GetSubtitlePosition() {
        std::pair<float, float> pos;

        BSFixedString menuStr("HUDMenu");
        if ((*G::ui)->IsMenuOpen(&menuStr)) {
            IMenu* menu = (*G::ui)->GetMenu(&menuStr);
            GFxMovieRoot* movieRoot = menu->movie->movieRoot;

            GFxValue subtitleX; movieRoot->GetVariable(&subtitleX, "root.BottomCenterGroup_mc.SubtitleText_mc.x");
            GFxValue subtitleY; movieRoot->GetVariable(&subtitleY, "root.BottomCenterGroup_mc.SubtitleText_mc.y");

            pos.first = subtitleX.GetNumber();
            pos.second = subtitleY.GetNumber();
        } else {
            _MESSAGE("WARNING: Unable to retrieve the subtitle position because the HUDMenu is not open.");
        }

        return pos;
    }

    bool SetSubtitlePosition(float x, float y) {
        BSFixedString menuStr("HUDMenu");
        if ((*G::ui)->IsMenuOpen(&menuStr)) {
            IMenu* menu = (*G::ui)->GetMenu(&menuStr);
            GFxMovieRoot* movieRoot = menu->movie->movieRoot;

            GFxValue subtitle; movieRoot->GetVariable(&subtitle, "root.BottomCenterGroup_mc.SubtitleText_mc");
            subtitle.SetMember("x", &GFxValue(x));
            subtitle.SetMember("y", &GFxValue(y));
            return true;
        }
        return false;
    }
}

void Scaleform::RegisterFuncs(GFxValue* codeObj, GFxMovieRoot* movieRoot) {
    RegisterFunction<IsFrameworkActive>(codeObj, movieRoot, "IsFrameworkActive");
    RegisterFunction<GetDialogueOptions>(codeObj, movieRoot, "GetDialogueOptions");
    RegisterFunction<SelectDialogueOption>(codeObj, movieRoot, "SelectDialogueOption");
    RegisterFunction<GetTargetName>(codeObj, movieRoot, "GetTargetName");

    RegisterFunction<SetWheelZoomEnabled>(codeObj, movieRoot, "SetWheelZoomEnabled");
    RegisterFunction<SetFavoritesEnabled>(codeObj, movieRoot, "SetFavoritesEnabled");
    RegisterFunction<SetMovementEnabled>(codeObj, movieRoot, "SetMovementEnabled");
    RegisterFunction<SetPlayerControls>(codeObj, movieRoot, "SetPlayerControls");

    RegisterFunction<GetINISetting>(codeObj, movieRoot, "GetINISetting");
    RegisterFunction<GetModSetting>(codeObj, movieRoot, "GetModSetting");

    RegisterFunction<GetSubtitlePosition_GFX>(codeObj, movieRoot, "GetSubtitlePosition");
    RegisterFunction<SetSubtitlePosition_GFX>(codeObj, movieRoot, "SetSubtitlePosition");

    RegisterFunction<SetXDIResult>(codeObj, movieRoot, "SetXDIResult");
}

bool Scaleform::RegisterScaleform(GFxMovieView * view, GFxValue * f4se_root)
{
    GFxMovieRoot* movieRoot = view->movieRoot;

    GFxValue currentSWFPath;
    const char* currentSWFPathString = nullptr;

    if (movieRoot->GetVariable(&currentSWFPath, "root.loaderInfo.url")) {
        currentSWFPathString = currentSWFPath.GetString();
    } else {
        _MESSAGE("WARNING: Scaleform registration failed.");
    }

    // Look for the menu that we want to inject into.
    if (strcmp(currentSWFPathString, "Interface/DialogueMenu.swf") == 0) {
        // Register native code handlers
        GFxValue codeObj;
        movieRoot->GetVariable(&codeObj, "root.Menu_mc.BGSCodeObj");
        RegisterFuncs(&codeObj, movieRoot);

        movieRoot->Invoke("root.XDI_Init", nullptr, nullptr, 0);
    }

    return true;
}

//--------------------------
// Internal functions
//--------------------------
namespace {
    void GetDialogueGFXValue(GFxMovieRoot* movieRoot, GFxValue* outValue) {
        using namespace DialogueEx;

        movieRoot->CreateArray(outValue);

        if (auto playerDialogue = GetCurrentPlayerDialogueAction()) {
            std::vector<DialogueOption> options = GetDialogueOptions();

            for (auto option : options) {
                GFxValue optionIDValue, promptValue, responseValue, enabledValue, saidValue, challengeLevelValue, challengeResultValue, linkedToSelfValue;

                optionIDValue.SetInt(option.optionID);
                promptValue.SetString(option.promptText);
                responseValue.SetString(option.responseText.c_str());
                enabledValue.SetBool(option.enabled);
                saidValue.SetBool(option.said);
                challengeLevelValue.SetInt(option.challengeLevel);
                challengeResultValue.SetInt(option.challengeResult);
                linkedToSelfValue.SetBool(option.linkedToSelf);

                GFxValue dialogueValue;
                movieRoot->CreateObject(&dialogueValue);
                dialogueValue.SetMember("optionID", &optionIDValue);
                dialogueValue.SetMember("prompt", &promptValue);
                dialogueValue.SetMember("response", &responseValue);
                dialogueValue.SetMember("enabled", &enabledValue);
                dialogueValue.SetMember("said", &saidValue);
                dialogueValue.SetMember("challengeLevel", &challengeLevelValue);
                dialogueValue.SetMember("challengeResult", &challengeResultValue);
                dialogueValue.SetMember("linkedToSelf", &linkedToSelfValue);
                dialogueValue.SetMember("endsScene", &GFxValue(option.endsScene));
                outValue->PushBack(&dialogueValue);
            }
        } else {
            // Player dialogue option not currently available.
            _MESSAGE("[GetDialogueOptions] WARNING: Player dialogue not currently available. No dialogue will be retrieved.");
        }
    }
}