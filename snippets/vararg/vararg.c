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

STATIC mp_obj_t vararg_function(size_t n_args, const mp_obj_t *args) {
    if(n_args == 0) {
        printf("no arguments supplied\n");
    } else if(n_args == 1) {
        printf("this is a %lu\n", mp_obj_get_int(args[0]));
    } else if(n_args == 2) {
        printf("hm, we will sum them: %lu\n", mp_obj_get_int(args[0]) + mp_obj_get_int(args[1]));
    } else if(n_args == 3) {
        printf("Look at that! A triplet: %lu, %lu, %lu\n", mp_obj_get_int(args[0]), mp_obj_get_int(args[1]), mp_obj_get_int(args[2]));
    }
    return mp_const_none;
} 

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(vararg_function_obj, 0, 3, vararg_function);

STATIC const mp_rom_map_elem_t vararg_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_vararg) },
    { MP_ROM_QSTR(MP_QSTR_vararg), MP_ROM_PTR(&vararg_function_obj) },
};
STATIC MP_DEFINE_CONST_DICT(vararg_module_globals, vararg_module_globals_table);

const mp_obj_module_t vararg_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&vararg_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_vararg, vararg_user_cmodule, MODULE_VARARG_ENABLED);
