#ifndef STRG_MUTIL_H
#define STRG_MUTIL_H

#include <limits.h>
#include <math.h>

// some number and math utilities :3

static inline int round_double_to_int(double x, int *out) {
    if (!isfinite(x))
        return 0;

    const long r = lround(x);

    if (r < INT_MIN || r > INT_MAX)
        return 0;

    *out = (int)r;
    return 1;
}

#endif //STRG_MUTIL_H