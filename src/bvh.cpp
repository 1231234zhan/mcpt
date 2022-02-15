
#include <algorithm>

#include "bvh.hpp"
#include "global.hpp"
#include "object.hpp"

BVHnode::BVHnode()
{
    child[0] = child[1] = NULL;
}

BVHnode::BVHnode(const BBox& input_box)
{
    box = input_box;
    child[0] = child[1] = NULL;
}

BVHnode::BVHnode(const Hittable* object)
{
    object->bounding_box(this->box);
    child[0] = child[1] = NULL;
}

bool BVHnode::hit(const Ray& ray, const vec2& t_range, HitRecord& rec) const
{
    // present node is not a valid node
    if (box.max_p[0] < box.min_p[0]) {
        ERRORM("Not valid box!!\n");
    }

    BBox child_box[2];
    flt t_hit[2];
    bool is_hit[2];

    child[0]->bounding_box(child_box[0]);
    child[1]->bounding_box(child_box[1]);

    is_hit[0] = child_box[0].hit(ray, t_range, t_hit[0]);
    is_hit[1] = child_box[1].hit(ray, t_range, t_hit[1]);

    if (is_hit[0] && is_hit[1]) {
        int near = t_hit[0] < t_hit[1] ? 0 : 1;
        int far = 1 - near;
        if (child[near]->hit(ray, t_range, rec)) {
            if (rec.t < t_hit[far])
                return true;
            HitRecord rec_far;
            if (child[far]->hit(ray, t_range, rec_far) && rec_far.t < rec.t) {
                rec = rec_far;
            }
            return true;
        } else {
            return child[far]->hit(ray, t_range, rec);
        }
    }

    for (int i = 0; i < 2; i++) {
        if (is_hit[i]) {
            return child[i]->hit(ray, t_range, rec);
        }
    }
    return false;
}

bool BVHnode::bounding_box(BBox& box) const
{
    box = this->box;
    return true;
}

template <int dim>
bool comp_objects_dim(const Hittable* obj1, const Hittable* obj2)
{
    BBox box1, box2;
    obj1->bounding_box(box1);
    obj2->bounding_box(box2);
    return box1.min_p[dim] < box2.min_p[dim];
}

Hittable* build_BVH(std::vector<Hittable*>& objects, int i_begin, int i_end)
{
    if (i_end - i_begin == 0)
        return NULL;
    if (i_end - i_begin == 1) {
        return objects[i_begin];
    }

    BVHnode* node = new BVHnode();
    for (int i = i_begin; i < i_end; i++) {
        BBox foo;
        objects[i]->bounding_box(foo);
        node->box.update(foo);
    }

    vec3 box_size = node->box.max_p - node->box.min_p;
    int id_max_size = 0;
    for (int i = 0; i < 3; i++) {
        if (box_size[i] > box_size[id_max_size])
            id_max_size = i;
    }
    auto comp = id_max_size == 0 ? comp_objects_dim<0> : id_max_size == 1 ? comp_objects_dim<1>
                                                                          : comp_objects_dim<2>;

    std::sort(objects.begin() + i_begin, objects.begin() + i_end, comp);

    int i_mid = (i_begin + i_end) / 2;
    node->child[0] = build_BVH(objects, i_begin, i_mid);
    node->child[1] = build_BVH(objects, i_mid, i_end);

    if(!node->child[0] || !node->child[1]){
        ERRORM("NULL child\n");
    }

    return static_cast<Hittable*>(node);
}