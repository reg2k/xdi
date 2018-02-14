#include "DialogueEx.h"

#include "Config.h"
#include "rva/RVA.h"

#include "Utils.h"
#include "Globals.h"
#include "GameUtils.h"

#include "Scaleform.h"

#include "f4se_common/SafeWrite.h"
#include "f4se_common/BranchTrampoline.h"

#include "f4se/GameData.h"
#include "f4se/GameReferences.h"
#include "f4se/GameRTTI.h"
#include "f4se/GameMenus.h"

namespace DialogueEx {
    //--------------------
    // Addresses [23]
    //--------------------

    RVA <uintptr_t> GetInfoForPlayerDialogueOption_HookTarget                   ({{RUNTIME_VERSION_1_10_64, 0x002C5241}}, "4C 8B C3 41 B9 ? ? ? ? 49 8B D5", 0xF);
    RVA <uintptr_t> GetInfoForNPCDialogueOption_HookTarget                      ({{RUNTIME_VERSION_1_10_64, 0x002C55D7}}, "4C 8B 05 ? ? ? ? 41 B9 ? ? ? ? 49 8B D5", 0x13);
    RVA <uintptr_t> GetNPCResponseInfoForOption_HookTarget                      ({{RUNTIME_VERSION_1_10_64, 0x002C3408}}, "49 8B D5 49 8B CF E8 ? ? ? ? 48 85 C0", 0x6);
    RVA <uintptr_t> IsPlayerTalking_Call                                        ({{RUNTIME_VERSION_1_10_64, 0x00D913C3}}, "FF 90 70 02 00 00 84 C0 0F 85 ? ? ? ? 48 8B 05 ? ? ? ? 80 78 4E 00");
    RVA <uintptr_t> DialogueLimiter_Check                                       ({{RUNTIME_VERSION_1_10_64, 0x0126B99E}}, "0F 83 ? ? ? ? 48 8B 0D ? ? ? ? 48 83 C4 48"); //
    RVA <uintptr_t> StartPlayerDialogue_Call                                    ({{RUNTIME_VERSION_1_10_64, 0x002C548E}}, "48 8B C8 C6 44 24 ? ? C6 44 24 ? ? E8 ? ? ? ? 84 C0");
    RVA <uintptr_t> SwitchToPlayerCC                                            ({{RUNTIME_VERSION_1_10_64, 0x0083CC1A}}, "C7 87 ? ? ? ? ? ? ? ? 45 84 E4"); // C7 87 D4 01 00 00 01 00 00 00

    RVA <_GetQuestAliasHandle>          GetQuestAliasHandle                     ({{RUNTIME_VERSION_1_10_64, 0x005DCDE0}}, "44 8B 45 18 49 8B 0F", 0xC, 1, 5);
    RVA <_InitSceneActions>             InitSceneActions_Internal               ({{RUNTIME_VERSION_1_10_64, 0x0058EDC0}}, "48 8B 5F 38 48 3B F3", 0x14, 1, 5);
    RVA <_StartScene>                   StartScene_Internal                     ({{RUNTIME_VERSION_1_10_64, 0x0058D8F0}}, "40 53 48 83 EC 40 81 61 ? ? ? ? ?");
    RVA <_StartDialogueCamera>          StartDialogueCamera_Internal            ({{RUNTIME_VERSION_1_10_64, 0x00EA2EE0}}, "88 54 24 10 55 56 41 56"); //
    RVA <_SelectDialogueOption>         SelectDialogueOption_Internal           ({{RUNTIME_VERSION_1_10_64, 0x00EB8AD0}}, "4C 8B DC 56 48 83 EC 30");
    RVA <_GetSpeechChallengeLevel>      GetSpeechChallengeLevel                 ({{RUNTIME_VERSION_1_10_64, 0x006218E0}}, "49 8B CE 48 8B B0 ? ? ? ?", 0xA, 1, 5);
    RVA <_GetSpeechChallengeState>      GetSpeechChallengeState                 ({{RUNTIME_VERSION_1_10_64, 0x00621B30}}, "E8 ? ? ? ? 83 F8 FF 74 4C 8B C7", 0, 1, 5);
    RVA <_GetInfoGroupParent>           GetInfoGroupParent                      ({{RUNTIME_VERSION_1_10_64, 0x00621280}}, "48 8B CB E8 ? ? ? ? 48 8B F8 48 85 C0 74 28", 0x3, 1, 5);
    RVA <_PostDialogueHandler>          PostDialogueHandler_Internal            ({{RUNTIME_VERSION_1_10_64, 0x00DA9970}}, "40 56 41 57 48 81 EC ? ? ? ? 48 8B F1");
    RVA <_SetPlayerControlsEnabled>     SetPlayerControls1_Internal             ({{RUNTIME_VERSION_1_10_64, 0x01B21D20}}, "48 8B 0D ? ? ? ? 45 33 C9 44 8B C7", 0x15, 1, 5);
    RVA <_SetPlayerControlsEnabled>     SetPlayerControls2_Internal             ({{RUNTIME_VERSION_1_10_64, 0x01B21E40}}, "48 8B 0D ? ? ? ? 45 33 C9 44 8B C7", 0x3C, 1, 5);
    RVA <_DoTextReplacement>            DoTextReplacement_Internal              ({{RUNTIME_VERSION_1_10_64, 0x005752B0}}, "44 8B 45 50 48 8D 4C 24 ? 48 8B D5", 0xC, 1, 5);

