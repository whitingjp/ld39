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
	heightmap_shader.num_uniforms = 5;
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

	// whitgl_sys_color sky = {0x1e,0x21,0x55,0xff};
	// whitgl_sys_color sky = {0xff,0xee,0xc4};
	whitgl_sys_color sky = {0x19,0x05,0x02,0xff};
	whitgl_sys_color low = {0x9a,0x50,0x50,0xff};
	whitgl_sys_color med = {0xb9,0x8e,0x63,0xff};
	whitgl_sys_color high = {0xd2,0xdb,0xc3,0xff};
	whitgl_set_shader_color(WHITGL_SHADER_EXTRA_0, 1, sky);
	whitgl_set_shader_color(WHITGL_SHADER_EXTRA_0, 2, low);
	whitgl_set_shader_color(WHITGL_SHADER_EXTRA_0, 3, med);
	whitgl_set_shader_color(WHITGL_SHADER_EXTRA_0, 4, high);
	whitgl_set_shader_color(WHITGL_SHADER_EXTRA_1, 1, sky);


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
			glider = ld39_glider_update(glider);
			ld39_world_update(world, glider.pos);
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
					glider.thermal_lift += 0.01;
				}
			}
		}
		if(running == false)
			break;
		whitgl_sys_draw_init(0);
		whitgl_fmat view = ld39_glider_onboard_camera(glider);
		whitgl_fmat perspective = whitgl_fmat_perspective(whitgl_tau/4, (float)setup.size.x/(float)setup.size.y, 0.1f, 1024.0f);

		ld39_world_draw(world, time, view, perspective);
		ld39_glider_draw_meters(glider, setup.size);
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
