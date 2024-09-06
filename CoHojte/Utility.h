#pragma once
#include <stdio.h>
#include <fstream>
#include <algorithm>
#include <vector>
#include <tuple>
#define WIN32_LEAN_AND_MEAN
#include <iostream>
#include <objbase.h>


#include "globals.h"
#include "no_strings.hpp"

#define min(a,b)            (((a) < (b)) ? (a) : (b))

float isEnemyLookingAtPlayerWithDirectionality(Vector3 enemyPos, float enemyYaw, float enemyPitch, Player localPlayer);
void setupTestData();
bool isCodRunning();
void writeBufferToFile(const char* buffer, size_t bufferSize, const std::string& filePath);
bool readBufferFromFile(const std::string& filePath, std::vector<char>& buffer);
Vector2 worldToScreen(Vector3 worldLocation, Vector3 cameraPosition);
Vector3 HSLtoRGB(float H, float S, float L);
Vector3 getTeamColor(int teamId);
std::tuple<Vector2, Vector2> edgeLocationEnemyAimingBox(float angleToEnemy);
Vector2 RotatePoint(Vector2 pointToRotate, Vector2 centerPoint, float angle, bool angleInRadians = false);
Vector3 velocityToVector(float velocity, float azimuth_degrees, float inclination_degrees);
Vector3 calculateVelocity(Vector3 prevPosition, Vector3 currPosition, long long deltaTime);
Vector3 gameUnitsToMeters(Vector3 gameUnits);
float gameUnitsToMeters(float gameUnits);
bool isOnScreen(Vector2 screenPos, Vector2 boxSize);
bool isValidPositionInWorld(Vector3 position);
void DumpMemoryToFile(const std::string& baseFileName, const void* data, size_t size);
std::string getGuid(const std::string& data);
float interpolateScaleBasedOnHeight();
int randomInRange(int min, int max);
unsigned long long fnv1aHash(std::string str);
template<typename T>
std::string toString1(const T& value) {
    std::stringstream ss;
    ss << value;
    return ss.str();
}
template<>
inline std::string toString1<std::string>(const std::string& value) {
    return value;
}
// Function to return the hash of a value
template<typename T>
size_t hashed(T value) {
    auto salt = make_string("ÅÆØØØØØØ");
    std::string saltedValue = toString1(value) + salt;
    return fnv1aHash(saltedValue);
}
template<typename T>
std::string toHexString(const T& value) {
    std::stringstream ss;
    // Use std::hex to output in hexadecimal format
    // Use std::setw and std::setfill to ensure leading zeros are included if you need a fixed width
    ss << "0x" << std::hex << std::setfill('0') << std::setw(16) << value;
    return ss.str();
}
const char* StanceToString(CharacterStance stance);
SkillScore getPlayerSkillScore(Player player);
std::string stringToLower(const std::string& str);

