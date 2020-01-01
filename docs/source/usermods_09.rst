Creating new types
==================

Sometimes you might need something beyond the standard python data
types, and you have to define your own. At first, the task seems
daunting, but types are really nothing but a C structure with a couple
of special fields. The steps required are very similar to those for
classes. Take the following type definition, which could be regarded as
the Cartesian components of a vector in three-dimensional space:

.. code:: c

   typedef struct _vector_obj_t {
       mp_obj_base_t base;
       float x, y, z;
   } vector_obj_t;

Now, in order to see, how we can work with this structure, we are going
to define a new type that simply stores the three values. The module
will also have a method called ``length``, returning the absolute value
of the vector. Also note that here we check the type of the argument,
and bail out, if it is not a vector. The beauty of all this is that once
the type is defined, the available micropython methods just work. Can
you still recall the

.. code:: c

   MP_OBJ_IS_TYPE(myobject, &my_type)

macro in Section `Type checking? <#Type-checking>`__ I thought so.

We have our vector structure at the C level. It has four members: an
``mp_obj_base_t``, and three floats called ``x``, ``y``, and ``z``. But
this is still not usable in the python interpreter. We have to somehow
tell the interpreter, what it is supposed to do with this new type, and
how a variable of this type is to be presented to the user. This is,
where the structure

.. code:: c

   const mp_obj_type_t vector_type = {
       { &mp_type_type },
       .name = MP_QSTR_vector,
       .print = vector_print,
       .make_new = vector_make_new,
   };

takes centre stage. Does this look familiar? This structure contains the
new type’s name (a string, ``vector``), how it presents itself to users
(a function, ``vector_print``), and how a new instance is to be created
(a function, ``vector_make_new``). These latter two we have to implement
ourselves.

In ``vector_print`` we have three arguments, namely
``const mp_print_t *print``, which is a helper that we don’t call,
``mp_obj_t self_in`` which is a reference to the vector itself, and
``mp_print_kind_t kind``, which we can graciously ignore, because we are
not going to use it anyway.

Having seen the bits and pieces, we should build some new firmware.

https://github.com/v923z/micropython-usermod/tree/master/snippets/vector/vector.c

.. code:: cpp
        
    
    #include <math.h>
    #include <stdio.h>
    #include "py/obj.h"
    #include "py/runtime.h"
    
    const mp_obj_type_t vector_type;
    
    typedef struct _vector_obj_t {
        mp_obj_base_t base;
        float x, y, z;
    } vector_obj_t;
    
    STATIC mp_obj_t vector_length(mp_obj_t o_in) {
        if(!mp_obj_is_type(o_in, &vector_type)) {
            mp_raise_TypeError("argument is not a vector");
        }
        vector_obj_t *vector = MP_OBJ_TO_PTR(o_in);
        return mp_obj_new_float(sqrtf(vector->x*vector->x + vector->y*vector->y + vector->z*vector->z));
    }
    
    STATIC MP_DEFINE_CONST_FUN_OBJ_1(vector_length_obj, vector_length);
    
    STATIC void vector_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
        (void)kind;
        vector_obj_t *self = MP_OBJ_TO_PTR(self_in);
        printf("vector(%f, %f, %f)\n", (double)self->x, (double)self->y, (double)self->z);
    }
    
    STATIC mp_obj_t vector_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
        mp_arg_check_num(n_args, n_kw, 3, 3, true);
        
        vector_obj_t *vector = m_new_obj(vector_obj_t);
        vector->base.type = &vector_type;
        vector->x = mp_obj_get_float(args[0]);
        vector->y = mp_obj_get_float(args[1]);
        vector->z = mp_obj_get_float(args[2]);
        return MP_OBJ_FROM_PTR(vector);
    }
    
    const mp_obj_type_t vector_type = {
        { &mp_type_type },
        .name = MP_QSTR_vector,
        .print = vector_print,
        .make_new = vector_make_new,
    };
    
    STATIC const mp_rom_map_elem_t vector_module_globals_table[] = {
        { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_vector) },
        { MP_OBJ_NEW_QSTR(MP_QSTR_vector), (mp_obj_t)&vector_type },
        { MP_ROM_QSTR(MP_QSTR_length), MP_ROM_PTR(&vector_length_obj) },
    };
    STATIC MP_DEFINE_CONST_DICT(vector_module_globals, vector_module_globals_table);
    
    const mp_obj_module_t vector_user_cmodule = {
        .base = { &mp_type_module },
        .globals = (mp_obj_dict_t*)&vector_module_globals,
    };
    
    MP_REGISTER_MODULE(MP_QSTR_vector, vector_user_cmodule, MODULE_VECTOR_ENABLED);

https://github.com/v923z/micropython-usermod/tree/master/snippets/vector/micropython.mk

.. code:: make
        
    
    USERMODULES_DIR := $(USERMOD_DIR)
    
    # Add all C files to SRC_USERMOD.
    SRC_USERMOD += $(USERMODULES_DIR)/vector.c
    
    # We can add our module folder to include paths if needed
    # This is not actually needed in this example.
    CFLAGS_USERMOD += -I$(USERMODULES_DIR)
.. code:: bash

    !make clean
    !make USER_C_MODULES=../../../usermod/snippets/ CFLAGS_EXTRA=-DMODULE_VECTOR_ENABLED=1 all
.. code ::
        
    %%micropython
    
    import vector
    
    a = vector.vector(1, 20, 30)
    print(a)
    print(vector.length(a))
.. parsed-literal::

    vector(1.000000, 20.000000, 30.000000)
    
    36.06937789916993
    
    

Just to convince ourselves, when calculated in python proper, the length
of the vector is

.. code ::
        
    import math
    
    print(math.sqrt(1**2 + 20**2 + 30**2))
.. parsed-literal::

    36.069377593742864

Close enough.
