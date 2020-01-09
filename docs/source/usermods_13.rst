Working with larger modules
===========================

Once you add more and more functionality and functions to your module,
it will become unmanageably, and it might make more sense to split the
module into smaller components. We are going to hack our very first
module, ``simplefunction``, and factor out the function in it.

Since we will want to refer to our functions in the module definition,
we have to declare them in a header file. Let us call this file
``helper.h``. The functions declared therein operate on ``micropython``
types, so do not forget to include ``py/obj.h``, and possibly
``py/runtime.h``!

https://github.com/v923z/micropython-usermod/tree/master/snippets/largemodule/helper.h

.. code:: cpp
        
    
    #include "py/obj.h"
    #include "py/runtime.h"
    
    mp_obj_t largemodule_add_ints(mp_obj_t , mp_obj_t );
    mp_obj_t largemodule_subtract_ints(mp_obj_t , mp_obj_t );

Next, in ``helper.c``, we have to implement the functions. ``helper.c``
should also contain the declarations, i.e., ``header.h`` has to be
included.

https://github.com/v923z/micropython-usermod/tree/master/snippets/largemodule/helper.c

.. code:: cpp
        
    
    #include "helper.h"
    
    mp_obj_t largemodule_add_ints(mp_obj_t a_obj, mp_obj_t b_obj) {
        int a = mp_obj_get_int(a_obj);
        int b = mp_obj_get_int(b_obj);
        return mp_obj_new_int(a + b);
    }
    
    mp_obj_t largemodule_subtract_ints(mp_obj_t a_obj, mp_obj_t b_obj) {
        int a = mp_obj_get_int(a_obj);
        int b = mp_obj_get_int(b_obj);
        return mp_obj_new_int(a - b);
    }

Finally, in the module implementation, we include ``helper.h``, and
create the function objects with ``MP_DEFINE_CONST_FUN_OBJ_2``, and its
relatives. The rest of the code is equivalent to ``simplefunction.c``,
with the only exception of the module name.

https://github.com/v923z/micropython-usermod/tree/master/snippets/largemodule/largemodule.c

.. code:: cpp
        
    
    #include "py/obj.h"
    #include "py/runtime.h"
    #include "helper.h"
    
    
    STATIC MP_DEFINE_CONST_FUN_OBJ_2(largemodule_add_ints_obj, largemodule_add_ints);
    STATIC MP_DEFINE_CONST_FUN_OBJ_2(largemodule_subtract_ints_obj, largemodule_subtract_ints);
    
    STATIC const mp_rom_map_elem_t largemodule_module_globals_table[] = {
        { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_largemodule) },
        { MP_ROM_QSTR(MP_QSTR_add_ints), MP_ROM_PTR(&largemodule_add_ints_obj) },
        { MP_ROM_QSTR(MP_QSTR_subtract_ints), MP_ROM_PTR(&largemodule_subtract_ints_obj) },    
    };
    STATIC MP_DEFINE_CONST_DICT(largemodule_module_globals, largemodule_module_globals_table);
    
    const mp_obj_module_t largemodule_user_cmodule = {
        .base = { &mp_type_module },
        .globals = (mp_obj_dict_t*)&largemodule_module_globals,
    };
    
    MP_REGISTER_MODULE(MP_QSTR_largemodule, largemodule_user_cmodule, MODULE_LARGEMODULE_ENABLED);

Now, since we have multiple files in our module, we have to change the
``makefile`` accordingly, and before linking, we have to compile both
``helper.c``, and ``largemodule.c``, thus, we add
``$(USERMODULES_DIR)/helper.c``, *and*
``$(USERMODULES_DIR)/largemodule.c`` to ``SRC_USERMOD``.

https://github.com/v923z/micropython-usermod/tree/master/snippets/largemodule/micropython.mk

.. code:: make
        
    
    USERMODULES_DIR := $(USERMOD_DIR)
    
    # Add all C files to SRC_USERMOD.
    SRC_USERMOD += $(USERMODULES_DIR)/helper.c
    SRC_USERMOD += $(USERMODULES_DIR)/largemodule.c
    
    CFLAGS_USERMOD += -I$(USERMODULES_DIR)
.. code:: bash

    !make clean
    !make USER_C_MODULES=../../../usermod/snippets CFLAGS_EXTRA=-DMODULE_LARGEMODULE_ENABLED=1 all
.. code ::
        
    %%micropython
    
    import largemodule
    
    print(largemodule.add_ints(1, 2))
    print(largemodule.subtract_ints(1, 2))
.. parsed-literal::

    3
    -1
    
    
