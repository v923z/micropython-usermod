
The micropython code base
=========================

Since we are going to test our code mainly on the unix port, we set that
as the current working directory.

.. code:: bash

    !cd ../../micropython/ports/unix/
The micropython codebase itself is set up a rather modular way. Provided
you cloned the micropython repository with

.. code:: bash

    !git clone https://github.com/micropython/micropython.git 
onto your computer, and you look at the top-level directories, you will
see something like this:

.. code:: bash

    !ls ../../../micropython/
.. parsed-literal::

    ACKNOWLEDGEMENTS    docs      lib	 pic16bit   teensy   zephyr
    bare-arm	    drivers   LICENSE	 py	    tests
    cc3200		    esp8266   logo	 qemu-arm   tools
    CODECONVENTIONS.md  examples  minimal	 README.md  unix
    CONTRIBUTING.md     extmod    mpy-cross  stmhal     windows

Out of all the directoties, at least two are of particular interest.
Namely, ``/py/``, where the python interpreter is implemented, and
``/ports/``, which contains the hardware-specific files. All questions
pertaining to programming micropython in C can be answered by browsing
these two directories, and perusing the relevant files therein.

User modules in micropython
---------------------------

Beginning with the 1.10 version of micropython, it became quite simple
to add a user-defined C module to the firmware. You simply drop two or
three files in an arbitrary directory, and pass two compiler flags to
``make`` like so:

.. code:: bash

    !make USER_C_MODULES=../../../user_modules CFLAGS_EXTRA=-DMODULE_EXAMPLE_ENABLED=1 all
Here, the ``USER_C_MODULES`` variable is the location (relative to the
location of ``make``) of your files, while ``CFLAGS_EXTRA`` defines the
flag for your particular module. This is relevant, if you have many
modules, but you want to include only some of them.

Alternatively, you can set the module flags in ``mpconfigport.h`` (to be
found in the port’s root folder, for which you are compiling) as

.. code:: make

   #define MODULE_SIMPLEFUNCTION_ENABLED (1)
   #define MODULE_SIMPLECLASS_ENABLED (1)
   #define MODULE_SPECIALCLASS_ENABLED (1)
   #define MODULE_KEYWORDFUNCTION_ENABLED (1)
   #define MODULE_CONSUMEITERABLE_ENABLED (1)
   #define MODULE_VECTOR_ENABLED (1)
   #define MODULE_RETURNITERABLE_ENABLED (1)
   #define MODULE_PROFILING_ENABLED (1)
   #define MODULE_MAKEITERABLE_ENABLED (1)
   #define MODULE_SUBSCRIPTITERABLE_ENABLED (1)
   #define MODULE_SLICEITERABLE_ENABLED (1)
   #define MODULE_VARARG_ENABLED (1)
   #define MODULE_STRINGARG_ENABLED (1)

and then call ``make`` without the ``CFLAGS_EXTRA`` flag:

.. code:: bash

    !make USER_C_MODULES=../../../user_modules all
This separation of the user code from the micropython code base is
definitely a convenience, because it is much easier to keep track of
changes, and also because you can’t possibly screw up micropython
itself: you can also go back to a working piece of firmware by dropping
the ``USER_C_MODULES`` argument of ``make``.
