#include "MetricsSampler.h"

auto skillScoreList = std::vector<SkillScore>();
std::string seed = "none";
// Metric Sampler, used to sample metrics from the local player and store them in a text file for persistence.
DWORD WINAPI MetricsSamplingThread(void* params)
{
    // Constants
    const int64_t ITERATION_TIME = 20; // milliseconds
    const std::string fileName = make_string("data");
    const std::string aFrontEntclient = make_string("frontEntclient");
    const std::string fileLocation = make_string("./");

    // clientName is the name of the local player
    bool endOfGameHandled = false;
    int64_t scoreZeroTs = 0; // flickering scoreboard fix
    score_t lastValidScore = {};
    SkillScore lastValidSkillScore = {};
    bool wasWzAtLastValidScore = false;
    uint64_t detectedNameHash;
    uint64_t changeNameRequestTs = 0;
    
    // read serialized data from file
    std::ifstream inFile(fileLocation+fileName, std::ios::in | std::ios::binary);
    if (inFile) {
        inFile.read(reinterpret_cast<char*>(&g_killRecordMP), sizeof(g_killRecordMP));
        inFile.read(reinterpret_cast<char*>(&g_killRecordWZ), sizeof(g_killRecordWZ));
        inFile.read(reinterpret_cast<char*>(&g_deathRecordMP), sizeof(g_deathRecordMP));
        inFile.read(reinterpret_cast<char*>(&g_deathRecordWZ), sizeof(g_deathRecordWZ));
        inFile.read(reinterpret_cast<char*>(&g_programRestarts), sizeof(g_programRestarts));
        inFile.read(reinterpret_cast<char*>(&g_inGameTimeMP), sizeof(g_inGameTimeMP));
        inFile.read(reinterpret_cast<char*>(&g_inGameTimeWZ), sizeof(g_inGameTimeWZ));
        inFile.read(reinterpret_cast<char*>(&g_timeAliveMP), sizeof(g_timeAliveMP));
        inFile.read(reinterpret_cast<char*>(&g_timeAliveWZ), sizeof(g_timeAliveWZ));
        inFile.read(reinterpret_cast<char*>(&g_timeAimbotActive), sizeof(g_timeAimbotActive));
        inFile.read(reinterpret_cast<char*>(&g_timeSpentShooting), sizeof(g_timeSpentShooting));
        inFile.read(reinterpret_cast<char*>(&g_timeSpentAiming), sizeof(g_timeSpentAiming));
        inFile.read(reinterpret_cast<char*>(&g_gameOpenTime), sizeof(g_gameOpenTime));
        inFile.read(reinterpret_cast<char*>(&g_AimBotActiveInADSDurationMillis), sizeof(g_AimBotActiveInADSDurationMillis));
        inFile.read(reinterpret_cast<char*>(&g_AimBotActiveInShootingDurationMillis), sizeof(g_AimBotActiveInShootingDurationMillis));
        inFile.read(reinterpret_cast<char*>(&g_surveyRemindLater), sizeof(g_surveyRemindLater));
        inFile.read(reinterpret_cast<char*>(&g_showPlayerNames), sizeof(g_showPlayerNames));

        // General settings
        inFile.read(reinterpret_cast<char*>(&g_EnableAimingAlert), sizeof(g_EnableAimingAlert));
        inFile.read(reinterpret_cast<char*>(&g_AimBotMode), sizeof(g_AimBotMode));
        inFile.read(reinterpret_cast<char*>(&g_enablePlayerLookingDirection), sizeof(g_enablePlayerLookingDirection));
        inFile.read(reinterpret_cast<char*>(&g_guiOpacity), sizeof(g_guiOpacity));
        inFile.read(reinterpret_cast<char*>(&g_crosshair), sizeof(g_crosshair));

        // Aimbot settings
        inFile.read(reinterpret_cast<char*>(&g_AimbotSettings.maximumDistance), sizeof(g_AimbotSettings.maximumDistance));
        inFile.read(reinterpret_cast<char*>(&g_AimbotSettings.maxSpeed), sizeof(g_AimbotSettings.maxSpeed));
        inFile.read(reinterpret_cast<char*>(&g_AimbotSettings.minSpeed), sizeof(g_AimbotSettings.minSpeed));
        inFile.read(reinterpret_cast<char*>(&g_AimbotSettings.aimCircleRadius), sizeof(g_AimbotSettings.aimCircleRadius));
        inFile.read(reinterpret_cast<char*>(&g_AimbotSettings.deadZoneThreshold), sizeof(g_AimbotSettings.deadZoneThreshold));
        inFile.read(reinterpret_cast<char*>(&g_AimbotSettings.invisibleAimCircleRadius), sizeof(g_AimbotSettings.invisibleAimCircleRadius));
        inFile.read(reinterpret_cast<char*>(&g_AimbotSettings.worldDistanceWeight), sizeof(g_AimbotSettings.worldDistanceWeight));

        inFile.read(reinterpret_cast<char*>(&g_assistRecordMP), sizeof(g_assistRecordMP));
        inFile.read(reinterpret_cast<char*>(&g_assistRecordWZ), sizeof(g_assistRecordWZ));
        inFile.read(reinterpret_cast<char*>(&g_damageRecordMP), sizeof(g_damageRecordMP));
        inFile.read(reinterpret_cast<char*>(&g_damageRecordWZ), sizeof(g_damageRecordWZ));
        inFile.read(reinterpret_cast<char*>(&g_damageTakenRecordMP), sizeof(g_damageTakenRecordMP));
        inFile.read(reinterpret_cast<char*>(&g_damageTakenRecordWZ), sizeof(g_damageTakenRecordWZ));
        inFile.read(reinterpret_cast<char*>(&g_pointsRecordMP), sizeof(g_pointsRecordMP));
        inFile.read(reinterpret_cast<char*>(&g_pointsRecordWZ), sizeof(g_pointsRecordWZ));
        inFile.read(reinterpret_cast<char*>(&g_gameCountMP), sizeof(g_gameCountMP));
        inFile.read(reinterpret_cast<char*>(&g_gameCountWZ), sizeof(g_gameCountWZ));
        inFile.read(reinterpret_cast<char*>(&g_avgSkillIndex), sizeof(g_avgSkillIndex));

        inFile.read(reinterpret_cast<char*>(&detectedNameHash), sizeof(detectedNameHash));
        inFile.read(reinterpret_cast<char*>(&g_accumulatedDamageDone), sizeof(g_accumulatedDamageDone));
        inFile.read(reinterpret_cast<char*>(&g_accumulatedShotsFired), sizeof(g_accumulatedShotsFired));
        inFile.read(reinterpret_cast<char*>(&g_accumulatedShotsMissed), sizeof(g_accumulatedShotsMissed));
        
        inFile.close();
    }
    g_programRestarts++;

    auto lastUpdateTime = getms();
    
    
    while (g_Active)
    {
        /*if (GetAsyncKeyState('U') & 1)
            handleEndOfGame(lastValidScore, lastValidSkillScore, wasWzAtLastValidScore);
        g_detectedLocalName = "hojte1111";*/
        seed = g_detectedLocalName;
        auto currentTime = getms();
        auto elapsedMs = getms() - lastUpdateTime;
        lastUpdateTime = currentTime;
        if (g_gameRunning) g_gameOpenTime += elapsedMs;
        else
        {
            Sleep(ITERATION_TIME);
            continue;
        }
        
        //g_errMsg = std::to_string(changeNameRequestTs);
        if (g_hasVisibleTarget && g_detectedLocalName.empty()) g_detectedLocalName = g_lPlayer.name;
        if (g_hasVisibleTarget && g_lPlayer.name != g_detectedLocalName && !changeNameRequestTs) changeNameRequestTs = getSec();
        if (g_hasVisibleTarget && g_lPlayer.name != g_detectedLocalName && changeNameRequestTs && getSec()-changeNameRequestTs>20) {
            g_detectedLocalName = g_lPlayer.name;
            changeNameRequestTs = 0;
        }
        if (g_lPlayer.name == g_detectedLocalName) // reset change counter bcs it was just a killcam / final kill
            changeNameRequestTs = 0;
        

        // Update realLocalPlayer
        for (int i = 0; i < MAX_PLAYER_COUNT; i++)
        {
            if (g_playerArr[i].skipPlayer()) continue;
            auto currName = g_playerArr[i].name;
            if (!currName.empty() && fnv1aHash(currName) == detectedNameHash) g_detectedLocalName = currName;
            if (currName == g_detectedLocalName)
            {
                g_realLocalPlayer = g_playerArr[i];
                break;
            }
        }
        if (g_detectedLocalName.empty())
        {
            Sleep(ITERATION_TIME);
            continue;
        }

        bool isInAnyGame = g_MaxPlayers > 1;
        bool isInWZ = isInAnyGame && g_MaxPlayers > 30;
        bool isInMP = isInAnyGame && g_MaxPlayers < 30;

        if (isInMP) g_inGameTimeMP += elapsedMs;
        else if (isInWZ) g_inGameTimeWZ += elapsedMs;

        bool isInGamePlaying = isInAnyGame && g_realLocalPlayer.score.status == 0 && g_realLocalPlayer.score.rank_level != 0;
        if (isInGamePlaying)
        {
            calculateSkillScoreInLobby();
            lastValidScore = g_realLocalPlayer.score;
            auto tmp = g_realLocalPlayer.getSkillScore();
            if (tmp.skillIndex != 0) lastValidSkillScore = g_realLocalPlayer.getSkillScore();
            wasWzAtLastValidScore = isInWZ;
            isInWZ ? g_timeAliveWZ += elapsedMs : g_timeAliveMP += elapsedMs;

            // Aimbot metrics
            if (g_realLocalPlayer.aiming) g_timeSpentAiming += elapsedMs;
            if (g_realLocalPlayer.shooting) g_timeSpentShooting += elapsedMs;
            if (g_AimBotActive) {
                g_timeAimbotActive += elapsedMs;
                if (g_realLocalPlayer.aiming) g_AimBotActiveInADSDurationMillis += elapsedMs;
                if (g_realLocalPlayer.shooting) g_AimBotActiveInShootingDurationMillis += elapsedMs;
            }
        }
        if (!isInAnyGame && !endOfGameHandled) {
            if (!scoreZeroTs) scoreZeroTs = getms();
            else if (getms() - scoreZeroTs > 1000) { // time that score has to be 0 before we update the metrics
                // g_errMsg = make_string("End of game detected, points: ") + std::to_string(latestScore.points);
                handleEndOfGame(lastValidScore, lastValidSkillScore, wasWzAtLastValidScore);
                endOfGameHandled = true; 
            }
        }
        else if (isInAnyGame && endOfGameHandled) {
            endOfGameHandled = false;
            scoreZeroTs = 0;
        }

        // Reset low score in order to update lower bound
        g_LowScore = {};
        g_HighScore = {};
        // float max
        g_LowScore.points = 999999;
        g_LowScore.assists = 999999;
        g_LowScore.kills = 999999;
        g_LowScore.damageDone = 999999;
        g_LowScore.level = 999999;

        Sleep(ITERATION_TIME);
    }
    // On Close
    g_surveyRemindLater = g_remindSet;
    std::ofstream outFile(fileLocation+fileName, std::ios::out | std::ios::binary);
    if (outFile) {
        outFile.write(reinterpret_cast<const char*>(&g_killRecordMP), sizeof(g_killRecordMP));
        outFile.write(reinterpret_cast<const char*>(&g_killRecordWZ), sizeof(g_killRecordWZ));
        outFile.write(reinterpret_cast<const char*>(&g_deathRecordMP), sizeof(g_deathRecordMP));
        outFile.write(reinterpret_cast<const char*>(&g_deathRecordWZ), sizeof(g_deathRecordWZ));
        outFile.write(reinterpret_cast<const char*>(&g_programRestarts), sizeof(g_programRestarts));
        outFile.write(reinterpret_cast<const char*>(&g_inGameTimeMP), sizeof(g_inGameTimeMP));
        outFile.write(reinterpret_cast<const char*>(&g_inGameTimeWZ), sizeof(g_inGameTimeWZ));
        outFile.write(reinterpret_cast<const char*>(&g_timeAliveMP), sizeof(g_timeAliveMP));
        outFile.write(reinterpret_cast<const char*>(&g_timeAliveWZ), sizeof(g_timeAliveWZ));
        outFile.write(reinterpret_cast<const char*>(&g_timeAimbotActive), sizeof(g_timeAimbotActive));
        outFile.write(reinterpret_cast<const char*>(&g_timeSpentShooting), sizeof(g_timeSpentShooting));
        outFile.write(reinterpret_cast<const char*>(&g_timeSpentAiming), sizeof(g_timeSpentAiming));
        outFile.write(reinterpret_cast<const char*>(&g_gameOpenTime), sizeof(g_gameOpenTime));
        outFile.write(reinterpret_cast<const char*>(&g_AimBotActiveInADSDurationMillis), sizeof(g_AimBotActiveInADSDurationMillis));
        outFile.write(reinterpret_cast<const char*>(&g_AimBotActiveInShootingDurationMillis), sizeof(g_AimBotActiveInShootingDurationMillis));
        outFile.write(reinterpret_cast<const char*>(&g_surveyRemindLater), sizeof(g_surveyRemindLater));
        outFile.write(reinterpret_cast<const char*>(&g_showPlayerNames), sizeof(g_showPlayerNames));

        // General settings
        outFile.write(reinterpret_cast<const char*>(&g_EnableAimingAlert), sizeof(g_EnableAimingAlert));
        outFile.write(reinterpret_cast<const char*>(&g_AimBotMode), sizeof(g_AimBotMode));
        outFile.write(reinterpret_cast<const char*>(&g_enablePlayerLookingDirection), sizeof(g_enablePlayerLookingDirection));
        outFile.write(reinterpret_cast<const char*>(&g_guiOpacity), sizeof(g_guiOpacity));
        outFile.write(reinterpret_cast<const char*>(&g_crosshair), sizeof(g_crosshair));

        // Aimbot settings
        outFile.write(reinterpret_cast<const char*>(&g_AimbotSettings.maximumDistance), sizeof(g_AimbotSettings.maximumDistance));
        outFile.write(reinterpret_cast<const char*>(&g_AimbotSettings.maxSpeed), sizeof(g_AimbotSettings.maxSpeed));
        outFile.write(reinterpret_cast<const char*>(&g_AimbotSettings.minSpeed), sizeof(g_AimbotSettings.minSpeed));
        outFile.write(reinterpret_cast<const char*>(&g_AimbotSettings.aimCircleRadius), sizeof(g_AimbotSettings.aimCircleRadius));
        outFile.write(reinterpret_cast<const char*>(&g_AimbotSettings.deadZoneThreshold), sizeof(g_AimbotSettings.deadZoneThreshold));
        outFile.write(reinterpret_cast<const char*>(&g_AimbotSettings.invisibleAimCircleRadius), sizeof(g_AimbotSettings.invisibleAimCircleRadius));
        outFile.write(reinterpret_cast<const char*>(&g_AimbotSettings.worldDistanceWeight), sizeof(g_AimbotSettings.worldDistanceWeight));

        outFile.write(reinterpret_cast<char*>(&g_assistRecordMP), sizeof(g_assistRecordMP));
        outFile.write(reinterpret_cast<char*>(&g_assistRecordWZ), sizeof(g_assistRecordWZ));
        outFile.write(reinterpret_cast<char*>(&g_damageRecordMP), sizeof(g_damageRecordMP));
        outFile.write(reinterpret_cast<char*>(&g_damageRecordWZ), sizeof(g_damageRecordWZ));
        outFile.write(reinterpret_cast<char*>(&g_damageTakenRecordMP), sizeof(g_damageTakenRecordMP));
        outFile.write(reinterpret_cast<char*>(&g_damageTakenRecordWZ), sizeof(g_damageTakenRecordWZ));
        outFile.write(reinterpret_cast<char*>(&g_pointsRecordMP), sizeof(g_pointsRecordMP));
        outFile.write(reinterpret_cast<char*>(&g_pointsRecordWZ), sizeof(g_pointsRecordWZ));
        outFile.write(reinterpret_cast<char*>(&g_gameCountMP), sizeof(g_gameCountMP));
        outFile.write(reinterpret_cast<char*>(&g_gameCountWZ), sizeof(g_gameCountWZ));
        outFile.write(reinterpret_cast<char*>(&g_avgSkillIndex), sizeof(g_avgSkillIndex));

        if (!g_detectedLocalName.empty())
            detectedNameHash = fnv1aHash(g_detectedLocalName);
        outFile.write(reinterpret_cast<char*>(&detectedNameHash), sizeof(detectedNameHash));
        outFile.write(reinterpret_cast<char*>(&g_accumulatedDamageDone), sizeof(g_accumulatedDamageDone));
        outFile.write(reinterpret_cast<char*>(&g_accumulatedShotsFired), sizeof(g_accumulatedShotsFired));
        outFile.write(reinterpret_cast<char*>(&g_accumulatedShotsMissed), sizeof(g_accumulatedShotsMissed));
        
        outFile.close();
    }
    return 0;
}

