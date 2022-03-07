#include <algorithm>
#include <cmath>
#include <cstdio>
#include <map>
#include <string>
#include <vector>

#include <omp.h>

#define STB_IMAGE_IMPLEMENTATION
#include "glm/glm.hpp"
#include "stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"
#include "tinyxml2.h"

// #define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include "bvh.hpp"
#include "camera.hpp"
#include "global.hpp"
#include "scene.hpp"

Buffer::Buffer()
{
}

Buffer::Buffer(int w, int h)
{
    this->init(w, h);
}

void Buffer::init(int w, int h)
{
    this->width = w;
    this->height = h;
    b_array.resize(h);
    for (auto& col : b_array) {
        col.resize(w);
    }
}

void Buffer::clear()
{
    vec3 black = vec3(0, 0, 0);
    for (int y_t = 0; y_t < height; y_t++) {
        for (int x_t = 0; x_t < width; x_t++) {
            b_array[y_t][x_t] = black;
        }
    }
}

void Buffer::to_picture(const std::string& jpgfile, int sample_num, flt gamma) const
{
    uchar* img = new uchar[height * width * kChannel];
    int pt = 0;
    for (int y_t = 0; y_t < height; y_t++) {
        for (int x_t = 0; x_t < width; x_t++) {
            vec3 color = b_array[y_t][x_t] / (sample_num * 1.0f);

            // Check if color is in range
            for (int i = 0; i < 3; i++) {
                if (color[i] >= 1.0) {
                    color[i] = 1.0;
                }
                if (color[i] < 0) {
                    ERRORM("Color %d %d %d is negative %f\n", y_t, x_t, i, color[i]);
                }
            }
            // Gamma correction
            for (int i = 0; i < 3; i++) {
                color[i] = powf(color[i], 1 / gamma);
            }

            for (int i = 0; i < 3; i++) {
                img[pt + i] = static_cast<uchar>(color[i] * 255);
            }
            pt = pt + 3;
        }
    }

    stbi_write_jpg(jpgfile.c_str(), width, height, kChannel, img, 100);
    delete[] img;
}

void read_objfile(const std::string inputfile, tinyobj::ObjReader& objreader)
{
    tinyobj::ObjReaderConfig objreader_config;

    if (!objreader.ParseFromFile(inputfile + ".obj", objreader_config)) {
        if (!objreader.Error().empty()) {
            ERRORM("TinyObjReader %s\n", objreader.Error().c_str());
        }
        ERRORM("TinyObjReader error\n");
    }

    if (!objreader.Warning().empty()) {
        INFO("TinyObjReader %s\n", objreader.Warning().c_str());
    }
}

