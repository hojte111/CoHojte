#pragma once
#define WIN32_LEAN_AND_MEAN
#include <WinSock2.h> 
#include <Windows.h>
#include <cstdint>
#include "ImGui/imgui.h"
#include <string>
#include "Vector3.h"
#include "Vector2.h"
#include <chrono>
#include <vector>
#include <mutex>
#include <shared_mutex>
#include <array>
#include <unordered_map>

struct CharacterInfo; // forward lol game.h

#define MAX_CLIENT_SIZE_EVER 0x15000
#define MAX_ITEM_COUNT 650 // 650 is the max items loaded around the player
#define MAX_PLAYER_COUNT 125 // 102 is the max players ever in a game in BR trios. FIXED upped to 120 bcs of g_MaxPlayers...!
#define MAX_ENTITY_COUNT 2046 // 2046 is the max entities loaded around the player (reversed from IDA)
#define MAX_TEAM_COUNT 100 // 100 is the max team count (BR solos)

//global structs
typedef unsigned __int64 QWORD;
typedef unsigned __int32 uint32_t;

enum RenderDistances { Range_Close, Range_Medium, Range_Far, Range_COUNT };

using ClientBits = std::array<uint32_t, (MAX_PLAYER_COUNT >> 5) + 1>; // 155 is MAX_PLAYERS

#pragma pack(push, 1)
struct Weapon_t
{
    unsigned __int16 weaponIdx;
    unsigned __int16 stickerIndices[4];
    unsigned __int16 weaponClientLoadout;
    unsigned __int16 weaponOthers;
    unsigned __int8 weaponAttachments[13];
    unsigned __int8 attachmentVariationIndices[29];
    unsigned __int8 weaponCamo;
    unsigned __int8 weaponLootId;
    unsigned __int8 scopeVariation;
    unsigned __int8 visualAttachmentHighAddr;
    unsigned __int16 unknown;
    char pad[0xE];
    char pad1[0x4];
};

class score_t // https://www.unknowncheats.me/forum/3932855-post118.html
{
public:
    __int32 clientNum; //0x0000 
    __int32 status; //0x0004 dead==2, alive==0
    __int32 points; //0x0008 
    __int32 ping; //0x000C 
    __int32 deaths; //0x0010 
    __int32 team; //0x0014 
    bool isBot; //0x0018 
    char pad_0x0019[0x3]; //0x0019
    __int32 kills; //0x001C
    __int32 rank_mp; //0x0020
    __int32 prestige_mp; //0x0024
    __int32 rank_level; //0x0028
    __int32 assists; //0x002C
    __int32 extraScore1; //0x0030 captures, time_on_hardpoint
    __int32 extraScore2; //0x0034 confirms, offense
    __int32 extraScore3; //0x0038 denies, ?wz_damage_taken?, in plane
    __int32 extraScore4; //0x003C wz_damage_done
    __int32 extraScore5; //0x0040
    __int32 extraScore6; //0x0044, defends_hp
    __int32 mp_damage_done; //0x0048
    __int32 mp_damage_taken; //0x004C
    __int32 other1; //0x004C
    char pad_0x0054[0x1C]; //0x0048
    uint64_t deathTimer; // 0x70
    uint64_t icon; // 0x78
    char unk0x80[0x8]; // 0x80
    bool isValid() const;
}; //Size=0x0080
static_assert(offsetof(score_t, mp_damage_done) == 0x48, "Score_t damage offset incorrect");
static_assert(sizeof(score_t) == 0x88, "Size of score_t incorrect");



// Structs for the entity list.
class trajectory_t
{
public:
    __int32 trType; //0x0000 //0x14
    int trTime; //0x0004 //0x18
    int trDuration; //0x0008 //0x1C
    Vector3 trBase; //0x000C //0x20
    Vector3 trDelta; //0x0018 //0x2C
 
}; //Size=0x0024

class LerpEntityState
{
public:
    uint64_t eFlags; //0x0000 // 0xC
    //char m_pad[0x4]; //0x0008 // 0x10
    trajectory_t pos; //0x000C // 0x14
    trajectory_t apos; //0x0030
 
}; //Size=0x0054

