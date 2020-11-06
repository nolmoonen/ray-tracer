#include "scene.h"

/** Whitted scene definition */
#ifdef WHITTED_SCENE

const uint32_t SIZE_X = 1000;
const uint32_t SIZE_Y = 750;
const float FOV = (float) M_PI / 2.f;

const vec3f EYE = {0.f, 4.f, -10.f};
const vec3f TARGET = {0.f, 1.f, 0.f};
const vec3f UP = {0.f, 1.f, 0.f};

const vec3f BACKGROUND = {.2f, .7f, 1.f};

const light_t LIGHTS[] = {
        {.type=LIGHT_AMBIENT, .intensity=.9f},
        {.type=LIGHT_DIRECTIONAL, .intensity=.1f, .v.direction={0.f, 1.f, 0.f}}
};

const sphere_t SPHERES[] = {
        // reflective sphere
        {
                .center={0.f, 2.5f, 2.f}, .radius=2.f,
                .material={
                        .color=WHITE, .shininess=10000.f,
                        .reflection={.type=REFLECTIVE, .fraction.reflectiveness=.8f}
                }
        },
        // refractive sphere
        {
                .center={-2.5f, 3.f, -4.f}, .radius=2.f,
                .material={
                        .color=WHITE, .shininess=10000.f,
                        .reflection={.type=REFLECTIVE, .fraction.refractiveness=.8f, .refractive_index=1.5f}
                }
        },
        // transparent sphere
        {
                .center={2.5f, 3.f, -4.f}, .radius=2.f,
                .material={
                        .color=WHITE, .shininess=10000.f,
                        .reflection={.type=REFLECTIVE_REFRACTIVE, .refractive_index=1.5f}
                }
        }
};

const plane_t PLANES[] = {
        // x/z-plane
        {
                .type=PLANE_BOUNDED, .point={-8.f, 0.f, -8.f}, .normal={0.f, 1.f, 0.f},
                .first={16.f, 0.f, 0.f}, .second={0.f, 0.f, 16.f},
                .material={
                        .color=RED, .shininess=-1.f,
                        .reflection={.type=NONE}
                }
        },
};

#endif

/** Pool scene definition */
#ifdef POOL_SCENE

#define POINT_LIGHT_POS {5.f, 5.f, 0.f}

const uint32_t SIZE_X = 1000;
const uint32_t SIZE_Y = 200;
const float FOV = (float) M_PI / 10.f;

const vec3f EYE = {0.f, 4.f, -40.f};
const vec3f TARGET = {0.f, 1.f, 0.f};
const vec3f UP = {0.f, 1.f, 0.f};

const vec3f BACKGROUND = {.1f, .1f, .1f};

const light_t LIGHTS[] = {
        {.type=LIGHT_AMBIENT, .intensity=.2f},
        {.type=LIGHT_POINT, .intensity=.9f, .v.location=POINT_LIGHT_POS},
        {.type=LIGHT_DIRECTIONAL, .intensity=.2f, .v.direction={1.f, 4.f, 4.f}}
};

#define RADIUS 1.f
#define RADIUS_DIAG 1.732050808f // sqrtf(3)
#define SHININESS 1000.f
#define REFLECTION {.type=REFLECTIVE, .fraction.reflectiveness=.3f}