void read_materials(const std::vector<tinyobj::material_t>& materi_list,
    const tinyxml2::XMLDocument& xmlconfig,
    std::vector<Material*>& mat_ptr_list)
{
    // Obtain all the light arguments in xml file
    std::map<std::string, vec3> light_map;
    auto light_xmlnode = xmlconfig.FirstChildElement("light");
    if (!light_xmlnode) {
        ERRORM("No light source found in xml file\n");
    }
    while (light_xmlnode) {
        auto foo = light_xmlnode->Attribute("mtlname");
        if (!foo) {
            ERRORM("light has no attribute name \"mtlname\"\n");
        }
        std::string mtlname(foo);
        auto radiance_str = light_xmlnode->Attribute("radiance");
        if (!radiance_str) {
            ERRORM("light has no attribute name \"radiance\"\n");
        }
        vec3 radiance;
        // radiance type: float (flt)
        if (sscanf(radiance_str, "%f,%f,%f", &radiance[0], &radiance[1], &radiance[2]) != 3) {
            ERRORM("cannot read 3 floats in radiance attribute\n");
        }

        light_map.insert(std::make_pair(mtlname, radiance));
        DEBUGM("light: %s  radiance: %f %f %f\n", mtlname.c_str(), radiance[0], radiance[1], radiance[2]);

        light_xmlnode = light_xmlnode->NextSiblingElement("light");
    }

    // loop over all the materials in mtl file
    for (const auto& materi_tinyobj : materi_list) {
        // glassmaterial
        if (materi_tinyobj.ior > 1.0f) {
            GlassMaterial* material = new GlassMaterial();
            material->Ni = materi_tinyobj.ior;
            DEBUGM("material name: %s    Ni: %f\n", materi_tinyobj.name.c_str(), material->Ni);
            mat_ptr_list.push_back(static_cast<Material*>(material));
            continue;
        }

        // phongmaterial
        PhongMaterial* material = new PhongMaterial();

        // diffuse
        for (int i = 0; i < 3; i++) {
            material->Kd[i] = materi_tinyobj.diffuse[i];
        }

        // specular
        for (int i = 0; i < 3; i++) {
            material->Ks[i] = materi_tinyobj.specular[i];
        }

        // brightness and refraction
        material->Ns = materi_tinyobj.shininess;

        // if a material give out light
        if (light_map.count(materi_tinyobj.name)) {
            vec3 emissive = light_map[materi_tinyobj.name];
            material->Ke = emissive;
            material->has_emissive = true;
        }

        // if a material has texture
        // TODO: load picture path
        if (materi_tinyobj.diffuse_texname.length() > 0) {
            material->has_texture = true;
            auto& texture = material->texture;
            int load_channel;
            texture.t_ptr = stbi_loadf(materi_tinyobj.diffuse_texname.c_str(), &(texture.width), &(texture.height), &load_channel, kChannel);
            if (!texture.t_ptr) {
                ERRORM("Cannot load texture %s\n", materi_tinyobj.diffuse_texname.c_str());
            }
            if (load_channel != 3) {
                ERRORM("The channel of input texture is not equal to 3\n");
            }
        }

        DEBUGM("material name: %s  Ns: %f  width: %d  height: %d\n",
            materi_tinyobj.name.c_str(), material->Ns, material->texture.width, material->texture.height);

        mat_ptr_list.push_back(static_cast<Material*>(material));
    }
    DEBUGM("material num: %d\n", mat_ptr_list.size());
}

void save_obj_and_mat(const std::vector<tinyobj::shape_t>& shapes,
    const tinyobj::attrib_t& attrib,
    const std::vector<Material*>& materials,
    std::vector<Hittable*>& objects,
    std::vector<Emissive*>& light_objects)
{
    // Code from https://github.com/tinyobjloader/tinyobjloader
    DEBUGM("mtl num: %d\n", shapes.size());
    for (size_t s = 0; s < shapes.size(); s++) {
        size_t point_index_offset = 0;
        DEBUGM("faces num: %d\n", shapes[s].mesh.num_face_vertices.size());
        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
            size_t fv = shapes[s].mesh.num_face_vertices[f];
            if (fv != 3) {
                ERRORM("Polygon not triangulated at object %zu face %zu\n", s, f);
            }
            vec3 p[3];
            vec3 normal[3];
            vec2 uv[3];
            bool has_uv = false;
            bool has_normal = false;
            for (size_t v = 0; v < fv; v++) {
                tinyobj::index_t idx = shapes[s].mesh.indices[point_index_offset + v];
                if (3 * size_t(idx.vertex_index) + 2 >= attrib.vertices.size())
                    ERRORM("vectices id exceed %d\n", 3 * size_t(idx.vertex_index) + 2);
                tinyobj::real_t vx = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
                tinyobj::real_t vy = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
                tinyobj::real_t vz = attrib.vertices[3 * size_t(idx.vertex_index) + 2];
                p[v] = vec3(vx, vy, vz);

                if (idx.normal_index >= 0) {
                    has_normal = true;
                    if (3 * size_t(idx.normal_index) + 2 >= attrib.normals.size())
                        ERRORM("normal id exceed %d\n", 3 * size_t(idx.normal_index) + 2);

                    tinyobj::real_t nx = attrib.normals[3 * size_t(idx.normal_index) + 0];
                    tinyobj::real_t ny = attrib.normals[3 * size_t(idx.normal_index) + 1];
                    tinyobj::real_t nz = attrib.normals[3 * size_t(idx.normal_index) + 2];
                    normal[v] = vec3(nx, ny, nz);
                }

                if (idx.texcoord_index >= 0) {
                    has_uv = true;
                    if (2 * size_t(idx.texcoord_index) + 1 >= attrib.texcoords.size())
                        ERRORM("texcoord id exceed %d\n", 2 * size_t(idx.texcoord_index) + 1);

                    tinyobj::real_t tx = attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
                    tinyobj::real_t ty = attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];
                    uv[v] = vec2(tx, ty);
                }
            }
            point_index_offset += fv;

            Triangle* tri = new Triangle(p[0], p[1], p[2]);
            if (has_normal && glm::dot(tri->normal, normal[0]) < 0)
                tri->normal = -tri->normal;

            tri->has_uv = has_uv;
            if (has_uv) {
                for (int i = 0; i < 3; i++) {
                    tri->uv[i] = uv[i];
                }
            }

            int material_id = shapes[s].mesh.material_ids[f];
            if (material_id >= materials.size())
                ERRORM("material_id exceed %d\n", material_id);
            tri->mat = materials[material_id];
            objects.push_back(static_cast<Hittable*>(tri));

            // light samping
            vec3 foo;
            if (tri->mat->type(foo) == Material::LIGHT) {
                light_objects.push_back(static_cast<Emissive*>(tri));
            }
        }
    }
}