void handleEndOfGame(score_t cachedScore, SkillScore cachedSkillScore, bool isWZ)
{
    if (isWZ) // was BR / WZ
    {
        g_killRecordWZ += cachedScore.kills;
        g_deathRecordWZ += cachedScore.deaths;
        g_damageRecordWZ += cachedScore.extraScore4;
        g_damageTakenRecordWZ += cachedScore.extraScore3; // todo not working?
        g_assistRecordWZ += cachedScore.assists;
        g_pointsRecordWZ += cachedScore.points;
        g_gameCountWZ++;
    }
    else
    {
        g_killRecordMP += cachedScore.kills;
        g_deathRecordMP += cachedScore.deaths;
        g_damageRecordMP += cachedScore.mp_damage_done;
        g_damageTakenRecordMP += cachedScore.mp_damage_taken;
        g_assistRecordMP += cachedScore.assists;
        g_pointsRecordMP += cachedScore.points;
        g_gameCountMP++;
    }
    // reset high score and low scores
    g_HighScore = {};
    g_LowScore = {};
    skillScoreList.clear();

}

void calculateSkillScoreInLobby() {
    std::unordered_map<int, SkillScore>  localSkillScore;
    for (int i = 0; i < MAX_PLAYER_COUNT; i++) {
        auto cPlayer = g_playerArr[i];
        auto skillScore = cPlayer.getSkillScore();

        // Find old score
        auto it_old = std::ranges::find_if(skillScoreList, 
                                           [&skillScore](const SkillScore& item) {
                                               return item.playerIndex == skillScore.playerIndex;
                                           });

        // Remove old score if it exists
        SkillScore cached = {};
        if (it_old != skillScoreList.end()) {
            cached = *it_old;
            skillScoreList.erase(it_old);
        }

        // Use cached score if needed
        if (cPlayer.skipPlayer() || cPlayer.score.rank_level == 0)
            skillScore = cached;

        // Re-insert at the correct position to maintain sorted order
        auto it_new = std::ranges::lower_bound(skillScoreList, skillScore,
                                               [](const SkillScore& a, const SkillScore& b) {
                                                   return a.skillIndex > b.skillIndex; // Maintains descending order by skillIndex
                                               });
        if (it_new != skillScoreList.end())
            it_new = skillScoreList.insert(it_new, skillScore);

        // Update placement based on iterator position
        skillScore.placement = std::distance(skillScoreList.begin(), it_new) + 1;

        localSkillScore[cPlayer.entIndex] = skillScore;
    }
    // Update global skill scores with locking for thread safety
    std::unique_lock lock(g_SkillScoresMutex);
    g_SkillScores = localSkillScore;
    lock.unlock();
}