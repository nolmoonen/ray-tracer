#define STB_IMAGE_WRITE_IMPLEMENTATION

#include <stb_image_write.h>
#include <stdint.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>
#include <float.h>
#include "nm_math.h"
#include "types.h"
#include "scene.h"

// only use to write to file
typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} color_t;

/**
 * returns whether ray and sphere intersect, if so:
 * {result} contains the reflection ray, {t} contains the distance between ray origin and intersection */
bool reflect(ray_t *result, float *t, ray_t ray, sphere_t sphere);

/** returns the intensity of a ray at a intersection */
float compute_lighting(ray_t origin, vec3f normal, ray_t reflected, float shininess);

/**
 * returns whether {origin} intersects with a ball in BALLS, if so:
 * {ball} contains the closest ball, {reflected} contains the ray bouncing off that ball */
bool get_closest_ball(ray_t *reflected, ball_t *ball, ray_t origin, float t_min, float t_max);

/** returns the color of a ray */
vec3f trace_ray(ray_t ray, uint32_t depth, float t_min, float t_max);

const float T_MAX = FLT_MAX;              // default far clipping plane
const float T_MIN = 0.f;                  // default near clipping plane
const float T_CLOSE = 0.005f;             // near clipping plane preventing sphere
// casting shadows and reflections on self
const vec3f BACKGROUND = {.1f, .1f, .1f}; // color of the background
const uint32_t DEPTH = 1;                 // number of iterations for reflections/refractions

int main()
{
    /** parameter to calculate camera rays */
    // https://en.wikipedia.org/wiki/Ray_tracing_(graphics)
    const uint32_t K = 800;          // number of pixels in horizontal direction
    const uint32_t M = 600;          // number of pixels in vertical direction
    const vec3f E = {0.f, 1.f, 0.f}; // eye
    const vec3f T = {0.f, 1.f, 1.f}; // target
    const vec3f W = {0.f, 1.f, 0.f}; // up-vector
    const float THETA = M_PI / 2.f;  // field of view

    /** pre-calculation for camera rays */
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
            color_t colorRGB = {(uint8_t) (color.x * 255), (uint8_t) (color.y * 255), (uint8_t) (color.z * 255)};
            memcpy(&buffer[j * K + i], &colorRGB, sizeof(color_t));
        }
    }

    stbi_write_png("out.png", K, M, sizeof(color_t), buffer, (signed) (K * sizeof(color_t)));

    free(buffer);

    return 0;
}

bool reflect(ray_t *result, float *t, ray_t ray, sphere_t sphere)
{
    vec3f v = vec3f_sub(ray.start, sphere.center);
    float discriminant = powf(vec3f_dot(v, ray.direction), 2) - (vec3f_dot(v, v) - powf(sphere.radius, 2));

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
    vec3f n = vec3f_norm(vec3f_sub(y, sphere.center));

    // reflection direction
    vec3f r = vec3f_norm(vec3f_sub(ray.direction, vec3f_scale(n, 2.f * vec3f_dot(n, ray.direction))));

    result->start = y;
    result->direction = r;

    return true;
}

