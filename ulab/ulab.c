#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "py/runtime.h"
#include "py/binary.h"
#include "py/obj.h"
#include "py/objarray.h"

#define SWAP(t, a, b) { t tmp = a; a = b; b = tmp; }
#define PRINT_MAX	10
#define epsilon		1e-6

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

enum NDARRAY_TYPE {
	NDARRAY_UINT8 = 'b',
	NDARRAY_INT8 = 'B',
	NDARRAY_UINT16 = 'i', 
	NDARRAY_INT16 = 'I',
	NDARRAY_FLOAT = 'f'
};

typedef struct _ndarray_obj_t {
	mp_obj_base_t base;
	size_t m, n;
	mp_obj_array_t *data;
	size_t bytes;
} ndarray_obj_t;

STATIC void ndarray_print_row(const mp_print_t *print, mp_obj_array_t *data, size_t n0, size_t n) {
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

STATIC void ulab_ndarray_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
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

const mp_obj_type_t ulab_ndarray_type;

STATIC void assign_elements(mp_obj_array_t *data, mp_obj_t iterable, uint8_t bytecode, size_t *idx) {
	// assigns a single row in the matrix
	mp_obj_t item;
	while ((item = mp_iternext(iterable)) != MP_OBJ_STOP_ITERATION) {
		mp_binary_set_val_array(bytecode, data->items, (*idx)++, item);
	}	
}

STATIC ndarray_obj_t *create_new_ndarray(size_t m, size_t n, uint8_t typecode) {
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

STATIC mp_obj_t ulab_ndarray_copy(mp_obj_t self_in) {
	// returns a verbatim (shape and typecode) copy of self_in
	ndarray_obj_t *self = MP_OBJ_TO_PTR(self_in);
	ndarray_obj_t *out = create_new_ndarray(self->m, self->n, self->data->typecode);
	int typecode_size = mp_binary_get_size('@', self->data->typecode, NULL);
	memcpy(out->data->items, self->data->items, self->data->len*typecode_size);
	return MP_OBJ_FROM_PTR(out);
}

STATIC mp_obj_t ulab_ndarray_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
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


STATIC mp_obj_t ulab_ndarray_transpose(mp_obj_t self_in) {
	ndarray_obj_t *self = MP_OBJ_TO_PTR(self_in);
	// the size of a single item in the array
	uint8_t _sizeof = mp_binary_get_size('@', self->data->typecode, NULL);
	
	// NOTE: In principle, we could simply specify the stride direction, and then we wouldn't 
	// even have to shuffle the elements. The downside of that approach is that we would have 
	// to implement two versions of the matrix multiplication and inversion functions
	
	// NOTE: 
	// if the matrices are square, we can simply swap items, but 
	// generic matrices can't be transposed in place, so we have to 
	// declare a temporary variable
	
	// NOTE: 
	//  In the old matrix, the coordinate (m, n) is m*self->n + n
	//  We have to assign this to the coordinate (n, m) in the new 
	//  matrix, i.e., to n*self->m + m
	
	// one-dimensional arrays can be transposed by simply swapping the dimensions
	if((self->m != 1) && (self->n != 1)) {
		uint8_t *c = (uint8_t *)self->data->items;
		// self->bytes is the size of the bytearray, irrespective of the typecode
		uint8_t *tmp = (uint8_t *)malloc(self->bytes);
		for(size_t m=0; m < self->m; m++) {
			for(size_t n=0; n < self->n; n++) {
				memcpy(tmp+_sizeof*(n*self->m + m), c+_sizeof*(m*self->n + n), _sizeof);
			}
		}
		memcpy(self->data->items, tmp, self->bytes);
		free(tmp);
	} 
	SWAP(size_t, self->m, self->n);
	return mp_const_none;
}

MP_DEFINE_CONST_FUN_OBJ_1(ulab_ndarray_transpose_obj, ulab_ndarray_transpose);


STATIC mp_obj_t ulab_ndarray_shape(mp_obj_t self_in) {
	ndarray_obj_t *self = MP_OBJ_TO_PTR(self_in);
	mp_obj_t tuple[2] = {
		mp_obj_new_int(self->m),
		mp_obj_new_int(self->n)
	};
	return mp_obj_new_tuple(2, tuple);
}

MP_DEFINE_CONST_FUN_OBJ_1(ulab_ndarray_shape_obj, ulab_ndarray_shape);


STATIC mp_obj_t ulab_ndarray_size(mp_obj_t self_in, mp_obj_t axis) {
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

MP_DEFINE_CONST_FUN_OBJ_2(ulab_ndarray_size_obj, ulab_ndarray_size);

STATIC mp_obj_t ulab_ndarray_rawsize(mp_obj_t self_in) {
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

MP_DEFINE_CONST_FUN_OBJ_1(ulab_ndarray_rawsize_obj, ulab_ndarray_rawsize);

ndarray_obj_t *invert_matrix(mp_obj_array_t *data, size_t N) {
	// After inversion the matrix is most certainly a float
	ndarray_obj_t *tmp = create_new_ndarray(N, N, NDARRAY_FLOAT);
	// initially, this is the unit matrix: this is what will be returned a
	// after all the transformations
	ndarray_obj_t *unitm = create_new_ndarray(N, N, NDARRAY_FLOAT);
	
	float *c = (float *)tmp->data->items;
	float *unit = (float *)unitm->data->items;
	mp_obj_t elem;
	float elemf;
	for(size_t m=0; m < N; m++) { // rows first
		for(size_t n=0; n < N; n++) { // columns next
			elem = mp_binary_get_val_array(data->typecode, data->items, m*N+n);
			elemf = (float)mp_obj_get_float(elem);
			memcpy(&c[m*N+n], &elemf, sizeof(float));
		}
		// initialise the unit matrix
		elemf = 1.0;
		memcpy(&unit[m*(N+1)], &elemf, sizeof(float));
	}
	for(size_t m=0; m < N; m++){
		// this could be faster with ((c < epsilon) && (c > -epsilon))
		if(abs(c[m*(N+1)]) < epsilon) {
			// TODO: check what kind of exception numpy raises
			mp_raise_ValueError("input matrix is singular");
		}
		for(size_t n=0; n < N; n++){
			if(m != n){
				elemf = c[N*n+m] / c[m*(N+1)];
				for(size_t k=0; k < N; k++){
					c[N*n+k] -= elemf * c[N*m+k];
					unit[N*n+k] -= elemf * unit[N*m+k];					
				}
			}
		}
	}
    for(size_t m=0; m < N; m++){ 
        elemf = c[m*(N+1)];
        for(size_t n=0; n < N; n++){
            c[N*m+n] /= elemf;
            unit[N*m+n] /= elemf;
        }
    }
	return unitm;
}

STATIC mp_obj_t ulab_ndarray_inv(mp_obj_t o_in) {
	ndarray_obj_t *o = MP_OBJ_TO_PTR(o_in);
	if(!MP_OBJ_IS_TYPE(o_in, &ulab_ndarray_type)) {
		mp_raise_TypeError("only ndarray objects can be inverted");
	}
	if(o->m != o->n) {
		mp_raise_ValueError("only square matrices can be inverted");
	}
	ndarray_obj_t *inverted = invert_matrix(o->data, o->m);
	return MP_OBJ_FROM_PTR(inverted);
}

MP_DEFINE_CONST_FUN_OBJ_1(ulab_ndarray_inv_obj, ulab_ndarray_inv);

STATIC mp_obj_t ulab_ndarray_reshape(mp_obj_t self_in, mp_obj_t shape) {
	ndarray_obj_t *self = MP_OBJ_TO_PTR(self_in);
	if(!MP_OBJ_IS_TYPE(shape, &mp_type_tuple) || (MP_OBJ_SMALL_INT_VALUE(mp_obj_len_maybe(shape)) != 2)) {
		mp_raise_ValueError("shape must be a 2-tuple");
	}
	
	mp_obj_iter_buf_t iter_buf;
	mp_obj_t item, iterable = mp_getiter(shape, &iter_buf);
	uint16_t m, n;
	item = mp_iternext(iterable);
	m = mp_obj_get_int(item);
	item = mp_iternext(iterable);
	n = mp_obj_get_int(item);
	if(m*n != self->m*self->n) {
		// TODO: the proper error message would be "cannot reshape array of size %d into shape (%d, %d)"
		mp_raise_ValueError("cannot reshape array");
	}
	self->m = m;
	self->n = n;
	return MP_OBJ_FROM_PTR(self);
}

MP_DEFINE_CONST_FUN_OBJ_2(ulab_ndarray_reshape_obj, ulab_ndarray_reshape);

/*
STATIC mp_obj_t ulab_ndarray_dot(mp_obj_t self_in, mp_obj_t other_in) {
	ndarray_obj_t *self = MP_OBJ_TO_PTR(self_in);
	// TODO: check for type of other_in
	ndarray_obj_t *other = MP_OBJ_TO_PTR(other_in);
	// both inputs are vectors
	if(((self->m == 1) || (self->n == 1)) && ((other->m == 1) || (other->n == 1))) {
		if(self->_size != other->_size) {
			mp_raise_ValueError("inputs must be of same length");
		} else {
			// what should happen with overflow?
			return mp_const_none;
		}
	}
	return mp_const_true;
}

MP_DEFINE_CONST_FUN_OBJ_2(ulab_ndarray_dot_obj, ulab_ndarray_dot);
*/

// Copied the range object from objrange.c The problem is that objrange.c 
// can't be included directly (there is no header file)
typedef struct _mp_obj_range_t {
    mp_obj_base_t base;
    // TODO make these values generic objects or something
    mp_int_t start;
    mp_int_t stop;
    mp_int_t step;
} mp_obj_range_t;

// getter method till I figure out how to implement slicing
STATIC mp_obj_t ulab_ndarray_get(mp_obj_t self_in, mp_obj_t rangem_in, mp_obj_t rangen_in) {
	// the positional arguments beyond self_in should be range objects, or integers
	ndarray_obj_t *self = MP_OBJ_TO_PTR(self_in);
	uint8_t *data = (uint8_t *)self->data->items;
	
	// This is not quite safe: here we should use MP_OBJ_IS_TYPE(rangem_in, &mp_type_int), 
	// but for some mysterious reason that simply doesn't work
	if(!MP_OBJ_IS_TYPE(rangem_in, &mp_type_range)) {
		size_t m = mp_obj_get_int(rangem_in);
		if((m < 0) || (m >= self->m)) {
			mp_raise_ValueError("indices are out of range");		
		}
		if(!MP_OBJ_IS_TYPE(rangen_in, &mp_type_range)) { // return a singe value
			// Note that this case can actually be gotten from a[m][n], too
			size_t n = mp_obj_get_int(rangen_in);
			if((n < 0) || (n >= self->n)) {
				mp_raise_ValueError("indices are out of range");
			}
			mp_obj_t value = mp_binary_get_val_array(self->data->typecode, self->data->items, m*self->n+n);
			return MP_OBJ_FROM_PTR(value);
			
		} else { // At least the second argument is a range object
			// TODO: get length of range
			// Unfortunately, range_len in objrange.c is static, so we have to re-implement the function here
			mp_obj_range_t *rangen = MP_OBJ_TO_PTR(rangen_in);
			if((rangen->start < 0) || (rangen->start > self->n) || (rangen->stop < 0) || (rangen->stop > self->n)) {
				mp_raise_ValueError("indices are out of range");				
			}
			mp_int_t idx = rangen->start;
			mp_int_t len_n = (rangen->stop - rangen->start) / abs(rangen->step);
			ndarray_obj_t *array = create_new_ndarray(1, len_n, self->data->typecode);
			uint8_t *c = (uint8_t *)array->data->items;
			int _sizeof = mp_binary_get_size('@', self->data->typecode, NULL);
			size_t pos, i=0;
			while(i < len_n) {
				pos = _sizeof*(m*self->n + idx);
				memcpy(&c[i*_sizeof], &data[pos], _sizeof);
				idx += rangen->step;
				i++;
			}
			return MP_OBJ_FROM_PTR(array);
		}
		// TODO: now do the same for m, and then for (m, n)
	}
	ndarray_obj_t *array = create_new_ndarray(self->m, self->n, self->data->typecode);
	return MP_OBJ_FROM_PTR(array);
}

MP_DEFINE_CONST_FUN_OBJ_3(ulab_ndarray_get_obj, ulab_ndarray_get);

/*
// setter method till I figure out how to implement slicing
STATIC mp_obj_t ulab_ndarray_set(const mp_obj_t self_in, const mp_obj_t *args) {
	// the positional arguments beyond self_in should be range objects, or integers
	ndarray_obj_t *self = MP_OBJ_TO_PTR(self_in);
	ndarray_obj_t *array = create_new_ndarray(self->m, self->n, self->data->typecode);
	return MP_OBJ_FROM_PTR(array);
}

STATIC MP_DEFINE_CONST_FUN_OBJ_3(ulab_ndarray_set_obj, ulab_ndarray_set);
*/

STATIC const mp_rom_map_elem_t ulab_ndarray_locals_dict_table[] = {
	{ MP_ROM_QSTR(MP_QSTR_transpose), MP_ROM_PTR(&ulab_ndarray_transpose_obj) },
	{ MP_ROM_QSTR(MP_QSTR_shape), MP_ROM_PTR(&ulab_ndarray_shape_obj) },
	{ MP_ROM_QSTR(MP_QSTR_size), MP_ROM_PTR(&ulab_ndarray_size_obj) },
	{ MP_ROM_QSTR(MP_QSTR_rawsize), MP_ROM_PTR(&ulab_ndarray_rawsize_obj) },
	{ MP_ROM_QSTR(MP_QSTR_reshape), MP_ROM_PTR(&ulab_ndarray_reshape_obj) },
	{ MP_ROM_QSTR(MP_QSTR_get), MP_ROM_PTR(&ulab_ndarray_get_obj) },	
//	{ MP_ROM_QSTR(MP_QSTR_dot), MP_ROM_PTR(&ulab_ndarray_dot_obj) },
	// class constants
    { MP_ROM_QSTR(MP_QSTR_uint8), MP_ROM_INT(NDARRAY_UINT8) },
	{ MP_ROM_QSTR(MP_QSTR_int8), MP_ROM_INT(NDARRAY_INT8) },
    { MP_ROM_QSTR(MP_QSTR_uint16), MP_ROM_INT(NDARRAY_UINT16) },
	{ MP_ROM_QSTR(MP_QSTR_int16), MP_ROM_INT(NDARRAY_INT16) },
	{ MP_ROM_QSTR(MP_QSTR_float), MP_ROM_INT(NDARRAY_FLOAT) },
};

STATIC MP_DEFINE_CONST_DICT(ulab_ndarray_locals_dict, ulab_ndarray_locals_dict_table);

/*
mp_obj_t ulab_ndarray_binary_op_helper(mp_binary_op_t op, mp_obj_t lhs, mp_obj_t rhs) {
	// TODO: support scalar operations
	if (MP_OBJ_IS_TYPE(rhs, &mp_type_int) || MP_OBJ_IS_TYPE(rhs, &mp_type_float)) {
		return MP_OBJ_NULL; // op not supported
	} else if(MP_OBJ_IS_TYPE(rhs, &ulab_ndarray_type)) {
		// At this point, the operands should have the same shape
		ndarray_obj_t *ol = MP_OBJ_TO_PTR(lhs);
		ndarray_obj_t *or = MP_OBJ_TO_PTR(rhs);
		ndarray_obj_t *array;
		if((ol->m != or->m) || (ol->n != or->n)) {
			mp_raise_ValueError("operands could not be broadcast together");
		}
		// do not convert types, if they are identical
		// do not convert either, if the left hand side is a float
		if((ol->data->typecode == or->data->typecode) || ol->data->typecode == NDARRAY_FLOAT) {
			array = ulab_ndarray_copy(ol);		
		} else {
			// the types are not equal, we have to do some conversion here
			if(or->data->typecode == NDARRAY_FLOAT) {
				array = ulab_ndarray_copy(ol);
			} else if((ol->data->typecode == NDARRAY_INT16) || (or->data->typecode == NDARRAY_INT16)) {
				array = create_new_ndarray(ol->m, ol->n, NDARRAY_INT16);
			} else if((ol->data->typecode == NDARRAY_UINT16) || (or->data->typecode == NDARRAY_UINT16)) {
				array = create_new_ndarray(ol->m, ol->n, NDARRAY_INT16);
			}
		}
		switch(op) {
			case MP_BINARY_OP_ADD:
//				for(size_t i=0; i < ol->data->len; i++) {
					
				//}
				return MP_OBJ_FROM_PTR(array);	
				break;
			default:
				break;
		}

	}
}

STATIC mp_obj_t ulab_ndarray_binary_op(mp_binary_op_t op, mp_obj_t lhs, mp_obj_t rhs) {
    ndarray_obj_t *ol = MP_OBJ_TO_PTR(lhs);
	ndarray_obj_t *or = MP_OBJ_TO_PTR(rhs);

	ndarray_obj_t *array = ulab_ndarray_copy(ol);    
	switch (op) {
		case MP_BINARY_OP_EQUAL:
			if(!MP_OBJ_IS_TYPE(rhs, &ulab_ndarray_type)) {
				return mp_const_false;
			} else {
				// Two arrays are equal, if their shape, typecode, and elements are equal
				if((ol->m != or->m) || (ol->n != or->n) || (ol->data->typecode != or->data->typecode)) {
					return mp_const_false;
				} else {
					size_t i = ol->bytes;
					uint8_t *l = (uint8_t *)ol->data->items;
					uint8_t *r = (uint8_t *)or->data->items;					
					while(i) { // At this point, we can simply compare the bytes, the types is irrelevant
						if(*l++ != *r++) {
							return mp_const_false;
						}
						i--;
					}
					return mp_const_true;
				}
			}
			break;
		case MP_BINARY_OP_ADD:
		case MP_BINARY_OP_MULTIPLY: 
			return MP_OBJ_FROM_PTR(array);	
			break;
			
		default:
            return MP_OBJ_NULL; // op not supported
	}
}
*/

/*
STATIC mp_obj_t ndarray_iterator_new(mp_obj_t array_in, mp_obj_iter_buf_t *iter_buf) {
    assert(sizeof(mp_obj_array_t) <= sizeof(mp_obj_iter_buf_t));
    ndarray_obj_t *array = MP_OBJ_TO_PTR(array_in);
    mp_obj_array_it_t *o = (mp_obj_array_it_t*)iter_buf;
    o->base.type = &array_it_type;
    o->array = array->data;
    o->offset = 0;
    o->cur = 0;
    return MP_OBJ_FROM_PTR(o);
}
*/

STATIC mp_obj_t ndarray_subscr(mp_obj_t self_in, mp_obj_t index_in, mp_obj_t value) {
    if (value == MP_OBJ_NULL) {
        return MP_OBJ_NULL;
    } 
    ndarray_obj_t *self = MP_OBJ_TO_PTR(self_in);
	if(MP_OBJ_IS_TYPE(index_in, &mp_type_tuple)) {
		printf("tuple!!!");
	}
	if (MP_OBJ_IS_TYPE(index_in, &mp_type_slice)) {
		printf("slice: ");
		mp_bound_slice_t slice;
		if (!mp_seq_get_fast_slice_indexes(self->data->len, index_in, &slice)) {
			mp_raise_NotImplementedError("only slices with step=1 (aka None) are supported");
		}
		ndarray_obj_t *array = ulab_ndarray_copy(self_in);
		printf("%lu, %lu, %ld\n", slice.start, slice.stop, slice.step);
		return MP_OBJ_FROM_PTR(array);
	} else {
		// Now that we know that we haven't a slice, let's attempt to extract the index
		size_t idx = mp_get_index(self->base.type, (self->m == 1) ? self->n : self->m, index_in, false);
		
		// TODO: treat the setter here
//		mp_obj_list_t *self = MP_OBJ_TO_PTR(self_in);
		//self->items[i] = value;

		if(self->m > 1) {
			ndarray_obj_t *row = create_new_ndarray(1, self->n, self->data->typecode);
			row->bytes = self->n * mp_binary_get_size('@', self->data->typecode, NULL);
			uint8_t *c = (uint8_t *)self->data->items;
			memcpy(row->data->items, &c[idx*self->n], row->bytes);
			return MP_OBJ_FROM_PTR(row);
		} 
	}
	return mp_const_none;
}

STATIC mp_obj_t ulab_ndarray_unary_op(mp_unary_op_t op, mp_obj_t self_in) {
	ndarray_obj_t *self = MP_OBJ_TO_PTR(self_in);
    switch (op) {
        case MP_UNARY_OP_BOOL: return mp_obj_new_bool(self->data->len != 0);
        case MP_UNARY_OP_LEN: return MP_OBJ_NEW_SMALL_INT(self->data->len);
        default: return MP_OBJ_NULL; // op not supported
    }
}

const mp_obj_type_t ulab_ndarray_type = {
	{ &mp_type_type },
	.name = MP_QSTR_ndarray,
	.print = ulab_ndarray_print,
	.make_new = ulab_ndarray_make_new,
	.unary_op = ulab_ndarray_unary_op,
	.subscr = ndarray_subscr,
//	.getiter = ndarray_iterator_new,
//	.binary_op = ulab_ndarray_binary_op,
	.locals_dict = (mp_obj_dict_t*)&ulab_ndarray_locals_dict,
};

STATIC mp_obj_t kw_test(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    static const mp_arg_t allowed_args[] = {
		{ MP_QSTR_input, MP_ARG_OBJ, {.u_rom_obj = MP_ROM_PTR(&mp_const_none_obj)} }, 
        { MP_QSTR_base,     MP_ARG_INT, {.u_int = 12} },
        { MP_QSTR_mode,     MP_ARG_INT, {.u_int = 555} },
        { MP_QSTR_addr,     MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 33} },
        { MP_QSTR_dtype,     MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 33} },        
    };

    // parse args
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);
    if(MP_OBJ_IS_TYPE(args[0].u_rom_obj, &mp_type_tuple)) {
		printf("tuple!!!");
	}
    printf("base: %lu\n\r",  args[1].u_int);
    printf("mode: %lu\n\r",  args[2].u_int);
    printf("address: %lu\n\r",  args[3].u_int);    
    printf("dtypes: %lu\n\r",  args[4].u_int);
    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_KW(kw_test_obj, 1, kw_test);


STATIC mp_obj_t ulab_sum(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_array,    MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_rom_obj = MP_ROM_PTR(&mp_const_none_obj)} },
        { MP_QSTR_axis,     MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 0} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);
	mp_obj_t *array = MP_OBJ_TO_PTR(args[0].u_obj);
    if(MP_OBJ_IS_TYPE(array, &mp_type_NoneType)) {
		mp_raise_ValueError("missing first argument");
	}
    // TODO: it would be great, if we could pass a range as the first argument
	if(MP_OBJ_IS_TYPE(array, &mp_type_tuple) || MP_OBJ_IS_TYPE(array, &mp_type_list)) {
		mp_float_t _sum = 0.0;
		mp_obj_iter_buf_t iter_buf;
		mp_obj_t item, iterable = mp_getiter(array, &iter_buf);
		while ((item = mp_iternext(iterable)) != MP_OBJ_STOP_ITERATION) {
			_sum += mp_obj_get_float(item);
		}
		return mp_obj_new_float(_sum);		
	} else if(MP_OBJ_IS_TYPE(array, &ulab_ndarray_type)) {
		ndarray_obj_t *array_in = MP_OBJ_TO_PTR(array);
		size_t m, n;
		if(args[1].u_int == 0) {
			m = 1;
			n = array_in->n;
		} else if(args[1].u_int == 1) {
			m = array_in->m;
			n = 1;
		} else {
			mp_raise_ValueError("axis must be 0, or 1");
		}
		ndarray_obj_t *array_out = create_new_ndarray(m, n, NDARRAY_FLOAT);
		float *c = (float *)array_out->data->items;
		mp_obj_t elem;
		// I believe, these loops could be combined
		if(m == 1) {
			// contract along the first axis
			for(size_t i=0; i < array_in->n; i++) {
				for(size_t j=0; j < array_in->m; j++) {
					elem = mp_binary_get_val_array(array_in->data->typecode, array_in->data->items, j*array_in->n+i);
					c[i] += (float)mp_obj_get_float(elem);
				}
			}
		} else if(n == 1) {
			// contract along the second axis
			for(size_t i=0; i < array_in->m; i++) {
				for(size_t j=0; j < array_in->n; j++) {
					elem = mp_binary_get_val_array(array_in->data->typecode, array_in->data->items, i*array_in->n+j);
					c[i] += (float)mp_obj_get_float(elem);
				}
			}			
		}
		return MP_OBJ_FROM_PTR(array_out);
	}
	mp_raise_TypeError("wrong input type");
}

STATIC MP_DEFINE_CONST_FUN_OBJ_KW(ulab_sum_obj, 1, ulab_sum);

STATIC const mp_map_elem_t ulab_globals_table[] = {
	{ MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_ulab) },
	{ MP_OBJ_NEW_QSTR(MP_QSTR_kw_test), (mp_obj_t)&kw_test_obj },	
	{ MP_OBJ_NEW_QSTR(MP_QSTR_ndarray), (mp_obj_t)&ulab_ndarray_type },	
	{ MP_OBJ_NEW_QSTR(MP_QSTR_inv), (mp_obj_t)&ulab_ndarray_inv_obj },
	{ MP_OBJ_NEW_QSTR(MP_QSTR_sum), (mp_obj_t)&ulab_sum_obj },	
};

STATIC MP_DEFINE_CONST_DICT (
	mp_module_ulab_globals,
	ulab_globals_table
);

const mp_obj_module_t ulab_user_cmodule = {
	.base = { &mp_type_module },
	.globals = (mp_obj_dict_t*)&mp_module_ulab_globals,
};


MP_REGISTER_MODULE(MP_QSTR_ulab, ulab_user_cmodule, MODULE_ULAB_ENABLED);
