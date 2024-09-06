#include "GuiEx.h"

#include <sstream>

# define M_PI           3.14159265358979323846
#define STB_IMAGE_IMPLEMENTATION

MonitorRects monitors;

int windowWidth = 670;
int windowHeight = 800;

bool showUI = true;
char inputBuffer[36] = {0};

const int RADAR_RADIUS = 250;
ImVec2 RADAR_CENTER;
float RADAR_SCALE;

int RES_VAL = 474;

bool showMemView = false;

const char* gameOptions[] = {"wz", "mul", "reb"};
const char* searchOptions[] = {"in", "flt"};

char itemSearchBuffer[256] = "orange,urani";

void renderUI();

// Data
static int const NUM_FRAMES_IN_FLIGHT = 3;
static FrameContext g_frameContext[NUM_FRAMES_IN_FLIGHT] = {};
static UINT g_frameIndex = 0;

static int const NUM_BACK_BUFFERS = 3;
static ID3D12Device* g_pd3dDevice = NULL;
static ID3D12DescriptorHeap* g_pd3dRtvDescHeap = NULL;
static ID3D12DescriptorHeap* g_pd3dSrvDescHeap = NULL;
static ID3D12CommandQueue* g_pd3dCommandQueue = NULL;
static ID3D12GraphicsCommandList* g_pd3dCommandList = NULL;
static ID3D12Fence* g_fence = NULL;
static HANDLE g_fenceEvent = NULL;
static UINT64 g_fenceLastSignaledValue = 0;
static IDXGISwapChain3* g_pSwapChain = NULL;
static HANDLE g_hSwapChainWaitableObject = NULL;
static ID3D12Resource* g_mainRenderTargetResource[NUM_BACK_BUFFERS] = {};
static D3D12_CPU_DESCRIPTOR_HANDLE g_mainRenderTargetDescriptor[NUM_BACK_BUFFERS] = {};

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
void WaitForLastSubmittedFrame();
FrameContext* WaitForNextFrameResources();
void ResizeSwapChain(HWND hWnd, int width, int height);
static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

HWND hwnd;
WNDCLASSEX wc;

// Global state
static bool active = true;

