#include "GUI.h"

#include "dma.h"

// GUI Globals
HWND gui_hwnd = nullptr;
std::string guid;
ImVec2 debugWindowPos = ImVec2(0, 0);
ImVec2 debugWindowSize = ImVec2(0, 0);
ImVec2 settingsWindowPos = ImVec2(0, 0);
ImVec2 settingsWindowSize = ImVec2(0, 0);
ImVec2 radarWindowPos = ImVec2(0, 0);
ImVec2 radarWindowSize = ImVec2(0, 0);
ImVec2 metricsWindowPos = ImVec2(0, 0);
ImVec2 metricsWindowSize = ImVec2(0, 0);
ImDrawList* overlayDrawList = nullptr;
bool enablePlayerLookingDirection = true;
int keyToSimulate = 0;
bool doEntityESP = true;
float guiScaleFactor = interpolateScaleBasedOnHeight(); // at 1080p // 4k = 1.75f
auto stylingFactor = max(1.0f, guiScaleFactor);
bool imGuiWindowOpen = true;
bool wmCloseReq = false;
bool remindWasSet = false;
ImVec2 centerMiniMap = ImVec2(169, 155);

MemoryEditor mem_edit;
std::atomic showMemoryEditor(false);
std::vector<UINT8> memoryViewBuffer;
int memoryViewSource = RAW_PLAYER_ARRAY;
size_t memoryViewBaseAddress = 0;
int selectedMemorySource = RAW_PLAYER_ARRAY;

uintptr_t customAddress = 0;
size_t customSize = 0;
int sizeUnit = 0; // 0 for bytes, 1 for KB, 2 for MB
int selectedAddressOption = 0;

// Static text and configuration
const auto overlayActiveText = make_string("ESP");
const char* zoom_levels[] = {
    "1", "2", "3", "4", "5"
};
std::string keys_simulate_1;
std::string keys_simulate_2;
std::string keys_simulate_3;
std::string keys_simulate_4;

std::string addressOptions_0; // None
std::string addressOptions_1; // Module Base Address
std::string addressOptions_2; // Peb Address
std::string addressOptions_3; // Cg_t Address
std::string addressOptions_4; // Client Base Address
std::string addressOptions_5; // CEntity Address

std::string memorySourceOptions_0;
std::string memorySourceOptions_1;
std::string memorySourceOptions_2;
std::string memorySourceOptions_3;

std::string aESP;
std::string aRadarZoomLevel;
std::string aClose;
std::string aMedium;
std::string aFar;
std::string aReviveStim;
std::string aPlateDeploy;

std::string aCrosshairActive;
std::string aAimbotActive;
std::string aEnableAlert;
std::string aAlertHelp;
std::string aSoftReset;
std::string aOpacity;
std::string aClear;
std::string aAr;
std::string aLm;
std::string aPi;
std::string aSm;
std::string aSh;
std::string aSn;
std::string aMe;
std::string aPlayerLookDirection;
std::string aDrawPlayerNames;
std::string aLookDirectionHelp;
std::string aSSF;
std::string aEspRenderDistance;
std::string aResetAimbotSettings;
std::string aHelp;
std::string zoomDecription;
std::string reviveStim;
std::string plateDeploy;
std::string scaleText;
std::string aNormalRange;
std::string aLongRange;
std::string aEspActive;
std::string aAfkBot;
std::string aPlayAgainAdjustment;
std::string aDebugMode;
std::string aRadarNorthOriented;
std::string aItemESP;
std::string aLinkTeams;
std::string aGameAddress;
std::string aCgTAddress;
std::string aDecryptionCase;
std::string aCharacterInfoAddress;
std::string aCEntityAddress;
std::string aGameHandle;
std::string aFPS;
std::string aItemSearch;
std::string aErrorMessage;
std::string aAfkState;
std::string aLocalPlayerInfo;
std::string aRefDebugInfo;
std::string aValidPlayersDebugInfo;
std::string aInputValueSearch;
std::string aSearchInteger;
std::string aSearchFloat;
std::string aQueOffset;
std::string aSetGW;
std::string aF12Scan;
std::string aEnableMemoryView;
std::string aAddress;
std::string aValAsInt;
std::string aValAsFloat;
std::string aInputOutput;
std::string aEntityESP;
std::string aSimulate;
std::string aRapidClick;

// Colors
constexpr ImU32 magenta = IM_COL32(226, 0, 116, 200);
ImU32 red = IM_COL32(255, 0, 0, 255 * g_guiOpacity);
ImU32 neonYellow = IM_COL32(207, 255, 4, 255 * g_guiOpacity);
ImU32 gray = IM_COL32(128, 128, 128, 255 * g_guiOpacity);
ImU32 green = IM_COL32(0, 255, 0, 255 * g_guiOpacity);
ImU32 darkGreen = IM_COL32(0, 100, 0, 255 * g_guiOpacity);
ImU32 yellow = IM_COL32(255, 255, 0, 255 * g_guiOpacity);
ImU32 orange = IM_COL32(255, 165, 0, 255 * g_guiOpacity);
ImU32 miniPlrOrange = IM_COL32(255, 87, 87, 255 * g_guiOpacity);
ImU32 blue = IM_COL32(0, 0, 255, 255 * g_guiOpacity);
ImU32 cyan = IM_COL32(0, 255, 255, 255 * g_guiOpacity);
ImU32 cyanCream = IM_COL32(0, 255, 200, 255 * g_guiOpacity);
ImU32 purple = IM_COL32(160, 10, 190, 255 * g_guiOpacity);
ImU32 pink = IM_COL32(255, 20, 147, 255 * g_guiOpacity);
ImU32 white = IM_COL32(255, 255, 255, 255 * g_guiOpacity);

// Configuration
const auto settingsHeight = 200;
const auto settingsWidth = 150;
float RADAR_SCALE = 0.01f;
const int RADAR_RADIUS = 250;
int totalScreenWidth;
int totalScreenHeight;

RECT monitorRect;

const int settingsXLocation = 500;
char inputBuffer[36] = {0};
char itemSearchBuffer[256] = "orange,urani";


DWORD WINAPI GUIThread(LPVOID lpParam)
{
    aESP = make_string("ESP");
    aRadarZoomLevel = make_string("Radar Zoom level");
    aClose = make_string("Close");
    aMedium = make_string("Medium");
    aFar = make_string("Far");
    aPlateDeploy = make_string("plate_deploy");
    aReviveStim = make_string("revive_stim");
    aCrosshairActive = make_string("Crosshair[5]");
    aAimbotActive = make_string("Aimbot Experimental");
    aEnableAlert = make_string("High Alert");
    aAlertHelp = make_string("Acts like the high alert perk");
    aSoftReset = make_string("Soft Reset");
    aOpacity = make_string("ESP Opacity");
    aClear = make_string("Clear screen message");
    aAr = make_string("_ar_");
    aLm = make_string("_lm_");
    aPi = make_string("_pi_");
    aSm = make_string("_sm_");
    aSh = make_string("_sh_");
    aSn = make_string("_sn_");
    aMe = make_string("_me_");
    aPlayerLookDirection = make_string("Player Look Direction");
    aDrawPlayerNames = make_string("Draw Player Names");
    aLookDirectionHelp = make_string("Shows where the player is looking on the overlay.");
    aSSF = make_string("%s-%s-%.0f");
    aEspRenderDistance = make_string("ESP Render distance");
    aResetAimbotSettings = make_string("Reset aimbot settings");
    aHelp = make_string("Help: Discord hojte111");
    zoomDecription = make_string("Zoom level");
    reviveStim = make_string("revive_stim");
    plateDeploy = make_string("plate_deploy");
    scaleText = make_string("Scale=%.3f");
    aNormalRange = make_string("NormalRange");
    aLongRange = make_string("Long Range[L]");
    aEspActive = make_string("ESP[6]");
    aAfkBot = make_string("AFK Bot[F7]");
    aPlayAgainAdjustment = make_string("PlayAgain Button Adjustment");
    aDebugMode = make_string("Debug Mode[F8]");
    aRadarNorthOriented = make_string("North oriented radar[9]");
    aItemESP = make_string("Item ESP");
    aLinkTeams = make_string("Link Teams ESP");
    aGameAddress = make_string("Game address:");
    aCgTAddress = make_string("Cg_t address: 0x%llX");
    aDecryptionCase = make_string("Decryption case: %d");
    aCharacterInfoAddress = make_string("CharacterInfo base address: 0x%llX");
    aCEntityAddress = make_string("C_Entity base address: 0x%llX");
    aGameHandle = make_string("Game window handle: 0x%I64x");
    aFPS = make_string("FPS: %.1f");
    aItemSearch = make_string("Item Name Search");
    aErrorMessage = make_string("Error Message: %s");
    aAfkState = make_string("AFK state: --> %s");
    aRefDebugInfo = make_string("RefDef: (%.2f, %.2f) FOV(%.3f,%.3f) w:%d,h:%d G bug: %.0f,%.0f");
    aValidPlayersDebugInfo = make_string("Valid Players: %d, MaxPayers: %d, hasVisibleEnemy %d, HighAlertCount %lld, BotsOnscreen %d");
    aInputValueSearch = make_string("<- input value");
    aSearchInteger = make_string("Search Integer");
    aSearchFloat = make_string("Search Float");
    aQueOffset = make_string("Que 0x offset");
    aSetGW = make_string("set g_W value");
    aF12Scan = make_string("F12 scan value: %d, %.4f");
    aEnableMemoryView = make_string("View Local CharacterInfo memory");
    aAddress = make_string("Adr");
    aValAsInt = make_string("ValAsInt");
    aValAsFloat = make_string("ValAsFloat");
    aInputOutput = make_string("0x%X -> (int) %d");
    aEntityESP = make_string("Entity ESP");
    aSimulate = make_string("Simulate key stroke");
    aRapidClick = make_string("X1 Rapid Click");
    
    // Multi-Monitor support
    monitorRect.left = monitorRect.top = INT_MAX;
    monitorRect.right = monitorRect.bottom = INT_MIN;
    EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, 0);
    totalScreenWidth = monitorRect.right - monitorRect.left;
    totalScreenHeight = monitorRect.bottom - monitorRect.top;
    
    WNDCLASSEX wc = {
        sizeof(WNDCLASSEX), CS_CLASSDC, WindowProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, _T("Assistant"),
        NULL
    };
    ::RegisterClassEx(&wc);

    gui_hwnd = CreateWindowEx(WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED, wc.lpszClassName, wc.lpszClassName,
                              WS_POPUP, monitorRect.left, monitorRect.top, totalScreenWidth, totalScreenHeight, nullptr, nullptr, wc.hInstance, nullptr);
    if (!gui_hwnd) return 1;
    ImGuiContext* guiContext = initImGui(gui_hwnd, totalScreenWidth, totalScreenHeight, wc);
    if (!guiContext) return 1;
    if (monitorRect.left != 0 || monitorRect.top != 0) {
        g_errMsg = make_string("Adjust your monitor setup, the overlay is not on the main monitor.");
        //closeProgramRequest = getms() - 2000;
    }
    g_WarfareHeight = GetSystemMetrics(SM_CYSCREEN); //g_Refdef.height;
    g_WarfareWidth = GetSystemMetrics(SM_CXSCREEN); // g_Refdef.width;
    keys_simulate_1 = make_string("Jump - Scroll Tilt Right");
    keys_simulate_2 = make_string("Tactical - Scroll Tilt Left");
    keys_simulate_3 = make_string("Shoot - X1"); // field upgrade is scroll up
    keys_simulate_4 = make_string("Forward - X2");

    addressOptions_0 = make_string("None");
    addressOptions_1 = make_string("Module Base Address");
    addressOptions_2 = make_string("Peb Address");
    addressOptions_3 = make_string("Cg_t Address");
    addressOptions_4 = make_string("CharacterInfo Address");
    addressOptions_5 = make_string("CEntity Address");
    memorySourceOptions_0 = make_string("CharacterInfo Array");
    memorySourceOptions_1 = make_string("Clientinfo_t Array");
    memorySourceOptions_2 = make_string("C_Entity Array");
    memorySourceOptions_3 = make_string("Custom");
    mem_edit = MemoryEditor();
    mem_edit.OptShowDataPreview = true;
    while (g_Active)
    {
        MSG msg;
        while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
            {
                std::cout << std::endl << make_string("Received WM_QUIT message") << std::endl;
                //g_Active = false;
            }
        }
        // Start the Dear ImGui frame
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        //SetWindowLong(gui_hwnd, GWL_EXSTYLE, GetWindowLong(gui_hwnd, GWL_EXSTYLE) & ~WS_EX_TRANSPARENT);
        SetWindowLong(gui_hwnd, GWL_EXSTYLE, GetWindowLong(gui_hwnd, GWL_EXSTYLE) | WS_EX_TRANSPARENT);
        centerMiniMap = g_MaxPlayers > 30 ? ImVec2(169, 155) : ImVec2(134, 130); // warzone / mp
        //ImGui::ShowDemoWindow();
        doOverlay();
        
        buildSettingsWindow();
        buildMetricsWindow();
        buildDebugWindow();
        if (g_AimBotMode == 1) {
            buildAimbotSettings(g_AimbotSettings, ImGui::GetBackgroundDrawList());
        }
        buildRadarWindow();
        
        // Rendering
        ImGui::Render();
        constexpr float clear_color_with_alpha[4] = {0.0f, 0.0f, 0.0f, 0.0f};
        pd3dDeviceContext->OMSetRenderTargets(1, &mainRenderTargetView, nullptr);
        pd3dDeviceContext->ClearRenderTargetView(mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        pSwapChain->Present(1, 0); // Present with vsync
    }


    cleanup(gui_hwnd, wc);
    printf(make_string("GUI Closed\n").c_str());
    g_Active = false;
    return 0;
}

