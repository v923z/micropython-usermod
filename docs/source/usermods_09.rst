Working with classes
====================

Of course, python would not be python without classes. A module can also
include the implementation of classes. The procedure is similar to what
we have already seen in the context of standard functions, except that
we have to define a structure that holds at least a string with the name
of the class, a pointer to the initialisation and printout functions,
and a local dictionary. A typical class structure would look like

.. code:: c

   const mp_obj_type_t simpleclass_type;

   STATIC const mp_rom_map_elem_t simpleclass_locals_dict_table[] = {
       { MP_ROM_QSTR(MP_QSTR_method1), MP_ROM_PTR(&simpleclass_method1_obj) },
       { MP_ROM_QSTR(MP_QSTR_method2), MP_ROM_PTR(&simpleclass_method2_obj) },
       ...                                                           
   }

   MP_DEFINE_CONST_OBJ_TYPE(
        simpleclass_type,
        MP_QSTR_simpleclass,
        MP_TYPE_FLAG_NONE,
        print, simpleclass_print,
        make_new, simpleclass_make_new,
        locals_dict, &simpleclass_locals_dict
    );


The ``MP_DEFINE_CONST_OBJ_TYPE`` macro defines a structure called
``simpleclass_type``, which is of type ``mp_obj_type_t``. The structure
contains a pointer to the type of the object, ``&mp_type_type``, the
name of the class, ``MP_QSTR_simpleclass``, the flags, which are set to
``MP_TYPE_FLAG_NONE``, the printout and initialisation functions,
``print`` and ``make_new`` which are roughly the equivalent of ``__str__`` 
and ``__init__``, and the local dictionary, ``locals_dict`` that contains
all user-facing methods and constants of the class.

In order to see how this all works, we are going to implement a very
simple class, which holds two integer variables, and has a method that
returns the sum of these two variables. In python, a possible
realisation could look like this:

.. code ::
        
    class myclass:
        
        def __init__(self, a, b):
            self.a = a
            self.b = b
            
        def mysum(self):
            return self.a + self.b
        
        
    A = myclass(1, 2)
    A.mysum()



.. parsed-literal::

    3



In addition to the class implementation above and in order to show how
class methods and regular functions can live in the same module, we will
also have a function, which is not bound to the class itself, and which
adds the two components in the class, i.e., that is similar to

.. code ::
        
    def add(class_instance):
        return class_instance.a + class_instance.b
    
    add(A)



.. parsed-literal::

    3



(Note that retrieving values from the class in this way is not exactly
elegant, nor is it pythonic. We would usually implement a getter method
for that.)

https://github.com/v923z/micropython-usermod/tree/master/snippets/simpleclass/simpleclass.c

