#define STB_IMAGE_WRITE_IMPLEMENTATION

#include <stb_image_write.h>
#include <stdint.h>
#include <math.h>
#include <stdbool.h>
#include <malloc.h>
#include <string.h>
#include "nm_math.h"

typedef struct {
    vec3f c; // center
    float r; // radius
} sphere_t;

typedef struct {
    vec3f s; // origin
    vec3f d; // direction
} ray_t;

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} color_t;

const color_t LIGHT = {200, 200, 200};
const color_t DARK = {20, 20, 20};

/** returns true if ray and sphere intersect */
bool intersect(ray_t ray, sphere_t sphere);

int main()
{
    /** parameter definition */
    // todo only works for square pixels for now
    //  https://en.wikipedia.org/wiki/Ray_tracing_(graphics)
    const uint32_t K = 800;         // number of pixels in horizontal direction
    const uint32_t M = 800;         // number of pixels in vertical direction
    const vec3f E = {0, 1.f, 0};    // eye
    const vec3f T = {0, 1.f, 1.f};  // target
    const vec3f W = {0, 1.f, 0};    // up-vector
    const float THETA = M_PI / 2.f; // field of view

    const sphere_t LAMP = {.c={0, 1.f, 20.f}, .r=4.f};

    /** pre-calculation */
    vec3f t = vec3f_sub(T, E);   // look direction
    vec3f b = vec3f_cross(W, t); // perpendicular to up and look
    t = vec3f_norm(t);
    b = vec3f_norm(b);
    vec3f v = vec3f_cross(t, b);

    float gx = tanf(THETA / 2.f); // (half) viewport size in horizontal dimension
    float gy = (gx * M) / K;      // (half) viewport size in vertical dimension

    vec3f qx = vec3f_scale(b, (2 * gx) / ((float) K - 1));
    vec3f qy = vec3f_scale(v, (2 * gy) / ((float) M - 1));

    vec3f p1m = vec3f_sub(vec3f_sub(t, vec3f_scale(b, gx)), vec3f_scale(v, gy));

    /** allocate memory */
    color_t *buffer = malloc(M * K * sizeof(color_t));

    /** begin tracing */
    for (uint32_t i = 1; i <= K; i++) {
        for (uint32_t j = 1; j <= M; j++) {
            vec3f rij = vec3f_norm(vec3f_add(p1m, vec3f_add(
                    vec3f_scale(qx, (float) i - 1),
                    vec3f_scale(qy, (float) j - 1)
            )));
            ray_t ray = {.s=E, .d=rij};

            color_t *pixel = &buffer[(i - 1) * M + (j - 1)];
            if (intersect(ray, LAMP)) {
                memcpy(pixel, &LIGHT, sizeof(color_t));
            } else {
                memcpy(pixel, &DARK, sizeof(color_t));
            }
        }
    }

    stbi_write_png("out.png", K, M, sizeof(color_t), buffer, (signed) (K * sizeof(color_t)));

    free(buffer);

    return 0;
}

bool intersect(ray_t ray, sphere_t sphere)
{
    vec3f v = vec3f_sub(ray.s, sphere.c);
    float discriminant = powf(vec3f_dot(v, ray.d), 2) - (vec3f_dot(v, v) - powf(sphere.r, 2));

    return discriminant >= 0;
}