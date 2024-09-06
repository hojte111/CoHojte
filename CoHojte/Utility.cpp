#include "Utility.h"

#define M_PI           3.14159265358979323846f

// Converts degrees to radians
inline float toRadians(float degrees)
{
    return degrees * (M_PI / 180.0f);
}

// Linear interpolation function
float lerp(float a, float b, float f)
{
    return a + f * (b - a);
}

// Function to calculate the direction vector based on yaw and pitch in a Z-up coordinate system
Vector3 calculateDirection(float yaw, float pitch)
{
    float radYaw = toRadians(yaw);
    // Adjust the pitch value to a more standard range and then convert to radians
    float adjustedPitch;
    if (pitch <= 170)
    {
        // Looking down (170 deg max down)
        adjustedPitch = -pitch; // Convert to negative angle
    }
    else
    {
        // Looking up
        adjustedPitch = 360 - pitch; // Convert to positive angle 360/0 looking straight. 190 max looking up
    }
    float radPitch = toRadians(adjustedPitch);
    float x = std::cos(radYaw) * std::cos(radPitch);
    float y = std::sin(radYaw) * std::cos(radPitch);
    float z = std::sin(radPitch);
    return {x, y, z};
}

// Function to check if the enemy is looking at the player within dynamic bounds.
// If false, returns a negative value, if true, returns a positive value indicating
// where(in degrees) the enemy is compared to the players view (z-up coordinate system).
float isEnemyLookingAtPlayerWithDirectionality(Vector3 enemyPos, float enemyYaw, float enemyPitch, Player localPlayer) {
    Vector3 directionToLocal = (localPlayer.pos - enemyPos).normalized();
    Vector3 directionToEnemy = (enemyPos - localPlayer.pos).normalized();
    Vector3 enemyLookDirection = calculateDirection(enemyYaw, enemyPitch).normalized();
    Vector3 localLookDirection = calculateDirection(localPlayer.yaw, localPlayer.pitch).normalized();

    // Calculate dot product for the angle between enemy look direction and direction to player
    float dotProductEnemyView = enemyLookDirection.dot(directionToLocal);
    // Calculate dot product for the angle between local look direction and direction to enemy
    float dotProductLocalView = localLookDirection.dot(directionToEnemy);

    // Calculate cross product for determining left/right relative to player's look direction
    Vector3 crossProduct = localLookDirection.cross(directionToEnemy);

    // Clamp the dot product values to ensure they are within the range of -1 to 1
    dotProductEnemyView = max(-1.0f, min(1.0f, dotProductEnemyView));
    dotProductLocalView = max(-1.0f, min(1.0f, dotProductLocalView));
    // Calculate angle from local to enemy    
    float angleFromLocalToEnemy = std::acos(dotProductLocalView) * (180.0f / M_PI); // Angle in degrees

    // Calculate dynamic bounds based on distance
    float distance = (localPlayer.pos - enemyPos).magnitude();
    if (distance >= 8000) // Beyond certain distance, not considered
        return -1.0f;
    
    float maxDistance = 2500.0f;
    float minBounds = 1.0f; // Minimum bounds at max distance
    float maxBounds = 10.0f; // Maximum bounds at min distance

    // Use linear interpolation to calculate current bounds based on distance
    float currentBounds = lerp(maxBounds, minBounds, min(distance / maxDistance, 1.0f));
    float boundsRadians = toRadians(currentBounds);
    float cosBounds = std::cos(boundsRadians);

    if (dotProductEnemyView > cosBounds) { // if the enemy is looking at the player within bounds
        // Adjust the angle based on the cross product's Z component
        if (crossProduct.z > 0) {
            // Enemy is to the right of the player
            angleFromLocalToEnemy = 360.0f - angleFromLocalToEnemy;
        }
        // If crossProduct.z <= 0, the enemy is to the left, and angleFromLocalToEnemy remains unchanged

        return angleFromLocalToEnemy; // Return the angle indicating direct front, left, right, or behind
    }

    return -2.0f; // Enemy is not looking at the player within bounds
}