class entityState_t
{
public:
    __int16 number; //0x0000 
    __int16 otherEntityNum; //0x0002 
    __int16 attackerEntityNum; //0x0004 
    __int16 groundEntityNum; //0x0006 
    WORD eType; //0x0008 
    unsigned char surfType; //0x000A 
    unsigned char inAltWeaponMode; //0x000B 
    LerpEntityState lerp; //0x000C 
    char pad_0x005C[0x44]; //0x005C
    __int32 clientNum; //0x00A0 
    char pad_0x00A4[0x4]; //0x00A4
 
}; //Size=0x00A8
#pragma pack(pop)
static_assert(offsetof(entityState_t, lerp) == 0xC, "lerp incorrect");
static_assert(offsetof(entityState_t, lerp.eFlags) == 0xC, "eflag offset incorrect");
//static_assert(offsetof(entityState_t, lerp.pos) == 0x14, "tr pos offset incorrect");
//static_assert(offsetof(entityState_t, lerp.pos.trType) == 0x14, "trType offset incorrect: ");
//static_assert(offsetof(entityState_t, lerp.pos.trBase) == 0x20, "trBase offset incorrect");


#pragma pack(push, 1)
struct C_Entity {
    BYTE pad_0[0x224];
    unsigned char C_ENTITY_VALID; // 0x224
    BYTE pad_1[0xF];
    entityState_t C_ENTITY_STATE; // 0x234
    BYTE pad_final[0x13C]; // Final padding
};
#pragma pack(pop)

// A struct for the global item list.
struct item_s
{
    int idx;
    std::string friendlyName;
    char* Id;
    Vector3 pos;
    Vector2 screenPos;
    bool isValid;
    bool isSearched;
    float distToLocal;
    bool isOpenable;
    bool isContainer;
};

// A players possible stances.
enum class CharacterStance : unsigned char
{
    Standing = 0,
    Crouching = 1,
    Crawling = 2,
    Downed = 3,
};

// A players possible teams. Is just casted to an int. To get the team number.
enum class team_t
{
    TEAM_FREE = 0x00,
    TEAM_NUM = 0x9B,
    TEAM_SPECTATING = 0x99,
};


// Struct for the objects in the NAME_ARRAY LIST.
#pragma pack(push, 1)
struct clientInfo_t
{
    uint32_t m_entity_index; // 0x0
    char m_szName[36]; // 0x4
    char pad_1[0x48];
    uint32_t m_extra_flags; // 0x70
    char pad_2[0x10];
    int32_t m_health; // 0x84
    char pad_3[0x38];
}; // size = 0xC0. Called NAME_LIST_SIZE
#pragma pack(pop)
static_assert(sizeof(clientInfo_t) == 0xC0, "Size of name_t incorrect");
static_assert(offsetof(clientInfo_t, m_extra_flags) == 0x70, "extra_flags incorrect");
static_assert(offsetof(clientInfo_t, m_entity_index) == 0x0, "m_entity_index has an incorrect offset");
static_assert(offsetof(clientInfo_t, m_health) == 0x84, "m_health has an incorrect offset");

// A struct for the refdef view.
class RefdefView
{
public:
    Vector2 tanHalfFov; // 0x00
    unsigned __int8 unk1[0xC]; // 0x08
    Vector3 axis[3]; // 0x14
};

// A struct for the refdef.
class refdef_t
{
public:
    int x; // 0x00
    int y; // 0x04
    int width; // 0x08
    int height; // 0x0C
    RefdefView view; // 0x10
};

// A struct for the player ESP object. Which is used for drawing the ESP. And in the aimbot
class ESPBox
{
public:
    Vector2 prediction; // 2D prediction position
    Vector2 screenPos; // 2D screen position
    Vector2 size; // 2D size
    std::string textDistance; // Distance text
    std::string textName; // Player name
    int textSize; // Text size NOT USED
    ImU32 color; // Color of the box. (Aiming, Visible, Not visible, Res, Plating, Downed)
    int hp; // Health of the player
    Vector2 playerLooking2d; // Where the player is looking, drawn from the center of the box.
    ImU32 lookingColor; // Color of the player looking line. (Aiming/Shooting/None)
};

class RadarTriangle
{
public:
    // The 3 points/tips of the triangle
    Vector2 p1;
    Vector2 p2;
    Vector2 p3;
    std::string textTeam; // team number
    Vector3 color; // Color of the triangle, always orange.
};

class SkillScore
{
public:
    float skillIndex; // The calculated skill index of the player.
    int placement; // The placement of the player compared to lobby.
    float points;
    float kills;
    float deaths;
    float assists;
    float damageDone;
    float damageTaken;
    float level;
    int playerIndex;
};