Scene::Scene(const std::string& objdir)
{
    tinyobj::ObjReader reader;
    read_objfile(objdir, reader);

    tinyxml2::XMLDocument xmlconfig;

    if (xmlconfig.LoadFile((objdir + ".xml").c_str()) != 0) {
        ERRORM("xml file error code %d\n", xmlconfig.ErrorID());
    }

    auto& shapes = reader.GetShapes();
    auto& attrib = reader.GetAttrib();
    auto& materi_list = reader.GetMaterials();

    read_materials(materi_list, xmlconfig, this->materials);

    size_t num_faces = 0;

    for (size_t s = 0; s < shapes.size(); s++) {
        num_faces += shapes[s].mesh.num_face_vertices.size();
    }
    INFO("Scene face num %zu\n", num_faces);

    std::vector<Emissive*> light_objects;
    save_obj_and_mat(shapes, attrib, this->materials,
        this->objects, light_objects);

    this->egroup.init(light_objects);
    this->camera.init(xmlconfig);
    this->buffer.init(this->camera.width, this->camera.height);

    INFO("Image size: %d x %d (W x H)\n", camera.width, camera.height);
    // bvhtree
    DEBUGM("begin build BVH\n");
    std::vector<Hittable*> objects_copy(this->objects);
    DEBUGM("objects num: %d\n", objects_copy.size());
    bvh_root = build_BVH(objects_copy, 0, objects_copy.size());
    DEBUGM("end build BVH\n");
}

// Loop over all the objects to find intersections
bool Scene::hit(const Ray& ray, const vec2& t_range, HitRecord& rec)
{
    bool flag = false;
    rec.t = INFINITY;
    HitRecord now_rec;
    for (const auto obj : objects) {
        if (obj->hit(ray, t_range, now_rec) && now_rec.t < rec.t) {
            rec = now_rec;
            flag = true;
        }
    }
    return flag;
}

inline flt power_heuristic(flt f, flt g)
{
    return (f * f) / (f * f + g * g);
}

