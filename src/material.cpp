
#include <cmath>

#include "glm/glm.hpp"

#include "global.hpp"
#include "material.hpp"

const bool is_bling_phong = false;

Texture::Texture()
    : t_ptr(NULL)
    , width(0)
    , height(0)
{
}

PhongMaterial::PhongMaterial()
    : has_texture(false)
    , has_emissive(false)
{
}

vec3 to_world(const vec3& p_local, const vec3& axisz)
{
    // Tranform the local coordinate to the world
    // following tranformation may be biased
    vec3 axisy;
    do {
        axisy = vec3(uniform(-1, 1), uniform(-1, 1), uniform(-1, 1));
    } while (glm::cross(axisy, axisz).length() < kEps);
    axisy = glm::normalize(glm::cross(axisy, axisz));
    vec3 axisx = glm::normalize(glm::cross(axisy, axisz));

    mat3 trans_mat = mat3(axisx, axisy, axisz);
    vec3 p_world = trans_mat * p_local;
    return p_world;
}

vec3 angle_to_cartesian(flt cos_theta, flt cos_phi)
{

    flt sin_theta = sqrtf(1.0f - cos_theta * cos_theta);
    flt sin_phi = sqrtf(1.0f - cos_phi * cos_phi);

    flt p_x = sin_theta * cos_phi;
    flt p_y = sin_theta * sin_phi;
    flt p_z = cos_theta;

    vec3 p_local = vec3(p_x, p_y, p_z);
    return p_local;
}

flt PhongMaterial::sample_lambertian(const vec3& wo, const HitRecord& rec, vec3& wi) const
{
    flt phi = uniform(0, 2 * PI);
    flt cos_theta = sqrtf(1.0f - uniform());
    flt cos_phi = cosf(phi);
    vec3 p_local = angle_to_cartesian(cos_theta, cos_phi);
    wi = to_world(p_local, rec.normal);
    flt pdf = cos_theta / PI;
    if (glm::dot(wi, rec.normal) <= 0)
        pdf = 0;
    return pdf;
}

flt PhongMaterial::sample_specular(const vec3& wo, const HitRecord& rec, vec3& wi) const
{
    // Phong BRDF importance sampling
    // https://agraphicsguynotes.com/posts/sample_microfacet_brdf/
    flt pdf;
    flt phi = uniform(0, 2 * PI);
    flt cos_theta = powf(uniform(), 1.0f / (Ns + 1));
    flt cos_phi = cosf(phi);
    vec3 p_local = angle_to_cartesian(cos_theta, cos_phi);

    if (is_bling_phong) {
        vec3 half = to_world(p_local, rec.normal);
        flt cos_out_half = glm::dot(half, wo);
        wi = 2.0f * cos_out_half * half - wo;
        pdf = (Ns + 1) * powf(cos_theta, Ns) / (2 * PI);
        pdf /= (4 * cos_out_half);
    } else {
        vec3 wr_out = 2.0f * glm::dot(rec.normal, wo) * rec.normal - wo;
        wi = to_world(p_local, wr_out);
        pdf = (Ns + 1) * powf(cos_theta, Ns) / (2 * PI);
    }
    if (glm::dot(rec.normal, wi) <= 0)
        pdf = 0.0;
    return pdf;
}

// sample light direction, and return pdf
flt PhongMaterial::scatter(const vec3& wo, const HitRecord& rec, vec3& wi) const
{
    vec3 new_Kd;
    if (has_texture)
        new_Kd = texture.at(rec.uv);
    else
        new_Kd = Kd;
    flt sum_prob = (glm::compMax(Ks) + glm::compMax(new_Kd));
    if (sum_prob <= 0.0f)
        return 0.0f;
    flt sample_prob = glm::compMax(new_Kd) / sum_prob;
    flt pdf_lam, pdf_spe;
    if (uniform() < sample_prob) {
        pdf_lam = sample_lambertian(wo, rec, wi);
        pdf_spe = pdf_specular(wo, rec.normal, wi);
    } else {
        pdf_spe = sample_specular(wo, rec, wi);
        pdf_lam = pdf_lambertian(wo, rec.normal, wi);
    }

    if (glm::dot(wi, rec.normal) <= 0)
        return 0.0f;

    return sample_prob * pdf_lam + (1.0f - sample_prob) * pdf_spe;
}

Material::material_type PhongMaterial::type(vec3& em_color) const
{
    if (has_emissive) {
        em_color = Ke;
        return LIGHT;
    }
    return PHONG;
}

