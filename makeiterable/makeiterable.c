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

typedef struct _itarray_obj_t {
    mp_obj_base_t base;
    mp_fun_1_t iternext;
    uint16_t *elements;
    size_t len;
} itarray_obj_t;

const mp_obj_type_t iterable_array_type;
mp_obj_t mp_obj_new_itarray_iterator(mp_obj_t , size_t , mp_obj_iter_buf_t *);

STATIC void itarray_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    (void)kind;
    itarray_obj_t *self = MP_OBJ_TO_PTR(self_in);
    printf("itarray: ");
    for(uint16_t i=0; i < self->len; i++) {
        printf("%d ", self->elements[i]);
    }
    printf("\n");
}

STATIC mp_obj_t itarray_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 1, 1, true);
    itarray_obj_t *self = m_new_obj(itarray_obj_t);
    self->base.type = &iterable_array_type;
    self->len = mp_obj_get_int(args[0]);
    uint16_t *arr = malloc(self->len * sizeof(uint16_t));
    for(uint16_t i=0; i < self->len; i++) {
        arr[i] = i*i;
    }
    self->elements = arr;
    return MP_OBJ_FROM_PTR(self);
}

STATIC mp_obj_t itarray_getiter(mp_obj_t o_in, mp_obj_iter_buf_t *iter_buf) {
    return mp_obj_new_itarray_iterator(o_in, 0, iter_buf);
}

const mp_obj_type_t iterable_array_type = {
    { &mp_type_type },
    .name = MP_QSTR_itarray,
    .print = itarray_print,
    .make_new = itarray_make_new,
    .getiter = itarray_getiter,
};

STATIC const mp_rom_map_elem_t makeiterable_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_makeiterable) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_square), (mp_obj_t)&iterable_array_type },	
};
STATIC MP_DEFINE_CONST_DICT(makeiterable_module_globals, makeiterable_module_globals_table);

const mp_obj_module_t makeiterable_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&makeiterable_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_makeiterable, makeiterable_user_cmodule, MODULE_MAKEITERABLE_ENABLED);

// itarray iterator
typedef struct _mp_obj_itarray_it_t {
    mp_obj_base_t base;
    mp_fun_1_t iternext;
    mp_obj_t itarray;
    size_t cur;
} mp_obj_itarray_it_t;

mp_obj_t itarray_iternext(mp_obj_t self_in) {
    mp_obj_itarray_it_t *self = MP_OBJ_TO_PTR(self_in);
    itarray_obj_t *itarray = MP_OBJ_TO_PTR(self->itarray);
    if (self->cur < itarray->len) {
        // read the current value
        uint16_t *arr = itarray->elements;
        mp_obj_t o_out = MP_OBJ_NEW_SMALL_INT(arr[self->cur]);
        self->cur += 1;
        return o_out;
    } else {
        return MP_OBJ_STOP_ITERATION;
    }
}

mp_obj_t mp_obj_new_itarray_iterator(mp_obj_t itarray, size_t cur, mp_obj_iter_buf_t *iter_buf) {
    assert(sizeof(mp_obj_itarray_it_t) <= sizeof(mp_obj_iter_buf_t));
    mp_obj_itarray_it_t *o = (mp_obj_itarray_it_t*)iter_buf;
    o->base.type = &mp_type_polymorph_iter;
    o->iternext = itarray_iternext;
    o->itarray = itarray;
    o->cur = cur;
    return MP_OBJ_FROM_PTR(o);
}