.. code:: cpp
        
    
    #include <stdio.h>
    #include "py/runtime.h"
    #include "py/obj.h"
    
    typedef struct _simpleclass_myclass_obj_t {
        mp_obj_base_t base;
        int16_t a;
        int16_t b;
    } simpleclass_myclass_obj_t;
    
    const mp_obj_type_t simpleclass_myclass_type;
    
    STATIC void myclass_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
        (void)kind;
        simpleclass_myclass_obj_t *self = MP_OBJ_TO_PTR(self_in);
        mp_print_str(print, "myclass(");
        mp_obj_print_helper(print, mp_obj_new_int(self->a), PRINT_REPR);
        mp_print_str(print, ", ");
        mp_obj_print_helper(print, mp_obj_new_int(self->b), PRINT_REPR);
        mp_print_str(print, ")");
    }
    
    STATIC mp_obj_t myclass_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
        mp_arg_check_num(n_args, n_kw, 2, 2, true);
        simpleclass_myclass_obj_t *self = m_new_obj(simpleclass_myclass_obj_t);
        self->base.type = &simpleclass_myclass_type;
        self->a = mp_obj_get_int(args[0]);
        self->b = mp_obj_get_int(args[1]);
        return MP_OBJ_FROM_PTR(self);
    }
    
    // Class methods
    STATIC mp_obj_t myclass_sum(mp_obj_t self_in) {
        simpleclass_myclass_obj_t *self = MP_OBJ_TO_PTR(self_in);
        return mp_obj_new_int(self->a + self->b);
    }
    
    MP_DEFINE_CONST_FUN_OBJ_1(myclass_sum_obj, myclass_sum);
    
    STATIC const mp_rom_map_elem_t myclass_locals_dict_table[] = {
        { MP_ROM_QSTR(MP_QSTR_mysum), MP_ROM_PTR(&myclass_sum_obj) },
    };
    
    STATIC MP_DEFINE_CONST_DICT(myclass_locals_dict, myclass_locals_dict_table);
    
    MP_DEFINE_CONST_OBJ_TYPE(
        simpleclass_myclass_type,
        MP_QSTR_simpleclass,
        MP_TYPE_FLAG_NONE,
        print, myclass_print,
        make_new, myclass_make_new,
        locals_dict, &myclass_locals_dict
    );
    
    // Module functions
    STATIC mp_obj_t simpleclass_add(const mp_obj_t o_in) {
        simpleclass_myclass_obj_t *class_instance = MP_OBJ_TO_PTR(o_in);
        return mp_obj_new_int(class_instance->a + class_instance->b);
    }
    
    MP_DEFINE_CONST_FUN_OBJ_1(simpleclass_add_obj, simpleclass_add);
    
    STATIC const mp_map_elem_t simpleclass_globals_table[] = {
        { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_simpleclass) },
        { MP_OBJ_NEW_QSTR(MP_QSTR_myclass), (mp_obj_t)&simpleclass_myclass_type },	
        { MP_OBJ_NEW_QSTR(MP_QSTR_add), (mp_obj_t)&simpleclass_add_obj },
    };
    
    STATIC MP_DEFINE_CONST_DICT (
        mp_module_simpleclass_globals,
        simpleclass_globals_table
    );
    
    const mp_obj_module_t simpleclass_user_cmodule = {
        .base = { &mp_type_module },
        .globals = (mp_obj_dict_t*)&mp_module_simpleclass_globals,
    };
    
    MP_REGISTER_MODULE(MP_QSTR_simpleclass, simpleclass_user_cmodule);

One more thing to note: the functions that are pointed to in
``simpleclass_myclass_type`` are not registered with the macro
``MP_DEFINE_CONST_FUN_OBJ_VAR`` or similar. The reason for this is that
this happens automatically: ``myclass_print`` does not require
user-supplied arguments beyond ``self``, so it is known what the
signature should look like. In ``myclass_make_new``, we inspect the
argument list, when calling

.. code:: c

   mp_arg_check_num(n_args, n_kw, 2, 2, true);

so, again, there is no need to turn our function into a function object.

Printing class properties
-------------------------

In ``myclass_print``, instead of the standard the C function ``printf``, we
made use of ``mp_print_str``, and ``mp_obj_print_helper``, which are
options in this case. Both take ``print`` as their first argument. The
value of ``print`` is supplied by the ``print`` method of the class
itself. The second argument is a string (in the case of
``mp_print_str``), or a ``micropython`` object (for
``mp_obj_print_helper``). In addition, ``mp_obj_print_helper`` takes a
pre-defined constant, ``PRINT_REPR`` as its third argument. By resorting
to these ``micropython`` printing functions, we can make certain that
the output is formatted nicely, independent of the platform. Whenever
``print`` is available, these function should be used instead of
``printf``. For debugging purposes, ``printf`` is also fine. More on the
subject can be found in ``mpprint.c``.

https://github.com/v923z/micropython-usermod/tree/master/snippets/simpleclass/micropython.mk

