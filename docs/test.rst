


.. parsed-literal::

    Overwriting templates/rst.tpl


We have the relevant jinja templates, we can convert our notebook to
restructured text:

.. code:: ipython3

    import nbconvert
    
    from nbconvert import RSTExporter
    
    rstexporter = RSTExporter()
    
    (body, resources) = rstexporter.from_filename('test.ipynb')
    body[:100]


.. parsed-literal::

    /home/v923z/anaconda3/lib/python3.7/site-packages/nbconvert/filters/datatypefilter.py:41: UserWarning: Your element with mimetype(s) dict_keys(['application/javascript']) is not able to be represented.
      mimetypes=output.keys())




.. parsed-literal::

    '\n.. code:: ipython3\n\n    from IPython.core.magic import register_cell_magic\n    import subprocess\n\nN'



.. code:: ipython3

    !jupyter nbconvert test.ipynb --to rst


.. parsed-literal::

    [NbConvertApp] Converting notebook test.ipynb to rst
    /home/v923z/anaconda3/lib/python3.7/site-packages/nbconvert/filters/datatypefilter.py:41: UserWarning: Your element with mimetype(s) dict_keys(['application/javascript']) is not able to be represented.
      mimetypes=output.keys())
    [NbConvertApp] Writing 5468 bytes to test.rst


We are done with the notebook-related matters, and in position to devote
ourselves to writing our first C module.

['/simplefunction/simplefunction.c']

.. literalinclude:: /simplefunction/simplefunction.c
    :language: c
    :linenos:




.. parsed-literal::

    written 1090 bytes to /simplefunction/simplefunction.c


You should be able to compile the module above by calling
(``> /dev/null`` is not necessary, especially under windows. I included
that in order to suppress the output of the shell command).

.. code:: ipython3

    !make USER_C_MODULES=../../../usermod/ all > /dev/null