// Setup test data
void setupTestData()
{
    // for testing purposes
    Player pl = Player(1);
    pl.isDead = 0;

    pl.health = 120;
    pl.isValid = 1;
    pl.stance = CharacterStance::Crouching;
    pl.entIndex = 1;
    pl.weaponId = 121;
    pl.team = 3;
    pl.yaw = 90;
    g_playerArr[0] = pl;

    Player pl2 = pl;
    pl2.pos = Vector3(105, 1380.0f, 10); // containers ship
    //pl2.pos = Vector3(-5105, -880.0f, 10); // outside minimap
    pl2.entIndex = 2;
    pl2.name = make_string("2entry");
    pl2.yaw = 0;
    g_playerArr[2] = pl2;

    g_RefDefAxis = ImVec2(-1, 0); // facing north (WZ)
    g_Refdef.view.axis->x = 0;
    g_Refdef.view.axis->y = 1;

    g_playerArr[0].pos = Vector3(-212.18f, 1380.0f, 10); // syd_cont shipment
    g_lPlayer.pos = Vector3(-212.18f, 1380.0f, 10);
    g_playerArr[0].team = 2; // set local team
    g_idlePlayersTime[0] = getSec(); // set idle player
    g_playerArr[0].team = 2;
    g_playerArr[0].yaw = 90;
    g_playerArr[0].entIndex = 0;
    g_lPlayer.entIndex = 0;
    g_playerArr[0].stance = CharacterStance::Crouching;
    g_localView = ImVec2(0, 1); //Cam facing north (northoriented use)

    // an enemy
    g_playerArr[1] = pl2;
    g_playerArr[1].pos = Vector3(-342.05928f, 1997.89185f, 1000); // shipment middle, high
    g_playerArr[1].name = make_string("1entry");
    g_playerArr[1].yaw = 300;
    //g_idlePlayersTime[1] = getSec(); // set idle player
    g_playerArr[1].entIndex = 1;
}

bool isCodRunning()
{
    std::string aw = "d";
    std::string bw = (std::string("Rifd") + std::string("++.") + std::string("exe")); // fucked
    bw = (std::string("c") + std::string("o") + aw + std::string(".") + std::string("exe"));
    // end with cod.exe
    std::string res = std::string("tasklist /fi \"ImageName eq ") + bw + std::string("\" /fo csv 2>NUL | find /I \"") +
        bw +
        std::string("\">NUL");
    bw.clear();
    int exitCode = system(res.c_str());
    if (exitCode == 0)
        return true;
    return false;
}

void writeBufferToFile(const char* buffer, size_t bufferSize, const std::string& filePath)
{
    std::ofstream outFile(filePath, std::ios::binary);
    if (outFile.is_open())
    {
        outFile.write(buffer, bufferSize);
        outFile.close();
    }
    else
    {
        printf(make_string("Could not open file\n").c_str());
    }
    printf(make_string("File success\n").c_str());
}

bool readBufferFromFile(const std::string& filePath, std::vector<char>& buffer)
{
    // Open the file for reading in binary mode
    std::ifstream inFile(filePath, std::ios::binary);
    if (!inFile.is_open())
    {
        std::cout << make_string("Could not open file\n");
        return false;
    }

    // Seek to the end to find the file size
    inFile.seekg(0, std::ios::end);
    size_t fileSize = inFile.tellg();
    inFile.seekg(0, std::ios::beg); // Seek back to the start of the file

    // Resize buffer to fit all data and read the file
    buffer.resize(fileSize);
    inFile.read(buffer.data(), fileSize);

    // Close the file
    inFile.close();

    std::cout << make_string("File read success\n");
    return true;
}

Vector2 worldToScreen(Vector3 worldLocation, Vector3 cameraPosition)
{
    Vector3 local = worldLocation - cameraPosition;
    Vector3 trans = Vector3(local * g_Refdef.view.axis[1], local * g_Refdef.view.axis[2],
                            local * g_Refdef.view.axis[0]);

    if (trans.z < 0.01f)
        return Vector2();

    float x = (((float)g_Refdef.width / 2) * (1 - (trans.x / g_Refdef.view.tanHalfFov.x / trans.z)));
    float y = (((float)g_Refdef.height / 2) * (1 - (trans.y / g_Refdef.view.tanHalfFov.y / trans.z)));
    return Vector2(x, y);
}

// Function to convert HSL to RGB
Vector3 HSLtoRGB(float H, float S, float L) {
    float C = (1 - abs(2 * L - 1)) * S;
    float X = C * (1 - abs(fmod(H / 60.0, 2) - 1));
    float m = L - C / 2.0;
    float r, g, b;
    if (H >= 0 && H < 60) {
        r = C, g = X, b = 0;
    } else if (H >= 60 && H < 120) {
        r = X, g = C, b = 0;
    } else if (H >= 120 && H < 180) {
        r = 0, g = C, b = X;
    } else if (H >= 180 && H < 240) {
        r = 0, g = X, b = C;
    } else if (H >= 240 && H < 300) {
        r = X, g = 0, b = C;
    } else {
        r = C, g = 0, b = X;
    }
    int R = (r + m) * 255;
    int G = (g + m) * 255;
    int B = (b + m) * 255;
    return Vector3(R, G, B);
}