.. code:: make
        
    
    USERMODULES_DIR := $(USERMOD_DIR)
    
    # Add all C files to SRC_USERMOD.
    SRC_USERMOD_C += $(USERMODULES_DIR)/simpleclass.c
    
    CFLAGS_USERMOD += -I$(USERMODULES_DIR)
.. code:: bash

    !make clean
    !make USER_C_MODULES=../../../usermod/snippets/simpleclass
.. code ::
        
    %%micropython
    
    import simpleclass
    a = simpleclass.myclass(2, 3)
    print(a)
    print(a.mysum())
.. parsed-literal::

    myclass(2, 3)
    5
    
    

Special methods of classes
--------------------------

Python has a number of special methods, which will make a class behave
as a native object. So, e.g., if a class implements the
``__add__(self, other)`` method, then instances of that class can be
added with the ``+`` operator. Here is an example in python:

.. code ::
        
    class Adder:
        
        def __init__(self, value):
            self.value = value
            
        def __add__(self, other):
            self.value = self.value + other.value
            return self
    
    a = Adder(1)
    b = Adder(2)
    
    c = a + b
    c.value



.. parsed-literal::

    3



Note that, while the above example is not particularly useful, it proves
the point: upon calling the ``+`` operator, the values of ``a``, and
``b`` are added. If we had left out the implementation of the
``__add__`` method, the python interpreter would not have a clue as to
what to do with the objects. You can see for yourself, how sloppiness
makes itself manifest:

.. code ::
        
    class Adder:
        
        def __init__(self, value):
            self.value = value
    
    a = Adder(1)
    b = Adder(2)
    
    c = a + b
    c.value

::


    ---------------------------------------------------------------------------

    TypeError                                 Traceback (most recent call last)

    <ipython-input-77-635006a6f7bc> in <module>
          7 b = Adder(2)
          8 
    ----> 9 c = a + b
         10 c.value


    TypeError: unsupported operand type(s) for +: 'Adder' and 'Adder'


Indeed, we do not support the ``+`` operator.

Now, the problem is that in the C implementation, these special methods
have to be treated in a special way. The naive approach would be to add
the pointer to the function to the locals dictionary as

.. code:: c

   STATIC const mp_rom_map_elem_t simpleclass_locals_dict_table[] = {
       { MP_ROM_QSTR(MP_QSTR___add__), MP_ROM_PTR(&simpleclass_add_obj) },
   };

but that would not work. Well, this is not entirely true: the ``+``
operator would not work, but one could still call the method explicitly
as

.. code:: python

   a = Adder(1)
   b = Adder(2)

   a.__add__(b)

Before we actually add the ``+`` operator to our class, we should note
that there are two kinds of special methods, namely the unary and the
binary operators.

In the first group are those, whose sole argument is the class instance
itself. Two frequently used cases are the length operator, ``len``, and
``bool``. So, e.g., if your class implements the ``__len__(self)``
method, and the method returns an integer, then you can call the ``len``
function in the console

.. code:: python

   len(myclass)

In the second category of operators are those, which require a left, as
well as a right hand side: the operand on the left hand side is the
class instance itself, while the right hand side can, in principle, be
another instance of the same class, or some other type. An example for
this was the ``__add__`` method in our ``Adder`` class. To prove that
the right hand side needn’t be of the same type, think of the
*multiplication* of lists:

.. code ::
        
    [1, 2, 3]*5



.. parsed-literal::

    [1, 2, 3, 1, 2, 3, 1, 2, 3, 1, 2, 3, 1, 2, 3]



is perfectly valid, and has a well-defined meaning. It is the
responsibility of the C implementation to inspect the right hand side,
and decide how to interpret the operation. The complete list of unary,
as well as binary operators can be found in ``runtime.h``.

The module below implements five special methods altogether. Two unary,
namely, ``bool``, and ``len``, and three binary operators, ``==``,
``+``, and ``*``. Since the addition and multiplication will return a
new instance of ``specialclass_myclass``, we define a new function,
``create_new_class``, that, well, creates a new instance of
``specialclass_myclass``, and initialises the members with the two input
arguments. This function will also be called in the class initialisation
function, ``myclass_make_new``, immediately after the argument checking.

