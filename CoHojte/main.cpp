#include "main.h"

using namespace std;

uint64_t localAddressStart = 0;
bool opdaterPeb();
bool opdaterBase();
bool gameRunning();
static Vector2 center;

static DWORD WINAPI RapidClick(void* params)
{
    while (g_Active)
    {
        Sleep(10);
        while (GetKeyState(VK_XBUTTON1) & 0x8000 && g_RapidClick) // BAGERSTE razer knap X1 
        {
            mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
            Sleep(randomInRange(8, 25));
            mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
            int baseDelay = 1000 / 39; // 11-11.5 CPS
            int randomAdjustment = randomInRange(11, 36);
            int delay = baseDelay + randomAdjustment;
            Sleep(delay);
        }
    }
    return 0;
}

static DWORD WINAPI KeyMonitor(void* params)
{
    while (g_Active)
    {
        Sleep(1);
        if (GetKeyState(VK_UP) & 0x8000) // arrow up pressed
        {
            g_Y += 1.0f;
            Sleep(2);
        }
        if (GetKeyState(VK_DOWN) & 0x8000) // arrow down pressed
        {
            g_Y -= 1.0f;
            Sleep(2);
        }
        if (GetKeyState(VK_LEFT) & 0x8000) // arrow left pressed
        {
            g_X -= 1.0f;
            Sleep(2);
        }
        if (GetKeyState(VK_RIGHT) & 0x8000) // arrow right pressed
        {
            g_X += 1.0f;
            Sleep(2);
        }

        if (GetKeyState(0x4C) & 0x8000)
        {
            // L trykket
            g_RenderDistance = (g_RenderDistance + 1) % Range_COUNT;
            Sleep(250);
        }
        if (GetKeyState(VK_F4) & 0x8000)
        {
            //F4 trykket
            g_Active = false;
            break;
        }
        if (GetKeyState(VK_F5) & 0x8000)
        {
            g_SoftRestart = true;
            Sleep(2500);
        }
        // if (GetKeyState(VK_SPACE) & 0x8000 && !g_northOriented)
        // {
        //     //SPACE trykket - bunnyhop (slaa fra ved northoriented true)
        //     mouse_event(MOUSEEVENTF_HWHEEL, 0, 0, 1, 0); //tilt to right to jump out of plane
        //     Sleep(random1_6() * 9);
        // }
        if (GetKeyState(0x6D) & 0x8000)
        {
            //- numpad trykket
            if (g_zoomLevel > 0)
            {
                g_zoomLevel--;
                Sleep(150);
            }
        }
        if (GetKeyState(0x6B) & 0x8000)
        {
            //+ numpad trykket
            if (g_zoomLevel < 5)
            {
                g_zoomLevel++;
                Sleep(150);
            }
        }
        if (GetKeyState(0x39) & 0x8000)
        {
            //9 trykket
            g_northOriented = !g_northOriented;
            Sleep(250);
        }
        if (GetKeyState(VK_F8) & 0x8000)
        {
            //F8 trykket
            g_Verbose = !g_Verbose;
            Sleep(250);
        }
        if (GetKeyState(VK_F7) & 0x8000)
        {
            g_AFK = !g_AFK;
            Sleep(250);
        }
        if (GetKeyState(0x36) & 0x8000)
        {
            //6 trykket
            g_overlayActive = !g_overlayActive;
            //InvalidateRect(g_OverlayHWND, NULL, TRUE);
            //UpdateWindow(g_OverlayHWND);
            Sleep(250);
        }
        if (GetKeyState(0x35) & 0x8000) // 5
        {
            g_crosshair = !g_crosshair;
            Sleep(250);
        }
        if (GetKeyState(0x02) & 0x8000) // mus right (ADS)
        {
            g_ADS = true;
            Sleep(2);
        }
        else g_ADS = false;
        if (GetKeyState(VK_LMENU) & 0x8000) // L ALT
        {
            g_AimBotActive = true;
            Sleep(2);
        }
        else if (!g_AFK) g_AimBotActive = false;
        if (GetKeyState(VK_F12) & 0x8000)
        {
            Sleep(250);
            if (!localAddressStart)
            {
                printf(make_string("No LocalPlayer memory ot scan...\n").c_str());
                continue;
            }
            printf(make_string("Searching value: %.4f, Data type: %s\n").c_str(), g_SearchIntVal ? g_SearchIntVal : g_SearchFloatVal,
                   g_SearchFloatVal ? make_string("float") : make_string("integer"));
            if (g_SearchFloatVal != 0.0f)
            {
                searchBuffer<float>(g_LPBuffer, 1700000, g_SearchFloatVal, 5.0f);
            }
            else if (g_SearchIntVal)
            {
                searchBuffer<QWORD>(g_LPBuffer, 1700000, g_SearchIntVal);
            }
            //writeBufferToFile(g_LPBuffer, Offsets::SIZE, "./mem.bin"); // dump localplayer state to file
        }
    }
    return 0;
}

