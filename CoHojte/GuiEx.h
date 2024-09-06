#pragma once
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_impl_dx11.h"
#include <d3d12.h>
#include <dxgi1_4.h>
#include <tchar.h>
#include <time.h>
#include "globals.h"
#include "frameContext.h"
#include <vector>
#include "Utility.h"
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib, "windowscodecs.lib")
#pragma comment(lib, "dxgi")

int mainWindow();
void quitGUI();

class EasterEgg
{
public:
    ImVec2 wpos;
    std::string name;
    int npos;
    int val;
    int plus;
    int displace;
    bool show;
};

struct MonitorRects
{
    std::vector<RECT> rcMonitors;

    static BOOL CALLBACK MonitorEnum(HMONITOR hMon, HDC hdc, LPRECT lprcMonitor, LPARAM pData)
    {
        MonitorRects* pThis = reinterpret_cast<MonitorRects*>(pData);
        pThis->rcMonitors.push_back(*lprcMonitor);
        return TRUE;
    }

    MonitorRects()
    {
        EnumDisplayMonitors(0, 0, MonitorEnum, (LPARAM)this);
    }
};
