=====
dænim
=====

dænim is an animation viewer for collada (.dae) files which is based on the
OpenSceneGraph library.

Install
=======

TODO: create & publish binary packages and explain how to use them.

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

The only prerequisites is openscenegraph 2.9.9 with collada support.

On Ubuntu lucid lynx, we use OSG 2.9.9 available on dedicated PPA::

    sudo add-apt-repository ppa:barthelemy/collada
    sudo apt-get update
    sudo aptitude install cmake libopenscenegraph-dev


On Mac os X, one needs to build OSG manually. collada-dom is available through
`macports <http://www.macports.org/>`_.

FIXME: add Windows details

Build
-----

::

    cd daenim
    mkdir ../daenim-build
    cd ../daenim-build
    cmake ../daenim
    make