void DoActions()
{
    // Opdater spiller adresse(r)
    printf(make_string("Running...\n").c_str());
    SOCKET sock = g_Sock;

    int warningCount = 0;
    g_lPlayer.entIndex = -1;
    auto* characterArray = new CharacterInfo[MAX_PLAYER_COUNT];
    auto* scoreArray = new score_t[MAX_PLAYER_COUNT];
    
    constexpr int WEAPON_LIST_SIZE = 500;
    std::vector<std::string> weaponIdToName(WEAPON_LIST_SIZE, "");
    
    g_ClientInfoArrPtr = 0;
    long long lastLateDamageAdjustment = 0;

    while (g_Active)
    {
        center = Vector2(g_WarfareWidth / 2, g_WarfareHeight / 2);
        Sleep(g_Verbose ? 3000 : 2);
        if (g_SoftRestart)
        {
            //delete[] characterArray;
            return; // Go back out to Update addresses(PID etc.)
        }

        //if (!g_errMsg.empty() && getSec() % 15 == 0) g_errMsg = ""; // try reset errmesg every 15th sec
        if (!g_refDefPtr || !g_Refdef.height) g_refDefPtr = DecryptRefDef();
        g_Refdef = DMA::Read<refdef_t>(g_refDefPtr);
        g_localView = ImVec2(g_Refdef.view.axis->x, g_Refdef.view.axis->y);
        g_RefDefAxis = ImVec2(g_Refdef.view.axis[1].x, g_Refdef.view.axis[0].x);
        g_MaxPlayers = DMA::Read<int>(g_Base + Offsets::GAME_MODE);
        auto camera = DMA::Read<uint64_t>(g_Base + Offsets::CAMERA_BASE);
        if (camera)
        {
            g_CameraPos = DMA::Read<Vector3>(camera + Offsets::CAMERA_POS);
            if (g_Verbose) printf(
                make_string("Camera pos: (%.1f, %.1f, %.1f)\n").c_str(), g_CameraPos.x, g_CameraPos.y, g_CameraPos.z);
        }

        // Fill weapon list
        if (weaponIdToName[425].empty()) // 425 index that must be filled for the list to be OK
        {
            auto* weaponPtrs = new QWORD[WEAPON_LIST_SIZE];
            DMA::read_memory(g_Base + Offsets::WEAPON_DEFINITIONS, reinterpret_cast<uint64_t>(weaponPtrs),
                              sizeof(QWORD) * WEAPON_LIST_SIZE);

            int i;
            for (i = 0; i < WEAPON_LIST_SIZE; i++)
            {
                if (!weaponPtrs[i]) continue;
                uint64_t weaponNameAdr = DMA::Read<uint64_t>(weaponPtrs[i] + 0x8);
                if (!weaponNameAdr) continue;
                char buffer[50];
                DMA::read_memory(weaponNameAdr, uint64_t(&buffer), sizeof(buffer));
                buffer[sizeof(buffer) - 1] = '\0'; // null terminate
                weaponIdToName[i] = string(buffer);
            }
        }

        // Update name array
        if (!g_ClientInfoArrPtr) g_ClientInfoArrPtr = DMA::Read<uint64_t>(g_Base + Offsets::NAME_ARRAY_OFFSET);
        else if (g_Verbose) printf(make_string("NameArray -> 0x%llX, GameMode -> %d\n").c_str(), g_ClientInfoArrPtr, g_MaxPlayers);
        clientInfo_t nameArray[MAX_PLAYER_COUNT];
        DMA::read_memory(g_ClientInfoArrPtr + 0x2C80, uint64_t(&nameArray), sizeof(nameArray)); // NAME_LIST_OFFSET ( from MPScoreboard->ClientInfo_t base)
        if (string(nameArray[g_lPlayer.entIndex].m_szName).empty()) g_ClientInfoArrPtr = 0; // bad name ptr
        if (g_Verbose)
        {
            printf(make_string("Local player name: '%s'\n").c_str(), string(nameArray[g_lPlayer.entIndex].m_szName).c_str());
            if (g_lPlayer.weaponId < weaponIdToName.size()) printf(make_string("Local player weapon: '%s'\n").c_str(), weaponIdToName[g_lPlayer.weaponId].c_str());
        }

        g_decryptCgTPtr = DecryptCg_t();
        g_decryptBasePtr = DecryptCharacterInfoBase();
        g_CEntityBase = DecryptCEntity();
        if (g_decryptCgTPtr != 0)
        {
            uint64_t LocalInfo = DMA::Read<uint64_t>(g_decryptCgTPtr + Offsets::LOCAL_INDEX);
            g_lPlayer.entIndex = DMA::Read<int>(LocalInfo + 0x2f0); //0x2E4
            g_lPlayer.name = string(nameArray[g_lPlayer.entIndex].m_szName);
        }
        if (g_decryptBasePtr > 0x800000000000 || !g_decryptBasePtr || !g_decryptCgTPtr)
        {
            if (!gameRunning())
            {
                g_SoftRestart = true;
                break; // mw closed
            }
            Sleep(500);
            continue;
        }
        
        /* C_Entity without decryption */
        if (!g_CEntityBase && g_decryptCgTPtr/* || g_CEntityBase > 0x9999999999*/)
            g_CEntityBase = DecryptCEntity();
        if (g_Verbose) printf(make_string("Cg_t: \t\t0x%llX\nCB: \t\t0x%llX\nC_Entity: \t0x%llX\n").c_str(),
            g_decryptCgTPtr, g_decryptBasePtr, g_CEntityBase);
        g_visible_enemy_bits = DMA::Read<ClientBits>(g_decryptCgTPtr + Offsets::visible_enemy_bits);
        auto time = getms();

        // _-_-_-_-_ LOADING PLAYER DATA _-_-_-_-_
        DMA::read_memory(g_decryptBasePtr, reinterpret_cast<uint64_t>(characterArray), sizeof(CharacterInfo) * MAX_PLAYER_COUNT);
        DMA::read_memory(g_CEntityBase, reinterpret_cast<uint64_t>(g_EntityArray), sizeof(C_Entity) * MAX_ENTITY_COUNT);
        if(g_MaxPlayers) {
            DMA::read_memory(g_decryptCgTPtr + Offsets::SCORE_OFS, reinterpret_cast<uint64_t>(scoreArray), sizeof(score_t) * MAX_PLAYER_COUNT);
        }

        int validPlayersL = 0;

        auto localTeamToLinkPositions = std::vector<std::vector<Vector2>>(MAX_TEAM_COUNT, std::vector<Vector2>());
        auto playersAimingFromBack = std::vector<float>();

        for (int i = 0; i < MAX_PLAYER_COUNT; i++)
        {
            Player cPlayer = Player(i);

            cPlayer.c_entity = g_EntityArray[i];
            cPlayer.weaponId = characterArray[i].WEAPON_INDEX;
            cPlayer.team = characterArray[i].TEAM;
            cPlayer.isValid = characterArray[i].VALID;
            cPlayer.stance = characterArray[i].STANCE;
            cPlayer.yaw = characterArray[i].YAW;
            cPlayer.pitch = characterArray[i].PITCH;
            cPlayer.aiming = characterArray[i].AIMING;
            cPlayer.shooting = characterArray[i].SHOOTING;
            cPlayer.isDead = characterArray[i].DEAD1 == 1 /*|| characterArray[i].DEAD2 == 1 || cPlayer.c_entity.C_ENTITY_STATE.lerp.eFlags & (1 << 18)*/;

            cPlayer.entIndex = i;
            if (cPlayer.c_entity.C_ENTITY_VALID & 1 && cPlayer.c_entity.C_ENTITY_STATE.lerp.pos.trType == 4) {
                cPlayer.decPos = TrBase_decrypt(cPlayer.c_entity.C_ENTITY_STATE.lerp.pos);
                // cPlayer.pos = isValidPositionInWorld(decrypted) ? decrypted : DMA::Read<Vector3>(characterArray[i].POS_PTR + 0x48);
            }
            // Positioning
            if (cPlayer.c_entity.C_ENTITY_VALID & 1 && cPlayer.c_entity.C_ENTITY_STATE.lerp.pos.trType != 4 && isValidPositionInWorld(cPlayer.c_entity.C_ENTITY_STATE.lerp.pos.trBase)) // not encrypted position
                cPlayer.pos = cPlayer.c_entity.C_ENTITY_STATE.lerp.pos.trBase;
            else cPlayer.pos = DMA::Read<Vector3>(characterArray[i].POS_PTR + 0x48); // todo read struct
            // if (cPlayer.c_entity.C_ENTITY_STATE.lerp.pos.trBase != g_playerArr[i].c_entity.C_ENTITY_STATE.lerp.pos.trBase && cPlayer.pos == g_playerArr[i].pos) cPlayer.pos = Vector3(); // entity moved but client position is the same (dormant client)
            
            cPlayer.speed = DMA::Read<float>(characterArray[i].POS_PTR + 0x30);

            
            // Scoring
            auto tmp = scoreArray[cPlayer.c_entity.C_ENTITY_STATE.clientNum];
            if (tmp.isValid()) cPlayer.score = tmp;
            else cPlayer.score = g_playerArr[i].score;
            cPlayer.registerScores();

            cPlayer.health = nameArray[i].m_health;
            cPlayer.name = string(nameArray[i].m_szName);
            auto flags = (uint16_t)nameArray[i].m_extra_flags >> 8;
            cPlayer.isInGulag = (flags & 0x02) || (flags & 0x04);
            cPlayer.isSelfReviving = (flags & 0x20);

            cPlayer.isVisible = is_visible(g_visible_enemy_bits, i);
            if (cPlayer.weaponId < weaponIdToName.size())
                cPlayer.weapon_name = weaponIdToName[cPlayer.weaponId];
            else if (g_Verbose && cPlayer.isValid && cPlayer.health)  // might/should never happen
                printf(make_string("Weapon Definitions Index out of bounds: %d out of %lld").c_str(), cPlayer.weaponId, weaponIdToName.size());

            cPlayer.screenPos = worldToScreen(cPlayer.pos, g_lPlayer.pos);

            // High alert!
            cPlayer.localInCrosshairDirection = isEnemyLookingAtPlayerWithDirectionality(cPlayer.pos, cPlayer.yaw, cPlayer.pitch, g_lPlayer);
            // Metrics
            if (0 <= isEnemyLookingAtPlayerWithDirectionality(g_lPlayer.pos, g_lPlayer.yaw, g_lPlayer.pitch, cPlayer))
                cPlayer.inCrosshairOfLocalLastTs = getms();
            else cPlayer.inCrosshairOfLocalLastTs = g_playerArr[i].inCrosshairOfLocalLastTs;
            if (cPlayer.health == 0 && g_playerArr[i].health != 0) cPlayer.lastHpZeroTs = getms();
            else cPlayer.lastHpZeroTs = g_playerArr[i].lastHpZeroTs;
            if (cPlayer.stance == CharacterStance::Downed && g_playerArr[i].stance != CharacterStance::Downed)
                cPlayer.downedTs = getms();
            else cPlayer.downedTs = g_playerArr[i].downedTs;
            if (cPlayer.isVisible) cPlayer.lastVisibleTs = getms();
            else cPlayer.lastVisibleTs = g_playerArr[i].lastVisibleTs;
            if (cPlayer.shooting) cPlayer.lastShootingTs = getms();
            else cPlayer.lastShootingTs = g_playerArr[i].lastShootingTs;
            cPlayer.debug = g_playerArr[i].debug;
            cPlayer.lastUpdateTime = getms();
            // idle check
            if (cPlayer.isVisible) g_idlePlayersTime[i] = 0; // player is visible and is therefore not idle DUH!
            if ((g_playerArr[i].pos == cPlayer.pos && (int)g_playerArr[i].yaw == (int)cPlayer.yaw) &&
                g_idlePlayersTime[i] == 0)
            {
                // started standing still
                g_idlePlayersTime[i] = getSec();
            }
            else if (g_playerArr[i].pos != cPlayer.pos)
            {
                // moving!
                g_idlePlayersTime[i] = 0;
            }
            else if (g_playerArr[i].yaw != cPlayer.yaw)
            {
                // yaw'ing!
                g_idlePlayersTime[i] = 0;
            }

            if (cPlayer.entIndex == g_realLocalPlayer.entIndex)
            {
                auto prevPlayer = g_playerArr[i];
                bool skipSample =
                    cPlayer.score.status == 2 || // in killcam
                    cPlayer.score.ping == 0; // not playing
                if (!skipSample) {
                    cPlayer.shotsFired = DMA::Read<BYTE>(g_decryptCgTPtr + Offsets::SHOTS_FIRED);
                    auto damageDoneDiff = g_MaxPlayers > 30
                                          ? cPlayer.score.extraScore4 - prevPlayer.score.extraScore4
                                          : cPlayer.score.mp_damage_done - prevPlayer.score.mp_damage_done;
                    int shotsFiredDiff = cPlayer.shotsFired - prevPlayer.shotsFired;
                    if (shotsFiredDiff > 0) { // Only accumulate if there are new shots fired
                        if (cPlayer.shooting) {
                            g_accumulatedShotsFired += shotsFiredDiff;
                            if (damageDoneDiff == 0) {
                                g_accumulatedShotsMissed += shotsFiredDiff;
                            }
                        }
                    }
                    if (shotsFiredDiff == 0 && damageDoneDiff > 0 && getms() - cPlayer.lastShootingTs < 100 && getms() - lastLateDamageAdjustment > 100) { // long bullet travel time(just a fucked fix for br)
                        g_accumulatedShotsMissed--; // remove missed shot
                        lastLateDamageAdjustment = getms();
                        //cPlayer.lastShootingTs = 0; // dont let one shot count multiple times (yes we'll probably miss a few hits i know)
                    }
                    if (damageDoneDiff > 0) g_accumulatedDamageDone += damageDoneDiff;
                }
                
                g_realLocalPlayer = cPlayer;
            }
            if (g_playerArr[i].entIndex == g_lPlayer.entIndex) // if local player
            {
                //cPlayer.weapon = CoD_GetEntityWeapon(cPlayer.c_entity);
                g_PlayerLoopTime = getms() - g_playerArr[i].lastUpdateTime;
                //cPlayer.debug = g_lPlayer.debug; // keep debug view state
                g_lPlayer = cPlayer; // current is local player
                localAddressStart = g_decryptBasePtr + Offsets::SIZE * i;
                if (g_lPlayer.pos.z > 15000 && !secondStampPlane) secondStampPlane = getSec(); // plane started
                if (g_lPlayer.pos.z < 14000) secondStampPlane = 0; // jumped
                if(g_SearchFloatVal != 0.0f || g_SearchIntVal)
                    DMA::read_memory(g_decryptCgTPtr, uint64_t(&g_LPBuffer), 1700000);
                if (g_readAddressInput)
                    g_readAddressVal = DMA::Read<int>(
                        g_decryptBasePtr + Offsets::SIZE * i + g_readAddressInput);
            }
            if (cPlayer.isValid) validPlayersL++;

            // Velocity calulation
            auto timeSinceSample = getms() - g_playerArr[i].lastVelocitySampleTs;
            if (timeSinceSample > 100)
            {
                cPlayer.lastVelocitySamplePos = cPlayer.pos;
                cPlayer.velocity = calculateVelocity(g_playerArr[i].lastVelocitySamplePos, cPlayer.pos, timeSinceSample);
                cPlayer.lastVelocitySampleTs = getms();
            } else {
                cPlayer.velocity = g_playerArr[i].velocity;
                cPlayer.lastVelocitySamplePos = g_playerArr[i].lastVelocitySamplePos;
                cPlayer.lastVelocitySampleTs = g_playerArr[i].lastVelocitySampleTs;
            }

            if (!cPlayer.skipPlayer())
            {
                // Team links
                if (isOnScreen(cPlayer.screenPos, Vector2(30,30)) && (cPlayer.team>=0 && cPlayer.team<MAX_TEAM_COUNT) && cPlayer.distanceToLocal() < g_dontDrawCap && cPlayer.team!=g_lPlayer.team)
                    localTeamToLinkPositions[cPlayer.team].push_back(cPlayer.screenPos);
                // Enemy looks at player from outside fov (~170).
                if (cPlayer.localInCrosshairDirection > 50.0f && cPlayer.localInCrosshairDirection < 310.0f && g_lPlayer.team != cPlayer.team)
                    playersAimingFromBack.push_back(cPlayer.localInCrosshairDirection);
               /* switch (g_boolPick) {
                case 0: cPlayer.dormant = DMA::Read<bool>(g_decryptBasePtr + 0x1626 + i * Offsets::SIZE);
                case 1: cPlayer.dormant = DMA::Read<bool>(g_decryptBasePtr + 0x2af + i * Offsets::SIZE);
                case 2: cPlayer.dormant = DMA::Read<bool>(g_decryptBasePtr + 0x1704 + i * Offsets::SIZE);
                case 3: cPlayer.dormant = DMA::Read<bool>(g_decryptBasePtr + 0x568 + i * Offsets::SIZE);
                case 4: cPlayer.dormant = DMA::Read<bool>(g_decryptBasePtr + 0xd74 + i * Offsets::SIZE);
                case 5: cPlayer.dormant = DMA::Read<bool>(g_decryptBasePtr + 0x1556 + i * Offsets::SIZE);
                case 6: cPlayer.dormant = DMA::Read<bool>(g_decryptBasePtr + 0x15be + i * Offsets::SIZE);
                default: cPlayer.dormant = DMA::Read<bool>(g_decryptBasePtr + 0x1626 + i * Offsets::SIZE);
                }*/
                if (g_Verbose && !g_ItemESP)
                {
                    // print player info
                    printf(make_string("\n#%d: Name: %s, \n").c_str(),
                                       i, cPlayer.name.c_str());
                }
            }
            g_playerArr[i] = cPlayer;
        }
        std::unique_lock lock(g_TeamLinkMutex);
        g_TeamToLinkPositions.clear();
        g_TeamToLinkPositions = localTeamToLinkPositions; // anti-flickering
        lock.unlock();
        g_AimingAlert = !playersAimingFromBack.empty();
        g_AimingAlertAngles = playersAimingFromBack;
        g_validPlayers = validPlayersL; // fixes flickering to/from 0
        if (g_lPlayer.entIndex == -1) g_errMsg = make_string("Local player index not found!").c_str();
        if (g_Verbose) printf(make_string("Players registered: %d\n").c_str(), validPlayersL);
        if (g_Verbose) printf(make_string("Main loop time: %d\n").c_str(), getms() - time);
    }
    // delete[] characterArray;
}

