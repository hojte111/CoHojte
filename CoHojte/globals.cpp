#include "globals.h"

using ClientBits = std::array<uint32_t, (MAX_PLAYER_COUNT >> 5) + 1>; // 155 is MAX_PLAYERS

// The driver communication sockets. Used for concurrent communication with the driver. (Up to 10 sockets)
SOCKET g_Sock = 0;
SOCKET g_Sock2 = 0;
SOCKET g_Sock3 = 0;
SOCKET g_SockItem = 0;

// The process ID of the game.
DWORD g_PID = 0;
// The base address of the game. Used for reading the game memory.
uint64_t g_Base = 0;
// The process environment block.
uint64_t g_Peb = 0;

// The player array. Used for storing the player data.
Player g_playerArr[MAX_PLAYER_COUNT];
// THE Local player. Used for storing the local player data.
Player g_lPlayer;
// Valid players. Used for counting the number of players in the game. Based on the VALID offset (not always working).
int g_validPlayers = -1;
// Error message. Used for displaying error messages in the GUI.
std::string g_errMsg;
// RefDef axis, used for calculating local yaw in world2Radar.
ImVec2 g_RefDefAxis;
// The radar zoom level. Used for zooming in and out of the Gui radar.
int g_zoomLevel = 1;
// Idea was to display the minimap in the GuiEx.cpp. By screenshotting the top corner i think.
// But it's not used/working
HBITMAP g_hBitmapRadar;
HDC g_hBitmapRadarDC;
// The local view. Used for drawing the north-oriented player view arrow on the GUI radar.
ImVec2 g_localView;
// The local player name. Can be used to hardcode the local player name. If LOCAL_INDEX is not working.
std::string g_LocalName;
// North oriented flag. If true, the radar is north-oriented.
bool g_northOriented = false;
// AFK flag. If true, the AFK script is active.
bool g_AFK;
// The overlay window handle. Used for drawing the overlay. Is global because GuiEx does InvalidateRect on it. IDK why.
HWND g_OverlayHWND = NULL;
// The width and height of the game window. Used for drawing the overlay. And center of screen.
int g_WarfareWidth = 1920;
int g_WarfareHeight = 1080;
// The reference definition. Used for getting the local player view. Used in WorldToScreen.
refdef_t g_Refdef;
// The ESP draw list. Populated by GuiEx.cpp and drawn in Paint.cpp. Used by aimbot.
ESPBox g_ESP_DrawList[MAX_PLAYER_COUNT];
// Radar draw list. Used for drawing enemies on the game mini-map. Populated by GuiEx.cpp and drawn in Paint.cpp
RadarTriangle g_Radar_DrawList[MAX_PLAYER_COUNT];
// Overlay active flag. If true, the overlay(ESP and map radar) is active.
bool g_overlayActive;
// List of players that are idle. Used for calculating the time that players have been idle.
int64_t g_idlePlayersTime[MAX_PLAYER_COUNT];
// Crosshair flag. If true, the green crosshair is drawn.
bool g_crosshair = true;
// Local is ADS flag. If true, the local player is aiming down sight.
// Detected with right click mouse button. Used for removing the green crosshair when aiming down sight.
bool g_ADS;
// Used for AFK click on the play again button. Has a slider in the GUI. And debug drawing in the overlay.
int g_playAgainAdjustmentY = 0;
// The play again hardcoded Y-location, X-location is hardcoded in the AFK script - 1500.
int g_playAgainY = 910;
// The decryption case. Legacy(might be reintroduced), was used to determine which decryption to do for the client base.
uint64_t g_decryptCase;
// The pointer to the client info (AKA. Cg_t). Used for reading the local player data.
uint64_t g_decryptCgTPtr;
// The pointer to ClientBase/Player list (AKA. ClientInfo). Used for reading player data.
uint64_t g_decryptBasePtr = 0;
// Aimbot active flag. If true, the aimbot is active.
bool g_AimBotActive;
// Used for the aimbot. The index of the closest player to the local players crosshair.
// Is a global variable because it was used in the GUI for development of the aimbot.
int g_closestIndex;
// Verbose flag. If true, the console will print more information.
bool g_Verbose;
// Has visible target flag. If true, the local player has a visible target.
bool g_hasVisibleTarget;
// Search int value for dynamic testing purposes (in the local player object).
QWORD g_SearchIntVal;
//int g_SearchIntVal;
// Search float value for dynamic testing purposes (in the local player object).
float g_SearchFloatVal;
// The player offset to read from. This is used for dynamic testing purposes (in the local player object).
int g_readAddressInput;
// The value read from g_readAddressInput
int g_readAddressVal;
// X and Y variables for dynamic testing purposes.
// Right now it's used for changing the ESP Box size to fit the enemy.
float g_Y = 0;
float g_X = 0;
// g_W not used
int g_W = 25;
// The distance in units to draw the cap. 10000 units =~250 meters
int g_dontDrawCap = 10000;
// The buffer for the local client, used for analyzing the local client data.
// Max client size is the max that the Player object has ever been (changes on each game update).
// This is needed because the buffer is used to store the local client data, and the buffer size is fixed/statically allocated.
char g_LPBuffer[1700000];
// The item list, used for drawing the items on the radar. NOT USED.
// MAX_ITEM_COUNT is the limit of items to load (there are a lot to load AFAIK!).
item_s g_ItemList[MAX_ITEM_COUNT];
// The main active flag. Overlay, Gui, and other stuff are only active if this is true.
bool g_Active = true;
// Soft restart flag. If true, the overlay and player loop will exit and all variables should reset to default.
bool g_SoftRestart = false;
// Aiming alert flag. If true, a beep is sounded if an enemy outside locals view is aiming at local.
bool g_AimingAlert = false;
// List of aiming alert angles. Used for drawing the angles of the enemies aiming at the local player.
std::vector<float> g_AimingAlertAngles = std::vector<float>();
// Enable aiming alert flag. If true, the aiming alert is enabled.
bool g_EnableAimingAlert = true;
// AFK State for sanity, debugging and info.
std::string g_AFKState;
// Search bar buffer ofr item names filtering
std::vector<std::string> g_ItemSearchKeywords = {"orange", "urani"};
// Item ESP flag. If true, the item ESP is active.
bool g_ItemESP = true;
// C_Entity list start
uint64_t g_CEntityBase;
// TeamLinks list of lines between teams on the ESP
std::vector<std::vector<Vector2>> g_TeamToLinkPositions = std::vector(MAX_TEAM_COUNT, std::vector<Vector2>(MAX_PLAYER_COUNT, Vector2()));
// TeamLinks active flag. If true, the team link ESP is active.
bool g_DoTeamLink = true;
// TeamLinks mutex. Used for locking the team link positions.
std::mutex g_TeamLinkMutex;
// Player Loop Time Metric
long long g_PlayerLoopTime = 0;
// Entity array. Used for storing the entity data.
C_Entity* g_EntityArray = new C_Entity[MAX_ENTITY_COUNT];
// Bots on screen. Used for aimbotting bots.
std::vector<Vector2> g_botsOnScreen = std::vector<Vector2>();
// raw clientinfo_t array
clientInfo_t* g_ClientInfoRaw;
// raw CharacterInfo array
CharacterInfo* g_CharacterInfoRaw;
// raw C_Entity array
C_Entity* g_CEntityRaw;
// max players. Used for determining the game mode: Lobby=1, Solos=100, Resurgence=51trio/72solo  Zombies=24, Hardpoint=12, 
int g_MaxPlayers;
// The clientinfo_t array pointer. Used for reading the player names.
uint64_t g_ClientInfoArrPtr = 0;
// The visible enemy bits. Used for determining which enemies are visible.
ClientBits g_visible_enemy_bits;
// is game running (has valid memory at 0x50)
bool g_gameRunning = false;

