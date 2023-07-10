Argument parsing
================

In practically all cases, you will have to inspect the arguments of your
function. Even if you can resort to functions in the micropython
implementation, that simply means that the burden of inspection was
taken off your shoulders, but not that the inspection does not happen at
all. In this section, we are going to see what we can do with both
positional, and keyword arguments, and how we can retrieve their values.

Positional arguments
--------------------

Known number of arguments
~~~~~~~~~~~~~~~~~~~~~~~~~

A known number of positional arguments are pretty much a done deal: we
have seen how to get the C values of such arguments: in our very first
module, we called ``mp_obj_get_int()``, because we wanted to sum two
integers. Should we like to work with float, we could call
``mp_obj_get_float()``. (This function will properly work, if the value
is an integer, by the way.)

If we have a more complicated construct, like a tuple or a list, we can
turn the argument into a pointer with

.. code:: c

   mp_obj_t some_function(mp_obj_t object_in) {
       mp_obj_tuple_t *object = MP_OBJ_TO_PTR(object_in);
       ...
   }

and continue with ``*object``. We can then retrieve the tuple’s
structure members with ``object->items`` (the elements in the tuple),
and ``object->len`` (the length of the tuple). This procedure works even
with newly-defined object types. A complete example can be found in
Section `Creating new types <#Creating-new-types>`__:

.. code:: c

   typedef struct _vector_obj_t {
       mp_obj_base_t base;
       float x, y, z;
   } vector_obj_t;


   mp_obj_t some_function(mp_obj_t object_in) {
       vector_obj_t *vector = MP_OBJ_TO_PTR(object_in);
       ...
   }

Unknown number of arguments
~~~~~~~~~~~~~~~~~~~~~~~~~~~

Now, we pointed out that the macros generating the function objects can
be of the form

.. code:: c

   MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(some_function_obj, n_argmin, n_argmax, some_function);

In such a case, we surely can’t just enumerate the arguments of the
function without any checks, especially, that we don’t even know how far
we have to go, and the behaviour of the function can depend on the
number of arguments. What shall we do in such an instance?

We have to reckon that the signature of a function with a variable
number of arguments looks like

.. code:: c

   mp_obj_t some_function(size_t n_args, const mp_obj_t *args) {
       if (n_args == 2) {
           ...
       }
       ...
   }

and the first argument of the C function will store the number of
positional arguments of the python function. Once ``n_args`` is known,
we are set. It is important to note that the work is done by the
``MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN`` macro, we do not have to set up
the C function in any particular way.

Here is a small example that will drive this point home.

https://github.com/v923z/micropython-usermod/tree/master/snippets/vararg/vararg.c

.. code:: cpp
        
    
    #include "py/obj.h"
    #include "py/runtime.h"
    
    STATIC mp_obj_t vararg_function(size_t n_args, const mp_obj_t *args) {
        if(n_args == 0) {
            printf("no arguments supplied\n");
        } else if(n_args == 1) {
            printf("this is a %lu\n", mp_obj_get_int(args[0]));
        } else if(n_args == 2) {
            printf("hm, we will sum them: %lu\n", mp_obj_get_int(args[0]) + mp_obj_get_int(args[1]));
        } else if(n_args == 3) {
            printf("Look at that! A triplet: %lu, %lu, %lu\n", mp_obj_get_int(args[0]), mp_obj_get_int(args[1]), mp_obj_get_int(args[2]));
        }
        return mp_const_none;
    } 
    
    STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(vararg_function_obj, 0, 3, vararg_function);
    
    STATIC const mp_rom_map_elem_t vararg_module_globals_table[] = {
        { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_vararg) },
        { MP_ROM_QSTR(MP_QSTR_vararg), MP_ROM_PTR(&vararg_function_obj) },
    };
    STATIC MP_DEFINE_CONST_DICT(vararg_module_globals, vararg_module_globals_table);
    
    const mp_obj_module_t vararg_user_cmodule = {
        .base = { &mp_type_module },
        .globals = (mp_obj_dict_t*)&vararg_module_globals,
    };
    
    MP_REGISTER_MODULE(MP_QSTR_vararg, vararg_user_cmodule);

