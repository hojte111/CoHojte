#pragma once
#include <winternl.h>
#include "vmmdll.h"
#include "leechcore.h"
#pragma comment(lib,"./DMA/leechcore.lib")
#pragma comment(lib,"./DMA/vmm.lib")
#include "../no_strings.hpp"

namespace DMA
{
    extern VMM_HANDLE hVMM;
    extern bool Connected;
    extern uint32_t AttachedProcessId;
    extern uint64_t BaseAddress;
    extern uint64_t PebAddress;
    extern bool TestMode;

    bool Connect();
    void Disconnect();
    DWORD AttachToProcessId(LPSTR szProcessName);
    uint64_t GetBaseAddress();
    uint64_t GetPEBAddress();

    template<typename T>
    T Read(uintptr_t address)
    {
        T buffer{};
        if (!AttachedProcessId || !Connected || !address) {
            return buffer;
        }

        uint32_t bytesRead = 0;
        uint32_t flags = VMMDLL_FLAG_NOCACHE | VMMDLL_FLAG_NOPAGING | VMMDLL_FLAG_ZEROPAD_ON_FAIL | VMMDLL_FLAG_NOPAGING_IO;

        BOOL bRetn = (VMMDLL_MemReadEx(hVMM, AttachedProcessId, (uint64_t)address, (uint8_t*)&buffer, sizeof(T),
            reinterpret_cast<PDWORD>(&bytesRead), flags) && bytesRead != 0);

        if (!bRetn || bytesRead != sizeof(T)) {
            return buffer;
        }

        return buffer;
    }

    template<typename U>
    uintptr_t ReadPtr(U address)
    {
        return Read<uintptr_t>(address, sizeof(uintptr_t), true);
    }

    inline bool read_memory(uintptr_t address, uintptr_t buffer, size_t reqSize) {
        if (!AttachedProcessId || !Connected || !address) {
            return false;
        }

        uint32_t bytesRead = 0;
        uint32_t flags = VMMDLL_FLAG_NOCACHE | VMMDLL_FLAG_NOPAGING | VMMDLL_FLAG_ZEROPAD_ON_FAIL | VMMDLL_FLAG_NOPAGING_IO;

        BOOL bRetn = VMMDLL_MemReadEx(hVMM, AttachedProcessId, (uint64_t)address, reinterpret_cast<uint8_t*>(buffer), reqSize,
            reinterpret_cast<PDWORD>(&bytesRead), flags) && bytesRead != 0;

        if (!bRetn || bytesRead != reqSize) {
            return false;
        }

        return true;
    }

    // ScatterMemory class defzinition
    class ScatterMemory
    {
    public:
        VMMDLL_SCATTER_HANDLE Initialize()
        {
            return VMMDLL_Scatter_Initialize(hVMM, AttachedProcessId, VMMDLL_FLAG_NOCACHE | VMMDLL_FLAG_NOPAGING_IO);
        }

        bool Clear(VMMDLL_SCATTER_HANDLE hSCATTER)
        {
            return VMMDLL_Scatter_Clear(hSCATTER, AttachedProcessId, VMMDLL_FLAG_NOCACHE | VMMDLL_FLAG_NOPAGING_IO);
        }

        void Close(VMMDLL_SCATTER_HANDLE hSCATTER)
        {
            VMMDLL_Scatter_CloseHandle(hSCATTER);
        }

        bool ExecuteRead(VMMDLL_SCATTER_HANDLE hSCATTER)
        {
            return VMMDLL_Scatter_ExecuteRead(hSCATTER);
        }

        template<typename U, typename P>
        bool PrepareEX(VMMDLL_SCATTER_HANDLE hSCATTER, U vAddress, P pOutput, size_t uiSize)
        {
            if (!hSCATTER || !vAddress || !uiSize) {
                return false;
            }

            return VMMDLL_Scatter_PrepareEx(hSCATTER, vAddress, uiSize, (uint8_t*)pOutput, NULL);
        }
    };
}