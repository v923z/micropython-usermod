
Dealing with iterables
======================

Without going too deeply into specifics, in python, an iterable is
basically an object that you can have in a ``for`` loop:

.. code:: python

   for item in my_iterable:
       print(item)

Amongst others, lists, tuples, and ranges are iterables, as are strings.
The key is that these objects have a special internal method, an
iterator, attached to them. This iterator is responsible for keeping
track of the index during the iteration, and serving the objects in the
iterable one by one to the ``for`` loop. When writing our own iterable,
we will look under the hood, and see how this all works at the C level.
For now, we are going to discuss only, how we can *consume* the content
of an iterable in the C code.

Iterating over built-in types
-----------------------------

In order to demonstrate the use of an iterator, we are going to write a
function that sums the square of the values in an iterable. The python
version of the function could be something like this:

.. code::

    def sumsq(some_iterable):
        return sum([item**2 for item in some_iterable])
    
    sumsq([1, 2, 3, 4, 5, 6, 7, 8, 9, 10])




.. parsed-literal::

    385



In C, the key is in the snippet

.. code:: c

   mp_obj_iter_buf_t iter_buf;
   mp_obj_t item, iterable = mp_getiter(o_in, &iter_buf);
   while ((item = mp_iternext(iterable)) != MP_OBJ_STOP_ITERATION) {
       // do something with the item just retrieved
   }

This is more or less the equivalent of the ``for item in some_iterable``
instruction. In C, the ``mp_obj_t`` object ``o_in`` is the argument of
our python function, which is turned into an iterable by passing it into
the ``mp_getiter`` function. This function also needs a buffer object
that is of type ``mp_obj_iter_buf_t``. The buffer type is defined in
``obj.h`` as

.. code:: c

   typedef struct _mp_obj_iter_buf_t {
       mp_obj_base_t base;
       mp_obj_t buf[3];
   } mp_obj_iter_buf_t;

where ``.buf[2]`` holds the index value, and this is how ``mp_iternext``
keeps track of the position in the loop.

Once ``item`` is retrieved, the rest of the code is trivial: you do
whatever you want to do with the value, and return at the very end.

Now, what happens, if you pass a non-iterable object to the function?
For a while, nothing. Everything will work till the point
``item = mp_iternext(iterable)``, where the interpreter will raise a
``TypeError`` exception. So, on the python console, you can either
enclose your function in a

.. code:: python

   try:
       sumsq(some_iterable)
   except TypeError:
       print('something went terribly wrong`)

construct, or you can inspect the type of the variable at the C level.
Unfortunately, there does not seem to be a type identifier for iterables
in general, so you have to check, whether the argument is a list, tuple,
range, etc. This can be done by calling the ``MP_OBJ_IS_TYPE`` macro,
and see which Boolean it returns, if you pass ``&mp_type_tuple``,
``&mp_type_list``, ``&mp_type_range`` etc. to it, as we discussed in the
section `Object representation <#Object-representation>`__.

The complete code listing of ``consumeiterable.c`` follows below. If you
ask me, this is a lot of code just to replace a python one-liner.

https://github.com/v923z/micropython-usermod/tree/master/snippets/consumeiterable/consumeiterable.c

.. code::
        

	#include "py/obj.h"
	#include "py/runtime.h"
	
	STATIC mp_obj_t consumeiterable_sumsq(mp_obj_t o_in) {
	    mp_float_t _sum = 0.0, itemf;
	    mp_obj_iter_buf_t iter_buf;
	    mp_obj_t item, iterable = mp_getiter(o_in, &iter_buf);
	    while ((item = mp_iternext(iterable)) != MP_OBJ_STOP_ITERATION) {
	        itemf = mp_obj_get_float(item);
	        _sum += itemf*itemf;
	    }
	    return mp_obj_new_float(_sum);
	}
	
	STATIC MP_DEFINE_CONST_FUN_OBJ_1(consumeiterable_sumsq_obj, consumeiterable_sumsq);
	
	STATIC const mp_rom_map_elem_t consumeiterable_module_globals_table[] = {
	    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_consumeiterable) },
	    { MP_ROM_QSTR(MP_QSTR_sumsq), MP_ROM_PTR(&consumeiterable_sumsq_obj) },
	};
	STATIC MP_DEFINE_CONST_DICT(consumeiterable_module_globals, consumeiterable_module_globals_table);
	
	const mp_obj_module_t consumeiterable_user_cmodule = {
	    .base = { &mp_type_module },
	    .globals = (mp_obj_dict_t*)&consumeiterable_module_globals,
	};
	
	MP_REGISTER_MODULE(MP_QSTR_consumeiterable, consumeiterable_user_cmodule, MODULE_CONSUMEITERABLE_ENABLED);

.. parsed-literal::

    written 1272 bytes to /consumeiterable/consumeiterable.c


