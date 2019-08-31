
micropython internals
=====================

Before exploring the exciting problem of micropython function
implementation in C, we should first understand how python objects are
stored and treated at the firmware level.

Object representation
---------------------

Whenever you write

.. code:: python

   >>> a = 1
   >>> b = 2
   >>> a + b

on the python console, first the two new variables, ``a``, and ``b`` are
created and a reference to them is stored in memory. Then the value of
1, and 2, respectively, will be associated with these variables. In the
last line, when the sum is to be computed, the interpreter somehow has
to figure out, how to decipher the values stored in ``a``, and ``b``: in
the RAM, these two variables are just bytes, but depending on the type
of the variable, different meanings will be associated with these bytes.
Since the type cannot be known at compile time, there must be a
mechanism for keeping stock of this extra piece of information. This is,
where ``mp_obj_t``, defined in ``obj.h``, takes centre stage.

If you cast a cursory glance at any of the C functions that are exposed
to the python interpreter, you will always see something like this

.. code:: c

   mp_obj_t some_function(mp_obj_t some_variable, ...) {
       // some_variable is converted to fundamental C types (bytes, ints, floats, pointers, structures, etc.)
       ...
   }

Variables of type ``mp_obj_t`` are passed to the function, and the
function returns the results as an object of type ``mp_obj_t``. So, what
is all this fuss this about? Basically, ``mp_obj_t`` is nothing but an
8-byte segment of the memory, where all concrete objects are encoded.
There can be various object encodings. E.g., in the ``A`` encoding,
integers are those objects, whose rightmost bit in this 8-byte
representation is set to 1, and the value of the integer can then be
retrieved by shifting these 8 bytes by one to the right, and then
applying a mask. In the ``B`` encoding, the variable is an integer, if
its value is 1, when ANDed with 3, and the value will be returned, if we
shift the 8 bytes by two to the right.

Type checking
-------------

Fortunately, we do not have to be concerned with the representations and
the shifts, because there are pre-defined macros for such operations.
So, if we want to find out, whether ``some_variable`` is an integer, we
can inspect the value of the Boolean

.. code:: c

   MP_OBJ_IS_SMALL_INT(some_variable)

The integer value stored in ``some_variable`` can then be gotten by
calling ``MP_OBJ_SMALL_INT_VALUE``:

.. code:: c

   int value_of_some_variable = MP_OBJ_SMALL_INT_VALUE(some_variable);

These decoding steps take place somewhere in the body of
``some_function``, before we start working with native C types. Once we
are done with the calculations, we have to return an ``mp_obj_t``, so
that the interpreter can handle the results (e.g., show it on the
console, or pipe it to the next instruction). In this case, the encoding
is done by calling

.. code:: c

   mp_obj_new_int(value_of_some_variable)

More generic types can be treated with the macro ``MP_OBJ_IS_TYPE``,
which takes the object as the first, and a pointer to the type as the
second argument. Now, if you want to find out, whether ``some_variable``
is a tuple, you could apply the ``MP_OBJ_IS_TYPE`` macro,

.. code:: c

   MP_OBJ_IS_TYPE(some_variable, &mp_type_tuple)

While the available types can be found in ``obj.h``, they all follow the
``mp_type_`` + python type pattern, so in most cases, it is not even
necessary to look them up. We should also note that it is also possible
to define new types. When done properly, ``MP_OBJ_IS_TYPE`` can be
called on objects with this new type, i.e.,

.. code:: c

   MP_OBJ_IS_TYPE(myobject, &my_type)

will just work. We return to this question later.

python constants
----------------

At this point, we should mention that python constants,\ ``True`` (in C
``mp_const_true``), ``False`` (in C ``mp_const_false``), ``None`` (in C
``mp_const_none``) and the like are also defined in ``obj.h``. These are
objects of type ``mp_obj_t``, as almost anything else, so you can return
them from a function, when the function is meant to return directly to
the interpreter.
