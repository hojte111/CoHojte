#pragma once
#pragma once
#include <iostream>

class Vector3
{
public:
    Vector3();
    Vector3(float x, float y, float z);
    float distance(Vector3 vector) const;
    float length() const;
    bool isZero() const;
    float magnitude() const;
    Vector3 normalized() const;
    float dot(const Vector3& other) const;
    Vector3 cross(const Vector3& other) const;

public:
    float x;
    float y;
    float z;
};

Vector3 operator+(Vector3 a, Vector3 b);
Vector3 operator-(Vector3 a, Vector3 b);
float operator*(Vector3 a, Vector3 b);
bool operator==(Vector3 a, Vector3 b);
std::ostream& operator<<(std::ostream& os, Vector3 vector);

class Vector3int
{
public:
    int x;
    int y;
    int z;
    Vector3int();
    Vector3int(int x, int y, int z);
};