void doOverlay()
{
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(
        ImVec2(static_cast<float>(totalScreenWidth-3), static_cast<float>(totalScreenHeight-3)),
        ImGuiCond_Always);
    ImGui::Begin("-", nullptr,
                 ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoInputs);
    overlayDrawList = ImGui::GetBackgroundDrawList();
    if (g_decryptBasePtr) overlayDrawList->AddText(NULL, 30.0f, ImVec2(10,10), magenta, aESP.c_str());

    if (!g_errMsg.empty())
    {
        overlayDrawList->AddText(NULL, 30.0f, ImVec2(g_WarfareWidth/2, g_WarfareHeight/3), magenta, g_errMsg.c_str());
    }

    if (g_crosshair && !g_ADS) // green crosshair
            overlayDrawList->AddCircle(ImVec2(g_WarfareWidth / 2, g_WarfareHeight / 2),
                                    3*guiScaleFactor, green);
    
    if (g_Verbose) // playAgain Adjustment
    {   overlayDrawList->AddRect(ImVec2(1500, g_playAgainY + g_playAgainAdjustmentY),
                                    ImVec2(1505, g_playAgainY + g_playAgainAdjustmentY + 5), cyan);
        overlayDrawList->AddRect(ImVec2(35, 22), ImVec2(305, 292), yellow); // show 1st quadrant minimap
    }

    for (int i = 0; i < MAX_ITEM_COUNT && g_ItemESP; i++)
    {
        auto itemBox = g_ItemList[i];
        itemBox.screenPos = worldToScreen(itemBox.pos, g_lPlayer.pos);
        // skip out of view items
        if (itemBox.screenPos.x <= 0 || itemBox.screenPos.y <= 0 || itemBox.pos.isZero() || (!itemBox.isOpenable && itemBox.isContainer)) continue;
        if (!g_ItemSearchKeywords.empty() && !itemBox.isSearched) continue;
        constexpr float slope = (0.5f - 7.0f) / (4000.0f - 300.0f);
        float boxSize = 7.0f + slope * (itemBox.distToLocal - 300.0f);
        if (itemBox.distToLocal < 300) boxSize = 7.0f;
        else if (itemBox.distToLocal > 4000) boxSize = 0.5f;
        overlayDrawList->AddRect(ImVec2(itemBox.screenPos.x, itemBox.screenPos.y),
                                        ImVec2(itemBox.screenPos.x + boxSize, itemBox.screenPos.y + boxSize),
                                        cyanCream);
        if (itemBox.distToLocal < 1000) // within ~25m
            overlayDrawList->AddText(ImVec2(itemBox.screenPos.x, itemBox.screenPos.y - 17), cyanCream,
                                        itemBox.friendlyName.c_str());
    }
    for (int i = 0; i < MAX_TEAM_COUNT && g_DoTeamLink && g_MaxPlayers>20; i++)
    {
        // Check if the current team has members to link
        std::unique_lock lock(g_TeamLinkMutex);
        if (g_TeamToLinkPositions[i].empty()) continue;

        auto currTeamPositions = g_TeamToLinkPositions[i];
        lock.unlock();
        // Loop through each member of the team
        for (size_t j = 0; j < currTeamPositions.size(); j++)
        {
            // Now loop through all other members to draw lines to other team members
            for (size_t k = j + 1; k < currTeamPositions.size(); k++)
            {
                // Get positions of both team members
                Vector2 startPos = currTeamPositions[j];
                Vector2 endPos = currTeamPositions[k];

                // Assume getTeamColor returns Vector3 with RGB values suitable for drawing
                Vector3 teamColor = getTeamColor(i);
                auto color = IM_COL32(teamColor.x, teamColor.y, teamColor.z, 255 * g_guiOpacity);

                drawDottedLine(overlayDrawList, ImVec2(startPos.x, startPos.y), ImVec2(endPos.x, endPos.y), color, 0.5f, 10.f);
            }
        }
    }
    // HIGH ALERT feature
    auto alertAngles = g_AimingAlertAngles;
    for (int i = 0; i < alertAngles.size() && g_EnableAimingAlert; i++)
    {
        float angleToEnemy = alertAngles[i];
        auto [pos, size] = edgeLocationEnemyAimingBox(angleToEnemy);
        overlayDrawList->AddRectFilled(ImVec2(pos.x, pos.y), ImVec2(pos.x + size.x, pos.y + size.y), miniPlrOrange);
    }

    ImGui::End();
}

