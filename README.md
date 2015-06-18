Heat master
==========

<b>This master is my own work, but the worker is a collaboration with Baptiste Pauget.</b>

Master software for parallel simulation of a discrete heat equation/propagation equation (depends on the workers!)

Its role is to supply the workers with some work, as well as split the map between them dynamically (remap when a new worker comes/leaves). It might be well to improve it adding even more dynamism (measure the speed of each worker to allocate dynamically more or less work to each worker).

Requirements
----------

Qt, Qt networking, and at least two computers in the same LAN :)

Compiling
--------

```bash
$ qmake
$ make
```

Running 
------

You need to feed the master with some information through standard input (stdin). Input format:

On the first line, the size (in pixels) of the map (which is a square) and the scale of display (an integer), useful to view 50x50 maps for instance ; space separated.

On each of the [size] following lines, [size] integers in range [0, 16000] with the 16th bit (0x8000) set for a fixed value over time (the edges MUST be fixed over time!), space-separated.

A sample input can be generated with fillInput.py.

When the GUI shows up, you still have to connect workers to it!
