#ifndef LD39_TOWER_H_
#define LD39_TOWER_H_

#include <whitgl/math.h>

typedef struct
{
	whitgl_bool active;
	whitgl_fvec3 pos;
	whitgl_float rotate;
} ld39_tower;
static const ld39_tower ld39_tower_zero = {false, {0,0,0}, 0};


void ld39_tower_draw(ld39_tower tower, whitgl_fmat view, whitgl_fmat perspective);

#endif // LD39_TOWER_H_
