//Vector2.cpp
#include "Vector2.h"

#include <cassert>

Vector2::Vector2()
{
    this->x = 0;
    this->y = 0;
}

Vector2::Vector2(float x, float y)
{
    this->x = x;
    this->y = y;
}

float Vector2::distance(Vector2 vector)
{
    float distance = 0.0f;
    distance += (float)pow(this->x - vector.x, 2);
    distance += (float)pow(this->y - vector.y, 2);
    return (float)sqrt(distance);
}

// Return the squared length of the vector
float Vector2::lengthSquared() { return x*x + y*y; }

// Return the length of the vector
float Vector2::length() { return sqrt(lengthSquared()); }

// Normalize the vector and return it
Vector2 Vector2::normalize() {
    float l = length();
    assert(l > 0);
    x /= l;
    y /= l;
    return *this;
}

Vector2 operator+(Vector2 a, Vector2 b)
{
    return Vector2(a.x + b.x, a.y + b.y);
}

Vector2 operator-(Vector2 a, Vector2 b)
{
    return Vector2(a.x - b.x, a.y - b.y);
}

float operator*(Vector2 a, Vector2 b)
{
    return (a.x * b.x) + (a.y * b.y);
}

Vector2 operator*(Vector2 v, float f) {
    return Vector2(f * v.x, f * v.y);
}

std::ostream& operator<<(std::ostream& os, Vector2 vector)
{
    return os << "Vec2(" << vector.x << ", " << vector.y << ")";
}