https://github.com/v923z/micropython-usermod/tree/master/snippets/vararg/micropython.mk

.. code:: make
        
    
    USERMODULES_DIR := $(USERMOD_DIR)
    
    # Add all C files to SRC_USERMOD.
    SRC_USERMOD_C += $(USERMODULES_DIR)/vararg.c
    
    CFLAGS_USERMOD += -I$(USERMODULES_DIR)
.. code:: bash

    !make clean
    !make USER_C_MODULES=../../../usermod/snippets/vararg
.. code ::
        
    %%micropython
    
    import vararg
    
    vararg.vararg()
    vararg.vararg(1)
    vararg.vararg(10, 20)
    vararg.vararg(1, 22, 333)
.. parsed-literal::

    no arguments supplied
    this is a 1
    hm, we will sum them: 30
    Look at that! A triplet: 1, 22, 333
    
    

Working with strings
--------------------

We have discussed numerical values in micropython at length. We know how
we convert an ``mp_obj_t`` object to a native C type, and we also know,
how we can turn an integer or float into an ``mp_obj_t``, and return it
at the end of the function. The key components were the
``mp_obj_get_int()``, ``mp_obj_new_int()``, and ``mp_obj_get_float()``,
and ``mp_obj_new_float()`` functions. Later we will see, what we can do
with various iterables, like lists and tuples, but before that, I would
like to explain, how one handles strings. (Strings are also iterables in
python, by the way, however, they also have a native C equivalent.)

At the beginning, we said that in micropython, almost everything is an
``mp_obj_t`` object. Strings are no exception: however, the ``mp_obj_t``
that denotes the string does not store its value, but a pointer to the
memory location, where the characters are stored. The reason is rather
trivial: the ``mp_obj_t`` has a size of 8 bytes, hence, the object can’t
possibly store a string that is longer than 7 bytes. (The same applies
to more complicated objects, e.g., lists, or tuples.)

Now, the procedure of working with the string would kick out with
retrieving the pointer, and then we could increment its value till we
encounter the ``\0`` character, which indicates that the string has
ended. Fortunately, micropython has a handy macro for retrieving the
string’s value and its length, so we don’t have to concern ourselves
with the really low-level stuff. For the string utilities, we should
include ``py/objstr.h`` (for the micropython things), and ``string.h``
(for ``strcpy``). ``py/objstr.c`` contains a number of tools for string
manipulation. Before you try to implement your own functions, it might
be worthwhile to check that out. You might find something useful.

Our next module is going to take a single string as an argument, print
out its length (you already know, how to return the length, don’t you?),
and return the contents backwards. All this in 33 lines.

https://github.com/v923z/micropython-usermod/tree/master/snippets/stringarg/stringarg.c

.. code:: cpp
        
    
    #include <string.h>
    #include "py/obj.h"
    #include "py/runtime.h"
    #include "py/objstr.h"
    
    #define byteswap(a,b) char tmp = a; a = b; b = tmp; 
    
    STATIC mp_obj_t stringarg_function(const mp_obj_t o_in) {
        mp_check_self(mp_obj_is_str_or_bytes(o_in));
        GET_STR_DATA_LEN(o_in, str, str_len);
        printf("string length: %lu\n", str_len);
        char out_str[str_len];
        strcpy(out_str, (char *)str);
        for(size_t i=0; i < (str_len-1)/2; i++) {
            byteswap(out_str[i], out_str[str_len-i-1]);
        }
        return mp_obj_new_str(out_str, str_len);
    } 
    
    STATIC MP_DEFINE_CONST_FUN_OBJ_1(stringarg_function_obj, stringarg_function);
    
    STATIC const mp_rom_map_elem_t stringarg_module_globals_table[] = {
        { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_stringarg) },
        { MP_ROM_QSTR(MP_QSTR_stringarg), MP_ROM_PTR(&stringarg_function_obj) },
    };
    STATIC MP_DEFINE_CONST_DICT(stringarg_module_globals, stringarg_module_globals_table);
    
    const mp_obj_module_t stringarg_user_cmodule = {
        .base = { &mp_type_module },
        .globals = (mp_obj_dict_t*)&stringarg_module_globals,
    };
    
    MP_REGISTER_MODULE(MP_QSTR_stringarg, stringarg_user_cmodule);

