#include <cmath>

#include "glm/ext.hpp"
#include "glm/glm.hpp"
#include "tinyxml2.h"

#include "camera.hpp"
#include "global.hpp"
#include "ray.hpp"

Camera::Camera()
{
}

void Camera::init(const tinyxml2::XMLDocument& xmlconfig)
{
    auto camera_xmlnode = xmlconfig.FirstChildElement("camera");
    if (!camera_xmlnode) {
        ERRORM("No camera founded in the xml file\n");
    }

    // Warning: NULL pointer check is missing
    vec3 foo;
    sscanf(camera_xmlnode->FirstChildElement("eye")->Attribute("value"), "%f,%f,%f", &foo[0], &foo[1], &foo[2]);
    position = foo;
    sscanf(camera_xmlnode->FirstChildElement("lookat")->Attribute("value"), "%f,%f,%f", &foo[0], &foo[1], &foo[2]);
    lookat = foo;
    sscanf(camera_xmlnode->FirstChildElement("up")->Attribute("value"), "%f,%f,%f", &foo[0], &foo[1], &foo[2]);
    up = foo;
    camera_xmlnode->FirstChildElement("fovy")->QueryFloatAttribute("value", &(fovy));
    camera_xmlnode->FirstChildElement("width")->QueryIntAttribute("value", &(width));
    camera_xmlnode->FirstChildElement("height")->QueryIntAttribute("value", &(height));

    DEBUGM("camera arguments\n");
    DEBUGM("eye: %f %f %f\n", position[0], position[1], position[2]);
    DEBUGM("lookat: %f %f %f\n", lookat[0], lookat[1], lookat[2]);
    DEBUGM("up: %f %f %f\n", up[0], up[1], up[2]);
    DEBUGM("fovy %f \n", fovy);

    aspect = 1.0f * width / height;

    // Assuming z_length = 1
    flt y_length = 2 * tan(glm::radians(fovy * 0.5f));
    flt x_length = y_length * aspect;

    DEBUGM("xy_length: %f %f\n", x_length, y_length);

    vec4 left_top_corner_origin = vec4(-x_length / 2, y_length / 2, -1, 1);
    vec4 right_top_corner_origin = vec4(x_length / 2, y_length / 2, -1, 1);
    vec4 left_bottom_corner_origin = vec4(-x_length / 2, -y_length / 2, -1, 1);

    auto lookat_mat_inv = glm::inverse(glm::lookAt(position, lookat, up));

    left_top_corner = vec3(lookat_mat_inv * left_top_corner_origin);
    vec3 right_top_corner = vec3(lookat_mat_inv * right_top_corner_origin);
    vec3 left_bottom_corner = vec3(lookat_mat_inv * left_bottom_corner_origin);

    dw = (right_top_corner - left_top_corner) / (1.0f * width);
    dh = (left_bottom_corner - left_top_corner) / (1.0f * height);

    DEBUGM("left_top_corner: %f %f %f\n", left_top_corner[0], left_top_corner[1], left_top_corner[2]);
    DEBUGM("dw: %f %f %f\n", dw[0], dw[1], dw[2]);
    DEBUGM("dh: %f %f %f\n", dh[0], dh[1], dh[2]);
}

Ray Camera::cast_ray(int x, int y)
{
    return Ray(position, (left_top_corner + (x + uniform() - 0.5f) * dw + (y + uniform() - 0.5f) * dh) - position);
}
