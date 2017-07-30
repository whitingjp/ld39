#ifndef LD39_TOWER_H_
#define LD39_TOWER_H_

#include <whitgl/math.h>

#define MAX_CONNECTIONS (3)


typedef struct
{
	whitgl_bool active;
	whitgl_fvec3 pos;
	whitgl_float rotate;
	whitgl_fvec3 connections[MAX_CONNECTIONS];
	whitgl_int num_connections;
} ld39_tower;
static const ld39_tower ld39_tower_zero = {false, {0,0,0}, 0, {}, 0};


void ld39_tower_draw(ld39_tower tower, whitgl_fmat view, whitgl_fmat perspective);
void ld39_tower_draw_connections(ld39_tower tower, whitgl_fmat view, whitgl_fmat perspective);

#endif // LD39_TOWER_H_
