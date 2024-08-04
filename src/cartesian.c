#include "cartesian.h"
void rotate(v2 *point, float angle) {
    double x = point->x, y = point->y;
    point->x = x * cos(angle) - y * sin(angle);
    point->y = x * sin(angle) + y * cos(angle);
}
