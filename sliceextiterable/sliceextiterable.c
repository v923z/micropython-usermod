#include <stdlib.h>
#include "py/obj.h"
#include "py/runtime.h"

#if 0
typedef struct _sliceextitarray_obj_t {
    mp_obj_base_t base;
    mp_fun_1_t iternext;
    uint16_t *elements;
    size_t len;
} sliceextitarray_obj_t;

const mp_obj_type_t sliceextiterable_array_type;
mp_obj_t mp_obj_new_sliceextitarray_iterator(mp_obj_t , size_t , mp_obj_iter_buf_t *);

STATIC void sliceextitarray_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
	(void)kind;
	sliceextitarray_obj_t *self = MP_OBJ_TO_PTR(self_in);
	printf("sliceextitarray: ");
	for(uint16_t i=0; i < self->len; i++) {
		printf("%d ", self->elements[i]);
	}
	printf("\n");
}

sliceitarray_obj_t *create_new_sliceextitarray(uint16_t len) {
	sliceextitarray_obj_t *self = m_new_obj(sliceextitarray_obj_t);
	self->base.type = &sliceextiterable_array_type;
	self->len = len;
	uint16_t *arr = malloc(self->len * sizeof(uint16_t));
	self->elements = arr;
	return self;
}

STATIC mp_obj_t sliceextitarray_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
	mp_arg_check_num(n_args, n_kw, 1, 1, true);
	sliceextitarray_obj_t *self = create_new_sliceextitarray(mp_obj_get_int(args[0]));
	for(uint16_t i=0; i < self->len; i++) {
		self->elements[i] = i*i;
	}
	return MP_OBJ_FROM_PTR(self);
}

STATIC mp_obj_t sliceextitarray_getiter(mp_obj_t o_in, mp_obj_iter_buf_t *iter_buf) {
    return mp_obj_new_sliceextitarray_iterator(o_in, 0, iter_buf);
}

STATIC mp_obj_t sliceextitarray_subscr(mp_obj_t self_in, mp_obj_t index, mp_obj_t value) {
	sliceitarray_obj_t *self = MP_OBJ_TO_PTR(self_in);
    if (value == MP_OBJ_SENTINEL) { // simply return the values at index, no assignment

// Insert this here
    mp_obj_t ostart, ostop, ostep;
    mp_obj_slice_get(index, &ostart, &ostop, &ostep);
    printf("%lu", mp_obj_get_int(ostop));
///

#if MICROPY_PY_BUILTINS_SLICE
        if (MP_OBJ_IS_TYPE(index, &mp_type_slice)) {
            mp_bound_slice_t slice;
            if (!mp_seq_get_fast_slice_indexes(self->len, index, &slice)) {
				return mp_const_none;
            }
            sliceitarray_obj_t *res = create_new_sliceextitarray(slice.stop - slice.start);
            for(size_t i=0; i < slice.stop-slice.start; i++) {
				res->elements[i] = self->elements[i+slice.start];
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

const mp_obj_type_t sliceextiterable_array_type = {
    { &mp_type_type },
    .name = MP_QSTR_sliceextitarray,
    .print = sliceextitarray_print,
    .make_new = sliceextitarray_make_new,
    .getiter = sliceextitarray_getiter,
    .subscr = sliceextitarray_subscr,
};

STATIC const mp_rom_map_elem_t sliceextiterable_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_sliceextiterable) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_square), (mp_obj_t)&sliceextiterable_array_type },
};
STATIC MP_DEFINE_CONST_DICT(sliceextiterable_module_globals, sliceextiterable_module_globals_table);

const mp_obj_module_t sliceextiterable_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&sliceextiterable_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_sliceextiterable, sliceextiterable_user_cmodule, MODULE_SLICEEXTITERABLE_ENABLED);

// itarray iterator
typedef struct _mp_obj_sliceextitarray_it_t {
    mp_obj_base_t base;
    mp_fun_1_t iternext;
    mp_obj_t sliceextitarray;
    size_t cur;
} mp_obj_sliceextitarray_it_t;

mp_obj_t sliceextitarray_iternext(mp_obj_t self_in) {
    mp_obj_sliceextitarray_it_t *self = MP_OBJ_TO_PTR(self_in);
    sliceextitarray_obj_t *sliceextitarray = MP_OBJ_TO_PTR(self->sliceextitarray);
    if (self->cur < sliceextitarray->len) {
		// read the current value
		uint16_t *arr = sliceextitarray->elements;
        mp_obj_t o_out = MP_OBJ_NEW_SMALL_INT(arr[self->cur]);
        self->cur += 1;
        return o_out;
    } else {
        return MP_OBJ_STOP_ITERATION;
    }
}

mp_obj_t mp_obj_new_sliceextitarray_iterator(mp_obj_t sliceitarray, size_t cur, mp_obj_iter_buf_t *iter_buf) {
    assert(sizeof(mp_obj_sliceextitarray_it_t) <= sizeof(mp_obj_iter_buf_t));
    mp_obj_sliceextitarray_it_t *o = (mp_obj_sliceextitarray_it_t*)iter_buf;
    o->base.type = &mp_type_polymorph_iter;
    o->iternext = sliceextitarray_iternext;
    o->sliceextitarray = sliceextitarray;
    o->cur = cur;
    return MP_OBJ_FROM_PTR(o);
}    
#endif
