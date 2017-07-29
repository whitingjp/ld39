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
	heightmap_shader.num_uniforms = 1;
	heightmap_shader.uniforms[0].type = WHITGL_UNIFORM_FVEC3;
	heightmap_shader.uniforms[0].name = "eye";
	if(!whitgl_change_shader(WHITGL_SHADER_EXTRA_0, heightmap_shader))
		WHITGL_PANIC("failed to change shader");

	whitgl_sys_color sky = {0x1e,0x21,0x55,0xff};
	whitgl_sys_set_clear_color(sky);

	WHITGL_LOG("Initiating sound");
	whitgl_sound_init();
	WHITGL_LOG("Initiating input");
	whitgl_input_init();
	WHITGL_LOG("Initiating timer");
	whitgl_timer_init();

	ld39_world *world = malloc(sizeof(ld39_world));
	whitgl_fvec start = {1024,1024};
	ld39_world_generate(world, start);
	ld39_glider glider = ld39_glider_zero;

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
		}
		if(running == false)
			break;
		whitgl_sys_draw_init(0);
		whitgl_fmat view = ld39_glider_onboard_camera(glider);
		ld39_world_draw(view, setup.size);
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
