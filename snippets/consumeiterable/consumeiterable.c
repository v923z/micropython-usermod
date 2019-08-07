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

STATIC mp_obj_t consumeiterable_sumsq(mp_obj_t o_in) {
    mp_float_t _sum = 0.0, itemf;
    mp_obj_iter_buf_t iter_buf;
    mp_obj_t item, iterable = mp_getiter(o_in, &iter_buf);
    while ((item = mp_iternext(iterable)) != MP_OBJ_STOP_ITERATION) {
        itemf = mp_obj_get_float(item);
        _sum += itemf*itemf;
    }
    return mp_obj_new_float(_sum);
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(consumeiterable_sumsq_obj, consumeiterable_sumsq);

STATIC const mp_rom_map_elem_t consumeiterable_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_consumeiterable) },
    { MP_ROM_QSTR(MP_QSTR_sumsq), MP_ROM_PTR(&consumeiterable_sumsq_obj) },
};
STATIC MP_DEFINE_CONST_DICT(consumeiterable_module_globals, consumeiterable_module_globals_table);

const mp_obj_module_t consumeiterable_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&consumeiterable_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_consumeiterable, consumeiterable_user_cmodule, MODULE_CONSUMEITERABLE_ENABLED);
