#ifndef WHITGL_LD39_THERMAL_H_
#define WHITGL_LD39_THERMAL_H_

#include <whitgl/math.h>

typedef struct
{
	whitgl_fvec3 base;
	whitgl_float height;
	whitgl_float radius;
} ld39_thermal;

static const ld39_thermal ld39_thermal_zero = {{1024*6,1024*6-16,24},16,4};

void ld39_thermal_draw(ld39_thermal thermal, whitgl_float time, whitgl_fmat view, whitgl_fmat perspective);

#endif // WHITGL_LD39_THERMAL_H_
