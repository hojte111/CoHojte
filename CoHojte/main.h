#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <time.h>
#include <chrono>
#include <fstream>
#include <iostream>
#include <thread>
#include <random>
#include <string>
#include <array>
#include <stdio.h>

#include "DMA/dma.h"
#include "AFK.h"
#include "MetricsSampler.h"
#include "Visible.h"
#include "Game.h"
#include "globals.h"
#include "GUI.h"
#include "ImGui/imgui.h"
#include "MemoryUtil.h"
#include "Utility.h"
#include "Items.h"
#include "no_strings.hpp"

DWORD WINAPI AimLoop(void* params);
DWORD WINAPI AimAssistThread(LPVOID lpParam);