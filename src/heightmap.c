#include "heightmap.h"

#include <stdlib.h>
#include <whitgl/sys.h>
#include <whitgl/random.h>
#include <whitgl/logging.h>


whitgl_float noise2d(whitgl_int x, whitgl_int y, whitgl_int seed)
{
	whitgl_random_seed seed_x = whitgl_random_seed_init(seed+x);
	whitgl_random_seed seed_y = whitgl_random_seed_init(seed+y*2);
	whitgl_int new_seed = whitgl_random_int(&seed_x, whitgl_int_max);
	new_seed ^= whitgl_random_int(&seed_y, whitgl_int_max);
	whitgl_random_seed sseed = whitgl_random_seed_init(new_seed);
	return whitgl_fpow(whitgl_random_float(&sseed),2);
}

whitgl_float scurve(whitgl_float a)
{
	return -2*whitgl_fpow(a,3) + 3*whitgl_fpow(a,2);
}

whitgl_float perlin2d(whitgl_float x, whitgl_float y, whitgl_float spacing, whitgl_int seed)
{
	x *= spacing;
	y *= spacing;
	whitgl_int ix = x > 0 ? x : x-1;
	whitgl_int iy = y > 0 ? y : y-1;
	whitgl_float x_part = whitgl_fwrap(x, 0, 1);
	whitgl_float y_part = whitgl_fwrap(y, 0, 1);
	if(x < 0) WHITGL_LOG("x %.2f ix %d x_part %.2f", x, ix, x_part);
	whitgl_float tl = noise2d(ix,iy,seed);
	whitgl_float tr = noise2d(ix+1,iy,seed);
	whitgl_float bl = noise2d(ix,iy+1,seed);
	whitgl_float br = noise2d(ix+1,iy+1,seed);
	whitgl_float t = whitgl_finterpolate(tl, tr, scurve(x_part));
	whitgl_float b = whitgl_finterpolate(bl, br, scurve(x_part));
	return whitgl_finterpolate(t, b, scurve(y_part));
}

whitgl_float stacked_perlin2d(whitgl_float x, whitgl_float y, whitgl_int seed)
{
	whitgl_float height = 0;
	height += perlin2d(x,y,1/13.0,seed)*8;
	height += perlin2d(x,y,1/7.0,seed)*4;
	height += perlin2d(x,y,1/3.0,seed)*2;
	height += perlin2d(x,y,1/1.0,seed);
	return height;
}

// void ld39_heightmap_generate(ld39_heightmap* heightmap, whitgl_fvec center)
// {
// 	whitgl_int i;
// 	whitgl_int tri = 0;
// 	for(i=0; i<heightmap_size.x*heightmap_size.y; i++)
// 	{
// 		whitgl_int index = tri/4;
// 		whitgl_float x = index%heightmap_size.x-heightmap_size.x/2+center.x;
// 		whitgl_float y = index/heightmap_size.y-heightmap_size.y/2+center.y;
// 		whitgl_fvec3 corners[4];
// 		whitgl_fvec3 tl = {x, y, stacked_perlin2d(x,y,0)};
// 		corners[0] = tl;
// 		whitgl_fvec3 tr = {x+1, y, stacked_perlin2d(x+1,y,0)};
// 		corners[1] = tr;
// 		whitgl_fvec3 br = {x+1, y+1, stacked_perlin2d(x+1,y+1,0)};
// 		corners[2] = br;
// 		whitgl_fvec3 bl = {x, y+1, stacked_perlin2d(x,y+1,0)};
// 		corners[3] = bl;
// 		whitgl_fvec3 mid = {x+0.5, y+0.5, stacked_perlin2d(x+0.5,y+0.5,0)};

