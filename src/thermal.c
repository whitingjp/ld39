#include "thermal.h"

#include <whitgl/sys.h>

void ld39_thermal_draw(ld39_thermal thermal, whitgl_float time, whitgl_fmat view, whitgl_fmat perspective)
{
	whitgl_float t = whitgl_fwrap(time/(thermal.radius*16),0,1);
	whitgl_int num_particles = whitgl_fclamp(thermal.height*4, 0, 128);
	float data[sizeof(float)*128*2*9];
	whitgl_int off = 0;
	whitgl_int i;
	for(i=0; i<num_particles; i++)
	{
		whitgl_fvec3 pos = thermal.base;
		pos.z += (thermal.height*i)/num_particles+time/4;
		pos.z = whitgl_fwrap(pos.z, thermal.base.z, thermal.base.z+thermal.height);
		whitgl_float angle = (whitgl_tau/4+whitgl_tau/16)*i-t*whitgl_tau;
		whitgl_fvec angle_v = whitgl_angle_to_fvec(angle);
		pos.x += angle_v.x*thermal.radius;
		pos.y += angle_v.y*thermal.radius;

		whitgl_float size = 1;
		if(pos.z > thermal.height+thermal.base.z-1)
			size = (thermal.height+thermal.base.z)-pos.z;
		if(pos.z < thermal.base.z+1)
			size = pos.z-thermal.base.z;

		size /= 2;

		data[off++] = pos.x+size/2; data[off++] = pos.y; data[off++] = pos.z+size; data[off++] = 1; data[off++] = 1; data[off++] = 0; data[off++] = 0; data[off++] = 0; data[off++] = 0;
		data[off++] = pos.x+size; data[off++] = pos.y; data[off++] = pos.z; data[off++] = 1; data[off++] = 1; data[off++] = 0; data[off++] = 0; data[off++] = 0; data[off++] = 0;
		data[off++] = pos.x; data[off++] = pos.y; data[off++] = pos.z; data[off++] = 1; data[off++] = 1; data[off++] = 0; data[off++] = 0; data[off++] = 0; data[off++] = 0;

		data[off++] = pos.x+size/2; data[off++] = pos.y; data[off++] = pos.z+size; data[off++] = 1; data[off++] = 1; data[off++] = 0; data[off++] = 0; data[off++] = 0; data[off++] = 0;
		data[off++] = pos.x; data[off++] = pos.y; data[off++] = pos.z; data[off++] = 1; data[off++] = 1; data[off++] = 0; data[off++] = 0; data[off++] = 0; data[off++] = 0;
		data[off++] = pos.x+size; data[off++] = pos.y; data[off++] = pos.z; data[off++] = 1; data[off++] = 1; data[off++] = 0; data[off++] = 0; data[off++] = 0; data[off++] = 0;

	}
	whitgl_sys_update_model_from_data(16, off/9, (char*)data);
	whitgl_sys_draw_model(16, WHITGL_SHADER_EXTRA_0, whitgl_fmat_identity, view, perspective);
}
