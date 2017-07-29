#include "heightmap.h"

#include <whitgl/sys.h>
#include <whitgl/random.h>

whitgl_float noise2d(whitgl_int x, whitgl_int y, whitgl_int seed)
{
	whitgl_random_seed seed_x = whitgl_random_seed_init(seed+x);
	whitgl_random_seed seed_y = whitgl_random_seed_init(seed+y*2);
	whitgl_int new_seed = whitgl_random_int(&seed_x, whitgl_int_max);
	new_seed ^= whitgl_random_int(&seed_y, whitgl_int_max);
	whitgl_random_seed sseed = whitgl_random_seed_init(new_seed);
	return whitgl_random_float(&sseed);
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
	return whitgl_fpow(whitgl_finterpolate(t, b, scurve(y_part)),3);
}


void ld39_heightmap_draw()
{
	whitgl_ivec size = {512,512};
	unsigned char data[4*512*512];
	whitgl_int i;
	for(i=0; i<size.x*size.y; i++)
	{
		whitgl_int height = perlin2d(i%size.x,i/size.x,1/64.0,0)*63;
		height += perlin2d(i%size.x,i/size.x,1/32.0,0)*63;
		height += perlin2d(i%size.x,i/size.x,1/16.0,0)*63;
		height += perlin2d(i%size.x,i/size.x,1/8.0,0)*63;
		data[i*4+0] = height;
		data[i*4+1] = height;
		data[i*4+2] = height;
		data[i*4+3] = 255;
	}
	whitgl_sys_update_image_from_data(0, size, data);
	whitgl_sprite sprite = {0, whitgl_ivec_zero, size};
	whitgl_sys_draw_sprite(sprite, whitgl_ivec_zero, whitgl_ivec_zero);
}
