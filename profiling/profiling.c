#include <math.h>
#include <stdio.h>
#include "py/obj.h"
#include "py/runtime.h"
#include "mphalport.h"

STATIC mp_obj_t measure_cpu(mp_obj_t _x, mp_obj_t _y, mp_obj_t _z) {
	// first, measure the time taken up by the assignments
	mp_uint_t start = mp_hal_ticks_cpu();
	float x = mp_obj_get_float(_x);
	float y = mp_obj_get_float(_y);
	float z = mp_obj_get_float(_z);
	mp_uint_t stop = mp_hal_ticks_cpu();
	printf("time elapsed: %d CPU cycles\n", stop - start);
	
	// measure the time used by the square root
	start = mp_hal_ticks_cpu();
	float hypo = sqrtf(x*x + y*y + z*z);
	stop = mp_hal_ticks_cpu();
	printf("time elapsed: %d CPU cycles\n", stop - start);	
	return mp_obj_new_float(hypo);
}

STATIC MP_DEFINE_CONST_FUN_OBJ_3(measure_cpu_obj, measure_cpu);

STATIC const mp_rom_map_elem_t profiling_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_profiling) },
    { MP_ROM_QSTR(MP_QSTR_measure), MP_ROM_PTR(&measure_cpu_obj) },
};
STATIC MP_DEFINE_CONST_DICT(profiling_module_globals, profiling_module_globals_table);

const mp_obj_module_t profiling_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&profiling_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_profiling, profiling_user_cmodule, MODULE_PROFILING_ENABLED);

