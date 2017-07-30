#ifndef WHITGL_HEIGHTMAP_H_
#define WHITGL_HEIGHTMAP_H_

#include <whitgl/math.h>
#include <thermal.h>

static const whitgl_ivec heightmap_size = {64, 64};
#define heightmap_num_tris (64*64*4)

typedef struct
{
	whitgl_fvec3 p[3];
} ld39_triangle;

typedef struct
{
	whitgl_fvec center;
	whitgl_int model_id;
	ld39_triangle tris[heightmap_num_tris];
	whitgl_int tri;
	whitgl_bool active;
	ld39_thermal thermal;
} ld39_heightmap;
static const ld39_heightmap ld39_heightmap_zero = {{0,0}, 0, {}, 0, false, ld39_thermal_zero};

#define MAX_ACTIVE_MAPS (31)
typedef struct
{
	ld39_heightmap maps[MAX_ACTIVE_MAPS];
	whitgl_int current_gen;
} ld39_world;

void ld39_world_generate(ld39_world* world, whitgl_fvec3 glider_pos);
void ld39_world_update(ld39_world* world, whitgl_fvec3 glider_pos);
void ld39_world_draw(ld39_world* world, whitgl_float time, whitgl_fmat view, whitgl_fmat perspective);
whitgl_float stacked_perlin2d(whitgl_float x, whitgl_float y, whitgl_int seed);


#endif // WHITGL_HEIGHTMAP_H_
