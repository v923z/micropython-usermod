/*
 * This file is part of the micropython-ulab project, 
 *
 * https://github.com/v923z/micropython-ulab
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Zoltán Vörös
*/
    
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "py/runtime.h"
#include "py/binary.h"
#include "py/obj.h"
#include "py/objtuple.h"
#include "ndarray.h"

STATIC mp_obj_array_t *array_new(char typecode, size_t n) {
    int typecode_size = mp_binary_get_size('@', typecode, NULL);
    mp_obj_array_t *o = m_new_obj(mp_obj_array_t);
    // this step could probably be skipped: we are never going to store a bytearray per se
    #if MICROPY_PY_BUILTINS_BYTEARRAY && MICROPY_PY_ARRAY
    o->base.type = (typecode == BYTEARRAY_TYPECODE) ? &mp_type_bytearray : &mp_type_array;
    #elif MICROPY_PY_BUILTINS_BYTEARRAY
    o->base.type = &mp_type_bytearray;
    #else
    o->base.type = &mp_type_array;
    #endif
    o->typecode = typecode;
    o->free = 0;
    o->len = n;
    o->items = m_new(byte, typecode_size * o->len);
    return o;
}

void ndarray_print_row(const mp_print_t *print, mp_obj_array_t *data, size_t n0, size_t n) {
    mp_print_str(print, "[");
    size_t i;
    if(n < PRINT_MAX) { // if the array is short, print everything
        mp_obj_print_helper(print, mp_binary_get_val_array(data->typecode, data->items, n0), PRINT_REPR);
        for(i=1; i<n; i++) {
            mp_print_str(print, ", ");
            mp_obj_print_helper(print, mp_binary_get_val_array(data->typecode, data->items, n0+i), PRINT_REPR);
        }
    } else {
        mp_obj_print_helper(print, mp_binary_get_val_array(data->typecode, data->items, n0), PRINT_REPR);
        for(i=1; i<3; i++) {
            mp_print_str(print, ", ");
            mp_obj_print_helper(print, mp_binary_get_val_array(data->typecode, data->items, n0+i), PRINT_REPR);
        }
        mp_printf(print, ", ..., ");
        mp_obj_print_helper(print, mp_binary_get_val_array(data->typecode, data->items, n0+n-3), PRINT_REPR);
        for(size_t i=1; i<3; i++) {
            mp_print_str(print, ", ");
            mp_obj_print_helper(print, mp_binary_get_val_array(data->typecode, data->items, n0+n-3+i), PRINT_REPR);
        }
    }
    mp_print_str(print, "]");
}

void ulab_ndarray_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    (void)kind;
    ndarray_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_print_str(print, "ndarray(");	
    if((self->m == 1) || (self->n == 1)) {
        ndarray_print_row(print, self->data, 0, self->data->len);
    } else {
        // TODO: add vertical ellipses for the case, when self->m > PRINT_MAX
        mp_print_str(print, "[");
        ndarray_print_row(print, self->data, 0, self->n);
        for(size_t i=1; i < self->m; i++) {
            mp_print_str(print, ",\n\t ");
            ndarray_print_row(print, self->data, i*self->n, self->n);
        }
        mp_print_str(print, "]");
    }
    // TODO: print typecode
    mp_print_str(print, ")\n");
}

void assign_elements(mp_obj_array_t *data, mp_obj_t iterable, uint8_t bytecode, size_t *idx) {
    // assigns a single row in the matrix
    mp_obj_t item;
    while ((item = mp_iternext(iterable)) != MP_OBJ_STOP_ITERATION) {
        mp_binary_set_val_array(bytecode, data->items, (*idx)++, item);
    }
}

ndarray_obj_t *create_new_ndarray(size_t m, size_t n, uint8_t typecode) {
    // Creates the base ndarray with shape (m, n), and initialises the values to straight 0s
    ndarray_obj_t *ndarray = m_new_obj(ndarray_obj_t);
    ndarray->base.type = &ulab_ndarray_type;
    ndarray->m = m;
    ndarray->n = n;
    mp_obj_array_t *data = array_new(typecode, m*n);
    ndarray->bytes = m * n * mp_binary_get_size('@', typecode, NULL);
    // this should set all elements to 0, irrespective of the of the typecode (all bits are zero)
    // we could, perhaps, leave this step out, and initialise the array, only, when needed
    memset(data->items, 0, ndarray->bytes); 
    ndarray->data = data;
    return ndarray;
}

