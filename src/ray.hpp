#pragma once

#include "global.hpp"

class Material;
class Hittable;

class Ray {
public:
    Ray() { }
    Ray(const vec3& ori, const vec3& direct)
    {
        origin = ori;
        direction = glm::normalize(direct);
    }

public:
    vec3 origin;
    // Direction vector should be normalized
    vec3 direction;
};

struct HitRecord {
    vec3 p;
    vec3 normal;
    flt t;
    vec2 uv;
    Material* mat;
    // int obj_id;
    Hittable* obj;
};