When implementing the operators, we have to keep a couple of things in
mind. First, the ``specialclass_myclass_type`` has to be extended with
the two pairs, ``unary_op``, and ``binary_op``, and their callback 
functions. Where ``unary_op``'s callback is the function that handles 
the unary operation (``specialclass_unary_op`` in the example below), 
and ``binary_op``'s callback is the function that deals with binary 
operations (``specialclass_binary_op`` below). These two functions have 
the signatures

.. code:: c

   STATIC mp_obj_t specialclass_unary_op(mp_unary_op_t op, mp_obj_t self_in)

and

.. code:: c

   STATIC mp_obj_t specialclass_binary_op(mp_binary_op_t op, mp_obj_t lhs, mp_obj_t rhs)

respectively, and we have to inspect the value of ``op`` in the
implementation. This is done in the two ``switch`` statements.

Second, if ``unary_op``, or ``binary_op`` are defined for the class,
then the handler function must have an implementation of all possible
operators. This doesn’t necessarily mean that you have to have all cases
in the ``switch``, but if you haven’t, then there must be a ``default``
case with a reasonable return value, e.g., ``MP_OBJ_NULL``, or
``mp_const_none``, so as to indicate that that particular method is not
available.

https://github.com/v923z/micropython-usermod/tree/master/snippets/specialclass/specialclass.c

.. code:: cpp
        
    
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
        mp_print_str(print, "myclass(");
        mp_obj_print_helper(print, mp_obj_new_int(self->a), PRINT_REPR);
        mp_print_str(print, ", ");
        mp_obj_print_helper(print, mp_obj_new_int(self->b), PRINT_REPR);
        mp_print_str(print, ")");
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
            default: return MP_OBJ_NULL; // operator not supported
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
                return MP_OBJ_NULL; // operator not supported
        }
    }
    
    MP_DEFINE_CONST_OBJ_TYPE(
        specialclass_myclass_type,
        MP_QSTR_specialclass,
        MP_TYPE_FLAG_NONE,
        print, myclass_print,
        make_new, myclass_make_new,
        unary_op, specialclass_unary_op,
        binary_op, specialclass_binary_op,
        locals_dict, myclass_locals_dict
    );
    
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
    
    MP_REGISTER_MODULE(MP_QSTR_specialclass, specialclass_user_cmodule);

https://github.com/v923z/micropython-usermod/tree/master/snippets/specialclass/micropython.mk

.. code:: make
        
    
    USERMODULES_DIR := $(USERMOD_DIR)
    
    # Add all C files to SRC_USERMOD.
    SRC_USERMOD_C += $(USERMODULES_DIR)/specialclass.c
    
    CFLAGS_USERMOD += -I$(USERMODULES_DIR)
.. code:: bash

    !make clean
    !make USER_C_MODULES=../../../usermod/snippets/specialclass
.. code ::
        
    %%micropython
    
    import specialclass
    
    a = specialclass.myclass(1, 2)
    b = specialclass.myclass(10, 20)
    print(a)
    print(b)
    print(a + b)
.. parsed-literal::

    myclass(1, 2)
    myclass(10, 20)
    myclass(11, 22)
    
    

Properties
----------

In addition to methods, in python, classes can also have properties,
which will basically return some read-only attributes of the class. Take
the following example:

.. code ::
        
    class PropClass:
    
        def __init__(self, x):
            self._x = x
    
        @property
        def x(self):
            return self._x
We can now create an instance of ``PropClass``, and access the value of
``_x`` by “calling” the decorated ``x`` method without the brackets
characteristic of function calls:

.. code ::
        
    c = PropClass(12.3)
    c.x



.. parsed-literal::

    12.3



