#ifndef COMMONS_H
#define COMMONS_H

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <memory>
#include <chrono>
#include <cstdint>


// C++ Std Usings

using std::make_shared;
using std::shared_ptr;

// Constants

// Use max() instead of infinity to avoid UB under fast-math-style flags.
const float infinity = std::numeric_limits<float>::max();
const float pi = 3.1415926535897932385;

// Utility Functions

inline float degrees_to_radians(float degrees) {
    return degrees * pi / 180.0;
}

inline float random_float() {
    // Returns a random real in [0,1)
    // xorshift64*; fast thread-local RNG state.
    static thread_local uint64_t state = 0;
    if (state == 0) {
        auto seed = static_cast<uint64_t>(
            std::chrono::high_resolution_clock::now().time_since_epoch().count()
        );
        seed ^= static_cast<uint64_t>(
            reinterpret_cast<uintptr_t>(&state)
        );
        state = seed ? seed : 0x9E3779B97F4A7C15ULL;
    }
    uint64_t x = state;
    x ^= x >> 12;
    x ^= x << 25;
    x ^= x >> 27;
    x *= 0x2545F4914F6CDD1DULL;
    state = x;
    return (x >> 40) * (1.0f / 16777216.0f);
}

inline float random_float(float min, float max) {
    // Returns a random real in [min,max)
    return min + (max-min)*random_float();
}

inline int random_int(int min, int max) {
    // returns random int in [min, max]
    return int(random_float(min, max + 1));
}

// Common Headers

#include "colour.h"
#include "interval.h"
#include "ray.h"
#include "vec3.h"

#endif