// 		whitgl_int j;
// 		for(j=0; j<4; j++)
// 		{
// 			whitgl_int next = (j+1)%4;
// 			ld39_triangle triangle = {{mid, corners[j], corners[next]}};
// 			heightmap->tris[tri++] = triangle;
// 		}
// 	}
// 	heightmap->center = center;
// }
void ld39_heightmap_new(ld39_heightmap* heightmap, whitgl_fvec center)
{
	heightmap->tri = 0;
	heightmap->center = center;
}
#define NUMBER_OF_FRAMES_PER_GEN (16)
void ld39_heightmap_do_some_generating(ld39_heightmap* heightmap)
{
	whitgl_int i;
	for(i=0; i<(heightmap_size.x*heightmap_size.y)/NUMBER_OF_FRAMES_PER_GEN; i++)
	{
		whitgl_int index = heightmap->tri/4;
		whitgl_float x = index%heightmap_size.x-heightmap_size.x/2+heightmap->center.x;
		whitgl_float y = index/heightmap_size.y-heightmap_size.y/2+heightmap->center.y;
		whitgl_fvec3 corners[4];
		whitgl_fvec3 tl = {x, y, stacked_perlin2d(x,y,0)};
		corners[0] = tl;
		whitgl_fvec3 tr = {x+1, y, stacked_perlin2d(x+1,y,0)};
		corners[1] = tr;
		whitgl_fvec3 br = {x+1, y+1, stacked_perlin2d(x+1,y+1,0)};
		corners[2] = br;
		whitgl_fvec3 bl = {x, y+1, stacked_perlin2d(x,y+1,0)};
		corners[3] = bl;
		whitgl_fvec3 mid = {x+0.5, y+0.5, stacked_perlin2d(x+0.5,y+0.5,0)};

		whitgl_int j;
		for(j=0; j<4; j++)
		{
			whitgl_int next = (j+1)%4;
			ld39_triangle triangle = {{mid, corners[j], corners[next]}};
			heightmap->tris[heightmap->tri++] = triangle;
		}
	}
}
void ld39_heightmap_update_model(ld39_heightmap* heightmap, whitgl_int id)
{
	float *data = malloc(sizeof(float)*heightmap_num_tris*3*9);
	whitgl_int off = 0;
	whitgl_int i;
	for(i=0; i<heightmap_num_tris; i++)
	{
		ld39_triangle t = heightmap->tris[i];
		whitgl_fvec3 normal = whitgl_fvec3_cross(whitgl_fvec3_sub(t.p[1],t.p[0]), whitgl_fvec3_sub(t.p[2],t.p[0]));
		normal = whitgl_fvec3_normalize(normal);
		data[off++] = t.p[0].x; data[off++] = t.p[0].y; data[off++] = t.p[0].z; data[off++] = normal.x; data[off++] = normal.y; data[off++] = normal.z; data[off++] = 0; data[off++] = 0; data[off++] = 0;
		data[off++] = t.p[1].x; data[off++] = t.p[1].y; data[off++] = t.p[1].z; data[off++] = normal.x; data[off++] = normal.y; data[off++] = normal.z; data[off++] = 0; data[off++] = 0; data[off++] = 0;
		data[off++] = t.p[2].x; data[off++] = t.p[2].y; data[off++] = t.p[2].z; data[off++] = normal.x; data[off++] = normal.y; data[off++] = normal.z; data[off++] = 0; data[off++] = 0; data[off++] = 0;
	}
	whitgl_sys_update_model_from_data(id, off/9, (char*)data);
	free(data);
}



void ld39_heightmap_draw(whitgl_int id, whitgl_fmat view, whitgl_ivec setup_size)
{
	whitgl_sys_enable_depth(true);
	whitgl_fmat perspective = whitgl_fmat_perspective(whitgl_tau/4, (float)setup_size.x/(float)setup_size.y, 0.1f, 1024.0f);

	whitgl_set_shader_color(WHITGL_SHADER_FLAT, 0, whitgl_sys_color_white);
	whitgl_sys_draw_model(id, WHITGL_SHADER_EXTRA_0, whitgl_fmat_identity, view, perspective);
}

void ld39_world_generate(ld39_world* world, whitgl_fvec center)
{
	whitgl_int i;
	for(i=0; i<MAX_ACTIVE_MAPS; i++)
	{
		whitgl_ivec p = {i%4, i/4};
		whitgl_fvec offset = {(p.x-2)*(heightmap_size.x), (p.y-2)*(heightmap_size.y)};
		whitgl_int j;
		ld39_heightmap_new(&world->maps[i], whitgl_fvec_add(center, offset));
		for(j=0; j<NUMBER_OF_FRAMES_PER_GEN; j++)
			ld39_heightmap_do_some_generating(&world->maps[i]);
		ld39_heightmap_update_model(&world->maps[i], i);
	}
}
void ld39_world_update(ld39_world* world)
{
	if(world->maps[0].tri == heightmap_num_tris)
	{
		ld39_heightmap_update_model(&world->maps[0], 0);
		ld39_heightmap_new(&world->maps[0], world->maps[0].center);
	}
	else
	{
		ld39_heightmap_do_some_generating(&world->maps[0]);
	}
}

void ld39_world_draw(whitgl_fmat view, whitgl_ivec setup_size)
{
	whitgl_int i;
	for(i=0; i<MAX_ACTIVE_MAPS; i++)
	{
		ld39_heightmap_draw(i, view, setup_size);
	}
}
