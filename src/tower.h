#ifndef LD39_TOWER_H_
#define LD39_TOWER_H_

#include <whitgl/math.h>

#define MAX_CONNECTIONS (3)

#define MAX_LIVE_CONNECTIONS (1024)

typedef struct
{
	whitgl_fvec3 connections[MAX_LIVE_CONNECTIONS];
	whitgl_int num_connections;
} live_connection_list;
static const live_connection_list live_connection_list_zero = {{}, 0};

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
void ld39_tower_draw_connections(ld39_tower tower, live_connection_list* list, whitgl_fmat view, whitgl_fmat perspective);

#endif // LD39_TOWER_H_
