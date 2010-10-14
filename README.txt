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

On Ubuntu lucid lynx, we use OSG 2.9.8 available on a PPA::

    sudo add-apt-repository ppa:barthelemy/collada
    sudo apt-get update
    sudo aptitude install cmake libopenscenegraph-dev


On Mac os X, the only available version of OpenSceneGraph via
`macports <http://www.macports.org/>`_ is 2.9.7 while we need 2.9.8.

FIXME: add Windows details

Build
-----

::

    cd daenim
    mkdir build
    cd build
    cmake ..
    make