void buildRadarWindow()
{
    ImGui::SetNextWindowPos(ImVec2(20, 400), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(230, 230), ImGuiCond_FirstUseEver);
    ImGui::Begin("Map");
    radarWindowPos = ImGui::GetWindowPos();
    radarWindowSize = ImGui::GetWindowSize();
    checkHoverWindows();
    std::string seed = g_detectedLocalName;
    ImVec2 RADAR_CENTER = ImVec2(radarWindowPos.x + radarWindowSize.x / 2.0f, radarWindowPos.y + radarWindowSize.y / 2.0f);
    RADAR_SCALE = updateRadarScale();
    ImGui::GetWindowDrawList()->AddRect(
        ImVec2(RADAR_CENTER.x - RADAR_RADIUS, RADAR_CENTER.y - RADAR_RADIUS),
        ImVec2(RADAR_CENTER.x + RADAR_RADIUS, RADAR_CENTER.y + RADAR_RADIUS),
        greenCol,
        5.0f);
    if (g_northOriented)
        ImGui::GetWindowDrawList()->AddText(
            ImVec2(RADAR_CENTER.x - 13, RADAR_CENTER.y - RADAR_RADIUS - 13), greenCol, "NORD");
    ImGui::GetWindowDrawList()->AddLine(ImVec2(RADAR_CENTER.x, RADAR_CENTER.y - RADAR_RADIUS),
                                        ImVec2(RADAR_CENTER.x, RADAR_CENTER.y + RADAR_RADIUS), greenCol,
                                        1.0f); // middle line y (vertical)
    ImGui::GetWindowDrawList()->AddLine(ImVec2(RADAR_CENTER.x - RADAR_RADIUS, RADAR_CENTER.y),
                                        ImVec2(RADAR_CENTER.x + RADAR_RADIUS, RADAR_CENTER.y), greenCol,
                                        1.0f); // middle line x (horizontal)
    // Draw Orientation line <3 (for static view)
    if (g_northOriented)
        ImGui::GetWindowDrawList()->AddLine(ImVec2(RADAR_CENTER.x, RADAR_CENTER.y),
                                            ImVec2(RADAR_CENTER.x + g_localView.x * 20,
                                                   RADAR_CENTER.y - g_localView.y * 20),
                                            ImColor({teamCol}), 1.5f);
    // Meter measure rings
    ImGui::GetWindowDrawList()->AddCircle(
        ImVec2(RADAR_CENTER.x, RADAR_CENTER.y),
        RADAR_RADIUS / 2.0f,
        greenCol
    );

    ImGui::GetWindowDrawList()->AddCircle(
        ImVec2(RADAR_CENTER.x, RADAR_CENTER.y),
        RADAR_RADIUS,
        greenCol
    );
    if (g_northOriented)
    {
        ImGui::GetWindowDrawList()->AddText(ImVec2(RADAR_CENTER.x - 10, RADAR_CENTER.y + RADAR_RADIUS + 2),
                                            greenCol, "180"); // compass South
        ImGui::GetWindowDrawList()->AddText(ImVec2(RADAR_CENTER.x - RADAR_RADIUS - 23, RADAR_CENTER.y - 5),
                                            greenCol, "270"); // compass Wets
        ImGui::GetWindowDrawList()->AddText(ImVec2(RADAR_CENTER.x + RADAR_RADIUS + 2, RADAR_CENTER.y - 5), greenCol,
                                            "90"); // compass East
    }
    else
    {
        // Draw me triangle
        ImGui::GetWindowDrawList()->AddTriangleFilled(ImVec2(RADAR_CENTER.x, RADAR_CENTER.y - 12),
                                                      ImVec2(RADAR_CENTER.x - 8, RADAR_CENTER.y + 5),
                                                      ImVec2(RADAR_CENTER.x + 8, RADAR_CENTER.y + 5), teamCol);
    }
    float degrees = ((-atan2(g_RefDefAxis.x, g_RefDefAxis.y) + M_PI) / 2) * 180 / M_PI * 2;
    //for multiplayer/rebirth island
        degrees = ((-atan2(g_Refdef.view.axis->x, g_Refdef.view.axis->y) + M_PI) / 2) * 180 / M_PI
            * 2; // for warzone
    if (degrees < 90)
    {
        degrees = 90 + (90 - degrees);
    }
    else
    {
        degrees = 90 - (degrees - 90);
    }
    if (degrees < 0) degrees = 360 + degrees;
    degrees++;
    char degreeChar[24];
    int resConv = snprintf(degreeChar, sizeof degreeChar, "%.0f", degrees);
    if (degrees > 349 || degrees < 11) resConv = snprintf(degreeChar, sizeof degreeChar, "%s", "NORTH");
    if (degrees > 79 && degrees < 101) resConv = snprintf(degreeChar, sizeof degreeChar, "%s", "EAST");
    if (degrees > 259 && degrees < 281) resConv = snprintf(degreeChar, sizeof degreeChar, "%s", "WEST");
    if (degrees > 169 && degrees < 191) resConv = snprintf(degreeChar, sizeof degreeChar, "%s", "SOUTH");
    ImGui::GetWindowDrawList()->AddText(ImVec2(RADAR_CENTER.x - 17, RADAR_CENTER.y - RADAR_RADIUS - 25), greenCol,
                                        degreeChar);
    float relativeRotationRadar;
    guid = getGuid(seed);
    bool localHasVisible = false;
    //Draw POS <3
    for (int i = 0; i < MAX_PLAYER_COUNT; i++)
    {
        Player cPlayer = g_playerArr[i];
        if (cPlayer.debug) {
            DisplayPlayerDebugInfo(cPlayer);  // Display each player's info in a separate window
        }
        Vector3 pos = cPlayer.pos;

        float distX = (pos.x - g_lPlayer.pos.x);
        float distY = (pos.y - g_lPlayer.pos.y);
        float distZ = (pos.z - g_lPlayer.pos.z);

        float diffLength = 0.0f;
        diffLength += distX * distX;
        diffLength += distY * distY;
        diffLength = (float)sqrt(diffLength);

        //float drawBorderCap = 750;
        bool skipPlayer =
                cPlayer.skipPlayer() || // dead player
                cPlayer.calcTimeIdle() > 10 || // still standing/looking for > 10 secs
                i == g_lPlayer.entIndex || // ignore local Player
                (pos.z < -600 && g_lPlayer.pos.z > -600) // dont draw gulag unless LP is in gulag too
            ;
        if (skipPlayer)
        {
            g_ESP_DrawList[i] = ESPBox();
            g_Radar_DrawList[i] = RadarTriangle();
            continue;
        }

        // Relative radar rotation
        relativeRotationRadar = -atan2(g_RefDefAxis.x, g_RefDefAxis.y) - M_PI / 2;
        float x = RADAR_CENTER.x + (diffLength * RADAR_SCALE) * cos(relativeRotationRadar - atan2(distY, distX));
        float y = RADAR_CENTER.y + (diffLength * RADAR_SCALE) * sin(relativeRotationRadar - atan2(distY, distX));

        ImVec2 locationCenterRotated = ImVec2(x, y);
        //if (abs(diffLength) > dontDrawCap) // todo: border radar stuff (normalize)

        ImVec2 locationCenter = ImVec2(RADAR_CENTER.x + distX * RADAR_SCALE, RADAR_CENTER.y - distY * RADAR_SCALE);

        ImVec2 finalDrawPos = g_northOriented ? locationCenter : locationCenterRotated;


        /*  Enemy Orientation  */
        float enemy_pointing = enemy_pointing = g_lPlayer.yaw - cPlayer.yaw;
        enemy_pointing -= 90;
        Vector2 triangleTip = RotatePoint(Vector2(finalDrawPos.x + 17, finalDrawPos.y),
                                          Vector2(finalDrawPos.x, finalDrawPos.y), enemy_pointing);
        Vector2 triangleLeft = RotatePoint(Vector2(finalDrawPos.x - 5, finalDrawPos.y + 10),
                                           Vector2(finalDrawPos.x, finalDrawPos.y), enemy_pointing);
        Vector2 triangleRight = RotatePoint(Vector2(finalDrawPos.x - 5, finalDrawPos.y - 10),
                                            Vector2(finalDrawPos.x, finalDrawPos.y), enemy_pointing);

        if (cPlayer.team == g_lPlayer.team)
        {
            //Team Mate
            // Over or under Lines
            if (distZ > 60)
                ImGui::GetWindowDrawList()->AddLine(finalDrawPos,
                                                    ImVec2(finalDrawPos.x, finalDrawPos.y - 10),
                                                    teamCol, 2.5f); // player is below
            if (distZ < -60)
                ImGui::GetWindowDrawList()->AddLine(finalDrawPos,
                                                    ImVec2(finalDrawPos.x, finalDrawPos.y + 10),
                                                    teamCol, 2.5f); // player is above
            // pos triangle team mate
            ImGui::GetWindowDrawList()->AddTriangleFilled(ImVec2(triangleLeft.x, triangleLeft.y),
                                                          ImVec2(triangleRight.x, triangleRight.y),
                                                          ImVec2(triangleTip.x, triangleTip.y), teamCol);
        }
        else
        {
            // enemy
            ImColor lineGrad;
            ImColor enemyEvalCol;
            if (cPlayer.stance == CharacterStance::Downed) enemyEvalCol = purple; // purple if downed
            else if (cPlayer.weapon_name.find(aReviveStim.c_str()) != std::string::npos) enemyEvalCol = ImColor(13, 255, 0);
                // green if reviving
            else if (cPlayer.weapon_name.find(aPlateDeploy.c_str()) != std::string::npos) enemyEvalCol = ImColor(0, 13, 255);
                // blue if plating
            else if (cPlayer.shooting) enemyEvalCol = orangeCol; // orange if shooting
            else if (cPlayer.aiming) enemyEvalCol = darkOrangeCol; // dark orange if aimed
            else enemyEvalCol = enemyCol;
            //if (cPlayer.dormant) enemyEvalCol = gray; // gray if dormant
            if ((((abs(distZ) - 60) / 2) / 255.0f) < 1.0f)
                lineGrad = ImColor(ImVec4{
                    255.0f, 255.0f, 0.0f, (((abs(distZ) - 60.0f) / 2.0f) / 255.0f) + 0.1f
                }); // more yellow the farer up/down
            else lineGrad = ImColor({255, 255, 0}); // just plain yellow if over gradient cap
            // over or under lines
            if (distZ > 60)
                ImGui::GetWindowDrawList()->AddLine(finalDrawPos,
                                                    ImVec2(finalDrawPos.x, finalDrawPos.y - 15),
                                                    lineGrad, 2.5f); // over
            if (distZ < (-60))
                ImGui::GetWindowDrawList()->AddLine(finalDrawPos,
                                                    ImVec2(finalDrawPos.x, finalDrawPos.y + 15),
                                                    lineGrad, 2.5f); // under
            // pos circle
            ImGui::GetWindowDrawList()->AddTriangleFilled(ImVec2(triangleLeft.x, triangleLeft.y),
                                                          ImVec2(triangleRight.x, triangleRight.y),
                                                          ImVec2(triangleTip.x, triangleTip.y), enemyEvalCol);
            char teamChar[11];
            snprintf(teamChar, sizeof teamChar, "%d", cPlayer.team);
            ImGui::GetWindowDrawList()->AddText(ImVec2(finalDrawPos.x - 6, finalDrawPos.y - 6),
                                                ImColor(1.0f, 1.0f, 1.0f, 1.0f), teamChar); // draw team num
        }
        // Distance text
        char distChar[11];
        float meterDist = cPlayer.distanceToLocal() / 41.6666666666667f; // maybe wZ?
        snprintf(distChar, sizeof(distChar), "%.0f", meterDist);
        ImGui::GetWindowDrawList()->AddText(ImVec2(finalDrawPos.x + 4, finalDrawPos.y + 4),
                                            ImColor(1.0f, 1.0f, 1.0f, 1.0f), distChar); // draw distance

        // Weapon categories
        std::string weaponType;
        if (cPlayer.weapon_name.find(aAr.c_str()) != std::string::npos) weaponType = "AR"; // Assault Rifles
        if (cPlayer.weapon_name.find(aLm.c_str()) != std::string::npos) weaponType = "LM"; // Light Machine Guns
        if (cPlayer.weapon_name.find(aPi.c_str()) != std::string::npos) weaponType = "P"; // Pistols
        if (cPlayer.weapon_name.find(aSm.c_str()) != std::string::npos) weaponType = "SM"; // Submachineguns
        if (cPlayer.weapon_name.find(aSh.c_str()) != std::string::npos) weaponType = "SH"; // Shotguns
        if (cPlayer.weapon_name.find(aSn.c_str()) != std::string::npos) weaponType = "SR"; // Sniper Rifles
        if (cPlayer.weapon_name.find(aMe.c_str()) != std::string::npos) weaponType = "ME"; // Melee

        // Name
        char nameChar[5];
        snprintf(nameChar, sizeof nameChar, "%s", cPlayer.name.c_str());
        ImGui::GetWindowDrawList()->AddText(ImVec2(finalDrawPos.x + 3, finalDrawPos.y - 17),
                                            ImColor(1.0f, 1.0f, 1.0f, 1.0f), nameChar); // draw name

        // Weapon on everyone:
        char wepChar[5];
        snprintf(wepChar, sizeof wepChar, "%s", weaponType.c_str());
        ImGui::GetWindowDrawList()->AddText(ImVec2(finalDrawPos.x + 7, finalDrawPos.y - 7),
                                            ImColor(1.0f, 1.0f, 1.0f, 1.0f), wepChar); // draw weapon type


        /* Mini Radar ingame */
        float gameMiniMapScale = g_MaxPlayers > 30 ? 0.0248f : 0.1268f; // warzone / mp
        float xMiniRadar = centerMiniMap.x + (diffLength * gameMiniMapScale) * cos(
            relativeRotationRadar - atan2(distY, distX));
        float yMiniRadar = centerMiniMap.y + (diffLength * gameMiniMapScale) * sin(
            relativeRotationRadar - atan2(distY, distX));
        Vector2 triangleTipMini = RotatePoint(Vector2(xMiniRadar + 14, yMiniRadar), Vector2(xMiniRadar, yMiniRadar),
                                              enemy_pointing);
        Vector2 triangleLeftMini = RotatePoint(Vector2(xMiniRadar - 3, yMiniRadar + 8),
                                               Vector2(xMiniRadar, yMiniRadar), enemy_pointing);
        Vector2 triangleRightMini = RotatePoint(Vector2(xMiniRadar - 3, yMiniRadar - 8),
                                                Vector2(xMiniRadar, yMiniRadar), enemy_pointing);
        RadarTriangle miniPlr = RadarTriangle();
        miniPlr.p1 = triangleTipMini;
        miniPlr.p2 = triangleLeftMini;
        miniPlr.p3 = triangleRightMini;
        miniPlr.color = Vector3(255, 87, 87); //orange
        char miniMapTextBuffer[8];
        snprintf(miniMapTextBuffer, sizeof miniMapTextBuffer, "%d", cPlayer.team);
        miniPlr.textTeam = miniMapTextBuffer; // over
        //if (cPlayer.team == g_lPlayer.team) miniPlr.color = Vector3(46, 116, 255); //teamCol
        if (cPlayer.team == g_lPlayer.team) miniPlr = RadarTriangle(); //teamCol
        if (xMiniRadar > 35 + 270 || yMiniRadar > 22 + 270 || i == g_lPlayer.entIndex)
            g_Radar_DrawList[i] =
                RadarTriangle();
        else
        {
            // Draw player on minimap
            g_Radar_DrawList[i] = miniPlr;
            overlayDrawList->AddTriangle(ImVec2(miniPlr.p1.x, miniPlr.p1.y), ImVec2(miniPlr.p2.x, miniPlr.p2.y),
                                         ImVec2(miniPlr.p3.x, miniPlr.p3.y),
                                         miniPlrOrange);
        }

        /* ESP */
        //adjustments (i.e. stance)
        Vector3 headPosition = cPlayer.pos + Vector3(0, 0, 5); // 5 is head height when standing
        Vector3 localHeadPosition = g_lPlayer.pos + Vector3(0, 0, 5); // 5 is head height when standing
        float diffLengthWZ = cPlayer.distanceToLocal();
        Vector2 boxSize = Vector2(25080 / diffLengthWZ, 45160 / diffLengthWZ);
        //kWidth & kHeight -> size of box, when standing

        pos.z += 6;
        switch (cPlayer.stance)
        {
        case CharacterStance::Crouching:
            pos.z -= 30.0f;
            headPosition.z -= 15.0f;
            boxSize.y *= 0.70f; // smaller box when crouching
            break;
        case CharacterStance::Crawling:
        case CharacterStance::Downed:
            pos.z -= 50.0f;
            headPosition.z -= 45.0f;
            boxSize.y *= 0.4f; // even smaller box when downed/crawling
            break;
        default:
            pos.z -= 25.0f; // 25 units lower is ~middle of the body when standing. Used for drawing.
            break;
        }

        switch (g_lPlayer.stance)
        {
        case CharacterStance::Crouching:
            localHeadPosition.z -= 15.0f;
            break;
        case CharacterStance::Crawling:
        case CharacterStance::Downed:
            localHeadPosition.z -= 45.0f;
            break;
        default: break;
        }

        Vector2 screenPosition = worldToScreen(
            pos,
            localHeadPosition
        );

        // Calculate FOV ratio for both x and y
        float fov_ratio_x = 1.570f / g_Refdef.view.tanHalfFov.x; // min/max - tested in game with 15x scope
        float fov_ratio_y = 0.883f / g_Refdef.view.tanHalfFov.y;
        boxSize.x *= fov_ratio_x;
        boxSize.y *= fov_ratio_y;
        boxSize.x *= guiScaleFactor;
        boxSize.y *= guiScaleFactor;
        bool skipESPPlayer =
            abs(diffLength) > g_dontDrawCap || // dont draw players > drawCap
            !isOnScreen(screenPosition, boxSize)
            || cPlayer.team == g_lPlayer.team;
        if (skipESPPlayer)
        {
            g_ESP_DrawList[i] = ESPBox();
            continue;
        }

        // Convert yaw and pitch to radians and adjust pitch
        float radYaw = cPlayer.yaw * (M_PI / 180.0);
        float adjustedPitch = (cPlayer.pitch <= 170) ? -cPlayer.pitch : 360 - cPlayer.pitch;
        float radPitch = adjustedPitch * (M_PI / 180.0);

        // Calculate direction vector where z is up
        float xDir = cos(radYaw) * cos(radPitch);
        float yDir = sin(radYaw) * cos(radPitch);
        float zDir = sin(radPitch);

        // Calculate the point in the direction the enemy is looking
        auto LOOK_DISTANCE = 400;
        auto playerLooking = cPlayer.pos +
            Vector3(xDir * LOOK_DISTANCE, yDir * LOOK_DISTANCE, zDir * LOOK_DISTANCE);


        screenPosition.x -= (boxSize.x / 2);
        screenPosition.y -= (boxSize.y / 2);

        // Prediction of aim position for hitting head based on bullet velocity and enemy velocity
        //float CROSSBOW_VELOCITY = 110.0f; // m/s
        float CROSSBOW_VELOCITY = 80.0f; // m/s
        float XRK_SNIPER_VELOCITY = 860.0f; // m/s
        float timeToTarget = meterDist / (g_northOriented ? CROSSBOW_VELOCITY : XRK_SNIPER_VELOCITY);
        float verticalDisplacement = 0.5f * 9.81f * timeToTarget * timeToTarget;
        auto velocity = gameUnitsToMeters(cPlayer.velocity);
        headPosition = headPosition + Vector3(velocity.x * timeToTarget, velocity.y * timeToTarget,
                                              velocity.z * timeToTarget);
        Vector3 aimPrediction = headPosition + Vector3(0, 0, verticalDisplacement);

        ESPBox espBox;
        espBox.prediction = worldToScreen(aimPrediction, localHeadPosition);
        espBox.screenPos = screenPosition;
        espBox.size = boxSize;
        espBox.textDistance = distChar;
        espBox.hp = cPlayer.health;
        espBox.playerLooking2d = worldToScreen(
            playerLooking,
            localHeadPosition
        );

        static char emptyString[] = "";
        char nameTeamChar[30];
        snprintf(nameTeamChar, sizeof(nameTeamChar), aSSF.c_str(), cPlayer.name.c_str(), weaponType.c_str(), getPlayerSkillScore(cPlayer).skillIndex);
        espBox.textName = (diffLength > 20000 || !g_showPlayerNames) ? emptyString : nameTeamChar; //don't draw names on ppl far away
        espBox.color = cPlayer.stance == CharacterStance::Downed ? purple : red;
        if (cPlayer.isVisible && cPlayer.stance != CharacterStance::Downed)
        {
            espBox.color = green; // green if visible and not downed
            if (cPlayer.localInCrosshairDirection > 0) espBox.color = darkGreen; // darker when aiming on lp
            localHasVisible = true;
        }
        else if (cPlayer.localInCrosshairDirection > 0) espBox.color = orange;
        // pink if reviving or self reviving
        if ((cPlayer.weapon_name.find(aReviveStim.c_str()) != std::string::npos) || cPlayer.isSelfReviving) espBox.color = pink;
        // blue if plating
        if (cPlayer.weapon_name.find(aPlateDeploy.c_str()) != std::string::npos) espBox.color = blue;

        if (cPlayer.shooting) espBox.lookingColor = red; 
        else if (cPlayer.aiming) espBox.lookingColor = green; 
        else espBox.lookingColor = cyan;

        g_ESP_DrawList[i] = espBox;
        overlayDrawList->AddRect(ImVec2(espBox.screenPos.x, espBox.screenPos.y),
                                 ImVec2(espBox.screenPos.x + espBox.size.x, espBox.screenPos.y + espBox.size.y),
                                 espBox.color);
        auto circleRadius = 2.75f * fov_ratio_y;
        circleRadius *= guiScaleFactor;
        overlayDrawList->AddCircle(ImVec2(espBox.prediction.x, espBox.prediction.y), 2.0f,
                                   blue);
        overlayDrawList->AddText(ImVec2(espBox.screenPos.x + 3, espBox.screenPos.y - 17),
                                 white, espBox.textName.c_str());
        overlayDrawList->AddText(ImVec2(espBox.screenPos.x + 3, espBox.screenPos.y + 3),
                                 white, espBox.textDistance.c_str());
        if (g_enablePlayerLookingDirection) overlayDrawList->AddLine(
            ImVec2(espBox.screenPos.x + (espBox.size.x / 2), espBox.screenPos.y + espBox.size.y / 4),
            ImVec2(espBox.playerLooking2d.x, espBox.playerLooking2d.y),
            espBox.lookingColor, 1.f);
        // Health bar
        float healthBarHeight = espBox.size.y * 0.1f; // Height of the health bar
        float healthBarPadding = espBox.size.y * 0.05f; // Space between the box and the health bar
        float healthBarWidth = boxSize.x; // Health bar width same as the box width

        // Health bar position: directly below the box with some padding
        ImVec2 healthBarPos(screenPosition.x, screenPosition.y + boxSize.y + healthBarPadding);
        float healthPercentage = static_cast<float>(min(127, cPlayer.health)) / 127.f; // 127 is the maximum health

        ImU32 fgColor;
        if (healthPercentage > 0.66f) fgColor = green;
        else if (healthPercentage > 0.33f) fgColor = yellow;
        else fgColor = red;
        // Draw health bar background
        overlayDrawList->AddRectFilled(healthBarPos,
                                       ImVec2(healthBarPos.x + healthBarWidth, healthBarPos.y + healthBarHeight),
                                       gray);
        // Draw health bar foreground based on health percentage
        overlayDrawList->AddRectFilled(healthBarPos,
                                       ImVec2(healthBarPos.x + healthBarWidth * healthPercentage,
                                              healthBarPos.y + healthBarHeight), fgColor);
    }
    g_hasVisibleTarget = localHasVisible;
    for (int i = 0; i < MAX_ITEM_COUNT; i++)
    {
        auto currItem = g_ItemList[i];
        Vector3 pos = currItem.pos;
        if (pos.isZero()) continue;
        if (!currItem.isOpenable && currItem.isContainer) continue;
        float distX = (pos.x - g_lPlayer.pos.x);
        float distY = (pos.y - g_lPlayer.pos.y);
        float distZ = (pos.z - g_lPlayer.pos.z);

        float diffLength = 0.0f;
        diffLength += distX * distX;
        diffLength += distY * distY;
        diffLength = (float)sqrt(diffLength);
        // Relative radar rotation
        relativeRotationRadar = -atan2(g_RefDefAxis.x, g_RefDefAxis.y) - M_PI / 2;
        float x = RADAR_CENTER.x + (diffLength * RADAR_SCALE) * cos(relativeRotationRadar - atan2(distY, distX));
        float y = RADAR_CENTER.y + (diffLength * RADAR_SCALE) * sin(relativeRotationRadar - atan2(distY, distX));

        ImVec2 locationCenterRotated = ImVec2(x, y);

        ImVec2 locationCenter = ImVec2(RADAR_CENTER.x + distX * RADAR_SCALE, RADAR_CENTER.y - distY * RADAR_SCALE);

        ImVec2 finalDrawPos = g_northOriented ? locationCenter : locationCenterRotated;
        ImGui::GetWindowDrawList()->AddCircle(finalDrawPos, 2.0f, teamCol);

        
        if (!g_ItemSearchKeywords.empty() && currItem.isSearched || g_ItemSearchKeywords.empty())
        {
            char nameChar[8];
            snprintf(nameChar, sizeof nameChar, "%s", currItem.friendlyName.c_str());
            ImGui::GetWindowDrawList()->AddText(ImVec2(finalDrawPos.x + 3, finalDrawPos.y - 17),
                                                ImColor(1.0f, 1.0f, 1.0f, 1.0f), nameChar); // draw name
        }
    }
    auto tmpBotsOnScreen = std::vector<Vector2>();
    for (int i = 0; i < MAX_ENTITY_COUNT && doEntityESP; i++)
    {
        auto currEnt = g_EntityArray[i];
        // only ET_AGENTs
        if (static_cast<int>(currEnt.C_ENTITY_STATE.eType) != 17 || !(currEnt.C_ENTITY_VALID & 1)  || currEnt.C_ENTITY_STATE.lerp.pos.trBase.isZero() || i==g_lPlayer.entIndex) continue;
        Vector3 pos = currEnt.C_ENTITY_STATE.lerp.pos.trBase;
        float distX = (pos.x - g_lPlayer.pos.x);
        float distY = (pos.y - g_lPlayer.pos.y);

        float diffLength = 0.0f;
        diffLength += distX * distX;
        diffLength += distY * distY;
        diffLength = (float)sqrt(diffLength);
        // Relative radar rotation
        relativeRotationRadar = -atan2(g_RefDefAxis.x, g_RefDefAxis.y) - M_PI / 2;
        float x = RADAR_CENTER.x + (diffLength * RADAR_SCALE) * cos(relativeRotationRadar - atan2(distY, distX));
        float y = RADAR_CENTER.y + (diffLength * RADAR_SCALE) * sin(relativeRotationRadar - atan2(distY, distX));

        ImVec2 locationCenterRotated = ImVec2(x, y);

        ImVec2 locationCenter = ImVec2(RADAR_CENTER.x + distX * RADAR_SCALE, RADAR_CENTER.y - distY * RADAR_SCALE);

        ImVec2 finalDrawPos = g_northOriented ? locationCenter : locationCenterRotated;
        ImGui::GetWindowDrawList()->AddCircle(finalDrawPos, 2.0f, orange);
        // draw type
        char typeChar[8];
        snprintf(typeChar, sizeof typeChar, "%d", currEnt.C_ENTITY_STATE.eType);
        ImGui::GetWindowDrawList()->AddText(ImVec2(finalDrawPos.x + 3, finalDrawPos.y - 17),
                                            ImColor(1.0f, 1.0f, 1.0f, 1.0f), typeChar); // draw name

        Vector3 localHeadPosition = g_lPlayer.pos + Vector3(0, 0, 5); // 5 is head height when standing
        float diffLengthWZ = pos.distance(g_lPlayer.pos);
        Vector2 boxSize = Vector2(25080 / diffLengthWZ, 45160 / diffLengthWZ);
        //kWidth & kHeight -> size of box, when standing

        pos.z -= 25.0f; // 25 units lower is ~middle of the body when standing. Used for drawing.

        switch (g_lPlayer.stance)
        {
        case CharacterStance::Crouching:
            localHeadPosition.z -= 15.0f;
            break;
        case CharacterStance::Crawling:
        case CharacterStance::Downed:
            localHeadPosition.z -= 45.0f;
            break;
        default: break;
        }

        Vector2 screenPosition = worldToScreen(
            pos,
            localHeadPosition
        );

        // Calculate FOV ratio for both x and y
        float fov_ratio_x = 1.570f / g_Refdef.view.tanHalfFov.x; // min/max - tested in game with 15x scope
        float fov_ratio_y = 0.883f / g_Refdef.view.tanHalfFov.y;
        boxSize.x *= fov_ratio_x;
        boxSize.y *= fov_ratio_y;
        bool skipESPPlayer =
            abs(diffLength) > g_dontDrawCap || // dont draw players > drawCap
            !isOnScreen(screenPosition, boxSize)
        ;
        if (skipESPPlayer) continue;

        screenPosition.x -= (boxSize.x / 2);
        screenPosition.y -= (boxSize.y / 2);
        // Draw zombie/AI
            Vector3 headPosition = pos + Vector3(0, 0, 5);
            tmpBotsOnScreen.push_back(worldToScreen(headPosition, localHeadPosition));
            overlayDrawList->AddCircle(ImVec2(headPosition.x + g_X, headPosition.y + g_Y), 3.0f, orange);
            overlayDrawList->AddRect(ImVec2(screenPosition.x, screenPosition.y),
                                     ImVec2(screenPosition.x + boxSize.x, screenPosition.y + boxSize.y),
                                     orange);
    }
    g_botsOnScreen = tmpBotsOnScreen;
}