.. code::

    %%makefile /consumeiterable/consumeiterable.c
    
    USERMODULES_DIR := $(USERMOD_DIR)
    
    # Add all C files to SRC_USERMOD.
    SRC_USERMOD += $(USERMODULES_DIR)/consumeiterable.c
    
    # We can add our module folder to include paths if needed
    # This is not actually needed in this example.
    CFLAGS_USERMOD += -I$(USERMODULES_DIR)

.. code::

    !make USER_C_MODULES=../../../usermod/snippets/ all

.. code::

    %%micropython
    
    import consumeiterable
    
    a = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
    print(a)
    print(consumeiterable.sumsq(a))


.. parsed-literal::

    [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
    385.0
    
    


Returning iterables
-------------------

Let us suppose that the result of some operation is an iterable, e.g., a
tuple, or a list. How would we return such an object? How about a
function that returns the powers of its argument? In python

.. code::

    def powerit(base, exponent):
        return [base**e for e in range(0, exponent+1)]
    
    powerit(2, 10)




.. parsed-literal::

    [1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024]



and in C,

https://github.com/v923z/micropython-usermod/tree/master/snippets/returniterable/returniterable.c

.. code::
        

	#include "py/obj.h"
	#include "py/runtime.h"
	
	STATIC mp_obj_t powers_iterable(mp_obj_t base, mp_obj_t exponent) {
	    int e = mp_obj_get_int(exponent);
	    mp_obj_t tuple[e+1];
	    int b = mp_obj_get_int(base), ba = 1;
	    for(int i=0; i <= e; i++) {
	        tuple[i] = mp_obj_new_int(ba);
	        ba *= b;
	    }
	    return mp_obj_new_tuple(e+1, tuple);
	}
	
	STATIC MP_DEFINE_CONST_FUN_OBJ_2(powers_iterable_obj, powers_iterable);
	
	STATIC const mp_rom_map_elem_t returniterable_module_globals_table[] = {
	    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_returniterable) },
	    { MP_ROM_QSTR(MP_QSTR_powers), MP_ROM_PTR(&powers_iterable_obj) },
	};
	STATIC MP_DEFINE_CONST_DICT(returniterable_module_globals, returniterable_module_globals_table);
	
	const mp_obj_module_t returniterable_user_cmodule = {
	    .base = { &mp_type_module },
	    .globals = (mp_obj_dict_t*)&returniterable_module_globals,
	};
	
	MP_REGISTER_MODULE(MP_QSTR_returniterable, returniterable_user_cmodule, MODULE_RETURNITERABLE_ENABLED);

.. parsed-literal::

    written 1194 bytes to /returniterable/returniterable.c


As everything else, the elements of tuples and lists are objects of type
``mp_obj_t``, so, after finding out how far we have got to go with the
exponents, we declare an array of the required length. Values are
generated and assigned in the ``for`` loop. Since on the left hand side
of the assignment we have an ``mp_obj_t``, we convert the results with
``mp_obj_new_int``. Once we are done with the computations, we return
the array with ``mp_obj_new_tuple``. This functions takes the array as
the second argument, while the first argument specifies the length.

If you happen to want to return a list instead of a tuple, all you have
to do is use ``mp_obj_new_list`` instead at the very end.

.. code::

    %%makefile /returniterable/returniterable.c
    
    USERMODULES_DIR := $(USERMOD_DIR)
    
    # Add all C files to SRC_USERMOD.
    SRC_USERMOD += $(USERMODULES_DIR)/returniterable.c
    
    # We can add our module folder to include paths if needed
    # This is not actually needed in this example.
    CFLAGS_USERMOD += -I$(USERMODULES_DIR)

.. code::

    !make USER_C_MODULES=../../../usermod/snippets/ all

.. code::

    %%micropython
    
    import returniterable
    print(returniterable.powers(3, 10))


.. parsed-literal::

    (1, 3, 9, 27, 81, 243, 729, 2187, 6561, 19683, 59049)
    
    


Creating iterables
------------------

Having seen how we can consume the elements in an iterable, it is time
to explore what this ``.getiter`` magic is doing. So, let us create a
new type, ``itarray``, and make it iterable! This new type will have a
constructor method,\ ``square``, generating 16-bit integers, where the
values are simply the squares of the indices, i.e., 1, 4, 9, 16… We are
interested only in the iterability of the object, and for this reason,
we will implement only the ``.getiter`` special method, and skip
``.binary_op``, and ``.unary_op``. If needed, these can easily be added
based on the discussion in Special methods of classes.

