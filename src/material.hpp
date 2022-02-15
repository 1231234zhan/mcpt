#pragma once

#include <string>

#include "global.hpp"
#include "ray.hpp"

class Material {
public:
    enum material_type {
        PHONG,
        LIGHT,
        GLASS
    };

public:
    virtual flt scatter(const vec3& wo, const HitRecord& rec, vec3& wi) const = 0;
    virtual flt pdf(const vec3& wo, const HitRecord& rec, const vec3& wi) const = 0;
    virtual vec3 bsdf(const vec3& wo, const vec3& wi, const HitRecord& rec) const = 0;
    virtual material_type type(vec3& vec) const = 0;
};

class Texture {
public:
    Texture();
    inline vec3 at(int x, int y) const
    {
        int id = kChannel * ((y % height + height) % height * width + (x % width + width) % width);
        return vec3(t_ptr[id + 0], t_ptr[id + 1], t_ptr[id + 2]);
    }

    inline vec3 at(flt x, flt y) const
    {
        return this->at(static_cast<int>(round(x)), static_cast<int>(round(y)));
    }

    inline vec3 at(const vec2& uv) const
    {
        return this->at((width)*uv[0], (height) * (1.0f - uv[1]));
    }

public:
    int width, height;
    float* t_ptr;
};

class PhongMaterial : public Material {
public:
    PhongMaterial();

    virtual flt scatter(const vec3& wo, const HitRecord& rec, vec3& wi) const override;
    virtual vec3 bsdf(const vec3& wo, const vec3& wi, const HitRecord& rec) const override;
    virtual material_type type(vec3& vec) const override;
    virtual flt pdf(const vec3& wo, const HitRecord& rec, const vec3& wi) const override;

    flt sample_lambertian(const vec3& wo, const HitRecord& rec, vec3& wi) const;
    flt sample_specular(const vec3& wo, const HitRecord& rec, vec3& wi) const;
    flt pdf_lambertian(const vec3& wo, const vec3& normal, const vec3& wi) const;
    flt pdf_specular(const vec3& wo, const vec3& normal, const vec3& wi) const;

public:
    vec3 Kd;
    vec3 Ks;
    vec3 Ke; // light emitted by the material
    flt Ns;

    bool has_texture;
    bool has_emissive;
    Texture texture;
};

class GlassMaterial : public Material {
public:
    GlassMaterial();

    virtual flt scatter(const vec3& wo, const HitRecord& rec, vec3& wi) const override;
    virtual vec3 bsdf(const vec3& wo, const vec3& wi, const HitRecord& rec) const override;
    virtual material_type type(vec3& vec) const override;
    virtual flt pdf(const vec3& wo, const HitRecord& rec, const vec3& wi) const override;

public:
    flt Ni;
};