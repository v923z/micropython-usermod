/*
 * This file is part of the micropython-usermod project, 
 *
 * https://github.com/v923z/micropython-usermod
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2019-2020 Zoltán Vörös
*/
    
#include <math.h>
#include <stdio.h>
#include "py/obj.h"
#include "py/runtime.h"

const mp_obj_type_t vector_type;

typedef struct _vector_obj_t {
    mp_obj_base_t base;
    float x, y, z;
} vector_obj_t;

STATIC mp_obj_t vector_length(mp_obj_t o_in) {
    if(!mp_obj_is_type(o_in, &vector_type)) {
        mp_raise_TypeError("argument is not a vector");
    }
    vector_obj_t *vector = MP_OBJ_TO_PTR(o_in);
    return mp_obj_new_float(sqrtf(vector->x*vector->x + vector->y*vector->y + vector->z*vector->z));
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(vector_length_obj, vector_length);

STATIC void vector_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    (void)kind;
    vector_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_print_str(print, "vector(");
    mp_obj_print_helper(print, mp_obj_new_float(self->x), PRINT_REPR);
    mp_print_str(print, ", ");
    mp_obj_print_helper(print, mp_obj_new_float(self->y), PRINT_REPR);
    mp_print_str(print, ", ");
    mp_obj_print_helper(print, mp_obj_new_float(self->z), PRINT_REPR);
    mp_print_str(print, ")");
}

STATIC mp_obj_t vector_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 3, 3, true);
    
    vector_obj_t *vector = m_new_obj(vector_obj_t);
    vector->base.type = &vector_type;
    vector->x = mp_obj_get_float(args[0]);
    vector->y = mp_obj_get_float(args[1]);
    vector->z = mp_obj_get_float(args[2]);
    return MP_OBJ_FROM_PTR(vector);
}

const mp_obj_type_t vector_type = {
    { &mp_type_type },
    .name = MP_QSTR_vector,
    .print = vector_print,
    .make_new = vector_make_new,
};

STATIC const mp_rom_map_elem_t vector_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_vector) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_vector), (mp_obj_t)&vector_type },
    { MP_ROM_QSTR(MP_QSTR_length), MP_ROM_PTR(&vector_length_obj) },
};
STATIC MP_DEFINE_CONST_DICT(vector_module_globals, vector_module_globals_table);

const mp_obj_module_t vector_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&vector_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_vector, vector_user_cmodule, MODULE_VECTOR_ENABLED);