/* Metrics */
// Kill record. Used for storing the kill record of the local player in Multiplayer.
int g_killRecordMP = 0;
// Kill record. Used for storing the kill record of the local player in Warzone.
int g_killRecordWZ = 0;
// Damage record. Used for storing the damage record of the local player in Multiplayer.
int g_damageRecordMP = 0;
// Damage record. Used for storing the damage record of the local player in Warzone.
int g_damageRecordWZ = 0;
// Damage taken record. Used for storing the damage taken record of the local player in Multiplayer.
int g_damageTakenRecordMP = 0;
// Damage taken record. Used for storing the damage taken record of the local player in Warzone.
int g_damageTakenRecordWZ = 0;
// Points record. Used for storing the points record of the local player in Multiplayer.
int g_pointsRecordMP = 0;
// Points record. Used for storing the points record of the local player in Warzone.
int g_pointsRecordWZ = 0;
// Assists record. Used for storing the assists record of the local player in Multiplayer.
int g_assistRecordMP = 0;
// Assists record. Used for storing the assists record of the local player in Warzone.
int g_assistRecordWZ = 0;
// Aimbot active in ADS duration. Used for determining how long the aimbot has been active while in ADS.
int64_t g_AimBotActiveInADSDurationMillis = 0;
// Aimbot active shooting duration. Used for determining how long the aimbot has been active while shooting.
int64_t g_AimBotActiveInShootingDurationMillis = 0;
// game open time millis
int64_t g_gameOpenTime = 0;
// in game time in millis in multiplayer
int64_t g_inGameTimeMP = 0;
// in game time in millis in warzone
int64_t g_inGameTimeWZ = 0;
// time alive in MP
int64_t g_timeAliveMP = 0;
// time alive in WZ
int64_t g_timeAliveWZ = 0;
// Death record. Used for storing the deaths in multiplayer of the local player.
int g_deathRecordMP = 0;
// Death record. Used for storing the deaths in Warzone of the local player.
int g_deathRecordWZ = 0;
// Program restarts. Used for counting the number of program restarts.
int g_programRestarts = 0;
// game count in multiplayer
int g_gameCountMP = 0;
// game count in warzone
int g_gameCountWZ = 0;
// best skill index achieved
float g_avgSkillIndex = 0;
// The local player name. Used for debugging the local player name.
std::string g_detectedLocalName;
// The aimbot mode. Used for determining the aimbot mode. 0 = Normal, 1 = Jupiops Version.
int g_AimBotMode = 0;
// for calculating aimbot percentages, 1 to prevent division by zero
int64_t g_timeSpentAiming = 1;
int64_t g_timeSpentShooting = 1;
int64_t g_timeAimbotActive = 1;
// real detected local player, detected in metricsSampler
Player g_realLocalPlayer = Player(-1);
// The camera position. Used for determining the world to screen conversion.
Vector3 g_CameraPos = Vector3();
// Aimbot settings. Used for storing the aimbot settings.
AimbotSettings g_AimbotSettings = AimbotSettings();
// Flag for not doing a popup when program closes.
bool g_surveyRemindLater = false;
// Intermediate flag for not doing a popup when program closes.
bool g_remindSet = false;
// Flag for player look direction, if true the player looking direction is drawn on the overlay.
bool g_enablePlayerLookingDirection = true;
// The gui opacity
float g_guiOpacity = 1.0f;
// Flag for showing player names. If true, the players names are drawn on the overlay.
bool g_showPlayerNames = true;
// rapid click flag for the rapid click script
bool g_RapidClick = true;
// render distance for the ESP
int g_RenderDistance = Range_Medium;
// Highest scores in the current lobby
SkillScore g_HighScore = {};
// The lowest scores in the current lobby
SkillScore g_LowScore = {};
// player skill scores, sorted by player id
std::unordered_map<int, SkillScore>  g_SkillScores;
// Mutex to protect access to g_SkillScores
std::shared_mutex g_SkillScoresMutex;
// for picking a bool degubbing dormant finding
int g_boolPick = 0;
// accumulated shots fired by detected player
int g_accumulatedShotsFired = 1;
// acucmulated damage done by detected player
int g_accumulatedDamageDone = 1;
// acucmulated shots missed by detected player
int g_accumulatedShotsMissed = 0;
// refdef_ptr for gui showing for debug
QWORD g_refDefPtr = 0;