const sphere_t SPHERES[] = {
        // yellow (1)
        {
                .center={0.f, 1.f, 0.f - RADIUS_DIAG * 2.f}, .radius=1.f,
                .material={.color=YELLOW, .shininess=SHININESS, .reflection=REFLECTION}
        },
        // blue (2)
        {
                .center={0.f - RADIUS, 1.f, 0.f - RADIUS_DIAG}, .radius=1.f,
                .material={.color=BLUE, .shininess=SHININESS, .reflection=REFLECTION}
        },
        // red (3)
        {
                .center={0.f + RADIUS, 1.f, 0.f - RADIUS_DIAG}, .radius=1.f,
                .material={.color=RED, .shininess=SHININESS, .reflection=REFLECTION}
        },
        // pink (4)
        {
                .center={0.f - RADIUS * 2.f, 1.f, 0.f}, .radius=1.f,
                .material={.color=VIOLET, .shininess=SHININESS, .reflection=REFLECTION}
        },
        // orange (5) (target)
        {
                .center= {0.f, 1.f, 0.f}, .radius=1.f,
                .material={.color=ORANGE, .shininess=SHININESS, .reflection=REFLECTION}
        },
        // green (6)
        {
                .center={0.f + RADIUS * 2.f, 1.f, 0.f}, .radius=1.f,
                .material={.color=GREEN, .shininess=SHININESS, .reflection=REFLECTION}
        },
        // brown (7)
        {
                .center={0.f - RADIUS * 3.f, 1.f, 0.f + RADIUS_DIAG}, .radius=1.f,
                .material={.color=MAROON, .shininess=SHININESS, .reflection=REFLECTION}
        },
        // black (8)
        {
                .center={0.f - RADIUS * 1.f, 1.f, 0.f + RADIUS_DIAG}, .radius=1.f,
                .material={.color=BLACK, .shininess=SHININESS, .reflection=REFLECTION}
        },
        // yellow (9)
        {
                .center={0.f + RADIUS * 1.f, 1.f, 0.f + RADIUS_DIAG}, .radius=1.f,
                .material={.color=YELLOW, .shininess=SHININESS, .reflection=REFLECTION}
        },
        // blue (10)
        {
                .center={0.f + RADIUS * 3.f, 1.f, 0.f + RADIUS_DIAG}, .radius=1.f,
                .material={.color=BLUE, .shininess=SHININESS, .reflection=REFLECTION}
        },
        // red (11)
        {
                .center={0.f - RADIUS * 4.f, 1.f, 0.f + RADIUS_DIAG * 2.f}, .radius=1.f,
                .material={.color=RED, .shininess=SHININESS, .reflection=REFLECTION}
        },
        // pink (12)
        {
                .center={0.f - RADIUS * 2.f, 1.f, 0.f + RADIUS_DIAG * 2.f}, .radius=1.f,
                .material={.color=VIOLET, .shininess=SHININESS, .reflection=REFLECTION}
        },
        // orange (13)
        {
                .center={0.f, 1.f, 0.f + RADIUS_DIAG * 2.f}, .radius=1.f,
                .material={.color=ORANGE, .shininess=SHININESS, .reflection=REFLECTION}
        },
        // green (14)
        {
                .center={0.f + RADIUS * 2.f, 1.f, 0.f + RADIUS_DIAG * 2.f}, .radius=1.f,
                .material={.color=GREEN, .shininess=SHININESS, .reflection=REFLECTION}
        },
        // brown (15)
        {
                .center={0.f + RADIUS * 4.f, 1.f, 0.f + RADIUS_DIAG * 2.f}, .radius=1.f,
                .material={.color=MAROON, .shininess=SHININESS, .reflection=REFLECTION}
        },
};

const plane_t PLANES[] = {
        // x/z-plane
        {
                .type=PLANE_UNBOUNDED, .point={0.f, 0.f, 0.f}, .normal={0.f, 1.f, 0.f},
                .material={
                        .color={.2f, .6f, .2f}, .shininess=-1.f,
                        .reflection={.type=NONE}
                }
        },
        {
                .type=PLANE_UNBOUNDED, .point={0.f, 0.f, 0.f + RADIUS_DIAG * 2.f + RADIUS}, .normal={0.f, 0.f, -1.f},
                .material={
                        .color={.2f, .6f, .2f}, .shininess=-1.f,
                        .reflection={.type=NONE}
                }
        }
};

#endif

/** Art scene definition */
#ifdef ART_SCENE

const uint32_t SIZE_X = 1000;
const uint32_t SIZE_Y = 1000;
const float FOV = (float) M_PI / 2.f;

const vec3f EYE = {0.f, 5.f, -40.f};
const vec3f TARGET = {0.f, -24.f, 0.f};
const vec3f UP = {0.f, 1.f, 0.f};

const vec3f BACKGROUND = BLACK;

const light_t LIGHTS[] = {
        {.type=LIGHT_AMBIENT, .intensity=.3f},
        {.type=LIGHT_POINT, .intensity=.9f, .v.location={0.f, 5.f, -41.f}},
};

