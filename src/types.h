/*
 * File containing only all types, this allows for the scene definition to be in a different file.
 */
#ifndef RAY_TRACER_TYPES_H
#define RAY_TRACER_TYPES_H

#include "nm_math.h"

typedef struct {
    vec3f center;
    float radius;
} sphere_t;

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
    sphere_t sphere;
    vec3f color;
    float shininess; // blinn-phong shininess
    reflection_t reflection;
} ball_t;

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
        vec3f direction; // LIGHT_DIRECTIONAL: direction vector of the light
        vec3f location;  // LIGHT_POINT: location of the point light
    } v;
} light_t;

#endif //RAY_TRACER_TYPES_H