The macro defined in ``objstr.h`` takes three arguments, out of which
only the first one is actually defined. The other two are defined in the
macro itself. So, in the line

.. code:: c

   GET_STR_DATA_LEN(o_in, str, str_len);

only ``o_in`` is known at the moment the macro is called, ``str``, which
will be a pointer to type character, and ``str_len``, which is of type
``size_t``, and holds the length of the string, are created by
``GET_STR_DATA_LEN`` itself. This is, why we can later stick
``str_len``, and ``str`` into print statements, though, we never
declared these variables.

After ``GET_STR_DATA_LEN`` has been called, we are in C land. First, we
print out the length, then reverse the string. But why can’t we do the
string inversion on the original string, and why do we have to declare a
new variable, ``out_str``? The reason for that is that the
``GET_STR_DATA_LEN`` macro declares a ``const`` string, which we can’t
change anymore, so we have to copy the content (``strcpy`` from
``string.h``), and swap the bytes in ``out_str``. When doing so, we
should keep in mind that the very last byte in the string is the
termination character, hence, we exchange the ``i``\ th position with
the ``str_len-i-1``\ th position. If you fail to notice the ``-1``,
you’ll end up with an empty string: even though the byte swapping would
run without complaints, the very first byte would be equal to ``\0``.

At the very end, we return from our function with a call to
``mp_obj_new_str``, which creates a new ``mp_obj_t`` object that points
to the content of the string. And we are done! All there is left to do
is compilation. Let’s take care of that!

https://github.com/v923z/micropython-usermod/tree/master/snippets/stringarg/micropython.mk

.. code:: make
        
    
    USERMODULES_DIR := $(USERMOD_DIR)
    
    # Add all C files to SRC_USERMOD.
    SRC_USERMOD_C += $(USERMODULES_DIR)/stringarg.c
    
    CFLAGS_USERMOD += -I$(USERMODULES_DIR)
.. code:: bash

    !make clean
    !make USER_C_MODULES=../../../usermod/snippets/stringarg
.. code ::
        
    %%micropython
    
    import stringarg
    
    print(stringarg.stringarg('...krow ta eludom gragnirts eht'))
.. parsed-literal::

    string length: 31
    the stringarg module at work...
    
    

Keyword arguments
-----------------

One of the most useful features of python is that functions can accept
positional as well as keyword arguments, thereby providing a very
flexible and instructive function interface. (Instructive, insofar as
the intent of a variable is very explicit, even at the user level.) In
this subsection, we will learn how the processing of keyword arguments
is done. Our new module will be the sexed-up version of our very first
one, where we added two integers. We will do the same here, except that
the second argument will be a keyword, and will assume a default value
of 0.

Before jumping into the implementation, we should contemplate the task
for a second. It does not matter, whether we have positional or keyword
arguments, at one point, the interpreter has to turn all arguments into
a deterministic sequence of objects. We stipulate this sequence in the
constant variable called ``allowed_args[]``. This is an array of type
``mp_arg_t``, which is nothing but a structure with two ``uint16``
values, and a union named ``mp_arg_val_t``. This union holds the default
value and the type of the variable that we want to pass. The
``mp_arg_t`` structure, defined in ``runtime.h``, looks like this:

.. code:: c

   typedef struct _mp_arg_t {
       uint16_t qst;
       uint16_t flags;
       mp_arg_val_t defval;
   } mp_arg_t;

The last member, ``mp_arg_val_t`` is

.. code:: c

   typedef union _mp_arg_val_t {
       bool u_bool;
       mp_int_t u_int;
       mp_obj_t u_obj;
       mp_rom_obj_t u_rom_obj;
   } mp_arg_val_t;