    // Note: the following addresses are offset-adjusted in the Init() method.
    RVA <tHashSet<SceneLink, TESTopicInfo*>>            g_sceneLinks            ({{RUNTIME_VERSION_1_10_64, 0x0374FB20}}, "48 83 C7 20 85 F6 75 DF", 0x8, 3, 7);
    RVA <tHashSet<DialoguePrompt, TESTopicInfo*>>       g_dialoguePrompts       ({{RUNTIME_VERSION_1_10_64, 0x0374FAC0}}, "48 83 C6 18 48 83 C7 18", 0xC, 3, 7);

    RVA <MenuTopicManager*>     g_menuTopicManager                              ({{RUNTIME_VERSION_1_10_64, 0x05907BB0}}, "48 8B 0D ? ? ? ? 48 83 C4 48", 0, 3, 7);
    RVA <void*>                 g_inputEnableManager                            ({{RUNTIME_VERSION_1_10_64, 0x05909800}}, "48 8B 0D ? ? ? ? 45 33 C9 44 8B C7", 0, 3, 7);

    _GetInfoForDialogueOption       GetInfoForPlayerDialogueOption_Original;
    _GetInfoForDialogueOption       GetInfoForNPCDialogueOption_Original;
    _GetNPCResponseInfoForOption    GetNPCResponseInfoForOption_Original;

    bool g_frameworkActiveOverride = false; // for debug only

    //-----------------------
    // Scene Links
    //-----------------------

    SceneLink* GetSceneLink(TESTopicInfo* topicInfo) {
        return g_sceneLinks->Find(&topicInfo);
    }

    void SetSceneLink(TESTopicInfo* topicInfo, BGSScene* scene, int phase) {
        if (!topicInfo) return;
        SceneLink* sceneLink = g_sceneLinks->Find(&topicInfo);
        if (sceneLink) {
            sceneLink->scene = scene;
            sceneLink->phase = phase;
        } else {
            SceneLink* link = (SceneLink*)Heap_Allocate(sizeof(SceneLink));
            link->key = topicInfo;
            link->scene = scene;
            link->phase = phase;
            g_sceneLinks->Add(link);
        }
    }

    void StartScene(BGSScene* scene, int phase) {
        if (!scene) return;
        BGSScene* currentScene = (*G::player)->GetCurrentScene();

        if (currentScene && currentScene != scene)
            currentScene->status |= BGSScene::kFlag_Stopped;

        InitSceneActions_Internal(scene);
        scene->status &= ~(BGSScene::kFlag_Paused);
        StartScene_Internal(scene, 1);
        scene->nextPhase = phase;
    }

    //-------------------------
    // Player Dialogue Control
    //-------------------------

    // Controls whether player dialogue is enabled.
    void SetPlayerDialogue(bool enable) {
        if (enable) {
            // Restore player dialogue starting
            unsigned char data[] = { 0x48, 0x8B, 0xC8 };
            SafeWriteBuf(StartPlayerDialogue_Call.GetUIntPtr(), &data, sizeof(data));

            // Restore dialogue cam switching to player
            // C7 87 D4 01 00 00 01 00 00 00
            // ------------------^^
            SafeWrite8(SwitchToPlayerCC.GetUIntPtr() + 6, 0x1);

        } else {
            // Disable player dialogue starting
            unsigned char data[] = { 0xEB, 0x14, 0x90 };
            SafeWriteBuf(StartPlayerDialogue_Call.GetUIntPtr(), &data, sizeof(data));

            // Disable dialogue cam switching to player
            SafeWrite8(SwitchToPlayerCC.GetUIntPtr() + 6, 0x2);

            // Todo: disable player interrupt dialogue
        }
    }

