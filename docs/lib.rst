.. SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
..
.. SPDX-License-Identifier: MIT


C++ Lib
=======

The PrioriSea C++ library (``src/lib``) implements the policy engine: formulas,
time windows, decay, scoring, the Flux KVS/RPC plumbing and the SeanerBUS
integration. Its public API is documented below, grouped by translation unit.

Types
-----

.. doxygenfile:: types.hpp

Errors
------

.. doxygenfile:: error.hpp

Utilities
---------

.. doxygenfile:: util.hpp

Configuration
-------------

.. doxygenfile:: config.hpp

Formula
-------

.. doxygenfile:: formula.hpp

Time Windows
------------

.. doxygenfile:: timewin.hpp

Decay
-----

.. doxygenfile:: decay.hpp

Score
-----

.. doxygenfile:: score.hpp

Flux
----

.. doxygenfile:: flux.hpp
