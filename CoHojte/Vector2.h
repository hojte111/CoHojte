#pragma once
#include <iostream>

class Vector2
{
public:
    Vector2();
    Vector2(float x, float y);
    float distance(Vector2 vec2);
    float lengthSquared();
    float length();
    Vector2 normalize();

public:
    float x;
    float y;
};

Vector2 operator+(Vector2 a, Vector2 b);
Vector2 operator-(Vector2 a, Vector2 b);
float operator*(Vector2 a, Vector2 b);
Vector2 operator*(Vector2 v, float f);
std::ostream& operator<<(std::ostream& os, Vector2 vector);

