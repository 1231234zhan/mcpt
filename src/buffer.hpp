#pragma once

#include <string>
#include <vector>

#include "global.hpp"

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