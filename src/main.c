#define STB_IMAGE_WRITE_IMPLEMENTATION

#include <stb_image_write.h>
#include <stdint.h>
#include <math.h>
#include <stdbool.h>
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

typedef struct {
    float shininess;
    vec3f intensity_specular;
    vec3f intensity_diffuse;
} material_t;

const material_t GREEN_SPHERE = {
        .shininess=16.f,
        .intensity_specular={1.f, 1.f, 1.f},
        .intensity_diffuse={.5f, 0.f, 0.f}
};

/** returns whether ray and sphere intersect, if so result contains the the reflection ray  */
bool reflect(ray_t *result, ray_t ray, sphere_t sphere);

int main()
{
    /** parameter definition */
    // https://en.wikipedia.org/wiki/Ray_tracing_(graphics)
    const uint32_t K = 800;          // number of pixels in horizontal direction
    const uint32_t M = 600;          // number of pixels in vertical direction
    const vec3f E = {0.f, 1.f, 0.f}; // eye
    const vec3f T = {0.f, 1.f, 1.f}; // target
    const vec3f W = {0.f, 1.f, 0.f}; // up-vector
    const float THETA = M_PI / 2.f;  // field of view
    const sphere_t SPHERE = {.c={0.f, 1.f, 21.f}, .r=4.f};
    const sphere_t LAMP = {.c={0.f, 15.f, 21.f}, .r=3.f};

    float specular = 0.2f;
    float diffuse = 0.4f;
    float ambient = 0.4f;
    const vec3f INTENSITY_AMBIENT = {.5f, .5f, .5f};

    /** pre-calculation */
    vec3f t = vec3f_sub(T, E);   // look direction
    vec3f b = vec3f_cross(W, t); // perpendicular to up and look
    t = vec3f_norm(t);
    b = vec3f_norm(b);
    vec3f v = vec3f_cross(t, b);

    float gx = tanf(THETA / 2.f);    // (half) viewport size in horizontal dimension
    float gy = (gx * M) / (float) K; // (half) viewport size in vertical dimension

    vec3f p11 = vec3f_add(vec3f_sub(t, vec3f_scale(b, gx)), vec3f_scale(v, gy)); // top left ray

    vec3f qx = vec3f_scale(b, (2.f * gx) / (K - 1.f));
    vec3f qy = vec3f_scale(v, (2.f * gy) / (M - 1.f));

    /** allocate memory */
    color_t *buffer = malloc(M * K * sizeof(color_t));

    /** begin tracing */
    for (uint32_t j = 0; j < M; j++) {
        for (uint32_t i = 0; i < K; i++) {
            vec3f rij = vec3f_norm(vec3f_sub(vec3f_add(p11, vec3f_scale(qx, i)), vec3f_scale(qy, j)));
            ray_t ray = {.s=E, .d=rij};
            vec3f color = vec3f_scale(INTENSITY_AMBIENT, ambient);

            ray_t reflection;
            bool intersection = reflect(&reflection, ray, SPHERE);
            if (intersection) {
                // do something with reflection
                ray_t new_reflection;
                bool light_intersection = reflect(&new_reflection, reflection, LAMP);
                if (light_intersection) {
                    color = vec3f_add(color, vec3f_scale(GREEN_SPHERE.intensity_specular, specular));
                }

                color = vec3f_add(color, vec3f_scale(GREEN_SPHERE.intensity_diffuse, diffuse));
            }

            bool light_intersection = reflect(&reflection, ray, LAMP);
            if (light_intersection) {
                memcpy(&color, &LIGHT, sizeof(color_t));
            }

            // apply pixel
            color_t colorRGB = {
                    (uint8_t) (color.x * 255),
                    (uint8_t) (color.y * 255),
                    (uint8_t) (color.z * 255)
            };
            memcpy(&buffer[j * K + i], &colorRGB, sizeof(color_t));
        }
    }

    stbi_write_png("out.png", K, M, sizeof(color_t), buffer, (signed) (K * sizeof(color_t)));

    free(buffer);

    return 0;
}

bool reflect(ray_t *result, ray_t ray, sphere_t sphere)
{
    vec3f v = vec3f_sub(ray.s, sphere.c);
    float discriminant = powf(vec3f_dot(v, ray.d), 2) - (vec3f_dot(v, v) - powf(sphere.r, 2));

    if (discriminant < 0) {
        // quantity under the square root is negative, no intersection
        return false;
    }

    float square_root = sqrtf(discriminant);
    float t_neg = -vec3f_dot(v, ray.d) - square_root;
    float t_pos = -vec3f_dot(v, ray.d) + square_root;

    if (t_neg < 0 && t_pos < 0) {
        // no positive solution, no intersection
        return false;
    }

    // choose t to be closest intersection point (t >= 0)
    float t = fminf(t_neg, t_pos);

    // intersection point of ray and sphere
    vec3f y = vec3f_add(ray.s, vec3f_scale(ray.d, t));

    // normal to the sphere
    vec3f n = vec3f_norm(vec3f_sub(y, sphere.c));

    // reflection direction
    vec3f r = vec3f_sub(ray.d, vec3f_scale(n, 2.f * vec3f_dot(n, ray.d)));

    result->s = y;
    result->d = r;

    return true;
}