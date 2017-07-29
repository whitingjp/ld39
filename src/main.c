#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

#include <whitgl/input.h>
#include <whitgl/logging.h>
#include <whitgl/math.h>
#include <whitgl/sound.h>
#include <whitgl/sys.h>
#include <whitgl/timer.h>

#include <heightmap.h>



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

	WHITGL_LOG("Initiating sys");
	if(!whitgl_sys_init(&setup))
	{
		WHITGL_LOG("Failed to initiate sys");
		return 1;
	}

	WHITGL_LOG("Initiating sound");
	whitgl_sound_init();
	WHITGL_LOG("Initiating input");
	whitgl_input_init();
	WHITGL_LOG("Initiating timer");
	whitgl_timer_init();

	ld39_heightmap *heightmap = malloc(sizeof(ld39_heightmap));
	ld39_heightmap_generate(heightmap);

	bool running = true;
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
		}
		whitgl_sys_draw_init(0);
		ld39_heightmap_draw(setup.size);
		whitgl_sys_draw_finish();
	}

	free(heightmap);

	WHITGL_LOG("Shutting down input");
	whitgl_input_shutdown();
	WHITGL_LOG("Shutting down sound");
	whitgl_sound_shutdown();

	whitgl_sys_close();

	return 0;
}
