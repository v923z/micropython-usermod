
Setting up the environment
==========================

You can definitely skip the next two subsections, if you don’t care
about things related to ipython/jupyter.

ipython matters
---------------

General remarks
~~~~~~~~~~~~~~~

I have written this document entirely as an jupyter notebook. There are
several reasons for this. First, the prompt visual feedback on markdown
text, and python/C code (highlighting and code folding). Second, for
demonstration purposes, I did need to run python code either on the
local machine, or on the unix/stm32 port of micropython, and it was
simply natural to do that via jupyter. Third, and perhaps most
importantly, by using a notebook, I could work from a single location:
the documentation, the C source with its compilation, and the tests
(either on the unix port, or on the pyboard) are all in a single
container. It is impossible to overstate the advantages of this.

micropython magic
~~~~~~~~~~~~~~~~~

To make the usage a bit more convenient, we will just register a magic
method here to run micropython directly from the notebook. If you don’t
know what ipython magics are, you can read more at
https://ipython.readthedocs.io/en/stable/interactive/magics.html. In any
case, we are going to take the contents of a code cell, and pass it to
micropython, either on the local machine (unix port), or the bare metal
hardware (e.g. the pyboard) as a script.

Note that adding the magic commands makes the python code a wee bit
ugly: when running (micro)python with a script file, we won’t have so
much output as in the interactive console. In fact, except for
tracebacks and the results of explicit ``print`` statements, we won’t
see anything at all. For this reason, we will have to call ``print``,
whenever we would like to import the results into the notebook. But what
the heck! I can definitely put up with that much.

.. code::

    from IPython.core.magic import Magics, magics_class, line_cell_magic
    from IPython.core.magic import cell_magic, register_cell_magic, register_line_magic
    import subprocess
    import os

Note: if you are trying to run the notebook on windows, you will have to
change the destination file accordingly. I chose ``/dev/shm/``, so that
I won’t have to clean up the mess at the end of the session, but any
other place should do.

.. code::

    @register_cell_magic
    def micropython(line, cell):
        with open('/dev/shm/micropython.py', 'w') as fout:
            fout.write(cell)
        proc = subprocess.Popen(["./micropython", "/dev/shm/micropython.py"], 
                                stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        print(proc.stdout.read().decode("utf-8"))
        print(proc.stderr.read().decode("utf-8"))
        return None

Once we have the cell magic, we can run arbitrary micropython commands
from the notebook. We should only keep in mind that the header of the
cell, the line beginning with ``%%micropython``, is not part of the code
that is running.

C and makefile magic
~~~~~~~~~~~~~~~~~~~~

Since we will have C code all over the place, we need at least two more
magic functions. The first one takes care of the C highlighting, and
saving of the content of a cell. The cell will be saved to the location
given in the header. The second magic command generates the required
makefile.

The following function does nothing outside the notebook: it simply
registers a new mode for syntax highlighting, and switches to C,
whenever the cell begins with the string ``%%ccode``, or ``%%makefile``.

.. code::

    import IPython
    
    js = """
        (function () {
            var defaults = IPython.CodeCell.config_defaults || IPython.CodeCell.options_default;
            defaults.highlight_modes['magic_text/x-csrc'] = {'reg':[/^\\s*%%ccode/]};
        })();
        """
    IPython.core.display.display_javascript(js, raw=True)
    
    js = """
        (function () {
            var defaults = IPython.CodeCell.config_defaults || IPython.CodeCell.options_default;
            defaults.highlight_modes['magic_text/x-csrc'] = {'reg':[/^\\s*%%makefile/]};
        })();
        """
    IPython.core.display.display_javascript(js, raw=True)






And finally, at long last, here are the two magic commands.
``%makefile`` is simple: each ``micropython.mk`` makefile is the same,
with the exception of the file name that it is supposed to compile. So,
we can take a very generic string, and insert the target. In order to
have some trace in the notebook, we also insert the content of the
so-generated file into the input field of the cell.

``%%ccode`` reads the contents of the input field of the cell, adds a
small header, and writes everything into a file.

.. code::

    @magics_class
    class MyMagics(Magics):
    
        @line_cell_magic
        def makefile(self, line, cell=None):
            _makefile = """USERMODULES_DIR := $(USERMOD_DIR)
    
    # Add all C files to SRC_USERMOD.
    SRC_USERMOD += $(USERMODULES_DIR)/TARGET_MAKEFILE
    
    # We can add our module folder to include paths if needed
    # This is not actually needed in this example.
    CFLAGS_USERMOD += -I$(USERMODULES_DIR)"""
            raw_cell = _makefile.replace('TARGET_MAKEFILE', line.split('/')[-1])
            with open('../../../usermod/snippets'+line.replace(line.split('/')[-1], 'micropython.mk'), 'w') as mout:
                mout.write(raw_cell)
            self.shell.set_next_input('%%makefile {}\n\n{}'.format(line, raw_cell), replace=True)
            return None
            
        @cell_magic
        def ccode(self, line, cell):
            copyright = """/*
     * This file is part of the micropython-usermod project, 
     *
     * https://github.com/v923z/micropython-usermod
     *
     * The MIT License (MIT)
     *
     * Copyright (c) 2019 Zoltán Vörös
    */
        """
            if line:
                with open('../../../usermod/snippets'+line, 'w') as cout:
                    cout.write(copyright)
                    cout.write(cell)
                print('written %d bytes to %s'%(len(copyright) + len(cell), line))
                return None
    
    ip = get_ipython()
    ip.register_magics(MyMagics)

Note: Since both ``%makefile`` and ``%%ccode`` have the very same
argument, namely, the name of the C file, we could’ve combined the two
functions. I decided to split them for the simple reason that by doing
so, the listing of the makefile is explicit with a header.

We are done with the notebook-related matters, and in position to devote
ourselves to writing our first C module.

Notebook conversion
-------------------

This is, where the notebook becomes somewhat *meta*: the following piece
of code is really only for the conversion of the notebook to
restructured text. We are converting this very notebook from within the
notebook. You’ve got to admit that this is sort of cool!

I used sphinx as the documentation generator, and this is, why I
converted the notebook into a number of restructured text files, each
containing a single chapter. These files can be found under
https://github.com/v923z/micropython-usermod/tree/master/docs/source.

.. code::

    %cd ../../../usermod/docs/


.. parsed-literal::

    /home/v923z/sandbox/micropython/v1.11/usermod/docs


.. code::

    import nbformat as nb
    import nbformat.v4.nbbase as nb4
    from nbconvert import RSTExporter
    
    def convert_notebook(node, fn):
        (rst, resources) = rstexporter.from_notebook_node(notebook)
        with open(fn, 'w') as fout:
            fout.write(rst)
            
    rstexporter = RSTExporter()
    rstexporter.template_file = './templates/rst.tpl'
    
    source = nb.read('micropython-usermod.ipynb',nb.NO_CONVERT)
    i = 0
    notebook = nb4.new_notebook()
    for j, cell in enumerate(source['cells']):
        if cell['cell_type'] == 'markdown':
            if cell['source'].split('\n')[0].startswith('# '):
                if i >= 1:
                    convert_notebook(notebook,'./source/usermods_%02d.rst'%i)
                    notebook = nb4.new_notebook()
                i += 1
            notebook.cells.append(cell)
        else:
            notebook.cells.append(cell)
    convert_notebook(notebook,'./source/usermods_%02d.rst'%i)


.. parsed-literal::

    /home/v923z/anaconda3/lib/python3.7/site-packages/nbconvert/filters/datatypefilter.py:41: UserWarning: Your element with mimetype(s) dict_keys(['application/javascript']) is not able to be represented.
      mimetypes=output.keys())
    /home/v923z/anaconda3/lib/python3.7/site-packages/nbconvert/filters/datatypefilter.py:41: UserWarning: Your element with mimetype(s) dict_keys(['application/javascript']) is not able to be represented.
      mimetypes=output.keys())


