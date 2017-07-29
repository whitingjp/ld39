#include "glider.h"

#include <whitgl/input.h>
#include <whitgl/logging.h>
#include <whitgl/sys.h>
#include <heightmap.h>


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
	glider.pos = whitgl_fvec3_add(glider.pos, whitgl_fvec3_scale_val(glider.speed, 1.0/120));

	whitgl_fvec joystick = whitgl_input_joystick();
	glider.joystick = whitgl_fvec_interpolate(joystick, glider.joystick, 0.95);

	whitgl_fvec3 roll_axis = {0,1,0};
	whitgl_fvec3 pitch_axis = {0,0,1};
	whitgl_quat roll = whitgl_quat_rotate(-glider.joystick.x/64, roll_axis);
	whitgl_quat pitch = whitgl_quat_rotate(glider.joystick.y/64, pitch_axis);
	glider.facing = whitgl_quat_multiply(pitch, glider.facing);
	glider.facing = whitgl_quat_multiply(roll, glider.facing);

	whitgl_fmat rot = whitgl_quat_to_fmat(glider.facing);
	whitgl_fvec3 forward = {0,1,0};
	whitgl_fvec3 up = {0,0,1};

	forward = whitgl_fvec3_apply_fmat(forward, rot);
	up = whitgl_fvec3_apply_fmat(up, rot);
	glider.speed = whitgl_fvec3_interpolate(glider.speed, whitgl_fvec3_scale_val(forward, glider.forward_speed), 0.01);

	whitgl_fvec3 gravity = {0,0,-1};
	whitgl_float gravity_dot = whitgl_fvec3_dot(glider.speed, gravity);

	whitgl_float old_forward_speed = glider.forward_speed;
	glider.forward_speed = glider.forward_speed+gravity_dot/512;
	glider.forward_speed = glider.forward_speed*0.9999;
	glider.forward_speed_change = glider.forward_speed - old_forward_speed;

	// WHITGL_LOG("glider.forward_speed %.2f", glider.forward_speed);
	if(glider.forward_speed < 2)
	{
		whitgl_float stall_factor = whitgl_fpow(1-glider.forward_speed/2, 2);
		whitgl_quat stall_down = whitgl_quat_rotate(-whitgl_tau/4, pitch_axis);
		// WHITGL_LOG("stall_factor %.2f", stall_factor);
		glider.facing = whitgl_quat_slerp(glider.facing, stall_down, stall_factor/20);
	}

	if(whitgl_input_pressed(WHITGL_INPUT_A) && glider.boost <= 0)
		glider.boost = 1.0;

	if(glider.boost > 0)
	{
		glider.boost -= 1.0/30;
		whitgl_float boost_factor = 1-whitgl_fpow(glider.boost*2-1,2);
		glider.forward_speed += 0.1*boost_factor;
	}


	whitgl_set_shader_fvec3(WHITGL_SHADER_EXTRA_0, 0, glider.pos);

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

void ld39_glider_draw_meters(ld39_glider glider, whitgl_ivec setup_size)
{
	whitgl_sys_enable_depth(false);
	whitgl_int border = setup_size.x/32;
	whitgl_iaabb altimeter_box = {{border, border}, {setup_size.x/16+border,setup_size.y-border}};
	whitgl_sys_color altimeter_bg = {0xff,0xff,0xff,0x40};
	whitgl_sys_draw_iaabb(altimeter_box, altimeter_bg);

	whitgl_float max_altitude = 55.5;
	whitgl_float altimeter_pos = whitgl_finterpolate(altimeter_box.b.y, altimeter_box.a.y, glider.pos.z/max_altitude);
	whitgl_iaabb altimeter_player = {{altimeter_box.a.x, altimeter_pos-border/16}, {altimeter_box.b.x, altimeter_pos+border/16}};
	whitgl_sys_color altimeter_player_col = {0xff,0xff,0xff,0xcc};
	whitgl_sys_draw_iaabb(altimeter_player, altimeter_player_col);

	whitgl_float altitude_speed = glider.speed.z*10;
	whitgl_int center_altimer_box_x = (altimeter_box.a.x+altimeter_box.b.x)/2;
	whitgl_iaabb altimeter_player_speed = {{center_altimer_box_x-border/16, altimeter_pos-border/16}, {center_altimer_box_x+border/16, altimeter_pos+border/16}};
	if(altitude_speed > 0)
		altimeter_player_speed.a.y -= altitude_speed;
	else
		altimeter_player_speed.b.y -= altitude_speed;
	whitgl_sys_draw_iaabb(altimeter_player_speed, altimeter_player_col);

	whitgl_float ground_height = stacked_perlin2d(glider.pos.x, glider.pos.y, 0);
	whitgl_float altimeter_ground_pos = whitgl_finterpolate(altimeter_box.b.y, altimeter_box.a.y, ground_height/max_altitude);
	whitgl_iaabb altimeter_ground = {{altimeter_box.a.x, altimeter_ground_pos}, {altimeter_box.b.x, altimeter_box.b.y}};
	whitgl_sys_color altimeter_ground_col = {0xff,0x00,0x00,0x40};
	whitgl_sys_draw_iaabb(altimeter_ground, altimeter_ground_col);

	whitgl_iaabb velocity_box = {{setup_size.x-border-setup_size.x/16, border}, {setup_size.x-border,setup_size.y-border}};
	whitgl_sys_draw_iaabb(velocity_box, altimeter_bg);
	whitgl_float max_velocity = 10;
	whitgl_float velocity_pos = whitgl_finterpolate(velocity_box.b.y, velocity_box.a.y, glider.forward_speed/max_velocity);
	whitgl_iaabb velocity_player = {{velocity_box.a.x, velocity_pos-border/16}, {velocity_box.b.x, velocity_pos+border/16}};
	whitgl_sys_color velocity_player_col = {0xff,0xff,0xff,0xcc};
	whitgl_sys_draw_iaabb(velocity_player, velocity_player_col);

	whitgl_float forward_speed_change = glider.forward_speed_change*5000;
	whitgl_int center_velocity_box_x = (velocity_box.a.x+velocity_box.b.x)/2;
	whitgl_iaabb velocity_speed_change = {{center_velocity_box_x-border/16, velocity_pos-border/16}, {center_velocity_box_x+border/16, velocity_pos+border/16}};
	if(forward_speed_change > 0)
		velocity_speed_change.a.y -= forward_speed_change;
	else
		velocity_speed_change.b.y -= forward_speed_change;
	whitgl_sys_draw_iaabb(velocity_speed_change, altimeter_player_col);

	whitgl_float stall_speed = 2;
	whitgl_float stall_pos = whitgl_finterpolate(altimeter_box.b.y, altimeter_box.a.y, stall_speed/max_velocity);
	whitgl_iaabb velocity_stall = {{velocity_box.a.x, stall_pos}, {velocity_box.b.x, velocity_box.b.y}};
	whitgl_sys_color velocity_stall_col = {0xff,0x00,0x00,0x40};
	whitgl_sys_draw_iaabb(velocity_stall, velocity_stall_col);

}
