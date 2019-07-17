#include <stdio.h>
#include "py/runtime.h"
#include "py/obj.h"
#include "py/binary.h"

typedef struct _specialclass_myclass_obj_t {
	mp_obj_base_t base;
	int16_t a;
	int16_t b;
} specialclass_myclass_obj_t;

const mp_obj_type_t specialclass_myclass_type;

STATIC void myclass_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
	(void)kind;
	specialclass_myclass_obj_t *self = MP_OBJ_TO_PTR(self_in);
	printf("myclass(%d, %d)", self->a, self->b);
}

mp_obj_t create_new_myclass(uint16_t a, uint16_t b) {
	specialclass_myclass_obj_t *out = m_new_obj(specialclass_myclass_obj_t);
	out->base.type = &specialclass_myclass_type;
	out->a = a;
	out->b = b;
	return MP_OBJ_FROM_PTR(out);
}

STATIC mp_obj_t myclass_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
	mp_arg_check_num(n_args, n_kw, 2, 2, true);
	return create_new_myclass(mp_obj_get_int(args[0]), mp_obj_get_int(args[1]));
}

STATIC const mp_rom_map_elem_t myclass_locals_dict_table[] = {
};

STATIC MP_DEFINE_CONST_DICT(myclass_locals_dict, myclass_locals_dict_table);

STATIC mp_obj_t specialclass_unary_op(mp_unary_op_t op, mp_obj_t self_in) {
	specialclass_myclass_obj_t *self = MP_OBJ_TO_PTR(self_in);
    switch (op) {
        case MP_UNARY_OP_BOOL: return mp_obj_new_bool((self->a > 0) && (self->b > 0));
        case MP_UNARY_OP_LEN: return mp_obj_new_int(2);
        default: return MP_OBJ_NULL; // op not supported
    }
}

STATIC mp_obj_t specialclass_binary_op(mp_binary_op_t op, mp_obj_t lhs, mp_obj_t rhs) {
    specialclass_myclass_obj_t *left_hand_side = MP_OBJ_TO_PTR(lhs);
	specialclass_myclass_obj_t *right_hand_side = MP_OBJ_TO_PTR(rhs);
	switch (op) {
		case MP_BINARY_OP_EQUAL:
			return mp_obj_new_bool((left_hand_side->a == right_hand_side->a) && (left_hand_side->b == right_hand_side->b));
		case MP_BINARY_OP_ADD:
			return create_new_myclass(left_hand_side->a + right_hand_side->a, left_hand_side->b + right_hand_side->b);
		case MP_BINARY_OP_MULTIPLY:
			return create_new_myclass(left_hand_side->a * right_hand_side->a, left_hand_side->b * right_hand_side->b);
		default:
			return MP_OBJ_NULL; // op not supported
	}
}

const mp_obj_type_t specialclass_myclass_type = {
	{ &mp_type_type },
	.name = MP_QSTR_specialclass,
	.print = myclass_print,
	.make_new = myclass_make_new,
	.unary_op = specialclass_unary_op, 
	.binary_op = specialclass_binary_op,
	.locals_dict = (mp_obj_dict_t*)&myclass_locals_dict,
};

STATIC const mp_map_elem_t specialclass_globals_table[] = {
	{ MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_specialclass) },
	{ MP_OBJ_NEW_QSTR(MP_QSTR_myclass), (mp_obj_t)&specialclass_myclass_type },	
};

STATIC MP_DEFINE_CONST_DICT (
	mp_module_specialclass_globals,
	specialclass_globals_table
);

const mp_obj_module_t specialclass_user_cmodule = {	
	.base = { &mp_type_module },
	.globals = (mp_obj_dict_t*)&mp_module_specialclass_globals,
};

MP_REGISTER_MODULE(MP_QSTR_specialclass, specialclass_user_cmodule, MODULE_SPECIALCLASS_ENABLED);
