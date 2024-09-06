#ifndef PUBLIC_RELEASE
#include "AFK.h"

long long secondStampPlane = 0;

int random1_6()
{
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist6(1, 6); // distribution in range [1, 6]
    return dist6(rng);
}

void MoveRelative(int x, int y, int speed = 10)
{
    g_AFKState = "MoveRelative(" + std::to_string(x) + "," + std::to_string(y) + ") " + std::to_string(speed);
    const int MIN_SPEED = 1;
    const int MAX_SPEED = 20;
    const int DEFAULT_SPEED = 10;

    // Ensure speed is within bounds
    if (speed < MIN_SPEED) speed = DEFAULT_SPEED;
    if (speed > MAX_SPEED) speed = MAX_SPEED;

    // Calculate the total distance and the number of steps
    double distance = sqrt(x * x + y * y);
    int steps = distance / speed;
    if (steps < 1) steps = 1;

    // Calculate the increments per step
    double xIncrement = (double)x / steps;
    double yIncrement = (double)y / steps;

    for (int i = 0; i < steps; i++)
    {
        // Calculate the next step with some randomness
        int dx = (int)(xIncrement + ((rand() % (speed * 2)) - speed) / 2.0);
        int dy = (int)(yIncrement + ((rand() % (speed * 2)) - speed) / 2.0);

        // Use mouse_event to simulate the relative movement
        mouse_event(MOUSEEVENTF_MOVE, dx, dy, 0, 0);

        // Sleep to mimic human speed variability
        Sleep(10 + (rand() % 20));
    }
}

// Move mouse to x, y and click if click is true, if ingame use maxTries before skipping
void moveNClick(int x, int y, bool click = true, int maxTries = 50, bool updateState = true)
{
    if (updateState) g_AFKState = "MoveNClick(" + std::to_string(x) + "," + std::to_string(y) + ") " + (
        click ? "click" : "no click");
    // TODO - add random movement to make it look (even) more human
    // TODO - use g_WarfareWith/Height to get correct coordinates assumes (1920x1080) resolution atm.

    int64_t startTime = getSec();

    POINT p;

    GetCursorPos(&p);
    int dx;
    int dy;

    int tries = 0;
    int oldX;
    int oldY;
    oldX = x - p.x;
    oldY = y - p.y;

    while (1)
    {
        dx = x - p.x;
        dy = y - p.y;
        int yToMove = 0;
        int xToMove = 0;
        if (abs(dx) < 15 && abs(dy) < 15) break; // We are on target...
        if (dx != 0) xToMove = dx < 0 ? -3 : 3; // altid flyt sig lidt // SPEED
        if (abs(dy) > 3) yToMove = dy < 0 ? -3 : 3;

        // Random movement
        yToMove += random1_6() <= 3 ? -random1_6() / 2 : random1_6() / 2;
        yToMove += random1_6() <= 3 ? -random1_6() / 2 : random1_6() / 2;

        mouse_event(MOUSEEVENTF_MOVE, xToMove, yToMove, NULL, NULL);

        Sleep(10);
        GetCursorPos(&p);
        if ((abs(dx - oldX) < 50) && ++tries > maxTries || !g_AFK)
        {
            // the game moves cursor to the center
            if (g_Verbose) printf("[MOVE'N'CLICK] skipper efter %d gange fordi vi er ingame (%d)\n", tries,
                                  abs(dx - oldX));
            return;
        }
        if (getSec() - startTime > 9)
        {
            if (g_Verbose) printf("[MOVE'N'CLICK] skipper efter 8 sek fordi vi er ingame\n");
            return;
        }
    }

    Sleep(150 + random1_6() * 10);
    if (!click) return;
    mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
    Sleep(98 * random1_6());
    mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
    Sleep(50 * random1_6());
}

// Shoot targets for duration in millis, if force is true, shoot even if no targets are visible
void shootTargets(int duration, bool force = false)
{
    if (!g_hasVisibleTarget && !force) return; // dont shoot if no targets...
    g_AimBotActive = true;
    Sleep(50); // Initial aimbot to not shoot before target is aimed
    int someDuration = duration * 0.3f; // more or less shooting time
    int toAdd = random1_6() <= 3 ? -random1_6() * someDuration : random1_6() * someDuration;
    int shootTime = abs(duration + toAdd);
    g_AFKState = "Shoot " + std::to_string(duration);
    mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
    for (int i = 0; i < 4; i++)
    {
        // Make bot not look aimbotting obvious by turning aimbot on and off during shooting
        g_AimBotActive = false;
        Sleep((shootTime / 4) / 2);
        g_AimBotActive = true;
        Sleep((shootTime / 4) / 2);
    }
    mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
    g_AimBotActive = false;
}

