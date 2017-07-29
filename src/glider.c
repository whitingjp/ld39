#include "glider.h"

#include <whitgl/input.h>

ld39_glider ld39_glider_update(ld39_glider glider)
{
	glider.pos = whitgl_fvec3_add(glider.pos, glider.speed);

	whitgl_fvec joystick = whitgl_input_joystick();

	whitgl_fvec3 forward = {0,1,0};
	whitgl_fvec3 side = {0,0,1};
	// whitgl_fmat rot = whitgl_quat_to_fmat(glider.facing);
	// forward = whitgl_fvec3_apply_fmat(forward, rot);
	// side = whitgl_fvec3_apply_fmat(side, rot);

			// whitgl_fvec3 u = {0,-1,0};
			// whitgl_fvec3 r = {1,0,0};
			// whitgl_fvec3 f = {0,0,1};
			// whitgl_fvec target_jrot = {joy2.x*joy2.x, joy2.y*joy2.y};
			// if(joy2.x < 0) target_jrot.x *= -1;
			// if(joy2.y < 0) target_jrot.y *= -1;
			// jrot = whitgl_fvec_interpolate(jrot, target_jrot, 0.2);
			// orientation = whitgl_quat_multiply(whitgl_quat_rotate(jrot.y/48, f), orientation);
			// orientation = whitgl_quat_multiply(whitgl_quat_rotate(jrot.x/48, u), orientation);
	whitgl_quat roll = whitgl_quat_rotate(-joystick.x/64, forward);
	whitgl_quat pitch = whitgl_quat_rotate(joystick.y/64, side);
	glider.facing = whitgl_quat_multiply(pitch, glider.facing);
	glider.facing = whitgl_quat_multiply(roll, glider.facing);


	whitgl_fmat rot = whitgl_quat_to_fmat(glider.facing);
	glider.speed = whitgl_fvec3_scale_val(whitgl_fvec3_apply_fmat(forward, rot),0.1);


	return glider;
}
whitgl_fmat ld39_glider_onboard_camera(ld39_glider glider)
{
	whitgl_fvec3 eye = glider.pos;
	whitgl_fvec3 forward = {0,1,0};
	whitgl_fvec3 up = {0,0,1};

	whitgl_fmat rot = whitgl_quat_to_fmat(glider.facing);
	forward = whitgl_fvec3_apply_fmat(forward, rot);
	up = whitgl_fvec3_apply_fmat(up, rot);

	whitgl_fvec3 center = whitgl_fvec3_add(eye, forward);
	whitgl_fmat view = whitgl_fmat_lookAt(eye, center, up);
	return view;
}