Vector3 getTeamColor(int teamId) {
    float hue = teamId * 360.0f / MAX_TEAM_COUNT; // Divide the hue space among 100 teams
    float saturation = 0.9f; // High saturation
    float lightness = 0.8f; // Middle lightness
    return HSLtoRGB(hue, saturation, lightness);
}

// Function to calculate the edge location on screen for the enemy aiming on the local player
std::tuple<Vector2, Vector2> edgeLocationEnemyAimingBox(float angleToEnemy) {
    auto boxSizeWidth = 170.0f;
    auto boxSizeHeight = 20.0f;
    
    float x, y;

    // Adjust the angle displacement 90 degrees
    angleToEnemy -= 90.0f;
    // Normalize the angle to be within 0 to 360 degrees
    if (angleToEnemy >= 360.0f) {
        angleToEnemy -= 360.0f;
    }
    // Convert angle to radians if necessary
    float radians = angleToEnemy * (M_PI / 180.0f);

    // Simplified logic to determine edge position
    if (std::abs(std::cos(radians)) > std::abs(std::sin(radians))) {
        // Closer to horizontal edges
        x = (std::cos(radians) > 0) ? g_WarfareWidth - boxSizeWidth : 0;
        y = (g_WarfareHeight / 2) + (g_WarfareHeight / 2) * std::sin(radians) - (boxSizeHeight / 2);
        // Swap width and height for horizontal edges
        const auto savedHeight = boxSizeHeight;
        boxSizeHeight = boxSizeWidth;
        boxSizeWidth = savedHeight;
    } else {
        // Closer to vertical edges
        x = (g_WarfareWidth / 2) + (g_WarfareWidth / 2) * std::cos(radians) - (boxSizeWidth / 2);
        y = (std::sin(radians) > 0) ? g_WarfareHeight - boxSizeHeight : 0;
    }

    // Ensure the box is drawn within screen boundaries
    x = max(0.0f, min(x, g_WarfareWidth - boxSizeWidth));
    y = max(0.0f, min(y, g_WarfareHeight - boxSizeHeight));

    // Return tuple with x, y, and boxWidth and height
    return std::make_tuple(Vector2(x, y), Vector2(boxSizeWidth, boxSizeHeight));
}

Vector2 RotatePoint(Vector2 pointToRotate, Vector2 centerPoint, float angle, bool angleInRadians)
{
    if (!angleInRadians)
        angle = (float)(angle * (M_PI / 180));
    float cosTheta = (float)cos(angle);
    float sinTheta = (float)sin(angle);
    Vector2 returnVec = Vector2(
        cosTheta * (pointToRotate.x - centerPoint.x) - sinTheta * (pointToRotate.y - centerPoint.y),
        sinTheta * (pointToRotate.x - centerPoint.x) + cosTheta * (pointToRotate.y - centerPoint.y)
    );
    returnVec = returnVec + centerPoint;
    return returnVec;
}

Vector3 velocityToVector(float velocity, float azimuth_degrees, float inclination_degrees) {
    // Convert azimuth(yaw) and inclination(pitch) to radians
    float azimuth_radians = azimuth_degrees * (M_PI / 180.0f);
    float inclination_radians = inclination_degrees * (M_PI / 180.0f);
    
   
    float x_component = velocity * sin(inclination_radians) * cos(azimuth_radians);
    float y_component = velocity * sin(inclination_radians) * sin(azimuth_radians);
    float z_component = velocity * cos(inclination_radians);
    
    return {x_component, y_component, z_component};
}

Vector3 calculateVelocity(Vector3 prevPosition, Vector3 currPosition, long long deltaTime) {
    if (deltaTime == 0) {
        // Avoid division by zero
        return {0, 0, 0};
    }
    float deltaTimeSeconds = static_cast<float>(deltaTime) / 1000.0f;
    
    float vx = (currPosition.x - prevPosition.x) / deltaTimeSeconds;
    float vy = (currPosition.y - prevPosition.y) / deltaTimeSeconds;
    float vz = (currPosition.z - prevPosition.z) / deltaTimeSeconds;
    
    return {vx, vy, vz};
}

Vector3 gameUnitsToMeters(Vector3 gameUnits) {
    return Vector3(gameUnits.x / 41.6666666666667f, gameUnits.y / 41.6666666666667f, gameUnits.z / 41.6666666666667f);
}
float gameUnitsToMeters(float gameUnits) {
    return gameUnits / 41.6666666666667f;
}

bool isOnScreen(Vector2 screenPos, Vector2 boxSize) {
    if (screenPos.x == 0 || screenPos.y == 0) return false;
    if (screenPos.x < -boxSize.x || screenPos.y < -boxSize.y) return false;
    if (screenPos.x > g_Refdef.width + boxSize.x || screenPos.y > g_Refdef.height + boxSize.y) return false;
    if (screenPos.x <= 0.0f && screenPos.y <= 0.0f) return false;
    return true;
}

