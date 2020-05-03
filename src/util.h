#ifndef RAY_TRACER_UTIL_H
#define RAY_TRACER_UTIL_H

#include <stdint.h>
#include <float.h>
#include "vec3.h"

typedef enum {
    NONE,                 // no transparency, no reflection
    REFLECTIVE,           // fraction of light is reflected
    REFRACTIVE,           // fraction of light is refracted
    REFLECTIVE_REFRACTIVE // fully transparent, with reflection and refraction
} reflect_type_t;

typedef struct {
    reflect_type_t type;
    union {
        // REFLECTIVE: in [0,1], fraction of light reflected
        float reflectiveness;
        // REFRACTIVE: in [0,1], fraction of light refracted
        float refractiveness;
        // NONE, REFLECTIVE_REFRACTIVE: value is not used
    } fraction;
    // NONE, REFLECTIVE: value not used
    float refractive_index;
} reflection_t;

typedef struct {
    vec3f color;
    float shininess; // blinn-phong shininess
    reflection_t reflection;
} material_t;

typedef struct {
    /** definition of sphere */
    vec3f center;
    float radius;
    /** properties of sphere */
    material_t material;
} sphere_t;

typedef enum {
    PLANE_UNBOUNDED, PLANE_BOUNDED
} plane_type_t;

typedef struct {
    /** definition of plane */
    plane_type_t type; // whether plane is bounded or unbounded
    vec3f point;       // point
    vec3f normal;      // normal
    vec3f first;       // PLANE_BOUNDED: plane in one direction from point
    vec3f second;      // PLANE_BOUNDED: plane in other direction from point
    /** properties of plane */
    material_t material;
} plane_t;

typedef struct {
    vec3f start;
    vec3f direction;
} ray_t;

typedef struct {
    vec3f normal;  // normal of object at position of hit
    ray_t reflect; // reflection invoked by ray hitting
    float t;       // distance from origin ray to hit
} hit_t;

typedef enum {
    LIGHT_AMBIENT, LIGHT_POINT, LIGHT_DIRECTIONAL
} light_type_t;

typedef struct {
    light_type_t type;
    float intensity;
    union {
        vec3f direction; // LIGHT_DIRECTIONAL: direction vector of the light
        vec3f location;  // LIGHT_POINT: location of the point light
    } v;
} light_t;

extern const float T_CLOSE; // near clipping plane preventing sphere casting shadows and reflections on self

/** returns reflected ray. {ray} and {normal} should have been normalized */
vec3f reflect(vec3f ray, vec3f normal);

/** returns whether {ray} and {sphere} intersect, if so: {hit} contains the information about the intersection */
bool reflect_sphere(hit_t *hit, ray_t ray, sphere_t sphere);

/** returns whether {ray} and {plane} intersect, if so: {hit} contains the information about the intersection */
bool reflect_plane(hit_t *hit, ray_t ray, plane_t plane);

/** returns the intensity of a ray at a intersection */
float compute_lighting(ray_t origin, vec3f normal, ray_t reflected, float shininess);

/**
 * returns whether {origin} intersects with a sphere in SPHERES, if so:
 * {sphere} contains the closest sphere, {reflected} contains the ray bouncing off that sphere */
bool get_closest_sphere(hit_t *reflected, sphere_t *sphere, ray_t origin, float t_min, float t_max);

/**
 * returns whether {origin} intersects with a plane in PLANES, if so:
 * {plane} contains the closest plane, {reflected} contains the ray bouncing off that plane */
bool get_closest_plane(hit_t *reflected, plane_t *plane, ray_t origin, float t_min, float t_max);

/** returns the color of a ray */
vec3f trace_ray(ray_t ray, uint32_t depth, float t_min, float t_max);

/** returns the fraction of light that passes trough a material */
float get_light_troughput(material_t material);

/** {l} ray towards light */
float get_shadow_factor(ray_t l, float t_min, float t_max);

#endif //RAY_TRACER_UTIL_H
