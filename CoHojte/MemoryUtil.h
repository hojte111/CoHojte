#pragma once

#include <type_traits>

template <typename T>
bool searchBuffer(char* buffer, size_t bufferSize, T searchValue, T tolerance = T())
{
    bool found = false;
    size_t typeSize = sizeof(T);

    for (unsigned j = 0; j <= bufferSize - typeSize; ++j)
    {
        T something;
        memcpy(&something, buffer + j, typeSize);

        if constexpr (std::is_floating_point<T>::value)
        {
            T diff = something - searchValue;
            if ((diff < 0 && diff > -tolerance) || (diff > 0 && diff < tolerance)) {
                printf("0x%X -> %.3f\n", j, something);
                found = true;
            }
        }
        else
        {
            if (something == searchValue) {
                //printf("0x%X -> %d\n", j, something);
                printf("0x%X -> %llu\n", j, something);
                found = true;
            }
        }
    }
    return found;
}
