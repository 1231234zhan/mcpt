
#include "global.hpp"
#include "bbox.hpp"
#include "ray.hpp"

BBox::BBox()
{
    min_p = vec3(INFINITY, INFINITY, INFINITY);
    max_p = vec3(-INFINITY, -INFINITY, -INFINITY);
}

void BBox::update(const vec3& p)
{
    for (int i = 0; i < 3; i++) {
        min_p[i] = std::min(min_p[i], p[i]);
        max_p[i] = std::max(max_p[i], p[i]);
    }
}

void BBox::update(const BBox& box)
{
    this->update(box.max_p);
    this->update(box.min_p);
}

// https://raytracing.github.io/books/RayTracingTheNextWeek.html
bool BBox::hit(const Ray& ray, const vec2& t_range, flt& t_hit) const
{
    flt t_min = t_range[0];
    flt t_max = t_range[1];
    for (int a = 0; a < 3; a++) {
        auto invD = 1.0f / ray.direction[a];
        auto t0 = (min_p[a] - ray.origin[a]) * invD;
        auto t1 = (max_p[a] - ray.origin[a]) * invD;
        if (invD < 0.0f)
            std::swap(t0, t1);
        t_min = t0 > t_min ? t0 : t_min;
        t_max = t1 < t_max ? t1 : t_max;
        if (t_max < t_min)
            return false;
    }
    t_hit = t_min;
    return true;
}
