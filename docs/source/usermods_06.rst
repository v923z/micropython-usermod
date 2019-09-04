
Error handling
==============

There will be cases, when something goes wrong, and you want to bail out
in an elegant way. If bailing out, and elegance can be used in the same
sentence, that is. Depending on what kind of difficulty you are facing,
you can indicate this to the user in different ways, and there seems to
be a divide between programmers as to whether one should return an error
code, or do something else.

But in the python world, the most common method is to raise some sort of
exception, and let the user handle the problem. In the following
snippet, we will see a couple of ways of going about exceptions. We
implement a single function that raises an exception, no matter what.
When developing user-friendly code, that is as vicious as you can get, I
guess.

First, the code listing:

https://github.com/v923z/micropython-usermod/tree/master/snippets/sillyerrors/sillyerrors.c

.. code:: cpp
        
    
    #include "py/obj.h"
    #include "py/builtin.h"
    #include "py/runtime.h"
    #include <stdlib.h>
    
    STATIC mp_obj_t mean_function(mp_obj_t error_code) {
        int e = mp_obj_get_int(error_code);
        if(e == 0) {
            mp_raise_msg(&mp_type_ZeroDivisionError, "thou shall not try to divide by 0 on a microcontroller!");
        } else if(e == 1) {
            mp_raise_msg(&mp_type_IndexError, "dude, that was a silly mistake!");
        } else if(e == 2) {
            mp_raise_TypeError("look, chap, you can't be serious!");
        } else if(e == 3) {
            mp_raise_OSError(e);
        } else if(e == 4) {
            char *buffer;
            buffer = malloc(100);
            sprintf(buffer, "you are really out of luck today: error code %d", e);
            mp_raise_NotImplementedError(buffer);
        } else {
            mp_raise_ValueError("sorry, you've exhausted all your options");
        }
        return mp_const_false;
    }
    
    STATIC MP_DEFINE_CONST_FUN_OBJ_1(mean_function_obj, mean_function);
    
    STATIC const mp_rom_map_elem_t sillyerrors_module_globals_table[] = {
        { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_sillyerrors) },
        { MP_ROM_QSTR(MP_QSTR_mean), MP_ROM_PTR(&mean_function_obj) },
    };
    STATIC MP_DEFINE_CONST_DICT(sillyerrors_module_globals, sillyerrors_module_globals_table);
    
    const mp_obj_module_t sillyerrors_user_cmodule = {
        .base = { &mp_type_module },
        .globals = (mp_obj_dict_t*)&sillyerrors_module_globals,
    };
    
    MP_REGISTER_MODULE(MP_QSTR_sillyerrors, sillyerrors_user_cmodule, MODULE_SILLYERRORS_ENABLED);

Now, not all exceptions are created equal. Some are more exceptional
than the others: ``ValueError``, ``TypeError``, ``OSError``, and
``NotImplementedError`` can be raised with the syntax

.. code:: c

   mp_raise_ValueError("wrong value");

which will, in addition to raising the exception at the C level (i.e.,
interrupting the execution of the code), also return a pretty traceback:

::

   Traceback (most recent call last):
     File "<stdin>", line 1, in <module>
   ValueError: wrong value

with the error message that we supplied to the ``mp_raise_ValueError``
function. If you want to have a traceback message that is not a
compile-time constant, you could deal with the problem as in case 4 in
the function ``mean_function``. Such a message might be useful, if the
nature of the exception is somehow related to a quantity that is not
known at compile time, e.g., if you have a function that should not ever
run, if the up-time is shorter than some predefined value. Of course,
one can just say that the “microcontroller hasn’t run long enough yet”,
and this is a pretty good constant string, but perhaps we can give the
user a bit more information, if we can also indicate, how much time is
still missing.