const sphere_t SPHERES[] = {
        {
                .center={1.7f, .8f, -22.f}, .radius=.8f,
                .material={
                        .color=YELLOW, .shininess=10000.f,
                        .reflection={.type=REFRACTIVE, .fraction.refractiveness=.9f, .refractive_index=1.5f}
                }
        },
        {
                .center={1.7f, 1.8f, -31.f}, .radius=1.8f,
                .material={
                        .color=BLUE, .shininess=10000.f,
                        .reflection={.type=REFRACTIVE, .fraction.refractiveness=.9f, .refractive_index=1.5f}
                }
        },
        {
                .center={2.4f, 1.f, -36.f}, .radius=1.f,
                .material={
                        .color=GREEN, .shininess=10000.f,
                        .reflection={.type=REFRACTIVE, .fraction.refractiveness=.9f, .refractive_index=1.5f}
                }
        },
        {
                .center={-1.8f, .6f, -35.5f}, .radius=.6f,
                .material={
                        .color=RED, .shininess=10000.f,
                        .reflection={.type=REFRACTIVE, .fraction.refractiveness=.9f, .refractive_index=1.5f}
                }
        }
};

#define DISTANT {5.5f, 0.f, 8.f}

const plane_t PLANES[] = {
        // x/z-plane
        {
                .type=PLANE_UNBOUNDED, .point=DISTANT, .normal={0.f, 1.f, 0.f},
                .material={
                        .color=WHITE, .shininess=1.f,
                        .reflection={.type=NONE}
                },
                .checkered_xz=true, .checker_color=BLACK
        },
        // roof
        {
                .type=PLANE_UNBOUNDED, .point=DISTANT, .normal={0.f, -7.5f, -1.f},
                .material={
                        .color=WHITE, .shininess=-1.f,
                        .reflection={.type=REFLECTIVE, .fraction.reflectiveness=1.f}
                }
        },
        // right wall
        {
                .type=PLANE_UNBOUNDED, .point=DISTANT, .normal={-1.f, 0.f, 0.f},
                .material={
                        .color=WHITE, .shininess=-1.f,
                        .reflection={.type=REFLECTIVE, .fraction.reflectiveness=1.f}
                }
        },
        // left wall
        {
                .type=PLANE_UNBOUNDED, .point=DISTANT, .normal={5.f, 0.f, -1.f},
                .material={
                        .color=WHITE, .shininess=-1.f,
                        .reflection={.type=REFLECTIVE, .fraction.reflectiveness=1.f}
                }
        }
};

#endif

/** Profile scene definition */
#ifdef PROFILE_SCENE

const uint32_t SIZE_X = 500;
const uint32_t SIZE_Y = 500;
const float FOV = (float) M_PI / 10.f;

const vec3f EYE = {0.f, 0.f, -8.f};
const vec3f TARGET = {0.f, 0.f, 0.f};
const vec3f UP = {0.f, 1.f, 0.f};

const vec3f BACKGROUND = {1.f, 1.f, 1.f};

const light_t LIGHTS[] = {
        {.type=LIGHT_AMBIENT, .intensity=.6f},
        {.type=LIGHT_POINT, .intensity=.3f, .v.location={1.f, 1.f, 1.f}},
        {.type=LIGHT_DIRECTIONAL, .intensity=.1f, .v.direction={1.f, 1.f, 1.f}}
};

#define SHININESS 2.f
#define REFLECTION {.type=REFLECTIVE, .fraction.reflectiveness=.05f}

const sphere_t SPHERES[] = {
        {
                .center={0.f, 0.f, 0.f}, .radius=1.f,
                .material={
                        .color={187.f / 255.f, 209.f / 255.f, 135.f / 255.f},
                        .shininess=SHININESS, .reflection=REFLECTION}
        }
};

const plane_t PLANES[] = {};

#endif

const uint32_t LIGHTS_SIZE = sizeof(LIGHTS) / sizeof(light_t);
const uint32_t SPHERES_SIZE = sizeof(SPHERES) / sizeof(sphere_t);
const uint32_t PLANES_SIZE = sizeof(PLANES) / sizeof(plane_t);