#include "heightmap.h"

#include <stdlib.h>
#include <whitgl/sys.h>
#include <whitgl/random.h>
#include <whitgl/logging.h>

#include <glider.h>


whitgl_float noise2d(whitgl_int x, whitgl_int y, whitgl_int seed)
{
	whitgl_random_seed seed_base = whitgl_random_seed_init(seed+10);
	whitgl_random_seed seed_x = whitgl_random_seed_init(x);
	whitgl_random_seed seed_y = whitgl_random_seed_init(whitgl_int_max-y);
	whitgl_int new_seed = whitgl_random_int(&seed_base, whitgl_int_max);
	new_seed ^= whitgl_random_int(&seed_x, whitgl_int_max);
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
	height += perlin2d(x,y,1/61.0,seed)*32;
	height += perlin2d(x,y,1/29.0,seed)*16;
	height += perlin2d(x,y,1/13.0,seed)*4;
	height += perlin2d(x,y,1/7.0,seed)*2;
	height += perlin2d(x,y,1/3.0,seed);
	height += perlin2d(x,y,1/1.0,seed)/2;
	return height;
}

void ld39_heightmap_new(ld39_heightmap* heightmap, whitgl_fvec center, whitgl_int model_id, whitgl_bool first_ever)
{
	heightmap->tri = 0;
	heightmap->center = center;
	heightmap->active = false;
	heightmap->model_id = model_id;
	heightmap->first_ever = first_ever;
}

#define NUMBER_OF_FRAMES_PER_GEN (32)
void ld39_heightmap_do_some_generating(ld39_heightmap* heightmap)
{
	if(heightmap->tri == heightmap_num_tris)
	{
		heightmap->active = true;
		float *data = malloc(sizeof(float)*heightmap->tri*3*9);
		whitgl_int off = 0;
		whitgl_int i;
		for(i=0; i<heightmap->tri; i++)
		{
			ld39_triangle t = heightmap->tris[i];
			whitgl_fvec3 normal = whitgl_fvec3_cross(whitgl_fvec3_sub(t.p[1],t.p[0]), whitgl_fvec3_sub(t.p[2],t.p[0]));
			normal = whitgl_fvec3_normalize(normal);
			data[off++] = t.p[0].x; data[off++] = t.p[0].y; data[off++] = t.p[0].z;  data[off++] = t.variance; data[off++] = 0; data[off++] = 0;data[off++] = normal.x; data[off++] = normal.y; data[off++] = normal.z;
			data[off++] = t.p[1].x; data[off++] = t.p[1].y; data[off++] = t.p[1].z;  data[off++] = t.variance; data[off++] = 0; data[off++] = 0;data[off++] = normal.x; data[off++] = normal.y; data[off++] = normal.z;
			data[off++] = t.p[2].x; data[off++] = t.p[2].y; data[off++] = t.p[2].z;  data[off++] = t.variance; data[off++] = 0; data[off++] = 0;data[off++] = normal.x; data[off++] = normal.y; data[off++] = normal.z;
		}
		whitgl_sys_update_model_from_data(heightmap->model_id, off/9, (char*)data);
		free(data);


		ld39_thermal thermal = ld39_thermal_zero;
		whitgl_fvec gliderstart2d = {ld39_glider_zero.pos.x, ld39_glider_zero.pos.y};
		whitgl_fvec diff = whitgl_fvec_sub(heightmap->center, gliderstart2d);
		whitgl_float magnitude = whitgl_fvec_magnitude(diff);
		if(noise2d(heightmap->center.x, heightmap->center.y, 6) < 1/(magnitude*0.01))
		{
			whitgl_fvec relative = {noise2d(heightmap->center.x, heightmap->center.y, 1)-0.5, noise2d(heightmap->center.x, heightmap->center.y, 2)-0.5};
			whitgl_fvec offset = whitgl_fvec_scale(relative, whitgl_ivec_to_fvec(heightmap_size));
			whitgl_fvec actual_pos = whitgl_fvec_add(heightmap->center, offset);
			whitgl_float base_height = stacked_perlin2d(actual_pos.x,actual_pos.y,0)+2+12*noise2d(heightmap->center.x, heightmap->center.y, 3);
			thermal.base.x = actual_pos.x;
			thermal.base.y = actual_pos.y;
			thermal.base.z = base_height;
			thermal.height = ((whitgl_int)(noise2d(heightmap->center.x, heightmap->center.y, 5)*3)+1)*8;
			// thermal.radius = 6 + 4*noise2d(actual_pos.x,actual_pos.y,4);
			// thermal.height = 12 + 6*noise2d(actual_pos.x,actual_pos.y,5);
			thermal.active = true;
		}
		heightmap->thermal = thermal;
		return;
	}

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
			ld39_triangle triangle = {{mid, corners[j], corners[next]}, perlin2d(x+0.5,y+0.5,1/13.0,50)};
			heightmap->tris[heightmap->tri++] = triangle;
		}
	}
}