One use case is, when you want to protect the value of ``_x``, and want
to prevent accidental changes: if you want to write to the ``x``
property, you’ll get a nicely-formatted exception:

.. code ::
        
    c.x = 55.5

::


    ---------------------------------------------------------------------------

    AttributeError                            Traceback (most recent call last)

    <ipython-input-50-63b5601caccb> in <module>
    ----> 1 c.x = 55.5
    

    AttributeError: can't set attribute


It is nifty, isn’t it? Now, let us see, how we can deal with this in
micropython. In order to simplify things, we will implement what we have
just seen above: a class that holds a single floating point value, and
does nothing else.

Most of the code should be familiar from our first example on classes,
so I will discuss the single new function that is relevant to
properties. At the C level, a property is nothing but a void function
with exactly three arguments

.. code:: c

   STATIC void some_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination) {
       ...
   }

where ``self_in`` is the class instance, ``attribute`` is a string with
the property’s name, and ``destination`` is a pointer to the return
value of the function that is going to be called, when querying for the
property. So, in the python example above, ``attribute`` is ``x``,
because we queried the ``x`` property, and ``destination`` will be the
equivalent of ``self._x``, because ``self._x`` is what is returned by
the ``PropClass.x()`` method.

In the C function, we do not return anything, instead, we assign the
desired property (attribute) of the class to ``destination[0]`` as in
the snippet below:

.. code:: c

   STATIC void propertyclass_attr(mp_obj_t self, qstr attribute, mp_obj_t *destination) {
       if(attribute == MP_QSTR_x) {
           destination[0] = propertyclass_x(self);
       }
   }

The ``qstr`` is required, because a class might have multiple
properties, but all these properties are retrieved by a single function,
``propertyclass_attr``. Thus, should we want to return another property
with name ``y``, we would augment the function as

.. code:: c

   STATIC void propertyclass_attr(mp_obj_t self, qstr attribute, mp_obj_t *destination) {
       if(attribute == MP_QSTR_x) {
           destination[0] = propertyclass_x(self);
       } else if(attribute == MP_QSTR_y) {
           destination[0] = propertyclass_y(self);
       }
   }

Now, we are almost done, but we still have to implement the function
that actually retrieves the attribute. This is what happens here:

.. code:: c

   STATIC mp_obj_t propertyclass_x(mp_obj_t self_in) {
       propertyclass_obj_t *self = MP_OBJ_TO_PTR(self_in);
       return mp_obj_new_float(self->x);
   }

Remember, ``destination`` was a pointer to ``mp_obj_t``, so whatever
function we have, it must return ``mp_obj_t``. In this particular case,
the implementation is trivial: we fetch the value of ``self->x``, and
turn it into an ``mp_obj_new_float``.

We are now done, right? Not quite: while the required functions are
implemented, they will never be called. We have to attach them to the
class, so that the interpreter knows what is to do, when we try to
access ``c.x``. This act of attaching the function happens in the type
definition of the class: we add a new pair to our type definition. By 
adding ``atty`` and its callback function which is ``propertyclass_attr``
the interpreter can fill in the three arguments. The first argument is
the class instance, the second argument is the name of the property, and
the third argument is the return value of the function that is going to
be called, when querying for the property.

And with that, we are ready to compile the code.

https://github.com/v923z/micropython-usermod/tree/master/snippets/properties/properties.c

