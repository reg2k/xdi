#pragma once

#include "f4se/GameForms.h"

//-------------------------
// Forward Declarations
//-------------------------

class TESQuest;     // declaration only
class BGSScene;
class TESTopic;
class TESTopicInfo;
class BGSSceneAction;
class BGSSceneActionStartScene;
class BGSSceneActionTimer;
class BGSSceneActionDialogue;
class BGSSceneActionConversationBase;
class BGSSceneActionPlayerDialogue;
class BGSSceneActionNPCResponseDialogue;
class MenuTopicManager;
struct SceneLink;
struct DialoguePrompt;

class PlayerCharacter;  // declaration only

// Game Types
class BSStringEx;

//-------------------------
// Function Signatures
//-------------------------
typedef bool(*_GetQuestAliasHandle)(TESQuest* quest, UInt32* handle_out, UInt32 aliasID);
typedef void(*_InitSceneActions)(BGSScene* scene);
typedef void(*_StartScene)(BGSScene* scene, bool unk2);
typedef void(*_StartDialogueCamera)(PlayerCharacter* player, bool unk2, bool unk3);
typedef void(*_SelectDialogueOption)(PlayerCharacter* player, UInt32 dialogueOption);
typedef UInt32(*_GetSpeechChallengeLevel)(TESTopicInfo* topicInfo); // Accesses hashset. Returns 0 if no speech challenge
typedef SInt32(*_GetSpeechChallengeState)(TESTopicInfo* topicInfo); // Accesses hashset. Returns -1 if not attempted
typedef TESTopicInfo*(*_GetInfoGroupParent)(TESTopicInfo* topicInfo); // Accesses hashset. Returns nullptr if no parent.
typedef void(*_PostDialogueHandler)(Actor* actor, UInt32 refresh); // For triggering speech check result animation. Refresh = 0/1 (Set to 1 to refresh)
typedef void(*_SetPlayerControlsEnabled)(void* inputEnableManager, UInt32 layerID, UInt32 flags, bool enable, UInt32 unk4); // unk4 - 1: internal 2: game 3: user
typedef void(*_DoTextReplacement)(BSStringEx* str, TESQuest* quest, UInt32 unk3); // This modifies the input string. unk3 = *((UInt32*)(quest+0x50)).

typedef bool(*_IKeywordFormBase_HasKeyword)(IKeywordFormBase* keywordFormBase, BGSKeyword* keyword, UInt32 unk3); // unk3 = 0 in HasKeyword condition function

typedef TESTopicInfo* (*_GetInfoForDialogueOption)(BGSSceneActionPlayerDialogue* playerDialogue, BGSScene* scene, Actor* dialogueTarget, int dialogueOption);
typedef TESTopicInfo* (*_GetNPCResponseInfoForOption)(BGSSceneActionNPCResponseDialogue* npcDialogue, BGSScene* scene);
typedef bool(*_IsActorTalking)(Actor* actor);   // Actor vtable index 0x4E
const int _IsActorTalking_VtblIdx = 0x4E;

//-------------------------
// Forms
//-------------------------

// E8
class BGSScene : public TESForm
{
public:
    enum { kTypeID = kFormType_SCEN };

    enum StatusFlags {
        kFlag_Playing   = (1 << 0),
        kFlag_Paused	= (1 << 2),		// Set this flag to pause the scene. Unset the flag to resume.
        kFlag_Stopped	= (1 << 20),	// Set this flag to stop the scene immediately.
    };

    enum SceneFlags {
        kFlag_PreventPlayerExitDialgoue = (1 << 6),
        kFlag_DisableDialogueCamera     = (1 << 11),
    };

    // parent
    IKeywordFormBase keywordFormBase;   // 20
    UInt32      status;                 // 28
    UInt32      unk2C;                  // 2C
    UnkArray    unk30;                  // 30 - phases?
    UnkArray    unk48;                  // 48 - when Scene.Stop() is called, all (refs) in this array have their refcounts decremented. perhaps participating actors?
    UnkArray    unk60;                  // 60
    UnkArray    unk78;                  // 78
    tArray<BGSSceneAction*>	actions;    // 90
    TESQuest*   owningQuest;            // A8
    UInt64      unkB0;                  // B0 - BGSScene*, but for ..? 1402C20D0 recursively visits unkB0 until unkB0 is NULL, then returns sceneFlags >> 5. scene chain - paused scenes? parent? related to unk48
    UInt32      sceneFlags;             // B8 - (&= 0xFFFFFBFF (unsets bit 10) when player selects a dialogue option.)
    UInt32      unkBC;                  // BC
    UInt64      unkC0;                  // C0
    SInt32      unkC8;                  // C8 - some alias ID
    SInt32      currentPhase;           // CC
    SInt32      nextPhase;              // D0
    float       unkD4;                  // D4
    UInt32      unkD8;                  // D8
    UInt32      unkDC;                  // DC - handle to the next dialogue target for CC
    UInt64      unkE0;                  // E0

};
STATIC_ASSERT(offsetof(BGSScene, unkD4) == 0xD4);
STATIC_ASSERT(sizeof(BGSScene) == 0xE8);