int main()
{
    if (isCodRunning())
    {
        printf(make_string("\n\nCLOSE THE GAME BEFORE RUNNING THE CHEAT !").c_str());
        Sleep(3000);
        return 401;
    }
    SetWindowText(GetConsoleWindow(), L"Assist");

    setupTestData();

    // If LOCAL_INDEX is not found
    //g_LocalName = "awgXx";

    printf(make_string("Modern Warfare III\n").c_str());
    int port = 7000;
#ifndef DEBUG_GUI

    Sleep(100);

    printf(make_string("Connecting to DMA...").c_str());
    if (!DMA::Connect()) {
        printf(make_string("\x1B[31mConnection to DMA failed!\033[0m\n").c_str());
        Sleep(1000);
        return 501;
    }
    printf(make_string("\x1B[32mConnection Successful. (%s)\033[0m\n").c_str(),
        DMA::TestMode ? make_string("Mode = Dump").c_str() : make_string("Mode = Fpga").c_str());
#else
    printf(make_string("Running in GUI test release mode - dma connection ignored!!\n").c_str());
#endif

    constexpr int THREAD_CREATION_WAIT = 50;
    //g_overlayActive = true;
    printf(make_string("GUI->").c_str());
    CreateThread(nullptr, 0, GUIThread, nullptr, 0, nullptr);
    Sleep(THREAD_CREATION_WAIT);
    const auto ok = make_string("\x1B[32mOK\033[0m ");
    printf(ok.c_str());
    Sleep(THREAD_CREATION_WAIT);
    printf(make_string("KEY->").c_str());
    CreateThread(nullptr, 0, KeyMonitor, nullptr, 0, nullptr);
    CreateThread(nullptr, 0, RapidClick, nullptr, 0, nullptr);
    printf(ok.c_str());
    printf(make_string("AFK->").c_str());
    Sleep(THREAD_CREATION_WAIT);
    CreateThread(nullptr, 0, AFKThread, nullptr, 0, nullptr);
    printf(ok.c_str());
    Sleep(THREAD_CREATION_WAIT);
    printf(make_string("AIM->").c_str());
    CreateThread(nullptr, 0, AimLoop, nullptr, 0, nullptr);
    CreateThread(nullptr, 0, AimAssistThread, nullptr, 0, nullptr);
    printf(ok.c_str());
    printf(make_string("ITM->").c_str());
    CreateThread(nullptr, 0, ItemThread, nullptr, 0, nullptr);
    printf(ok.c_str());
    printf(make_string("METRICS->").c_str());
    CreateThread(nullptr, 0, MetricsSamplingThread, nullptr, 0, nullptr);
    printf(ok.c_str());
    printf("\n");


    while (g_Active)
    {
        if (g_SoftRestart)
        {
            Sleep(6000);
            printf(make_string("Soft Restart\n").c_str());
            g_SoftRestart = false;
        }
        // MAIN LOOP
        printf(make_string("\x1B[32m[*]\033[0m Launch CoD...").c_str());
        // wait for inpit
        while (g_PID == 0 && g_Active)
        {
            g_PID = DMA::AttachToProcessId(const_cast<LPSTR>(make_string("cod.exe").c_str()));

            if (g_PID == 0)
            {
                printf(".");
            }
            Sleep(2000);
        }
        if (!g_PID) {
            std::cerr << "\n [log] -> failed to attach to process.\n";
            std::cin.get();
            return 502;
        }
        std::cout << "\n [log] -> attached to process successfully.\n";
        if (!opdaterBase())
        {
            printf(make_string("Game base address not found\n").c_str());
            continue;
        }
        if (!opdaterPeb())
        {
            printf(make_string("PEB not found\n").c_str());
            continue;
        }
        DoActions();
        g_PID = 0;
        g_Base = 0;
        g_Peb = 0;
        g_decryptCase = 0;
        g_decryptBasePtr = 0;
        g_decryptCgTPtr = 0;
        g_CEntityBase = 0;
        g_Refdef = refdef_t();
        g_lPlayer = Player(-1);
        g_errMsg = "";
    }
    DMA::Disconnect();

    printf(make_string("\nEXIT\n").c_str());
    //Sleep(20000); 
    return 200;
}

