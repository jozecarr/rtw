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
const double infinity = std::numeric_limits<double>::max();
const double pi = 3.1415926535897932385;

// Utility Functions

inline double degrees_to_radians(double degrees) {
    return degrees * pi / 180.0;
}

inline double random_double() {
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
    return (x >> 11) * (1.0 / 9007199254740992.0);
}

inline double random_double(double min, double max) {
    // Returns a random real in [min,max)
    return min + (max-min)*random_double();
}

inline int random_int(int min, int max) {
    // returns random int in [min, max]
    return int(random_double(min, max + 1));
}

// Common Headers

#include "colour.h"
#include "interval.h"
#include "ray.h"
#include "vec3.h"

#endif
