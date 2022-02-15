#pragma once

#include <vector>

#include "bbox.hpp"
#include "global.hpp"
#include "object.hpp"

class Ray;

class BVHnode : public Hittable {
public:
    BVHnode();
    BVHnode(const BBox& input_box);
    BVHnode(const Hittable* object);
    virtual bool hit(const Ray& ray, const vec2& t_range, HitRecord& rec) const override;
    virtual bool bounding_box(BBox& box) const override;

public:
    Hittable* child[2];
    BBox box;
};

Hittable* build_BVH(std::vector<Hittable*>& objects, int i_begin, int i_end);