Keyword arguments come in three flavours: ``MP_ARG_BOOL``,
``MP_ARG_INT``, and ``MP_ARG_OBJ``.

Keyword arguments with numerical values
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

And now the implementation:

https://github.com/v923z/micropython-usermod/tree/master/snippets/keywordfunction/keywordfunction.c

.. code:: cpp
        
    
    #include <stdio.h>
    #include "py/obj.h"
    #include "py/runtime.h"
    #include "py/builtin.h"
    
    STATIC mp_obj_t keywordfunction_add_ints(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
        static const mp_arg_t allowed_args[] = {
            { MP_QSTR_a, MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
            { MP_QSTR_b, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 0} },
        };
        
        mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
        mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);
        int16_t a = args[0].u_int;
        int16_t b = args[1].u_int;
        printf("a = %d, b = %d\n", a, b);
        return mp_obj_new_int(a + b);
    }
    
    STATIC MP_DEFINE_CONST_FUN_OBJ_KW(keywordfunction_add_ints_obj, 1, keywordfunction_add_ints);
    
    STATIC const mp_rom_map_elem_t keywordfunction_module_globals_table[] = {
        { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_keywordfunction) },
        { MP_ROM_QSTR(MP_QSTR_add_ints), (mp_obj_t)&keywordfunction_add_ints_obj },
    };
    
    STATIC MP_DEFINE_CONST_DICT(keywordfunction_module_globals, keywordfunction_module_globals_table);
    
    const mp_obj_module_t keywordfunction_user_cmodule = {
        .base = { &mp_type_module },
        .globals = (mp_obj_dict_t*)&keywordfunction_module_globals,
    };
    
    MP_REGISTER_MODULE(MP_QSTR_keywordfunction, keywordfunction_user_cmodule);

One side effect of a function with keyword arguments is that we do not
have to care about the arguments in the C implementation: the argument
list is always the same, and it is passed in by the interpreter: the
number of arguments of the python function, an array with the positional
arguments, and a map for the keyword arguments.

After parsing the arguments with ``mp_arg_parse_all``, whatever was at
the zeroth position of ``allowed_args[]`` will be called ``args[0]``,
the object at the first position of ``allowed_args[]`` will be turned
into ``args[1]``, and so on.

This is, where we also define, what the name of the keyword argument is
going to be: whatever comes after ``MP_QSTR_``. But hey, presto! The
name should be an integer with 16 bits, shouldn’t it? After all, this is
the first member of ``mp_arg_t``. So what the hell is going on here?
Well, for the efficient use of RAM, all MP_QSTRs are turned into
``unint16_t`` internally. This applies not only to the names in
functions with keyword arguments, but also for module and function
names, in the ``_module_globals_table[]``.

The second member of the ``mp_arg_t`` structure is the flags that
determine, e.g., whether the argument is required, if it is of integer
or ``mp_obj_t`` type, and whether it is a positional or a keyword
argument. These flags can be combined by ORing them, as we have done in
the example above.

The last member in ``mp_arg_t`` is the default value. Since this is a
member variable, when we make use of it, we have to extract the value by
adding ``.u_int`` to the argument.

When turning our function into a function object, we have to call a
special macro, ``MP_DEFINE_CONST_FUN_OBJ_KW``, defined in ``obj.h``,
which is somewhat similar to ``MP_DEFINE_CONST_FUN_OBJ_VAR``: in
addition to the function object and the function, one also has to
specify the minimum number of arguments in the python function.

Other examples on passing keyword arguments can be found in some of the
hardware implementation files, e.g., ``ports/stm32/pyb_i2c.c``, or
``ports/stm32/pyb_spi.c``.

Now, let us see, whether we can add two numbers here.

https://github.com/v923z/micropython-usermod/tree/master/snippets/keywordfunction/micropython.mk

.. code:: make
        
    
    USERMODULES_DIR := $(USERMOD_DIR)
    
    # Add all C files to SRC_USERMOD.
    SRC_USERMOD_C += $(USERMODULES_DIR)/keywordfunction.c
    
    CFLAGS_USERMOD += -I$(USERMODULES_DIR)
