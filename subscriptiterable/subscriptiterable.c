/*
 * This file is part of the micropython-usermod project, 
 *
 * https://github.com/v923z/micropython-usermod
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Zoltán Vörös
*/
    
#include <stdlib.h>
#include "py/obj.h"
#include "py/runtime.h"

typedef struct _subitarray_obj_t {
    mp_obj_base_t base;
    mp_fun_1_t iternext;
    uint16_t *elements;
    size_t len;
} subitarray_obj_t;

const mp_obj_type_t subiterable_array_type;
mp_obj_t mp_obj_new_subitarray_iterator(mp_obj_t , size_t , mp_obj_iter_buf_t *);

STATIC void subitarray_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    (void)kind;
    subitarray_obj_t *self = MP_OBJ_TO_PTR(self_in);
    printf("subitarray: ");
    for(uint16_t i=0; i < self->len; i++) {
        printf("%d ", self->elements[i]);
    }
    printf("\n");
}

STATIC mp_obj_t subitarray_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 1, 1, true);
    subitarray_obj_t *self = m_new_obj(subitarray_obj_t);
    self->base.type = &subiterable_array_type;
    self->len = mp_obj_get_int(args[0]);
    uint16_t *arr = malloc(self->len * sizeof(uint16_t));
    for(uint16_t i=0; i < self->len; i++) {
        arr[i] = i*i;
    }
    self->elements = arr;
    return MP_OBJ_FROM_PTR(self);
}

STATIC mp_obj_t subitarray_getiter(mp_obj_t o_in, mp_obj_iter_buf_t *iter_buf) {
    return mp_obj_new_subitarray_iterator(o_in, 0, iter_buf);
}

STATIC mp_obj_t subitarray_subscr(mp_obj_t self_in, mp_obj_t index, mp_obj_t value) {
    subitarray_obj_t *self = MP_OBJ_TO_PTR(self_in);
    size_t idx = mp_obj_get_int(index);
    if(self->len <= idx) {
        mp_raise_msg(&mp_type_IndexError, "index is out of range");
    }
    if (value == MP_OBJ_SENTINEL) { // simply return the value at index, no assignment 		
        return MP_OBJ_NEW_SMALL_INT(self->elements[idx]);
    } else { // value was passed, replace the element at index
        self->elements[idx] = mp_obj_get_int(value);
    }
    return mp_const_none;
}

const mp_obj_type_t subiterable_array_type = {
    { &mp_type_type },
    .name = MP_QSTR_subitarray,
    .print = subitarray_print,
    .make_new = subitarray_make_new,
    .getiter = subitarray_getiter,
    .subscr = subitarray_subscr,
};

STATIC const mp_rom_map_elem_t subscriptiterable_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_subscriptiterable) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_square), (mp_obj_t)&subiterable_array_type },
};
STATIC MP_DEFINE_CONST_DICT(subscriptiterable_module_globals, subscriptiterable_module_globals_table);

const mp_obj_module_t subscriptiterable_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&subscriptiterable_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_subscriptiterable, subscriptiterable_user_cmodule, MODULE_SUBSCRIPTITERABLE_ENABLED);

// itarray iterator
typedef struct _mp_obj_subitarray_it_t {
    mp_obj_base_t base;
    mp_fun_1_t iternext;
    mp_obj_t subitarray;
    size_t cur;
} mp_obj_subitarray_it_t;

mp_obj_t subitarray_iternext(mp_obj_t self_in) {
    mp_obj_subitarray_it_t *self = MP_OBJ_TO_PTR(self_in);
    subitarray_obj_t *subitarray = MP_OBJ_TO_PTR(self->subitarray);
    if (self->cur < subitarray->len) {
        // read the current value
        uint16_t *arr = subitarray->elements;
        mp_obj_t o_out = MP_OBJ_NEW_SMALL_INT(arr[self->cur]);
        self->cur += 1;
        return o_out;
    } else {
        return MP_OBJ_STOP_ITERATION;
    }
}

mp_obj_t mp_obj_new_subitarray_iterator(mp_obj_t subitarray, size_t cur, mp_obj_iter_buf_t *iter_buf) {
    assert(sizeof(mp_obj_subitarray_it_t) <= sizeof(mp_obj_iter_buf_t));
    mp_obj_subitarray_it_t *o = (mp_obj_subitarray_it_t*)iter_buf;
    o->base.type = &mp_type_polymorph_iter;
    o->iternext = subitarray_iternext;
    o->subitarray = subitarray;
    o->cur = cur;
    return MP_OBJ_FROM_PTR(o);
}