// Used for getting the time a player has been idle.
int64_t Player::calcTimeIdle()
{
    return g_idlePlayersTime[this->entIndex] == 0 ? 0 : (int64_t)time(NULL) - g_idlePlayersTime[this->entIndex];
}

// Skip player func. Returns true, if the player should be skipped in the player loop.
// The player should be skipped if the player is dead, (not valid), has no position, or has no entity index.
bool Player::skipPlayer() {
    return
        isDead ||
        g_lPlayer.isInGulag != isInGulag ||
        health == 0 ||
        //!isValid ||
        //!entValid ||
        pos.isZero() ||
        entIndex < 0;
}

// Distance to local player. Returns the distance to the local player.
float Player::distanceToLocal()
{
    float distX = (this->pos.x - g_lPlayer.pos.x);
    float distY = (this->pos.y - g_lPlayer.pos.y);
    float distZ = (this->pos.z - g_lPlayer.pos.z);

    float diffLength = 0;
    diffLength += distX * distX;
    diffLength += distY * distY;
    diffLength += distZ * distZ;
    return (float)sqrt(diffLength);
}

bool score_t::isValid() const {
    return kills > 0 || deaths > 0 || assists > 0 || mp_damage_done > 0 || ping > 0 || points > 0 || rank_level > 0 || icon > 0;
}
// Constructor for the player object with index. Sets all the variables to default values.
Player::Player(int index)
{
    this->lastShootingTs = 0;
    this->decPos = Vector3();
    this->isDead = 0;
    this->entIndex = index;
    this->isValid = 0;
    this->entValid = 0;
    this->pos = Vector3();
    this->team = -1;
    this->stance = CharacterStance::Standing;
    this->inCrosshairOfLocalLastTs = 0;
    this->lastHpZeroTs = 0;
    this->lastVisibleTs = 0;
    this->downedTs = 0;
    this->debug = false;
    this->health = 0;
    this->localInCrosshairDirection = false;
    this->weaponId = 0;
    this->yaw = 0;
    this->pitch = 0;
    this->isVisible = false;
    this->aiming = false;
    this->shooting = false;
    this->speed = 0.0f;
    this->velocity = Vector3();
    this->screenPos = Vector2();
    this->lastUpdateTime = 0;
    this->lastVelocitySampleTs = 0;
    this->isSelfReviving = false;
    this->isInGulag = false;
    this->score = {};
    this->c_entity = {};
    this->dormant = false;
    this->weapon = {};
    this->shotsFired = 0;
}

