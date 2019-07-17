#include "py/obj.h"
#include "py/runtime.h"

#if 0
typedef struct _mp_obj_tuple_it_t {
    mp_obj_base_t base;
    mp_fun_1_t iternext;
    mp_obj_tuple_t *tuple;
    size_t cur;
} mp_obj_tuple_it_t;

const mp_obj_type_t mp_type_tuple = {
    { &mp_type_type },
    .name = MP_QSTR_tuple,
    .print = mp_obj_tuple_print,
    .make_new = mp_obj_tuple_make_new,
    .unary_op = mp_obj_tuple_unary_op,
    .binary_op = mp_obj_tuple_binary_op,
    .subscr = mp_obj_tuple_subscr,
    .getiter = mp_obj_tuple_getiter,
    .locals_dict = (mp_obj_dict_t*)&tuple_locals_dict,
};

STATIC mp_obj_t tuple_it_iternext(mp_obj_t self_in) {
    mp_obj_tuple_it_t *self = MP_OBJ_TO_PTR(self_in);
    if (self->cur < self->tuple->len) {
        mp_obj_t o_out = self->tuple->items[self->cur];
        self->cur += 1;
        return o_out;
    } else {
        return MP_OBJ_STOP_ITERATION;
    }
}

mp_obj_t mp_obj_tuple_getiter(mp_obj_t o_in, mp_obj_iter_buf_t *iter_buf) {
    assert(sizeof(mp_obj_tuple_it_t) <= sizeof(mp_obj_iter_buf_t));
    mp_obj_tuple_it_t *o = (mp_obj_tuple_it_t*)iter_buf;
    o->base.type = &mp_type_polymorph_iter;
    o->iternext = tuple_it_iternext;
    o->tuple = MP_OBJ_TO_PTR(o_in);
    o->cur = 0;
    return MP_OBJ_FROM_PTR(o);
}


STATIC const mp_rom_map_elem_t iterables_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_iterables) },
    { MP_ROM_QSTR(MP_QSTR_sumsq), MP_ROM_PTR(&iterables_sumsq_obj) },
};
STATIC MP_DEFINE_CONST_DICT(iterables_module_globals, iterables_module_globals_table);

const mp_obj_module_t iterables_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&iterables_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_iterables, iterables_user_cmodule, MODULE_ITERABLES_ENABLED);
#endif
