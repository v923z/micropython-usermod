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
#include "py/objstr.h"
#include "py/objtuple.h"

#define MAGIC_CONSTANT 42
STATIC const MP_DEFINE_STR_OBJ(version_string_obj, "1.2.3");

const mp_rom_obj_tuple_t version_tuple_obj = {
    {&mp_type_tuple},
    2,
    {
        MP_ROM_INT(1),
        MP_ROM_PTR(&version_string_obj),
    },
};

STATIC const mp_rom_map_elem_t constants_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_constants) },
    { MP_ROM_QSTR(MP_QSTR___version__), MP_ROM_PTR(&version_string_obj) },
    { MP_ROM_QSTR(MP_QSTR_magic), MP_ROM_INT(MAGIC_CONSTANT) },
    { MP_ROM_QSTR(MP_QSTR_version_tuple), MP_ROM_PTR(&version_tuple_obj) },    
};
STATIC MP_DEFINE_CONST_DICT(constants_module_globals, constants_module_globals_table);

const mp_obj_module_t constants_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&constants_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_constants, constants_user_cmodule);