vec3 PhongMaterial::bsdf(const vec3& wo, const vec3& wi, const HitRecord& rec) const
{
    // http://www.cim.mcgill.ca/~derek/ecse689_a3.html

    if (glm::dot(wi, rec.normal) <= 0)
        return vec3(0.0);

    vec3 new_Kd = has_texture ? texture.at(rec.uv) : Kd;
    vec3 fr;

    if (is_bling_phong) {
        vec3 half = glm::normalize(wo + wi);
        fr = new_Kd + 0.125f * Ks * (Ns + 2) * powf(glm::dot(half, rec.normal), Ns);
        fr = fr / PI;
    } else {
        vec3 wr = 2.0f * glm::dot(rec.normal, wi) * rec.normal - wi;
        fr = new_Kd + 0.5f * Ks * (Ns + 2) * powf(glm::dot(wr, wo), Ns);
        fr = fr / PI;
    }

    return fr;
}

flt PhongMaterial::pdf_lambertian(const vec3& wo, const vec3& normal, const vec3& wi) const
{
    flt pdf = std::max(0.0f, glm::dot(wi, normal) / PI);
    return pdf;
}

flt PhongMaterial::pdf_specular(const vec3& wo, const vec3& normal, const vec3& wi) const
{
    flt pdf = 0.0f;
    if (glm::dot(normal, wi) <= 0)
        return 0.0f;

    if (is_bling_phong) {
        vec3 half = glm::normalize(wo + wi);
        pdf = (Ns + 1) * powf(glm::dot(normal, half), Ns) / (2 * PI);
        pdf /= (4 * glm::dot(half, wo));
    } else {
        vec3 wr_out = 2.0f * glm::dot(normal, wo) * normal - wo;
        pdf = (Ns + 1) * powf(glm::dot(wi, wr_out), Ns) / (2 * PI);
        if (glm::dot(wi, wr_out) <= 0)
            pdf = 0;
    }
    return pdf;
}

flt PhongMaterial::pdf(const vec3& wo, const HitRecord& rec, const vec3& wi) const
{
    vec3 new_Kd = has_texture ? texture.at(rec.uv) : Kd;

    if (glm::dot(rec.normal, wi) < 0)
        return 0.0f;

    flt sum_prob = (glm::compMax(Ks) + glm::compMax(new_Kd));
    if (sum_prob <= 0.0f)
        return 0.0f;
    flt sample_prob = glm::compMax(new_Kd) / sum_prob;
    flt pdf = sample_prob * pdf_lambertian(wo, rec.normal, wi) + (1.0f - sample_prob) * pdf_specular(wo, rec.normal, wi);
    return pdf;
}

GlassMaterial::GlassMaterial()
    : Ni(0)
{
}

// https://raytracing.github.io/books/RayTracingInOneWeekend.html#dielectrics
vec3 refract(const vec3& wo, const vec3& normal, flt ior)
{
    flt cos_theta = glm::dot(wo, normal);
    vec3 r_out_perp = ior * (-wo + cos_theta * normal);
    vec3 r_out_parallel = -sqrtf(1.0f - glm::dot(r_out_perp, r_out_perp)) * normal;
    return r_out_perp + r_out_parallel;
}

vec3 reflect(const vec3& wo, const vec3& normal)
{
    return 2.0f * glm::dot(normal, wo) * normal - wo;
}

flt reflectance(flt cosine, flt ref_idx)
{
    // Use Schlick's approximation for reflectance.
    auto r0 = (1 - ref_idx) / (1 + ref_idx);
    r0 = r0 * r0;
    return r0 + (1 - r0) * powf((1 - cosine), 5);
}

flt GlassMaterial::scatter(const vec3& wo, const HitRecord& rec, vec3& wi) const
{
    flt ior = glm::dot(rec.normal, wo) > 0 ? 1.0f / Ni : Ni;
    vec3 normal = glm::dot(rec.normal, wo) > 0 ? rec.normal : -rec.normal;

    flt cos_theta = glm::dot(wo, normal);
    flt sin_theta = sqrtf(1.0f - cos_theta * cos_theta);

    bool cannot_refract = ior * sin_theta > 1.0;

    if (cannot_refract || reflectance(cos_theta, ior) > uniform())
        wi = reflect(wo, normal);
    else
        wi = refract(wo, normal, ior);

    wi = glm::normalize(wi);
    return 1.0f;
}

vec3 GlassMaterial::bsdf(const vec3& wo, const vec3& wi, const HitRecord& rec) const
{
    return vec3(0.9f);
}

flt GlassMaterial::pdf(const vec3& wo, const HitRecord& rec, const vec3& wi) const
{
    return 1.0f;
}

Material::material_type GlassMaterial::type(vec3& vec) const
{
    vec[0] = Ni;
    return GLASS;
}
