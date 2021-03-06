#pragma once

#include <cstdio>
#include <glm/glm.hpp>
#include <glm/gtx/component_wise.hpp>

#include <random>

#ifdef DEBUG
#define IF_DEBUG 1
#else
#define IF_DEBUG 0
#endif

#define ERRORM(fmt, ...)                            \
    do {                                            \
        fprintf(stderr, "[X] " fmt, ##__VA_ARGS__); \
        exit(-1);                                   \
    } while (false)

#define INFO(fmt, ...)                              \
    do {                                            \
        fprintf(stdout, "[v] " fmt, ##__VA_ARGS__); \
    } while (false)

#define DEBUGM(fmt, ...)                                \
    do {                                                \
        if (IF_DEBUG)                                   \
            fprintf(stdout, "[*] " fmt, ##__VA_ARGS__); \
    } while (false)

typedef unsigned char uchar;
typedef float flt;

typedef glm::vec4 vec4;
typedef glm::vec3 vec3;
typedef glm::vec2 vec2;
typedef glm::mat2 mat2;
typedef glm::mat3 mat3;
typedef glm::mat4 mat4;
typedef glm::ivec4 ivec4;
typedef glm::ivec3 ivec3;
typedef glm::ivec2 ivec2;

const flt kEps = 1e-7;
const flt kZero = 0.0;
const int kChannel = 3;
const flt PI = 3.14159265358979323846f;

const flt kHitEps = 1e-3;
template <typename T>
inline T clamp(T x, T a, T b)
{
    if (a > b)
        ERRORM("Clamp error\n");
    return (x < a) ? a : (b < x) ? b
                                 : x;
}

template <typename T>
inline bool inside(T x, T a, T b)
{
    if (a > b)
        ERRORM("Inside error\n");
    return (a <= x && x <= b) ? true : false;
}

template <typename T>
inline int sign(T x)
{
    return x > 0 ? 1 : (x < 0 ? -1 : 0);
}

template <typename T>
inline T sq(T x)
{
    return x * x;
}

inline flt uniform(flt l = 0.0, flt h = 1.0)
{
    static thread_local std::random_device dev;
    static thread_local std::mt19937 mt(dev());
    std::uniform_real_distribution<flt> rng(l, h);
    return rng(mt);
}