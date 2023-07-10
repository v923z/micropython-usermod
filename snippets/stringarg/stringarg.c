/*
 * This file is part of the micropython-usermod project, 
 *
 * https://github.com/v923z/micropython-usermod
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Zoltán Vörös
*/
    
#include <string.h>
#include "py/obj.h"
#include "py/runtime.h"
#include "py/objstr.h"

#define byteswap(a,b) char tmp = a; a = b; b = tmp; 

STATIC mp_obj_t stringarg_function(const mp_obj_t o_in) {
    mp_check_self(mp_obj_is_str_or_bytes(o_in));
    GET_STR_DATA_LEN(o_in, str, str_len);
    printf("string length: %lu\n", str_len);
    char out_str[str_len];
    strcpy(out_str, (char *)str);
    for(size_t i=0; i < (str_len-1)/2; i++) {
        byteswap(out_str[i], out_str[str_len-i-1]);
    }
    return mp_obj_new_str(out_str, str_len);
} 

STATIC MP_DEFINE_CONST_FUN_OBJ_1(stringarg_function_obj, stringarg_function);

STATIC const mp_rom_map_elem_t stringarg_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_stringarg) },
    { MP_ROM_QSTR(MP_QSTR_stringarg), MP_ROM_PTR(&stringarg_function_obj) },
};
STATIC MP_DEFINE_CONST_DICT(stringarg_module_globals, stringarg_module_globals_table);

const mp_obj_module_t stringarg_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&stringarg_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_stringarg, stringarg_user_cmodule);
