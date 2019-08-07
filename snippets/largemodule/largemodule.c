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
#include "helper.h"


STATIC MP_DEFINE_CONST_FUN_OBJ_2(largemodule_add_ints_obj, largemodule_add_ints);
STATIC MP_DEFINE_CONST_FUN_OBJ_2(largemodule_subtract_ints_obj, largemodule_subtract_ints);

STATIC const mp_rom_map_elem_t largemodule_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_largemodule) },
    { MP_ROM_QSTR(MP_QSTR_add_ints), MP_ROM_PTR(&largemodule_add_ints_obj) },
    { MP_ROM_QSTR(MP_QSTR_subtract_ints), MP_ROM_PTR(&largemodule_subtract_ints_obj) },    
};
STATIC MP_DEFINE_CONST_DICT(largemodule_module_globals, largemodule_module_globals_table);

const mp_obj_module_t largemodule_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&largemodule_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_largemodule, largemodule_user_cmodule, MODULE_LARGEMODULE_ENABLED);
