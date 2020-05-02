#define STB_IMAGE_WRITE_IMPLEMENTATION

#include <stb_image_write.h>
#include <stdint.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>
#include <float.h>
#include "nm_math.h"

typedef struct {
    vec3f c; // center
    float r; // radius
} sphere_t;

typedef struct {
    sphere_t sphere;
    vec3f color;
    float shininess;
    float reflectiveness;
} ball_t;

// only use to write to file
typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} color_t;

typedef struct {
    vec3f start;
    vec3f direction;
} ray_t;

typedef enum {
    LIGHT_AMBIENT, LIGHT_POINT, LIGHT_DIRECTIONAL
} light_type_t;

typedef struct {
    light_type_t type;
    float intensity;
    union {
        vec3f direction;
        vec3f location;
    } vec;
} light_t;

/**
 * returns whether ray and sphere intersect, if so:
 * {result} contains the reflection ray, {t} contains the distance between ray origin and intersection */
bool reflect(ray_t *result, float *t, ray_t ray, sphere_t sphere);

float compute_lighting(ray_t origin, ray_t reflected, float shininess);

/**
 * returns whether {origin} intersects with a ball in BALLS, if so:
 * {ball} contains the closest ball, {reflected} contains the ray bouncing off that ball */
bool get_closest_ball(ray_t *reflected, ball_t *ball, ray_t origin, float t_min, float t_max);

/** returns the color of a ray */
vec3f trace_ray(ray_t ray, uint32_t depth, float t_min, float t_max);

/** scene definition */
#define POINT_LIGHT_POS {14.f, 9.f, 10.f}

const light_t LIGHTS[] = {
        {.type=LIGHT_AMBIENT, .intensity=.2f},
        {.type=LIGHT_POINT, .intensity=.6f, .vec.location=POINT_LIGHT_POS},
        {.type=LIGHT_DIRECTIONAL, .intensity=.2f, .vec.direction={1.f, 4.f, 4.f}}
};

const ball_t BALLS[] = {
        // couple of same sized balls
        {.sphere={.c={-9.f, 1.f, 30.f}, .r=4.f}, .color={1.f, 0.f, 0.f},
                .shininess=10.f, .reflectiveness=.2f},   // red
        {.sphere={.c={-3.f, 1.f, 26.f}, .r=4.f}, .color={0.f, 1.f, 0.f},
                .shininess=100.f, .reflectiveness=.3f},  // green
        {.sphere={.c={+3.f, 1.f, 22.f}, .r=4.f}, .color={0.f, 0.f, 1.f},
                .shininess=500.f, .reflectiveness=.4f},  // blue
        {.sphere={.c={+9.f, 1.f, 18.f}, .r=4.f}, .color={1.f, 1.f, 0.f},
                .shininess=1000.f, .reflectiveness=.5f}, // yellow
        // a large ball that serves as a surface
        {.sphere={.c={0.f, -50002.5f, 24.f}, .r=50000.f}, .color={.9f, .9f, .9f}, .shininess=0.f},
        // DEBUG: render a ball in the location of a light
//        {.sphere={.c=POINT_LIGHT_POS, .r=1.f}, .color={1.f, 1.f, 1.f}, .shininess=0.f}
};

const float T_CLOSE = 0.005f; // near clipping plane to prevent from casting shadows and reflections on self

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
    const uint32_t DEPTH = 3;
    const float T_MIN = 0.f;
    const float T_MAX = FLT_MAX;

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
            ray_t ray = {.start=E, .direction=rij};

            vec3f color = trace_ray(ray, DEPTH, T_MIN, T_MAX);

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