// 78
class TESTopic : public TESForm
{
public:
	enum { kTypeID = kFormType_DIAL };

    TESFullName     fullName;       // 20
    UInt32          unk30;          // 30 - unk30 (uint16), unk32
    UInt32          priority;       // 34
    void*           dialogueBranch; // 38 - TESDialogueBranch
    TESQuest*       owningQuest;    // 40
    BGSKeyword*     keyword;        // 48
    TESTopicInfo**  topicInfos;     // 50
    void*           unk58;          // 58
    UInt32          topicInfoCount; // 60
    UInt32          unk64;          // 64
    void*           unk68;          // 68
    BSFixedString   editorID;       // 70
};
STATIC_ASSERT(offsetof(TESTopic, unk68) == 0x68);
STATIC_ASSERT(sizeof(TESTopic) == 0x78);

// reg2k
// 50
class TESTopicInfo : public TESForm
{
public:
	enum { kTypeID = kFormType_INFO };

    enum Flags
    {
        kFlag_InfoGroup       = (1 << 6),
    };

    enum InfoFlags
    {
        kFlag_StartSceneOnEnd = (1 << 0),
        kFlag_Random          = (1 << 1),
        kFlag_SayOnce         = (1 << 2),
        kFlag_RandomEnd       = (1 << 5),
        kFlag_EndRunningScene = (1 << 6),

        kFlag_HasBeenSaid     = (1 << 14),
    };

    TESTopic*	                topic;	        // 20
	TESGlobal*		            resetGlobal;	// 28
    TESTopicInfo*		        sharedInfo;	    // 30
    Condition*                  conditions;	    // 38
    UInt8                       unk40;	        // 40 - info index / possibly dialogue flags (ResetDialogueFlags sets a single byte +0x43 to 0 and MarkUnchanged 0x80000000)
    UInt8                       unk41;	        // 41
    UInt8                       unk42;	        // 42 - subtitle priority - Low = 0, Normal = 1, Force = 3
    bool                        saidOnce;	    // 43 - Said Once flag
    UInt16		                infoFlags;	    // 44
    UInt16		                unk46;          // 46 - Hours to reset
    tList<StringCache::Entry>*  responses;      // 48 - note: responses are always external and stored in externData field. Use Get<char>() to retrieve.

};
STATIC_ASSERT(offsetof(TESTopicInfo, responses) == 0x48);
STATIC_ASSERT(sizeof(TESTopicInfo) == 0x50);

//-------------------------
// Scene Actions
//-------------------------

// 20?
class BGSSceneAction
{
public:
    virtual ~BGSSceneAction();

    virtual void	Unk_01();
    virtual void	Unk_02();
    virtual void	Unk_03();
    virtual void	Unk_04();
    virtual void	Unk_05();
    virtual void	Unk_06();
    virtual void	Unk_07();
    virtual UInt32	GetType();
    virtual bool	GetIgnoredForCompletion();
    virtual void	Unk_0A();
    virtual void	Unk_0B();
    virtual void	Unk_0C();
    virtual void	Unk_0D();   // init or reset?
    virtual void	Unk_0E();
    virtual void	Unk_0F();
    virtual void	Unk_10();
    virtual void	Unk_11();
    virtual void	Unk_12();   // Execute?
    virtual void	Unk_13(BGSScene* scene);   // result callback? see timer
    virtual void	Unk_14();
    virtual void	Unk_15(BGSScene* scene);   // important function

    enum ActionType
    {
        kType_Dialogue,
        kType_Package,
        kType_Timer,
        kType_PlayerDialogue,
        kType_StartScene,
        kType_NPCResponseDialogue,
        kType_Radio
    };

    enum ActionFlags
    {
        kFlag_PlayerUseDialogueSubtype_Positive = (1 << 7),
        kFlag_PlayerUseDialogueSubtype_Negative = (1 << 8),
        kFlag_PlayerUseDialogueSubtype_Neutral  = (1 << 9),
        kFlag_PlayerUseDialogueSubtype_Question = (1 << 11),
        kFlag_FaceTarget            = (1 << 15),
        kFlag_HeadTrackPlayer       = (1 << 17),
        kFlag_IgnoreForCompletion   = (1 << 19),
        kFlag_CameraSpeakerTarget   = (1 << 21),
    };

