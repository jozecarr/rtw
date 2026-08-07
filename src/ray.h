#ifndef RAY_H
#define RAY_H

#include "vec3.h"

class ray {
public:
    ray() {}

    ray(const point3& origin, const vec3& direction, float time) 
        : orig(origin), dir(direction), tm(time) {}

    ray(const point3& origin, const vec3& direction) 
        : ray(origin, direction, 0) {}

    const point3 &origin() const { return orig; }
    const vec3 &direction() const { return dir; }

    float time() const { return tm; }

    point3 at(float t) const {
        return orig + t*dir;
    }

private:
    point3 orig;
    vec3 dir;
    float tm;
};

#endif