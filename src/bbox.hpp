
#pragma once

#include "global.hpp"

class Ray;

class BBox {
public:
    BBox();
    void update(const vec3& p);
    void update(const BBox& box);
    bool hit(const Ray& ray, const vec2& t_range, flt& t_hit) const ;

public:
    vec3 min_p, max_p;
};