#include "tower.h"

#include <whitgl/sys.h>
#include <assets.h>

void ld39_tower_draw(ld39_tower tower, whitgl_fmat view, whitgl_fmat perspective)
{
	if(!tower.active)
		return;
	whitgl_fmat rotate = whitgl_fmat_rot_z(tower.rotate);
	whitgl_fmat translate = whitgl_fmat_translate(tower.pos);
	whitgl_fmat model =  whitgl_fmat_multiply(translate, rotate);
	whitgl_sys_draw_model(MDL_TOWER, WHITGL_SHADER_EXTRA_0, model, view, perspective);
}
void ld39_tower_draw_connections(ld39_tower tower, live_connection_list* live_list, whitgl_fmat view, whitgl_fmat perspective)
{
	if(!tower.active)
		return;

	float data[sizeof(float)*MAX_CONNECTIONS*12*9];
	whitgl_int off = 0;
	whitgl_int i;
	whitgl_fvec3 offset = {0,0,25};
	// whitgl_fvec3 mid_offset = {0,0,17};
	// whitgl_float mid_radius = 4;
	for(i=0; i<tower.num_connections; i++)
	{
		whitgl_fvec3 start = whitgl_fvec3_add(tower.pos, offset);
		whitgl_fvec3 end = whitgl_fvec3_add(tower.connections[i], offset);
		whitgl_float size = 0.25;
		whitgl_fvec3 a = {start.x, start.y, start.z-size};
		whitgl_fvec3 b = {start.x, start.y, start.z+size};
		whitgl_fvec3 c = {end.x, end.y, end.z+size};
		whitgl_fvec3 d = {end.x, end.y, end.z-size};

		whitgl_bool connected = false;

		whitgl_int j;
		for(j=0; j<live_list->num_connections; j++)
		{
			live_connection* connection = &live_list->connections[j];
			if(whitgl_fvec3_eq(connection->a, tower.pos) && whitgl_fvec3_eq(connection->b, tower.connections[i]))
				connected = true;
			if(whitgl_fvec3_eq(connection->b, tower.pos) && whitgl_fvec3_eq(connection->a, tower.connections[i]))
				connected = true;
			if(connected)
				break;
		}

		whitgl_sys_color live_connection_col = {0xef,0x2b,0xa8,0xff};
		whitgl_sys_color dead_connection_col = {0xfb,0xd5,0xbb,0x20};
		whitgl_sys_color connection_col = connected ? live_connection_col : dead_connection_col;
		whitgl_float colr = connection_col.r/255.0;
		whitgl_float colg = connection_col.g/255.0;
		whitgl_float colb = connection_col.b/255.0;

		whitgl_fvec3 normal = whitgl_fvec3_cross(whitgl_fvec3_sub(b,a), whitgl_fvec3_sub(c,a));
		normal = whitgl_fvec3_normalize(normal);

		data[off++] = a.x; data[off++] = a.y; data[off++] = a.z; data[off++] = colr; data[off++] = colg; data[off++] = colb; data[off++] = normal.x; data[off++] = normal.y; data[off++] = normal.z;
		data[off++] = b.x; data[off++] = b.y; data[off++] = b.z; data[off++] = colr; data[off++] = colg; data[off++] = colb; data[off++] = normal.x; data[off++] = normal.y; data[off++] = normal.z;
		data[off++] = c.x; data[off++] = c.y; data[off++] = c.z; data[off++] = colr; data[off++] = colg; data[off++] = colb; data[off++] = normal.x; data[off++] = normal.y; data[off++] = normal.z;

		data[off++] = a.x; data[off++] = a.y; data[off++] = a.z; data[off++] = colr; data[off++] = colg; data[off++] = colb; data[off++] = normal.x; data[off++] = normal.y; data[off++] = normal.z;
		data[off++] = c.x; data[off++] = c.y; data[off++] = c.z; data[off++] = colr; data[off++] = colg; data[off++] = colb; data[off++] = normal.x; data[off++] = normal.y; data[off++] = normal.z;
		data[off++] = d.x; data[off++] = d.y; data[off++] = d.z; data[off++] = colr; data[off++] = colg; data[off++] = colb; data[off++] = normal.x; data[off++] = normal.y; data[off++] = normal.z;

		normal = whitgl_fvec3_inverse(normal);

		data[off++] = a.x; data[off++] = a.y; data[off++] = a.z; data[off++] = colr; data[off++] = colg; data[off++] = colb; data[off++] = normal.x; data[off++] = normal.y; data[off++] = normal.z;
		data[off++] = c.x; data[off++] = c.y; data[off++] = c.z; data[off++] = colr; data[off++] = colg; data[off++] = colb; data[off++] = normal.x; data[off++] = normal.y; data[off++] = normal.z;
		data[off++] = b.x; data[off++] = b.y; data[off++] = b.z; data[off++] = colr; data[off++] = colg; data[off++] = colb; data[off++] = normal.x; data[off++] = normal.y; data[off++] = normal.z;

		data[off++] = a.x; data[off++] = a.y; data[off++] = a.z; data[off++] = colr; data[off++] = colg; data[off++] = colb; data[off++] = normal.x; data[off++] = normal.y; data[off++] = normal.z;
		data[off++] = d.x; data[off++] = d.y; data[off++] = d.z; data[off++] = colr; data[off++] = colg; data[off++] = colb; data[off++] = normal.x; data[off++] = normal.y; data[off++] = normal.z;
		data[off++] = c.x; data[off++] = c.y; data[off++] = c.z; data[off++] = colr; data[off++] = colg; data[off++] = colb; data[off++] = normal.x; data[off++] = normal.y; data[off++] = normal.z;
	}

	whitgl_sys_update_model_from_data(MDL_THERMAL, off/9, (char*)data);
	whitgl_sys_draw_model(MDL_THERMAL, WHITGL_SHADER_EXTRA_2, whitgl_fmat_identity, view, perspective);
}
