#include "driver.h"

#include "globals.h"
#include "no_strings.hpp"

int drvr::currentProcessId = 0;
// Link to winsock.
#pragma comment(lib, "Ws2_32.lib")
#ifndef PUBLIC_RELEASE

BOOLEAN TestConnection()
    {
        WSADATA wsaData;

        // Initialize Winsock
        int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (iResult != 0)
        {
            return FALSE;
        }

        const SOCKET connectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (connectSocket == INVALID_SOCKET)
        {
            WSACleanup();
            return FALSE;
        }

        SOCKADDR_IN clientService;
        clientService.sin_family = AF_INET;
        clientService.sin_addr.s_addr = htonl(server_ip); // Server IP
        clientService.sin_port = htons(7000); // Server Port

        iResult = connect(connectSocket, reinterpret_cast<SOCKADDR*>(&clientService), sizeof(clientService));
        if (iResult == SOCKET_ERROR)
        {
            closesocket(connectSocket);
            WSACleanup();
            return FALSE;
        }

        // Example process name to search for
        const auto enc = make_string("Notepad.exe");
        const auto processName = std::wstring(enc.begin(), enc.end());

        Packet packet;
        packet.header.type = PakkeType::packet_get_pid; // Set the packet type
        packet.data.get_pid.len = processName.length();
        wcsncpy_s(packet.data.get_pid.navn, processName.c_str(), processName.length() + 1);
        // Copy the process name into the struct

        iResult = send(connectSocket, reinterpret_cast<const char*>(&packet), sizeof(packet), 0);
        if (iResult == SOCKET_ERROR)
        {
            closesocket(connectSocket);
            WSACleanup();
            return FALSE;
        }

        char receiveBuffer[sizeof(Packet)]; // Buffer large enough to hold the incoming packet
        constexpr int receiveBufLen = sizeof(Packet); // The expected length of the packet

        iResult = recv(connectSocket, receiveBuffer, receiveBufLen, 0);
        if (iResult != sizeof(Packet))
        {
            closesocket(connectSocket);
            WSACleanup();
            return FALSE;
        }

        // Assuming you've received the packet fully and correctly
        const auto receivedPacket = reinterpret_cast<Packet*>(receiveBuffer);

        // Check if the packet magic and type are correct
        if (receivedPacket->header.magic != packet_magic || receivedPacket->header.type != PakkeType::packet_completed)
        {
            closesocket(connectSocket);
            WSACleanup();
            return FALSE;
        }

        closesocket(connectSocket);
        WSACleanup();
        return TRUE;
    }

// Send request packet and wait for completion.
static bool send_pakke( const SOCKET connectSocket, const Packet& packet, PRESULT response)
{
    if (send(connectSocket, (const char*)&packet, sizeof(packet), 0) == SOCKET_ERROR)
    {
        return FALSE;
    }

    char receiveBuffer[sizeof(Packet)]; // Buffer large enough to hold the incoming packet
    constexpr int receiveBufLen = sizeof(Packet); // The expected length of the packet

    int iResult = recv(connectSocket, receiveBuffer, receiveBufLen, 0);
    if (iResult != sizeof(Packet))
    {
        return FALSE;
    }

    // Assuming you've received the packet fully and correctly
    const auto receivedPacket = reinterpret_cast<Packet*>(receiveBuffer);

    // Check packet type
    if (receivedPacket->header.type == PakkeType::packet_completed && receivedPacket->header.magic == packet_magic)
    {
        *response = RESULT(receivedPacket->data.completed.status, receivedPacket->data.completed.value);
        return TRUE;
    }
    return FALSE;
}

uint64_t drvr::get_process_pid(SOCKET forbindelse, const std::wstring& processName)
{
    Packet pakke{};

    //packet.header.magic = packet_magic;
    pakke.header.type = PakkeType::packet_get_pid;

    auto& data = pakke.data.get_pid;
    wcsncpy_s(pakke.data.get_pid.navn, processName.c_str(), processName.length() + 1);

    data.len = processName.length();

    RESULT resultat;
    if (send_pakke(forbindelse, pakke, &resultat))
    {
        memset(data.navn, 0x00, 32);
        return resultat.value;
    }
    memset(data.navn, 0x00, 32);
    return 0;
}

