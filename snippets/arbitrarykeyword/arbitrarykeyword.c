/*
 * This file is part of the micropython-usermod project, 
 *
 * https://github.com/v923z/micropython-usermod
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Zoltán Vörös
*/
    
#include <stdio.h>
#include "py/obj.h"
#include "py/objlist.h"
#include "py/runtime.h"
#include "py/builtin.h"

// This is lifted from objfloat.c, because mp_obj_float_t is not exposed there (there is no header file)
typedef struct _mp_obj_float_t {
    mp_obj_base_t base;
    mp_float_t value;
} mp_obj_float_t;

const mp_obj_float_t my_float = {{&mp_type_float}, 0.987};

const mp_rom_obj_tuple_t my_tuple = {
    {&mp_type_tuple},
    3,
    {
        MP_ROM_INT(0),
        MP_ROM_QSTR(MP_QSTR_float),
        MP_ROM_PTR(&my_float),
    },
};

STATIC mp_obj_t arbitrarykeyword_print(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_a, MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_b, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 1} },
        { MP_QSTR_c, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_rom_obj = MP_ROM_QSTR(MP_QSTR_float)} },
        { MP_QSTR_d, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_rom_obj = MP_ROM_PTR(&my_float)} },
        { MP_QSTR_e, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_rom_obj = MP_ROM_PTR(&my_tuple)} },
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(1, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);
    mp_obj_t tuple[5];
    tuple[0] = mp_obj_new_int(args[0].u_int); // a
    tuple[1] = mp_obj_new_int(args[1].u_int); // b
    tuple[2] = args[2].u_obj; // c
    tuple[3] = args[3].u_obj; // d
    tuple[4] = args[4].u_obj; // e
    return mp_obj_new_tuple(5, tuple);
}

STATIC MP_DEFINE_CONST_FUN_OBJ_KW(arbitrarykeyword_print_obj, 1, arbitrarykeyword_print);

STATIC const mp_rom_map_elem_t arbitrarykeyword_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_arbitrarykeyword) },
    { MP_ROM_QSTR(MP_QSTR_print), (mp_obj_t)&arbitrarykeyword_print_obj },
};

STATIC MP_DEFINE_CONST_DICT(arbitrarykeyword_module_globals, arbitrarykeyword_module_globals_table);

const mp_obj_module_t arbitrarykeyword_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&arbitrarykeyword_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_arbitrarykeyword, arbitrarykeyword_user_cmodule, MODULE_ARBITRARYKEYWORD_ENABLED);