    // Checks to see whether the given scene action is active in the given phase.
    bool IsSceneActionWithinPhase(BGSSceneAction* action, UInt32 phase) {
        if (action->startPhase <= phase && action->endPhase >= phase)
            return true;
        else
            return false;
    }

    // Returns the reference associated with the action.
    TESObjectREFR* GetActionRef(BGSScene* scene, BGSSceneAction* action){
        if (scene) {
            UInt32          targetHandle = 0;
            TESObjectREFR*  targetRef    = nullptr;
            GetQuestAliasHandle(scene->owningQuest, &targetHandle, action->aliasID);
            if (targetHandle) {
                LookupREFRByHandle(&targetHandle, &targetRef);
                return targetRef;
            }
        }
        return nullptr;
    }

    // Returns the currently executing player dialogue action, or NULL if no player dialogue action is currently active.
    BGSSceneActionPlayerDialogue* GetCurrentPlayerDialogueAction() {
        BGSScene* scene = (*G::player)->GetCurrentScene();
        if (scene) {
            for (int i = 0; i < scene->actions.count; i++) {
                BGSSceneAction* action = scene->actions[i];
                if (action->GetType() == BGSSceneAction::kType_PlayerDialogue) {
                    if (action->status & BGSSceneAction::kStatus_Running || IsSceneActionWithinPhase(action, scene->currentPhase)) {
                        return DYNAMIC_CAST(action, BGSSceneAction, BGSSceneActionPlayerDialogue);
                    }
                }
            }
        }
        return nullptr;
    }

    bool SelectDialogueOption(int option) {
        if (!(*g_menuTopicManager)->awaitingPlayerInput) return false;
        if (auto playerDialogue = GetCurrentPlayerDialogueAction()) {
            if (IsFrameworkActive()) {
                // We're using option 5 and up for additional options. (Opt 5 = Opt 0)
                SelectDialogueOption_Internal(*G::player, option + 5);
            } else {
                SelectDialogueOption_Internal(*G::player, option);
            }
            return true;
        } else {
            return false;
        }
    }
    
    // Returns the target of the current player dialogue action, or NULL if no player dialogue action is currently active.
    TESObjectREFR* GetCurrentPlayerDialogueTarget() {
        if (auto playerDialogue = GetCurrentPlayerDialogueAction()) {
            UInt32          targetHandle = 0;
            TESObjectREFR*  targetRef = nullptr;
            BGSScene* scene = (*G::player)->GetCurrentScene();
            if (scene) {
                GetQuestAliasHandle(scene->owningQuest, &targetHandle, playerDialogue->aliasID);
            }
            if (targetHandle) {
                LookupREFRByHandle(&targetHandle, &targetRef);
                return targetRef;
            }
        }
        return nullptr;
    }

    //-------------------------
    // Topic Info
    //-------------------------

    bool EvaluateInfoConditions(TESTopicInfo* info, BGSSceneAction* action, bool swap) {
        Condition* conditions = info->conditions;
        if (!conditions) return true;

        // Get condition target.
        UInt32          targetHandle = 0;
        TESObjectREFR*  targetRef = nullptr;
        BGSScene* scene = (*G::player)->GetCurrentScene();
        if (scene) {
            GetQuestAliasHandle(scene->owningQuest, &targetHandle, action->aliasID);
        }
        if (targetHandle) {
            LookupREFRByHandle(&targetHandle, &targetRef);
        } else {
            targetRef = *G::player;
        }

        // Test against conditions. (subject = player)
        TESObjectREFR *refA, *refB;
        if (!swap) {
            refA = *G::player;
            refB = targetRef;
        } else {
            refA = targetRef;
            refB = *G::player;
        }
        return EvaluationConditions(&conditions, refA, refB);
    }