uint64_t drvr::get_process_base_adresse(const SOCKET connection, const uint32_t process_id)
{
    Packet pakke{};

    //packet.header.magic = packet_magic;
    pakke.header.type = PakkeType::packet_get_base_address;


    auto& data = pakke.data.get_base_adresse;
    data.process_id = process_id;


    RESULT resultat;
    if (send_pakke(connection, pakke, &resultat))
        return resultat.value;

    return 0;
}

uint64_t drvr::get_process_peb(const SOCKET connection, const uint32_t process_id)
{
    Packet pakke{};


    //packet.header.magic = packet_magic;
    pakke.header.type = PakkeType::packet_get_peb;

    auto& data = pakke.data.get_peb;
    data.process_id = process_id;

    RESULT resultat;
    if (send_pakke(connection, pakke, &resultat))
        return resultat.value;

    return 0;
}

static RESULT kopier_hukommelse(
    const SOCKET forbindelse,
    const uint32_t src_process_id,
    const uintptr_t src_address,
    const uint32_t dest_process_id,
    const uintptr_t dest_address,
    const size_t size)
{
    Packet pakke{};

    //packet.header.magic = packet_magic;
    pakke.header.type = PakkeType::packet_copy_memory;

    auto& data = pakke.data.kopier_hukommelse;
    data.source_process_id = src_process_id;
    data.source_address = uint64_t(src_address);
    data.target_proces_id = dest_process_id;


    data.target_adresse = uint64_t(dest_address);
    data.size = uint64_t(size);
    RESULT result;
    if (send_pakke(forbindelse, pakke, &result))
    {
        //printf("Resultat:laes: status: %x, adr: %x\n", pakke.data.fuldfoert.resultat);
        return result;
    }

    return RESULT(STATUS_UNSUCCESSFUL, 0);
}

void drvr::init()
{
    WSADATA wsa_data;
    WSAStartup(MAKEWORD(2, 2), &wsa_data);
    currentProcessId = GetCurrentProcessId();
}

SOCKET drvr::connect(int port)
{
    SOCKADDR_IN adresse{};

    adresse.sin_family = AF_INET;
    adresse.sin_addr.s_addr = htonl(server_ip);

    adresse.sin_port = htons(port);


    const auto forbindelse = socket(AF_INET, SOCK_STREAM, 0);


    if (forbindelse == INVALID_SOCKET)
        return INVALID_SOCKET;

    if (connect(forbindelse, (SOCKADDR*)&adresse, sizeof(adresse)) == SOCKET_ERROR)
    {
        closesocket(forbindelse);
        return INVALID_SOCKET;
    }

    return forbindelse;
}

void drvr::afbryd(const SOCKET forbindelse)
{
    closesocket(forbindelse);
}

void drvr::deinitialiser()
{
    WSACleanup();
}

RESULT drvr::read_memory(
    const SOCKET forbindelse,
    const uint32_t process_id,
    const uintptr_t address,
    const uintptr_t buffer,
    const size_t size)
{
    return kopier_hukommelse(forbindelse, process_id, address, currentProcessId, buffer, size);
}
#else
static bool send_pakke(
    const SOCKET connection,
    const Packet& packet,
    uint64_t& out_result)
{
    Packet completed_packet{};

    if (send(connection, (const char*)&packet, sizeof(Packet), 0) == SOCKET_ERROR)
    {
        printf(make_string("Driver error\n").c_str());
        if (g_errMsg.empty()) g_errMsg = make_string("An error has occurred. Please restart the system.");
        //Beep(900, 2000); // 900 hertz 2 sec
        return false;
    }

    const auto resultat = recv(connection, (char*)&completed_packet, sizeof(Packet), 0);
    if (resultat < sizeof(PacketHeader) ||
        //completion_packet.header.magic != packet_magic ||
        completed_packet.header.type != PakkeType::pakke_completed)
    {
        printf(make_string("Driver errer: %i\n").c_str(), completed_packet.header.type);
        if (g_errMsg.empty()) g_errMsg = make_string("An internal error has occurred. Please restart the system.");
        //Beep(900, 2000); // 900 hertz 2 sec
        return false;
    }
    out_result = completed_packet.data.completed.resultat;
    //if (!NT_SUCCESS(out_result)) printf("Pakke status: %llX\n", out_result);
    return true;
}

