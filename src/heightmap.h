#ifndef WHITGL_HEIGHTMAP_H_
#define WHITGL_HEIGHTMAP_H_

#include <whitgl/math.h>

static const whitgl_ivec heightmap_size = {128, 128};
#define heightmap_num_tris (128*128*4)

typedef struct
{
	whitgl_fvec3 p[3];
} ld39_triangle;

typedef struct
{
	ld39_triangle tris[heightmap_num_tris];
} ld39_heightmap;

void ld39_heightmap_generate(ld39_heightmap* heightmap);
void ld39_heightmap_draw(whitgl_ivec setup_size);

#endif // WHITGL_HEIGHTMAP_H_
