#ifndef WHITGL_LD39_GLIDER_H_
#define WHITGL_LD39_GLIDER_H_

#include <whitgl/math.h>

typedef struct
{
	whitgl_fvec3 pos;
	whitgl_quat facing;
	whitgl_fvec3 speed;
	whitgl_float forward_speed;
} ld39_glider;

static const ld39_glider ld39_glider_zero = {{64,64,12},{0,0,0,1},{0,-1,0},1};
ld39_glider ld39_glider_update(ld39_glider glider);
whitgl_fmat ld39_glider_onboard_camera(ld39_glider glider);

#endif // WHITGL_LD39_GLIDER_H_
