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
#include "py/builtin.h"
#include "py/runtime.h"
#include <stdlib.h>

STATIC mp_obj_t mean_function(mp_obj_t error_code) {
    int e = mp_obj_get_int(error_code);
    if(e == 0) {
        mp_raise_msg(&mp_type_ZeroDivisionError, "thou shall not try to divide by 0 on a microcontroller!");
    } else if(e == 1) {
        mp_raise_msg(&mp_type_IndexError, "dude, that was a silly mistake!");
    } else if(e == 2) {
        mp_raise_TypeError("look, chap, you can't be serious!");
    } else if(e == 3) {
        mp_raise_OSError(e);
    } else if(e == 4) {
        char *buffer;
        buffer = malloc(100);
        sprintf(buffer, "you are really out of luck today: error code %d", e);
        mp_raise_NotImplementedError(buffer);
    } else {
        mp_raise_ValueError("sorry, you've exhausted all your options");
    }
    return mp_const_false;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(mean_function_obj, mean_function);

STATIC const mp_rom_map_elem_t sillyerrors_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_sillyerrors) },
    { MP_ROM_QSTR(MP_QSTR_mean), MP_ROM_PTR(&mean_function_obj) },
};
STATIC MP_DEFINE_CONST_DICT(sillyerrors_module_globals, sillyerrors_module_globals_table);

const mp_obj_module_t sillyerrors_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&sillyerrors_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_sillyerrors, sillyerrors_user_cmodule, MODULE_SILLYERRORS_ENABLED);