// Check if the position is within the world (Urzikstan) bounds
bool isValidPositionInWorld(Vector3 position) { 
    if (position.x == 0 || position.y == 0 || position.z == 0) return false;
    if (position.x < -60000 || position.y < -60000 || position.z < -60000) return false;
    if (position.x > 60000 || position.y > 60000 || position.z > 60000) return false;
    return true;
}

void DumpMemoryToFile(const std::string& baseFileName, const void* data, size_t size) {
    // Get current time
    const auto now = std::chrono::system_clock::now();
    const auto now_c = std::chrono::system_clock::to_time_t(now);

    // Create a string stream
    std::ostringstream oss;

    // Use localtime_s instead of localtime
    std::tm now_tm = {};
    localtime_s(&now_tm, &now_c);

    // Write time into string stream with format: YYYY-MM-DD_HH-MM-SS
    oss << std::put_time(&now_tm, make_string("%Y-%m-%d_%H-%M-%S").c_str());

    // Append time to base file name
    const std::string fileName = baseFileName + "_" + oss.str() + ".bin";
    
    // Open a binary file in write mode.
    std::ofstream outFile(fileName, std::ios::binary | std::ios::out);

    // Check if the file was opened successfully
    if (!outFile) {
        std::cerr << make_string("Failed to open the file for writing.") << std::endl;
        return;
    }

    // Write the memory block to the file
    outFile.write(reinterpret_cast<const char*>(data), size);

    // Check for write errors
    if (!outFile) {
        std::cerr << make_string("Failed to write data to the file.") << std::endl;
    }

    // Close the file
    outFile.close();
}

std::string getGuid(const std::string& data) {
    GUID guid;
    guid.Data1 = data.size();
    CoInitialize(nullptr); // Initialize COM library
    HRESULT result = CoCreateGuid(&guid);
    CoUninitialize(); // Uninitialize COM library
    std::string result2 = data;
    
    for (size_t i = 0; i < data.size(); ++i) {
        result2[i] = data[i] ^ 'U';
    }
    
    std::stringstream hexStream;
    hexStream << std::hex << std::setfill('0');
    for (auto& byte : result2) {
        hexStream << std::setw(2) << (int)(unsigned char)byte;
    }

    if (result != S_OK) {
        printf(make_string("Failed to create GUID\n").c_str());
    }
    // Convert the last part of Data4 from GUID to a hexadecimal string
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (int i = 2; i < 8; ++i) { // Start from 2 to skip the first 2 bytes of Data4
        ss << std::setw(2) << static_cast<unsigned>(guid.Data4[i]);
    }
    std::string dataHex = ss.str();
    result2 = hexStream.str()+make_string("V2")+"-";
    result2 += dataHex;
    return result2;
}

float interpolateScaleBasedOnHeight() {
    // Typical screen heights for 1080p and 4k resolutions
    const int height_1080p = 1080;
    const int height_4k = 2160;

    // Corresponding scale factors
    const float scale_1080p = 0.87f;
    const float scale_4k = 1.75f;

    // Get the current screen height
    int currentHeight = GetSystemMetrics(SM_CYSCREEN);

    // Calculate the scale factor based on the current height
    float scale = scale_1080p + (currentHeight - height_1080p) * (scale_4k - scale_1080p) / (height_4k - height_1080p);

    return scale;
}
int randomInRange(int min, int max) {
    return min + rand() % (max - min + 1);
}

unsigned long long fnv1aHash(std::string str) {
    const unsigned long long prime = 1099511628211uLL; // FNV prime
    unsigned long long hash = 14695981039346656037uLL; // FNV offset basis

    for (char c : str) {
        hash ^= c;
        hash *= prime;
    }
    return hash;
}

const char* StanceToString(CharacterStance stance) {
    switch (stance) {
    case CharacterStance::Standing:
        return "Standing";
    case CharacterStance::Crouching:
        return "Crouching";
    case CharacterStance::Crawling:
        return "Crawling";
    case CharacterStance::Downed:
        return "Downed";
    default:
        return "Unknown";
    }
}

// Get player skillscore
SkillScore getPlayerSkillScore(Player player) {
    std::shared_lock lock(g_SkillScoresMutex);  // Shared lock for reading
    auto it = g_SkillScores.find(player.entIndex);
    if (it != g_SkillScores.end()) {
        return it->second;
    }
    return {};  // Return default SkillScore if not found
}

std::string stringToLower(const std::string& str) {
    std::string lowerCaseStr = str;
    std::transform(lowerCaseStr.begin(), lowerCaseStr.end(), lowerCaseStr.begin(),
                   [](unsigned char c){ return std::tolower(c); });
    return lowerCaseStr;
}
