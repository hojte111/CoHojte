#pragma once
#include <random>
#include "globals.h"


int random1_6();
void moveNClick(int x, int y, int maxTries);
void shootTargets(int duration, bool force);
DWORD WINAPI AFKThread(void* params);
extern long long secondStampPlane;