// Checks if ingame, and mouse is locked to center, and if not spectating another player
bool isPlaying()
{
    Sleep(random1_6() * 100);
    float savedYaw = g_lPlayer.yaw;
    float savedPitch = g_lPlayer.pitch;
    float savedX = g_lPlayer.pos.x;
    random1_6() < 4
        ? MoveRelative(40 + random1_6() * 10, 40 + random1_6() * 10)
        : MoveRelative(-40 - random1_6() * 10, -40 - random1_6() * 10);
    Sleep(24 * random1_6());
    // Check if spectating or in menu. Didn't move location, but moved view.
    if (abs(savedYaw - g_lPlayer.yaw) < 0.5f || abs(savedPitch - g_lPlayer.pitch) < 0.5f || abs(
        savedX - g_lPlayer.pos.x) > 0.5f)
    {
        // printf("[AFK] yaw | pitch samme | location forskellig, %.3f == %.3f | %.3f == %.3f | %.3f != %.3f\n",
        // savedYaw, g_lPlayer.yaw, savedPitch, g_lPlayer.pitch, savedX, g_lPlayer.pos.x);
        return false; // out of game or in menu
    }

    Sleep(100 * random1_6());
    savedYaw = g_lPlayer.yaw;
    savedPitch = g_lPlayer.pitch;
    MoveRelative(0, -40 - random1_6() * 10);
    Sleep(24 * random1_6());
    // moved only pitch.
    if (abs(savedYaw - g_lPlayer.yaw) > 0.5f || abs(savedPitch - g_lPlayer.pitch) < 0.5f || abs(
        savedX - g_lPlayer.pos.x) > 0.5f)
    {
        // printf("[AFK] yaw | locX er forskellig %.3f != %.3f | %.3f != %.3f\n",
        // savedYaw, g_lPlayer.yaw, savedX, g_lPlayer.pos.x);
        return false; // spectating
    }
    return true;
}

// Select first weapon class
void selectClass()
{
    g_AFKState = "Klasse select";
    moveNClick(161, 952, true, 50, false); // select class / quickplay
    Sleep(1000 + random1_6() * 10);
}

// Click play again button (down right) and confirm dialog
void playAgain()
{
    g_AFKState = "PlayAgain";
    Sleep(500);
    moveNClick(1500 + random1_6(), g_playAgainY + random1_6() + g_playAgainAdjustmentY, true, 50, false); // Play again
    g_AFKState = "PlayAgainConfirm";
    Sleep(500);
    moveNClick(720 + random1_6() * 10, 611 + random1_6(), true, 50, false); // Confirm dialog
}

// Restore to normal orientation
void lookStraight()
{
    g_AFKState = "LookStraight";

    float oldPitch = g_lPlayer.pitch;
    while (!(g_lPlayer.pitch >= 0.1f && g_lPlayer.pitch <= 20.0f)) // facing a bit down
    {
        if (!g_AFK) break;
        if (g_lPlayer.pitch >= 10 && g_lPlayer.pitch <= 170)
            MoveRelative(-3 + random1_6(), -17 - random1_6() * 3, 14); // somewhere down
        else if (g_lPlayer.pitch <= 360 && g_lPlayer.pitch >= 190)
            MoveRelative(-3 + random1_6(), 17 + random1_6() * 3, 14); // somewhere up
        else
        {
            g_AFKState = "LookStraightFault";
            Sleep(750);
            break;
        }
        if (oldPitch == g_lPlayer.pitch)
        {
            g_AFKState = "LookStraightOutOfG";
            Sleep(750);
            break; // out of game
        }
        Sleep(3 * random1_6());
    }
}

void turnHead(int degrees, bool right = true)
{
    g_AFKState = "TurnHead " + std::to_string(degrees) + (right ? " right" : " left");
    float savedYaw = g_lPlayer.yaw;
    while (abs(savedYaw - g_lPlayer.yaw) < degrees)
    {
        float oldYaw = g_lPlayer.yaw;
        if (!g_AFK) break;
        if (right) MoveRelative(80 + random1_6() * 9, -6 + random1_6() * 2, 12 + random1_6()); // somewhere right
        else MoveRelative(-80 - random1_6() * 9, -6 + random1_6() * 2, 12 + random1_6()); // somewhere left
        Sleep(5 * random1_6());
        if (oldYaw == g_lPlayer.yaw)
        {
            g_AFKState = "TurnHeadOutOfG";
            Sleep(750);
            break; // out of game
        }
    }
}

