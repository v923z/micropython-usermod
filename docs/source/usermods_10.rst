
Profiling
=========

There are times, when you might want to find out what resources (time
and RAM) a particular operation requires. Not because you are nosy, but
because the resources of a microcontroller are limited, therefore, if
you are out of luck, the desired operation might not even fit within the
constraints of the chip. In order to locate the bottleneck, you will
need to do a bit of profiling. Or perhaps, a lot. This is what we are
going to discuss now.

Since you are not going to face serious difficulties when running
micropython on a computer, profiling makes really sense only in the
context of the microcontroller, so this might be a golden opportunity to
brush up on how the firmware has to be compiled and uploaded. It is not
by accident that we spent some time on this at the very beginning of
this document.

Profiling in python
-------------------

Measuring time
~~~~~~~~~~~~~~

If you are interested in the execution time of a complete function, you
can measure it simply by making use of the python interpreter

.. code::

    %%micropython
    
    from utime import ticks_us, ticks_diff
    
    def test_function(n):
        for i in range(n):
            q = i*i*i
        return q # return the last 
    
    now = ticks_us()
    test_function(100)
    then = ticks_diff(ticks_us(), now)
    
    print("function test_function() took %d us to run"%then)


.. parsed-literal::

    function test_function() took 27 us to run
    


In fact, since our function is flanked by two other statements, this
construct easily lends itself to a decorator implementation, as in
(taken from
http://docs.micropython.org/en/v1.9.3/pyboard/reference/speed_python.html)

.. code:: python

   def timed_function(f, *args, **kwargs):
       myname = str(f).split(' ')[1]
       def new_func(*args, **kwargs):
           t = utime.ticks_us()
           result = f(*args, **kwargs)
           delta = utime.ticks_diff(utime.ticks_us(), t)
           print('Function {} Time = {:6.3f}ms'.format(myname, delta/1000))
           return result
       return new_func

(If you need an even better estimate, you can get the ticks twice, and
yank ``run_my_function()`` in the second pass: in this way, you would
get the cost of measuring time itself:

.. code:: python


   from utime import ticks_us, ticks_diff

   now = ticks_us
   then = ticks_diff(ticks_us(), now)

   print("the time measurement took %d us"%then)

Then you subtract the results of the second measurement from those of
the first.)

The memory cost of a function
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

While time is money, RAM is gold. We shouldn’t pass up on that! The
``micropython`` has a very handy function for printing a pretty summary
of the state of the RAM. You would call it like

.. code::

    %%micropython
    
    import micropython
    print(micropython.mem_info())


.. parsed-literal::

    mem: total=2755, current=663, peak=2289
    stack: 928 out of 80000
    GC: total: 2072832, used: 704, free: 2072128
     No. of 1-blocks: 6, 2-blocks: 3, max blk sz: 6, max free sz: 64745
    None
    


If you call ``mem_info()`` after you executed your function, but before
calling the garbage collector (if that is enabled, that is), then from
the two reports, you can figure out how many bytes the function has
eaten.

Profiling in C
--------------

With the profiling method above, we can measure the cost of a complete
function only, but we cannot say anything about individual instructions
in the body. Execution time is definitely a significant issue, but even
worse is the problem of RAM: it might happen that the function allocates
a huge amount of memory, but cleans up properly before returning. Such a
function could certainly wreak havoc, even if it is rather
inocuous-looking from the outside. So, what do we do? We should probably
just measure. It is not going to hurt.

In the example below (``profiling.c``), I discuss both time and RAM
measurements in a single module, because splitting them wouldn’t be
worth the trouble. The function whose behaviour we inspect does nothing,
but calculate the length of a three-dimensional vector. With that, we
can figure out, how much the assignment, and how much the actual
calculation cost.

https://github.com/v923z/micropython-usermod/tree/master/snippets/profiling/profiling.c

.. code::
        

	#include <math.h>
	#include <stdio.h>
	#include "py/obj.h"
	#include "py/runtime.h"
	#include "mphalport.h"  // needed for mp_hal_ticks_cpu()
	#include "py/builtin.h" // needed for mp_micropython_mem_info()
	
	STATIC mp_obj_t measure_cpu(mp_obj_t _x, mp_obj_t _y, mp_obj_t _z) {
	    size_t start, middle, end;
	    start = m_get_current_bytes_allocated();
	
	    float x = mp_obj_get_float(_x);
	    float y = mp_obj_get_float(_y);
	    float z = mp_obj_get_float(_z);
	    middle = m_get_current_bytes_allocated();
	
	    float hypo = sqrtf(x*x + y*y + z*z);
	    end = m_get_current_bytes_allocated();
	    mp_obj_t tuple[4];
	    tuple[0] = MP_OBJ_NEW_SMALL_INT(start);
	    tuple[1] = MP_OBJ_NEW_SMALL_INT(middle);
	    tuple[2] = MP_OBJ_NEW_SMALL_INT(end);
	    tuple[3] = mp_obj_new_float(hypo);
	    return mp_obj_new_tuple(4, tuple);
	}
	
	STATIC MP_DEFINE_CONST_FUN_OBJ_3(measure_cpu_obj, measure_cpu);
	
	STATIC const mp_rom_map_elem_t profiling_module_globals_table[] = {
	    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_profiling) },
	    { MP_ROM_QSTR(MP_QSTR_measure), MP_ROM_PTR(&measure_cpu_obj) },
	};
	STATIC MP_DEFINE_CONST_DICT(profiling_module_globals, profiling_module_globals_table);
	
	const mp_obj_module_t profiling_user_cmodule = {
	    .base = { &mp_type_module },
	    .globals = (mp_obj_dict_t*)&profiling_module_globals,
	};
	
	MP_REGISTER_MODULE(MP_QSTR_profiling, profiling_user_cmodule, MODULE_PROFILING_ENABLED);

