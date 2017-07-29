#ifndef WHITGL_LD39_GLIDER_H_
#define WHITGL_LD39_GLIDER_H_

#include <whitgl/math.h>

typedef struct
{
	whitgl_fvec3 pos;
	whitgl_quat facing;
	whitgl_fvec3 speed;
	whitgl_float forward_speed;
	whitgl_float forward_speed_change;
	whitgl_fvec joystick;

	whitgl_float boost;
} ld39_glider;

static const ld39_glider ld39_glider_zero = {{1024*8,1024*8,32},{0,0,0,1},{0,-1,0},3,0,{0,0}, 0};
ld39_glider ld39_glider_update(ld39_glider glider);
whitgl_fmat ld39_glider_onboard_camera(ld39_glider glider);
void ld39_glider_draw_meters(ld39_glider glider, whitgl_ivec setup_size);

#endif // WHITGL_LD39_GLIDER_H_
