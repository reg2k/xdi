#pragma once
#include "rva/RVA.h"

class UI;
class GameVM;
class DataHandler;
class PlayerCharacter;
class INISettingCollection;
class INIPrefSettingCollection;

namespace G
{
    void Init();
    extern RVA<UI*>                         ui;
    extern RVA<GameVM*>                     gameVM;
    extern RVA<DataHandler*>                dataHandler;
    extern RVA<PlayerCharacter*>            player;
    extern RVA<INISettingCollection*>       iniSettings;
    extern RVA<INIPrefSettingCollection*>   iniPrefSettings;
}