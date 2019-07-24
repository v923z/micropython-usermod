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

typedef struct _sliceitarray_obj_t {
    mp_obj_base_t base;
    mp_fun_1_t iternext;
    uint16_t *elements;
    size_t len;
} sliceitarray_obj_t;

const mp_obj_type_t sliceiterable_array_type;
mp_obj_t mp_obj_new_sliceitarray_iterator(mp_obj_t , size_t , mp_obj_iter_buf_t *);

STATIC void sliceitarray_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    (void)kind;
    sliceitarray_obj_t *self = MP_OBJ_TO_PTR(self_in);
    printf("sliceitarray: ");
    for(uint16_t i=0; i < self->len; i++) {
        printf("%d ", self->elements[i]);
    }
    printf("\n");
}

sliceitarray_obj_t *create_new_sliceitarray(uint16_t len) {
    sliceitarray_obj_t *self = m_new_obj(sliceitarray_obj_t);
    self->base.type = &sliceiterable_array_type;
    self->len = len;
    uint16_t *arr = malloc(self->len * sizeof(uint16_t));
    self->elements = arr;
    return self;
}

STATIC mp_obj_t sliceitarray_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 1, 1, true);
    sliceitarray_obj_t *self = create_new_sliceitarray(mp_obj_get_int(args[0]));
    for(uint16_t i=0; i < self->len; i++) {
        self->elements[i] = i*i;
    }
    return MP_OBJ_FROM_PTR(self);
}

STATIC mp_obj_t sliceitarray_getiter(mp_obj_t o_in, mp_obj_iter_buf_t *iter_buf) {
    return mp_obj_new_sliceitarray_iterator(o_in, 0, iter_buf);
}

STATIC mp_obj_t sliceitarray_subscr(mp_obj_t self_in, mp_obj_t index, mp_obj_t value) {
	sliceitarray_obj_t *self = MP_OBJ_TO_PTR(self_in);
    if (value == MP_OBJ_SENTINEL) { // simply return the values at index, no assignment

#if MICROPY_PY_BUILTINS_SLICE
        if (MP_OBJ_IS_TYPE(index, &mp_type_slice)) {
            mp_bound_slice_t slice;
            mp_seq_get_fast_slice_indexes(self->len, index, &slice);
            printf("start: %ld, stop: %ld, step: %ld\n", slice.start, slice.stop, slice.step);
            uint16_t len = (slice.stop - slice.start) / slice.step;
            sliceitarray_obj_t *res = create_new_sliceitarray(len);
            for(size_t i=0; i < len; i++) {
                res->elements[i] = self->elements[slice.start+i*slice.step];
            }
            return MP_OBJ_FROM_PTR(res);
        }
#endif
        // we have a single index, return a single number
        size_t idx = mp_obj_get_int(index);
        return MP_OBJ_NEW_SMALL_INT(self->elements[idx]);
    } else { // do not deal with assignment, bail out
        return mp_const_none;
    }
    return mp_const_none;
}

const mp_obj_type_t sliceiterable_array_type = {
    { &mp_type_type },
    .name = MP_QSTR_sliceitarray,
    .print = sliceitarray_print,
    .make_new = sliceitarray_make_new,
    .getiter = sliceitarray_getiter,
    .subscr = sliceitarray_subscr,
};

STATIC const mp_rom_map_elem_t sliceiterable_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_sliceiterable) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_square), (mp_obj_t)&sliceiterable_array_type },
};
STATIC MP_DEFINE_CONST_DICT(sliceiterable_module_globals, sliceiterable_module_globals_table);

const mp_obj_module_t sliceiterable_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&sliceiterable_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_sliceiterable, sliceiterable_user_cmodule, MODULE_SLICEITERABLE_ENABLED);

// itarray iterator
typedef struct _mp_obj_sliceitarray_it_t {
    mp_obj_base_t base;
    mp_fun_1_t iternext;
    mp_obj_t sliceitarray;
    size_t cur;
} mp_obj_sliceitarray_it_t;

mp_obj_t sliceitarray_iternext(mp_obj_t self_in) {
    mp_obj_sliceitarray_it_t *self = MP_OBJ_TO_PTR(self_in);
    sliceitarray_obj_t *sliceitarray = MP_OBJ_TO_PTR(self->sliceitarray);
    if (self->cur < sliceitarray->len) {
        // read the current value
        uint16_t *arr = sliceitarray->elements;
        mp_obj_t o_out = MP_OBJ_NEW_SMALL_INT(arr[self->cur]);
        self->cur += 1;
        return o_out;
    } else {
        return MP_OBJ_STOP_ITERATION;
    }
}

mp_obj_t mp_obj_new_sliceitarray_iterator(mp_obj_t sliceitarray, size_t cur, mp_obj_iter_buf_t *iter_buf) {
    assert(sizeof(mp_obj_sliceitarray_it_t) <= sizeof(mp_obj_iter_buf_t));
    mp_obj_sliceitarray_it_t *o = (mp_obj_sliceitarray_it_t*)iter_buf;
    o->base.type = &mp_type_polymorph_iter;
    o->iternext = sliceitarray_iternext;
    o->sliceitarray = sliceitarray;
    o->cur = cur;
    return MP_OBJ_FROM_PTR(o);
}