// Main code
int mainWindow()
{
    // Create application window
    //ImGui_ImplWin32_EnableDpiAwareness();
    wc = {
        sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, _T("Assist"),
        NULL
    };
    ::RegisterClassEx(&wc);
    int windowXLocation = 1950;
    if (GetSystemMetrics(SM_CMONITORS) == 1)
        windowXLocation = 500;
    hwnd = ::CreateWindow(wc.lpszClassName, _T("Assist"), WS_OVERLAPPEDWINDOW, windowXLocation, 10, windowWidth,
                          windowHeight, NULL, NULL, wc.hInstance, NULL);
    //EnumDisplayMonitors(NULL, NULL);

    // Initialize Direct3D
    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClass(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    // Show the window
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    ImFontConfig config;
    config.SizePixels = 30;
    ImFont* bigFont = io.Fonts->AddFontDefault(&config);
    ImFont* normalFont = io.Fonts->AddFontDefault();
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX12_Init(g_pd3dDevice, NUM_FRAMES_IN_FLIGHT,
                        DXGI_FORMAT_R8G8B8A8_UNORM, g_pd3dSrvDescHeap,
                        g_pd3dSrvDescHeap->GetCPUDescriptorHandleForHeapStart(),
                        g_pd3dSrvDescHeap->GetGPUDescriptorHandleForHeapStart());

    // Main loop
    MSG msg;
    ZeroMemory(&msg, sizeof(msg));
    while (msg.message != WM_QUIT)
    {
        switch (g_zoomLevel)
        {
        case -1:
            RADAR_SCALE = 0.001f;
            break;
        case 0:
            RADAR_SCALE = 0.01f;
            break;
        case 1:
            RADAR_SCALE = 0.05f;
            break;
        case 2:
            RADAR_SCALE = 0.1f;
            break;
        case 3:
            RADAR_SCALE = 0.2f;
            break;
        case 4:
            RADAR_SCALE = 0.45f;
            break;
        }
        RADAR_CENTER = ImVec2(static_cast<float>(windowWidth) / 2.0f, static_cast<float>(windowHeight) / 2.0f + 100);

        // Poll and handle messages (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        if (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            continue;
        }

        // Start the Dear ImGui frame
        ImGui_ImplDX12_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);


        ImGui::SetNextWindowSize(
            ImVec2(static_cast<float>(windowWidth) + 10.0f, static_cast<float>(windowHeight) + 10.0f),
            ImGuiCond_Always);
        ImGui::Begin(".", &active, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
        ImGui::PushFont(normalFont);

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
        if (g_gameMode == 0)
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

        ImGui::PushFont(bigFont);
        ImGui::GetWindowDrawList()->AddText(ImVec2(RADAR_CENTER.x - 17, RADAR_CENTER.y - RADAR_RADIUS - 25), greenCol,
                                            degreeChar);
        ImGui::PopFont();

        float relativeRotationRadar;
        bool localHasVisible = false;
        //Draw POS <3
        for (int i = 0; i < MAX_PLAYER_COUNT; i++)
        {
            Player cPlayer = g_playerArr[i];
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
                    cPlayer.calcTimeIdle() > 10 || // still standing/looking for > 15 secs
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

            ImVec2 finalDrawTip = ImVec2(finalDrawPos.x + 20 * -cos(enemy_pointing * M_PI / 180),
                                         finalDrawPos.y + 20 * -sin(enemy_pointing * M_PI / 180));
            /*ImVec2 finalDrawLeft = ImVec2(finalDrawPos.x + 10 * -cos(enemy_pointing * M_PI/180), finalDrawPos.y + 10 * sin(enemy_pointing * M_PI / 180));
            ImVec2 finalDrawRight = ImVec2(finalDrawPos.x + 10 * -cos(enemy_pointing * M_PI/180), finalDrawPos.y + 10 * sin(enemy_pointing * M_PI / 180));*/

            //ImGui::GetWindowDrawList()->AddLine(finalDrawPos, finalDrawTip, greenCol, 2.5f);


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
                // pos circle
                ImGui::GetWindowDrawList()->AddTriangleFilled(ImVec2(triangleLeft.x, triangleLeft.y),
                                                              ImVec2(triangleRight.x, triangleRight.y),
                                                              ImVec2(triangleTip.x, triangleTip.y), teamCol);
                /*ImGui::GetWindowDrawList()->AddCircleFilled(
                    finalDrawPos,
                    6.0f,
                    teamCol
                );*/
            }
            else
            {
                // enemy
                ImColor lineGrad;
                ImColor enemyEvalCol;
                if (cPlayer.stance == CharacterStance::Downed) enemyEvalCol = pinkCol; // pink if downed
                else if (cPlayer.weapon_name.find("revive_stim") != std::string::npos) enemyEvalCol = ImColor(13, 255, 0); // green if reviving
                else if (cPlayer.weapon_name.find("plate_deploy") != std::string::npos) enemyEvalCol = ImColor(0, 13, 255); // blue if plating
                else if (cPlayer.shooting) enemyEvalCol = orangeCol; // orange if shooting
                else if (cPlayer.aiming) enemyEvalCol = darkOrangeCol; // dark orange if aimed
                else enemyEvalCol = enemyCol;
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
            if (cPlayer.weapon_name.find("_ar_") != std::string::npos) weaponType = "AR"; // Assault Rifles
            if (cPlayer.weapon_name.find("_lm_") != std::string::npos) weaponType = "LM"; // Light Machine Guns
            if (cPlayer.weapon_name.find("_pi_") != std::string::npos) weaponType = "P"; //Pistols
            if (cPlayer.weapon_name.find("_sm_") != std::string::npos) weaponType = "SM"; //Submachinguns
            if (cPlayer.weapon_name.find("_sh_") != std::string::npos) weaponType = "SG"; //Shotguns
            if (cPlayer.weapon_name.find("_sn_") != std::string::npos) weaponType = "SR"; // Sniper Rifles
            if (cPlayer.weapon_name.find("_me_") != std::string::npos) weaponType = "ME"; // melee

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
            //float gameRadarScale = 0.000208333;
            float gameRadarScale = 0.02 + 0.0002 * 24; // minimap scale estimate
            float centerX = 169; // Estimated Values based on 1920 x 1080 resolution Pixels
            float centerY = 155;
            float xMiniRadar = centerX + (diffLength * gameRadarScale) * cos(
                relativeRotationRadar - atan2(distY, distX));
            float yMiniRadar = centerY + (diffLength * gameRadarScale) * sin(
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
            else g_Radar_DrawList[i] = miniPlr;

            /* ESP */
            //adjustments (i.e. stance)
            Vector3 headPosition = cPlayer.pos + Vector3(0, 0, 5); // 5 is head height when standing
            Vector3 localHeadPosition = g_lPlayer.pos + Vector3(0, 0, 5); // 5 is head height when standing
            float diffLengthWZ = cPlayer.distanceToLocal();
            Vector2 boxSize = Vector2(g_X / diffLengthWZ, g_Y / diffLengthWZ); //kWidth & kHeight -> size of box, when standing
            
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

            // todo use camera
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
                screenPosition.x < -boxSize.x || screenPosition.y < -boxSize.y || (screenPosition.x > g_Refdef.width+boxSize.x) || (
                    screenPosition.y > g_Refdef.height+boxSize.y) // skip, out of cam view
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

            // Prediction of aim position for hitting head based on bullet velocity
            float CROSSBOW_VELOCITY = 110.0f;
            float XRK_SNIPER_VELOCITY = 860.0f;
            float timeToTarget = meterDist / (g_northOriented ? CROSSBOW_VELOCITY : XRK_SNIPER_VELOCITY);
            float verticalDisplacement = 0.5f * 9.81f * timeToTarget * timeToTarget;
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
            snprintf(nameTeamChar, sizeof(nameTeamChar), "%s[%d]%s", cPlayer.name.c_str(), cPlayer.team,
                     weaponType.c_str());
            espBox.textName = diffLength > 20000 ? emptyString : nameTeamChar; //don't draw names on ppl far away
            espBox.color = cPlayer.stance == CharacterStance::Downed ? Vector3(160, 10, 190) : Vector3(255, 0, 0);
            if (cPlayer.isVisible)
            {
                espBox.color = Vector3(13, 255, 0); // green if visible
                if (cPlayer.localInCrosshairDirection > 0) espBox.color = Vector3(0, 100, 0); // darker when aiming on lp
                localHasVisible = true;
            } else if (cPlayer.localInCrosshairDirection > 0) espBox.color = Vector3(254, 100, 0); // orange
            if (cPlayer.weapon_name.find("revive_stim") != std::string::npos) espBox.color = Vector3(252, 0, 215); // pink if reviving
            if (cPlayer.weapon_name.find("plate_deploy") != std::string::npos) espBox.color = Vector3(0, 13, 255); // blue if plating

            if (cPlayer.shooting) espBox.lookingColor = Vector3(255, 0, 0); // red
            else if (cPlayer.aiming) espBox.lookingColor = Vector3(13, 255, 0); // green
            else espBox.lookingColor = Vector3(0, 255, 255); // cyan

            g_ESP_DrawList[i] = espBox;
        }
        g_hasVisibleTarget = localHasVisible;
        for (int i = 0; i < MAX_ITEM_COUNT; i++)
        {
            Vector3 pos = g_ItemList[i].pos;
            if (pos.isZero()) continue;
            g_ItemList[i].screenPos = worldToScreen(pos, g_lPlayer.pos);
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

            // Item name and search
            // Check if item name matches search list
            for (const auto& keyword : g_ItemSearchKeywords)
            {
                if (g_ItemList[i].friendlyName.find(keyword) != std::string::npos)
                {
                    g_ItemList[i].isSearched = true;
                    break; // Found a keyword match, no need to check other keywords for this item
                }
            }
            if (!g_ItemSearchKeywords.empty() && g_ItemList[i].isSearched || g_ItemSearchKeywords.empty())
            {
                char nameChar[8];
                snprintf(nameChar, sizeof nameChar, "%s", g_ItemList[i].friendlyName.c_str());
                ImGui::GetWindowDrawList()->AddText(ImVec2(finalDrawPos.x + 3, finalDrawPos.y - 17),
                                                    ImColor(1.0f, 1.0f, 1.0f, 1.0f), nameChar); // draw name
            }
        }
        /*---   WINDOW LAYOUT   ---*/
        ImGui::Checkbox("UI", &showUI);
        if (showUI)
        {
            renderUI();
        }

        ImGui::End();


        //shit


        //shit

        //shit

        //shit


        // Rendering
        FrameContext* frameCtxt = WaitForNextFrameResources();
        UINT backBufferIdx = g_pSwapChain->GetCurrentBackBufferIndex();
        frameCtxt->CommandAllocator->Reset();

        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = g_mainRenderTargetResource[backBufferIdx];
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

        g_pd3dCommandList->Reset(frameCtxt->CommandAllocator, NULL);
        g_pd3dCommandList->ResourceBarrier(1, &barrier);
        g_pd3dCommandList->ClearRenderTargetView(g_mainRenderTargetDescriptor[backBufferIdx], (float*)&clear_color, 0,
                                                 NULL);
        g_pd3dCommandList->OMSetRenderTargets(1, &g_mainRenderTargetDescriptor[backBufferIdx], FALSE, NULL);
        g_pd3dCommandList->SetDescriptorHeaps(1, &g_pd3dSrvDescHeap);
        ImGui::Render();
        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), g_pd3dCommandList);
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
        g_pd3dCommandList->ResourceBarrier(1, &barrier);
        g_pd3dCommandList->Close();

        g_pd3dCommandQueue->ExecuteCommandLists(1, (ID3D12CommandList* const*)&g_pd3dCommandList);

        g_pSwapChain->Present(1, 0); // Present with vsync
        //g_pSwapChain->Present(0, 0); // Present without vsync

        UINT64 fenceValue = g_fenceLastSignaledValue + 1;
        g_pd3dCommandQueue->Signal(g_fence, fenceValue);
        g_fenceLastSignaledValue = fenceValue;
        frameCtxt->FenceValue = fenceValue;
    }
    return 0;
}