Other exceptions can be raised as in the ``e == 1`` case, with the
``mp_raise_msg(&mp_type_IndexError, "dude, that was a silly mistake!")``
function. Here one also has to specify the type of the exception, which
is always of the form ``mp_type_``. A complete list can be found in
``obj.h``.

Incidentally, ``mp_raise_ValueError``, ``mp_raise_TypeError``, and
``mp_raise_NotImplementedError`` are nothing but a wrapper for
``mp_raise_msg``, which in turn is a wrapper for ``nlr_raise`` of
``nlr.c/nlr.h``. The ``OSError`` is somewhat curious in this respect,
because it is raised directly through ``nlr_raise``, and its argument is
not a string, but an integer error code. All these wrappers are defined
in ``runtime.c``, by the way.

In our ultimate mean function, we raised a lot of exceptions by now, but
we still have to return some value, because the function signature
stipulates that, and the compiler would be unsatisfied otherwise, even
though code execution will actually never reach the return statement.
Since we are in denial mode anyway, I cast my vote for a return value of
``mp_const_false``. ``mp_const_none`` was the other candidate, but ended
up as the runner-up.

I think, it is high time to compile our code.

https://github.com/v923z/micropython-usermod/tree/master/snippets/sillyerrors/micropython.mk

.. code:: make
        
    
    USERMODULES_DIR := $(USERMOD_DIR)
    
    # Add all C files to SRC_USERMOD.
    SRC_USERMOD += $(USERMODULES_DIR)/sillyerrors.c
    
    # We can add our module folder to include paths if needed
    # This is not actually needed in this example.
    CFLAGS_USERMOD += -I$(USERMODULES_DIR)
.. code:: bash

    !make USER_C_MODULES=../../../usermod/snippets/ all
.. code ::
        
    %%micropython
    
    import sillyerrors
    print(sillyerrors.mean(0))
.. parsed-literal::

    
    Traceback (most recent call last):
      File "/dev/shm/micropython.py", line 3, in <module>
    ZeroDivisionError: thou shall not try to divide by 0 on a microcontroller!
    

.. code ::
        
    %%micropython
    
    import sillyerrors
    print(sillyerrors.mean(1))
.. parsed-literal::

    
    Traceback (most recent call last):
      File "/dev/shm/micropython.py", line 3, in <module>
    IndexError: dude, that was a silly mistake!
    

.. code ::
        
    %%micropython
    
    import sillyerrors
    print(sillyerrors.mean(2))
.. parsed-literal::

    
    Traceback (most recent call last):
      File "/dev/shm/micropython.py", line 3, in <module>
    TypeError: look, chap, you can't be serious!
    

.. code ::
        
    %%micropython
    
    import sillyerrors
    print(sillyerrors.mean(3))
.. parsed-literal::

    
    Traceback (most recent call last):
      File "/dev/shm/micropython.py", line 3, in <module>
    OSError: 3
    

.. code ::
        
    %%micropython
    
    import sillyerrors
    print(sillyerrors.mean(4))
.. parsed-literal::

    
    Traceback (most recent call last):
      File "/dev/shm/micropython.py", line 3, in <module>
    NotImplementedError: you are really out of luck today: error code 4
    

One can’t but wonder, why we had to invoke our ``mean`` function in four
separate statements, and why we couldn’t execute everything in a nice
nifty package like

.. code ::
        
    %%micropython
    
    import sillyerrors
    print(sillyerrors.mean(0))
    print(sillyerrors.mean(1))
    print(sillyerrors.mean(2))
    print(sillyerrors.mean(3))
    print(sillyerrors.mean(4))
.. parsed-literal::

    
    Traceback (most recent call last):
      File "/dev/shm/micropython.py", line 3, in <module>
    ZeroDivisionError: you shall not try to divide by 0 on a microcontroller!
    

Well, we could have, but since we specifically raised an exception in
the first statement, our code would never had gotten beyond

.. code:: python

   sillyerror.mean(0)

After all, this is what exceptions do: they interrupt the execution of
the code.
