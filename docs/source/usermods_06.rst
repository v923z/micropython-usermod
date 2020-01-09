Module constants
================

We have just seen, how we add a function to python. But functions are
not the only objects that can be attached to a module, and of particular
interest are constants. If for nothing else, you can give your module a
version number. So, let us see, how that can be achieved.

Contstants, if they are true to their name, won’t change at run time,
hence, they can be stored in ROM. We have already seen this, because the
globals table of our very first module kicked out with the line

.. code:: c

       { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_simplefunction) }

Here, the ``MP_QSTR_simplefunction`` was a constant, namely, the string
``simplefunction`` stored in ROM. This is why it is wrapped by the macro
``MP_ROM_QSTR``. There are two other ``MP_ROM`` macros defined in
``obj.h``, namely, ``MP_ROM_INT``, and ``MP_ROM_PTR``.

Integer constants
-----------------

It should not be a big surprise that the ``MP_ROM_INT`` macro generates
ROM objects from integers. Thus, the following code will give you the
magic constant 42:

.. code:: c

   #define MAGIC_CONSTANT 42
   ...

       { MP_ROM_QSTR(MP_QSTR_magic), MP_ROM_INT(MAGIC_CONSTANT) },
   ...

Strings
-------

Now, in ``MP_QSTR_simplefunction``, ``simplefunction`` is a well-behaved
string, containing no special characters. But are we doomed, if we do
want to print out a version string, which would probably look like
``1.2.3``, or something similar? And should we give up all hope, if our
string contains an underscore? The answer to these questions is no, and
no! This is, where the ``MP_ROM_PTR`` macro comes to the rescue.

In general, ``MP_ROM_PTR`` will take the address of an object, and
convert it to a 32-bit unsigned integer. At run time, ``micropython``
works with this integer, if it has to access the constant. And this is
exactly what happens in the second line of the globals table of
``simplefunction``:

.. code:: c

   { MP_ROM_QSTR(MP_QSTR_add_ints), MP_ROM_PTR(&simplefunction_add_ints_obj) },

we associated the string ``add_ints`` (incidentally, also stored in ROM)
with the 32-bit unsigned integer generated from the address of
``simplefunction_add_ints_obj``. So, the bottom line is, if we can
somehow get hold of the address of an object, we can wrap it with
``MP_ROM_PTR``, and we are done.

Thus, if we want to define a string constant, we have to convert it to
something that has an address. The ``MP_DEFINE_STR_OBJ`` of ``objstr.h``
does exactly that:

.. code:: c

   STATIC const MP_DEFINE_STR_OBJ(version_string_obj, "1.2.3");

takes ``1.2.3`` as a string, and turns is into a micropython object of
type ``mp_obj_str_t``. After this, ``&version_string_obj`` can be passed
to the ``MP_ROM_PTR`` macro.

Tuples
------

We don’t have to be satisfied with integers and strings, we can
definitely go further. There is a python type, the ``tuple``, that is,
by definition, constant (not mutable), and for this reason, we can
easily define a tuple type module constant. ``objtuple.h`` defines
``mp_rom_obj_tuple_t`` for this purpose. This is a structure with three
members, and looks like this:

.. code:: c

   const mp_rom_obj_tuple_t version_tuple_obj = {
       {&mp_type_tuple},
       2,
       {
           MP_ROM_INT(1),
           MP_ROM_PTR(&version_string_obj),
       },
   };

The first member defines the base type of the object, the second is the
number of elements of the tuple that we want to define, and the third is
itself a structure, listing the tuple elements. The key point here is
that we can apply the address-of operator to ``version_tuple_obj``, and
pass it to the ``MP_ROM_PTR`` macro.

https://github.com/v923z/micropython-usermod/tree/master/snippets/constants/constants.c

.. code:: cpp
        
    
    #include "py/obj.h"
    #include "py/runtime.h"
    #include "py/objstr.h"
    #include "py/objtuple.h"
    
    #define MAGIC_CONSTANT 42
    STATIC const MP_DEFINE_STR_OBJ(version_string_obj, "1.2.3");
    
    const mp_rom_obj_tuple_t version_tuple_obj = {
        {&mp_type_tuple},
        2,
        {
            MP_ROM_INT(1),
            MP_ROM_PTR(&version_string_obj),
        },
    };
    
    STATIC const mp_rom_map_elem_t constants_module_globals_table[] = {
        { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_constants) },
        { MP_ROM_QSTR(MP_QSTR___version__), MP_ROM_PTR(&version_string_obj) },
        { MP_ROM_QSTR(MP_QSTR_magic), MP_ROM_INT(MAGIC_CONSTANT) },
        { MP_ROM_QSTR(MP_QSTR_version_tuple), MP_ROM_PTR(&version_tuple_obj) },    
    };
    STATIC MP_DEFINE_CONST_DICT(constants_module_globals, constants_module_globals_table);
    
    const mp_obj_module_t constants_user_cmodule = {
        .base = { &mp_type_module },
        .globals = (mp_obj_dict_t*)&constants_module_globals,
    };
    
    MP_REGISTER_MODULE(MP_QSTR_constants, constants_user_cmodule, MODULE_CONSTANTS_ENABLED);

https://github.com/v923z/micropython-usermod/tree/master/snippets/constants/micropython.mk

.. code:: make
        
    
    USERMODULES_DIR := $(USERMOD_DIR)
    
    # Add all C files to SRC_USERMOD.
    SRC_USERMOD += $(USERMODULES_DIR)/constants.c
    
    CFLAGS_USERMOD += -I$(USERMODULES_DIR)
.. code:: bash

    !make clean
    !make USER_C_MODULES=../../../usermod/snippets CFLAGS_EXTRA=-DMODULE_CONSTANTS_ENABLED=1 all
One comment before trying out what we have just implemented: the module
is definitely pathological. If all you need is a set of constants
organised in some way, then you should write it in python. There is
nothing to be gained by working in C, while python is much more
flexible.

.. code ::
        
    %%micropython -unix 1
    
    import constants
    
    print(constants.magic)
    print(constants.__version__)
    print(constants.version_tuple)
.. parsed-literal::

    42
    1.2.3
    (1, '1.2.3')
    
    
