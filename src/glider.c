#include "glider.h"

#include <whitgl/input.h>
#include <whitgl/logging.h>
#include <whitgl/sound.h>
#include <whitgl/sys.h>

#include <assets.h>
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
	if(whitgl_random_float(&glider.shake_seed) > 0.9)
		glider.target_shake_offset.x = (whitgl_random_float(&glider.shake_seed)*2-1)/10;
	if(whitgl_random_float(&glider.shake_seed) > 0.9)
		glider.target_shake_offset.y = (whitgl_random_float(&glider.shake_seed)*2-1)/10;
	if(whitgl_random_float(&glider.shake_seed) > 0.9)
		glider.target_shake_offset.z = (whitgl_random_float(&glider.shake_seed)*2-1)/10;
	glider.shake_offset = whitgl_fvec3_interpolate(glider.shake_offset, glider.target_shake_offset, 0.1);
	if(!glider.alive)
	{
		glider.last_pos = glider.pos;
		glider.pos = whitgl_fvec3_add(glider.pos, whitgl_fvec3_scale_val(glider.speed, 1.0/120));
		glider.speed.z -= 0.02;
		glider.camera_shake = glider.camera_shake*0.95;
		whitgl_float ground_height = stacked_perlin2d(glider.pos.x, glider.pos.y, 0);
		whitgl_float height = glider.pos.z-ground_height;
		whitgl_float velocity = whitgl_fvec3_magnitude(glider.speed);
		if(height < 0.5)
		{
			glider.speed = whitgl_fvec3_scale_val(glider.speed, 0.5);
			glider.speed.z = -glider.speed.z;
			glider.pos = glider.last_pos;
			if(velocity > 1)
			{
				whitgl_sound_play(SOUND_CRASH, 0.2, 0.75);
				glider.camera_shake += 2;
			}
		}
		if(velocity < 0.1) velocity = 0;
		whitgl_fvec3 pitch_axis = {0,0,1};
		whitgl_quat pitch = whitgl_quat_rotate(-velocity/32, pitch_axis);
		glider.facing = whitgl_quat_multiply(pitch, glider.facing);
		return glider;
	}

	glider.camera_shake = 0;
	glider.last_pos = glider.pos;
	glider.pos = whitgl_fvec3_add(glider.pos, whitgl_fvec3_scale_val(glider.speed, 1.0/120));
	glider.pos.z += glider.thermal_lift/120;
	glider.thermal_lift = glider.thermal_lift*0.99;

	whitgl_fvec joystick = whitgl_fvec_scale_val(whitgl_input_joystick(), glider.forward_speed/5);
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
	whitgl_float roll_dot = whitgl_fvec3_dot(up, pitch_axis);
	whitgl_float roll_descent = (roll_dot-1)/2;
	glider.speed.z += roll_descent/100;

	whitgl_float ground_height = stacked_perlin2d(glider.pos.x, glider.pos.y, 0);
	whitgl_float height = glider.pos.z-ground_height;
	whitgl_float drag_factor = height < 4 ? 1 : 0.9998;
	if(height < 0.5)
	{
		whitgl_sound_play(SOUND_CRASH, 1.0, 1.0);
		glider.alive = false;
		glider.camera_shake += 10;
		glider.speed = whitgl_fvec3_scale_val(glider.speed, 0.5);
		glider.speed.z = -glider.speed.z;
		glider.pos = glider.last_pos;
		glider.pos.z += 1;
	}

	whitgl_float old_forward_speed = glider.forward_speed;
	glider.forward_speed = glider.forward_speed+gravity_dot/512;
	glider.forward_speed = glider.forward_speed*drag_factor;
	glider.forward_speed -= whitgl_fvec_magnitude(glider.joystick)/5000;

	if(glider.forward_speed > 7)
	{
		whitgl_float extra_deccel_factor = (glider.forward_speed-7);
		extra_deccel_factor = whitgl_fpow(extra_deccel_factor, 2);
		glider.camera_shake += extra_deccel_factor*4;
		glider.forward_speed -= extra_deccel_factor*0.1;
	}

	glider.forward_speed_change = glider.forward_speed - old_forward_speed;

	glider.stall_factor = 0;
	if(glider.forward_speed < 2)
	{
		glider.stall_factor = whitgl_fpow(1-glider.forward_speed/2, 2);
		whitgl_quat stall_down = whitgl_quat_rotate(-whitgl_tau/4, pitch_axis);
		glider.facing = whitgl_quat_slerp(glider.facing, stall_down, glider.stall_factor/20);
		glider.camera_shake += glider.stall_factor*2;
	}

	if(whitgl_input_pressed(WHITGL_INPUT_A) && glider.boost <= 0 && glider.num_boosts > 0)
	{
		glider.num_boosts--;
		glider.boost = 1.0;
		whitgl_sound_play(SOUND_BOOST, 1.0, 1.0);
	}

	if(glider.boost > 0)
	{
		glider.boost -= 1.0/240;
		whitgl_float boost_factor = 1-whitgl_fpow(glider.boost*2-1,2);
		glider.forward_speed += 0.01*boost_factor;
		glider.camera_shake += boost_factor*2;
	}

	whitgl_set_shader_fvec3(WHITGL_SHADER_EXTRA_0, 0, glider.pos);
	whitgl_set_shader_fvec3(WHITGL_SHADER_EXTRA_1, 0, glider.pos);
	whitgl_set_shader_fvec3(WHITGL_SHADER_EXTRA_2, 0, glider.pos);

	return glider;
}
whitgl_fmat ld39_glider_onboard_camera(ld39_glider glider)
{
	whitgl_fvec3 eye = glider.pos;

	whitgl_fvec3 forward = {0,1,0};
	whitgl_fvec3 up = {0,0,1};

	whitgl_fvec3 actual_shake = whitgl_fvec3_scale_val(glider.shake_offset, glider.camera_shake);

	whitgl_quat shake1 = whitgl_quat_rotate(actual_shake.x, up);
	whitgl_quat shake2 = whitgl_quat_rotate(actual_shake.y, forward);
	whitgl_quat view_facing = glider.facing;
	view_facing = whitgl_quat_multiply(shake1, view_facing);
	view_facing = whitgl_quat_multiply(shake2, view_facing);

	whitgl_fmat rot = whitgl_quat_to_fmat(view_facing);
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

	whitgl_float altitude_speed = (glider.speed.z+glider.thermal_lift)*10;
	whitgl_int center_altimer_box_x = (altimeter_box.a.x+altimeter_box.b.x)/2;
	whitgl_iaabb altimeter_player_speed = {{center_altimer_box_x-border/16, altimeter_pos-border/16}, {center_altimer_box_x+border/16, altimeter_pos+border/16}};
	if(altitude_speed > 0)
		altimeter_player_speed.a.y -= altitude_speed;
	else
		altimeter_player_speed.b.y -= altitude_speed;
	whitgl_sys_draw_iaabb(altimeter_player_speed, altimeter_player_col);

	whitgl_float ground_height = stacked_perlin2d(glider.pos.x, glider.pos.y, 0);
	whitgl_float altimeter_ground_pos = whitgl_finterpolate(altimeter_box.b.y, altimeter_box.a.y, (ground_height+0.5)/max_altitude);
	whitgl_iaabb altimeter_ground = {{altimeter_box.a.x, altimeter_ground_pos}, {altimeter_box.b.x, altimeter_box.b.y}};
	whitgl_sys_color altimeter_ground_col = {0xff,0x00,0x00,0x40};
	whitgl_sys_draw_iaabb(altimeter_ground, altimeter_ground_col);

	whitgl_float altimeter_ground_effect_pos = whitgl_finterpolate(altimeter_box.b.y, altimeter_box.a.y, (ground_height+4)/max_altitude);
	whitgl_iaabb altimeter_ground_effect = {{altimeter_box.a.x, altimeter_ground_effect_pos}, {altimeter_box.b.x, altimeter_ground_pos}};
	whitgl_sys_color altimeter_ground_effect_col = {0x00,0xff,0x00,0x40};
	whitgl_sys_draw_iaabb(altimeter_ground_effect, altimeter_ground_effect_col);

	whitgl_iaabb velocity_box = {{setup_size.x-border-setup_size.x/16, border}, {setup_size.x-border,setup_size.y-border}};
	whitgl_sys_draw_iaabb(velocity_box, altimeter_bg);
	whitgl_float max_velocity = 8;
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

	whitgl_float too_fast = 7;
	whitgl_float too_fast_pos = whitgl_finterpolate(altimeter_box.b.y, altimeter_box.a.y, too_fast/max_velocity);
	whitgl_iaabb too_fast_box = {{velocity_box.a.x, velocity_box.a.y}, {velocity_box.b.x, too_fast_pos}};
	whitgl_sys_color too_fast_col = {0xff,0xff,0x00,0x40};
	whitgl_sys_draw_iaabb(too_fast_box, too_fast_col);

	whitgl_int i;
	for(i=0; i<3; i++)
	{
		whitgl_float boost_divider = setup_size.x/8;
		whitgl_fcircle circle = {{boost_divider*(i+3), setup_size.y-setup_size.y/8}, setup_size.y/16};
		whitgl_sys_draw_fcircle(circle, altimeter_bg, 32);

		whitgl_sys_color live_connection_col = {0xef,0x2b,0xa8,0x80};
		if(glider.num_boosts > i)
			whitgl_sys_draw_fcircle(circle, live_connection_col, 32);
		if(glider.num_boosts == i)
		{
			circle.size *= whitgl_fpow(glider.boost, 2);
			whitgl_sys_draw_fcircle(circle, live_connection_col, 32);
		}
	}

}
