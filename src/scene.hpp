#pragma once

#include <string>
#include <vector>

#include "bvh.hpp"
#include "camera.hpp"
#include "global.hpp"
#include "material.hpp"
#include "object.hpp"
#include "ray.hpp"
#include "tiny_obj_loader.h"

class Buffer {
public:
    Buffer();
    Buffer(int width, int height);
    void init(int width, int height);
    void clear();

    void to_picture(const std::string& jpgfile, int sample_num, flt gamma = 2.0) const;

public:
    int width, height;
    std::vector<std::vector<vec3>> b_array;
};

class Scene {
public:
    Scene();
    Scene(const std::string& objdir);
    void render(const std::string& outfile, int num_sample = 30);
    bool hit(const Ray& ray, const vec2& t_range, HitRecord& rec);

    vec3 Li(const Ray& ray, const Hittable* world);
    vec3 sample_light(const HitRecord& rec, const vec3& wo, const Hittable* world);

public:
    std::vector<Hittable*> objects;
    std::vector<Material*> materials;
    Camera camera;
    Buffer buffer;
    Hittable* bvh_root;

    // For light sampling
    EmissiveGroup egroup;
};