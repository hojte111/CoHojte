#pragma once
#include <cstdint>

constexpr auto packet_magic = 0xDEAD1337;
constexpr auto server_ip = 0x7F000001; // 127.0.0.1
#ifndef PUBLIC_RELEASE
enum class PakkeType
{
    packet_copy_memory,
    packet_get_base_address,
    packet_get_pid,
    packet_get_peb,
    packet_completed,
    packet_get_process_list,
    packet_get_req_plist_buf_size
};

struct PakkeKopierHukommelse
{
    uint32_t target_proces_id;
    uint64_t target_adresse;

    uint32_t source_process_id;
    uint64_t source_address;

    size_t size;
};

struct PakkeGetBaseAdresse
{
    uint32_t process_id;
};

struct PakkeGetPeb
{
    uint32_t process_id;
};

struct PakkeGetPid
{
    size_t len;
    wchar_t navn[32];
};

struct PakkeCompleted
{
    uint32_t status;
    size_t value;
};

struct PacketHeader
{
    uint32_t   magic;
    PakkeType type;
};
#else
enum class PakkeType
{
    pakke_kopier_hukommelse,
    pakke_get_base_adresse,
    pakke_get_pid,
    pakke_get_peb,
    pakke_completed
};

struct PakkeKopierHukommelse
{
    uint32_t target_proces_id;
    uint64_t target_adresse;

    uint32_t kilde_proces_id;
    uint64_t kilde_adresse;

    uint32_t size;
};

struct PakkeGetBaseAdresse
{
    uint32_t process_id;
};

struct PakkeGetPeb
{
    uint32_t process_id;
};

struct PakkeGetPid
{
    size_t len;
    wchar_t navn[256];
};

struct PakkeCompleted
{
    uint64_t resultat;
};

struct PacketHeader
{
    //uint32_t   magic;
    PakkeType type;
};
#endif
struct Packet
{
    PacketHeader header;
    union
    {
        PakkeKopierHukommelse kopier_hukommelse;
        PakkeGetBaseAdresse get_base_adresse;
        PakkeGetPid get_pid;
        PakkeGetPeb get_peb;
        PakkeCompleted completed;
    } data;
};