    std::vector<DialogueOption> GetDialogueOptions()
    {
        std::vector<DialogueOption> options;

        if (auto playerDialogue = GetCurrentPlayerDialogueAction()) {

            bool active = IsFrameworkActive();

            std::vector<TESTopicInfo*> infos;
            if (active) {
                infos = GetPlayerInfos();
            } else {
                infos = GetVanillaInfos();
            }

            BGSScene* currentScene = (*G::player)->GetCurrentScene();

            for (int i = 0; i < infos.size(); i++) {
                TESTopicInfo* info = infos[i];
                if (!info) continue;

                TESTopicInfo* originalInfo = info;
                while (info->sharedInfo) {
                    info = info->sharedInfo;
                }

                int vanillaDialogueOrder[] = { 3, 0, 1, 2 };   // Question, Positive, Negative, Neutral
                
                // Get prompt
                auto prompt = g_dialoguePrompts->Find(&info);

                // Get response and perform text replacement
                std::string responseText = "";
                if (info->responses) {
                    BSStringEx str(info->responses->GetNthItem(0)->Get<char>());
                    if (active && info->topic && info->topic->owningQuest)
                        DoTextReplacement_Internal(&str, info->topic->owningQuest, Utils::GetOffset<UInt32>(info->topic->owningQuest, 0x50));
                    responseText = str.Get();
                }

                // Get NPC response TopicInfo for dialogue cues.
                TESTopicInfo* npcResponseInfo = active ? GetNPCInfo(playerDialogue, i) : GetInfoForNPCDialogueOption_Original(playerDialogue, currentScene, *G::player, vanillaDialogueOrder[i]);
                if (!npcResponseInfo) {
                    // No NPC response info - look one phase ahead (only) for a NPC Response action.
                    if (auto npcResponseAction = FindNextNPCResponseAction(currentScene, currentScene->currentPhase)) {
                        if (active) {
                            npcResponseInfo = GetNPCResponseInfo(npcResponseAction, i);
                        } else {
                            UInt32* playerDialogueOption = Utils::GetOffsetPtr<UInt32>(*G::player, 0xD3C);
                            *playerDialogueOption = vanillaDialogueOrder[i];
                            npcResponseInfo = GetNPCResponseInfoForOption_Original(npcResponseAction, currentScene);
                        }
                    }
                }

                // Get scene link for response
                SceneLink* sceneLink = npcResponseInfo ? GetSceneLink(npcResponseInfo) : nullptr;

                DialogueOption option       = {};
                option.optionID             = active ? i : vanillaDialogueOrder[i];
                option.info                 = info;
                option.promptText           = prompt ? prompt->prompt.c_str() : "";
                option.responseText         = responseText;
                option.enabled              = active ? EvaluateInfoConditions(originalInfo, playerDialogue) : true;
                option.said                 = (info->infoFlags & TESTopicInfo::kFlag_HasBeenSaid) != 0;
                option.challengeLevel       = GetSpeechChallengeLevel(info);
                option.challengeResult      = GetSpeechChallengeState(info);
                option.linkedToSelf         = sceneLink ? (currentScene == sceneLink->scene && playerDialogue->startPhase >= sceneLink->phase && playerDialogue->endPhase <= sceneLink->phase) : false;
                option.endsScene            = npcResponseInfo ? (npcResponseInfo->infoFlags & TESTopicInfo::kFlag_EndRunningScene) != 0 : false;
                option.isBarterOption       = npcResponseInfo ? GameUtils::HasVMScript(npcResponseInfo, "VendorInfoScript") : false;
                option.isInventoryOption    = npcResponseInfo ? GameUtils::HasVMScript(npcResponseInfo, "OpenInventoryInfoScript") : false;
                options.push_back(option);
            }
        }

        return options;
    }

    std::vector<TESTopicInfo*> GetVanillaInfos() {
        std::vector<TESTopicInfo*> infos;

        if (auto playerDialogue = GetCurrentPlayerDialogueAction()) {
            BGSScene* scene = (*G::player)->GetCurrentScene();

            // Get dialogue target
            UInt32          targetHandle = 0;
            TESObjectREFR*  targetRef    = nullptr;
            GetQuestAliasHandle(scene->owningQuest, &targetHandle, playerDialogue->aliasID);
            LookupREFRByHandle(&targetHandle, &targetRef);

            int dialogueOrder[] = { 3, 0, 1, 2 };   // Question, Positive, Negative, Neutral
            for (int i = 0; i < 4; i++) {
                TESTopicInfo* info = GetInfoForPlayerDialogueOption_Original(playerDialogue, scene, targetRef, dialogueOrder[i]);
                infos.push_back(info);
            }
        }
        return infos;
    }