Player::Player()
{
    this->lastShootingTs = 0;
    this->decPos = Vector3();
    this->isDead = 0;
    this->entIndex = -1;
    this->isValid = 0;
    this->entValid = 0;
    this->pos = Vector3();
    this->team = -1;
    this->stance = CharacterStance::Standing;
    this->inCrosshairOfLocalLastTs = 0;
    this->lastHpZeroTs = 0;
    this->lastVisibleTs = 0;
    this->downedTs = 0;
    this->debug = false;
    this->health = 0;
    this->localInCrosshairDirection = false;
    this->weaponId = 0;
    this->yaw = 0;
    this->pitch = 0;
    this->isVisible = false;
    this->aiming = false;
    this->shooting = false;
    this->speed = 0.0f;
    this->velocity = Vector3();
    this->screenPos = Vector2();
    this->lastUpdateTime = 0;
    this->lastVelocitySampleTs = 0;
    this->isSelfReviving = false;
    this->isInGulag = false;
    this->score = {};
    this->c_entity = {};
    this->dormant = false;
    this->weapon = {};
    this->shotsFired = 0;
}

SkillScore Player::getSkillScore()
{
    SkillScore skill = {};
    int damageTaken;
    int damageDone;
    if (g_MaxPlayers < 30)
    {
        damageTaken = this->score.mp_damage_taken;
        damageDone = this->score.mp_damage_done;
    }
    else
    {
        damageTaken = this->score.extraScore3; // todo not working?
        damageDone = this->score.extraScore4;
    }
    // Normalize the values, using the min/max values in the lobby
    if (g_HighScore.kills - g_LowScore.kills != 0)
        skill.kills = (float)(this->score.kills - g_LowScore.kills) / (float)(g_HighScore.kills - g_LowScore.kills);
    if (g_HighScore.assists - g_LowScore.assists != 0)
        skill.assists = (float)(this->score.assists - g_LowScore.assists) / (float)(g_HighScore.assists - g_LowScore.assists);
    if (g_HighScore.damageDone - g_LowScore.damageDone != 0)
        skill.damageDone = (float)(damageDone - g_LowScore.damageDone) / (float)(g_HighScore.damageDone - g_LowScore.damageDone);
    if (g_HighScore.points - g_LowScore.points != 0)
    skill.points = (float)(this->score.points - g_LowScore.points) / (float)(g_HighScore.points - g_LowScore.points);
    if (g_HighScore.level - g_LowScore.level != 0)
        skill.level = (float)(this->score.rank_level - g_LowScore.level) / (float)(g_HighScore.level - g_LowScore.level);
    /*if (g_HighScore.deaths - g_LowScore.deaths != 0)
        skill.deaths = (float)(g_HighScore.deaths - this->score.deaths) / (float)(g_HighScore.deaths - g_LowScore.deaths);
    if (g_HighScore.damageTaken - g_LowScore.damageTaken != 0)
        skill.damageTaken = (float)(g_HighScore.damageTaken - damageTaken) / (float)(g_HighScore.damageTaken - g_LowScore.damageTaken);*/

    // Calculate the skill index score with weights
    constexpr float DAMAGE_DEALT_WEIGHT = 45.0f;
    constexpr float KILL_WEIGHT = 35.0f;
    constexpr float ASSIST_WEIGHT = 10.0f;
    constexpr float POINTS_WEIGHT = 5.0f;
    constexpr float LEVEL_WEIGHT = 5.0f;
    /*constexpr float DEATHS_WEIGHT = 2.5f;
    constexpr float DAMAGE_TAKEN_WEIGHT = 2.5f;*/
    static_assert(KILL_WEIGHT + DAMAGE_DEALT_WEIGHT + ASSIST_WEIGHT + POINTS_WEIGHT + LEVEL_WEIGHT /*+ DEATHS_WEIGHT + DAMAGE_TAKEN_WEIGHT*/ == 100.0f, "Weights must sum to 100.0");
    skill.skillIndex = 
        skill.kills * KILL_WEIGHT +
        skill.assists * ASSIST_WEIGHT +
        skill.damageDone * DAMAGE_DEALT_WEIGHT +
        skill.points * POINTS_WEIGHT +
        skill.level * LEVEL_WEIGHT/* +
        skill.deaths * DEATHS_WEIGHT +
        skill.damageTaken * DAMAGE_TAKEN_WEIGHT*/;

    skill.playerIndex = this->entIndex;
    return skill;
}

