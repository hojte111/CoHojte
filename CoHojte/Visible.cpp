#include "Visible.h"

bool is_visible(const ClientBits& visible_enemy_bits, uint64_t entity_num)
{
    std::size_t idx = (entity_num >> 5);

    if (idx >= visible_enemy_bits.size())
        return false;

    auto bitmask = (0x80000000 >> (entity_num & 0x1F));

    return visible_enemy_bits[idx] & (bitmask);
}
