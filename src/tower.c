#include "tower.h"

#include <whitgl/sys.h>
#include <assets.h>

void ld39_tower_draw(ld39_tower tower, whitgl_fmat view, whitgl_fmat perspective)
{
	whitgl_fmat rotate = whitgl_fmat_rot_z(tower.rotate);
	whitgl_fmat translate = whitgl_fmat_translate(tower.pos);
	whitgl_fmat model =  whitgl_fmat_multiply(translate, rotate);
	whitgl_sys_draw_model(MDL_TOWER, WHITGL_SHADER_EXTRA_0, model, view, perspective);
}
	// whitgl_float ground_height = stacked_perlin2d(1024*6,1024*6-32, 0);
	// whitgl_fvec3 translate = {1024*6,1024*6-32,ground_height-2};