.. code:: bash

    !make clean
    !make USER_C_MODULES=../../../usermod/snippets/keywordfunction
.. code ::
        
    %%micropython
    
    import keywordfunction
    print(keywordfunction.add_ints(-3, b=4))
    print(keywordfunction.add_ints(3))
.. parsed-literal::

    a = -3, b = 4
    1
    a = 3, b = 0
    3
    
    

As advertised, both function calls do what they were supposed to do: in
the first case, ``b`` assumes the value of 4, while in the second case,
it takes on 0, even though we didn’t supply anything to the function.

Arbitrary keyword arguments
~~~~~~~~~~~~~~~~~~~~~~~~~~~

We have seen how integer values can be extracted from keyword arguments,
but unfortunately, that method is going to get you only that far. What
if we want to pass something more complicated, in particular a string,
or a tuple, or some other non-trivial python type?

A simple solution could be to implement the C function without keywords
at all, and do the parsing in python. After all, it is highly unlikely
that parsing would be expensive in comparison to the body of the
function. But perhaps, you have your reasons for not going down that
rabbit hole.

For such cases, we can still resort to objects of type ``.u_rom_obj``.
In order to experiment with the possibilities, in the next module, we
define a function that simply returns the values passed to it. The input
arguments are going to be a single positional argument, and four keyword
arguments with type ``int``, ``string``, ``tuple``, and ``float``.

https://github.com/v923z/micropython-usermod/tree/master/snippets/arbitrarykeyword/arbitrarykeyword.c

.. code:: cpp
        
    
    #include <stdio.h>
    #include "py/obj.h"
    #include "py/objlist.h"
    #include "py/runtime.h"
    #include "py/builtin.h"
    
    // This is lifted from objfloat.c, because mp_obj_float_t is not exposed there (there is no header file)
    typedef struct _mp_obj_float_t {
        mp_obj_base_t base;
        mp_float_t value;
    } mp_obj_float_t;
    
    const mp_obj_float_t my_float = {{&mp_type_float}, 0.987};
    
    const mp_rom_obj_tuple_t my_tuple = {
        {&mp_type_tuple},
        3,
        {
            MP_ROM_INT(0),
            MP_ROM_QSTR(MP_QSTR_float),
            MP_ROM_PTR(&my_float),
        },
    };
    
    STATIC mp_obj_t arbitrarykeyword_print(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
        static const mp_arg_t allowed_args[] = {
            { MP_QSTR_a, MP_ARG_INT, {.u_int = 0} },
            { MP_QSTR_b, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 1} },
            { MP_QSTR_c, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_rom_obj = MP_ROM_QSTR(MP_QSTR_float)} },
            { MP_QSTR_d, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_rom_obj = MP_ROM_PTR(&my_float)} },
            { MP_QSTR_e, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_rom_obj = MP_ROM_PTR(&my_tuple)} },
        };
    
        mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
        mp_arg_parse_all(1, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);
        mp_obj_t tuple[5];
        tuple[0] = mp_obj_new_int(args[0].u_int); // a
        tuple[1] = mp_obj_new_int(args[1].u_int); // b
        tuple[2] = args[2].u_obj; // c
        tuple[3] = args[3].u_obj; // d
        tuple[4] = args[4].u_obj; // e
        return mp_obj_new_tuple(5, tuple);
    }
    
    STATIC MP_DEFINE_CONST_FUN_OBJ_KW(arbitrarykeyword_print_obj, 1, arbitrarykeyword_print);
    
    STATIC const mp_rom_map_elem_t arbitrarykeyword_module_globals_table[] = {
        { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_arbitrarykeyword) },
        { MP_ROM_QSTR(MP_QSTR_print), (mp_obj_t)&arbitrarykeyword_print_obj },
    };
    
    STATIC MP_DEFINE_CONST_DICT(arbitrarykeyword_module_globals, arbitrarykeyword_module_globals_table);
    
    const mp_obj_module_t arbitrarykeyword_user_cmodule = {
        .base = { &mp_type_module },
        .globals = (mp_obj_dict_t*)&arbitrarykeyword_module_globals,
    };
    
    MP_REGISTER_MODULE(MP_QSTR_arbitrarykeyword, arbitrarykeyword_user_cmodule);