bool opdaterPeb()
{
    g_Peb = DMA::GetPEBAddress();
    if (g_Peb == 0)
    {
        g_Peb = 0;
        Sleep(500);
        return false;
    }
    //printf("PEBadr = 0x%I64X\n", g_Peb);
    return true;
}

bool gameRunning()
{
    g_gameRunning = DMA::Read<uint64_t>(g_Base + 0x50); // check if theres non-zero memory at 0x50
    return g_gameRunning;
}

bool opdaterBase()
{
    g_Base = DMA::GetBaseAddress();
    if (g_Base == 0)
    {
        g_PID = 0;
        Sleep(500);
        return false;
    }
    return true;
}

static DWORD WINAPI AimLoop(void* params)
{
    int AIM_CIRCLE_RADIUS;
    g_closestIndex = -1;
    while (g_Active)
    {
        Sleep(10);

        while (g_AimBotActive && g_AimBotMode == 0)
        {
            if (g_AFK) AIM_CIRCLE_RADIUS = 300; // bigger detection for afk
            else AIM_CIRCLE_RADIUS = 150;
            if (g_hasVisibleTarget && g_AFK) AIM_CIRCLE_RADIUS += 300;
            Sleep(g_Verbose ? 1000 : 1);

            g_closestIndex = -1;
            for (int i = 0; i < MAX_PLAYER_COUNT; i++)
            {
                ESPBox box = g_ESP_DrawList[i];
                // Convert ImU32 to Vector3
                Vector3 color = Vector3((box.color & 0xFF), (box.color & 0xFF00) >> 8, (box.color & 0xFF0000) >> 16);
                if (box.screenPos.x <= 0 && box.screenPos.y <= 0) continue; // dont aim invalid player(s)
                // dont aim downed (x=red=160) or not visible players if we have vis
                if (color.x == 160 || g_hasVisibleTarget && color.x == 255) continue;
                if (g_closestIndex == -1) g_closestIndex = i;
                else if (center.distance(box.screenPos) < center.distance(g_ESP_DrawList[g_closestIndex].screenPos))
                    g_closestIndex = i; // another player is closer
                if (g_Verbose)
                    printf("%d: %s, (%.0f, %.0f), A: %.0f\n", i, box.textName.c_str(), box.screenPos.x, box.screenPos.y,
                           center.distance(box.screenPos));
            }
            vector<Vector2> cp_bots = g_botsOnScreen;
            for (int i = 0; i < cp_bots.size(); i++)
            {
                Vector2 bot = cp_bots[i];
                if (center.distance(bot) < center.distance(g_ESP_DrawList[g_closestIndex].screenPos))
                    g_closestIndex = MAX_PLAYER_COUNT + i; // bot is closer
            }
            if (g_closestIndex == -1)
            {
                if (g_Verbose) printf(make_string("[AimBot] No targets...\n").c_str());
                continue;
            }
            ESPBox closest;
            if (g_closestIndex < MAX_PLAYER_COUNT)
                closest = g_ESP_DrawList[g_closestIndex];
            else
            {
                closest = ESPBox();
                closest.screenPos = cp_bots[g_closestIndex - MAX_PLAYER_COUNT];
                closest.size = Vector2(30, 30);
            }
            Vector2 playerUpperCenter = Vector2(closest.size.x / 2, closest.size.y / 5);
            float distanceToClosest = center.distance(closest.screenPos + playerUpperCenter);
            // dist to center of person
            if (distanceToClosest > AIM_CIRCLE_RADIUS) continue;

            int speed = 10;
            if (distanceToClosest < 100) speed = 5;
            if (distanceToClosest < 50) speed = 2;
            //if (distanceToClosest < 20) g_AimSpeed = 5;
            Vector2 dV = closest.screenPos + playerUpperCenter - center; //dV -> center to centerOfPerson
            Vector2 revDV = dV; //Save dV
            dV.x /= speed;
            dV.y /= speed;
            if ((int)dV.x >= 25) dV.x = revDV.x < 0 ? -9 : 9; // altid flyt sig lidt
            if ((int)dV.y >= 25) dV.y = revDV.y < 0 ? -9 : 9;
            mouse_event(MOUSEEVENTF_MOVE, dV.x, dV.y, NULL, NULL);
            if (g_Verbose && g_AimBotActive)
            {
                printf(make_string("[AimBot] Aiming at: (%.0f,%.0f), Distance: %.0f \n").c_str(), closest.screenPos.x, closest.screenPos.y,
                       distanceToClosest);
            }
        }
    }
    return 201;
}

