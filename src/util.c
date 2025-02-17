#include "util.h"
#include "scene.h"

const float T_CLOSE = 0.005f;

vec3f reflect(vec3f ray, vec3f normal)
{
    return vec3f_norm(vec3f_sub(
            ray,
            vec3f_scale(normal, 2.f * vec3f_dot(normal, ray))
    ));
}

bool reflect_sphere(hit_t *hit, ray_t ray, sphere_t sphere)
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
    hit->t = fminf(t_neg, t_pos);

    // intersection point of ray and sphere
    vec3f y = vec3f_add(ray.start, vec3f_scale(ray.direction, hit->t));

    // normal to the sphere
    hit->normal = vec3f_norm(vec3f_sub(y, sphere.center));

    // reflection direction
    vec3f r = reflect(vec3f_norm(ray.direction), hit->normal);

    hit->reflect.start = y;
    hit->reflect.direction = r;

    return true;
}

bool reflect_plane(hit_t *hit, ray_t ray, plane_t plane)
{
    vec3f normal = vec3f_norm(plane.normal);
    float denominator = vec3f_dot(ray.direction, normal);

    if (fabsf(denominator) == 0.f) {
        // direction and plane parallel, no intersection
        return false;
    }

    float t = vec3f_dot(vec3f_sub(plane.point, ray.start), normal) / denominator;
    if (t < 0) {
        // plane behind ray's origin, no intersection
        return false;
    }

    // reflected direction
    vec3f r = reflect(ray.direction, normal);

    // hit point
    vec3f y = vec3f_add(ray.start, vec3f_scale(ray.direction, t));

    if (plane.type == PLANE_BOUNDED) {
        // if plane is bounded, perform containment checks
        vec3f a = vec3f_sub(y, plane.point);

        // scalar projection of a on the first direction
        float a_n = vec3f_dot(a, vec3f_norm(plane.first));
        if (a_n < 0 || a_n > vec3f_len(plane.first)) {
            return false;
        }

        // scalar projection of a on the second direction
        float a_m = vec3f_dot(a, vec3f_norm(plane.second));
        if (a_m < 0 || a_m > vec3f_len(plane.second)) {
            return false;
        }
    }

    hit->t = t;
    hit->reflect.start = y;
    hit->reflect.direction = r;
    hit->normal = normal;

    return true;
}

float compute_lighting(ray_t origin, vec3f normal, ray_t reflected, float shininess)
{
    float intensity = 0.f;

    for (uint32_t i = 0; i < LIGHTS_SIZE; i++) {
        const light_t *light = &LIGHTS[i];

        /** compute ambient contribution */
        if (light->type == LIGHT_AMBIENT) {
            intensity += light->intensity;
            continue;
        }

        /** compute direction to light */
        vec3f l; // direction to the light
        float t_max = FLT_MAX;
        if (light->type == LIGHT_POINT) {
            l = vec3f_norm(vec3f_sub(light->v.location, reflected.start));
            t_max = 1.f; // prevent creating shadows beyond the light source
        } else { // if (light->type == LIGHT_DIRECTION)
            l = vec3f_norm(light->v.direction);
        }

        /** compute how much light the objects between intersection and light let trough */
        ray_t intersection_to_light = {.start=reflected.start, .direction=l};
        // t_min = TCLOSE to prevent casting shadow on itself
        float shadow_factor = get_shadow_factor(intersection_to_light, T_CLOSE, t_max);
        if (shadow_factor == 0.f) {
            // objects let no light through, no diffuse and specular contributions
            continue;
        }

        /** diffuse contribution */
        float ln = vec3f_dot(l, normal);
        if (ln > 0.f) {
            intensity += shadow_factor * light->intensity * ln;
        }

        /** specular contribution */
        if (shininess != -1.f) {
            // if there is a specular component
            vec3f e = vec3f_norm(vec3f_sub(origin.start, reflected.start)); // direction to the eye
            vec3f h = vec3f_norm(vec3f_add(e, l));
            float hn = vec3f_dot(h, reflected.direction);
            if (hn > 0.f) {
                intensity += shadow_factor * light->intensity * powf(hn, shininess);
            }
        }
    }

    // clamp to ensure [0, 1]
    intensity = intensity > 1.f ? 1.f : intensity < 0.f ? 0.f : intensity;
    return intensity;
}

