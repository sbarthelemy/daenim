=====
dænim
=====

dænim is an animation viewer for collada (.dae) files which is based on the
OpenSceneGraph library.

Install
=======

On Ubuntu Precise Pangolin, the binary package is available on a dedicated PPA:

    sudo add-apt-repository ppa:barthelemy/collada
    sudo apt-get update
    sudo aptitude install daenim

TODO: create & publish binary packages for other platforms and explain how to
      use them.

Content
=======

:cmake:
    additional modules for cmake

:data:
    pictures for buttons etc.

:src:
    source code

Build from source
=================

Prerequisites
-------------

The only prerequisites is openscenegraph with collada support.

On Ubuntu Precise Pangolin, we use OSG 3.0.1 available on dedicated PPA::

    sudo add-apt-repository ppa:barthelemy/collada
    sudo apt-get update
    sudo aptitude install cmake libopenscenegraph-dev

On Mac os X, one needs to build OSG manually. collada-dom is available through
`macports <http://www.macports.org/>`_.

TODO: check Mac os instructions
TODO: add Windows instructions

Build
-----

::

    cd daenim
    mkdir ../daenim-build
    cd ../daenim-build
    cmake ../daenim
    make
    sudo make install

without the install step, the executable won't find its icons.