uint64_t drvr::get_process_pid(SOCKET forbindelse, const std::wstring& processName)
{
    Packet pakke{};

    //packet.header.magic = packet_magic;
    pakke.header.type = PakkeType::pakke_get_pid;

    auto& data = pakke.data.get_pid;
    wcsncpy_s(pakke.data.get_pid.navn, processName.c_str(), processName.length() + 1);
    // Copy the process name into the struct
    data.len = processName.length();
    
    uint64_t resultat = 0;
    if (send_pakke(forbindelse, pakke, resultat))
    {
        RtlSecureZeroMemory(&pakke, sizeof(pakke));
        return resultat;
    }
    RtlSecureZeroMemory(&pakke, sizeof(pakke));
    return 0;
}

uint64_t drvr::get_process_base_adresse(const SOCKET connection, const uint32_t process_id)
{
    Packet pakke{};

    //packet.header.magic = packet_magic;
    pakke.header.type = PakkeType::pakke_get_base_adresse;


    auto& data = pakke.data.get_base_adresse;
    data.process_id = process_id;


    uint64_t resultat = 0;
    if (send_pakke(connection, pakke, resultat))
        return resultat;

    return 0;
}

uint64_t drvr::get_process_peb(const SOCKET connection, const uint32_t process_id)
{
    Packet pakke{};


    //packet.header.magic = packet_magic;
    pakke.header.type = PakkeType::pakke_get_peb;

    auto& data = pakke.data.get_peb;
    data.process_id = process_id;

    uint64_t resultat = 0;


    if (send_pakke(connection, pakke, resultat))
        return resultat;

    return 0;
}

static uint32_t kopier_hukommelse(
    const SOCKET forbindelse,
    const uint32_t src_process_id,
    const uintptr_t src_address,
    const uint32_t dest_process_id,
    const uintptr_t dest_address,
    const size_t size)
{
    Packet pakke{};

    //packet.header.magic = packet_magic;
    pakke.header.type = PakkeType::pakke_kopier_hukommelse;

    auto& data = pakke.data.kopier_hukommelse;
    data.kilde_proces_id = src_process_id;
    data.kilde_adresse = uint64_t(src_address);
    data.target_proces_id = dest_process_id;


    data.target_adresse = uint64_t(dest_address);
    data.size = uint64_t(size);
    uint64_t result = 0;
    if (send_pakke(forbindelse, pakke, result))
    {
        //printf("Resultat:laes: status: %x, adr: %x\n", pakke.data.fuldfoert.resultat);
        return uint32_t(result);
    }

    return 0;
}

void drvr::init()
{
    WSADATA wsa_data;
    WSAStartup(MAKEWORD(2, 2), &wsa_data);
    currentProcessId = GetCurrentProcessId();
}

SOCKET drvr::connect(int port)
{
    SOCKADDR_IN adresse{};

    adresse.sin_family = AF_INET;
    adresse.sin_addr.s_addr = htonl(server_ip);

    adresse.sin_port = htons(port);


    const auto forbindelse = socket(AF_INET, SOCK_STREAM, 0);


    if (forbindelse == INVALID_SOCKET)
        return INVALID_SOCKET;

    if (connect(forbindelse, (SOCKADDR*)&adresse, sizeof(adresse)) == SOCKET_ERROR)
    {
        closesocket(forbindelse);
        return INVALID_SOCKET;
    }

    return forbindelse;
}

void drvr::afbryd(const SOCKET forbindelse)
{
    closesocket(forbindelse);
}

void drvr::deinitialiser()
{
    WSACleanup();
}

uint32_t drvr::read_memory(
    const SOCKET forbindelse,
    const uint32_t process_id,
    const uintptr_t address,
    const uintptr_t buffer,
    const size_t size)
{
    return kopier_hukommelse(forbindelse, process_id, address, currentProcessId, buffer, size);
}
#endif