Before listing the complete code, we discuss the relevant code snippets.
The first chunk is the assignment of ``.getiter`` in the
``iterable_array_type`` structure. ``.getiter`` will be made equal to a
function called ``iterarray_getiter``, which simply returns
``mp_obj_new_itarray_iterator``. Why can’t we simply assign
``mp_obj_new_itarray_iterator``, instead of wrapping it in
``iterarray_getiter``? The reason for that is that ``iterarray_getiter``
has a strict signature, and we want to pass an extra argument, 0. This
is nothing but the very first index in the sequence.

.. code:: c

   STATIC mp_obj_t itarray_getiter(mp_obj_t o_in, mp_obj_iter_buf_t *iter_buf) {
       return mp_obj_new_itarray_iterator(o_in, 0, iter_buf);
   }

   const mp_obj_type_t iterable_array_type = {
       { &mp_type_type },
       .name = MP_QSTR_itarray,
       .print = itarray_print,
       .make_new = itarray_make_new,
       .getiter = itarray_getiter,
   };

So, it appears that we have to scrutinise
``mp_obj_new_itarray_iterator``. This is a special object type in
micropython, with a base type of ``mp_type_polymorph_iter``. In
addition, it holds a pointer to the ``__next__`` method, which is
``itarray_iternext`` in this case, stores a pointer to the variable (the
one that we are iterating over), and the current index (which we
initialised to 0 in ``mp_obj_new_itarray_iterator``).

.. code:: c

   mp_obj_t mp_obj_new_itarray_iterator(mp_obj_t itarray, size_t cur, mp_obj_iter_buf_t *iter_buf) {
       assert(sizeof(mp_obj_itarray_it_t) <= sizeof(mp_obj_iter_buf_t));
       mp_obj_itarray_it_t *o = (mp_obj_itarray_it_t*)iter_buf;
       o->base.type = &mp_type_polymorph_iter;
       o->iternext = itarray_iternext;
       o->itarray = itarray;
       o->cur = cur;
       return MP_OBJ_FROM_PTR(o);
   }

``mp_obj_new_itarray_iterator`` is not much more than a declaration and
assignments. The object that we return is of type
``mp_obj_itarray_it_t``, which has the above-mentioned structure

.. code:: c

   // itarray iterator
   typedef struct _mp_obj_itarray_it_t {
       mp_obj_base_t base;
       mp_fun_1_t iternext;
       mp_obj_t itarray;
       size_t cur;
   } mp_obj_itarray_it_t;

   mp_obj_t itarray_iternext(mp_obj_t self_in) {
       mp_obj_itarray_it_t *self = MP_OBJ_TO_PTR(self_in);
       itarray_obj_t *itarray = MP_OBJ_TO_PTR(self->itarray);
       if (self->cur < itarray->len) {
           // read the current value
           uint16_t *arr = itarray->elements;
           mp_obj_t o_out = MP_OBJ_NEW_SMALL_INT(arr[self->cur]);
           self->cur += 1;
           return o_out;
       } else {
           return MP_OBJ_STOP_ITERATION;
       }
   }

Now, the complete code in one chunk:

https://github.com/v923z/micropython-usermod/tree/master/snippets/makeiterable/makeiterable.c

