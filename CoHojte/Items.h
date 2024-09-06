#pragma once
#include <algorithm>
#include <vector>
#include <regex>
//#include "DMA/dma.h"
#include "Game.h"
#include "Utility.h"

DWORD WINAPI ItemThread(void* params);
std::string makeFriendlyName(const std::string& itemName);

#pragma pack(push, 1)
struct Loot_t
{
    uint64_t namePtr;
    BYTE pad_1[0xC8 - sizeof(namePtr)]; // padding between NAME and VALID
    int valid; // 0xC8
    BYTE pad_2[0x11E - 0xC8 - sizeof(int)]; // padding between VALID and POS
    bool isOpenable; // 0x11E
    BYTE pad_4[0x1D0 - 0x11E - sizeof(bool)]; // padding between ISOPENABLE and pos
    Vector3 pos; // 0x1D0
    BYTE pad_3[0x338 - 0x1D0 - sizeof(Vector3)]; // padding between POS and SIZE
};
#pragma pack(pop)

static_assert(sizeof(Loot_t) == 0x338, "Loot size does not match the specified size.");
static_assert(offsetof(Loot_t, pos) == 0x1D0, "Itm pos incorrect");
static_assert(offsetof(Loot_t, valid) == 0xC8, "Itm valid incorrect");
static_assert(offsetof(Loot_t, isOpenable) == 0x11E, "Itm isOpenable incorrect");
