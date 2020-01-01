/*
 * This file is part of the micropython-usermod project, 
 *
 * https://github.com/v923z/micropython-usermod
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2019-2020 Zoltán Vörös
*/
    
#include <stdio.h>
#include "py/runtime.h"
#include "py/obj.h"

typedef struct _propertyclass_obj_t {
    mp_obj_base_t base;
    mp_float_t x;
} propertyclass_obj_t;

const mp_obj_type_t propertyclass_type;

STATIC mp_obj_t propertyclass_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 1, 1, true);
    propertyclass_obj_t *self = m_new_obj(propertyclass_obj_t);
    self->base.type = &propertyclass_type;
    self->x = mp_obj_get_float(args[0]);
    return MP_OBJ_FROM_PTR(self);
}

STATIC mp_obj_t propertyclass_x(mp_obj_t self_in) {
    propertyclass_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_float(self->x);
}

MP_DEFINE_CONST_FUN_OBJ_1(propertyclass_x_obj, propertyclass_x);

STATIC const mp_rom_map_elem_t propertyclass_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_x), MP_ROM_PTR(&propertyclass_x_obj) },
};

STATIC MP_DEFINE_CONST_DICT(propertyclass_locals_dict, propertyclass_locals_dict_table);

STATIC void propertyclass_attr(mp_obj_t self, qstr attribute, mp_obj_t *destination) {
    if(attribute == MP_QSTR_x) {
        destination[0] = propertyclass_x(self);
    }
}

const mp_obj_type_t propertyclass_type = {
    { &mp_type_type },
    .name = MP_QSTR_propertyclass,
    .make_new = propertyclass_make_new,
    .attr = propertyclass_attr,
    .locals_dict = (mp_obj_dict_t*)&propertyclass_locals_dict,
};

STATIC const mp_map_elem_t propertyclass_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_propertyclass) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_propertyclass), (mp_obj_t)&propertyclass_type },	
};

STATIC MP_DEFINE_CONST_DICT (
    mp_module_propertyclass_globals,
    propertyclass_globals_table
);

const mp_obj_module_t propertyclass_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mp_module_propertyclass_globals,
};

MP_REGISTER_MODULE(MP_QSTR_propertyclass, propertyclass_user_cmodule, MODULE_PROPERTYCLASS_ENABLED);