void buildSettingsWindow()
{
    ImGui::SetNextWindowPos(ImVec2(settingsXLocation, 20), ImGuiCond_FirstUseEver);
    ImGui::Begin("Settings", &imGuiWindowOpen); // Settings window
    checkHoverWindows();
    ImGui::SliderInt(aRadarZoomLevel.c_str(), &g_zoomLevel, 0, 5);
    ImGui::SameLine(); HelpMarker(make_string("The zoom level on the external map window.").c_str());
    const char* elems_names[Range_COUNT] = { aClose.c_str(), aMedium.c_str(), aFar.c_str() };
    const char* elem_name = (g_RenderDistance >= 0 && g_RenderDistance < Range_COUNT) ? elems_names[g_RenderDistance] : "";
    ImGui::SliderInt(aEspRenderDistance.c_str(), &g_RenderDistance, 0, Range_COUNT - 1, elem_name);
    if (g_RenderDistance==Range_Close) g_dontDrawCap = 6000;
    if (g_RenderDistance==Range_Medium) g_dontDrawCap = 10000;
    if (g_RenderDistance==Range_Far) g_dontDrawCap = 40000;
    ImGui::Checkbox(aCrosshairActive.c_str(), &g_crosshair);

    bool aimbotBotMode = g_AimBotMode == 1;
    ImGui::SameLine();
    ImGui::Checkbox(aAfkBot.c_str(), &g_AFK);
    ImGui::SameLine();
    ImGui::PushItemWidth(40);
    ImGui::DragInt(aPlayAgainAdjustment.c_str(), &g_playAgainAdjustmentY, 1.0f, -100, 200);
    ImGui::SameLine();
    ImGui::Checkbox(aAimbotActive.c_str(), &aimbotBotMode);
    g_AimBotMode = aimbotBotMode ? 1 : 0;
    ImGui::SameLine();
    ImGui::Checkbox(aEnableAlert.c_str(), &g_EnableAimingAlert);
    ImGui::SameLine(); HelpMarker(aAlertHelp.c_str());
    ImGui::Checkbox(aDebugMode.c_str(), &g_Verbose);
    ImGui::SameLine();
    ImGui::Checkbox(aRadarNorthOriented.c_str(), &g_northOriented);
    ImGui::SameLine();
    ImGui::Checkbox(aItemESP.c_str(), &g_ItemESP);
    ImGui::SameLine();
    ImGui::Checkbox(aLinkTeams.c_str(), &g_DoTeamLink);
    ImGui::Checkbox(aPlayerLookDirection.c_str(), &g_enablePlayerLookingDirection);
    ImGui::SameLine(); HelpMarker(aLookDirectionHelp.c_str());
    ImGui::SameLine();
    ImGui::Checkbox(aDrawPlayerNames.c_str(), &g_showPlayerNames);
    ImGui::SameLine();
    ImGui::Checkbox(aEntityESP.c_str(), &doEntityESP);
    ImGui::SameLine(); ImGui::Checkbox(aRapidClick.c_str(), &g_RapidClick);
    if (ImGui::SliderFloat(aOpacity.c_str(), &g_guiOpacity, 0.1f, 1.0f))
    {
        red = IM_COL32(255, 0, 0, 255 * g_guiOpacity);
        neonYellow = IM_COL32(207, 255, 4, 255 * g_guiOpacity);
        gray = IM_COL32(128, 128, 128, 255 * g_guiOpacity);
        green = IM_COL32(0, 255, 0, 255 * g_guiOpacity);
        yellow = IM_COL32(255, 255, 0, 255 * g_guiOpacity);
        orange = IM_COL32(255, 165, 0, 255 * g_guiOpacity);
        blue = IM_COL32(0, 0, 255, 255 * g_guiOpacity);
        cyan = IM_COL32(0, 255, 255, 255 * g_guiOpacity);
        darkGreen = IM_COL32(0, 100, 0, 255 * g_guiOpacity);
        cyanCream = IM_COL32(0, 255, 200, 255 * g_guiOpacity);
        miniPlrOrange = IM_COL32(255, 87, 87, 255 * g_guiOpacity);
        pink = IM_COL32(255, 20, 147, 255 * g_guiOpacity);
        purple = IM_COL32(128, 0, 128, 255 * g_guiOpacity);
        white = IM_COL32(255, 255, 255, 255 * g_guiOpacity);
    }
    const char* keys_simulate[] = {
        keys_simulate_1.c_str(),
        keys_simulate_2.c_str(),
        keys_simulate_3.c_str(),
        keys_simulate_4.c_str()
    };
    // Render ImGui ListBox to let user select the mouse key to simulate
    if (ImGui::Combo(aSimulate.c_str(), &keyToSimulate, keys_simulate, IM_ARRAYSIZE(keys_simulate), 4))
    {
        // User made a selection, start the thread
        // Allocate and set the parameter for the thread
        int* pParam = new int(keyToSimulate);

        // Create the thread, passing it the selected action
        HANDLE hThread = CreateThread(
            nullptr, // default security attributes
            0,       // default stack size
            SimulateKeyPressThread,
            pParam,  // parameter to thread function
            0,       // default creation flags
            nullptr  // returns the thread identifier
        );

        if (hThread == NULL) {
            // Handle error
            std::cerr << make_string("Failed to create key simulation thread") << std::endl;
            delete pParam; // Clean up allocated memory
        } else
        {
            // Optionally close the thread handle if you don't need it
            CloseHandle(hThread);
        }
    }
    if (ImGui::Button(aSoftReset.c_str())) g_SoftRestart = true;
    if (!imGuiWindowOpen || wmCloseReq)
    {
        g_Active = false;
        PostQuitMessage(0);
    }
    ImGui::End();
}

