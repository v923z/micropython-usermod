
Introduction
============

So, you have somehow bumped into micropython, fallen in love with it in
an instance, broken your piggy bank, and run off, head over heels, to
order a pyboard. You have probably paid extra for the expedited
shipping. Once the pyboard arrived, you became excited like a puppy with
a bone. You played with the hardware, learnt how to access the
accelerometer, switch, LEDs, and temperature sensor, and you
successfully communicated with other devices via the I2C, SPI, USART, or
CAN interfaces. You have plugged the board in a computer, and driven
someone crazy by emulating a seemingly disoriented mouse on it. You have
even tried to divide by zero, just to see if the chip would go up in
flames (this was vicious, by the way), and noticed that the interpreter
smartly prevented such things from happening. You have written your own
python functions, even compiled them into frozen modules, and burnt the
whole damn thing onto the microcontroller. Then you have toyed with the
on-board assembler, because you hoped that you could gain some
astronomical factors in speed. (But you couldn’t.)

And yet, after all this, you feel somewhat unsatisfied. You find that
you want to access the periphery in a special way, or you need some
fancy function that, when implemented in python itself, seems to consume
too much RAM, and takes an eternity to execute, and assembly, with its
limitations, is just far too awkward for it. Or perhaps, you are simply
dead against making your code easily readable by writing everything in
python, and you want to hide the magic, just for the heck of it. But you
still want to retain the elegance of python.

If, after thorough introspection and soul-searching, you have discovered
these latter symptoms in yourself, you have two choices: either you
despair, scrap your idea, and move on, or you learn how the heavy
lifting behind the micropython facade is done, and spin your own
functions, classes, and methods in C. As it turns out, it is not that
hard, once you get the hang of it. The sole trick is to get the hang of
it. And this is, where this document intends to play a role.

On the following pages, I would like to show how new functionality can
be added and exposed to the python interpreter. I will try to discuss
all aspects of micropython in an approachable way. Each concept will be
presented in an implementation, stripped to the bare minimum, that you
can compile right away, and try yourself. (The code here has been tested
against micropython v.1.11.) At the end of each chapter, I will list the
discussed code in its entirety, and I also include a link the the
source, so that copying and pasting does not involve copious amounts of
work. Moreover, I include a small demonstration, so that we can actually
see that our code works. The code, as well as the source of this
document are also available under
https://github.com/v923z/micropython-usermod.

I start out with a very simple module and slowly build upon it. At the
very end of the discussion, I will outline my version of a
general-purpose math library, similar to numpy. In fact, it was when I
was working on this math module that I realised that a decent
programming guide to micropython is sorely missing, hence this document.
Obviously, numpy is a gigantic library, and we are not going to
implement all aspects of it. But we will be able to define efficiently
stored arrays on which we can do vectorised computations, work with
matrices, invert and contract them, fit polynomials to measurement data,
and get the Fourier transform of an arbitrary sequence. I do hope that
you find the agenda convincing enough!

One last comment: I believe, all examples in this document could be
implemented with little effort in python itself, and I am definitely not
advocating the inclusion of such trivial cases in the firmware. I chose
these examples on two grounds: First, they are all simple, almost
primitive, but for this very reason, they demonstrate a single idea
without distraction. Second, having a piece of parallel python code is
useful insofar as it tells us what to expect, and it also encourages us
to implement the C version such that it results in *pythonic* functions.
