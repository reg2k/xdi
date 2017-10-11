#pragma once
#include "RVA.h"

class UI;
class GameVM;
class DataHandler;
class PlayerCharacter;

namespace G
{
    void Init();
    extern RVA<UI*>                 ui;
    extern RVA<GameVM*>             gameVM;
    extern RVA<DataHandler*>        dataHandler;
    extern RVA<PlayerCharacter*>    player;
}