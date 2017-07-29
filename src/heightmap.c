#include "heightmap.h"

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
	whitgl_int ix = x;
	whitgl_int iy = y;
	whitgl_float x_part = x-ix;
	whitgl_float y_part = y-iy;
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
	height += perlin2d(x,y,1/13.0,seed);
	height += perlin2d(x,y,1/7.0,seed);
	height += perlin2d(x,y,1/3.0,seed);
	height += perlin2d(x,y,1/1.0,seed);
	height /= 4;
	return height;
}

void ld39_heightmap_generate(ld39_heightmap* heightmap)
{
	whitgl_int i;
	whitgl_int tri = 0;
	for(i=0; i<heightmap_size.x*heightmap_size.y; i++)
	{
		whitgl_float x = i%heightmap_size.x;
		whitgl_float y = i/heightmap_size.y;
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
			ld39_triangle triangle = {{mid, corners[next], corners[j]}};
			heightmap->tris[tri++] = triangle;
		}
	}
	float data[heightmap_num_tris*3*9];
	whitgl_int off = 0;
	for(i=0; i<heightmap_num_tris; i++)
	{
		ld39_triangle t = heightmap->tris[i];
		data[off++] = t.p[0].x; data[off++] = t.p[0].y; data[off++] = t.p[0].z; data[off++] = t.p[0].z; data[off++] = 0; data[off++] = 0; data[off++] = 0; data[off++] = 0; data[off++] = 0;
		data[off++] = t.p[1].x; data[off++] = t.p[1].y; data[off++] = t.p[1].z; data[off++] = t.p[1].z; data[off++] = 0; data[off++] = 0; data[off++] = 0; data[off++] = 0; data[off++] = 0;
		data[off++] = t.p[2].x; data[off++] = t.p[2].y; data[off++] = t.p[2].z; data[off++] = t.p[2].z; data[off++] = 0; data[off++] = 0; data[off++] = 0; data[off++] = 0; data[off++] = 0;
	}
	whitgl_sys_update_model_from_data(0, off/9, (char*)data);
}

void ld39_heightmap_draw()
{
	// whitgl_ivec size = heightmap_size;
	// unsigned char data[4*heightmap_size.x*heightmap_size.y];
	// whitgl_int i;
	// for(i=0; i<size.x*size.y; i++)
	// {
	// 	whitgl_float height = stacked_perlin2d(i%size.x, i/size.x, 0)*255;
	// 	data[i*4+0] = height;
	// 	data[i*4+1] = height;
	// 	data[i*4+2] = height;
	// 	data[i*4+3] = 255;
	// }
	// whitgl_sys_update_image_from_data(0, size, data);
	// whitgl_sprite sprite = {0, whitgl_ivec_zero, size};
	// whitgl_sys_draw_sprite(sprite, whitgl_ivec_zero, whitgl_ivec_zero);

	whitgl_fmat projection = whitgl_fmat_orthographic(0, heightmap_size.x, 0, heightmap_size.y, 0, 1);
	whitgl_set_shader_color(WHITGL_SHADER_FLAT, 0, whitgl_sys_color_white);
	whitgl_sys_draw_model(0, WHITGL_SHADER_MODEL, whitgl_fmat_identity, whitgl_fmat_identity, projection);
}
