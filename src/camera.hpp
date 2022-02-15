#pragma once

#include "tinyxml2.h"

#include "global.hpp"
#include "ray.hpp"

class Camera {
public:
    Camera();
    void init(const tinyxml2::XMLDocument& xmlconfig);
    Ray cast_ray(int x, int y);

public:
    vec3 position;
    vec3 up;
    vec3 lookat;
    flt fovy;
    flt aspect;

    int width, height;
    vec3 dw, dh;
    vec3 left_top_corner;
};