    void BuildDialogueMap(bool force) {
        BGSSceneActionPlayerDialogue* playerDialogue = GetCurrentPlayerDialogueAction();
        if (!playerDialogue) return;
        BGSScene* currentScene = (*G::player)->GetCurrentScene();

        // If we already have a map for this scene + scene action then return.
        if (!force && g_dialogueHolder.scene == currentScene && g_dialogueHolder.playerDialogue == playerDialogue) return;

        g_dialogueHolder.dialogueMap.clear();

        for (int c = 0; c < 4; c++) {
            TESTopic* playerTopic   = playerDialogue->response[c];
            TESTopic* npcTopic      = playerDialogue->npcResponse[c];
            int playerInfoCount     = playerTopic->topicInfoCount;
            int npcInfoCount        = npcTopic ? npcTopic->topicInfoCount : 0;

            // Loop through each player TopicInfo in the dialogue topic.
            for (int i = 0; i < playerInfoCount; i++) {
                TESTopicInfo* playerInfo = playerTopic->topicInfos[i];
                if (!playerInfo->responses && !playerInfo->sharedInfo) continue;    // Skip over infos with no content.

                std::vector<TESTopicInfo*> npcResponses;

                // Look for next non-empty player info
                int nextPlayerInfoIdx;
                for (nextPlayerInfoIdx = i + 1; nextPlayerInfoIdx < playerInfoCount; nextPlayerInfoIdx++) {
                    TESTopicInfo* nextPlayerInfo = playerTopic->topicInfos[nextPlayerInfoIdx];
                    if (nextPlayerInfo->responses || nextPlayerInfo->sharedInfo) {
                        // Found it
                        break;
                    }
                }
                if (nextPlayerInfoIdx == playerInfoCount && npcInfoCount > playerInfoCount) {
                    // Reached the end of the player info list but there are still NPC infos.
                    // Associate the remaining NPC infos with the current player info index.
                    nextPlayerInfoIdx = npcInfoCount;
                }

                // Add all NPC infos from current player info index to next player info index.
                for (int k = i; k < nextPlayerInfoIdx; k++) {
                    if (k < npcInfoCount) {
                        TESTopicInfo* npcInfo = npcTopic->topicInfos[k];
                        //if (!npcInfo->responses && !npcInfo->sharedInfo) continue;  // Skip empty responses.
                        npcResponses.push_back(npcInfo);
                    }
                }

                g_dialogueHolder.dialogueMap.push_back(std::make_pair(playerInfo, npcResponses));
            }
        }

        g_dialogueHolder.scene = currentScene;
        g_dialogueHolder.playerDialogue = playerDialogue;

        // debug view map
        /*for (auto const& info : g_dialogueHolder.dialogueMap) {
            _MESSAGE("For info %08X", info.first->formID);
            auto const& responses = info.second;
            for (auto const& response : responses) {
                _MESSAGE("Response: %08X", response->formID);
            }
        }*/

    }

    // Returns player response TopicInfos.
    std::vector<TESTopicInfo*> GetPlayerInfos() {
        BuildDialogueMap();
        std::vector<TESTopicInfo*> infos;
        for (auto const& info : g_dialogueHolder.dialogueMap) {
            infos.push_back(info.first);
        }
        return infos;        
    }

    TESTopicInfo* GetPlayerInfo(BGSSceneActionPlayerDialogue* playerDialogue, int optionID) {
        BuildDialogueMap();
        if (optionID < g_dialogueHolder.dialogueMap.size()) {
            return g_dialogueHolder.dialogueMap[optionID].first;
        } else {
            return nullptr;
        }
    }

    // Returns the first NPC response info that passes its condition check.
    // Todo: Handle Random and Random End flagged infos.
    // Say once infos?
    TESTopicInfo* GetNPCInfo(BGSSceneActionPlayerDialogue* playerDialogue, int optionID)
    {
        BuildDialogueMap();
        auto npcInfos = g_dialogueHolder.dialogueMap[optionID].second;
        for (TESTopicInfo* info : npcInfos) {
            if (info->flags & TESForm::kFlag_IsDeleted) continue;
            if (EvaluateInfoConditions(info, playerDialogue, true)) {
                return info;
            }
        }
        // All the infos failed their condition checks..
        return nullptr;
    }