void quitGUI()
{
    printf("quitGUI");
    WaitForLastSubmittedFrame();
    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClass(wc.lpszClassName, wc.hInstance);
}

// Helper functions

bool CreateDeviceD3D(HWND hWnd)
{
    // Setup swap chain
    DXGI_SWAP_CHAIN_DESC1 sd;
    {
        ZeroMemory(&sd, sizeof(sd));
        sd.BufferCount = NUM_BACK_BUFFERS;
        sd.Width = 0;
        sd.Height = 0;
        sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
        sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        sd.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
        sd.Scaling = DXGI_SCALING_STRETCH;
        sd.Stereo = FALSE;
    }

    // [DEBUG] Enable debug interface
#ifdef DX12_ENABLE_DEBUG_LAYER
    ID3D12Debug* pdx12Debug = NULL;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&pdx12Debug))))
        pdx12Debug->EnableDebugLayer();
#endif

    // Create device
    D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
    if (D3D12CreateDevice(NULL, featureLevel, IID_PPV_ARGS(&g_pd3dDevice)) != S_OK)
        return false;

    // [DEBUG] Setup debug interface to break on any warnings/errors
#ifdef DX12_ENABLE_DEBUG_LAYER
    if (pdx12Debug != NULL)
    {
        ID3D12InfoQueue* pInfoQueue = NULL;
        g_pd3dDevice->QueryInterface(IID_PPV_ARGS(&pInfoQueue));
        pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
        pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
        pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
        pInfoQueue->Release();
        pdx12Debug->Release();
    }
