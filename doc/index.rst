==================================
Hermes Documentation (Legacy Code)
==================================

.. raw:: latex
 
   \floatplacement{figure}{H}

Thank you for your interest in Hermes!

Hermes is a C++ library for rapid development of adaptive *hp*-FEM and *hp*-DG solvers. You are 
viewing documentation for the **legacy code**. 

Version 1.0 is available
~~~~~~~~~~~~~~~~~~~~~~~~

Hermes version 1.0 is now available in separate 
repositories for the library itself, the tutorial, and examples. To obtain the source codes,
type::

    git clone git://github.com/hpfem/hermes.git
    git clone git://github.com/hpfem/hermes-tutorial.git
    git clone git://github.com/hpfem/hermes-examples.git

Each of these repositories contains a folder doc/ with Sphinx documentation. To build 
the docs, type "make html" in that folder. 

Documentation for the legacy code (continued)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This document is organized as follows: 

* Section 1 provides general information about Hermes and the computational methods it uses,
  and gives references to underlying scientific articles.
* Section 2 describes how to install Hermes on various hardware platforms, and how to 
  install matrix solver packages and various optional packages. 
* Section 3 explains how to use Git and Github, and how you can contribute to the project if interested.
* Section 4 contains a tutorial to Hermes2D. Please read this tutorial first even if you are 
  interested in 1D or 3D problems, since the syntax is virtually the same. The tutorial 
  will walk you in small steps through the solution
  of linear, nonlinear, and time-dependent problems from various engineering and scientific areas, 
  using higher-order elements and adaptivity algorithms, and solving multiphysics coupled problems. 
* Section 5 shows how Hermes performs on numerous benchmarks with known exact solutions. Many 
  of them come from the National Institute for Standards and Technology (NIST). Benchmarks 
  like this are great for assessing the performance of a finite element code.
* Section 6 presents examples from various application areas such as acoustics, fluid and solid
  mechanics, electromagnetics, neutronics, quantum chemistry, ground-water flow, and others. 
* Section 7 describes the usage of Hermes3D. Since it is very similar to Hermes2D, 
  we mostly focus on their differences.
* Section 8 shows how to solve 1D problems using Hermes2D.

This document is under continuous development. If you find bugs, typos, dead links 
and such, please report them to one of the mailing lists for 
`1D and 2D problems <http://groups.google.com/group/hermes2d/>`_, or
`3D problems <http://groups.google.com/group/hermes3d/>`_ -- thanks!

Introduction
------------

.. toctree::
    :maxdepth: 1

    src/about-hermes
    src/math-background
    src/web-access
    src/citing-hermes

Installation
------------

.. toctree::
    :maxdepth: 1

    src/installation/linux
    src/installation/mac
    src/installation/win-cygwin
    src/installation/win-msvc
    src/installation/matrix_solvers
    src/installation/cython_installation
    src/installation/exodusII_netcdf

Collaboration
-------------

.. toctree::
    :maxdepth: 1

    src/collaboration
    src/editing_sphinx

Tutorial
--------

.. toctree::
    :maxdepth: 1

    src/hermes2d/P01-linear
    src/hermes2d/P02-nonlinear
    src/hermes2d/P03-transient
    src/hermes2d/P04-adaptivity
    src/hermes2d/P05-eigenproblems
    src/hermes2d/P06-fvm-and-dg
    src/hermes2d/P07-trilinos
    src/hermes2d/P08-miscellaneous

Benchmarks
----------

.. toctree::
    :maxdepth: 1

    src/hermes2d/benchmarks-nist
    src/hermes2d/benchmarks-general

Examples
--------

Not all examples are documented, please check the examples/
directory as well.

.. toctree::
    :maxdepth: 1

    src/hermes2d/examples/acoustics.rst
    src/hermes2d/examples/advection-diffusion-reaction.rst
    src/hermes2d/examples/euler.rst
    src/hermes2d/examples/flame-propagation.rst
    src/hermes2d/examples/heat-transfer.rst
    src/hermes2d/examples/helmholtz.rst
    src/hermes2d/examples/linear-elasticity.rst
    src/hermes2d/examples/maxwell.rst
    src/hermes2d/examples/navier-stokes.rst
    src/hermes2d/examples/nernst-planck.rst
    src/hermes2d/examples/neutronics.rst
    src/hermes2d/examples/poisson.rst
    src/hermes2d/examples/richards.rst
    src/hermes2d/examples/schroedinger.rst
    src/hermes2d/examples/thermoelasticity.rst
    src/hermes2d/examples/wave-equation.rst
    src/hermes2d/examples/miscellaneous.rst
    
Hermes1D
--------

There is no separate Hermes1D library -- one-dimensional problems are solved using Hermes2D. 

.. toctree::
    :maxdepth: 1

    src/hermes1d/examples.rst
    src/hermes1d/quantum-notes.rst

Hermes3D
--------

.. toctree::
    :maxdepth: 1

    src/hermes3d/mesh.rst
    src/hermes3d/paraview.rst
    src/hermes3d/benchmarks.rst
    src/hermes3d/examples.rst




.. #####

    src/wrappers

    Indices and Tables
    ==================

    * :ref:`genindex`
    * :ref:`modindex`

    .. * :ref:`glossary`

    * :ref:`search`

    .. _Hermes: http://www.hpfem.org/hermes
    .. _FEMhub: http://www.hpfem.org/femhub
    .. _Agros2D: http://www.hpfem.org/agros2d
    .. _hp-FEM: http://www.hpfem.org
