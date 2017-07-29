#ifndef WHITGL_LD39_GLIDER_H_
#define WHITGL_LD39_GLIDER_H_

#include <whitgl/math.h>
#include <whitgl/random.h>


typedef struct
{
	whitgl_fvec3 pos;
	whitgl_quat facing;
	whitgl_fvec3 speed;
	whitgl_float forward_speed;
	whitgl_float forward_speed_change;
	whitgl_fvec joystick;

	whitgl_float boost;
	whitgl_float camera_shake;
	whitgl_random_seed shake_seed;
	whitgl_fvec3 shake_offset;
	whitgl_fvec3 target_shake_offset;
	whitgl_float stall_factor;
} ld39_glider;

static const ld39_glider ld39_glider_zero = {{1024*6,1024*6,32},{0,0,0,1},{0,-1,0},3,0,{0,0}, 0, 0, whitgl_random_seed_zero, {0,0,0}, {0,0,0}, 0};
ld39_glider ld39_glider_update(ld39_glider glider);
whitgl_fmat ld39_glider_onboard_camera(ld39_glider glider);
void ld39_glider_draw_meters(ld39_glider glider, whitgl_ivec setup_size);

#endif // WHITGL_LD39_GLIDER_H_