Player FindTargetPlayer(const float aimCircleRadius, const float invisibleAimCircleRadius, const float maximumDistance,
                        const float worldDistanceWeight)
{
    Player target;
    float minDistance = (std::numeric_limits<float>::max)();

    for (int i = 0; i < g_MaxPlayers; i++)
    {
        auto& player = g_playerArr[i];
        if (player.entIndex == g_lPlayer.entIndex
            || (player.team == g_lPlayer.team)
            || player.isDead
            || (player.health < 1 && player.stance != CharacterStance::Downed))
        {
            continue;
        }
        if (player.isInGulag != g_lPlayer.isInGulag)
        {
            continue;
        }

        const float worldDistance = player.distanceToLocal();
        if (worldDistance < 0.1 || worldDistance > g_dontDrawCap)
        // Same position or further than maximum distance threshold
        {
            continue;
        }

        auto pos = player.pos;
        pos.z += 51.f; // Adjust the Z position to aim at the head

        Vector2 screenPos = worldToScreen(player.pos, g_CameraPos);
        const float screenDistance = (screenPos - center).length();
        if (screenDistance > aimCircleRadius) continue; // Skip players outside the aim circle
        if (screenDistance > invisibleAimCircleRadius && !player.isVisible) continue;
        // Skip invisible players outside the invisible aim circle
        // Calculate a weighted sum of the 3D world distance and the 2D screen distance
        // Adjust the weights as needed to prioritize one distance over the other
        const float weightedSum = worldDistanceWeight * worldDistance + (1.0f - worldDistanceWeight) * screenDistance;

        if (weightedSum < minDistance)
        {
            minDistance = weightedSum;
            target = player;
        }
    }
    return target;
}