DWORD WINAPI AFKThread(void* params)
{
    g_AFK = false;
    int64_t outOfPlane = 0;
    while (g_Active)
    {
        Sleep(100);

        if (g_AFK)
        {
            g_AimBotActive = false; // some bug making the aimbot be permanently on
            // afk on
            // Check in plane
            if (getSec() - secondStampPlane > 20)
            {
                mouse_event(MOUSEEVENTF_HWHEEL, 0, 0, random1_6(), 0);
                outOfPlane = getSec();
            }
            if (getSec() - outOfPlane > 8) mouse_event(MOUSEEVENTF_HWHEEL, 0, 0, random1_6(), 0);

            if (!g_northOriented && !isPlaying()) selectClass(); // use g_NorthOriented as flag for warzone afk.
            else if (g_northOriented && !isPlaying()) playAgain(); // north true is warzone afk
            if (!isPlaying())
            {
                g_AFKState = "UdeAfSpil";
                Sleep(3000);
                continue;
            }

            shootTargets(238 + random1_6() * 100);
            lookStraight();
            g_AFKState = "Walk0";
            mouse_event(MOUSEEVENTF_XDOWN, 0, 0, XBUTTON2, 0); // START WALK
            turnHead(random1_6() * 4, random1_6() < 4); // random direction random amount
            Sleep(664 * random1_6());
            mouse_event(MOUSEEVENTF_XUP, 0, 0, XBUTTON2, 0); // END WALK
            turnHead(10, random1_6() < 4);
            shootTargets(100 * random1_6());
            mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, 0); // throw grenade with mouse wheel click
            Sleep(189 * random1_6());
            mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, 0);
            Sleep(486 * random1_6());
            turnHead(5 * random1_6(), random1_6() < 4);
            shootTargets(66 * random1_6(), true);
            if (!g_AFK) continue; // if afk is disabled mid-code skip rest
            shootTargets(87 * random1_6());
            mouse_event(MOUSEEVENTF_HWHEEL, 0, 0, -random1_6(), 0); // tilt to left to use tactical
            shootTargets(70 * random1_6());

            g_AFKState = "Walk1st";
            mouse_event(MOUSEEVENTF_XDOWN, 0, 0, XBUTTON2, 0); // START WALK
            turnHead(random1_6() * 7, random1_6() < 4); // random direction random amount
            mouse_event(MOUSEEVENTF_XUP, 0, 0, XBUTTON2, 0); // END WALK

            shootTargets(200 * random1_6());

            if (!g_AFK) continue; // if afk is disabled mid-code skip rest

            g_AFKState = "Walk2nd";
            mouse_event(MOUSEEVENTF_XDOWN, 0, 0, XBUTTON2, 0); // WALK START
            Sleep(489 * random1_6()); // WAIT(hold) for 0.4-2.4 secs
            shootTargets(54 * random1_6(), random1_6() < 4); // 50% chance of force
            mouse_event(MOUSEEVENTF_XUP, 0, 0, XBUTTON2, 0); // WALK END

            shootTargets(106 * random1_6());
            turnHead(random1_6() * 7, random1_6() < 4); // random direction random amount
            mouse_event(MOUSEEVENTF_HWHEEL, 0, 0, -random1_6(), 0); // tilt to left to use tactical


            if (!g_AFK) continue; // if afk is disabled mid-code skip rest
            if (random1_6() < 5) // 66% chance
            {
                if (isPlaying())
                {
                    // Use field upgrade
                    mouse_event(MOUSEEVENTF_HWHEEL, 0, 0, -random1_6(), 0); // scroll up
                }

                Sleep(250 * random1_6()); // wait
                mouse_event(MOUSEEVENTF_HWHEEL, 0, 0, -random1_6(), 0); //tilt to left to use tactical
            }

            shootTargets(400);

            if (!g_AFK) continue; // if afk is disabled mid-code skip rest
            if (g_Verbose) printf("%lld sek i fly...", secondStampPlane ? getSec() - secondStampPlane : 0);
            //tilt to right to jump out of plane
            if (getSec() - secondStampPlane > 15) mouse_event(MOUSEEVENTF_HWHEEL, 0, 0, random1_6(), 0);

            lookStraight();
            Sleep(100 * random1_6());

            g_AFKState = "WalkChecking";
            float savedLoc = g_lPlayer.pos.x;
            mouse_event(MOUSEEVENTF_XDOWN, 0, 0, XBUTTON2, 0); // WALK START
            Sleep(287 * random1_6()); // hold for 0.2-1.7 secs
            mouse_event(MOUSEEVENTF_XUP, 0, 0, XBUTTON2, 0); // WALK END
            if (abs(savedLoc - g_lPlayer.pos.x) < 0.5f) // Player stuck somewhere
            {
                g_AFKState = "StuckWalkTurning";
                turnHead(131 + random1_6() * 8, random1_6() < 4); // random direction random amount
            }
        }
    }
    return 0;
}
#endif