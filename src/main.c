#define STB_IMAGE_WRITE_IMPLEMENTATION

#include <stb_image_write.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
#include <float.h>
#include "vec3.h"
#include "scene.h"

// only use to write to file
typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} color_t;

const float T_MAX = FLT_MAX;               // default far clipping plane
const float T_MIN = 0.f;                   // default near clipping plane
const uint32_t DEPTH = 8;                  // number of iterations for reflections/refractions
const uint32_t RAYS_PER_PIXEL_X = 2;                // number of pixels in horizontal direction
const uint32_t RAYS_PER_PIXEL_Y = RAYS_PER_PIXEL_X; // number of pixels in vertical direction
const uint32_t RAYS_PER_PIXEL = RAYS_PER_PIXEL_X * RAYS_PER_PIXEL_Y; // the level of supersampling

int main()
{
    // https://en.wikipedia.org/wiki/Ray_tracing_(graphics)
    const uint32_t K = SIZE_X * RAYS_PER_PIXEL_X; // number of rays in horizontal direction
    const uint32_t M = SIZE_Y * RAYS_PER_PIXEL_Y; // number of rays in vertical direction

    /** pre-calculation for camera rays */
    vec3f t = vec3f_sub(TARGET, EYE);   // look direction
    vec3f b = vec3f_cross(UP, t); // perpendicular to up and look
    t = vec3f_norm(t);
    b = vec3f_norm(b);
    vec3f v = vec3f_cross(t, b);

    float gx = tanf(FOV / 2.f);    // (half) viewport size in horizontal dimension
    float gy = (gx * M) / (float) K; // (half) viewport size in vertical dimension

    vec3f p11 = vec3f_add(vec3f_sub(t, vec3f_scale(b, gx)), vec3f_scale(v, gy)); // top left ray

    vec3f qx = vec3f_scale(b, (2.f * gx) / (K - 1.f));
    vec3f qy = vec3f_scale(v, (2.f * gy) / (M - 1.f));

    /** allocate memory */
    color_t *buffer = malloc(M * K * sizeof(color_t));

    /** begin tracing */
    for (uint32_t j = 0; j < SIZE_Y; j++) {
        for (uint32_t i = 0; i < SIZE_X; i++) {
            // color for this pixel
            vec3f color = {0.f, 0.f, 0.f};

            for (uint32_t jj = 0; jj < RAYS_PER_PIXEL_Y; jj++) {
                for (uint32_t ii = 0; ii < RAYS_PER_PIXEL_X; ii++) {
                    uint32_t x = i * RAYS_PER_PIXEL_X + ii;
                    uint32_t y = j * RAYS_PER_PIXEL_Y + jj;
                    vec3f rij = vec3f_norm(vec3f_sub(vec3f_add(p11, vec3f_scale(qx, x)), vec3f_scale(qy, y)));
                    ray_t ray = {.start=EYE, .direction=rij};

                    vec3f computed_color = trace_ray(ray, DEPTH, T_MIN, T_MAX);
                    color = vec3f_add(color, vec3f_scale(computed_color, 1.f / (float) RAYS_PER_PIXEL));
                }
            }

            // apply pixel
            color_t colorRGB = {(uint8_t) (color.x * 255), (uint8_t) (color.y * 255), (uint8_t) (color.z * 255)};
            memcpy(&buffer[j * SIZE_X + i], &colorRGB, sizeof(color_t));
        }
    }

    stbi_write_png("out.png", SIZE_X, SIZE_Y, sizeof(color_t), buffer, (signed) (SIZE_X * sizeof(color_t)));

    free(buffer);

    return 0;
}