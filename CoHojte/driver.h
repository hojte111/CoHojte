#pragma once
#define WIN32_LEAN_AND_MEAN
#include <WinSock2.h>
#include <stdio.h>
#include <string>


#include "server_shared.h"

// Link to winsock.
#pragma comment(lib, "Ws2_32.lib")
#define STATUS_UNSUCCESSFUL (0xC0000001L)

typedef struct _RESULT
{
    UINT32 status;
    SIZE_T value;
} RESULT, * PRESULT;

BOOLEAN TestConnection();
namespace drvr
{
    void init();
    void deinitialiser();
    extern int currentProcessId;
    SOCKET connect(int port);
    void afbryd(SOCKET connection);

#ifndef PUBLIC_RELESE
    RESULT read_memory(SOCKET forbindelse, uint32_t process_id, uintptr_t address, uintptr_t buffer, size_t size);
#else
    uint32_t read_memory(SOCKET forbindelse, uint32_t process_id, uintptr_t address, uintptr_t buffer, size_t size);
#endif
    uint64_t get_process_base_adresse(SOCKET forbindelse, uint32_t process_id);
    uint64_t get_process_pid(SOCKET forbindelse, const std::wstring& processName);
    uint64_t get_process_peb(SOCKET forbindelse, uint32_t process_id);

    template <typename T>
    T rd(const SOCKET forbindelse, const uint32_t process_id, const uintptr_t adresse)
    {
        T buffer{};

        read_memory(forbindelse, process_id, adresse, uint64_t(&buffer), sizeof(T));

        return buffer;
    }
}
