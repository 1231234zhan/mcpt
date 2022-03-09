#pragma once

#include <map>
#include <vector>

#include "bbox.hpp"
#include "global.hpp"
#include "ray.hpp"

class Hittable {
public:
    virtual bool hit(const Ray& ray, const vec2& t_range, HitRecord& rec) const = 0;
    virtual bool bounding_box(BBox& box) const = 0;
};

class Emissive {
public:
    virtual flt get_area() const = 0;

    virtual flt sample_ray(const HitRecord& rec, const Hittable* world, HitRecord& light_rec, vec3& wi) const = 0;
    virtual flt pdf(const Ray& ray, const Hittable* world, HitRecord& light_rec) const = 0;
};

class Triangle : public Hittable, public Emissive {
public:
    Triangle();
    Triangle(const vec3&, const vec3&, const vec3&);

    inline const vec3& p1() const { return this->p[0]; }
    inline const vec3& p2() const { return this->p[1]; }
    inline const vec3& p3() const { return this->p[2]; }

    virtual bool hit(const Ray& ray, const vec2& t_range, HitRecord& rec) const override;
    virtual bool bounding_box(BBox& box) const override;

    virtual flt get_area() const override;

    virtual flt sample_ray(const HitRecord& rec, const Hittable* world, HitRecord& light_rec, vec3& wi) const override;
    virtual flt pdf(const Ray& ray, const Hittable* world, HitRecord& light_rec) const override;

    vec3 sample_point() const;

public:
    vec3 p[3];
    vec2 uv[3];
    vec3 normal;
    BBox box;
    bool has_uv;
    flt area;
    Material* mat;
};

class EmissiveGroup : public Emissive {
public:
    EmissiveGroup();
    EmissiveGroup(const std::vector<Emissive*>& emissive_objects);

    void init(const std::vector<Emissive*>& emissive_objects);

    virtual flt get_area() const override;

    virtual flt sample_ray(const HitRecord& rec, const Hittable* world, HitRecord& light_rec, vec3& wi) const override;
    virtual flt pdf(const Ray& ray, const Hittable* world, HitRecord& light_rec) const override;

public:
    std::vector<flt> sample_sum;
    std::vector<Emissive*> emissive_list;
    std::map<Emissive*, int> emissive_set;
};