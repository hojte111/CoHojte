#pragma once
#include <iostream>
#include <fstream>
#include "globals.h"
#include "no_strings.hpp"
#include <algorithm>
#include "Utility.h"


DWORD WINAPI MetricsSamplingThread(void* params);
void handleEndOfGame(score_t cachedScore, SkillScore cachedSkillScore, bool isWZ);
void calculateSkillScoreInLobby();