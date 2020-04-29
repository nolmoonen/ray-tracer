#include "nm_math.h"

vec3f vec3f_add(vec3f a, vec3f b)
{
    vec3f result = {a.x + b.x, a.y + b.y, a.z + b.z};

    return result;
}

vec3f vec3f_sub(vec3f a, vec3f b)
{
    vec3f result = {a.x - b.x, a.y - b.y, a.z - b.z};

    return result;
}

float vec3f_dot(vec3f a, vec3f b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

vec3f vec3f_cross(vec3f a, vec3f b)
{
    vec3f result = {
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x
    };

    return result;
}

float vec3f_len(vec3f a)
{
    return sqrtf(a.x * a.x + a.y * a.y + a.z * a.z);
}

vec3f vec3f_norm(vec3f a)
{
    float len = vec3f_len(a);

    if (len == 0) {
        vec3f result = {0.f, 0.f, 0.f};
        return result;
    }

    vec3f result = {a.x / len, a.y / len, a.z / len};

    return result;
}

vec3f vec3f_scale(vec3f a, float k)
{
    vec3f result = {a.x * k, a.y * k, a.z * k};

    return result;
}

bool vec3f_eq(vec3f a, vec3f b)
{
    return a.x == b.x && a.y == b.y && a.z == b.z;
}