    enum ActionStatus
    {
        kStatus_Stopped,
        kStatus_Running,
        kStatus_Complete
    };

    //	void	** _vtbl;	// 00
    UInt32  aliasID;        // 08 - The target actor that is performing this scene action.
    UInt16  startPhase;     // 0C
    UInt16  endPhase;       // 0E
    UInt32  flags;          // 10
    UInt8   status;         // 14 - 0: stopped, 1: running, 2: complete
    char    pad15[3];       // 15
    UInt32  index;          // 18 - when comparing, comparand == (id & 0x3FFFFFFF) (shave off top 2 bits) (see IsActionComplete)
    UInt32  unk1C;          // 1C

};
STATIC_ASSERT(sizeof(BGSSceneAction) == 0x20);

// 48
class BGSSceneActionStartScene : public BGSSceneAction
{
public:
    //...
    // 20
    // 38 - BGSScene* sceneToStart
    // 40 - UInt16 phase?
};

// 38
class BGSSceneActionTimer : public BGSSceneAction
{
public:
    //...
};

// 78
class BGSSceneActionDialogue : public BGSSceneAction
{
public:
    TESTopic*   topic;                      // 20
    float       loopingMax;                 // 28
    float       loopingMin;                 // 2C
    UInt64      unk30[(0x78-0x30)/8];       // 30
};
STATIC_ASSERT(sizeof(BGSSceneActionDialogue) == 0x78);

// 90
class BGSSceneActionConversationBase : public BGSSceneAction
{
public:
    TESTopic*   response[4];                // 20
    UInt64      unk40[(0x78-0x40)/8];       // 40
    UInt32      unk78;                      // 78
    UInt8       unk7C[8];                   // 7C
    float       unk84;                      // 84
    TESTopic*   unk88;                      // 88 - nextTopic or selected topic? this is set by vfunc unk15 after making a dialogue choice.
};
STATIC_ASSERT(sizeof(BGSSceneActionConversationBase) == 0x90);

// E8
class BGSSceneActionPlayerDialogue : public BGSSceneActionConversationBase
{
public:
    UInt64      unk90;                      // 90 - (UInt32) Alias ID - Dialogue Target Actor
    TESTopic*   npcResponse[4];             // 98
    UInt64      unkB8[(0xD8-0xB8)/8];       // B8
    TESTopic*   unkD8;                      // D8 - selected topic? this is set to the owner topic of the selected topicinfo.
    UInt32      selectedOption;             // E0 - defaults to 4 while no option has been selected yet.
    UInt32      padE4;                      // E4
};
STATIC_ASSERT(sizeof(BGSSceneActionPlayerDialogue) == 0xE8);

// 90
class BGSSceneActionNPCResponseDialogue : public BGSSceneActionConversationBase {};

//-------------------------
// Menu Topic Manager
//-------------------------

class MenuTopicManager
{
public:
    virtual ~MenuTopicManager();

    //void	**  _vtbl;	                    // 00
    void*       positionPlayerEventSink;    // 08
    UInt32      unk10;                      // 10
    UInt32      dialogueTargetHandle;       // 14
    char        unk18[28];                  // 18
    float       elapsedTime;                // 34 - phase or dialogue
    UInt32      unk38[4];                   // 38
    UInt8       unk48[8];                   // 48
    bool        unk50;                      // 50 - not in dialogue cam
    bool        unk51;                      // 51
    bool        unk52;                      // 52
    bool        awaitingPlayerInput_2;      // 53
    UInt8       unk54;                      // 54
    bool        awaitingPlayerInput;        // 55
    bool        unk56;                      // 56 - in dialogue
    UInt8       unk57[8];                   // 57

};
STATIC_ASSERT(offsetof(MenuTopicManager, unk57) == 0x57);

//-------------------------
// Scene Links
//-------------------------

// 18
struct SceneLink
{
    TESTopicInfo*   key;        // 00
    BGSScene*       scene;      // 08
    UInt32          phase;      // 10
    UInt32          pad14;      // 14

    operator TESTopicInfo*() const { return key; }
    static inline UInt32 GetHash(TESTopicInfo ** key)
    {
        UInt32 hash;
        CalculateCRC32_64(&hash, (UInt64)*key, 0);
        return hash;
    }
};
STATIC_ASSERT(sizeof(SceneLink) == 0x18);

// 10
struct DialoguePrompt
{
    TESTopicInfo*   key;        // 00
    BSFixedString   prompt;     // 08

    operator TESTopicInfo*() const { return key; }
    static inline UInt32 GetHash(TESTopicInfo ** key)
    {
        UInt32 hash;
        CalculateCRC32_64(&hash, (UInt64)*key, 0);
        return hash;
    }
};
STATIC_ASSERT(sizeof(DialoguePrompt) == 0x10);

