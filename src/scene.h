#ifndef RAY_TRACER_SCENE_H
#define RAY_TRACER_SCENE_H

#include "util.h"

/** Color definitions */
#define RED    {1.f, 0.f, 0.f}
#define BLUE   {0.f, 0.f, 1.f}
#define GREEN  {0.f, 1.f, 0.f}
#define BLACK  {0.f, 0.f, 0.f}
#define WHITE  {1.f, 1.f, 1.f}
#define ORANGE {1.f, .5f, 0.f}
#define VIOLET {.5f, .0f, 1.f}
#define MAROON {.5f, .0f, 0.f}
#define YELLOW {1.f, 1.f, 0.f}

/** define exactly one scene */
//#define WHITTED_SCENE
#define POOL_SCENE

/** scene-specific definitions */
extern const uint32_t SIZE_X;    // number of pixels in horizontal direction
extern const uint32_t SIZE_Y;    // number of pixels in vertical direction
extern const float FOV;          // field of view (radians)

extern const vec3f EYE;          // eye position
extern const vec3f TARGET;       // lookat position
extern const vec3f UP;           // up-vector

extern const vec3f BACKGROUND;   // color of the background

extern const light_t LIGHTS[];   // the lights in the scene
extern const sphere_t SPHERES[]; // the spheres in the scene
extern const plane_t PLANES[];   // the planes in the scene

/** workaround to obtain the size of the extern const array */
extern const uint32_t LIGHTS_SIZE;
extern const uint32_t SPHERES_SIZE;
extern const uint32_t PLANES_SIZE;

#endif //RAY_TRACER_SCENE_H
