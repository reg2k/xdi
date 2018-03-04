#pragma once

#include <vector>
#include "ExtraTypes.h"
#include "ExtraEvents.h"

namespace DialogueEx
{
    //-------------------------
    // Data Structures
    //-------------------------
    struct DialogueOption {
        int optionID;
        TESTopicInfo* info;
        const char* promptText;
        std::string responseText;
        bool enabled;
        bool said;
        int challengeLevel;     //  0: No speech challenge
        int challengeResult;    // -1: Not attempted, 0: Failed, 1: Succeeded. For repeatable challenges, always -1.
        bool linkedToSelf;      // Whether or not this dialogue option links to itself.
        bool endsScene;         // Whether or not this dialogue option will end the scene.
        bool isBarterOption;    // Whether or not this dialogue option has the VendorInfoScript attached.
        bool isInventoryOption; // Whether or not this dialogue option has the OpenInventoryInfoScript attached.
    };

    //-------------------------
    // Init
    //-------------------------
    void Init();
    void OnGameLoaded();

    //-------------------------
    // Status
    //-------------------------
    bool IsFrameworkActive();
    extern bool g_frameworkActiveOverride;  // for debug only. defaults to false.

    //-------------------------
    // Scene Control
    //-------------------------
    SceneLink* GetSceneLink(TESTopicInfo* topicInfo);
    void SetSceneLink(TESTopicInfo* topicInfo, BGSScene* scene, int phase);
    void StartScene(BGSScene* scene, int phase);

    //-------------------------
    // Player Dialogue Control
    //-------------------------
    void SetPlayerDialogue(bool enable);
    BGSSceneActionPlayerDialogue* GetCurrentPlayerDialogueAction();
    bool SelectDialogueOption(int option);

    TESObjectREFR* GetCurrentPlayerDialogueTarget();

    // Utilities
    bool IsSceneActionWithinPhase(BGSSceneAction* action, UInt32 phase);
    TESObjectREFR* GetActionRef(BGSScene* scene, BGSSceneAction* action);

    //-------------------------
    // Topic Info
    //-------------------------
    std::vector<DialogueOption> GetDialogueOptions();
    void BuildDialogueMap(bool force = false);

    bool EvaluateInfoConditions(TESTopicInfo* info, BGSSceneAction* action, bool swap = false);

    std::vector<TESTopicInfo*> GetVanillaInfos();
    std::vector<TESTopicInfo*> GetPlayerInfos();
    TESTopicInfo* GetPlayerInfo(BGSSceneActionPlayerDialogue* playerDialogue, int optionID);
    TESTopicInfo* GetNPCInfo(BGSSceneActionPlayerDialogue* playerDialogue, int optionID);
    TESTopicInfo* GetNPCResponseInfo(BGSSceneActionNPCResponseDialogue* npcDialogue, int optionID);

    // Utilities
    BGSSceneActionNPCResponseDialogue* FindNextNPCResponseAction(BGSScene* scene, int currentPhase);

    //-------------------------
    // Hooks
    //-------------------------
    TESTopicInfo* GetInfoForPlayerDialogueOption_Hook(BGSSceneActionPlayerDialogue* playerDialogue, BGSScene* scene, TESObjectREFR* dialogueTarget, int dialogueOption);
    TESTopicInfo* GetInfoForNPCDialogueOption_Hook(BGSSceneActionPlayerDialogue* playerDialogue, BGSScene* scene, TESObjectREFR* dialogueTarget, int dialogueOption);
    TESTopicInfo* GetNPCResponseInfoForOption_Hook(BGSSceneActionNPCResponseDialogue* npcDialogue, BGSScene* scene);
    bool IsPlayerTalking_Hook(Actor* actor);

    //-------------------------
    // Utilities
    //-------------------------
    void SetXDIResult(float value);
    void SetInputEnableFlags(int type, UInt32 flags, bool enabled);
    void SetWheelZoomEnabled(bool enabled);
    void SetFavoritesEnabled(bool enabled);
    void SetMovementEnabled(bool enabled);
}

//-------------------------
// Internal
//-------------------------
namespace {
    struct DialogueHolder
    {
        BGSScene* scene;
        BGSSceneActionPlayerDialogue* playerDialogue;

        // Vector of pair of (Player topic info + vector of NPC response topic infos).
        std::vector<std::pair<TESTopicInfo*, std::vector<TESTopicInfo*>>> dialogueMap;
    };
    extern DialogueHolder g_dialogueHolder;

    // Event Sinks
    class MenuOpenCloseHandler : public BSTEventSink<MenuOpenCloseEventEx>
    {
    public:
        virtual	EventResult	ReceiveEvent(MenuOpenCloseEventEx * evn, void * dispatcher) override;
        static void Register();
    };
}