// Linear interpolation (Lerp) function for smooth speed adjustment
float Lerp(const float start, const float end, const float factor)
{
    return start + factor * (end - start);
}

// Function to calculate the adjusted aim vector
Vector2 CalculateAimAdjustment(Vector2 directionToTarget, const float maxSpeed, const float minSpeed,
                               const float aimCircleRadius, const float deadZoneThreshold)
{
    const float distanceToTarget = directionToTarget.length();

    // Normalize the factor based on aimCircleRadius
    const float normalizedFactor = (std::min)(1.0f, distanceToTarget / aimCircleRadius);

    // Calculate speed using lerp for smoother adjustment
    float speed = Lerp(maxSpeed, minSpeed, normalizedFactor);

    // If within dead zone, reduce or stop adjustments
    if (distanceToTarget < deadZoneThreshold)
    {
        speed *= 0.1f; // Significantly reduce speed to minimize jitter
    }

    // Normalize direction to target and scale by speed
    return directionToTarget.normalize() * speed;
}

DWORD WINAPI AimAssistThread(LPVOID lpParam)
{
    while (g_Active)
    {
        while (g_Active && g_MaxPlayers <= 1)
        {
            Sleep(4000); // Wait for the game to start
        }

        if (!g_Active) break;

        // Calculate the screen center
        center.x = static_cast<float>(g_Refdef.width) / 2.0f;
        center.y = static_cast<float>(g_Refdef.height) / 2.0f;

        while (g_Active && g_MaxPlayers > 1)
        {
            if (g_AimBotActive && g_AimBotMode == 1)
            {
                const AimbotSettings* aim = &g_AimbotSettings;
                Player targetPlayer = FindTargetPlayer(aim->getAimCircleRadius(), aim->getInvisibleAimCircleRadius(),
                aim->getMaximumDistance(), aim->getWorldDistanceWeight());
                
                if (targetPlayer.skipPlayer())
                {
                    Sleep(40); // If no valid target, wait a bit and continue
                    continue;
                }

                auto pos = targetPlayer.pos;
                switch (targetPlayer.stance)
                {
                case CharacterStance::Standing:
                    pos.z += 51.f; // Adjust the Z position to aim at the upper body
                    break;
                case CharacterStance::Crouching:
                    pos.z += 36.f;
                    break;
                case CharacterStance::Crawling:
                case CharacterStance::Downed:
                    pos.z += 12.f;
                    break;
                }

                // Smoothly adjust the camera (or mouse) to aim at the target
                const Vector2 adjustment = CalculateAimAdjustment(worldToScreen(pos, g_CameraPos) - center,
                                                                  aim->getMaxSpeed(), aim->getMinSpeed(),
                                                                  aim->getAimCircleRadius(),
                                                                  aim->getDeadZoneThreshold());

                // Apply the movement
                mouse_event(MOUSEEVENTF_MOVE, static_cast<int>(adjustment.x), static_cast<int>(adjustment.y), 0, 0);
            }
            Sleep(20); // Small delay to prevent high CPU usage
        }
    }
    return 0;
}