#endif

    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        desc.NumDescriptors = NUM_BACK_BUFFERS;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        desc.NodeMask = 1;
        if (g_pd3dDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&g_pd3dRtvDescHeap)) != S_OK)
            return false;

        SIZE_T rtvDescriptorSize = g_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = g_pd3dRtvDescHeap->GetCPUDescriptorHandleForHeapStart();
        for (UINT i = 0; i < NUM_BACK_BUFFERS; i++)
        {
            g_mainRenderTargetDescriptor[i] = rtvHandle;
            rtvHandle.ptr += rtvDescriptorSize;
        }
    }

    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        desc.NumDescriptors = 2;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        if (g_pd3dDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&g_pd3dSrvDescHeap)) != S_OK)
            return false;
    }

    {
        D3D12_COMMAND_QUEUE_DESC desc = {};
        desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        desc.NodeMask = 1;
        if (g_pd3dDevice->CreateCommandQueue(&desc, IID_PPV_ARGS(&g_pd3dCommandQueue)) != S_OK)
            return false;
    }

    for (UINT i = 0; i < NUM_FRAMES_IN_FLIGHT; i++)
        if (g_pd3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
                                                 IID_PPV_ARGS(&g_frameContext[i].CommandAllocator)) != S_OK)
            return false;

    if (g_pd3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, g_frameContext[0].CommandAllocator, NULL,
                                        IID_PPV_ARGS(&g_pd3dCommandList)) != S_OK ||
        g_pd3dCommandList->Close() != S_OK)
        return false;

    if (g_pd3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&g_fence)) != S_OK)
        return false;

    g_fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (g_fenceEvent == NULL)
        return false;

    {
        IDXGIFactory4* dxgiFactory = NULL;
        IDXGISwapChain1* swapChain1 = NULL;
        if (CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory)) != S_OK ||
            dxgiFactory->CreateSwapChainForHwnd(g_pd3dCommandQueue, hWnd, &sd, NULL, NULL, &swapChain1) != S_OK ||
            swapChain1->QueryInterface(IID_PPV_ARGS(&g_pSwapChain)) != S_OK)
            return false;
        swapChain1->Release();
        dxgiFactory->Release();
        g_pSwapChain->SetMaximumFrameLatency(NUM_BACK_BUFFERS);
        g_hSwapChainWaitableObject = g_pSwapChain->GetFrameLatencyWaitableObject();
    }

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain)
    {
        g_pSwapChain->Release();
        g_pSwapChain = NULL;
    }
    if (g_hSwapChainWaitableObject != NULL) { CloseHandle(g_hSwapChainWaitableObject); }
    for (UINT i = 0; i < NUM_FRAMES_IN_FLIGHT; i++)
        if (g_frameContext[i].CommandAllocator)
        {
            g_frameContext[i].CommandAllocator->Release();
            g_frameContext[i].CommandAllocator = NULL;
        }
    if (g_pd3dCommandQueue)
    {
        g_pd3dCommandQueue->Release();
        g_pd3dCommandQueue = NULL;
    }
    if (g_pd3dCommandList)
    {
        g_pd3dCommandList->Release();
        g_pd3dCommandList = NULL;
    }
    if (g_pd3dRtvDescHeap)
    {
        g_pd3dRtvDescHeap->Release();
        g_pd3dRtvDescHeap = NULL;
    }
    if (g_pd3dSrvDescHeap)
    {
        g_pd3dSrvDescHeap->Release();
        g_pd3dSrvDescHeap = NULL;
    }
    if (g_fence)
    {
        g_fence->Release();
        g_fence = NULL;
    }
    if (g_fenceEvent)
    {
        CloseHandle(g_fenceEvent);
        g_fenceEvent = NULL;
    }
    if (g_pd3dDevice)
    {
        g_pd3dDevice->Release();
        g_pd3dDevice = NULL;
    }

