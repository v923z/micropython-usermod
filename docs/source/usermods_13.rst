A word for the lazy
===================

If you still find that coding in C is too cumbersome, you can try your
hand at a stub generator, e.g.,
https://github.com/pazzarpj/micropython-ustubby, or
https://gitlab.com/oliver.robson/mpy-c-stub-gen . These tools allow you
to convert your python code into C code. Basically, they will produce a
boilerplate file, which you can flesh out with the C implementation of
the required functionality.

Looking at the usage examples, it is clear to me that one can save a lot
of typing with these stub generators, but one will still need a basic
understanding of how to work with the micropython C code.
