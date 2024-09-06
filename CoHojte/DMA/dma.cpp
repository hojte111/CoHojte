#pragma once
#include <cstdint>
#include <iostream>
#include "vmmdll.h"
#include "dma.h"

VMM_HANDLE DMA::hVMM = nullptr;
bool DMA::Connected = false;
uint32_t DMA::AttachedProcessId = 0;
uint64_t DMA::BaseAddress = 0;
uint64_t DMA::PebAddress = 0;
bool DMA::TestMode = false;

bool DMA::Connect()
{
	if (DMA::Connected)
		return true;

	unsigned int iArgumentCount = 3;

	char liveArgs[3][50] = { "", "-device", "fpga" };
	char testArgs[3][50] = { "", "-device", "./pcileech-ingame-dump-shipment.raw" };

	LPSTR argv[3];
	for (int i = 0; i < 3; i++) {
		argv[i] = liveArgs[i];
	}
	
	DMA::hVMM = VMMDLL_Initialize(iArgumentCount, argv);

	if (!DMA::hVMM)
	{
		for (int i = 0; i < 3; i++) {
			argv[i] = testArgs[i];
		}
		DMA::hVMM = VMMDLL_Initialize(iArgumentCount, argv);
		if(!DMA::hVMM) {
			MessageBoxA(nullptr, "Could not initialize the DMA device!", nullptr, 0);
			return false;
		}
		DMA::TestMode = true;
	}

	DMA::Connected = true;

	std::cout << "DMA: Connected\n";
	return true;
}

DWORD DMA::AttachToProcessId(LPSTR ProcessName)
{
	if (!DMA::Connected)
		return 0;

	if (VMMDLL_PidGetFromName(DMA::hVMM, ProcessName, reinterpret_cast<PDWORD>(&DMA::AttachedProcessId)) == FALSE)
		return 0;
	std::cout << "PID: " << DMA::AttachedProcessId << std::endl;

	return DMA::AttachedProcessId;
}

uint64_t DMA::GetBaseAddress()
{
	if (!DMA::Connected)
		return 0;
	auto enc = make_string("cod.exe");
	auto processName = std::wstring(enc.begin(), enc.end());
	RtlSecureZeroMemory(&enc[0], enc.size());
	DMA::BaseAddress = VMMDLL_ProcessGetModuleBase(DMA::hVMM, DMA::AttachedProcessId, const_cast<LPWSTR>(processName.c_str())); // todo
	RtlSecureZeroMemory(&processName[0], processName.size() * sizeof(wchar_t));
	printf("Base Address: 0x%llx\n", DMA::BaseAddress);

	return DMA::BaseAddress;
}

void DMA::Disconnect()
{
	DMA::Connected = false;

	DMA::AttachedProcessId = 0;
	DMA::BaseAddress = 0;

	VMMDLL_CloseAll();
}

uint64_t DMA::GetPEBAddress()
{
	VMMDLL_PROCESS_INFORMATION procInfo = { 0 };
	SIZE_T cbProcInfo = sizeof(VMMDLL_PROCESS_INFORMATION);

	procInfo.magic = VMMDLL_PROCESS_INFORMATION_MAGIC;
	procInfo.wVersion = VMMDLL_PROCESS_INFORMATION_VERSION;
	procInfo.wSize = (WORD)cbProcInfo;

	if (!VMMDLL_ProcessGetInformation(DMA::hVMM, DMA::AttachedProcessId, &procInfo, &cbProcInfo)) {
		std::cerr << "Failed to get process information for PID: " << DMA::AttachedProcessId << std::endl;
		return false;
	}

	DMA::PebAddress = procInfo.win.vaPEB;
	std::cout << "PEB: 0x" << std::hex << DMA::PebAddress << std::dec << std::endl;
	return DMA::PebAddress;
}

