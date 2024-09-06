#pragma once
#include "driver.h"
#include "globals.h"
#include "dma.h"


namespace Offsets
{
    static const DWORD POS_DEC_KEY_BASE = 0xC070580; // 0x6101268
    static const DWORD C_ENTITY_VALID = 0x224; // 0x6C81416
    // static const DWORD C_ENTITY_STATE = 0x234; // 0x6C81423
    static const DWORD POS_PTR = 0x13680; // 0x6118C77
    static const DWORD DEAD1 = 0x164; // 0x613A9E3
    static const DWORD DEAD2 = 0x4CD; // 0x613AA29
    /*TODO - NO RESULTS!!!*/static const DWORD WEAPON_INDEX = 0x60; // manual UC
    static const DWORD TEAM = 0x12B80; // 0x2F20121
    static const DWORD STANCE = 0x12CCE; //0x612EE49
    static const DWORD SHOOTING = STANCE + 0x4;
    static const DWORD AIMING = SHOOTING + 0x4;
    static const DWORD VALID = 0x13598; // 0x3443795
    static const DWORD SIZE = 0x13E08; // 0x89CA400
    static const DWORD YAW = 0x13D10; // 0x80A7772
    static const DWORD PITCH = YAW + 0x8;
    static const DWORD ENCRYPT_PTR_OFFSET = 0x14FEDD88; // 0x8754FA3
    static const DWORD SCORE_OFS = 0x871D0; // 0x2E8E295
    static const DWORD ENCRYPT_PTR_OFFSET_BASE = 0x199758; // 0x875504A
    static const DWORD LOOT_PTR = 0xE5D1990; // 0x3253AA7
    static const DWORD NAME_ARRAY_OFFSET = 0x151CB638; // 0x2E4E78D
    static const DWORD REFDEF_PTR = 0x151CC3E0; // 0x2A313B8
    static const DWORD GLOBAL_PLAYER_STATE = 0x154313F0; // 0x6979D7A
    static const DWORD PREDICTED_PLAYER_STATE = 0x18160C; // 0x2DC2FB4
    static const DWORD PREDICTED_C_ENTITY = 0x173C78; // 0x2D496E7
    static const DWORD PREDICTED_CHARACTER_DISP = 0x10A378; // 0x6DD0AA7
    static const DWORD PREDICTED_CHARACTER_SUBTRACT = 0x8E0; // 0x6DD0AB3
    static const DWORD LOCAL_INDEX = 0x92190; // 0x2E8D2BE
    static const DWORD visible_enemy_bits = 0x73C04; // 0x2D15255
    static const DWORD WEAPON_DEFINITIONS = 0x150B3110; // 0x2A68A2F
    static const DWORD CAMERA_BASE = 0x15A4F320; // 0x8116506
    static const DWORD CAMERA_POS = 0x204; // 0x8116512
    static const DWORD GAME_MODE = 0x11D8A1B0; // 0x2F574AA
    static const DWORD CGT_WEAPON = 0x18160C; // 0x2C05A4A
    static const DWORD SHOTS_FIRED = CGT_WEAPON + 0x298;
    static const DWORD POS_PADDING = 0x48;
    static const DWORD NAME_ARRAY_PADDING = 0x2C80;
    static const DWORD LOCAL_INDEX_POS = 0x2F0;
}

#pragma pack(push, 1)
struct CharacterInfo {
    int WEAPON_INDEX; // 0x0
    BYTE pad_1[0x160];
    int DEAD1; // 0x164
    BYTE pad_2[0x365];
    int DEAD2; // 0x4CD
    BYTE pad_3[0x126AF];
    unsigned char TEAM; // 0x12B80
    BYTE pad_4[0x14D];
    CharacterStance STANCE; // 0x12CCE
    BYTE pad_5[0x3];
    bool SHOOTING; // 0x12CD2
    BYTE pad_6[0x3];
    bool AIMING; // 0x12CD6
    BYTE pad_7[0x8C1];
    unsigned char VALID; // 0x13598
    BYTE pad_8[0xE7];
    QWORD POS_PTR; // 0x13680
    BYTE pad_9[0x688];
    float YAW; // 0x13D10
    BYTE pad_10[0x4];
    float PITCH; // 0x13D18
    BYTE pad_final[0xEC]; // Final padding
};
#pragma pack(pop)


static_assert(sizeof(CharacterInfo) == Offsets::SIZE, "Player structure size does not match the specified SIZE.");
static_assert(offsetof(CharacterInfo, VALID) == Offsets::VALID, "valid incorrect");
static_assert(offsetof(CharacterInfo, DEAD1) == Offsets::DEAD1, "dead1 incorrect");
static_assert(offsetof(CharacterInfo, DEAD2) == Offsets::DEAD2, "dead2 incorrect");
//static_assert(offsetof(CharacterInfo, WEAPON_INDEX) == Offsets::WEAPON_INDEX, "weap incorrect");
static_assert(offsetof(CharacterInfo, TEAM) == Offsets::TEAM, "team incorrect");
static_assert(offsetof(CharacterInfo, STANCE) == Offsets::STANCE, "stance incorrect");
static_assert(offsetof(CharacterInfo, SHOOTING) == Offsets::SHOOTING, "shot incorrect");
static_assert(offsetof(CharacterInfo, AIMING) == Offsets::AIMING, "aim incorrect");
static_assert(offsetof(CharacterInfo, YAW) == Offsets::YAW, "yaw incorrect");
static_assert(offsetof(CharacterInfo, PITCH) == Offsets::PITCH, "pitch incorrect ");
static_assert(offsetof(CharacterInfo, POS_PTR) == Offsets::POS_PTR, "pos_ptr incorrect");

QWORD DecryptCg_t();
QWORD DecryptCharacterInfoBase();
uint64_t DecryptRefDef();
Vector3 TrBase_decrypt(trajectory_t traj);
QWORD DecryptCEntity();
score_t GetScore(int entIndex);
Weapon_t CoD_GetEntityWeapon(C_Entity a2);