bool get_closest_sphere(hit_t *reflected, sphere_t *sphere, ray_t origin, float t_min, float t_max)
{
    float t_smallest;
    bool found_one = false;
    for (uint32_t i = 0; i < SPHERES_SIZE; i++) {
        hit_t hit;
        if (reflect_sphere(&hit, origin, SPHERES[i])) {
            if ((!found_one && hit.t >= t_min && hit.t <= t_max) ||
                (hit.t < t_smallest && hit.t >= t_min && hit.t <= t_max)) {
                t_smallest = hit.t;
                found_one = true;

                *sphere = SPHERES[i];
                *reflected = hit;
            }
        }
    }

    return found_one;
}

bool get_closest_plane(hit_t *reflected, plane_t *plane, ray_t origin, float t_min, float t_max)
{
    float t_smallest;
    bool found_one = false;
    for (uint32_t i = 0; i < PLANES_SIZE; i++) {
        hit_t hit;
        if (reflect_plane(&hit, origin, PLANES[i])) {
            if ((!found_one && hit.t >= t_min && hit.t <= t_max) ||
                (hit.t < t_smallest && hit.t >= t_min && hit.t <= t_max)) {
                t_smallest = hit.t;
                found_one = true;

                *plane = PLANES[i];
                *reflected = hit;
            }
        }
    }

    return found_one;
}

