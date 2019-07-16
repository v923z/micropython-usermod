#include "py/obj.h"
#include "py/runtime.h"
#include "py/builtin.h"

STATIC mp_obj_t keywordfunction_add_ints(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {

	static const mp_arg_t allowed_args[] = {
		{ MP_QSTR_a, MP_ARG_INT | MP_ARG_REQUIRED, {.u_int = 0} }, 
        { MP_QSTR_b, MP_ARG_KW_ONLY | MP_ARG_INT | MP_ARG_REQUIRED, {.u_int = 0} },
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);
	int16_t a = args[0].u_int;
	int16_t b = args[1].u_int;
	printf("a = %d, b = %d\n", a, b);
	return mp_obj_new_int(a + b);
}

STATIC MP_DEFINE_CONST_FUN_OBJ_KW(keywordfunction_add_ints_obj, 2, keywordfunction_add_ints);

STATIC const mp_rom_map_elem_t keywordfunction_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_keywordfunction) },
    { MP_ROM_QSTR(MP_QSTR_add_ints), (mp_obj_t)&keywordfunction_add_ints_obj },
};

STATIC MP_DEFINE_CONST_DICT(keywordfunction_module_globals, keywordfunction_module_globals_table);

const mp_obj_module_t keywordfunction_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&keywordfunction_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_keywordfunction, keywordfunction_user_cmodule, MODULE_KEYWORDFUNCTION_ENABLED);
