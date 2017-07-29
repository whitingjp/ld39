#include "glider.h"

#include <whitgl/input.h>
#include <whitgl/logging.h>


	// more realistic maths that I couldn't get feeling right
	// whitgl_fmat rot = whitgl_quat_to_fmat(glider.facing);
	// whitgl_fvec3 forward = {0,1,0};
	// whitgl_fvec3 up = {0,0,1};
	// forward = whitgl_fvec3_apply_fmat(forward, rot);
	// up = whitgl_fvec3_apply_fmat(up, rot);

	// whitgl_float direction_dot = whitgl_fvec3_dot(whitgl_fvec3_normalize(glider.speed), forward);
	// WHITGL_LOG("direction_dot %.2f", direction_dot);

	// whitgl_float speed_mag = whitgl_fvec3_magnitude(glider.speed);
	// whitgl_float drag_factor = 0.0001/direction_dot;
	// whitgl_fvec3 drag = whitgl_fvec3_scale_val(forward, -drag_factor*speed_mag*speed_mag);
	// whitgl_float lift_factor = 0.0007/direction_dot;
	// whitgl_fvec3 lift = whitgl_fvec3_scale_val(up, lift_factor*speed_mag*speed_mag);
	// whitgl_fvec3 gravity = {0,0,-9.8/6000};
	// glider.speed = whitgl_fvec3_add(glider.speed, drag);
	// glider.speed = whitgl_fvec3_add(glider.speed, lift);
	// glider.speed = whitgl_fvec3_add(glider.speed, gravity);

ld39_glider ld39_glider_update(ld39_glider glider)
{
	glider.pos = whitgl_fvec3_add(glider.pos, whitgl_fvec3_scale_val(glider.speed, 1.0/60));

	whitgl_fvec joystick = whitgl_input_joystick();

	whitgl_fvec3 roll_axis = {0,1,0};
	whitgl_fvec3 pitch_axis = {0,0,1};
	whitgl_quat roll = whitgl_quat_rotate(-joystick.x/64, roll_axis);
	whitgl_quat pitch = whitgl_quat_rotate(joystick.y/64, pitch_axis);
	glider.facing = whitgl_quat_multiply(pitch, glider.facing);
	glider.facing = whitgl_quat_multiply(roll, glider.facing);


	whitgl_fmat rot = whitgl_quat_to_fmat(glider.facing);
	whitgl_fvec3 forward = {0,1,0};
	whitgl_fvec3 up = {0,0,1};

	forward = whitgl_fvec3_apply_fmat(forward, rot);
	up = whitgl_fvec3_apply_fmat(up, rot);
	glider.speed = whitgl_fvec3_interpolate(glider.speed, whitgl_fvec3_scale_val(forward, glider.forward_speed), 0.02);

	whitgl_fvec3 gravity = {0,0,-1};
	whitgl_float gravity_dot = whitgl_fvec3_dot(glider.speed, gravity);
	glider.forward_speed = glider.forward_speed+gravity_dot/100;
	glider.forward_speed = glider.forward_speed*0.999;

	if(whitgl_input_held(WHITGL_INPUT_A))
		glider.forward_speed += 0.1;




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