// 10
struct SpeechChallengeLevel
{
    enum ChallengeLevel {
        kChallengeLevel_Easy = 1,
        kChallengeLevel_Medium,
        kChallengeLevel_Hard,
        kChallengeLevel_AlwaysSucceeds,
        kChallengeLevel_EasyRepeatable,
        kChallengeLevel_MediumRepeatable,
        kChallengeLevel_HardRepeatable
    };

    TESTopicInfo*   key;                // 00
    UInt32          challengeLevel;     // 08

    operator TESTopicInfo*() const { return key; }
    static inline UInt32 GetHash(TESTopicInfo ** key)
    {
        UInt32 hash;
        CalculateCRC32_64(&hash, (UInt64)*key, 0);
        return hash;
    }
};
STATIC_ASSERT(sizeof(SpeechChallengeLevel) == 0x10);

// 10
struct SpeechChallengeState
{
    TESTopicInfo*   key;            // 00
    UInt32          succeeded;      // 08

    operator TESTopicInfo*() const { return key; }
    static inline UInt32 GetHash(TESTopicInfo ** key)
    {
        UInt32 hash;
        CalculateCRC32_64(&hash, (UInt64)*key, 0);
        return hash;
    }
};
STATIC_ASSERT(sizeof(SpeechChallengeState) == 0x10);

// 10
struct InfoGroupParent
{
    TESTopicInfo*   key;            // 00
    TESTopicInfo*   parentInfo;     // 08

    operator TESTopicInfo*() const { return key; }
    static inline UInt32 GetHash(TESTopicInfo ** key)
    {
        UInt32 hash;
        CalculateCRC32_64(&hash, (UInt64)*key, 0);
        return hash;
    }
};
STATIC_ASSERT(sizeof(InfoGroupParent) == 0x10);

// 1.10.26:
// 3735B80 - Set Parent Quest Stage
// 3735B50 - Greet Distance
// 3735BB0 - Speech Challenge State
// 3735C70 - Speech Challenge Level
// 10
struct UnkItem
{
    TESTopicInfo*   key;            // 00
    UInt64          unk08;          // 08

    operator TESTopicInfo*() const { return key; }
    static inline UInt32 GetHash(TESTopicInfo ** key)
    {
        UInt32 hash;
        CalculateCRC32_64(&hash, (UInt64)*key, 0);
        return hash;
    }
};

struct InputEnableManager
{
    enum InputLayer1 {
        kInputLayer_Movement        = (1 << 0),
        kInputLayer_Looking         = (1 << 1),
        kInputLayer_Activate        = (1 << 2),
        kInputLayer_Menu            = (1 << 3),
        kInputLayer_Console         = (1 << 4),
        kInputLayer_POVSwitch       = (1 << 5),
        kInputLayer_Fighting        = (1 << 6),
        kInputLayer_Sneaking        = (1 << 7),
        kInputLayer_MainFourMenu    = (1 << 8),
        kInputLayer_WheelZoom       = (1 << 9),
        kInputLayer_Jumping         = (1 << 10),
        kInputLayer_VATS            = (1 << 11),
    };

    enum InputLayer2 {
        kInputLayer_JournalTabs     = (1 << 0),
        kInputLayer_Activation      = (1 << 1),
        kInputLayer_FastTravel      = (1 << 2),
        kInputLayer_POVChange       = (1 << 3),
        kInputLayer_VATS_2          = (1 << 4),
        kInputLayer_Favorites       = (1 << 5),
        kInputLayer_PipboyLight     = (1 << 6),
        kInputLayer_ZKey            = (1 << 7),
        kInputLayer_Running         = (1 << 8),
        kInputLayer_Cursor          = (1 << 9),
        kInputLayer_Sprinting       = (1 << 10),
    };
};

//-----------------
// Game Types
//-----------------

// 10
class BSStringEx
{
public:
    BSStringEx() :m_data(NULL), m_dataLen(0), m_bufLen(0) { }

    // r2k
    BSStringEx(const char* str) {
        size_t size = strlen(str) + 1;
        m_data = (char*)Heap_Allocate(size);
        strcpy_s(m_data, size, str);
        m_dataLen = size - 1;
        m_bufLen = size;
    }

    ~BSStringEx()
    {
        if (m_data) {
            Heap_Free(m_data);
            m_data = nullptr;
            m_dataLen = 0;
            m_bufLen = 0;
        }
    };

    const char *Get(void) {
        return m_data;
    }

private:
    char	* m_data;	// 00
    UInt16	m_dataLen;	// 08
    UInt16	m_bufLen;	// 0A
    UInt32	pad0C;		// 0C
};