#ifdef DX12_ENABLE_DEBUG_LAYER
    IDXGIDebug1* pDebug = NULL;
    if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&pDebug))))
    {
        pDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_SUMMARY);
        pDebug->Release();
    }
#endif
}

void CreateRenderTarget()
{
    for (UINT i = 0; i < NUM_BACK_BUFFERS; i++)
    {
        ID3D12Resource* pBackBuffer = NULL;
        g_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&pBackBuffer));
        g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, g_mainRenderTargetDescriptor[i]);
        g_mainRenderTargetResource[i] = pBackBuffer;
    }
}

void CleanupRenderTarget()
{
    WaitForLastSubmittedFrame();

    for (UINT i = 0; i < NUM_BACK_BUFFERS; i++)
        if (g_mainRenderTargetResource[i])
        {
            g_mainRenderTargetResource[i]->Release();
            g_mainRenderTargetResource[i] = NULL;
        }
}

void WaitForLastSubmittedFrame()
{
    FrameContext* frameCtxt = &g_frameContext[g_frameIndex % NUM_FRAMES_IN_FLIGHT];

    UINT64 fenceValue = frameCtxt->FenceValue;
    if (fenceValue == 0)
        return; // No fence was signaled

    frameCtxt->FenceValue = 0;
    if (g_fence->GetCompletedValue() >= fenceValue)
        return;

    g_fence->SetEventOnCompletion(fenceValue, g_fenceEvent);
    WaitForSingleObject(g_fenceEvent, INFINITE);
}