void buildMetricsWindow()
{
    ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_FirstUseEver);
    ImGui::Begin("Metrics"); // Metrics window
    checkHoverWindows();

    ImGui::Text(make_string("Player name: '%s' Level: %d, SkillScore: %.0f, #%d").c_str(), g_detectedLocalName.c_str(), g_realLocalPlayer.score.rank_level, getPlayerSkillScore(g_realLocalPlayer).skillIndex, getPlayerSkillScore(g_realLocalPlayer).placement);
    ImGui::Text(make_string("Shot Accuracy: Dmg: %d, Shots: %d, Miss: %d, Hit: %d -- %.3f").c_str(), g_accumulatedDamageDone, g_accumulatedShotsFired, g_accumulatedShotsMissed, g_accumulatedShotsFired-g_accumulatedShotsMissed, static_cast<float>(g_accumulatedShotsFired-g_accumulatedShotsMissed) / static_cast<float>(g_accumulatedShotsFired));
    ImGui::Text(make_string("Game Open Time: %lld").c_str(), g_gameOpenTime / 1000);
    ImGui::Text(make_string("In Game Time MP/WZ: %lld/%lld").c_str(), g_inGameTimeMP / 1000, g_inGameTimeWZ / 1000);
    ImGui::Text(make_string("Games MP/WZ: %d/%d").c_str(), g_gameCountMP, g_gameCountWZ);
    ImGui::Text(make_string("Alive Time MP/WZ: %lld/%lld").c_str(), g_timeAliveMP / 1000, g_timeAliveWZ / 1000);
    ImGui::Text(make_string("Kills MP/WZ: %d/%d(%d)").c_str(), g_killRecordMP, g_killRecordWZ, g_realLocalPlayer.score.kills);
    ImGui::Text(make_string("Deaths MP/WZ: %d/%d(%d)").c_str(), g_deathRecordMP, g_deathRecordWZ, g_realLocalPlayer.score.deaths);
    ImGui::Text(make_string("Assist MP/WZ: %d/%d(%d)").c_str(), g_assistRecordMP, g_assistRecordWZ, g_realLocalPlayer.score.assists);
    ImGui::Text(make_string("Damage Done MP/WZ: %d/%d(%d)").c_str(), g_damageRecordMP, g_damageRecordWZ, g_MaxPlayers>30 ? g_realLocalPlayer.score.extraScore4 : g_realLocalPlayer.score.mp_damage_done);
    ImGui::Text(make_string("Damage Received MP/WZ: %d/%d(%d)").c_str(), g_damageTakenRecordMP, g_damageTakenRecordWZ, g_MaxPlayers>30 ? g_realLocalPlayer.score.extraScore3 : g_realLocalPlayer.score.mp_damage_taken); // todo not working?
    ImGui::Text(make_string("Points MP/WZ: %d/%d(%d)").c_str(), g_pointsRecordMP, g_pointsRecordWZ, g_realLocalPlayer.score.points);
    ImGui::Text(make_string("Aimbot ADS %: %.2f").c_str(), static_cast<float>(g_AimBotActiveInADSDurationMillis) / static_cast<float>(g_timeSpentAiming));
    ImGui::Text(make_string("Aimbot Fire %: %.2f").c_str(), static_cast<float>(g_AimBotActiveInShootingDurationMillis) / static_cast<float>(g_timeSpentShooting));
    ImGui::Text(make_string("Restarts: %d").c_str(), g_programRestarts);
    if (g_gameCountMP + g_gameCountWZ)
        ImGui::Text(make_string("Avg skill index: %.1f").c_str(), g_avgSkillIndex / (g_gameCountMP + g_gameCountWZ));
    ImGui::Text(make_string("DeathTimer: %d").c_str(), g_realLocalPlayer.score.deathTimer);
    ImGui::Text(make_string("High/low-score: Kills=%.0f/%.0f, Damage=%.0f/%.0f, Assists=%.0f/%.0f, Points=%.0f/%.0f").c_str(), g_HighScore.kills, g_LowScore.kills, g_HighScore.damageDone, g_LowScore.damageDone, g_HighScore.assists, g_LowScore.assists, g_HighScore.points, g_LowScore.points);
    ImGui::End();
}

