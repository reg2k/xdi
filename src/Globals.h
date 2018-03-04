#pragma once
#include "rva/RVA.h"

class UI;
class GameVM;
class DataHandler;
class PlayerCharacter;
class INISettingCollection;
class INIPrefSettingCollection;

class BGSKeyword;
class TESGlobal;

namespace G
{
    void Init();
    extern RVA<UI*>                         ui;
    extern RVA<GameVM*>                     gameVM;
    extern RVA<DataHandler*>                dataHandler;
    extern RVA<PlayerCharacter*>            player;
    extern RVA<INISettingCollection*>       iniSettings;
    extern RVA<INIPrefSettingCollection*>   iniPrefSettings;

    void OnDataLoaded();
    extern std::vector<BGSKeyword*>         activationKeywords;
    extern std::vector<TESGlobal*>          resultGlobals;
}

// Utilities
namespace {
    template <typename T>
    bool RegisterForm(std::vector<T*>& dataHolder, const char* pluginName, UInt32 formID) {
        TESForm* form = GameUtils::GetFormFromFile(pluginName, formID);
        if (form && form->formType == T::kTypeID) {
            dataHolder.push_back((T*)form);
            return true;
        } else {
            _WARNING("[%s] Warning: could not find formID %08X or it is the wrong type.", pluginName, formID);
            return false;
        }
    }
}