#ifndef WHITGL_HEIGHTMAP_H_
#define WHITGL_HEIGHTMAP_H_

#include <whitgl/math.h>

static const whitgl_ivec heightmap_size = {64, 64};
#define heightmap_num_tris (64*64*4)

typedef struct
{
	whitgl_fvec3 p[3];
} ld39_triangle;

typedef struct
{
	whitgl_fvec center;
	ld39_triangle tris[heightmap_num_tris];
	whitgl_int tri;
	whitgl_bool active;
} ld39_heightmap;

#define MAX_ACTIVE_MAPS (16)
typedef struct
{
	ld39_heightmap maps[MAX_ACTIVE_MAPS];
} ld39_world;

void ld39_heightmap_generate(ld39_heightmap* heightmap, whitgl_fvec center);
void ld39_heightmap_draw(whitgl_int id, whitgl_fmat view, whitgl_ivec setup_size);

void ld39_world_generate(ld39_world* heightmap, whitgl_fvec center);
void ld39_world_update(ld39_world* world);
void ld39_world_draw(whitgl_fmat view, whitgl_ivec setup_size);


#endif // WHITGL_HEIGHTMAP_H_
