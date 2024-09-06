#include "Game.h"


uint64_t DecryptRefDef() {
    refdefKeyStruct crypt = DMA::Read<refdefKeyStruct>(g_Base + Offsets::REFDEF_PTR);
    DWORD lower = crypt.ref0 ^ (crypt.ref2 ^ (unsigned __int64)(g_Base + Offsets::REFDEF_PTR)) * ((crypt.ref2 ^ (
        unsigned __int64)(g_Base + Offsets::REFDEF_PTR)) + 2);
    DWORD upper = crypt.ref1 ^ (crypt.ref2 ^ (unsigned __int64)(g_Base + Offsets::REFDEF_PTR + 0x4)) * ((crypt.ref2 ^ (
        unsigned __int64)(g_Base + Offsets::REFDEF_PTR + 0x4)) + 2);
    uint64_t refDefKey = (uint64_t)upper << 32 | lower; // Merge Both DWORD into QWORD
    return refDefKey;
}

Weapon_t CoD_GetEntityWeapon(C_Entity a2) {
    return {}; // DMA::Read<Weapon_t>(0x52 * DMA::Read<int16_t>(g_CEntityBase + a2.C_ENTITY_ID) + 2 + DMA::Read<QWORD>(g_Base + 0x14809480));
}

Vector3 TrBase_decrypt(trajectory_t traj) {
    int base[3] = {0, 0, 0};
    DMA::read_memory(g_Base + Offsets::POS_DEC_KEY_BASE, uintptr_t(&base),
                      sizeof(base));
    //if(g_Verbose) printf("key 0x%lX, 0x%lX, 0x%lX\n", decryption_keys[0], decryption_keys[1], decryption_keys[2]);
    int encrypted_keys[3] = {
        static_cast<int>(traj.trBase.x), static_cast<int>(traj.trBase.y), static_cast<int>(traj.trBase.z)
    };

    int v1[3]{0};
    v1[2] = base[2] ^ encrypted_keys[2] ^ encrypted_keys[1];
    v1[1] = base[1] ^ (encrypted_keys[0] ^ encrypted_keys[1]);
    v1[0] = encrypted_keys[0] ^ ~base[0];
    //v1[1] = decryption_keys[1] ^ (encrypted_keys[0] ^ encrypted_keys[1]);
    //dec[2] = decryption_keys[2] ^ encrypted_keys[2] ^ encrypted_keys[1];


    return {static_cast<float>(v1[0]), static_cast<float>(v1[1]), static_cast<float>(v1[2])};
    /*trBase->x = *reinterpret_cast<float*>(&v1[0]);
    trBase->y = *reinterpret_cast<float*>(&v1[1]);
    trBase->z = *reinterpret_cast<float*>(&v1[2]);*/
}

QWORD DecryptCg_t() {
    auto globalPlayerState = DMA::Read<QWORD>(g_Base + Offsets::GLOBAL_PLAYER_STATE);
    if (globalPlayerState < static_cast<QWORD>(Offsets::PREDICTED_PLAYER_STATE)) return 0;
    return globalPlayerState - static_cast<QWORD>(Offsets::PREDICTED_PLAYER_STATE);
}

QWORD DecryptCharacterInfoBase() {
    if (!g_decryptCgTPtr) return 0;
    auto character_displaced_ptr = DMA::Read<QWORD>(g_decryptCgTPtr + Offsets::PREDICTED_CHARACTER_DISP);
    if (character_displaced_ptr < static_cast<QWORD>(Offsets::PREDICTED_CHARACTER_SUBTRACT)) return 0;
    auto local_character = character_displaced_ptr - static_cast<QWORD>(Offsets::PREDICTED_CHARACTER_SUBTRACT);
    return local_character - static_cast<QWORD>(Offsets::SIZE) * g_lPlayer.entIndex;
}

QWORD DecryptCEntity() {
    if (!g_decryptCgTPtr) return 0;
    auto predEntPtr = DMA::Read<QWORD>(g_decryptCgTPtr + Offsets::PREDICTED_C_ENTITY);
    if (predEntPtr) {
        return predEntPtr - g_lPlayer.entIndex * sizeof(C_Entity);
    }
    return 0;
}
