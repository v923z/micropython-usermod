
Developing your first module
============================

Having seen, what the python objects look like to the interpreter, we
can start with our explorations in earnest. We begin by adding a simple
module to micropython. The module will have a single function that takes
two numbers, and adds them. I know that this is the most exciting thing
since sliced bread, and you have always wondered, why there isn’t a
built-in python function for such an fascinating task. Well, wonder no
more! From this moment, *your* micropython will have one.

First I show the file in its entirety (20 something lines all in all),
and then discuss the parts.

https://github.com/v923z/micropython-usermod/tree/master/snippets/simplefunction/simplefunction.c

.. code::
        

	#include "py/obj.h"
	#include "py/runtime.h"
	
	STATIC mp_obj_t simplefunction_add_ints(mp_obj_t a_obj, mp_obj_t b_obj) {
	    int a = mp_obj_get_int(a_obj);
	    int b = mp_obj_get_int(b_obj);
	    return mp_obj_new_int(a + b);
	}
	
	STATIC MP_DEFINE_CONST_FUN_OBJ_2(simplefunction_add_ints_obj, simplefunction_add_ints);
	
	STATIC const mp_rom_map_elem_t simplefunction_module_globals_table[] = {
	    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_simplefunction) },
	    { MP_ROM_QSTR(MP_QSTR_add_ints), MP_ROM_PTR(&simplefunction_add_ints_obj) },
	};
	STATIC MP_DEFINE_CONST_DICT(simplefunction_module_globals, simplefunction_module_globals_table);
	
	const mp_obj_module_t simplefunction_user_cmodule = {
	    .base = { &mp_type_module },
	    .globals = (mp_obj_dict_t*)&simplefunction_module_globals,
	};
	
	MP_REGISTER_MODULE(MP_QSTR_simplefunction, simplefunction_user_cmodule, MODULE_SIMPLEFUNCTION_ENABLED);

.. parsed-literal::

    written 1090 bytes to /simplefunction/simplefunction.c


Header files
------------

A module will not be too useful without at least two includes:
``py/obj.h``, where all the relevant constants and macros are defined,
and ``py/runtime.h``, which contains the declaration of the interpreter.
Many a time you will also need ``py/builtin.h``, where the python
built-in functions and modules are declared.

Defining user functions
-----------------------

After including the necessary headers, we define the function that is
going to do the heavy lifting. By passing variables of ``mp_obj_t``
type, we make sure that the function will be able to accept values from
the python console. If you happen to have an internal helper function in
your module that is not exposed in python, you can pass whatever type
you need. Similarly, by returning an object of ``mp_obj_t`` type, we
make the results visible to the interpreter, i.e., we can assign the
value returned to variables.

The downside of passing ``mp_obj_t``\ s around is that you cannot simply
assign them to usual C variables, i.e., when you want to operate on
them, you have to extract the values first. This is why we have to
invoke the ``mp_obj_get_int()`` function, and conversely, before
returning the results, we have to do a type conversion to ``mp_obj_t``
by calling ``mp_obj_new_int()``. These are the decoding/encoding steps
that we discussed above.

Referring to user functions
~~~~~~~~~~~~~~~~~~~~~~~~~~~

We have now a function that should be sort of OK (there is no error
checking whatsoever, so you are at the mercy of the firmware, when,
e.g., you try to pass a float to the function), but the python
interpreter still cannot work with. For that, we have to turn our
function into a function object. This is what happens in the line

.. code:: c

   STATIC MP_DEFINE_CONST_FUN_OBJ_2(simplefunction_add_ints_obj, simplefunction_add_ints);

The first argument of the macro is the name of the function object to
which our actual function, the last argument, will be bound. Now, these
``MP_DEFINE_CONST_FUN_OBJ_*`` macros, defined in the header file
``py/obj.h`` (one more reason not to forget about ``py/obj.h``), come in
seven flavours, depending on what kind of, and how many arguments the
function is supposed to take. In the example above, our function is
meant to take two arguments, hence the 2 at the end of the macro name.
Functions with 0 to 4 arguments can be bound in this way.

But what, if you want a function with more than four arguments, as is
the case many a time in python? Under such circumstances, one can make
use of the

.. code:: c

   MP_DEFINE_CONST_FUN_OBJ_VAR(obj_name, n_args_min, fun_name)

macro, where the second argument, an integer, gives the minimum number
of arguments. The number of arguments can be bound from above by
wrapping the function with

.. code:: c

   MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(obj_name, n_args_min, n_args_max, fun_name) 

Later we will see, how we can define functions that can also take
keyword arguments.

At this point, we are more or less done with the C implementation of our
function, but we still have to expose it. This we do by adding a table,
an array of key/value pairs to the globals of our module, and bind the
table to the ``_module_globals`` variable by applying the
``MP_DEFINE_CONST_DICT`` macro. This table should have at least one
entry, the name of the module, which is going to be stored in the string
``MP_QSTR___name__``.