FrameContext* WaitForNextFrameResources()
{
    UINT nextFrameIndex = g_frameIndex + 1;
    g_frameIndex = nextFrameIndex;

    HANDLE waitableObjects[] = {g_hSwapChainWaitableObject, NULL};
    DWORD numWaitableObjects = 1;

    FrameContext* frameCtxt = &g_frameContext[nextFrameIndex % NUM_FRAMES_IN_FLIGHT];
    UINT64 fenceValue = frameCtxt->FenceValue;
    if (fenceValue != 0) // means no fence was signaled
    {
        frameCtxt->FenceValue = 0;
        g_fence->SetEventOnCompletion(fenceValue, g_fenceEvent);
        waitableObjects[1] = g_fenceEvent;
        numWaitableObjects = 2;
    }

    WaitForMultipleObjects(numWaitableObjects, waitableObjects, TRUE, INFINITE);

    return frameCtxt;
}

void ResizeSwapChain(HWND hWnd, int width, int height)
{
    DXGI_SWAP_CHAIN_DESC1 sd;
    g_pSwapChain->GetDesc1(&sd);
    sd.Width = width;
    sd.Height = height;

    IDXGIFactory4* dxgiFactory = NULL;
    g_pSwapChain->GetParent(IID_PPV_ARGS(&dxgiFactory));

    g_pSwapChain->Release();
    CloseHandle(g_hSwapChainWaitableObject);

    IDXGISwapChain1* swapChain1 = NULL;
    dxgiFactory->CreateSwapChainForHwnd(g_pd3dCommandQueue, hWnd, &sd, NULL, NULL, &swapChain1);
    swapChain1->QueryInterface(IID_PPV_ARGS(&g_pSwapChain));
    swapChain1->Release();
    dxgiFactory->Release();

    g_pSwapChain->SetMaximumFrameLatency(NUM_BACK_BUFFERS);

    g_hSwapChainWaitableObject = g_pSwapChain->GetFrameLatencyWaitableObject();
    assert(g_hSwapChainWaitableObject != NULL);
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
        {
            windowWidth = LOWORD(lParam);
            windowHeight = HIWORD(lParam);
            WaitForLastSubmittedFrame();
            ImGui_ImplDX12_InvalidateDeviceObjects();
            CleanupRenderTarget();
            ResizeSwapChain(hWnd, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam));
            CreateRenderTarget();
            ImGui_ImplDX12_CreateDeviceObjects();
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        active = false;
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProc(hWnd, msg, wParam, lParam);
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

void renderUI()
{
    ImGui::Text("Spil:");
    ImGui::SameLine();
    if (g_Base) ImGui::Text("0x%llX", g_Base);
    else ImGui::Text("%c", "|/-\\"[(int)(ImGui::GetTime() / 0.05f) & 3]);
    ImGui::SameLine();
    if (g_decryptClientPtr) ImGui::Text("ClIfo: 0x%llX", g_decryptClientPtr);
    ImGui::SameLine();
    if (g_decryptCase) ImGui::Text("Noegle: %d", g_decryptCase);
    if (g_decryptBasePtr)
    {
        ImGui::SameLine();
        ImGui::Text("ClBa: 0x%llX", g_decryptBasePtr);
        ImGui::SameLine();
        ImGui::Text("CEnt: 0x%llX", g_CEntityBase);
    }

    if (g_gameHWND)
    {
        ImGui::SameLine();
        ImGui::Text(" H: 0x%I64x", g_gameHWND);
    }
    if (ImGui::Button("-") && g_zoomLevel > -1)
    {
        g_zoomLevel--;
    }
    ImGui::SameLine();
    ImGui::Text("Scale=%.3f", RADAR_SCALE);
    ImGui::SameLine();
    if (ImGui::Button("+") && (g_zoomLevel < 4))
        g_zoomLevel++;
    ImGui::SameLine();
    if (ImGui::Button("NormalRng")) g_dontDrawCap = 10000;
    ImGui::SameLine();
    if (ImGui::Button("Longrng[L]")) g_dontDrawCap = 40000;
    
    if (ImGui::InputText("ItemSearch", itemSearchBuffer, IM_ARRAYSIZE(itemSearchBuffer)))
        parseSearchInput(std::string(itemSearchBuffer));
    
    ImGui::Text("Besked: %s", g_errMsg.c_str());
    ImGui::SameLine();
    if (ImGui::Button("Ryd")) g_errMsg = "";
    if (g_AFK)
    {
        ImGui::SameLine();
        ImGui::Text("AFK: --> %s", g_AFKState.c_str());
    }
    ImGui::Text("Li: %d Hold.: %d sTid: %d Stnce: %d Death: %d Liv: %d XYZ(%.0f, %.0f, %.0f) Speed: %.0f We: %d-%s YW: (%.0f, %.0f)",
                g_lPlayer.entIndex, g_lPlayer.team, g_lPlayer.calcTimeIdle(), g_lPlayer.stance, g_lPlayer.isDead,
                g_lPlayer.health, g_lPlayer.pos.x, g_lPlayer.pos.y, g_lPlayer.pos.z, g_lPlayer.speed, g_lPlayer.weaponId, g_lPlayer.weapon_name.c_str(), g_lPlayer.yaw,
                g_lPlayer.pitch);
    ImGui::Text("ref: (%.2f, %.2f)fov(%.3f,%.3f)w:%d,h:%d G bug: %.0f,%.0f", g_Refdef.view.axis->x, g_Refdef.view.axis->y, g_Refdef.view.tanHalfFov.x, g_Refdef.view.tanHalfFov.y, g_Refdef.width, g_Refdef.height, g_X, g_Y);
    ImGui::Text("V-P's: %d hasVis %d alerts %lld", g_validPlayers, g_hasVisibleTarget, g_AimingAlertAngles.size());
    //if(g_closestIndex != -1) ImGui::Text("AB: %d: (%.0f, %.0f) A: %.0f -> %s", g_closestIndex, g_ESP_DrawList[g_closestIndex].pos.x, g_ESP_DrawList[g_closestIndex].pos.y, g_ESP_DrawList[g_closestIndex].pos.distance(Vector2(g_WarfareWidth/2, g_WarfareHeight/2)), g_playerArr[g_closestIndex].name); // Debug aimbot
    ImGui::PushItemWidth(70);
    ImGui::InputText("<input", inputBuffer, IM_ARRAYSIZE(inputBuffer), ImGuiInputTextFlags_EnterReturnsTrue);
    ImGui::PopItemWidth();
    ImGui::SameLine();
    if (ImGui::Button("int"))
    {
        g_SearchFloatVal = 0.0f;
        g_SearchIntVal = (int)atoi(inputBuffer);
    }
    ImGui::SameLine();
    if (ImGui::Button("float"))
    {
        g_SearchIntVal = 0;
        g_SearchFloatVal = (float)atof(inputBuffer);
        g_SearchFloatVal = trunc(g_SearchFloatVal * 100.0) / 100.0;
    }
    ImGui::SameLine();
    if (ImGui::Button("que 0x offset"))
    {
        g_readAddressInput = std::stoi(inputBuffer, nullptr, 16);
    }
    ImGui::SameLine();
    if (ImGui::Button("set g_W"))
    {
        g_W = std::stoi(inputBuffer, nullptr, 10);
    }
    ImGui::SameLine();
    ImGui::Text("F12scan: %d, %.4f", g_SearchIntVal, g_SearchFloatVal);
    ImGui::Checkbox("MemVw", &showMemView);
    if (showMemView)
    {
        g_SearchFloatVal = 0.1f;
        ImGui::Columns(3);
        ImGui::Text("Adr");
        ImGui::NextColumn();
        ImGui::Text("ValInt");
        ImGui::NextColumn();
        ImGui::Text("ValFlt");
        ImGui::NextColumn();
        for (unsigned offset = 0; offset < MAX_CLIENT_SIZE_EVER; offset++)
        {
            float fltValue;
            int intValue;
            memcpy(&fltValue, g_LPBuffer + offset, sizeof(float));
            memcpy(&intValue, g_LPBuffer + offset, sizeof(int));
            if (intValue == 0 && fltValue == 0.0f && (offset > 0x1000 && offset < 0x10000))
                continue;
            ImGui::Text("0x%X", offset); // address
            ImGui::NextColumn();
            ImGui::Text("%.3ff", fltValue);
            ImGui::NextColumn();
            ImGui::Text("%d", intValue);
            ImGui::NextColumn();
        }
    }
    else g_SearchFloatVal = 0.0f;

    ImGui::Text("0x%X -> %d", g_readAddressInput, g_readAddressVal);

    ImGui::Checkbox("Sigte[5]", &g_crosshair);
    ImGui::SameLine();
    if (ImGui::Checkbox("XR[6]", &g_overlayActive))
    {
        InvalidateRect(g_OverlayHWND, NULL, TRUE);
        UpdateWindow(g_OverlayHWND);
    }
    ImGui::SameLine();
    ImGui::Checkbox("AFK[F7]", &g_AFK);
    ImGui::SameLine();
    ImGui::PushItemWidth(40);
    ImGui::DragInt("P_Again", &g_playAgainAdjustmentY, 1.0f, -100, 200);
    ImGui::SameLine();
    ImGui::Checkbox("Enable Alert", &g_EnableAimingAlert);
    ImGui::SameLine();
    ImGui::Checkbox("Restart[F5]", &g_SoftRestart);
    ImGui::Checkbox("Info[F8]", &g_Verbose);
    ImGui::SameLine();
    ImGui::Checkbox("North[9]", &g_northOriented);
    ImGui::SameLine();
    ImGui::Checkbox("4k", &g_4kMonitor);
    ImGui::SameLine();
    ImGui::Checkbox("ItemXR", &g_ItemESP);
    ImGui::SameLine();
    ImGui::Checkbox("TeamLinks", &g_DoTeamLink);
    
    const char* keys_simulate[] = {
        "Scroll Tilt Left - Tactical", "Scroll Tilt Right - Jump", "X1 - Field Upgrade", "X2 - Forward"
    };
    int keyToSimulate = 0;

    // Render ImGui ListBox to let user select the mouse key to simulate
    if (ImGui::Combo("Simulate", &keyToSimulate, keys_simulate, IM_ARRAYSIZE(keys_simulate), 4))
    {
        // After selection, wait for 3 seconds before simulating the key press
        Sleep(3000); // Sleep is in milliseconds

        switch (keyToSimulate)
        {
        case 0: // Scroll Tilt Left
            mouse_event(MOUSEEVENTF_HWHEEL, 0, 0, 120, 0); // Simulate scroll tilt left
            break;
        case 1: // Scroll Tilt Right
            mouse_event(MOUSEEVENTF_HWHEEL, 0, 0, -120, 0); // Simulate scroll tilt right
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
    }
}
