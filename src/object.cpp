#include <algorithm>
#include <cmath>
#include <map>
#include <vector>

#include "glm/glm.hpp"

#include "global.hpp"
#include "material.hpp"
#include "object.hpp"
#include "ray.hpp"

Triangle::Triangle()
    : has_uv(false)
    , mat(NULL)
    , area(0)
{
}

Triangle::Triangle(const vec3& p1, const vec3& p2, const vec3& p3)
    : has_uv(false)
    , mat(NULL)
{
    p[0] = p1;
    p[1] = p2;
    p[2] = p3;

    normal = glm::normalize(glm::cross(p[1] - p[0], p[2] - p[0]));
    for (int i = 0; i < 3; i++) {
        box.update(p[i]);
    }

    area = fabsf(glm::length(glm::cross(p[1] - p[0], p[2] - p[0])));
}

// Möller–Trumbore intersection algorithm
// code from https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm
bool Triangle::hit(const Ray& ray, const vec2& t_range, HitRecord& rec) const
{
    vec3 vertex0 = p[0];
    vec3 vertex1 = p[1];
    vec3 vertex2 = p[2];
    vec3 edge1, edge2, h, s, q;
    flt a, f, u, v, w;
    edge1 = vertex1 - vertex0;
    edge2 = vertex2 - vertex0;
    h = glm::cross(ray.direction, edge2);
    a = glm::dot(edge1, h);
    if (a > -kEps && a < kEps)
        return false; // This ray is parallel to this triangle.
    f = 1.0f / a;
    s = ray.origin - vertex0;
    u = f * glm::dot(s, h);
    if (u < 0.0f || u > 1.0f)
        return false;
    q = glm::cross(s, edge1);
    v = f * glm::dot(ray.direction, q);
    if (v < 0.0f || u + v > 1.0f)
        return false;
    w = 1.0f - u - v;
    // At this stage we can compute t to find out where the intersection point is on the line.
    float t = f * glm::dot(edge2, q);
    if (t > t_range[0] && t < t_range[1]) // ray intersection
    {
        rec.p = ray.origin + ray.direction * t;
        rec.t = t;
        rec.normal = normal;
        rec.uv = uv[0] * w + uv[1] * u + uv[2] * v;
        rec.mat = mat;
        rec.obj = const_cast<Triangle*>(this);
        return true;
    } else // This means that there is a line intersection but not a ray intersection.
        return false;
}

bool Triangle::bounding_box(BBox& box) const
{
    box = this->box;
    return true;
}

// https://math.stackexchange.com/questions/538458/how-to-sample-points-on-a-triangle-surface-in-3d
vec3 Triangle::sample_point() const
{
    flt sqrt_a = sqrtf(uniform());
    flt b = uniform();
    vec3 sample_p = p[0] * (1.0f - sqrt_a) + p[1] * (sqrt_a * (1 - b)) + p[2] * (sqrt_a * b);
    return sample_p;
}

flt Triangle::get_area() const
{
    return area;
}


flt Triangle::sample_ray(const HitRecord& rec, const Hittable* world, HitRecord& light_rec, vec3& wi) const
{
    vec3 sample_p = this->sample_point();
    Ray light_ray(rec.p, sample_p - rec.p);
    flt pdf = 0.0f;

    wi = light_ray.direction;

    if (world->hit(light_ray, vec2(kHitEps, INFINITY), light_rec)
        && glm::dot(light_rec.p - sample_p, light_rec.p - sample_p) < kEps
        && glm::dot(rec.normal, wi) > 0
        && glm::dot(light_rec.normal, wi) < 0) {

        vec3 distance = rec.p - light_rec.p;
        flt area = this->get_area();
        pdf = glm::dot(distance, distance) / (area * glm::dot(-wi, light_rec.normal));
    }
    return pdf;
}

// Maybe useless
flt Triangle::pdf(const Ray& ray, const Hittable* world, HitRecord& light_rec) const
{
    flt pdf = 0.0f;

    if (world->hit(ray, vec2(kHitEps, INFINITY), light_rec)
        && light_rec.obj == this
        && glm::dot(light_rec.normal, ray.direction) < 0) {

        vec3 distance = ray.origin - light_rec.p;
        flt area = this->get_area();
        pdf = glm::dot(distance, distance) / (area * glm::dot(-ray.direction, light_rec.normal));
    }
    return pdf;
}

EmissiveGroup::EmissiveGroup()
{
}

EmissiveGroup::EmissiveGroup(const std::vector<Emissive*>& emissive_objects)
{
    this->init(emissive_objects);
}

void EmissiveGroup::init(const std::vector<Emissive*>& emissive_objects)
{
    emissive_list.assign(emissive_objects.begin(), emissive_objects.end());
    DEBUGM("emissive group size: %d\n", emissive_list.size());

    // each emissive has the same probability to be sampled
    for (int i = 0; i < emissive_list.size(); i++) {
        sample_sum.push_back((1.0f * (i + 1)));
        emissive_set.insert(std::make_pair(emissive_list[i], i));
    }
}

flt EmissiveGroup::get_area() const
{
    flt area = 0.0f;
    for (auto emi : emissive_list) {
        area += emi->get_area();
    }
    return area;
}

// sample ray in emissive list, each emissive has the same probability
flt EmissiveGroup::sample_ray(const HitRecord& rec, const Hittable* world, HitRecord& light_rec, vec3& wi) const
{
    flt a = uniform() * sample_sum[sample_sum.size() - 1];
    int id = std::lower_bound(sample_sum.begin(), sample_sum.end(), a) - sample_sum.begin();
    Emissive* sample_emissive = emissive_list[id];

    flt pdf = sample_emissive->sample_ray(rec, world, light_rec, wi);
    pdf *= (id == 0 ? sample_sum[id] : sample_sum[id] - sample_sum[id - 1]) / sample_sum[sample_sum.size() - 1];
    return pdf;
}

flt EmissiveGroup::pdf(const Ray& ray, const Hittable* world, HitRecord& light_rec) const
{
    flt pdf = 0.0f;

    // TODO: ugly code
    if (world->hit(ray, vec2(kHitEps, INFINITY), light_rec)) {
        auto hit_emi_obj = dynamic_cast<Triangle*>(light_rec.obj);

        if (emissive_set.count(hit_emi_obj)
            && glm::dot(light_rec.normal, ray.direction) < 0) {

            vec3 distance = ray.origin - light_rec.p;
            flt area = hit_emi_obj->get_area();
            pdf = glm::dot(distance, distance) / (area * glm::dot(-ray.direction, light_rec.normal));
            int id = emissive_set.find(hit_emi_obj)->second;
            pdf *= (id == 0 ? sample_sum[id] : sample_sum[id] - sample_sum[id - 1]) / sample_sum[sample_sum.size() - 1];
        }
    }
    return pdf;
}