Generating the documentation
----------------------------

While the C code of all the modules, as well as the documentation itself
is contained in this notebook, you don’t actually need jupyter for the
compilation of either the code, or the documentation. I will explain
later, how the code is to be compiled. As for the documentation, you
will need sphinx http://www.sphinx-doc.org/en/master/. Once that is
installed, you simply have to run

.. code:: bash

   sphinx-quickstart

answer the relevant questions, and overwrite ``./source/conf.py`` with
https://github.com/v923z/micropython-usermod/blob/master/docs/source/conf.py.

We converted the notebook into a dozen restructured text files under
``./source/``. In addition, you’ll also need an ``index.rst`` file,
which looks like this:

.. code::

    !head -100 ./source/index.rst


.. parsed-literal::

    .. micropython-usermod documentation master file, created by
       sphinx-quickstart on Sat Aug 31 10:56:56 2019.
       You can adapt this file completely to your liking, but it should at least
       contain the root `toctree` directive.
    
    Welcome to micropython-usermod's documentation!
    ===============================================
    
    .. toctree::
       :maxdepth: 2
       :caption: Content:
    
       usermods_01
       usermods_02
       usermods_03
       usermods_04
       usermods_05
       usermods_06
       usermods_07
       usermods_08
       usermods_09
       usermods_10
       usermods_11
       usermods_12
    
    Indices and tables
    ==================
    
    * :ref:`genindex`
    * :ref:`modindex`
    * :ref:`search`


The documentation output can now be generated by calling

.. code:: bash

   make html

or

.. code:: bash

   make latexpdf

on the command line.

The micropython code base
-------------------------

Since we are going to test our code mainly on the unix port, we set that
as the current working directory.

.. code::

    %cd ../../micropython/ports/unix/


.. parsed-literal::

    /home/v923z/sandbox/micropython/v1.11/micropython/ports/unix


The micropython codebase itself is set up a rather modular way. Provided
you cloned the micropython repository with

.. code:: bash

   git clone https://github.com/micropython/micropython.git 

onto your computer, and you look at the top-level directories, you will
see something like this:

.. code::

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

   make USER_C_MODULES=../../../user_modules CFLAGS_EXTRA=-DMODULE_EXAMPLE_ENABLED=1 all

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

and then call ``make`` without the ``CFLAGS_EXTRA`` flag:

.. code:: bash

   make USER_C_MODULES=../../../user_modules all

This separation of the user code from the micropython code base is
definitely a convenience, because it is much easier to keep track of
changes, and also because you can’t possibly screw up micropython
itself: you can also go back to a working piece of firmware by dropping
the ``USER_C_MODULES`` argument of ``make``.