.. code::
        

	#include <stdlib.h>
	#include "py/obj.h"
	#include "py/runtime.h"
	
	typedef struct _itarray_obj_t {
	    mp_obj_base_t base;
	    mp_fun_1_t iternext;
	    uint16_t *elements;
	    size_t len;
	} itarray_obj_t;
	
	const mp_obj_type_t iterable_array_type;
	mp_obj_t mp_obj_new_itarray_iterator(mp_obj_t , size_t , mp_obj_iter_buf_t *);
	
	STATIC void itarray_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
	    (void)kind;
	    itarray_obj_t *self = MP_OBJ_TO_PTR(self_in);
	    printf("itarray: ");
	    for(uint16_t i=0; i < self->len; i++) {
	        printf("%d ", self->elements[i]);
	    }
	    printf("\n");
	}
	
	STATIC mp_obj_t itarray_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
	    mp_arg_check_num(n_args, n_kw, 1, 1, true);
	    itarray_obj_t *self = m_new_obj(itarray_obj_t);
	    self->base.type = &iterable_array_type;
	    self->len = mp_obj_get_int(args[0]);
	    uint16_t *arr = malloc(self->len * sizeof(uint16_t));
	    for(uint16_t i=0; i < self->len; i++) {
	        arr[i] = i*i;
	    }
	    self->elements = arr;
	    return MP_OBJ_FROM_PTR(self);
	}
	
	STATIC mp_obj_t itarray_getiter(mp_obj_t o_in, mp_obj_iter_buf_t *iter_buf) {
	    return mp_obj_new_itarray_iterator(o_in, 0, iter_buf);
	}
	
	const mp_obj_type_t iterable_array_type = {
	    { &mp_type_type },
	    .name = MP_QSTR_itarray,
	    .print = itarray_print,
	    .make_new = itarray_make_new,
	    .getiter = itarray_getiter,
	};
	
	STATIC const mp_rom_map_elem_t makeiterable_module_globals_table[] = {
	    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_makeiterable) },
	    { MP_OBJ_NEW_QSTR(MP_QSTR_square), (mp_obj_t)&iterable_array_type },	
	};
	STATIC MP_DEFINE_CONST_DICT(makeiterable_module_globals, makeiterable_module_globals_table);
	
	const mp_obj_module_t makeiterable_user_cmodule = {
	    .base = { &mp_type_module },
	    .globals = (mp_obj_dict_t*)&makeiterable_module_globals,
	};
	
	MP_REGISTER_MODULE(MP_QSTR_makeiterable, makeiterable_user_cmodule, MODULE_MAKEITERABLE_ENABLED);
	
	// itarray iterator
	typedef struct _mp_obj_itarray_it_t {
	    mp_obj_base_t base;
	    mp_fun_1_t iternext;
	    mp_obj_t itarray;
	    size_t cur;
	} mp_obj_itarray_it_t;
	
	mp_obj_t itarray_iternext(mp_obj_t self_in) {
	    mp_obj_itarray_it_t *self = MP_OBJ_TO_PTR(self_in);
	    itarray_obj_t *itarray = MP_OBJ_TO_PTR(self->itarray);
	    if (self->cur < itarray->len) {
	        // read the current value
	        uint16_t *arr = itarray->elements;
	        mp_obj_t o_out = MP_OBJ_NEW_SMALL_INT(arr[self->cur]);
	        self->cur += 1;
	        return o_out;
	    } else {
	        return MP_OBJ_STOP_ITERATION;
	    }
	}
	
	mp_obj_t mp_obj_new_itarray_iterator(mp_obj_t itarray, size_t cur, mp_obj_iter_buf_t *iter_buf) {
	    assert(sizeof(mp_obj_itarray_it_t) <= sizeof(mp_obj_iter_buf_t));
	    mp_obj_itarray_it_t *o = (mp_obj_itarray_it_t*)iter_buf;
	    o->base.type = &mp_type_polymorph_iter;
	    o->iternext = itarray_iternext;
	    o->itarray = itarray;
	    o->cur = cur;
	    return MP_OBJ_FROM_PTR(o);
	}



.. parsed-literal::

    written code to /makeiterable/makeiterable.c


.. code::

    %%micropython
    
    import makeiterable
    
    a = makeiterable.square(15)
    print(a)
    for j, i in enumerate(a):
        if j == 1: print('%dst element: %d'%(j, i))
        elif j == 2: print('%dnd element: %d'%(j, i))
        elif j == 3: print('%drd element: %d'%(j, i))
        else:
            print('%dth element: %d'%(j, i))


.. parsed-literal::

    itarray: 0 1 4 9 16 25 36 49 64 81 100 121 144 169 196 
    
    0th element: 0
    1st element: 1
    2nd element: 4
    3rd element: 9
    4th element: 16
    5th element: 25
    6th element: 36
    7th element: 49
    8th element: 64
    9th element: 81
    10th element: 100
    11th element: 121
    12th element: 144
    13th element: 169
    14th element: 196
    


Subscripts
----------

We now know, how we construct something that can be passed to a ``for``
loop. This is a good start. But iterables have other very useful
properties. For instance, have you ever wondered, what actually happens
in the following snippet?

.. code::

    a = 'micropython'
    a[5]




.. parsed-literal::

    'p'



``a`` is a string, therefore, an iterable. Where does the interpreter
know from, that it has got to return ``p``, when asked for ``a[5]``? Or
have you ever been curious to know, how the interpreter replaces ``p``
by ``q``, if

.. code::

    a = [c for c in 'micropyton']
    a[5] = 'q'
    a




.. parsed-literal::

    ['m', 'i', 'c', 'r', 'o', 'q', 'y', 't', 'o', 'n']



is passed to it? If so, then it is your lucky day, because we are going
to make our iterable class be able to deal with such requests.

The code snippets above rely on a single special method, the
subscription. In the C code of micropython, this method is called
``.subscr``, and it should be assigned to in the class declaration,
i.e., if we take ``makeiterable.c`` as our basis for the following
discussion, then we would have to extend the ``iterable_array_type`` as

.. code:: c

   const mp_obj_type_t iterable_array_type {
       ...
       .subscr = itarray_subscr
   }

where the signature of ``itarray_subscr`` has the form

.. code:: c

   STATIC mp_obj_t itarray_subscr(mp_obj_t self_in, mp_obj_t index, mp_obj_t value)

If ``.subscr`` is not implemented, but you are daring enough to call

.. code:: python

   >>> a[5]

all the same, then the interpreter is going to throw a ``TypeError``:

