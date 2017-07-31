#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <whitgl/input.h>
#include <whitgl/logging.h>
#include <whitgl/math.h>
#include <whitgl/sound.h>
#include <whitgl/sys.h>
#include <whitgl/timer.h>
#include <whitgl/profile.h>
#include <whitgl/random.h>

#include <assets.h>
#include <heightmap.h>
#include <glider.h>
#include <thermal.h>



char _piece_shader[16384];
char _piece_shader2[16384];
whitgl_int _load_shader_piece(const char* filename, char* buffer, whitgl_int buffer_size)
{
	memset(buffer, 0, buffer_size);
	FILE *src;
	src = fopen(filename, "rb");
	if (src == NULL)
	{
		WHITGL_PANIC("Failed to open %s for load.", filename);
	}
	whitgl_int read = fread( buffer, 1, buffer_size, src );
	if(read >= buffer_size)
	{
		WHITGL_PANIC("Not enough room in buffer for", filename);
	}
	WHITGL_LOG("Loaded data from %s", filename);
	fclose(src);
	return read;
}

whitgl_bool side_of_plane(whitgl_fvec3 a, whitgl_fvec3 point_on_plane, whitgl_fvec3 plane_normal)
{
	whitgl_fvec3 direction = whitgl_fvec3_sub(a, point_on_plane);
	whitgl_float dot = whitgl_fvec3_dot(direction, plane_normal);
	return dot > 0;
}