These ``MP_QSRT_`` items are the C representation of the python strings
that come at the end of them. So, ``MP_QSRT_foo_bar`` in C will be
turned into a name, ``foo_bar``, in python. ``foo_bar`` can be a
constant, a function, a class, a type, etc., and depending on what is
associated with it, different things will happen on the console, when
``foo_bar`` is invoked. But the crucial point is that, if you want
``foo_bar`` to have any meaning in python, then somewhere in your C
code, you have to define ``MP_QSRT_foo_bar``.

The second key-value pair of the table is the pointer to the function
that we have just implemented, and the name that we want to call the
functions in python itself. So, in the example below, our
``simplefunction_add_ints`` function will be invoked, when we call
``add_ints`` in the console.

.. code:: c

   STATIC const mp_rom_map_elem_t simplefunction_module_globals_table[] = {
       { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_simplefunction) },
       { MP_ROM_QSTR(MP_QSTR_add_ints), MP_ROM_PTR(&simplefunction_add_ints_obj) },
   };
   STATIC MP_DEFINE_CONST_DICT(simplefunction_module_globals, simplefunction_module_globals_table);

This three-step pattern is common to all function implementations, so I
repeat it here:

1. implement the function
2. then turn it into a function object (i.e., call the relevant form of
   MP_DEFINE_CONST_FUN_OBJ_*)
3. and finally, register the function in the name space of the module
   (i.e., add it to the module’s globals table, and turn the table into
   a dictionary by applying MP_DEFINE_CONST_DICT)

It doesn’t matter, whether our function takes positional arguments, or
keyword argument, or both, these are the required steps.

Having defined the function object, we have finally to register the
module with

.. code:: c

   MP_REGISTER_MODULE(MP_QSTR_simplefunction, simplefunction_user_cmodule, MODULE_SIMPLEFUNCTION_ENABLED);

This last line is particularly useful, because by setting the
``MODULE_SIMPLEFUNCTION_ENABLED`` variable in ``mpconfigport.h``, you
can selectively exclude modules from the linking, i.e., if in
``mpconfigport.h``, which should be in the root directory of the port
you want to compile for,

.. code:: c

   #define MODULE_SIMPLEFUNCTION_ENABLED (1)

then ``simplefunction`` will be included in the firmware, while with

.. code:: c

   #define MODULE_SIMPLEFUNCTION_ENABLED (0)

the module will be dropped, even though the source is in your modules
folder. (N.B.: the module will still be compiled, but not linked.)

Compiling our module
--------------------

The implementation is done, and we would certainly like to see some
results. First we generate a makefile, which will be inserted in the
module’s own directory, ``simplefunction/``.

.. code::

    %%makefile /simplefunction/simplefunction.c
    
    USERMODULES_DIR := $(USERMOD_DIR)
    
    # Add all C files to SRC_USERMOD.
    SRC_USERMOD += $(USERMODULES_DIR)/simplefunction.c
    
    # We can add our module folder to include paths if needed
    # This is not actually needed in this example.
    CFLAGS_USERMOD += -I$(USERMODULES_DIR)

If ``mpconfigport.h`` is augmented with

.. code:: make

   #define MODULE_SIMPLEFUNCTION_ENABLED (1)

you should be able to compile the module above by calling

.. code::

    !make USER_C_MODULES=../../../usermod/snippets/ all

We can then test the module as

.. code::

    %%micropython
    
    import simplefunction
    print(simplefunction.add_ints(123, 456))


.. parsed-literal::

    579
    


What a surprise! It works! It works!

Compiling for the microcontroller
---------------------------------

As pointed out at the very beginning, our first module was compiled for
the unix port, and that it, why we set ``../../micropython/ports/unix/``
as our working directory. In case, we would like to compile for the
microcontroller, we would have to modify the ``mpconfigport.h`` file of
the port (e.g., in ``micropython/ports/stm32/``) as shown in Section
`User modules <#User-modules-in-micropython>`__.

Next, in the compilation command, one has to specify the target board,
e.g., pyboard, version 1.1, and probably the path to the cross-compiler,
if that could not be installed system-wide. You would issue the make
command in the directory of the port, e.g.,
``micropython/ports/stm32/``, and the path in the ``CROSS_COMPILE``
argument must be either absolute, or given relative to
``micropython/ports/stm32/``.

.. code:: bash

   make BOARD=PYBV11 CROSS_COMPILE=<Path where you uncompressed the toolchain>/bin/arm-none-eabi-

You will your firmware under
``micropython/ports/stm32/build-PYBV11/firmware.dfu``, and you can
upload it by issuing

.. code:: bash

   python ../../tools/pydfu.py -u build-PYBV11/firmware.dfu 

on the command line. You will find a more detailed explanation under
https://github.com/micropython/micropython/wiki/Pyboard-Firmware-Update.