vec3f trace_ray(ray_t ray, uint32_t depth, float t_min, float t_max)
{
    /** check sphere intersection */
    sphere_t closest_sphere; // sphere that has the closest intersection with the ray
    hit_t sphere_hit;               // ray reflecting of closest sphere
    bool sphere_intersect = get_closest_sphere(&sphere_hit, &closest_sphere, ray, t_min, t_max);

    /** check plane intersection */
    plane_t closest_plane;
    hit_t plane_hit;
    bool plane_intersect = get_closest_plane(&plane_hit, &closest_plane, ray, t_min, t_max);

    if (!sphere_intersect && !plane_intersect) {
        // if the ray does not hit an object, use the background color
        vec3f background_color = BACKGROUND;
        return background_color;
    }

    /** properties of the intersected object */
    material_t material;
    hit_t hit;
    if ((sphere_intersect && !plane_intersect) || (sphere_intersect && sphere_hit.t < plane_hit.t)) {
        material = closest_sphere.material;
        hit = sphere_hit;
    } else { // (!sphere_intersect && plane_intersect) || (plane_intersect && sphere_hit.t >= plane_hit.t)
        material = closest_plane.material;
        hit = plane_hit;

        if (closest_plane.checkered_xz) {
            // if a checker pattern should be applied in x/y-plane
            if (hit.reflect.start.x * hit.reflect.start.z > 0) {
                // components have different sign
                if (((signed) hit.reflect.start.x + (signed) hit.reflect.start.z) % 2) {
                    material.color = closest_plane.checker_color;
                }
            } else {
                // components have same sign (flip pattern)
                if (((signed) hit.reflect.start.x + (signed) hit.reflect.start.z + 1) % 2) {
                    material.color = closest_plane.checker_color;
                }
            }
        }
    }

    float intensity = compute_lighting(ray, hit.normal, hit.reflect, material.shininess);
    // the color of the closest sphere the ray hits
    vec3f color_intersection = vec3f_scale(material.color, intensity);

    if (depth == 0) {
        // if max depth is reached, do not reflect or refract ray
        return color_intersection;
    }

    /** compute reflective color */
    vec3f reflected_color;
    if (material.reflection.type == REFLECTIVE || material.reflection.type == REFLECTIVE_REFRACTIVE) {
        // recursive call
        reflected_color = trace_ray(hit.reflect, depth - 1, T_CLOSE, t_max);
    }

    /** common code for REFRACTIVE and REFLECTIVE_REFRACTIVE */
    float cos_i = vec3f_dot(hit.normal, ray.direction); // cosine of angle of incidence
    float eta_i; // refractive index of the material the ray is in
    float eta_t; // refractive index of the material the refractive ray is in
    vec3f n; // vector pointing into the material the ray is going into
    if (cos_i < 0) {
        // ray hits outside of sphere
        eta_i = 1.f;
        eta_t = material.reflection.refractive_index;
        n = hit.normal;
        cos_i = -cos_i; // cos_i needs to be positive
    } else {
        // ray hits inside of sphere
        eta_i = material.reflection.refractive_index;
        eta_t = 1.f;
        n = vec3f_scale(hit.normal, -1.f);
    }

    /** compute refractive color */
    vec3f refracted_color;
    if (material.reflection.type == REFRACTIVE || material.reflection.type == REFLECTIVE_REFRACTIVE) {
        float eta_r = eta_i / eta_t;
        float discriminant = 1.f - eta_r * eta_r * (1.f - cos_i * cos_i);

        if (discriminant < 0) {
            // discriminant is negative, total internal refraction
            return color_intersection;
        }

        float b = eta_r * cos_i - sqrtf(discriminant);
        vec3f refraction_dir = vec3f_norm(vec3f_add(vec3f_scale(ray.direction, eta_r), vec3f_scale(n, b)));
        ray_t refraction = {hit.reflect.start, refraction_dir};

        // recursive call
        refracted_color = trace_ray(refraction, depth - 1, T_CLOSE, t_max);
    }

    float kr; // reflected component
    float kt; // refracted component
    if (material.reflection.type == REFLECTIVE_REFRACTIVE) {
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

    switch (material.reflection.type) {
        case NONE:
            return color_intersection;
        case REFLECTIVE:
            // color is determined by color of intersection and reflection
            return vec3f_add(
                    vec3f_scale(color_intersection, 1.f - material.reflection.fraction.reflectiveness),
                    vec3f_scale(reflected_color, material.reflection.fraction.reflectiveness)
            );
        case REFRACTIVE:
            // color is determined by color of intersection and refraction
            return vec3f_add(
                    vec3f_scale(color_intersection, 1.f - material.reflection.fraction.refractiveness),
                    vec3f_scale(refracted_color, material.reflection.fraction.refractiveness)
            );
        case REFLECTIVE_REFRACTIVE:
            // color is determined by color of reflection and refraction
            return vec3f_add(
                    vec3f_scale(reflected_color, kr),
                    vec3f_scale(refracted_color, kt)
            );
    }
}

float get_light_troughput(material_t material)
{
    switch (material.reflection.type) {
        case NONE:
        case REFLECTIVE:
            // no light is let through, early termination
            return 0.f;
        case REFRACTIVE:
            // amount of light let through depends on refractive contribution
            // the more light it refracts, the more light passes through
            return material.reflection.fraction.refractiveness;
        case REFLECTIVE_REFRACTIVE:
            // fully transparent, all light is let through
            return 1.f;
    }
}

float get_shadow_factor(ray_t l, float t_min, float t_max)
{
    hit_t hit;

    // light starts at full strength
    float light_strength = 1.f;

    // calculate how much light the spheres let trough
    for (uint32_t i = 0; i < SPHERES_SIZE; i++) {
        if (reflect_sphere(&hit, l, SPHERES[i]) && t_min < hit.t && hit.t < t_max) {
            light_strength *= get_light_troughput(SPHERES[i].material);
            if (light_strength == 0.f) {
                // if the throughput has reached 0, return early
                return 0.f;
            }
        }
    }

    // calculate how much light the planes let trough
    for (uint32_t i = 0; i < PLANES_SIZE; i++) {
        if (reflect_plane(&hit, l, PLANES[i]) && t_min < hit.t && hit.t < t_max) {
            light_strength *= get_light_troughput(PLANES[i].material);
            if (light_strength == 0.f) {
                // if the throughput has reached 0, return early
                return 0.f;
            }
        }
    }

    return light_strength;
}