bool reflect(ray_t *result, float *t, ray_t ray, sphere_t sphere)
{
    vec3f v = vec3f_sub(ray.start, sphere.c);
    float discriminant = powf(vec3f_dot(v, ray.direction), 2) - (vec3f_dot(v, v) - powf(sphere.r, 2));

    if (discriminant < 0) {
        // quantity under the square root is negative, no intersection
        return false;
    }

    float square_root = sqrtf(discriminant);
    float t_neg = -vec3f_dot(v, ray.direction) - square_root;
    float t_pos = -vec3f_dot(v, ray.direction) + square_root;

    if (t_neg < 0 && t_pos < 0) {
        // no positive solution, no intersection
        return false;
    }

    // choose t to be closest intersection point (t >= 0)
    *t = fminf(t_neg, t_pos);

    // intersection point of ray and sphere
    vec3f y = vec3f_add(ray.start, vec3f_scale(ray.direction, *t));

    // normal to the sphere
    vec3f n = vec3f_norm(vec3f_sub(y, sphere.c));

    // reflection direction
    vec3f r = vec3f_sub(ray.direction, vec3f_scale(n, 2.f * vec3f_dot(n, ray.direction)));

    result->start = y;
    result->direction = r;

    return true;
}

float compute_lighting(ray_t origin, ray_t reflected, float shininess)
{
    float intensity = 0.f;

    for (uint32_t i = 0; i < sizeof(LIGHTS) / sizeof(light_t); i++) {
        const light_t *light = &LIGHTS[i];

        // ambient
        if (light->type == LIGHT_AMBIENT) {
            intensity += light->intensity;
            continue;
        }

        vec3f l; // direction to the light
        float t_max = FLT_MAX;
        if (light->type == LIGHT_POINT) {
            l = vec3f_sub(light->vec.location, reflected.start);
            t_max = 1.f; // prevent creating shadows beyond the light source
        } else { // if (light->type == LIGHT_DIRECTION)
            l = light->vec.direction;
        }

        vec3f n = vec3f_norm(reflected.direction); // surface normal
        l = vec3f_norm(l);

        ray_t r;
        ball_t b;
        ray_t intersection_to_light = {reflected.start, l};
        // prevent casting shadow on itself
        if (get_closest_ball(&r, &b, intersection_to_light, T_CLOSE, t_max)) {
            // if a ball is in between intersection and light, this is a shadow
            continue;
        }

        // diffuse
        float ln = vec3f_dot(l, n);
        if (ln > 0.f) {
            intensity += light->intensity * ln;
        }

        // specular
        if (shininess > 0.f) { // if there is a specular component
            vec3f e = vec3f_norm(vec3f_sub(origin.start, reflected.start)); // direction to the eye
            vec3f h = vec3f_norm(vec3f_add(e, l));
            float hn = vec3f_dot(h, reflected.direction);
            if (hn > 0.f) {
                intensity += light->intensity * powf(hn, shininess);
            }
        }
    }

    // clamp to ensure [0, 1]
    intensity = intensity > 1.f ? 1.f : intensity < 0.f ? 0.f : intensity;
    return intensity;
}

bool get_closest_ball(ray_t *reflected, ball_t *ball, ray_t origin, float t_min, float t_max)
{
    float t_smallest;
    bool found_one = false;
    for (uint32_t i = 0; i < sizeof(BALLS) / sizeof(ball_t); i++) {
        float t;
        ray_t reflection;
        if (reflect(&reflection, &t, origin, BALLS[i].sphere)) {
            if ((!found_one && t >= t_min && t <= t_max) || (t < t_smallest && t >= t_min && t <= t_max)) {
                t_smallest = t;
                found_one = true;

                *ball = BALLS[i];
                *reflected = reflection;
            }
        }
    }

    return found_one;
}

vec3f trace_ray(ray_t ray, uint32_t depth, float t_min, float t_max)
{
    ray_t reflection;
    ball_t closest_ball;
    bool intersect = get_closest_ball(&reflection, &closest_ball, ray, t_min, t_max);

    if (!intersect) {
        const vec3f BACKGROUND = {.1f, .1f, .1f};
        return BACKGROUND;
    }

    // get the color of the first intersection
    vec3f color = vec3f_scale(closest_ball.color, compute_lighting(ray, reflection, closest_ball.shininess));
    // todo bounce {reflection} again to more objects

    // if not reflective, or max depth is reached, do not
    if (closest_ball.reflectiveness <= 0 || depth == 0) {
        return color;
    }

    // reflect ray
    vec3f reflected_color = trace_ray(reflection, depth - 1, T_CLOSE, t_max);

    vec3f total_color = vec3f_add(
            vec3f_scale(color, 1.f - closest_ball.reflectiveness),
            vec3f_scale(reflected_color, closest_ball.reflectiveness)
    );

    return total_color;
}
