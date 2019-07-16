#include "py/obj.h"
#include "py/runtime.h"


STATIC mp_obj_t iterables_sumsq(mp_obj_t o_in) {
	mp_float_t _sum = 0.0, itemf;
	mp_obj_iter_buf_t iter_buf;
	mp_obj_t item, iterable = mp_getiter(o_in, &iter_buf);
	while ((item = mp_iternext(iterable)) != MP_OBJ_STOP_ITERATION) {
		itemf = mp_obj_get_float(item);
		_sum += itemf*itemf;
	}
	return mp_obj_new_float(_sum);
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(iterables_sumsq_obj, iterables_sumsq);

STATIC const mp_rom_map_elem_t iterables_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_iterables) },
    { MP_ROM_QSTR(MP_QSTR_sumsq), MP_ROM_PTR(&iterables_sumsq_obj) },
};
STATIC MP_DEFINE_CONST_DICT(iterables_module_globals, iterables_module_globals_table);

const mp_obj_module_t iterables_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&iterables_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_iterables, iterables_user_cmodule, MODULE_ITERABLES_ENABLED);
