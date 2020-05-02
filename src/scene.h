/*
 * Defines all scenes.
 */
#ifndef RAY_TRACER_SCENE_H
#define RAY_TRACER_SCENE_H

#define POINT_LIGHT_POS {14.f, 9.f, 10.f}
const light_t LIGHTS[] = {
        {.type=LIGHT_AMBIENT, .intensity=.2f},
        {.type=LIGHT_POINT, .intensity=.6f, .v.location=POINT_LIGHT_POS},
        {.type=LIGHT_DIRECTIONAL, .intensity=.2f, .v.direction={1.f, 4.f, 4.f}}
};

const ball_t BALLS[] = {
        // red ball
        {
                .sphere={.center={-9.f, 1.f, 30.f}, .radius=4.f}, .color={1.f, 0.f, 0.f}, .shininess=10.f,
                .reflection={.type=NONE}
        },
        // green ball
        {
                .sphere={.center={-3.f, 1.f, 26.f}, .radius=4.f}, .color={0.f, 1.f, 0.f}, .shininess=100.f,
                .reflection={.type=REFLECTIVE, .fraction.reflectiveness=.2f}
        },
        // blue ball
        {
                .sphere={.center={+3.f, 1.f, 22.f}, .radius=4.f}, .color = {0.f, 0.f, 1.f}, .shininess = 500.f,
                .reflection={.type=REFRACTIVE, .fraction.refractiveness=.9f, .refractive_index=1.5f}
        },
        // yellow ball
        {
                .sphere={.center={+9.f, 1.f, 18.f}, .radius=4.f}, .color = {1.f, 1.f, 0.f}, .shininess = 1000.f,
                .reflection={.type=REFLECTIVE_REFRACTIVE, .refractive_index=1.5f}
        },
        // a large ball that serves as a surface
        {
                .sphere={.center={0.f, -50002.5f, 24.f}, .radius=50000.f}, .color = {.9f, .9f, .9f}, .shininess = 0.f,
                .reflection={.type=NONE}
        },
        // DEBUG: render a ball in the location of a light
//        {.sphere={.c=POINT_LIGHT_POS, .r=1.f}, .color={1.f, 1.f, 1.f}, .shininess=0.f}
};

#endif //RAY_TRACER_SCENE_H