.. code:: pytb

   Traceback (most recent call last):
     File "<stdin>", line 1, in <module>
   TypeError: 'itarray' object isn't subscriptable

So, what happens in the method that we assigned in
``iterable_array_type``? A possible scenario is given below:

.. code:: c

   STATIC mp_obj_t subitarray_subscr(mp_obj_t self_in, mp_obj_t index, mp_obj_t value) {
       subitarray_obj_t *self = MP_OBJ_TO_PTR(self_in);
       size_t idx = mp_obj_get_int(index);
       if(self->len <= idx) {
           mp_raise_ValueError("index is out of range");
       }
       if (value == MP_OBJ_SENTINEL) { // simply return the value at index, no assignment      
           return MP_OBJ_NEW_SMALL_INT(self->elements[idx]);
       } else { // value was passed, replace the element at index
           self->elements[idx] = mp_obj_get_int(value);
       }
       return mp_const_none;
   }

``subitarray_subscr`` takes three arguments: the first is the instance
on which the method is called, i.e., ``self``. The second is the index,
i.e., what stands in []. And finally, the third argument is the value.
This is what we assign to the element at index ``idx``, or, when we do
not assign anything (i.e., when we *load* a value from the iterable),
then ``value`` takes on a special value. If we have

.. code:: python

   >>> a[5]

on the python console, then the interpreter will automatically assign
``value = MP_OBJ_SENTINEL`` (this is defined in ``obj.h``), so that,
though we did not explicitly set anything to it, we can still inspect
``value``. This is what happens, when we evaluate
``value == MP_OBJ_SENTINEL``: if this statement is true, then we query
for ``a[5]``. Note that we also implemented some very rudimentary error
checking: we raise an ``IndexError``, whenever the index is out of
range. We do this by calling

.. code:: c

   mp_raise_msg(&mp_type_IndexError, "index is out of range");

For a thorough discussion on how to raise exceptions see the Section
`Error handling <#Error-handling>`__.

There is one more thing that we should notice: at the very beginning of
the function, in the line

.. code:: c

   size_t idx = mp_obj_get_int(index);

we call ``mp_obj_get_int``. This means that any python object with an
integer value is a valid argument, i.e., the following instruction would
still work

.. code::

    %%micropython
    
    a = 'micropython'
    b = 5
    print(a[b])


.. parsed-literal::

    p
    
    


For compiling, here is the complete code:

https://github.com/v923z/micropython-usermod/tree/master/snippets/subscriptiterable/subscriptiterable.c