.. code:: cpp
        
    
    #include <stdio.h>
    #include "py/runtime.h"
    #include "py/obj.h"
    
    typedef struct _propertyclass_obj_t {
        mp_obj_base_t base;
        mp_float_t x;
    } propertyclass_obj_t;
    
    const mp_obj_type_t propertyclass_type;
    
    STATIC mp_obj_t propertyclass_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
        mp_arg_check_num(n_args, n_kw, 1, 1, true);
        propertyclass_obj_t *self = m_new_obj(propertyclass_obj_t);
        self->base.type = &propertyclass_type;
        self->x = mp_obj_get_float(args[0]);
        return MP_OBJ_FROM_PTR(self);
    }
    
    STATIC mp_obj_t propertyclass_x(mp_obj_t self_in) {
        propertyclass_obj_t *self = MP_OBJ_TO_PTR(self_in);
        return mp_obj_new_float(self->x);
    }
    
    MP_DEFINE_CONST_FUN_OBJ_1(propertyclass_x_obj, propertyclass_x);
    
    STATIC const mp_rom_map_elem_t propertyclass_locals_dict_table[] = {
        { MP_ROM_QSTR(MP_QSTR_x), MP_ROM_PTR(&propertyclass_x_obj) },
    };
    
    STATIC MP_DEFINE_CONST_DICT(propertyclass_locals_dict, propertyclass_locals_dict_table);
    
    STATIC void propertyclass_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination) {
        if(attribute == MP_QSTR_x) {
            destination[0] = propertyclass_x(self_in);
        }
    }
    
    MP_DEFINE_CONST_OBJ_TYPE(
        propertyclass_type,
        MP_QSTR_propertyclass,
        MP_TYPE_FLAG_NONE,
        make_new, propertyclass_make_new,
        attr, propertyclass_attr,
        locals_dict, propertyclass_locals_dict,
    );
    
    STATIC const mp_map_elem_t propertyclass_globals_table[] = {
        { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_propertyclass) },
        { MP_OBJ_NEW_QSTR(MP_QSTR_propertyclass), (mp_obj_t)&propertyclass_type },	
    };
    
    STATIC MP_DEFINE_CONST_DICT (
        mp_module_propertyclass_globals,
        propertyclass_globals_table
    );
    
    const mp_obj_module_t propertyclass_user_cmodule = {
        .base = { &mp_type_module },
        .globals = (mp_obj_dict_t*)&mp_module_propertyclass_globals,
    };
    
    MP_REGISTER_MODULE(MP_QSTR_propertyclass, propertyclass_user_cmodule);

Before we compile the module, I would like to add two comments to what
was said above.

First, in the function that made paired with ``attr``,

.. code:: c

   STATIC void propertyclass_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination) {
       if(attribute == MP_QSTR_x) {
           destination[0] = propertyclass_x(self_in);
       }
   }

we called a function on ``self_in``, ``propertyclass_x()``, and assigned
the results to ``destination[0]``. However, this extra trip is not
absolutely necessary: we could have equally done something along these
lines:

.. code:: c

   STATIC void propertyclass_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination) {
       if(attribute == MP_QSTR_x) {
           propertyclass_obj_t *self = MP_OBJ_TO_PTR(self_in);
           destination[0] = mp_obj_new_float(self->x);
       }
   }

The case in point being that ``destination[0]`` is simply an
``mp_obj_t`` object, it does not matter, where and how it is produced.
Since ``self`` is available to ``propertyclass_attr``, if the property
is simple, as above, one can save the function call, and do everything
in place.

Second, more examples on implementing properties can be found in
`py/profile.c <https://github.com/micropython/micropython/blob/master/py/profile.c>`__.
Just look for the ``attr`` string, and the associated functions!

https://github.com/v923z/micropython-usermod/tree/master/snippets/properties/micropython.mk

.. code:: make
        
    
    USERMODULES_DIR := $(USERMOD_DIR)
    
    # Add all C files to SRC_USERMOD.
    SRC_USERMOD_C += $(USERMODULES_DIR)/properties.c
    
    CFLAGS_USERMOD += -I$(USERMODULES_DIR)
.. code:: bash

    !make clean
    !make USER_C_MODULES=../../../usermod/snippets/properties
.. code ::
        
    %%micropython 
    
    import propertyclass
    a = propertyclass.propertyclass(12.3)
    
    print(a.x)
.. parsed-literal::

    12.3
    
    
