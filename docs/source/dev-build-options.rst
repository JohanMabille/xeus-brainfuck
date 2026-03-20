..  Copyright (c) 2026,    

   Distributed under the terms of the BSD 3-Clause License.  

   The full license is in the file LICENSE, distributed with this software.

Build and configuration
=======================

General Build Options
---------------------

Building the xeus-brainfuck library
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

``xeus-brainfuck`` build supports the following options:

- ``XEUS_BRAINFUCK_BUILD_SHARED``: Build the ``xeus-brainfuck`` shared library. **Enabled by default**.
- ``XEUS_BRAINFUCK_BUILD_STATIC``: Build the ``xeus-brainfuck`` static library. **Enabled by default**.


- ``XEUS_BRAINFUCK_USE_SHARED_XEUS``: Link with a `xeus` shared library (instead of the static library). **Enabled by default**.

Building the kernel
~~~~~~~~~~~~~~~~~~~

The package includes two options for producing a kernel: an executable ``xbrainfuck`` and a Python extension module, which is used to launch a kernel from Python.

- ``XEUS_BRAINFUCK_BUILD_EXECUTABLE``: Build the ``xbrainfuck``  executable. **Enabled by default**.


If ``XEUS_BRAINFUCK_USE_SHARED_XEUS_BRAINFUCK`` is disabled, xbrainfuck  will be linked statically with ``xeus-brainfuck``.

Building the Tests
~~~~~~~~~~~~~~~~~~

- ``XEUS_BRAINFUCK_BUILD_TESTS ``: enables the tets  **Disabled by default**.

