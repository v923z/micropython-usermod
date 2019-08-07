/*
 * This file is part of the micropython-usermod project, 
 *
 * https://github.com/v923z/micropython-usermod
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Zoltán Vörös
*/
    
#include "py/obj.h"
#include "py/runtime.h"

STATIC mp_obj_t powers_iterable(mp_obj_t base, mp_obj_t exponent) {
    int e = mp_obj_get_int(exponent);
    mp_obj_t tuple[e+1];
    int b = mp_obj_get_int(base), ba = 1;
    for(int i=0; i <= e; i++) {
        tuple[i] = mp_obj_new_int(ba);
        ba *= b;
    }
    return mp_obj_new_tuple(e+1, tuple);
}

STATIC MP_DEFINE_CONST_FUN_OBJ_2(powers_iterable_obj, powers_iterable);

STATIC const mp_rom_map_elem_t returniterable_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_returniterable) },
    { MP_ROM_QSTR(MP_QSTR_powers), MP_ROM_PTR(&powers_iterable_obj) },
};
STATIC MP_DEFINE_CONST_DICT(returniterable_module_globals, returniterable_module_globals_table);

const mp_obj_module_t returniterable_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&returniterable_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_returniterable, returniterable_user_cmodule, MODULE_RETURNITERABLE_ENABLED);