Before compiling the code, let us think a bit about what is going on
here. The first argument, ``a``, is straightforward: that is a
positional argument, and we deal with that as we did in the last
example. The same applies to the second argument, ``b``, which is our
first keyword argument with an integer default value.

Matters become more interesting with the third argument, ``c``: that is
supposed to be a string, whose default value is “float”. We generate the
respective C representation by prepending the ``MP_QSTR_``. At this
point, we have a string, but we still can’t assign it as a default
value. We do that by first applying the ``MP_ROM_QSTR`` macro, and
assigning the results to the ``.u_rom_obj`` member of the ``mp_arg_t``
structure. You most certainly will want to inspect the value at one
point. We have already discussed the drill in `Working with
strings <#Working-with-strings>`__.

The fourth argument, ``d``, is meant to be a float. Since there is no
equivalent of a float in the ``mp_arg_t`` structure, we have to turn our
number into an ``MP_ROM_PTR``, so we have to retrieve the address of the
float object. To this end, we define the number in the line

.. code:: c

   const mp_obj_float_t my_float = {{&mp_type_float}, 0.987};

Note that since ``mp_obj_float_t`` is not exposed in ``objfloat.c``,
where it is defined, we had to copy the type declaration. This is
certainly not very elegant, but desperate times call for desperate
measures. In addition, we also have to declare ``my_float`` as a
constant. The reason for this is that we have to assure the compiler
that this value is not going to change in the future, so that it can be
saved into the read-only memory.

The last argument, ``e``, is a tuple, which has a special type for such
cases, namely, the ``mp_rom_obj_tuple_t``, so we define ``my_tuple`` as
an ``mp_rom_obj_tuple_t`` object, with a base type of ``mp_type_tuple``,
and three elements, an integer, a string, and a float. The elements go
into the tuple as if they were assigned to the ``.u_rom_obj`` members
directly, hence the macros ``MP_ROM_INT``, ``MP_ROM_QSTR``, and
``MP_ROM_PTR``.

When we return the default values at the end of our function, we declare
an array of type ``mb_obj_t``, and of length 5, assign the elements, and
turn the array into a tuple with ``mp_obj_new_tuple``.

One final comment to this section: I referred to our function as
returning the values of the arguments, yet, I called it ``print``. Had I
called the function ``return``, it wouldn’t have worked for the simple
reason, that ``return`` is a keyword of the language itself. As a
friendly advice, do not try to override that!

Having thoroughly discussed the code, we should compile it, and see what
happens.

https://github.com/v923z/micropython-usermod/tree/master/snippets/arbitrarykeyword/micropython.mk

.. code:: make
        
    
    USERMODULES_DIR := $(USERMOD_DIR)
    
    # Add all C files to SRC_USERMOD.
    SRC_USERMOD_C += $(USERMODULES_DIR)/arbitrarykeyword.c
    
    CFLAGS_USERMOD += -I$(USERMODULES_DIR)
.. code:: bash

    !make clean
    !make USER_C_MODULES=../../../usermod/snippets/arbitrarykeyword
.. code ::
        
    %%micropython
    
    import arbitrarykeyword
    print(arbitrarykeyword.print(1, b=123))
    print(arbitrarykeyword.print(-35, b=555, c='foo', d='bar', e=[1, 2, 3]))
.. parsed-literal::

    (1, 123, 'float', 0.9869999999999999, (0, 'float', 0.9869999999999999))
    (-35, 555, 'foo', 'bar', [1, 2, 3])
    
    

We should note that the particular definition of the float constant will
work in the ``A``, and ``B`` object representations only. If your
``micropython`` platform uses either the ``C``, or the ``D``
representation, your code will still compile, but you’ll be surprised by
the results.