class Player
{
public:
    Player(int index);
    Player();
    Vector3 pos;
    CharacterStance stance;
    std::string name;
    int team;
    int entIndex;
    bool isDead;
    bool isValid;
    bool entValid;
    int health;
    long long inCrosshairOfLocalLastTs;
    long long lastHpZeroTs;
    long long lastVisibleTs;
    long long downedTs;
    bool debug;
    bool dormant;
    Weapon_t weapon;
    int shotsFired;
    long long lastShootingTs;
    Vector3 decPos;
    int64_t calcTimeIdle();
    bool skipPlayer();
    float distanceToLocal();
    SkillScore getSkillScore();
    void registerScores();
    float localInCrosshairDirection; // Is the local player in the crosshair of this player. If yes the angle to the enemy is stored in this variable.
    int weaponId; // The weapon id of the weapon the player is holding.
    std::string weapon_name; // Not used/working
    float yaw; // The yaw of the player view
    float pitch; // The pitch of the player view
    bool isVisible; // Is the player visible to the local player.
    bool aiming; // Is the player is aiming down sights.
    bool shooting; // Is the player shooting.
    float speed; // The speed/velocity of the player.
    Vector3 velocity; // The calculated velocity of the player.
    Vector2 screenPos; // The 2D screen position of the player.
    long long lastUpdateTime; // The last time the player was valid and read.
    long long lastVelocitySampleTs; // The last time the player's velocity was sampled.
    Vector3 lastVelocitySamplePos; // The last position the player's velocity was sampled.
    bool isSelfReviving; // Is the player self reviving.
    bool isInGulag; // Is the player in the gulag.
    score_t score; // The player score!
    C_Entity c_entity; // The player's entity.
};

// A struct used for decrypting refdef.
class refdefKeyStruct
{
public:
    DWORD ref0; // 0x00
    DWORD ref1; // 0x04
    DWORD ref2; // 0x08
};

class AimbotSettings
{
public:
    float maxSpeed;
    float minSpeed;
    float aimCircleRadius;
    float invisibleAimCircleRadius;
    float deadZoneThreshold;
    float maximumDistance; // Maximum distance is in meters
    float worldDistanceWeight; // Weight for the 3D world distance over the 2D screen distance
    bool showDebugDrawing; // Show the circle info on screen

    AimbotSettings() : maxSpeed(38.0f), minSpeed(1.5f), aimCircleRadius(500.0f), invisibleAimCircleRadius(70.0f), deadZoneThreshold(30.0f), maximumDistance(320.0f), worldDistanceWeight(0.3f)
    {
    }

    void setMaxSpeed(float value) { if (value > 0 && value >= minSpeed) { maxSpeed = value; } }
    void setMinSpeed(float value) { if (value > 0 && value <= maxSpeed) { minSpeed = value; } }

    void setAimCircleRadius(float value)
    {
        if (value > 0 && value >= invisibleAimCircleRadius && value >= deadZoneThreshold) { aimCircleRadius = value; }
    }

    void setInvisibleAimCircleRadius(float value)
    {
        if (value > 0 && value <= aimCircleRadius && value >= deadZoneThreshold) { invisibleAimCircleRadius = value; }
    }

    void setDeadZoneThreshold(float value)
    {
        if (value > 0 && value <= aimCircleRadius && value <= invisibleAimCircleRadius) { deadZoneThreshold = value; }
    }

    void setMaximumDistance(float value) { if (value > 0) { maximumDistance = value; } }
    void setWorldDistanceWeight(float value) { if (value >= 0.0f && value <= 1.0f) { worldDistanceWeight = value; } }

    float getMaxSpeed() const { return maxSpeed; }
    float getMinSpeed() const { return minSpeed; }
    float getAimCircleRadius() const { return aimCircleRadius; }
    float getInvisibleAimCircleRadius() const { return invisibleAimCircleRadius; }
    float getDeadZoneThreshold() const { return deadZoneThreshold; }
    float getMaximumDistance() const { return maximumDistance; }
    float getWorldDistanceWeight() const { return worldDistanceWeight; }
};

