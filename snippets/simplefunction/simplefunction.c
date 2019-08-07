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

STATIC mp_obj_t simplefunction_add_ints(mp_obj_t a_obj, mp_obj_t b_obj) {
    int a = mp_obj_get_int(a_obj);
    int b = mp_obj_get_int(b_obj);
    return mp_obj_new_int(a + b);
}

STATIC MP_DEFINE_CONST_FUN_OBJ_2(simplefunction_add_ints_obj, simplefunction_add_ints);

STATIC const mp_rom_map_elem_t simplefunction_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_simplefunction) },
    { MP_ROM_QSTR(MP_QSTR_add_ints), MP_ROM_PTR(&simplefunction_add_ints_obj) },
};
STATIC MP_DEFINE_CONST_DICT(simplefunction_module_globals, simplefunction_module_globals_table);

const mp_obj_module_t simplefunction_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&simplefunction_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_simplefunction, simplefunction_user_cmodule, MODULE_SIMPLEFUNCTION_ENABLED);
