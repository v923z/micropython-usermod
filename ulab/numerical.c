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
#include "py/runtime.h"
#include "numerical.h"

typedef struct {
    uint8_t uint8;
    int8_t int8;
    uint16_t uint16;
    int16_t int16;
    float float1, float2; // we need two floats, so that the standard deviation can be calculated
} numerical_results;

numerical_results n_results;

mp_obj_t numerical_linspace(mp_obj_t _start, mp_obj_t _stop, mp_obj_t _len) {
    // TODO: accept keyword argument endpoint=True
    mp_int_t len = mp_obj_get_int_truncated(_len);
    if(len < 2) {
        mp_raise_ValueError("number of points must be at least 2");
    }
    mp_float_t value, step;
    value = mp_obj_get_float(_start);
    step = (mp_obj_get_float(_stop)-value)/(len-1);
    ndarray_obj_t *nd_array = create_new_ndarray(1, len, NDARRAY_FLOAT);
    for(size_t i=0; i < len; i++, value += step) {
        mp_binary_set_val_array('f', nd_array->data->items, i, mp_obj_new_float(value));
    }
    return MP_OBJ_FROM_PTR(nd_array);
}
void clear_results(void) {
    n_results.uint8 = 0;
    n_results.int8 = 0;
    n_results.uint16 = 0;
    n_results.int16 = 0;
    n_results.float1 = n_results.float2 = 0.0;
}

void sum_matrix(void *data, size_t start, size_t stop, size_t stride, unsigned char type) {
    clear_results();
    for(size_t i=start; i < stop; i+= stride) {
        switch(type) {
            case NDARRAY_UINT8:
                n_results.uint8 += ((uint8_t *)data)[i];
                break;
            case NDARRAY_INT8:
                n_results.int8 += ((int8_t *)data)[i];
                break;
            case NDARRAY_UINT16:
                n_results.uint16 += ((uint16_t *)data)[i];
                break;
            case NDARRAY_INT16:
                n_results.int16 += ((int16_t *)data)[i];
                break;
            case NDARRAY_FLOAT:
                n_results.float1 += ((float *)data)[i];
        }
    }
}

mp_obj_t numerical_sum(mp_obj_t o_in, mp_obj_t axis) {
    // TODO: deal with keyword argument
    if(MP_OBJ_IS_TYPE(o_in, &mp_type_tuple) || MP_OBJ_IS_TYPE(o_in, &mp_type_list) || 
        MP_OBJ_IS_TYPE(o_in, &mp_type_range)) {
        mp_float_t _sum = 0.0;
        mp_obj_iter_buf_t iter_buf;
        mp_obj_t item, iterable = mp_getiter(o_in, &iter_buf);
        while ((item = mp_iternext(iterable)) != MP_OBJ_STOP_ITERATION) {
            _sum += mp_obj_get_float(item);
        }
        return mp_obj_new_float(_sum);
    } else if(MP_OBJ_IS_TYPE(o_in, &ulab_ndarray_type)) {
        ndarray_obj_t *o = MP_OBJ_TO_PTR(o_in);
        if((o->m == 1) || (o->n == 1)) {
            sum_matrix(o->data->items, 1, o->data->len, 1, o->data->typecode);
            if(o->data->typecode == NDARRAY_UINT8) {
                return mp_obj_new_int(n_results.uint8);
            } else if(o->data->typecode == NDARRAY_INT8) {
                return mp_obj_new_int(n_results.int8);
            } else if(o->data->typecode == NDARRAY_UINT16) {
                return mp_obj_new_int(n_results.uint16);
            } else if(o->data->typecode == NDARRAY_INT16) {
                return mp_obj_new_int(n_results.int16);
            } else {
                return mp_obj_new_float(n_results.float1);
            }
        } else {
            size_t m = (mp_obj_get_int(axis) == 1) ? 1 : o->m;
            size_t n = (mp_obj_get_int(axis) == 1) ? o->n : 1;
            ndarray_obj_t *out = create_new_ndarray(m, n, o->data->typecode);
            
            if(m == 1) { // sum vertically
                for(size_t i=0; i < n; i++) {
                    sum_matrix(o->data->items, i, o->data->len, m, o->data->typecode);
                    switch(o->data->typecode) {
                        case NDARRAY_UINT8:
                            *(uint8_t *)out->data->items[i] = n_results.uint8;
                            break;
                        case NDARRAY_INT8:
                            *(int8_t *)out->data->items[i] = n_results.int8;
                            break;
                        case NDARRAY_UINT16:
                            *(uint16_t *)out->data->items[i] = n_results.uint16;
                            break;
                        case NDARRAY_INT16:
                            *(int16_t *)out->data->items[i] = n_results.int16;
                            break;
                        case NDARRAY_FLOAT:
                            *(float_t *)out->data->items[i] = n_results.float1;
                            break;
                    }
                }
            } else { // sum horizontally
                for(size_t i=0; i < m; i++) {
                    sum_matrix(o->data->items, i*n, o->data->len, n, o->data->typecode);
                    switch(o->data->typecode) {
                        case NDARRAY_UINT8:
                            *(uint8_t *)out->data->items[i] = n_results.uint8;
                            break;
                        case NDARRAY_INT8:
                            *(int8_t *)out->data->items[i] = n_results.int8;
                            break;
                        case NDARRAY_UINT16:
                            *(uint16_t *)out->data->items[i] = n_results.uint16;
                            break;
                        case NDARRAY_INT16:
                            *(int16_t *)out->data->items[i] = n_results.int16;
                            break;
                        case NDARRAY_FLOAT:
                            *(float_t *)out->data->items[i] = n_results.float1;
                            break;
                    }
                }                
            }
        }
    } else {
        mp_raise_TypeError("input must be tuple, list, range, or ndarray");
    }
    return mp_const_none;
}