// Global variables, annotated in globals.cpp
extern SOCKET g_Sock;
extern SOCKET g_Sock2;
extern SOCKET g_Sock3;
extern SOCKET g_SockItem;
extern DWORD g_PID;
extern uint64_t g_Base;
extern uint64_t g_Peb;
extern Player g_playerArr[MAX_PLAYER_COUNT];
extern Player g_lPlayer;
extern int g_validPlayers;
extern std::string g_errMsg;
extern ImVec2 g_RefDefAxis;
extern int g_zoomLevel;
extern HBITMAP g_hBitmapRadar;
extern HDC g_hBitmapRadarDC;
extern ImVec2 g_localView;
extern std::string g_LocalName;
extern bool g_northOriented;
extern bool g_AFK;
extern HWND g_OverlayHWND;
extern int g_WarfareWidth;
extern int g_WarfareHeight;
extern refdef_t g_Refdef;
extern ESPBox g_ESP_DrawList[MAX_PLAYER_COUNT];
extern RadarTriangle g_Radar_DrawList[MAX_PLAYER_COUNT];
extern bool g_overlayActive;
extern int64_t g_idlePlayersTime[MAX_PLAYER_COUNT];
extern bool g_crosshair;
extern bool g_ADS;
extern int g_playAgainAdjustmentY;
extern int g_playAgainY;
extern uint64_t g_decryptCase;
extern uint64_t g_decryptBasePtr;
extern uint64_t g_decryptCgTPtr;
extern bool g_AimBotActive;
extern int g_closestIndex;
extern bool g_Verbose;
extern bool g_hasVisibleTarget;
//extern int g_SearchIntVal;
extern QWORD g_SearchIntVal;
extern float g_SearchFloatVal;
extern int g_readAddressVal;
extern int g_readAddressInput;
extern float g_X;
extern float g_Y;
extern int g_W;
extern int g_dontDrawCap;
extern char g_LPBuffer[1700000];
extern item_s g_ItemList[MAX_ITEM_COUNT];
extern bool g_Active;
extern bool g_SoftRestart;
extern bool g_AimingAlert;
extern std::vector<float> g_AimingAlertAngles;
extern bool g_EnableAimingAlert;
extern std::string g_AFKState;
extern std::vector<std::string> g_ItemSearchKeywords;
extern bool g_ItemESP;
extern uint64_t g_CEntityBase;
extern std::vector<std::vector<Vector2>> g_TeamToLinkPositions;
extern bool g_DoTeamLink;
extern std::mutex g_TeamLinkMutex;
extern long long g_PlayerLoopTime;
extern C_Entity* g_EntityArray;
extern std::vector<Vector2> g_botsOnScreen;
extern clientInfo_t* g_ClientInfoRaw;
extern CharacterInfo* g_CharacterInfoRaw;
extern C_Entity* g_CEntityRaw;
extern int g_MaxPlayers;
extern uint64_t g_ClientInfoArrPtr;
extern ClientBits g_visible_enemy_bits;
extern bool g_gameRunning;
extern int g_killRecordMP;
extern int g_killRecordWZ;
extern int g_damageRecordMP;
extern int g_damageRecordWZ;
extern int g_assistRecordMP;
extern int g_assistRecordWZ;
extern int g_damageTakenRecordMP;
extern int g_damageTakenRecordWZ;
extern int g_pointsRecordMP;
extern int g_pointsRecordWZ;
extern int64_t g_AimBotActiveInADSDurationMillis;
extern int64_t g_AimBotActiveInShootingDurationMillis;
extern int64_t g_gameOpenTime;
extern int64_t g_inGameTimeMP;
extern int64_t g_inGameTimeWZ;
extern int64_t g_timeAliveMP;
extern int64_t g_timeAliveWZ;
extern int g_deathRecordMP;
extern int g_deathRecordWZ;
extern int g_programRestarts;
extern int g_gameCountMP;
extern int g_gameCountWZ;
extern float g_avgSkillIndex;
extern std::string g_detectedLocalName;
extern int g_AimBotMode;
extern Vector3 g_CameraPos;
extern AimbotSettings g_AimbotSettings;
extern bool g_surveyRemindLater;
extern bool g_remindSet;
extern bool g_enablePlayerLookingDirection;
extern float g_guiOpacity;
extern bool g_showPlayerNames;
extern int64_t g_timeSpentAiming;
extern int64_t g_timeSpentShooting;
extern int64_t g_timeAimbotActive;
extern bool g_RapidClick;
extern int g_RenderDistance;
extern SkillScore g_HighScore;
extern SkillScore g_LowScore;
extern Player g_realLocalPlayer;
extern std::unordered_map<int, SkillScore> g_SkillScores;
extern std::shared_mutex g_SkillScoresMutex;
extern int g_boolPick;
extern int g_accumulatedShotsFired;
extern int g_accumulatedShotsMissed;
extern int g_accumulatedDamageDone;
extern QWORD g_refDefPtr;

// GUI's
extern ImVec4 clear_color;
extern ImColor teamCol;
extern ImColor enemyCol;
extern ImColor greenCol;
extern ImColor pinkCol;
extern ImColor orangeCol;
extern ImColor darkOrangeCol;

// Helpers
int64_t getSec();
long long getms();
