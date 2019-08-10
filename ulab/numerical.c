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
#include "py/obj.h"
#include "py/runtime.h"
#include "py/builtin.h"
#include "numerical.h"

enum SUM_MEAN_STD_TYPE {
    NUMERICAL_SUM = 1,
    NUMERICAL_MEAN = 2,
    NUMERICAL_STD = 3
};


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

void numerical_parse_args(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args, 
                              mp_obj_t *oin, int8_t *axis) {
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_oin, MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_rom_obj = MP_ROM_PTR(&mp_const_none_obj)} } ,
        { MP_QSTR_axis, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 1} },
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(1, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);
    
    *oin = args[0].u_obj;
    *axis = args[1].u_int;
}

void sum_matrix(void *in, void *out, size_t idx, size_t start, size_t stop, size_t stride, unsigned char type) {
    for(size_t i=start; i < stop; i += stride) {
        switch(type) {
            case NDARRAY_UINT8:
                ((uint8_t *)out)[idx] += ((uint8_t *)in)[i];
                break;
            case NDARRAY_INT8:
                ((int8_t *)out)[idx] += ((int8_t *)in)[i];            
                break;
            case NDARRAY_UINT16:
                ((uint16_t *)out)[idx] += ((uint16_t *)in)[i];
                break;
            case NDARRAY_INT16:
                ((int16_t *)out)[idx] += ((int16_t *)in)[i];
                break;
            case NDARRAY_FLOAT:
                ((float_t *)out)[idx] += ((float_t *)in)[i];
                break;
        }
    }
}

mp_obj_t sum_mean_std_array(mp_obj_t oin, uint8_t type) {
    mp_float_t value, sum = 0.0, sq_sum = 0.0;
    mp_obj_iter_buf_t iter_buf;
    mp_obj_t item, iterable = mp_getiter(oin, &iter_buf);
    mp_int_t len = mp_obj_get_int(mp_obj_len(oin));
    while ((item = mp_iternext(iterable)) != MP_OBJ_STOP_ITERATION) {
        value = mp_obj_get_float(item);
        sum += value;
        if(type == NUMERICAL_STD) {
            sq_sum += value*value;
        }
    }
    switch(type) {
        case NUMERICAL_SUM:
            return mp_obj_new_float(sum);
        case NUMERICAL_MEAN:
            return mp_obj_new_float(sum/len);
        case NUMERICAL_STD:
            sum /= len; // this is now the mean!
            return mp_obj_new_float(sqrtf((sq_sum/len-sum*sum)));
    }
    return mp_const_none;
}

mp_obj_t numerical_sum(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    mp_obj_t oin;
    int8_t axis;
    numerical_parse_args(n_args, pos_args, kw_args, &oin, &axis);
    
    if(MP_OBJ_IS_TYPE(oin, &mp_type_tuple) || MP_OBJ_IS_TYPE(oin, &mp_type_list) || 
        MP_OBJ_IS_TYPE(oin, &mp_type_range)) {
        return sum_mean_std_array(oin, NUMERICAL_SUM);
    } else if(MP_OBJ_IS_TYPE(oin, &ulab_ndarray_type)) {
        ndarray_obj_t *in = MP_OBJ_TO_PTR(oin);
        if((in->m == 1) || (in->n == 1)) {
            mp_raise_ValueError("summing of linear ndarrays has to be implemented");
        } else {
            size_t m = (axis == 1) ? 1 : in->m;
            size_t n = (axis == 1) ? in->n : 1;
            size_t len = in->data->len;
            ndarray_obj_t *out = create_new_ndarray(m, n, in->data->typecode);
            
            if(m == 1) { // sum vertically
                for(size_t i=0; i < n; i++) {
                    sum_matrix(in->data->items, out->data->items, i, i, len, n, in->data->typecode);
                }
            } else { // sum horizontally
                for(size_t i=0; i < m; i++) {
                    sum_matrix(in->data->items, out->data->items, i, i*in->n, (i+1)*in->n, 1, in->data->typecode);
                }                
            }
            return MP_OBJ_FROM_PTR(out);
        }
    } else {
        mp_raise_TypeError("input must be tuple, list, range, or ndarray");
    } 
    return mp_const_none;
}

mp_obj_t numerical_mean(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    mp_obj_t _sum = numerical_sum(n_args, pos_args, kw_args);
    mp_obj_t oin;
    int8_t axis;
    numerical_parse_args(n_args, pos_args, kw_args, &oin, &axis);
    if(MP_OBJ_IS_TYPE(oin, &mp_type_tuple) || MP_OBJ_IS_TYPE(oin, &mp_type_list) || 
        MP_OBJ_IS_TYPE(oin, &mp_type_range)) {
        return sum_mean_std_array(oin, NUMERICAL_MEAN);
    } else if(MP_OBJ_IS_TYPE(oin, &ulab_ndarray_type)) {
        ndarray_obj_t *in = MP_OBJ_TO_PTR(_sum);
        size_t len = in->data->len;
        ndarray_obj_t *_in = MP_OBJ_TO_PTR(oin);
        size_t contract_len = (axis == 1) ? _in->m : _in->n;
        // Here we could think about enforcing type retention, but that must be a keyword argument
        // Otherwise, we just turn everythin into floats
        if(in->data->typecode == NDARRAY_FLOAT) { 
            // With this if clause we can save an extra ndarray_obj_t
            // I am not sure it's worth it.
            for(size_t i=0; i < len; i++) {
                ((float_t *)in->data->items)[i] /= contract_len;
            }
            return MP_OBJ_FROM_PTR(in);
        }
        ndarray_obj_t *out = create_new_ndarray(in->m, in->n, NDARRAY_FLOAT);
        for(size_t i=0; i < len; i++) {
            switch(in->data->typecode) {
                case NDARRAY_UINT8:
                    ((float_t *)out->data->items)[i] = ((uint8_t *)in->data->items)[i] / contract_len;
                    break;
                case NDARRAY_INT8:
                    ((float_t *)out->data->items)[i] = ((int8_t *)in->data->items)[i] / contract_len;
                    break;
                case NDARRAY_UINT16:
                    ((float_t *)out->data->items)[i] = ((uint16_t *)in->data->items)[i] / contract_len;
                    break;
                case NDARRAY_INT16:
                    ((float_t *)out->data->items)[i] = ((int16_t *)in->data->items)[i] / contract_len;
                    break;                
            }
        }
        return MP_OBJ_FROM_PTR(out);
    } else {
        mp_raise_TypeError("input must be tuple, list, range, or ndarray");
    } 
    return mp_const_none;
}

mp_obj_t numerical_std(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    // The standard deviation is most certainly a float, so there is no point in 
    // trying to retain the type of the input object
    mp_obj_t oin;
    int8_t axis;
    numerical_parse_args(n_args, pos_args, kw_args, &oin, &axis);
    if(MP_OBJ_IS_TYPE(oin, &mp_type_tuple) || MP_OBJ_IS_TYPE(oin, &mp_type_list) || 
        MP_OBJ_IS_TYPE(oin, &mp_type_range)) {
        return sum_mean_std_array(oin, NUMERICAL_STD);
    } else if(MP_OBJ_IS_TYPE(oin, &ulab_ndarray_type)) {
         return mp_const_none;
    } else {
        mp_raise_TypeError("input must be tuple, list, range, or ndarray");
    } 
    return mp_const_none;
}
