#ifndef RAY_TRACE_NM_MATH_H
#define RAY_TRACE_NM_MATH_H

#include <math.h>
#include <stdbool.h>

typedef struct {
    float x;
    float y;
    float z;
} vec3f;

// todo: maybe pass pointer and return result to prevent unnecessary copies

/** return a + b */
vec3f vec3f_add(vec3f a, vec3f b);

/** return a - b */
vec3f vec3f_sub(vec3f a, vec3f b);

/** return dot product of a and b */
float vec3f_dot(vec3f a, vec3f b);

/** return cross product of a and b */
vec3f vec3f_cross(vec3f a, vec3f b);

/** return length of a */
float vec3f_len(vec3f a);

/** returns the normalized vector a */
vec3f vec3f_norm(vec3f a);

/** returns a scaled by k */
vec3f vec3f_scale(vec3f a, float k);

/** returns true if a and b are equal */
bool vec3f_eq(vec3f a, vec3f b);

#endif //RAY_TRACE_NM_MATH_H