    // Returns the first NPC response info in a NPC Response scene action that passes its condition check.
    // Allows infos to be grouped via info groups.
    TESTopicInfo* GetNPCResponseInfo(BGSSceneActionNPCResponseDialogue* npcDialogue, int optionID)
    {
        // Vector of vector of TESTopicInfo
        // infos[optionID][items inside the infogroup]
        std::vector<std::vector<TESTopicInfo*>> infos(optionID+1, std::vector<TESTopicInfo*>());

        std::map<TESTopicInfo*, int> infoGroupMap;
        int idx = 0;    // The next available free slot in the vector.

        for (int c = 0; c < 4; c++) {
            TESTopic* topic = npcDialogue->response[c];
            int infoCount = topic ? topic->topicInfoCount : 0;            

            // Loop through all TopicInfos while idx <= optionID+1
            for (int i = 0; i < infoCount && idx <= optionID+1; i++) {
                TESTopicInfo* info = topic->topicInfos[i];

                if (info->flags & TESTopicInfo::kFlag_InfoGroup) {
                    // This is an info group.
                    infoGroupMap[info] = idx++;
                } else {
                    if (TESTopicInfo* infoGroupParent = GetInfoGroupParent(info)) {
                        // Info has a parent.
                        auto index = infoGroupMap.find(infoGroupParent);
                        if (index != infoGroupMap.end()) {
                            // Put it into the parent's slot.
                            infos[index->second].push_back(info);
                        }
                    } else {
                        // No parent, add normally
                        if (idx <= optionID)
                            infos[idx++].push_back(info);
                        else
                            break;
                    }
                }
            }
        }

        // Evaluate conditions and pick best from group
        std::vector<TESTopicInfo*> infoGroup = infos[optionID];

        for (TESTopicInfo* info : infoGroup) {
            if (EvaluateInfoConditions(info, npcDialogue, true)) {
                if (info->responses || info->sharedInfo) {
                    return info;
                } else {
                    return nullptr; // Return nullptr if this is an empty info.
                }
            }
        }

        // Nothing was found.
        return nullptr;
    }

    // Returns the NPC Response Action in the next phase, if present. Otherwise returns NULL.
    // TODO: Should check phase conditions.
    BGSSceneActionNPCResponseDialogue* FindNextNPCResponseAction(BGSScene* scene, int currentPhase) {
        if (scene) {
            for (int i = 0; i < scene->actions.count; i++) {
                BGSSceneAction* action = scene->actions[i];
                if (action->startPhase == currentPhase + 1) {
                    // Check that the action is a NPC Response action and that the alias is filled for this action.
                    // If the alias is not filled, any conditions that check the subject will cause a CTD.
                    if (action->GetType() == BGSSceneAction::kType_NPCResponseDialogue && GetActionRef(scene, action)) {
                        return DYNAMIC_CAST(action, BGSSceneAction, BGSSceneActionNPCResponseDialogue);
                    }
                }
            }
        }
        return nullptr;
    }

    //-------------------------
    // DialogueEx Init
    //-------------------------

    void Init()
    {
        // Hook dialogue option -> TopicInfo function for responses.
        GetInfoForPlayerDialogueOption_Original = Utils::Hook(GetInfoForPlayerDialogueOption_Hook, GetInfoForPlayerDialogueOption_HookTarget.GetUIntPtr());
        GetInfoForNPCDialogueOption_Original = Utils::Hook(GetInfoForNPCDialogueOption_Hook, GetInfoForNPCDialogueOption_HookTarget.GetUIntPtr());
        GetNPCResponseInfoForOption_Original = Utils::Hook(GetNPCResponseInfoForOption_Hook, GetNPCResponseInfoForOption_HookTarget.GetUIntPtr());

        // Hook IsPlayerTalking
        g_branchTrampoline.Write6Call(IsPlayerTalking_Call.GetUIntPtr(), (uintptr_t)IsPlayerTalking_Hook);

        // Disable dialogue limiter
        //UInt64 data = 0x909090909090;
        //SafeWriteBuf(DialogueLimiter_Check.GetUIntPtr(), &data, 6);

        // Calculate addresses - move -0x28 to get to the start of the hashset.
        g_sceneLinks.SetEffective(g_sceneLinks.GetUIntPtr() - 0x28);
        g_dialoguePrompts.SetEffective(g_dialoguePrompts.GetUIntPtr() - 0x28);
    }

    void OnGameLoaded()
    {
        MenuOpenCloseHandler::Register();
    }

    //-------------------------
    // Hooks
    //-------------------------

    // Player Dialogue Option -> Player TopicInfo (after making selection)
    TESTopicInfo* GetInfoForPlayerDialogueOption_Hook(BGSSceneActionPlayerDialogue* playerDialogue, BGSScene* scene, TESObjectREFR* dialogueTarget, int dialogueOption)
    {
        // Use options >5 for custom selections.
        if (playerDialogue->selectedOption >= 5) {
            int selectedOption = playerDialogue->selectedOption - 5;
            TESTopicInfo* info = GetPlayerInfo(playerDialogue, selectedOption);

            if (info) {
                SetPlayerDialogue(false);   // Disable player dialogue
                info->infoFlags |= TESTopicInfo::kFlag_HasBeenSaid;   // Mark the info as 'said'.

                SceneLink* sceneLink = GetSceneLink(info);
                if (sceneLink) {
                    // This is necessary as no response => no pre/post TopicInfo actions will run.
                    _MESSAGE("Following scene link from player dialogue.");
                    StartScene(sceneLink->scene, sceneLink->phase);
                }
                
            } else {
                // Info not found...
                // Reset selectedOption to 4 so that the game doesn't keep trying to ask us for a non-existent TopicInfo.
                playerDialogue->selectedOption = 4;
            }

            return info;
        }
        SetPlayerDialogue(true);   // Re-enable player dialogue
        return GetInfoForPlayerDialogueOption_Original(playerDialogue, scene, dialogueTarget, dialogueOption);
    }

