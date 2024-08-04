#ifndef CARTESIAN_H_
#define CARTESIAN_H_
#include <math.h>
typedef struct {
    double x, y;
} v2;

void rotate(v2 *point, float angle);

#endif