.. code::
        

	#include <stdlib.h>
	#include "py/obj.h"
	#include "py/runtime.h"
	
	typedef struct _subitarray_obj_t {
	    mp_obj_base_t base;
	    mp_fun_1_t iternext;
	    uint16_t *elements;
	    size_t len;
	} subitarray_obj_t;
	
	const mp_obj_type_t subiterable_array_type;
	mp_obj_t mp_obj_new_subitarray_iterator(mp_obj_t , size_t , mp_obj_iter_buf_t *);
	
	STATIC void subitarray_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
	    (void)kind;
	    subitarray_obj_t *self = MP_OBJ_TO_PTR(self_in);
	    printf("subitarray: ");
	    for(uint16_t i=0; i < self->len; i++) {
	        printf("%d ", self->elements[i]);
	    }
	    printf("\n");
	}
	
	STATIC mp_obj_t subitarray_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
	    mp_arg_check_num(n_args, n_kw, 1, 1, true);
	    subitarray_obj_t *self = m_new_obj(subitarray_obj_t);
	    self->base.type = &subiterable_array_type;
	    self->len = mp_obj_get_int(args[0]);
	    uint16_t *arr = malloc(self->len * sizeof(uint16_t));
	    for(uint16_t i=0; i < self->len; i++) {
	        arr[i] = i*i;
	    }
	    self->elements = arr;
	    return MP_OBJ_FROM_PTR(self);
	}
	
	STATIC mp_obj_t subitarray_getiter(mp_obj_t o_in, mp_obj_iter_buf_t *iter_buf) {
	    return mp_obj_new_subitarray_iterator(o_in, 0, iter_buf);
	}
	
	STATIC mp_obj_t subitarray_subscr(mp_obj_t self_in, mp_obj_t index, mp_obj_t value) {
	    subitarray_obj_t *self = MP_OBJ_TO_PTR(self_in);
	    size_t idx = mp_obj_get_int(index);
	    if(self->len <= idx) {
	        mp_raise_msg(&mp_type_IndexError, "index is out of range");
	    }
	    if (value == MP_OBJ_SENTINEL) { // simply return the value at index, no assignment
	        return MP_OBJ_NEW_SMALL_INT(self->elements[idx]);
	    } else { // value was passed, replace the element at index
	        self->elements[idx] = mp_obj_get_int(value);
	    }
	    return mp_const_none;
	}
	
	const mp_obj_type_t subiterable_array_type = {
	    { &mp_type_type },
	    .name = MP_QSTR_subitarray,
	    .print = subitarray_print,
	    .make_new = subitarray_make_new,
	    .getiter = subitarray_getiter,
	    .subscr = subitarray_subscr,
	};
	
	STATIC const mp_rom_map_elem_t subscriptiterable_module_globals_table[] = {
	    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_subscriptiterable) },
	    { MP_OBJ_NEW_QSTR(MP_QSTR_square), (mp_obj_t)&subiterable_array_type },
	};
	STATIC MP_DEFINE_CONST_DICT(subscriptiterable_module_globals, subscriptiterable_module_globals_table);
	
	const mp_obj_module_t subscriptiterable_user_cmodule = {
	    .base = { &mp_type_module },
	    .globals = (mp_obj_dict_t*)&subscriptiterable_module_globals,
	};
	
	MP_REGISTER_MODULE(MP_QSTR_subscriptiterable, subscriptiterable_user_cmodule, MODULE_SUBSCRIPTITERABLE_ENABLED);
	
	// itarray iterator
	typedef struct _mp_obj_subitarray_it_t {
	    mp_obj_base_t base;
	    mp_fun_1_t iternext;
	    mp_obj_t subitarray;
	    size_t cur;
	} mp_obj_subitarray_it_t;
	
	mp_obj_t subitarray_iternext(mp_obj_t self_in) {
	    mp_obj_subitarray_it_t *self = MP_OBJ_TO_PTR(self_in);
	    subitarray_obj_t *subitarray = MP_OBJ_TO_PTR(self->subitarray);
	    if (self->cur < subitarray->len) {
	        // read the current value
	        uint16_t *arr = subitarray->elements;
	        mp_obj_t o_out = MP_OBJ_NEW_SMALL_INT(arr[self->cur]);
	        self->cur += 1;
	        return o_out;
	    } else {
	        return MP_OBJ_STOP_ITERATION;
	    }
	}
	
	mp_obj_t mp_obj_new_subitarray_iterator(mp_obj_t subitarray, size_t cur, mp_obj_iter_buf_t *iter_buf) {
	    assert(sizeof(mp_obj_subitarray_it_t) <= sizeof(mp_obj_iter_buf_t));
	    mp_obj_subitarray_it_t *o = (mp_obj_subitarray_it_t*)iter_buf;
	    o->base.type = &mp_type_polymorph_iter;
	    o->iternext = subitarray_iternext;
	    o->subitarray = subitarray;
	    o->cur = cur;
	    return MP_OBJ_FROM_PTR(o);
	}

.. parsed-literal::

    written 3962 bytes to /subscriptiterable/subscriptiterable.c


.. code::

    %%makefile /subscriptiterable/subscriptiterable.c
    
    USERMODULES_DIR := $(USERMOD_DIR)
    
    # Add all C files to SRC_USERMOD.
    SRC_USERMOD += $(USERMODULES_DIR)/subscriptiterable.c
    
    # We can add our module folder to include paths if needed
    # This is not actually needed in this example.
    CFLAGS_USERMOD += -I$(USERMODULES_DIR)

.. code::

    !make USER_C_MODULES=../../../usermod/snippets/ all > /dev/null

.. code::

    %%micropython
    
    import subscriptiterable
    a = subscriptiterable.square(15)
    print(a)
    print('the third element is %d'%a[3])
    b = 3+7
    a[b] = 0
    print(a)


.. parsed-literal::

    subitarray: 0 1 4 9 16 25 36 49 64 81 100 121 144 169 196 
    
    the third element is 9
    subitarray: 0 1 4 9 16 25 36 49 64 81 0 121 144 169 196 
    
    
    


Index reversing
~~~~~~~~~~~~~~~

Now, the code above works for non-negative indices, but in python it is
quite customary to have something like

.. code::

    a = 'micropython'
    a[-2]




.. parsed-literal::

    'o'



which is equivalent to querying for the last but one element (second
from the right) in the iterable. Knowing how long the iterable is (this
is stored in ``self->len``), it is a trivial matter to modify our code
in such a way that it can return the values at negative indices.

Slicing
-------

In the previous two sections we have worked with single elements of an
iterable. But python wouldn’t be python without slices. Slices are index
ranges specified in a ``start:end:step`` format. Taking our earlier
example, we can print every second character in ``micropython`` by

.. code::

    a = 'micropython'
    a[0:8:2]




.. parsed-literal::

    'mcoy'



This behaviour is also part of the ``.subscr`` special method. Let us
implement it, shall we? In order to simplify the discussion, we will
treat one case only: returning values, and we return a new instance of
the array, if a slice was requested, while a single number, if we passed
a single index.