void Player::registerScores()
{
    // skip invalid scores
    if (this->score.rank_level == 0)
        return;
    int damageTaken;
    int damageDone;
    if (g_MaxPlayers < 30)
    {
        damageTaken = this->score.mp_damage_taken;
        damageDone = this->score.mp_damage_done;
    }
    else
    {
        damageTaken = this->score.extraScore3;
        damageDone = this->score.extraScore4;
    }
    // High score registration
    g_HighScore.kills = max(g_HighScore.kills, (float)this->score.kills);
    g_HighScore.assists = max(g_HighScore.assists, (float)this->score.assists);
    g_HighScore.damageDone = max(g_HighScore.damageDone, (float)damageDone);
    g_HighScore.points = max(g_HighScore.points, (float)this->score.points);
    g_HighScore.level = max(g_HighScore.level, (float)this->score.rank_level);
    //g_HighScore.placement = g_MaxPlayers+1;
    // High Scores, where low values are better
    /*g_HighScore.deaths = min(g_HighScore.deaths, this->score.deaths);
    g_HighScore.damageTaken = min(g_HighScore.damageTaken, damageTaken);*/

    // Low score registration
    g_LowScore.kills = min(g_LowScore.kills,  (float)this->score.kills);
    g_LowScore.assists = min(g_LowScore.assists,  (float)this->score.assists);
    g_LowScore.damageDone = min(g_LowScore.damageDone,  (float)damageDone);
    g_LowScore.points = min(g_LowScore.points,  (float)this->score.points);
    g_LowScore.level = min(g_LowScore.level, (float)this->score.rank_level);
    //g_LowScore.placement = -1;
    // Low Scores, where high values are better
   /* g_LowScore.deaths = max(g_LowScore.deaths, this->score.deaths);
    g_LowScore.damageTaken = max(g_LowScore.damageTaken, damageTaken);*/
}


// ImGUI colors. Used for the GUI.
ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
ImColor teamCol = ImColor({46, 116, 255});
ImColor enemyCol = ImColor({255, 0, 0});
ImColor greenCol = ImColor({49, 227, 25});
ImColor pinkCol = ImColor({252, 3, 215});
ImColor orangeCol = ImColor({255, 111, 0});
ImColor darkOrangeCol = ImColor({99, 69, 2});

// getms function. Returns the current time in milliseconds.
long long getms()
{
    namespace sc = std::chrono;
    sc::milliseconds ms = sc::duration_cast<sc::milliseconds>(sc::system_clock::now().time_since_epoch());
    return ms.count();
}

// getSec function. Returns the current time in seconds.
int64_t getSec()
{
    return time(NULL);
}