// MIS light sample
vec3 Scene::sample_light(const HitRecord& rec, const vec3& wo, const Hittable* world)
{
    HitRecord light_rec;
    vec3 wi;
    vec3 color(0.0f), bsdf, Li;

    flt light_pdf, bsdf_pdf, weight;

    // sample light
    light_pdf = egroup.sample_ray(rec, world, light_rec, wi);
    if (light_pdf > 0.0f) {
        bsdf_pdf = rec.mat->pdf(wo, rec, wi);
        if (bsdf_pdf > 0.0f) {
            bsdf = rec.mat->bsdf(wo, wi, rec);
            if (!light_rec.mat->type(Li) == Material::LIGHT)
                ERRORM("Not an emissive!\n");
            weight = power_heuristic(light_pdf, bsdf_pdf);

            color += Li * bsdf * glm::dot(wi, rec.normal) * weight / light_pdf;
        }
    }
    // sample bsdf
    bsdf_pdf = rec.mat->scatter(wo, rec, wi);
    if (bsdf_pdf > 0.0f) {
        light_pdf = egroup.pdf(Ray(rec.p, wi), world, light_rec);
        if (light_pdf > 0.0f) {
            bsdf = rec.mat->bsdf(wo, wi, rec);
            if (!light_rec.mat->type(Li) == Material::LIGHT)
                ERRORM("Not an emissive!\n");
            weight = power_heuristic(bsdf_pdf, light_pdf);

            color += Li * bsdf * glm::dot(wi, rec.normal) * weight / bsdf_pdf;
        }
    }
    return color;
}

// https://computergraphics.stackexchange.com/questions/5152/progressive-path-tracing-with-explicit-light-sampling
vec3 Scene::Li(const Ray& init_ray, const Hittable* world)
{
    vec3 color(0.0f);
    vec3 throughput(1.0f);
    vec3 emissive_color;
    vec3 wo, wi;
    Ray ray = init_ray;
    HitRecord rec;
    const flt Krr = 0.8;
    bool emissive_flag = true;

    for (int bounce = 0;; bounce++) {
        if (world->hit(ray, vec2(kHitEps, INFINITY), rec) == false) {
            // No ray and object intersection, return black
            break;
        }

        wo = -ray.direction;

        // material self emissive
        if (rec.mat->type(emissive_color) == Material::LIGHT) {
            if (emissive_flag && glm::dot(rec.normal, ray.direction) < 0)
                color += throughput * emissive_color;
            break;
        }

        // glass material
        if (rec.mat->type(emissive_color) == Material::GLASS) {
            rec.mat->scatter(wo, rec, wi);
            throughput *= rec.mat->bsdf(wo, wi, rec);
            ray = Ray(rec.p, wi);
            continue;
        }

        emissive_flag = false;
        rec.normal = glm::dot(rec.normal, wo) > 0 ? rec.normal : -rec.normal;

        color += throughput * sample_light(rec, wo, world);
        flt pdf = rec.mat->scatter(wo, rec, wi);
        if (glm::dot(wi, rec.normal) > 0 && pdf > 0.0f) {
            ray = Ray(rec.p, wi);
            throughput *= rec.mat->bsdf(wo, wi, rec) * glm::dot(wi, rec.normal) / pdf;
        } else {
            // throughput will be all zero, stop
            break;
        }

        if (bounce >= 3) {
            if (uniform() < glm::compMax(throughput))
                throughput /= glm::compMax(throughput);
            else
                break;
        }
    }
    return color;
}

void Scene::render(const std::string& outfile, int num_sample)
{
    buffer.clear();
    INFO("Begin render images\n");
    for (int now_sample = 0; now_sample < num_sample; now_sample++) {
#pragma omp parallel for schedule(dynamic)
        for (int x_t = 0; x_t < buffer.width; x_t++) {
            for (int y_t = 0; y_t < buffer.height; y_t++) {
                vec3 light = Li(camera.cast_ray(x_t, y_t), bvh_root);

                if (std::isfinite(light[0]) && std::isfinite(light[1]) && std::isfinite(light[2])) {
                    buffer.b_array[y_t][x_t] += light;
                } else {
                    DEBUGM("Not finite number at sample %d x %d y %d\n", now_sample, x_t, y_t);
                }
            }
        }
        if (now_sample % 5 == 0) {
            INFO("sample num: %d\n", now_sample);
            buffer.to_picture(outfile, now_sample + 1);
        }
    }

    INFO("End render images\n");
    buffer.to_picture(outfile, num_sample);
}