void buildDebugWindow()
{
    ImGui::Begin("Info");
    debugWindowPos = ImGui::GetWindowPos();
    debugWindowSize = ImGui::GetWindowSize();
    checkHoverWindows();
    ImGui::Text(aGameAddress.c_str());
    ImGui::SameLine();
    if (g_Base) ImGui::Text("0x%llX, running=%d", g_Base, g_gameRunning);
    else ImGui::Text("%c", "|/-\\"[(int)(ImGui::GetTime() / 0.05f) & 3]);
    ImGui::SameLine();
    if (g_decryptCgTPtr) ImGui::Text(aCgTAddress.c_str(), g_decryptCgTPtr);
    if (g_decryptBasePtr)
    {
        ImGui::SameLine();
        ImGui::Text(aCharacterInfoAddress.c_str(), g_decryptBasePtr);
        ImGui::SameLine();
        ImGui::Text(aCEntityAddress.c_str(), g_CEntityBase);
    }

    ImGui::Text(aFPS.c_str(), ImGui::GetIO().Framerate);

    if (ImGui::InputText(aItemSearch.c_str(), itemSearchBuffer, IM_ARRAYSIZE(itemSearchBuffer)))
        parseSearchInput(std::string(itemSearchBuffer));

    ImGui::Text(aErrorMessage.c_str(), g_errMsg.c_str());
    ImGui::SameLine();
    if (ImGui::Button(aClear.c_str())) g_errMsg = "";
    ImGui::SameLine();
    ImGui::Combo(make_string("Pick bool").c_str(), &g_boolPick,"a1\0a2\0a3\0a4\0a5\0a6\0a7");
    if (g_AFK)
    {
        ImGui::SameLine();
        ImGui::Text(aAfkState.c_str(), g_AFKState.c_str());
    }
    if (ImGui::Button(make_string("Open Local Player Debug Window").c_str())) g_playerArr[g_lPlayer.entIndex].debug = true;
    ImGui::Text(aRefDebugInfo.c_str(), g_Refdef.view.axis->x,
                g_Refdef.view.axis->y, g_Refdef.view.tanHalfFov.x, g_Refdef.view.tanHalfFov.y, g_Refdef.width,
                g_Refdef.height, g_X, g_Y);
    displayCopyableAddressInHex(make_string("refdef_ptr "), g_refDefPtr);
    ImGui::Text(aValidPlayersDebugInfo.c_str(), g_validPlayers, g_MaxPlayers, g_hasVisibleTarget, g_AimingAlertAngles.size(), g_botsOnScreen.size());
    static int latestSelectedPlayer = -1;  // Default to no selection
    if (ImGui::BeginCombo(make_string("Select Player").c_str(), latestSelectedPlayer >= 0 ? g_playerArr[latestSelectedPlayer].name.c_str() : "None")) {
        for (int i = 0; i < MAX_PLAYER_COUNT; i++) {
            auto labelText = "#" + std::to_string(i) + " " + g_playerArr[i].name;
            if (ImGui::Selectable(labelText.c_str(), g_playerArr[i].debug)) {
                latestSelectedPlayer = i;
                g_playerArr[i].debug = true;  // Mark the player's window as 'open'
            }
            // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
            if (g_playerArr[i].debug)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    //if(g_closestIndex != -1) ImGui::Text("AB: %d: (%.0f, %.0f) A: %.0f -> %s", g_closestIndex, g_ESP_DrawList[g_closestIndex].pos.x, g_ESP_DrawList[g_closestIndex].pos.y, g_ESP_DrawList[g_closestIndex].pos.distance(Vector2(g_WarfareWidth/2, g_WarfareHeight/2)), g_playerArr[g_closestIndex].name); // Debug aimbot
    ImGui::PushItemWidth(70);
    ImGui::InputText(aInputValueSearch.c_str(), inputBuffer, IM_ARRAYSIZE(inputBuffer), ImGuiInputTextFlags_EnterReturnsTrue);
    ImGui::PopItemWidth();
    if (ImGui::Button(aSearchInteger.c_str()))
    {
        g_SearchFloatVal = 0.0f;
        // g_SearchIntVal = (int)atoi(inputBuffer);
        g_SearchIntVal = strtoull(inputBuffer, NULL, 16);
    }
    ImGui::SameLine();
    if (ImGui::Button(aSearchFloat.c_str()))
    {
        g_SearchIntVal = 0;
        g_SearchFloatVal = (float)atof(inputBuffer);
        g_SearchFloatVal = trunc(g_SearchFloatVal * 100.0) / 100.0;
    }
    ImGui::SameLine();
    if (ImGui::Button(aQueOffset.c_str()))
    {
        g_readAddressInput = std::stoi(inputBuffer, nullptr, 16);
    }
    ImGui::SameLine();
    if (ImGui::Button(aSetGW.c_str()))
    {
        g_W = std::stoi(inputBuffer, nullptr, 10);
    }
    ImGui::SameLine();
    ImGui::Text(aF12Scan.c_str(), g_SearchIntVal, g_SearchFloatVal);
    ImGui::Text(aInputOutput.c_str(), g_readAddressInput, g_readAddressVal);
    const char* memorySourceOptions[] = {
        memorySourceOptions_0.c_str(),
        memorySourceOptions_1.c_str(),
        memorySourceOptions_2.c_str(),
        memorySourceOptions_3.c_str()
    };

    // Dropdown for selecting the memory source
    ImGui::Combo(make_string("Memory Source").c_str(), &selectedMemorySource, memorySourceOptions, IM_ARRAYSIZE(memorySourceOptions));

    // Additional fields for the custom memory source
    if (selectedMemorySource == CUSTOM) {
        const char* addressOptions[] = {
            addressOptions_0.c_str(),
            addressOptions_1.c_str(),
            addressOptions_2.c_str(),
            addressOptions_3.c_str(),
            addressOptions_4.c_str(),
            addressOptions_5.c_str()
        };
        constexpr uint64_t step = 1; // Increment by 1 on arrow key press
        constexpr uint64_t stepFast = 16; // Increment by 16 on arrow key press with Shift
        ImGui::InputScalar(make_string("Address").c_str(), ImGuiDataType_U64, &customAddress, &step, &stepFast, make_string("%llX").c_str(), ImGuiInputTextFlags_CharsHexadecimal);
        ImGui::InputScalar(make_string("Size").c_str(), ImGuiDataType_U64, &customSize, &step, NULL, make_string("%llu").c_str(), 0);
        ImGui::Combo(make_string("Size Unit").c_str(), &sizeUnit, "Bytes\0KB\0MB\0");
        ImGui::Combo(make_string("Relative to").c_str(), &selectedAddressOption, addressOptions, IM_ARRAYSIZE(addressOptions));
    }

    const bool isDumpToFileClicked = ImGui::Button(make_string("Dump to File").c_str());
    ImGui::SameLine();
    const bool isViewMemoryClicked = ImGui::Button(make_string("View Memory").c_str());
    ImGui::SameLine();
    if (ImGui::Button(make_string("Close Memory").c_str())) showMemoryEditor = false;
    HandleMemoryOperations(isDumpToFileClicked, isViewMemoryClicked);
    if (showMemoryEditor) {
                void* data = nullptr;
                size_t size = 0;
                std::string title;
                uintptr_t baseAddress = 0;

                // Determine the parameters for DrawWindow based on memoryViewSource.
                switch (memoryViewSource) {
                case RAW_PLAYER_ARRAY:
                    data = g_CharacterInfoRaw;
                    size = sizeof(CharacterInfo) * MAX_PLAYER_COUNT;
                    title = make_string("Memory View - Raw Players");
                    baseAddress = g_decryptBasePtr;
                    break;
                case NAME_ENTRIES:
                    data = g_ClientInfoRaw;
                    size = sizeof(clientInfo_t) * MAX_PLAYER_COUNT;
                    title = make_string("Memory View - Name Entries");
                    baseAddress = g_ClientInfoArrPtr;
                    break;
                case C_ENTITY_ARRAY:
                    data = g_CEntityRaw;
                    size = sizeof(C_Entity) * MAX_ENTITY_COUNT;
                    title = make_string("Memory View - Raw Entities");
                    baseAddress = g_CEntityBase - (sizeof(C_Entity) * g_lPlayer.entIndex);
                    break;
                case CUSTOM:
                    data = memoryViewBuffer.data();
                    size = memoryViewBuffer.size();
                    title = make_string("Memory View - Custom");
                    baseAddress = memoryViewBaseAddress;
                    break;
                default:
                    // Handle any default case or possibly log an error/issue.
                        break;
                }

                // Draw the memory editor window if we have valid data to show.
                if (data != nullptr) {
                    mem_edit.DrawWindow(title.c_str(), data, size, baseAddress);
                    // Close the editor if the user has closed the window.
                }
            }
}

void buildAimbotSettings(AimbotSettings& settings, ImDrawList* draw_list)
{
    // Create a new ImGui window for the aimbot settings
    ImGui::Begin(make_string("Assistance Settings").c_str());
    checkHoverWindows();

    const ImVec2 center(static_cast<float>(g_WarfareWidth) / 2.0f, static_cast<float>(g_WarfareHeight) / 2.0f);

    ImGui::Checkbox(make_string("Show Aim Circle").c_str(), &settings.showDebugDrawing);
    ImGui::SameLine(); HelpMarker(make_string("Show debug circles for the aimbot settings.").c_str());
    ImGui::SameLine(); if (ImGui::Button(aResetAimbotSettings.c_str())) g_AimbotSettings = AimbotSettings();
    // Max Speed - Orange
    ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(1.0f, 0.647f, 0.0f, 0.50f)); // Bright Orange
    ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, ImVec4(1.0f, 0.549f, 0.0f, 1.0f)); // Darker Orange for Active
    ImGui::SliderFloat(make_string("Max Speed").c_str(), &settings.maxSpeed, 2.0f, 100.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
    ImGui::PopStyleColor(2);

    // Min Speed - Purple
    ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(0.502f, 0.0f, 0.502f, 0.50f)); // Bright Purple
    ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, ImVec4(0.294f, 0.0f, 0.510f, 1.0f));
    // Darker Purple for Active
    ImGui::SliderFloat(make_string("Min Speed").c_str(), &settings.minSpeed, 1.0f, 100.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
    ImGui::PopStyleColor(2);

    // Aim Circle Radius - Red
    ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(1.0f, 0.0f, 0.0f, 0.50f)); // Bright Red
    ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, ImVec4(0.8f, 0.0f, 0.0f, 1.0f)); // Darker Red for Active
    ImGui::SliderFloat(make_string("Aim Circle Radius").c_str(), &settings.aimCircleRadius, 2.0f, 1000.0f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
    ImGui::PopStyleColor(2);

    // Invisible Aim Circle Radius - Blue
    ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(0.0f, 0.0f, 1.0f, 0.50f)); // Bright Blue
    ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, ImVec4(0.0f, 0.0f, 0.8f, 1.0f)); // Darker Blue for Active
    ImGui::SliderFloat(make_string("Invisible Aim Circle Radius").c_str(), &settings.invisibleAimCircleRadius, 1.0f, 1000.0f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
    ImGui::PopStyleColor(2);

    // Dead Zone Threshold - Green
    ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(0.0f, 1.0f, 0.0f, 0.50f)); // Bright Green
    ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, ImVec4(0.0f, 0.8f, 0.0f, 1.0f)); // Darker Green for Active
    ImGui::SliderFloat(make_string("Dead Zone Threshold").c_str(), &settings.deadZoneThreshold, 1.0f, 100.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
    ImGui::PopStyleColor(2);

    // Distance Threshold
    ImGui::SliderFloat(make_string("Maximum Distance").c_str(), &settings.maximumDistance, 1.0f, 1000.0f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip(make_string("Adjusts the maximum distance for the players ESP boxes as well as for the aimbot to be considered as a target.\nSlide towards higher values to increase the distance threshold, allowing the aimbot to target players further away.").c_str());
    }

    // Weight3D
    ImGui::SliderFloat(make_string("World Distance Weight").c_str(), &settings.worldDistanceWeight, 0.0f, 1.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip(make_string("Adjusts the balance between 3D world distance and 2D screen distance from the crosshair in target selection.\nSlide towards higher values to prioritize targets closer in 3D world space, making the selection more depth-aware.\nLower values give more importance to how close targets appear to the crosshair, favoring their 2D screen proximity.").c_str());
    }

    if (settings.showDebugDrawing)
    {
        draw_list->AddCircle(center, settings.getAimCircleRadius(), IM_COL32(255, 0, 0, 255 * g_guiOpacity), 128);
        draw_list->AddCircle(center, settings.getInvisibleAimCircleRadius(), IM_COL32(0, 0, 255, 255 * g_guiOpacity), 128);
        draw_list->AddCircle(center, settings.getDeadZoneThreshold(), IM_COL32(0, 255, 0, 255 * g_guiOpacity), 64);
    }

    /*
    // In-game minimap radar
    draw_list->AddRect(ImVec2(minimapCenter.x - minimapSize, minimapCenter.y - minimapSize), ImVec2(minimapCenter.x + minimapSize, minimapCenter.y + minimapSize), IM_COL32(255, 255, 255, 255 * Overlay::guiOpacity), 0.0f, 0, 1.0f);
    draw_list->AddCircleFilled(minimapCenter, 3, IM_COL32(255, 255, 255, 255 * Overlay::guiOpacity), 8);
    */

    // Calculate end positions for vertical lines based on speeds
    const auto maxSpeedEndPos = ImVec2(center.x, center.y - settings.getMaxSpeed()); // Max Speed
    const auto minSpeedEndPos = ImVec2(center.x, center.y - settings.getMinSpeed()); // Min Speed

    // Draw line for Max Speed
    if (settings.showDebugDrawing) draw_list->AddLine(center, maxSpeedEndPos, IM_COL32(255, 165, 0, 255 * g_guiOpacity), 2.0f);
    // Max Speed in Red

    // Draw line for Min Speed
    if (settings.showDebugDrawing) draw_list->AddLine(center, minSpeedEndPos, IM_COL32(128, 0, 128, 255 * g_guiOpacity), 2.0f);
    // Min Speed in Blue

    // Optionally, adjust label positions for vertical orientation and to avoid overlap
    if (settings.showDebugDrawing) draw_list->AddText(ImVec2(maxSpeedEndPos.x + 5, maxSpeedEndPos.y - 15), IM_COL32(255, 140, 0, 255 * g_guiOpacity), make_string("Max Speed").c_str());
    if (settings.showDebugDrawing) draw_list->AddText(ImVec2(minSpeedEndPos.x - 80, minSpeedEndPos.y - 15), IM_COL32(75, 0, 130, 255 * g_guiOpacity), make_string("Min Speed").c_str());

    // End the ImGui window for the aimbot settings
    ImGui::End();
}