void ld39_world_generate(ld39_world* world, whitgl_fvec3 glider_pos)
{
	whitgl_int i;
	for(i=0; i<MAX_ACTIVE_MAPS; i++)
	{
		world->maps[i] = ld39_heightmap_zero;
		whitgl_sys_update_model_from_data(i, 0, NULL);
	}
	world->current_gen = -1;
	world->connections = live_connection_list_zero;

	while(!world->maps[MAX_ACTIVE_MAPS/2].active)
		ld39_world_update(world, glider_pos);
}
void ld39_world_update(ld39_world* world, whitgl_fvec3 glider_pos)
{
	if(world->maps[world->current_gen].active)
	{
		ld39_heightmap* heightmap = &world->maps[world->current_gen];
		ld39_tower tower = ld39_tower_zero;
		if(noise2d(heightmap->center.x, heightmap->center.y, 9) > 0.5 || heightmap->first_ever)
		{
			whitgl_fvec relative = {noise2d(heightmap->center.x, heightmap->center.y, 6)-0.5, noise2d(heightmap->center.x, heightmap->center.y, 7)-0.5};
			whitgl_fvec offset = whitgl_fvec_scale(relative, whitgl_ivec_to_fvec(heightmap_size));
			whitgl_fvec actual_pos = whitgl_fvec_add(heightmap->center, offset);
			whitgl_float base_height = stacked_perlin2d(actual_pos.x,actual_pos.y,0)-2;
			tower.pos.x = actual_pos.x;
			tower.pos.y = actual_pos.y;
			tower.pos.z = base_height;
			tower.rotate = noise2d(heightmap->center.x, heightmap->center.y, 8)*whitgl_tau;
			tower.active = true;

			while(tower.num_connections < MAX_CONNECTIONS)
			{
				whitgl_float best = whitgl_float_max;
				whitgl_int best_id = -1;
				whitgl_int i;
				for(i=0; i<MAX_ACTIVE_MAPS; i++)
				{
					if(i==world->current_gen)
						continue;

					ld39_heightmap* other = &world->maps[i];
					// if(!other->active)
					// 	continue;

					if(!other->tower.active)
						continue;

					if(other->tower.num_connections >= MAX_CONNECTIONS)
						continue;

					whitgl_int j;
					whitgl_bool already_connected = false;
					for(j=0; j<tower.num_connections; j++)
					{
						if(!whitgl_fvec3_eq(tower.connections[j], other->tower.pos))
							continue;
						already_connected = true;
						break;
					}
					if(already_connected)
						continue;

					whitgl_fvec3 diff = whitgl_fvec3_sub(other->tower.pos, tower.pos);
					whitgl_float sqmag = whitgl_fvec3_sqmagnitude(diff);

					if(sqmag > 128*128)
						continue;

					if(sqmag >= best)
						continue;

					best = sqmag;
					best_id = i;

				}
				if(best_id == -1)
					break;
				if(best > 64*64 && tower.num_connections > 0)
					break;
				ld39_heightmap* other = &world->maps[best_id];
				tower.connections[tower.num_connections++] = other->tower.pos;
				other->tower.connections[other->tower.num_connections++] = tower.pos;

				if(other->first_ever)
				{
					live_connection connection = {tower.pos, other->tower.pos};
					if(world->connections.num_connections < MAX_LIVE_CONNECTIONS)
						world->connections.connections[world->connections.num_connections++] = connection;
				}
			}
		}
		heightmap->tower = tower;
	}
	if(world->maps[world->current_gen].active || world->current_gen < 0)
	{
		whitgl_bool first_ever = world->current_gen < 0;
		// find the furthest existing
		whitgl_fvec glider_pos_2d = {glider_pos.x, glider_pos.y};
		whitgl_float best_dist = 0;
		whitgl_int best_id = 0;
		whitgl_int i;
		for(i=0; i<MAX_ACTIVE_MAPS; i++)
		{
			if(!world->maps[i].active)
			{
				best_id = i;
				break;
			}

			whitgl_fvec diff = whitgl_fvec_sub(world->maps[i].center, glider_pos_2d);
			whitgl_float sqmag = whitgl_fvec_sqmagnitude(diff);
			if(sqmag > best_dist)
			{
				best_id = i;
				best_dist = sqmag;
			}
		}
		world->maps[best_id].active = false;

		// find the nearest unnoccupied potential
		best_dist = whitgl_float_max;
		whitgl_fvec best_center = whitgl_fvec_zero;

		whitgl_fvec start_point = {whitgl_fnearest(glider_pos.x, heightmap_size.x), whitgl_fnearest(glider_pos.y, heightmap_size.y)};
		const whitgl_int search_radius = 6;
		for(i=0; i<search_radius*search_radius; i++)
		{
			whitgl_ivec p = {i%search_radius, i/search_radius};
			whitgl_fvec offset = {(p.x-search_radius/2)*(heightmap_size.x), (p.y-search_radius/2)*(heightmap_size.y)};
			whitgl_fvec center = whitgl_fvec_add(start_point, offset);
			whitgl_fvec diff = whitgl_fvec_sub(center, glider_pos_2d);
			whitgl_float sqmag = whitgl_fvec_sqmagnitude(diff);
			if(sqmag > best_dist)
				continue;
			whitgl_int j;
			whitgl_bool already_in_use = false;
			for(j=0; j<MAX_ACTIVE_MAPS; j++)
			{
				if(!world->maps[j].active) continue;
				if(!whitgl_fvec_eq(world->maps[j].center, center)) continue;
				already_in_use = true;
			}
			if(already_in_use)
				continue;
			best_dist = sqmag;
			best_center = center;
		}

		world->current_gen = best_id;
		ld39_heightmap_new(&world->maps[best_id], best_center, best_id, first_ever);
	}
	else
	{
		ld39_heightmap_do_some_generating(&world->maps[world->current_gen]);
	}
}

void ld39_world_draw(ld39_world* world, whitgl_float time, whitgl_fmat view, whitgl_fmat perspective)
{
	whitgl_sys_enable_depth(true);

	whitgl_int i;
	for(i=0; i<MAX_ACTIVE_MAPS; i++)
	{
		whitgl_sys_draw_model(i, WHITGL_SHADER_EXTRA_0, whitgl_fmat_identity, view, perspective);
	}
	for(i=0; i<MAX_ACTIVE_MAPS; i++)
	{
		if(!world->maps[i].active)
			continue;
		ld39_tower_draw(world->maps[i].tower, view, perspective);
	}
	for(i=0; i<MAX_ACTIVE_MAPS; i++)
	{
		if(!world->maps[i].active)
			continue;
		ld39_tower_draw_connections(world->maps[i].tower, &world->connections, view, perspective);
	}
	for(i=0; i<MAX_ACTIVE_MAPS; i++)
	{
		if(!world->maps[i].active)
			continue;
		ld39_thermal_draw(world->maps[i].thermal, time, view, perspective);
	}
}
