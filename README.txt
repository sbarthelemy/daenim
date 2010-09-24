=====
dænim
=====

dænim is 

Install
=======

TODO: create & publish binary packages and explain how to use them.

Content
=======

:cmake:
    additional modules for cmake

:src:
    source code


Build from source
=================

Prerequisites
-------------

On Ubuntu lucid lynx ::

    sudo add-apt-repository ppa:barthelemy/collada
    sudo apt-get update
    sudo aptitude install cmake libcolladadom-dev


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