void HandleMemoryOperations(bool isDumpToFileClicked, bool isViewMemoryClicked) {
    if ((isDumpToFileClicked || (isViewMemoryClicked)) && (g_decryptCgTPtr || selectedMemorySource == CUSTOM)) {
        auto params = new DumpParams();
        bool createThread = false;
        uintptr_t finalAddress = customAddress;
        DWORD (WINAPI *threadFunction)(LPVOID) = nullptr;

        switch (selectedMemorySource) {
        case RAW_PLAYER_ARRAY:
            *params = {make_string("raw_players_dump"), g_CharacterInfoRaw, sizeof(CharacterInfo) * MAX_PLAYER_COUNT, false};
            break;
        case NAME_ENTRIES:
            *params = {make_string("name_entries_dump"), g_ClientInfoRaw, sizeof(clientInfo_t) * MAX_PLAYER_COUNT, false};
            break;
        case C_ENTITY_ARRAY:
            *params = {make_string("c_entity_dump"), g_CEntityRaw, sizeof(C_Entity) * MAX_ENTITY_COUNT, false};
            break;
        case CUSTOM:
            size_t finalSize = customSize;
            if (finalSize == 0) finalSize = 1024; // Default to 1 KB if size is 0
            if (sizeUnit == 1) finalSize *= 1024; // KB to bytes
            else if (sizeUnit == 2) finalSize *= 1024 * 1024; // MB to bytes

            switch (selectedAddressOption) {
            case 1: // "Module Base Address"
                finalAddress += g_Base;
                break;
            case 2: // "Peb Address"
                finalAddress += g_Peb;
                break;
            case 3: // "Cg_t base Address"
                finalAddress += g_decryptCgTPtr;
                break;
            case 4: // "CharacterInfo Base Address"
                finalAddress += g_decryptBasePtr;
                break;
            case 5: // "CEntity Address"
                finalAddress += g_CEntityBase;
                break;
            default: // "Absolute Address"
                break;
            }
            *params = {make_string("custom_dump"), reinterpret_cast<void*>(finalAddress), finalSize, true};
            break;
        }

        if (isDumpToFileClicked)
        {
            createThread = true;
            threadFunction = DumpMemoryToFileThread;
        }

        if (isViewMemoryClicked)
        {
            showMemoryEditor = true;
            memoryViewSource = selectedMemorySource;
            if (selectedMemorySource == CUSTOM) {
                memoryViewBaseAddress = finalAddress;
                createThread = true;
                threadFunction = MemoryViewThread;
            }
        }

        if (createThread) {
            HANDLE hThread = CreateThread(NULL, 0, threadFunction, params, 0, NULL);
            if (hThread) {
                CloseHandle(hThread);
            }
            else {
                std::cout << make_string("Failed to create thread") << std::endl;
                delete params;
            }
        } else {
            delete params; // Cleanup if no thread is created
        }
    }
}

void drawDottedLine(ImDrawList* drawList, ImVec2 p1, ImVec2 p2, ImU32 color, float thickness, float whiteSpaceLength)
{
    float length = sqrt((p2.x - p1.x) * (p2.x - p1.x) + (p2.y - p1.y) * (p2.y - p1.y));
    ImVec2 dir = ImVec2((p2.x - p1.x) / length, (p2.y - p1.y) / length);
    for (float i = 0.0f; i < length; i += whiteSpaceLength)
    {
        drawList->AddLine(ImVec2(p1.x + dir.x * i, p1.y + dir.y * i),
                          ImVec2(p1.x + dir.x * (i + 1.0f), p1.y + dir.y * (i + 1.0f)), color, thickness);
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam))
    {
        return 0;
    }

    switch (uMsg)
    {
    case WM_CLOSE:
        // Instead of destroying the window immediately, we flag that the confirmation dialog should be shown.
        wmCloseReq = true;
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void checkHoverWindows()
{
    POINT mousePos;
    GetCursorPos(&mousePos);
    auto windowPos = ImGui::GetWindowPos();
    auto windowSize = ImGui::GetWindowSize();
    const bool isMouseOverWindow = (mousePos.x >= windowPos.x) && (mousePos.x <= windowPos.x +
            windowSize.x) &&
        (mousePos.y >= windowPos.y) && (mousePos.y <= windowPos.y + windowSize.y);

    if (isMouseOverWindow)
        SetWindowLong(gui_hwnd, GWL_EXSTYLE, GetWindowLong(gui_hwnd, GWL_EXSTYLE) & ~WS_EX_TRANSPARENT);
}

ImGuiContext* initImGui(HWND hwnd, int screenWidth, int screenHeight, WNDCLASSEX wc)
{
    SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), BYTE(255), LWA_ALPHA);

    MARGINS margins = {-1}; // Special value to make the whole window client area a glass sheet
    DwmExtendFrameIntoClientArea(hwnd, &margins);
    // Initialize Direct3D
    if (!CreateDeviceD3D(hwnd, screenWidth, screenHeight))
    {
        CleanupDeviceD3D();
        UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return nullptr;
    }

    // Show the window
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    // Setup Dear ImGui context
    //IMGUI_CHECKVERSION();
    auto guiContext = ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;

    // Setup Dear ImGui style
    //ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();
    ImGuiEx::SetupImGuiStyle(nullptr, true, 0.9f);

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(pd3dDevice, pd3dDeviceContext);
    // Use the array and its length to load the font
    float fontSize = 16.0f * stylingFactor;
    ImFont* font = io.Fonts->AddFontFromMemoryCompressedTTF(roboto_regular_ttf_compressed_data, roboto_regular_ttf_compressed_size, fontSize, nullptr, io.Fonts->GetGlyphRangesDefault());
    if (font == nullptr) {
        std::cout << make_string("Failed to load font") << std::endl;
        CleanupDeviceD3D();
        UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return nullptr;
    }
    // Adjust style scaling
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(stylingFactor);
    return guiContext;
}

bool CreateDeviceD3D(HWND hWnd, int width, int height)
{
    // Setup swap chain
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = width;
    sd.BufferDesc.Height = height;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    //createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[4] = {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0
    };
    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags,
                                                featureLevelArray, 4, D3D11_SDK_VERSION, &sd, &pSwapChain,
                                                &pd3dDevice, &featureLevel, &pd3dDeviceContext);
    if (res == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software driver if hardware is not available.
        res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags,
                                            featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &pSwapChain,
                                            &pd3dDevice,
                                            &featureLevel, &pd3dDeviceContext);
    if (res != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (pSwapChain)
    {
        pSwapChain->Release();
        pSwapChain = nullptr;
    }
    if (pd3dDeviceContext)
    {
        pd3dDeviceContext->Release();
        pd3dDeviceContext = nullptr;
    }
    if (pd3dDevice)
    {
        pd3dDevice->Release();
        pd3dDevice = nullptr;
    }
}

void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget()
{
    if (mainRenderTargetView)
    {
        mainRenderTargetView->Release();
        mainRenderTargetView = nullptr;
    }
}

void cleanup(HWND hwnd, WNDCLASSEX wc)
{
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    CleanupDeviceD3D();
    DestroyWindow(hwnd);
    UnregisterClassW(wc.lpszClassName, wc.hInstance);
}


float updateRadarScale()
{
    auto result = 0.01f;
    switch (g_zoomLevel)
    {
    case 0:
        result = 0.001f;
        break;
    case 1:
        result = 0.01f;
        break;
    case 2:
        result = 0.05f;
        break;
    case 3:
        result = 0.1f;
        break;
    case 4:
        result = 0.2f;
        break;
    case 5:
        result = 0.45f;
        break;
    }
    return result;
}

void parseSearchInput(const std::string& input)
{
    std::vector<std::string> keywords;
    std::istringstream stream(input);
    std::string keyword;
    while (std::getline(stream, keyword, ','))
    {
        if (!keyword.empty())
        {
            keywords.push_back(keyword);
        }
    }
    g_ItemSearchKeywords = keywords;
}

DWORD WINAPI MemoryViewThread(LPVOID lpParam) {
    DumpParams* params = static_cast<DumpParams*>(lpParam);
    if (params) {
        if (params->copyNeeded) {
            memoryViewBuffer.resize(params->size);

            while (g_Active && showMemoryEditor) {
                    if (!DMA::read_memory(reinterpret_cast<UINT_PTR>(params->data), reinterpret_cast<UINT_PTR>(memoryViewBuffer.data()), params->size)) {
                        std::cout << make_string("Failed to read memory for memory view\n");
                        Sleep(500); // Shorter wait after a failed read
                    } else {
                        Sleep(20); // Brief pause after a successful read
                    }
            }
        }
        delete params; // Clean up allocated memory
    }
    return 0;
}