.. parsed-literal::

    written 1605 bytes to /profiling/profiling.c


The above-mentioned ``mem_info()`` function of the micropython module
can directly be called from C: after including the ``builtin.h`` header,
we can issue ``mp_micropython_mem_info(0, NULL);``, defined in
``modmicropython.c``, which will print everything we need. Although its
signature contains two arguments, a ``size_t`` and an ``mp_obj_t``
pointer to the arguments, the function does not seem to care about them,
so we can pass ``0``, and ``NULL`` without any meaning.

The function ``mp_micropython_mem_info()`` doesn’t carry out any
measurements in itself, it is only for pretty printing. The stats are
collected by ``mp_micropython_mem_total()``,
``mp_micropython_mem_current()``, and ``mp_micropython_mem_peak()``.
Unfortunately, these functions are all declared STATIC, so we cannot
call them from outsize ``modmicropython.c``. If you need a numeric
representation of the state of the RAM, you can make use of the
``m_get_total_bytes_allocated(void)``,
``m_get_current_bytes_allocated(void)``, and
``m_get_peak_bytes_allocated(void)`` functions of ``py/malloc.c``. All
three return a ``size_t``.

With the help of these three functions, we could, e.g., return the size
of the consumed memory to the micropython interpreter at the end of our
calculations. ``measure_cpu()`` could be modified as

.. code:: c

   STATIC mp_obj_t measure_cpu(mp_obj_t _x, mp_obj_t _y, mp_obj_t _z) {
       size_t start, middle, end;
       start = m_get_current_bytes_allocated();

       float x = mp_obj_get_float(_x);
       float y = mp_obj_get_float(_y);
       float z = mp_obj_get_float(_z);
       middle = m_get_current_bytes_allocated();

       float hypo = sqrtf(x*x + y*y + z*z);
       end = m_get_current_bytes_allocated();
       mp_obj_t tuple[4];
       tuple[0] = MP_OBJ_NEW_SMALL_INT(start);
       tuple[1] = MP_OBJ_NEW_SMALL_INT(middle);
       tuple[2] = MP_OBJ_NEW_SMALL_INT(end);
       tuple[3] = mp_obj_new_float(hypo);
       return mp_obj_new_tuple(4, tuple);
   }

.. code::

    %%makefile /profiling/profiling.c
    
    USERMODULES_DIR := $(USERMOD_DIR)
    
    # Add all C files to SRC_USERMOD.
    SRC_USERMOD += $(USERMODULES_DIR)/profiling.c
    
    # We can add our module folder to include paths if needed
    # This is not actually needed in this example.
    CFLAGS_USERMOD += -I$(USERMODULES_DIR)

.. code::

    !make USER_C_MODULES=../../../usermod/snippets/ all > /dev/null

.. code::

    %%micropython 
    
    import profiling
    print(profiling.measure(123, 233, 344))


.. parsed-literal::

    (672, 672, 672, 433.305908203125)
    
    