    // Player Dialogue Option -> NPC TopicInfo (after making selection)
    TESTopicInfo* GetInfoForNPCDialogueOption_Hook(BGSSceneActionPlayerDialogue* playerDialogue, BGSScene* scene, TESObjectREFR* dialogueTarget, int dialogueOption)
    {
        // Use options >5 for custom selections.
        if (playerDialogue->selectedOption >= 5) {
            int selectedOption = playerDialogue->selectedOption - 5;

            TESTopicInfo* info = GetNPCInfo(playerDialogue, selectedOption);
            if (info) {
                // If the player info was a speech challenge then trigger the post-dialogue handler to display the challenge-success animation.
                // This needs to be done explicitly as the challenge-success animation is coded to trigger after the player dialogue-elapsed timer reaches zero.
                // Since player dialogue length is always zero with the framework active, the timed code will not run and an explicit call needs to be made.
                if (auto playerInfo = GetPlayerInfo(playerDialogue, selectedOption)) {
                    if (GetSpeechChallengeLevel(playerInfo) > 0) {
                        PostDialogueHandler_Internal(*G::player, 1);
                    }
                }
            }

            return info;
        }
        return GetInfoForPlayerDialogueOption_Original(playerDialogue, scene, dialogueTarget, dialogueOption);
    }

    TESTopicInfo* GetNPCResponseInfoForOption_Hook(BGSSceneActionNPCResponseDialogue* npcDialogue, BGSScene* scene)
    {
        UInt32 dialogueOption = Utils::GetOffset<UInt32>(*G::player, 0xD3C);

        // Use options >5 for custom selections.
        if (dialogueOption >= 5) {
            int selectedOption = dialogueOption - 5;

            TESTopicInfo* info = GetNPCResponseInfo(npcDialogue, selectedOption);
            return info;
        }

        return GetNPCResponseInfoForOption_Original(npcDialogue, scene);
    }

    // Vanilla has a bug where player interrupts would delay the second SetButtonText callback.
    // One of the conditions for the second callback is when IsActorTalking on the player returns false.
    // IsActorTalking only returns false when when dialogueTimeLeft == 0, so if the player is speaking an interrupt then the interrupt will delay the call.
    // This hook will return false if the menu topic manager indicates that it's ready to receive player input.
    bool IsPlayerTalking_Hook(Actor* actor)
    {
        if ((*g_menuTopicManager)->awaitingPlayerInput) {
            return false;
        } else {
            return Utils::GetVirtualFunction<_IsActorTalking>(actor, _IsActorTalking_VtblIdx)(actor);
        }
    }

    //-------------------------
    // Utilities
    //-------------------------

    void SetXDIResult(float value) {
        static TESGlobal* resultGlobal = nullptr;

        if (!resultGlobal) {
            const ModInfo* mod = (*G::dataHandler)->LookupModByName(GAME_PLUGIN_NAME);
            if (mod && mod->modIndex != -1) {
                UInt32 formID = GAME_RESULT_ID | (mod->modIndex) << 24;
                TESForm* form = LookupFormByID(formID);
                if (!form) {
                    _MESSAGE("WARNING: Failed to retrieve XDI_Result from %s.", GAME_PLUGIN_NAME);
                    return;
                }
                if (form->formType != TESGlobal::kTypeID) {
                    _MESSAGE("WARNING: Form type mismatch.", GAME_PLUGIN_NAME);
                    return;
                }
                resultGlobal = reinterpret_cast<TESGlobal*>(form);
            } else {
                _MESSAGE("WARNING: %s is not loaded.", GAME_PLUGIN_NAME);
            }
        }

        if (resultGlobal) {
            resultGlobal->value = value;
        }
    }

