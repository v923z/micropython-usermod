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

typedef struct _simpleclass_myclass_obj_t {
    mp_obj_base_t base;
    int16_t a;
    int16_t b;
} simpleclass_myclass_obj_t;

const mp_obj_type_t simpleclass_myclass_type;

STATIC void myclass_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    (void)kind;
    simpleclass_myclass_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_print_str(print, "myclass(");
    mp_obj_print_helper(print, mp_obj_new_int(self->a), PRINT_REPR);
    mp_print_str(print, ", ");
    mp_obj_print_helper(print, mp_obj_new_int(self->b), PRINT_REPR);
    mp_print_str(print, ")");
}

STATIC mp_obj_t myclass_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 2, 2, true);
    simpleclass_myclass_obj_t *self = m_new_obj(simpleclass_myclass_obj_t);
    self->base.type = &simpleclass_myclass_type;
    self->a = mp_obj_get_int(args[0]);
    self->b = mp_obj_get_int(args[1]);
    return MP_OBJ_FROM_PTR(self);
}

// Class methods
STATIC mp_obj_t myclass_sum(mp_obj_t self_in) {
    simpleclass_myclass_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_int(self->a + self->b);
}

MP_DEFINE_CONST_FUN_OBJ_1(myclass_sum_obj, myclass_sum);

STATIC const mp_rom_map_elem_t myclass_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_mysum), MP_ROM_PTR(&myclass_sum_obj) },
};

STATIC MP_DEFINE_CONST_DICT(myclass_locals_dict, myclass_locals_dict_table);

const mp_obj_type_t simpleclass_myclass_type = {
    { &mp_type_type },
    .name = MP_QSTR_simpleclass,
    .print = myclass_print,
    .make_new = myclass_make_new,
    .locals_dict = (mp_obj_dict_t*)&myclass_locals_dict,
};

// Module functions
STATIC mp_obj_t simpleclass_add(const mp_obj_t o_in) {
    simpleclass_myclass_obj_t *class_instance = MP_OBJ_TO_PTR(o_in);
    return mp_obj_new_int(class_instance->a + class_instance->b);
}

MP_DEFINE_CONST_FUN_OBJ_1(simpleclass_add_obj, simpleclass_add);

STATIC const mp_map_elem_t simpleclass_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_simpleclass) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_myclass), (mp_obj_t)&simpleclass_myclass_type },	
    { MP_OBJ_NEW_QSTR(MP_QSTR_add), (mp_obj_t)&simpleclass_add_obj },
};

STATIC MP_DEFINE_CONST_DICT (
    mp_module_simpleclass_globals,
    simpleclass_globals_table
);

const mp_obj_module_t simpleclass_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mp_module_simpleclass_globals,
};

MP_REGISTER_MODULE(MP_QSTR_simpleclass, simpleclass_user_cmodule, MODULE_SIMPLECLASS_ENABLED);
