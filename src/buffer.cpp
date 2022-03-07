
#include <string>
#include <vector>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

#include "buffer.hpp"

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