int main()
{
	WHITGL_LOG("Starting main.");

	whitgl_sys_setup setup = whitgl_sys_setup_zero;
	setup.size.x = 1920/2;
	setup.size.y = 1080/2;
	setup.pixel_size = 1;
	setup.name = "game";
	setup.fullscreen = false;
	setup.resolution_mode = RESOLUTION_USE_WINDOW;
	setup.start_focused = !setup.fullscreen;

	WHITGL_LOG("Initiating sys");
	if(!whitgl_sys_init(&setup))
	{
		WHITGL_LOG("Failed to initiate sys");
		return 1;
	}

	// whitgl_ivec buffer_size = {setup.size.x*2, setup.size.y*2};
	// whitgl_resize_framebuffer(0, buffer_size, false);

	// _load_shader_piece("data/shader/super_sample.gl", _piece_shader, 16384);
	// whitgl_shader post_shader = whitgl_shader_zero;
	// post_shader.fragment_src = _piece_shader;
	// post_shader.uniforms[0].type = WHITGL_UNIFORM_FVEC;
	// post_shader.uniforms[0].name = "resolution";
	// post_shader.num_uniforms = 1;
	// if(!whitgl_change_shader( WHITGL_SHADER_POST, post_shader))
	// 	WHITGL_PANIC("failed to change shader");
	// whitgl_fvec resolution = {setup.size.x, setup.size.y};
	// whitgl_set_shader_fvec(WHITGL_SHADER_POST, 0, resolution);

	_load_shader_piece("data/shader/heightmap.gl", _piece_shader, 16384);
	whitgl_shader heightmap_shader = whitgl_shader_zero;
	heightmap_shader.fragment_src = _piece_shader;
	heightmap_shader.num_uniforms = 6;
	heightmap_shader.uniforms[0].type = WHITGL_UNIFORM_FVEC3;
	heightmap_shader.uniforms[0].name = "eye";
	heightmap_shader.uniforms[1].type = WHITGL_UNIFORM_COLOR;
	heightmap_shader.uniforms[1].name = "skycol";
	heightmap_shader.uniforms[2].type = WHITGL_UNIFORM_COLOR;
	heightmap_shader.uniforms[2].name = "lowcol";
	heightmap_shader.uniforms[3].type = WHITGL_UNIFORM_COLOR;
	heightmap_shader.uniforms[3].name = "medcol";
	heightmap_shader.uniforms[4].type = WHITGL_UNIFORM_COLOR;
	heightmap_shader.uniforms[4].name = "highcol";
	heightmap_shader.uniforms[5].type = WHITGL_UNIFORM_COLOR;
	heightmap_shader.uniforms[5].name = "boostcol";
	if(!whitgl_change_shader(WHITGL_SHADER_EXTRA_0, heightmap_shader))
		WHITGL_PANIC("failed to change shader");

	_load_shader_piece("data/shader/thermal.gl", _piece_shader, 16384);
	whitgl_shader thermal_shader = whitgl_shader_zero;
	thermal_shader.fragment_src = _piece_shader;
	thermal_shader.num_uniforms = 2;
	thermal_shader.uniforms[0].type = WHITGL_UNIFORM_FVEC3;
	thermal_shader.uniforms[0].name = "eye";
	thermal_shader.uniforms[1].type = WHITGL_UNIFORM_COLOR;
	thermal_shader.uniforms[1].name = "skycol";
	if(!whitgl_change_shader(WHITGL_SHADER_EXTRA_1, thermal_shader))
		WHITGL_PANIC("failed to change shader");

	_load_shader_piece("data/shader/connection.gl", _piece_shader, 16384);
	whitgl_shader connection_shader = whitgl_shader_zero;
	connection_shader.fragment_src = _piece_shader;
	connection_shader.num_uniforms = 2;
	connection_shader.uniforms[0].type = WHITGL_UNIFORM_FVEC3;
	connection_shader.uniforms[0].name = "eye";
	connection_shader.uniforms[1].type = WHITGL_UNIFORM_COLOR;
	connection_shader.uniforms[1].name = "skycol";
	if(!whitgl_change_shader(WHITGL_SHADER_EXTRA_2, thermal_shader))
		WHITGL_PANIC("failed to change shader");

	// whitgl_sys_color sky = {0x1e,0x21,0x55,0xff};
	// whitgl_sys_color sky = {0xff,0xee,0xc4};
	whitgl_sys_color sky = {0xff,0xe3,0xc3,0xff};
	whitgl_sys_color low = {0x9a,0x50,0x50,0xff};
	whitgl_sys_color med = {0xb9,0x8e,0x63,0xff};
	whitgl_sys_color high = {0xd2,0xdb,0xc3,0xff};
	whitgl_set_shader_color(WHITGL_SHADER_EXTRA_0, 1, sky);
	whitgl_set_shader_color(WHITGL_SHADER_EXTRA_0, 2, low);
	whitgl_set_shader_color(WHITGL_SHADER_EXTRA_0, 3, med);
	whitgl_set_shader_color(WHITGL_SHADER_EXTRA_0, 4, high);
	whitgl_set_shader_color(WHITGL_SHADER_EXTRA_1, 1, sky);
	whitgl_set_shader_color(WHITGL_SHADER_EXTRA_2, 1, sky);


	whitgl_sys_set_clear_color(sky);

	WHITGL_LOG("Initiating sound");
	whitgl_sound_init();
	WHITGL_LOG("Initiating input");
	whitgl_input_init();
	WHITGL_LOG("Initiating timer");
	whitgl_timer_init();

	ld39_world *world = malloc(sizeof(ld39_world));
	ld39_glider glider = ld39_glider_zero;
	glider.shake_seed = whitgl_random_seed_init(0);
	ld39_world_generate(world, glider.pos);
	whitgl_float time = 0;

	bool running = true;
	whitgl_int frame = 0;
	// whitgl_profile_should_report(true);

	whitgl_bool started = false;
	whitgl_float off = 0;

	whitgl_random_seed thermal_seed = whitgl_random_seed_init(2515);

	whitgl_load_model(MDL_TOWER, "data/model/tower.wmd");

	whitgl_loop_add(SOUND_WIND, "data/sound/wind.wav");
	whitgl_loop_set_paused(SOUND_WIND, false);

	whitgl_loop_add(SOUND_STALL, "data/sound/stall.wav");
	whitgl_loop_volume(SOUND_STALL, 0.00);
	whitgl_loop_set_paused(SOUND_STALL, false);

	whitgl_loop_add(SOUND_LIVE_WIRE, "data/sound/live_wire.wav");
	whitgl_loop_volume(SOUND_LIVE_WIRE, 0.00);
	whitgl_loop_set_paused(SOUND_LIVE_WIRE, false);

	whitgl_sound_add(SOUND_CRASH, "data/sound/crash.wav");
	whitgl_sound_add(SOUND_POWER_ON, "data/sound/power_on.wav");
	whitgl_sound_add(SOUND_BOOST, "data/sound/boost.wav");
	whitgl_sound_add(SOUND_THERMAL, "data/sound/thermal.wav");

	whitgl_sys_add_image(0, "data/sprites/sprites.png");

	while(running)
	{
		whitgl_sound_update();
		whitgl_timer_tick();
		while(whitgl_timer_should_do_frame(60))
		{
			whitgl_input_update();
			if(whitgl_input_pressed(WHITGL_INPUT_ESC))
				running = false;
			if(whitgl_sys_should_close())
				running = false;
			if(running == false)
				break;

			ld39_world_update(world, glider.pos);

			if(!started && whitgl_input_pressed(WHITGL_INPUT_A))
			{
				started = true;
				continue;
			}
			if(!started)
				continue;

			off += 1.0/60.0;
			glider = ld39_glider_update(glider);

			time += 1.0/60;

			whitgl_int i;
			for(i=0; i<MAX_ACTIVE_MAPS; i++)
			{
				if(!world->maps[i].active)
					continue;
				ld39_thermal thermal = world->maps[i].thermal;
				if(!thermal.active)
					continue;
				whitgl_fvec glider2d = {glider.pos.x, glider.pos.y};
				whitgl_fvec base2d = {thermal.base.x, thermal.base.y};
				whitgl_fvec diff = whitgl_fvec_sub(glider2d, base2d);
				if(thermal.active && whitgl_fvec_magnitude(diff) < thermal.radius && glider.pos.z > thermal.base.z && glider.pos.z < thermal.base.z+thermal.height)
				{
					if(whitgl_random_float(&thermal_seed) > 0.9)
						whitgl_sound_play(SOUND_THERMAL, whitgl_random_float(&thermal_seed)*0.1, whitgl_random_float(&thermal_seed)+0.5);
					glider.thermal_lift += 0.015;
				}
			}

			for(i=0; i<MAX_ACTIVE_MAPS; i++)
			{
				if(!world->maps[i].active)
					continue;
				ld39_tower tower = world->maps[i].tower;
				if(!tower.active)
					continue;
				whitgl_fvec3 mid_offset = {0,0,17};
				whitgl_float mid_radius = 4;
				whitgl_fvec3 mid_pos = whitgl_fvec3_add(tower.pos, mid_offset);
				whitgl_fvec3 diff = whitgl_fvec3_sub(mid_pos, glider.pos);

				whitgl_float mag = whitgl_fvec3_magnitude(diff);
				if(mag > mid_radius)
					continue;

				whitgl_fvec3 plane_normal = {0,1,0};
				whitgl_fmat rot = whitgl_fmat_rot_z(tower.rotate);
				plane_normal = whitgl_fvec3_apply_fmat(plane_normal, rot);
				whitgl_bool side_new = side_of_plane(glider.pos, tower.pos, plane_normal);
				whitgl_bool side_old = side_of_plane(glider.last_pos, tower.pos, plane_normal);
				if(side_new == side_old)
					continue;

				whitgl_bool connected = false;
				whitgl_int j;
				for(j=0; j<world->connections.num_connections; j++)
				{
					whitgl_fvec3 connection = world->connections.connections[j];
					if(whitgl_fvec3_eq(connection, tower.pos))
						connected = true;
					if(connected)
						break;
				}
				if(connected)
				{
					continue; // already in
				}

				whitgl_bool ready_to_connect = false;
				for(j=0; j<world->connections.num_connections; j++)
				{
					whitgl_fvec3 connection = world->connections.connections[j];
					whitgl_int k;
					for(k=0; k<tower.num_connections; k++)
					{
						if(whitgl_fvec3_eq(connection, tower.connections[k]))
							ready_to_connect = true;
					}
					if(ready_to_connect)
						break;
				}
				if(!ready_to_connect)
				{
					continue;
				}

				if(world->connections.num_connections < MAX_LIVE_CONNECTIONS)
					world->connections.connections[world->connections.num_connections++] = tower.pos;
				// WHITGL_LOG("YAY");
				whitgl_sound_play(SOUND_POWER_ON, 1.0, 1.0);
				if(glider.num_boosts < 3)
					glider.num_boosts++;
			}

			for(i=0; i<MAX_ACTIVE_MAPS; i++)
			{
				if(!world->maps[i].active)
					continue;
				ld39_tower tower = world->maps[i].tower;
				if(!tower.active)
					continue;
				whitgl_fvec3 mid_offset = {0,0,17};
				whitgl_float mid_radius = 4;
				whitgl_float outer_radius = 5;
				whitgl_fvec3 mid_pos = whitgl_fvec3_add(tower.pos, mid_offset);
				whitgl_fvec3 diff = whitgl_fvec3_sub(mid_pos, glider.pos);

				whitgl_float mag = whitgl_fvec3_magnitude(diff);
				if(mag < mid_radius || mag > outer_radius)
					continue;
				whitgl_fvec3 plane_normal = {0,1,0};
				whitgl_fmat rot = whitgl_fmat_rot_z(tower.rotate);
				plane_normal = whitgl_fvec3_apply_fmat(plane_normal, rot);

				whitgl_fvec3 direction = whitgl_fvec3_sub(glider.pos, tower.pos);
				whitgl_float dot = whitgl_fvec3_dot(direction, plane_normal);
				if(dot < 1 || dot > 1)
					continue;

				glider.alive = false;
				glider.camera_shake += 10;
				glider.speed = whitgl_fvec3_scale_val(glider.speed, -0.5);
				glider.pos = glider.last_pos;
				whitgl_sound_play(SOUND_CRASH, 1.0, 1.0);

			}

			for(i=0; i<MAX_ACTIVE_MAPS; i++)
			{
				if(!world->maps[i].active)
					continue;
				ld39_tower tower = world->maps[i].tower;
				if(!tower.active)
					continue;
				whitgl_fvec3 mid_offset = {0,0,17};
				whitgl_float mid_radius = 4;
				whitgl_fvec3 mid_pos = whitgl_fvec3_add(tower.pos, mid_offset);
				whitgl_fvec3 diff = whitgl_fvec3_sub(mid_pos, glider.pos);

				whitgl_float mag = whitgl_fvec3_magnitude(diff);
				if(mag < mid_radius)
					continue;

				if(glider.pos.z > tower.pos.z+25)
					continue;

				whitgl_fvec glider2d = {glider.pos.x, glider.pos.y};
				whitgl_fvec tower2d = {tower.pos.x, tower.pos.y};
				whitgl_fvec diff2d = whitgl_fvec_sub(glider2d, tower2d);
				if(whitgl_fvec_magnitude(diff2d) > 0.5)
					continue;

				glider.alive = false;
				glider.camera_shake += 10;
				glider.speed = whitgl_fvec3_scale_val(glider.speed, -0.5);
				glider.pos = glider.last_pos;
				whitgl_sound_play(SOUND_CRASH, 1.0, 1.0);

			}

			whitgl_float live_wire_vol = 0;
			for(i=0; i<MAX_ACTIVE_MAPS; i++)
			{
				if(!world->maps[i].active)
					continue;
				ld39_tower tower = world->maps[i].tower;
				if(!tower.active)
					continue;
				whitgl_fvec3 mid_offset = {0,0,17};
				whitgl_float sound_radius = 64;
				whitgl_fvec3 mid_pos = whitgl_fvec3_add(tower.pos, mid_offset);
				whitgl_fvec3 diff = whitgl_fvec3_sub(mid_pos, glider.pos);

				whitgl_float mag = whitgl_fvec3_magnitude(diff);
				if(mag > sound_radius)
					continue;

				whitgl_bool we_active = false;
				whitgl_int j;
				for(j=0; j<world->connections.num_connections; j++)
				{
					if(whitgl_fvec3_eq(world->connections.connections[j], tower.pos))
						we_active = true;
				}
				if(!we_active)
					continue;
				live_wire_vol += whitgl_fpow(1-mag/sound_radius, 2);
			}
			whitgl_loop_volume(SOUND_LIVE_WIRE, live_wire_vol*0.5);


			whitgl_float air_speed = whitgl_fvec3_magnitude(glider.speed);
			whitgl_float joystick_speed = whitgl_fvec_magnitude(glider.joystick);
			whitgl_float factor = air_speed/16+joystick_speed/8;
			whitgl_loop_frequency(0, 0.2+factor);
			whitgl_loop_volume(0, (0.2+factor)*0.5);

			if(glider.forward_speed < 2)
				whitgl_loop_volume(SOUND_STALL, (2-glider.forward_speed)*0.02);
			else
				whitgl_loop_volume(SOUND_STALL, 0);

		}
		if(running == false)
			break;

		whitgl_set_shader_fvec3(WHITGL_SHADER_EXTRA_0, 0, glider.pos);
		whitgl_set_shader_fvec3(WHITGL_SHADER_EXTRA_1, 0, glider.pos);
		whitgl_set_shader_fvec3(WHITGL_SHADER_EXTRA_2, 0, glider.pos);

		whitgl_sys_draw_init(0);
		whitgl_fmat view = ld39_glider_onboard_camera(glider);
		whitgl_fmat perspective = whitgl_fmat_perspective(whitgl_tau/4, (float)setup.size.x/(float)setup.size.y, 0.1f, 1024.0f);

		ld39_world_draw(world, time, view, perspective);
		if(started)
			ld39_glider_draw_meters(glider, setup.size);

		whitgl_float off_y = -setup.size.y*whitgl_fpow(off,2);
		whitgl_sprite text_sprite = {0, {0,0}, {5*16,5*16}};
		whitgl_ivec title_1 = {setup.size.x/2-(text_sprite.size.x/2.0)*8, 16*2+off_y};
		whitgl_sys_draw_text(text_sprite, "zephyria", title_1);
		whitgl_ivec title_2 = {setup.size.x/2-(text_sprite.size.x/2.0)*6, 16*2+16+text_sprite.size.y+off_y};
		whitgl_sys_draw_text(text_sprite, "planum", title_2);
 // Planum
		whitgl_sys_draw_finish();

		frame++;

		// if(time >= 2 && time < 12)
		// {
		// if(frame%2)
		// {
		// 	char buffer[1024];
		// 	snprintf(buffer, 1024, "gif_%04d.png", (int)frame/2);
		// 	WHITGL_LOG("buffer %s", buffer);
		// 	whitgl_sys_capture_frame(buffer);
		// }
	}

	free(world);

	WHITGL_LOG("Shutting down input");
	whitgl_input_shutdown();
	WHITGL_LOG("Shutting down sound");
	whitgl_sound_shutdown();

	whitgl_sys_close();

	return 0;
}
