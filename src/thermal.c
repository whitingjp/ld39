#include "thermal.h"

#include <whitgl/sys.h>
#include <assets.h>

void ld39_thermal_draw(ld39_thermal thermal, whitgl_float time, whitgl_fmat view, whitgl_fmat perspective)
{
	if(!thermal.active)
		return;
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

		size /= 4;

		whitgl_fvec3 a = {0, 0, size};
		whitgl_fvec3 b = {size, 0, -size};
		whitgl_fvec3 c = {-size, 0, -size};
		whitgl_fmat spin = whitgl_fmat_rot_z((time/2.0+i/8.0)*whitgl_tau);
		a = whitgl_fvec3_add(pos, whitgl_fvec3_apply_fmat(a, spin));
		b = whitgl_fvec3_add(pos, whitgl_fvec3_apply_fmat(b, spin));
		c = whitgl_fvec3_add(pos, whitgl_fvec3_apply_fmat(c, spin));
		whitgl_sys_color thermal_col = {0xf6,0xb1,0x5f,0xff};
		whitgl_float colr = thermal_col.r/255.0;
		whitgl_float colg = thermal_col.g/255.0;
		whitgl_float colb = thermal_col.b/255.0;

		whitgl_fvec3 normal = whitgl_fvec3_cross(whitgl_fvec3_sub(b,a), whitgl_fvec3_sub(c,a));
		normal = whitgl_fvec3_normalize(normal);

		data[off++] = a.x; data[off++] = a.y; data[off++] = a.z; data[off++] = colr; data[off++] = colg; data[off++] = colb; data[off++] = normal.x; data[off++] = normal.y; data[off++] = normal.z;
		data[off++] = b.x; data[off++] = b.y; data[off++] = b.z; data[off++] = colr; data[off++] = colg; data[off++] = colb; data[off++] = normal.x; data[off++] = normal.y; data[off++] = normal.z;
		data[off++] = c.x; data[off++] = c.y; data[off++] = c.z; data[off++] = colr; data[off++] = colg; data[off++] = colb; data[off++] = normal.x; data[off++] = normal.y; data[off++] = normal.z;

		normal = whitgl_fvec3_inverse(normal);
		data[off++] = a.x; data[off++] = a.y; data[off++] = a.z; data[off++] = colr; data[off++] = colg; data[off++] = colb; data[off++] = normal.x; data[off++] = normal.y; data[off++] = normal.z;
		data[off++] = c.x; data[off++] = c.y; data[off++] = c.z; data[off++] = colr; data[off++] = colg; data[off++] = colb; data[off++] = normal.x; data[off++] = normal.y; data[off++] = normal.z;
		data[off++] = b.x; data[off++] = b.y; data[off++] = b.z; data[off++] = colr; data[off++] = colg; data[off++] = colb; data[off++] = normal.x; data[off++] = normal.y; data[off++] = normal.z;

	}
	whitgl_sys_update_model_from_data(MDL_THERMAL, off/9, (char*)data);
	whitgl_sys_draw_model(MDL_THERMAL, WHITGL_SHADER_EXTRA_1, whitgl_fmat_identity, view, perspective);
}