    void SetInputEnableFlags(int type, UInt32 flags, bool enabled) {
        BSFixedString menuName("DialogueMenu");
        if ((*G::ui)->IsMenuOpen(menuName)) {
            IMenu* menu = (*G::ui)->GetMenu(menuName);
            UInt32* pInputLayerID = Utils::GetOffset<UInt32*>(menu, 0x158);
            if (type == 1) {
                SetPlayerControls1_Internal(*g_inputEnableManager, *pInputLayerID, flags, enabled, 2);
            } else {
                SetPlayerControls2_Internal(*g_inputEnableManager, *pInputLayerID, flags, enabled, 2);
            }
        }
    }

    // Controls the Wheel Zoom restriction on the DialogueMenu input layer.
    // This is so that players can scroll in the dialogue menu without switching to third-person.
    void SetWheelZoomEnabled(bool enabled) {
        SetInputEnableFlags(1, InputEnableManager::kInputLayer_WheelZoom, enabled);
    }

    // For dpad navigation on gamepads
    void SetFavoritesEnabled(bool enabled) {
        SetInputEnableFlags(2, InputEnableManager::kInputLayer_Favorites, enabled);
    }

    // For time-limited dialogue decisions.
    void SetMovementEnabled(bool enabled) {
        SetInputEnableFlags(1, InputEnableManager::kInputLayer_Movement, enabled);
    }

    //-------------------------
    // Status
    //-------------------------

    // Returns true if the player is currently making a dialogue selection and:
    // - The XDI keyword is on the scene.
    bool IsFrameworkActive()
    {
        if (auto playerDialogue = GetCurrentPlayerDialogueAction()) {
            if (g_frameworkActiveOverride) return true;

            // If the scene has the framework activation keyword then return true.
            if (BGSKeyword* activationKeyword = GetFrameworkActivationKeyword()) {
                BGSScene* currentScene = (*G::player)->GetCurrentScene();
                IKeywordFormBase* keywordFormBase = DYNAMIC_CAST(currentScene, BGSScene, IKeywordFormBase);

                if (keywordFormBase) {
                    auto HasKeyword_Internal = Utils::GetVirtualFunction<_IKeywordFormBase_HasKeyword>(keywordFormBase, 1);
                    if (HasKeyword_Internal(keywordFormBase, activationKeyword, 0)) {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    BGSKeyword* GetFrameworkActivationKeyword()
    {
        static BGSKeyword* g_activateFrameworkKeyword = nullptr;
        if (!g_activateFrameworkKeyword) {
            const ModInfo* mod = (*G::dataHandler)->LookupModByName(GAME_PLUGIN_NAME);
            if (mod && mod->modIndex != -1) {
                UInt32 formID = GAME_ACTIVATION_KEYWORD_ID | (mod->modIndex) << 24;
                g_activateFrameworkKeyword = reinterpret_cast<BGSKeyword*>(LookupFormByID(formID));
                if (!g_activateFrameworkKeyword) _MESSAGE("WARNING: Failed to retrieve framework activation keyword from %s.", GAME_PLUGIN_NAME);
            } else {
                _MESSAGE("WARNING: %s is not loaded.", GAME_PLUGIN_NAME);
            }
        }
        return g_activateFrameworkKeyword;
    }

}

//-----------------------
// Internal
//-----------------------
namespace {
    DialogueHolder g_dialogueHolder;

    std::pair<float, float> savedSubtitlePosition;

    // Event Handlers
    EventResult MenuOpenCloseHandler::ReceiveEvent(MenuOpenCloseEventEx * evn, void * dispatcher) {
        static BSFixedString dialogueMenu("DialogueMenu");
        if (evn->menuName == dialogueMenu) {
            if (evn->opening) {
                savedSubtitlePosition = Scaleform::GetSubtitlePosition();
            } else {
                Scaleform::SetSubtitlePosition(savedSubtitlePosition.first, savedSubtitlePosition.second);
            }
        }

        return kEvent_Continue;
    }
    void MenuOpenCloseHandler::Register()
    {
        static MenuOpenCloseHandler eventSink;
        BSTEventDispatcher<MenuOpenCloseEventEx>* eventDispatcher = Utils::GetOffsetPtr<BSTEventDispatcher<MenuOpenCloseEventEx>>(*G::ui, 0x18);
        eventDispatcher->AddEventSink(&eventSink);
    }

    // Utilities
    TESForm * GetFormFromIdentifier(const std::string & identifier)
    {
        auto delimiter = identifier.find('|');
        if (delimiter != std::string::npos) {
            std::string modName = identifier.substr(0, delimiter);
            std::string modForm = identifier.substr(delimiter + 1);

            const ModInfo* mod = (*G::dataHandler)->LookupModByName(modName.c_str());
            if (mod && mod->modIndex != -1) {
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
}