Since we want to return an array if the indices stem from a slice, we
split our original ``subscriptitarray_make_new`` function, and separate
those parts that reserve space for the array from those that do the
assignments.

It shouldn’t come as a surprise that we have to modify the function that
was hooked up to ``.subscr``. Let us take a look at the following
snippet:

.. code:: c

   STATIC mp_obj_t sliceitarray_subscr(mp_obj_t self_in, mp_obj_t index, mp_obj_t value) {
       sliceitarray_obj_t *self = MP_OBJ_TO_PTR(self_in);
       if (value == MP_OBJ_SENTINEL) { // simply return the values at index, no assignment

   #if MICROPY_PY_BUILTINS_SLICE
           if (MP_OBJ_IS_TYPE(index, &mp_type_slice)) {
               mp_bound_slice_t slice;
               mp_seq_get_fast_slice_indexes(self->len, index, &slice);
               uint16_t len = (slice.stop - slice.start) / slice.step;
               sliceitarray_obj_t *res = create_new_sliceitarray(len);
               for(size_t i=0; i < len; i++) {
                   res->elements[i] = self->elements[slice.start+i*slice.step];
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

As advertised, we treat only the case, when ``value`` is empty, i.e., it
is equal to an ``MP_OBJ_SENTINEL``. Now, there is no point in trying to
read out the parameters of a slice, if the slice object is not even
defined, is there? This is the case for the minimal ports. So, in order
to prevent nasty things from happening, we insert the ``#if/#endif``
macro with the parameter ``MICROPY_PY_BUILTINS_SLICE``. Provided that
``MICROPY_PY_BUILTINS_SLICE`` is defined, we inspect the index, and find
out if it is a slice by calling

.. code:: c

   MP_OBJ_IS_TYPE(index, &mp_type_slice)

If so, we attempt to load the slice parameters into the ``slice`` object
with

.. code:: c

   mp_seq_get_fast_slice_indexes(self->len, index, &slice)

The function ``mp_seq_get_fast_slice_indexes`` returns Boolean ``true``,
if the increment in the slice is 1, and ``false`` otherwise. For the
goal that we are trying to pursue here, it doesn’t matter what the step
size is, so we don’t care about the return value. But the main purpose
of the function is actually something different: the function expands
the ``start:end:step`` slice into a triplet, and it does so, even if one
or two of the slice parameters are missing. So, ``start::step``,
``start::``, ``:end:step`` etc. will also work. In fact, this is why we
have to pass the length of the array: ``self->len`` will be substituted,
if the ``:end:`` parameter is missing.

Equipped with the values of ``slice.start``, ``slice.stop``, and
``slice.step``, we can determine the length of the new array, and assign
the values in the ``for`` loop.

https://github.com/v923z/micropython-usermod/tree/master/snippets/sliceiterable/sliceiterable.c

.. code::
        

	#include <stdlib.h>
	#include "py/obj.h"
	#include "py/runtime.h"
	
	typedef struct _sliceitarray_obj_t {
	    mp_obj_base_t base;
	    mp_fun_1_t iternext;
	    uint16_t *elements;
	    size_t len;
	} sliceitarray_obj_t;
	
	const mp_obj_type_t sliceiterable_array_type;
	mp_obj_t mp_obj_new_sliceitarray_iterator(mp_obj_t , size_t , mp_obj_iter_buf_t *);
	
	STATIC void sliceitarray_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
	    (void)kind;
	    sliceitarray_obj_t *self = MP_OBJ_TO_PTR(self_in);
	    printf("sliceitarray: ");
	    for(uint16_t i=0; i < self->len; i++) {
	        printf("%d ", self->elements[i]);
	    }
	    printf("\n");
	}
	
	sliceitarray_obj_t *create_new_sliceitarray(uint16_t len) {
	    sliceitarray_obj_t *self = m_new_obj(sliceitarray_obj_t);
	    self->base.type = &sliceiterable_array_type;
	    self->len = len;
	    uint16_t *arr = malloc(self->len * sizeof(uint16_t));
	    self->elements = arr;
	    return self;
	}
	
	STATIC mp_obj_t sliceitarray_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
	    mp_arg_check_num(n_args, n_kw, 1, 1, true);
	    sliceitarray_obj_t *self = create_new_sliceitarray(mp_obj_get_int(args[0]));
	    for(uint16_t i=0; i < self->len; i++) {
	        self->elements[i] = i*i;
	    }
	    return MP_OBJ_FROM_PTR(self);
	}
	
	STATIC mp_obj_t sliceitarray_getiter(mp_obj_t o_in, mp_obj_iter_buf_t *iter_buf) {
	    return mp_obj_new_sliceitarray_iterator(o_in, 0, iter_buf);
	}
	
	STATIC mp_obj_t sliceitarray_subscr(mp_obj_t self_in, mp_obj_t index, mp_obj_t value) {
	    sliceitarray_obj_t *self = MP_OBJ_TO_PTR(self_in);
	    if (value == MP_OBJ_SENTINEL) { // simply return the values at index, no assignment
	
	#if MICROPY_PY_BUILTINS_SLICE
	        if (MP_OBJ_IS_TYPE(index, &mp_type_slice)) {
	            mp_bound_slice_t slice;
	            mp_seq_get_fast_slice_indexes(self->len, index, &slice);
	            printf("start: %ld, stop: %ld, step: %ld\n", slice.start, slice.stop, slice.step);
	            uint16_t len = (slice.stop - slice.start) / slice.step;
	            sliceitarray_obj_t *res = create_new_sliceitarray(len);
	            for(size_t i=0; i < len; i++) {
	                res->elements[i] = self->elements[slice.start+i*slice.step];
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
	
	const mp_obj_type_t sliceiterable_array_type = {
	    { &mp_type_type },
	    .name = MP_QSTR_sliceitarray,
	    .print = sliceitarray_print,
	    .make_new = sliceitarray_make_new,
	    .getiter = sliceitarray_getiter,
	    .subscr = sliceitarray_subscr,
	};
	
	STATIC const mp_rom_map_elem_t sliceiterable_module_globals_table[] = {
	    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_sliceiterable) },
	    { MP_OBJ_NEW_QSTR(MP_QSTR_square), (mp_obj_t)&sliceiterable_array_type },
	};
	STATIC MP_DEFINE_CONST_DICT(sliceiterable_module_globals, sliceiterable_module_globals_table);
	
	const mp_obj_module_t sliceiterable_user_cmodule = {
	    .base = { &mp_type_module },
	    .globals = (mp_obj_dict_t*)&sliceiterable_module_globals,
	};
	
	MP_REGISTER_MODULE(MP_QSTR_sliceiterable, sliceiterable_user_cmodule, MODULE_SLICEITERABLE_ENABLED);
	
	// itarray iterator
	typedef struct _mp_obj_sliceitarray_it_t {
	    mp_obj_base_t base;
	    mp_fun_1_t iternext;
	    mp_obj_t sliceitarray;
	    size_t cur;
	} mp_obj_sliceitarray_it_t;
	
	mp_obj_t sliceitarray_iternext(mp_obj_t self_in) {
	    mp_obj_sliceitarray_it_t *self = MP_OBJ_TO_PTR(self_in);
	    sliceitarray_obj_t *sliceitarray = MP_OBJ_TO_PTR(self->sliceitarray);
	    if (self->cur < sliceitarray->len) {
	        // read the current value
	        uint16_t *arr = sliceitarray->elements;
	        mp_obj_t o_out = MP_OBJ_NEW_SMALL_INT(arr[self->cur]);
	        self->cur += 1;
	        return o_out;
	    } else {
	        return MP_OBJ_STOP_ITERATION;
	    }
	}
	
	mp_obj_t mp_obj_new_sliceitarray_iterator(mp_obj_t sliceitarray, size_t cur, mp_obj_iter_buf_t *iter_buf) {
	    assert(sizeof(mp_obj_sliceitarray_it_t) <= sizeof(mp_obj_iter_buf_t));
	    mp_obj_sliceitarray_it_t *o = (mp_obj_sliceitarray_it_t*)iter_buf;
	    o->base.type = &mp_type_polymorph_iter;
	    o->iternext = sliceitarray_iternext;
	    o->sliceitarray = sliceitarray;
	    o->cur = cur;
	    return MP_OBJ_FROM_PTR(o);
	}

.. parsed-literal::

    written 4698 bytes to /sliceiterable/sliceiterable.c


.. code::

    %%makefile /sliceiterable/sliceiterable.c
    
    USERMODULES_DIR := $(USERMOD_DIR)
    
    # Add all C files to SRC_USERMOD.
    SRC_USERMOD += $(USERMODULES_DIR)/sliceiterable.c
    
    # We can add our module folder to include paths if needed
    # This is not actually needed in this example.
    CFLAGS_USERMOD += -I$(USERMODULES_DIR)

.. code::

    !make USER_C_MODULES=../../../usermod/snippets/ all

.. code::

    %%micropython 
    
    import sliceiterable
    a = sliceiterable.square(20)
    
    print(a)
    print(a[1:15:3])


.. parsed-literal::

    sliceitarray: 0 1 4 9 16 25 36 49 64 81 100 121 144 169 196 225 256 289 324 361 
    
    start: 1, stop: 15, step: 3
    sliceitarray: 1 16 49 100 
    
    
    


A word of caution is in order here: if the step size is negative, the
array is reversed. This means that ``slice.start`` is larger than
``slice.stop``, and when we calculate the length of the new array, we
end up with a negative number. Just saying.