float compute_lighting(ray_t origin, vec3f normal, ray_t reflected, float shininess)
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
            l = vec3f_norm(vec3f_sub(light->v.location, reflected.start));
            t_max = 1.f; // prevent creating shadows beyond the light source
        } else { // if (light->type == LIGHT_DIRECTION)
            l = vec3f_norm(light->v.direction);
        }

        ray_t r;
        ball_t b;
        ray_t intersection_to_light = {.start=reflected.start, .direction=l};
        // prevent casting shadow on itself
        // todo get all balls, and use transparency to calculate shadow coefficient
        if (get_closest_ball(&r, &b, intersection_to_light, T_CLOSE, t_max)) {
            // if a ball is in between intersection and light, this is a shadow
            continue;
        }

        // diffuse
        float ln = vec3f_dot(l, normal);
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
    ball_t closest_ball; // ball that has the closest intersection with the ray
    ray_t reflection;    // ray reflecting of that ball
    bool intersect = get_closest_ball(&reflection, &closest_ball, ray, t_min, t_max);

    if (!intersect) {
        // if the ray does not hit an object, use the background color
        return BACKGROUND;
    }

    // normal outwards from the sphere center
    vec3f normal = vec3f_norm(vec3f_sub(reflection.start, closest_ball.sphere.center));
    float intensity = compute_lighting(ray, normal, reflection, closest_ball.shininess);
    // the color of the closest ball the ray hits
    vec3f color_intersection = vec3f_scale(closest_ball.color, intensity);

    if (depth == 0) {
        // if max depth is reached, do not reflect or refract ray
        return color_intersection;
    }

    /** compute reflective color */
    vec3f reflected_color;
    if (closest_ball.reflection.type == REFLECTIVE || closest_ball.reflection.type == REFLECTIVE_REFRACTIVE) {
        // recursive call
        reflected_color = trace_ray(reflection, depth - 1, T_CLOSE, t_max);
    }

    /** common code for REFRACTIVE and REFLECTIVE_REFRACTIVE */
    float cos_i = vec3f_dot(normal, ray.direction); // cosine of angle of incidence
    float eta_i; // refractive index of the material the ray is in
    float eta_t; // refractive index of the material the refractive ray is in
    vec3f n; // vector pointing into the material the ray is going into
    if (cos_i < 0) {
        // ray hits outside of sphere
        eta_i = 1.f;
        eta_t = closest_ball.reflection.refractive_index;
        n = normal;
        cos_i = -cos_i; // cos_i needs to be positive
    } else {
        // ray hits inside of sphere
        eta_i = closest_ball.reflection.refractive_index;
        eta_t = 1.f;
        n = vec3f_scale(normal, -1.f);
    }

    /** compute refractive color */
    vec3f refracted_color;
    if (closest_ball.reflection.type == REFRACTIVE || closest_ball.reflection.type == REFLECTIVE_REFRACTIVE) {
        float eta_r = eta_i / eta_t;
        float discriminant = 1.f - eta_r * eta_r * (1.f - cos_i * cos_i);

        if (discriminant < 0) {
            // discriminant is negative, total internal refraction
            return color_intersection;
        }

        float b = eta_r * cos_i - sqrtf(discriminant);
        vec3f refraction_dir = vec3f_norm(vec3f_add(vec3f_scale(ray.direction, eta_r), vec3f_scale(n, b)));
        ray_t refraction = {reflection.start, refraction_dir};

        // recursive call
        refracted_color = trace_ray(refraction, depth - 1, T_CLOSE, t_max);
    }

    float kr; // reflected component
    float kt; // refracted component
    if (closest_ball.reflection.type == REFLECTIVE_REFRACTIVE) {
        // use snell's law to compute the reflective and refractive components
        // sinus of angle of refraction
        float sin_t = eta_i / eta_t * sqrtf(fmaxf(0.f, 1.f - cos_i * cos_i));

        if (sin_t >= 1.f) {
            // total internal reflection, only reflection
            kr = 1.f;
        } else {
            // cosinus of angle of refraction
            float cos_t = sqrtf(fmaxf(0.f, 1.f - sin_t * sin_t));
            float r_parallel = ((eta_t * cos_i) - (eta_i * cos_t)) / ((eta_t * cos_i) + (eta_i * cos_t));
            float r_perpendicular = ((eta_i * cos_i) - (eta_t * cos_t)) / ((eta_i * cos_i) + (eta_t * cos_t));
            kr = (r_parallel * r_parallel + r_perpendicular * r_perpendicular) / 2.f;
        }

        kt = 1.f - kr;
    }

    switch (closest_ball.reflection.type) {
        case NONE:
            return color_intersection;
        case REFLECTIVE:
            // color is determined by color of intersection and reflection
            return vec3f_add(
                    vec3f_scale(color_intersection, 1.f - closest_ball.reflection.fraction.reflectiveness),
                    vec3f_scale(reflected_color, closest_ball.reflection.fraction.reflectiveness)
            );
        case REFRACTIVE:
            // color is determined by color of intersection and refraction
            return vec3f_add(
                    vec3f_scale(color_intersection, 1.f - closest_ball.reflection.fraction.refractiveness),
                    vec3f_scale(refracted_color, closest_ball.reflection.fraction.refractiveness)
            );
        case REFLECTIVE_REFRACTIVE:
            // color is determined by color of reflection and refraction
            return vec3f_add(
                    vec3f_scale(reflected_color, kr),
                    vec3f_scale(refracted_color, kt)
            );
    }
}
