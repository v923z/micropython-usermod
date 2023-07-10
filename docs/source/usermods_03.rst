The micropython code base
=========================

Since we are going to test our code mainly on the unix port, we set that
as the current working directory.

.. code:: bash

    !cd ../../micropython/ports/unix/
.. parsed-literal::

    /home/v923z/sandbox/micropython/v1.20.0/micropython/ports/unix

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

Beginning with the 1.20.0 version of micropython, it became quite simple
to add a user-defined C module to the firmware. You simply drop two or
three files in an arbitrary directory, and pass two compiler flags to
``make`` like so:

.. code:: bash

    !make USER_C_MODULES=../../../user_modules
Here, the ``USER_C_MODULES`` variable is the location (relative to the
location of ``make``) of your files. This separation of the user code from the micropython code base is
definitely a convenience, because it is much easier to keep track of
changes, and also because you can’t possibly screw up micropython
itself: you can also go back to a working piece of firmware by dropping
the ``USER_C_MODULES`` argument of ``make``.