DWORD WINAPI DumpMemoryToFileThread(LPVOID lpParam) {
    DumpParams* params = static_cast<DumpParams*>(lpParam);
    if (params) {
        if (params->copyNeeded) {
            std::vector<UINT8> dump_buffer(params->size);

            if (DMA::read_memory(reinterpret_cast<UINT_PTR>(params->data), reinterpret_cast<UINT_PTR>(dump_buffer.data()), params->size)) {
                DumpMemoryToFile(params->fileName, dump_buffer.data(), params->size);
            }
            else {
                std::cerr << make_string("Failed to read memory for dumping\n");
            }
        } else {
            // No need to copy, dump directly
            DumpMemoryToFile(params->fileName, params->data, params->size);
        }
        delete params; // Clean up allocated memory
    }
    return 0;
}

DWORD WINAPI SimulateKeyPressThread(LPVOID lpParam)
{
    int action = *reinterpret_cast<int*>(lpParam);
    delete reinterpret_cast<int*>(lpParam); // Clean up the allocated memory

    // std::cout << make_string("Simulating key press: ") << action << std::endl;

    // Wait for 3 seconds
    Sleep(3000);

    // Depending on the passed int, simulate different mouse actions
    switch (action)
    {
    case 0: // Scroll Tilt Right
        mouse_event(MOUSEEVENTF_HWHEEL, 0, 0, 120, 0); // Simulate scroll tilt right
        break;
    case 1: // Scroll Tilt Left
        mouse_event(MOUSEEVENTF_HWHEEL, 0, 0, -120, 0); // Simulate scroll tilt left
        break;
    case 2: // X1
        mouse_event(MOUSEEVENTF_XDOWN, 0, 0, XBUTTON1, 0); // Simulate pressing the X1 button
        Sleep(85);
        mouse_event(MOUSEEVENTF_XUP, 0, 0, XBUTTON1, 0); // Simulate releasing the X1 button
        break;
    case 3: // X2
        mouse_event(MOUSEEVENTF_XDOWN, 0, 0, XBUTTON2, 0); // Simulate pressing the X2 button
        Sleep(90);
        mouse_event(MOUSEEVENTF_XUP, 0, 0, XBUTTON2, 0); // Simulate releasing the X2 button
        break;
    }

    return 0;
}

void HelpMarker(const char* desc)
{
    ImGui::TextDisabled(make_string("(?)").c_str());
    if (ImGui::BeginItemTooltip())
    {
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData) {
    MONITORINFOEX mi;
    mi.cbSize = sizeof(MONITORINFOEX);
    GetMonitorInfo(hMonitor, &mi);
    if (mi.rcMonitor.left < monitorRect.left)
        monitorRect.left = mi.rcMonitor.left;
    if (mi.rcMonitor.top < monitorRect.top)
        monitorRect.top = mi.rcMonitor.top;
    if (mi.rcMonitor.right > monitorRect.right)
        monitorRect.right = mi.rcMonitor.right;
    if (mi.rcMonitor.bottom > monitorRect.bottom)
        monitorRect.bottom = mi.rcMonitor.bottom;

    return TRUE;
}

void DisplayPlayerDebugInfo(Player& player) {
    std::string window_label = "#" + std::to_string(player.entIndex) + " Information";
    POINT mousePos;
    GetCursorPos(&mousePos);
    ImGui::SetNextWindowPos(ImVec2(mousePos.x,mousePos.y), ImGuiCond_Once);
    if (ImGui::Begin(window_label.c_str(), &g_playerArr[player.entIndex].debug, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse)) {
        checkHoverWindows();
        if (ImGui::TreeNode(make_string("Player").c_str()))
        {
            displayCopyableAddressInHex(make_string("CharacterInfo + "), player.entIndex*sizeof(CharacterInfo));
            ImGui::Text(make_string("Name: %s").c_str(), player.name.c_str());
            ImGui::Text(make_string("Team: %d").c_str(), player.team);
            ImGui::Text(make_string("Index: %d").c_str(), player.entIndex);
            ImGui::Text(make_string("Stance: %s").c_str(), StanceToString(player.stance));
            ImGui::Text(make_string("Dead: %s").c_str(), player.isDead ? "Yes" : "No");
            ImGui::Text(make_string("Valid: %s").c_str(), player.isValid ? "Yes" : "No");
            ImGui::Text(make_string("Health: %d").c_str(), player.health);
            ImGui::Text(make_string("Position: (%.2f, %.2f, %.2f)").c_str(), player.pos.x, player.pos.y, player.pos.z);
            ImGui::Text(make_string("Last velocity pos: (%.2f, %.2f, %.2f)").c_str(), player.lastVelocitySamplePos.x, player.lastVelocitySamplePos.y, player.lastVelocitySamplePos.z);
            ImGui::Text(make_string("Velocity: (%.2f, %.2f, %.2f)").c_str(), player.velocity.x, player.velocity.y, player.velocity.z);
            ImGui::Text(make_string("Speed: %.2f").c_str(), player.speed);
            ImGui::Text(make_string("Idle Time: %.2f").c_str(), player.calcTimeIdle());
            ImGui::Text(make_string("Distance to Local(m): %.2f").c_str(), gameUnitsToMeters(player.distanceToLocal()));
            ImGui::Text(make_string("Yaw: %.2f, Pitch: %.2f").c_str(), player.yaw, player.pitch);
            ImGui::Text(make_string("Visible: %s").c_str(), player.isVisible ? "Yes" : "No");
            ImGui::Text(make_string("Aiming: %s").c_str(), player.aiming ? "Yes" : "No");
            ImGui::Text(make_string("Shooting: %s").c_str(), player.shooting ? "Yes" : "No");
            ImGui::Text(make_string("Weapon: #%d->%s | %d, %d").c_str(), player.weaponId, player.weapon_name.c_str(), player.weapon.weaponIdx, player.weapon.weaponLootId);
            displayCopyableAddressInHex(make_string("Base + "), Offsets::WEAPON_DEFINITIONS + (player.weaponId * 0x8));
            ImGui::Text(make_string("SkipPlayer(): %s").c_str(), player.skipPlayer() ? "Yes" : "No");
            ImGui::TreePop();
        }
        
        if (ImGui::TreeNode(make_string("Score_t").c_str())) {
            const auto offsetToScoreBoardFromCgT = Offsets::SCORE_OFS + (player.entIndex << 7);
            auto skillScore = getPlayerSkillScore(player);
            ImGui::Text(make_string("Calculated Skill Score: %.3f / 100 #%d").c_str(), skillScore.skillIndex, skillScore.placement);
            displayCopyableAddressInHex(make_string("Cg_t + "), offsetToScoreBoardFromCgT);
            ImGui::Text(make_string("Kills: %d").c_str(), player.score.kills);
            ImGui::Text(make_string("Deaths: %d").c_str(), player.score.deaths);
            ImGui::Text(make_string("Ping: %d").c_str(), player.score.ping);
            ImGui::Text(make_string("Score: %d").c_str(), player.score.points);
            ImGui::Text(make_string("Status: %d").c_str(), player.score.status);
            ImGui::Text(make_string("Team: %d").c_str(), player.score.team);
            ImGui::Text(make_string("IsBot: %s").c_str(), player.score.isBot ? "Yes" : "No");
            ImGui::Text(make_string("Assists: %d").c_str(), player.score.assists);
            ImGui::Text(make_string("MP Damage done: %d").c_str(), player.score.mp_damage_done);
            ImGui::Text(make_string("MP Damage taken: %d").c_str(), player.score.mp_damage_taken);
            ImGui::Text(make_string("Extra1 (dom_Captures/hp_time/wz_money?): %d").c_str(), player.score.extraScore1);
            ImGui::Text(make_string("Extra2 (kc_Confirms/hp_Offense/dom_Defense): %d").c_str(), player.score.extraScore2);
            ImGui::Text(make_string("Extra3 (kc_Denies/wz_damage_taken): %d").c_str(), player.score.extraScore3);
            ImGui::Text(make_string("Extra4 (wz_Damage_done): %d").c_str(), player.score.extraScore4);
            ImGui::Text(make_string("Extra5: %d").c_str(), player.score.extraScore5);
            ImGui::Text(make_string("Extra5 (hp_Defends): %d").c_str(), player.score.extraScore6);
            ImGui::Text(make_string("Other: %d").c_str(), player.score.other1);
            ImGui::Text(make_string("Level: %d").c_str(), player.score.rank_level);
            ImGui::Text(make_string("Death timer / in killcam: %d").c_str(), player.score.deathTimer);
            ImGui::TreePop();
        }

        if (ImGui::TreeNode(make_string("Entity Info").c_str())) {
            displayCopyableAddressInHex(make_string("Entity + "), player.c_entity.C_ENTITY_STATE.clientNum * sizeof(C_Entity));
            ImGui::Text(make_string("Valid: %d").c_str(), player.c_entity.C_ENTITY_VALID & 1);
            ImGui::Text(make_string("Type: %d").c_str(), player.c_entity.C_ENTITY_STATE.eType);
            std::bitset<64> bits(player.c_entity.C_ENTITY_STATE.lerp.eFlags);
            ImGui::Text(make_string("Flags: %s").c_str(), bits.to_string().c_str());
            ImGui::Text(make_string("Flags dead: %d, alive: %d, crouch: %d, prone %d").c_str(), player.c_entity.C_ENTITY_STATE.lerp.eFlags & 1 << 18, player.c_entity.C_ENTITY_STATE.lerp.eFlags & 1 << 17, player.c_entity.C_ENTITY_STATE.lerp.eFlags & 1 << 3, player.c_entity.C_ENTITY_STATE.lerp.eFlags & 1 << 4);
            ImGui::Text(make_string("Origin: (%.2f, %.2f, %.2f)").c_str(), player.c_entity.C_ENTITY_STATE.lerp.pos.trBase.x, player.c_entity.C_ENTITY_STATE.lerp.pos.trBase.y, player.c_entity.C_ENTITY_STATE.lerp.pos.trBase.z);
            ImGui::Text(make_string("DecOrigin: (%.2f, %.2f, %.2f)").c_str(), player.decPos.x, player.decPos.y, player.decPos.z);
            ImGui::Text(make_string("Yaw: %.2f Pitch %.2f Z: %.2f").c_str(), player.c_entity.C_ENTITY_STATE.lerp.apos.trBase.x, player.c_entity.C_ENTITY_STATE.lerp.apos.trBase.y, player.c_entity.C_ENTITY_STATE.lerp.apos.trBase.z);
            ImGui::Text(make_string("Speed: %.2f").c_str(), player.c_entity.C_ENTITY_STATE.lerp.pos.trDelta.x);
            ImGui::Text(make_string("ClientNum: %d").c_str(), player.c_entity.C_ENTITY_STATE.clientNum);
            ImGui::TreePop();
        }

        if (ImGui::TreeNode(make_string("Metrics").c_str())) {
            ImGui::Text(make_string("Last HP Zero: %lld").c_str(), getms() - player.lastHpZeroTs);
            ImGui::Text(make_string("In Crosshair Of Local Last: %lld").c_str(), getms() - player.inCrosshairOfLocalLastTs);
            ImGui::Text(make_string("Last Visible: %lld").c_str(), getms() - player.lastVisibleTs);
            ImGui::Text(make_string("Downed: %lld").c_str(), getms() - player.downedTs);
            ImGui::Text(make_string("Last Valid Read: %lld").c_str(), getms() - player.lastUpdateTime);
            ImGui::TreePop();
        }

        ImGui::End();
    }
}