mp_obj_t ulab_ndarray_copy(mp_obj_t self_in) {
    // returns a verbatim (shape and typecode) copy of self_in
    ndarray_obj_t *self = MP_OBJ_TO_PTR(self_in);
    ndarray_obj_t *out = create_new_ndarray(self->m, self->n, self->data->typecode);
    int typecode_size = mp_binary_get_size('@', self->data->typecode, NULL);
    memcpy(out->data->items, self->data->items, self->data->len*typecode_size);
    return MP_OBJ_FROM_PTR(out);
}

mp_obj_t ulab_ndarray_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 1, 3, true);
    
    size_t len1, len2=0, i=0;
    mp_obj_t len_in = mp_obj_len_maybe(args[0]);
    if (len_in == MP_OBJ_NULL) {
        mp_raise_ValueError("first argument must be an iterable");
    } else {
        len1 = MP_OBJ_SMALL_INT_VALUE(len_in);
    }

    // We have to figure out, whether the first element of the iterable is an iterable
    // Perhaps, there is a more elegant way of handling this
    mp_obj_iter_buf_t iter_buf1;
    mp_obj_t item1, iterable1 = mp_getiter(args[0], &iter_buf1);
    while ((item1 = mp_iternext(iterable1)) != MP_OBJ_STOP_ITERATION) {
        len_in = mp_obj_len_maybe(item1);
        if(len_in != MP_OBJ_NULL) { // indeed, this seems to be an iterable
            // Next, we have to check, whether all elements in the outer loop have the same length
            if(i > 0) {
                if(len2 != MP_OBJ_SMALL_INT_VALUE(len_in)) {
                    mp_raise_ValueError("iterables are not of the same length");
                }
            }
            len2 = MP_OBJ_SMALL_INT_VALUE(len_in);
            i++;
        }
    }
    // By this time, it should be established, what the shape is, so we can now create the array
    // set the typecode to float, if the format specifier is missing
    // TODO: this would probably be more elegant with keyword arguments... 
    uint8_t typecode;
    if(n_args == 1) {
        typecode = NDARRAY_FLOAT;
    } else {
        typecode = (uint8_t)mp_obj_get_int(args[1]);
    }
    ndarray_obj_t *self = create_new_ndarray(len1, (len2 == 0) ? 1 : len2, typecode);
    iterable1 = mp_getiter(args[0], &iter_buf1);
    i = 0;
    if(len2 == 0) { // the first argument is a single iterable
        assign_elements(self->data, iterable1, typecode, &i);
    } else {
        mp_obj_iter_buf_t iter_buf2;
        mp_obj_t iterable2; 

        while ((item1 = mp_iternext(iterable1)) != MP_OBJ_STOP_ITERATION) {
            iterable2 = mp_getiter(item1, &iter_buf2);
            assign_elements(self->data, iterable2, typecode, &i);
        }
    }
    return MP_OBJ_FROM_PTR(self);
}


mp_obj_t ndarray_shape(mp_obj_t self_in) {
    ndarray_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_obj_t tuple[2] = {
        mp_obj_new_int(self->m),
        mp_obj_new_int(self->n)
    };
    return mp_obj_new_tuple(2, tuple);
}

mp_obj_t ndarray_size(mp_obj_t self_in, mp_obj_t axis) {
    ndarray_obj_t *self = MP_OBJ_TO_PTR(self_in);
    uint8_t ax = mp_obj_get_int(axis);
    if(ax == 0) {
        return mp_obj_new_int(self->data->len);
    } else if(ax == 1) {
        return mp_obj_new_int(self->m);
    } else if(ax == 2) {
        return mp_obj_new_int(self->n);
    } else {
        return mp_const_none;
    }
}

mp_obj_t ndarray_rawsize(mp_obj_t self_in) {
    // returns a 5-tuple with the 
    // 
    // 1. number of rows
    // 2. number of columns
    // 3. length of the storage (should be equal to the product of 1. and 2.)
    // 4. length of the data storage in bytes
    // 5. datum size in bytes
    ndarray_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_obj_tuple_t *tuple = MP_OBJ_TO_PTR(mp_obj_new_tuple(5, NULL));
    tuple->items[0] = MP_OBJ_NEW_SMALL_INT(self->m);
    tuple->items[1] = MP_OBJ_NEW_SMALL_INT(self->n);
    tuple->items[2] = MP_OBJ_NEW_SMALL_INT(self->bytes);
    tuple->items[3] = MP_OBJ_NEW_SMALL_INT(self->data->len);
    tuple->items[4] = MP_OBJ_NEW_SMALL_INT(mp_binary_get_size('@', self->data->typecode, NULL));
    return tuple;
}
