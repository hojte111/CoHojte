#pragma once
#define WIN32_LEAN_AND_MEAN
#include <d3d11.h>
#include <D3Dcompiler.h>
#include <dwmapi.h>
#include <dxgi.h>
#include <tchar.h>
#include <bitset>

#include "imgui/imgui.h"
#include "ImGui/imgui_internal.h"
#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_impl_win32.h"
#include "ImGui/imgui_memory_editor.h"
#include "ImGuiUtils.h"
#include "RobotoRegular.h"


#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "dxgi.lib")
#pragma comment (lib, "D3DCompiler.lib")
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

#include "globals.h"
#include "no_strings.hpp"
#include "Utility.h"
//#include "DMA/dma.h"
#include "Game.h"


#define M_PI           3.14159265358979323846f

// Data
static ID3D11Device* pd3dDevice = nullptr;
static ID3D11DeviceContext* pd3dDeviceContext = nullptr;
static IDXGISwapChain* pSwapChain = nullptr;
static UINT ResizeWidth = 0, ResizeHeight = 0;
static ID3D11RenderTargetView* mainRenderTargetView = nullptr;

bool CreateDeviceD3D(HWND hWnd, int width, int height);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

DWORD WINAPI CreateOverlay(LPVOID lpParam);
ImGuiContext* initImGui(HWND hwnd, int screenWidth, int screenHeight, WNDCLASSEX wc);
DWORD WINAPI GUIThread(LPVOID lpParam);
void cleanup(HWND hwnd, WNDCLASSEX wc);

void doOverlay();
void buildSettingsWindow();
void buildMetricsWindow();
void buildDebugWindow();
void buildAimbotSettings(AimbotSettings& settings, ImDrawList* draw_list);
void buildRadarWindow();
// Gui helpers
void drawDottedLine(ImDrawList* drawList, ImVec2 p1, ImVec2 p2, ImU32 color, float thickness, float whiteSpaceLength);
void checkHoverWindows();
float updateRadarScale();
void parseSearchInput(const std::string& input);
// Memory helpers
enum MemorySource { RAW_PLAYER_ARRAY, NAME_ENTRIES, C_ENTITY_ARRAY, CUSTOM };

struct DumpParams {
    std::string fileName;
    const void* data;
    size_t size;
    bool copyNeeded;
};
void HandleMemoryOperations(bool isDumpToFileClicked, bool isViewMemoryClicked);
DWORD WINAPI MemoryViewThread(LPVOID lpParam);
DWORD WINAPI DumpMemoryToFileThread(LPVOID lpParam);
DWORD WINAPI SimulateKeyPressThread(LPVOID lpParam);
void showCloseModal();
bool surveyButton();
void HelpMarker(const char* desc);
void DisplayPlayerDebugInfo(Player& player);
template <typename T>
void displayCopyableAddressInHex(std::string prefix, T address)
{
    const std::string addressText = prefix + toHexString(address);
    if (ImGui::Selectable(addressText.c_str(), false, ImGuiSelectableFlags_AllowDoubleClick))
        ImGui::SetClipboardText(toHexString(address).c_str());
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Click to copy address to clipboard");
}
BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData);