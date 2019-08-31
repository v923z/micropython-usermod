
Outline of a math library
=========================

As I indicated at the very beginning, my main motivation for writing
this document was that I wanted to have a reasonable programming manual
for the development of a math library. Since I couldn’t find any, I have
turned the problem around, and written up, what I have learnt by
developing the library. But the question is, what this library should
achieve in the first place?

Requirements
------------

Recently, I have run into a limitations with the micropython
interpreter. These difficulties were related to both speed, and RAM.
Therefore, I wanted to have something that can perform common
mathematical calculations in a pythonic way, with little burden on the
RAM, and possibly fast. On PCs, such a library is called ``numpy``, and
it felt only natural to me to implement those aspects of ``numpy`` that
would find an applications in the context of data acquisition of
moderate volume: after all, no matter what, the microcontroller is not
going to produce or collect huge amounts of data, but it might still be
useful to process these data within the constraints of the
microcontroller. Due to the nature of the data that would be dealt with,
one can work with a very limited subset of ``numpy``.

Keeping these considerations in mind, I set my goals as follows:

-  One should be able to vectorise standard mathematical functions,
   while these functions should still work for scalars, so

.. code:: python

   a = 1.0
   sin(a)

and

.. code:: python

   a = [1.0, 2.0, 3.0]
   sin(a)

should both be valid expressions.

-  There should be a binary container, (``ndarray``) for numbers that
   are results of vectorised operations, and one should be able to
   initialise a container by passing arbitrary ``iterables`` to a
   constructor (see ``sin([1, 2, 3])`` above).

-  The array should be iterable, so that we can turn it into lists,
   tuples, etc.

-  The relevant binary operations should work on arrays as in ``numpy``,
   that is, e.g.,

.. code:: python

   >>> a = ndarray([1, 2, 3, 4])
   >>> (a + 1) + a*10

should evaluate to ``ndarray([12, 23, 34, 45])``.

-  2D arrays (matrices) could be useful (see below), thus, the
   above-mentioned container should be able to store its ``shape``.

-  Having matrices, it is only natural to implement standard matrix
   operations (inversion, transposition etc.)

-  These numerical arrays and matrices should have a reasonable visual
   representation (pretty printing)

-  With the help of matrices, one can also think of polynomial fits to
   measurement data

-  There should be an FFT routine that can work with linear arrays. I do
   not think that 2D transforms would be very useful for data that come
   from the ADC of the microcontroller, but being able to extract
   frequency components of 1D signals would be an asset.

And this is, how ``ulab`` was born. But that is another story, for
another day https://github.com/v923z/micropython-ulab/.
