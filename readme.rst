=========================
Real-Time Water Rendering
=========================

.. image:: /screenshots/screen_ours_1.jpg
    :align: center
    :width: 500pt


.. sectnum::

.. contents:: Table of contents

This repository contains the implementation of the Bachelor thesis "Real-Time
Water Rendering". 

Abstract
--------

Rendering large bodies of water is still an open problem in computer graphics.
On the one hand, because no physical model exists which is able to represent at
the same time the motion of deep and shallow ocean waves. On the other hand,
because computational time is limited to less than 1/60 seconds in real-time
applications. We implement the projected grid level of detail technique into the
open-source game engine Pyrogenesis. The ocean waves are synthesized using
Tessendorf's technique. The results show that it is possible to implement a
realistic water rendering technique in a game engine.

Compilation
-----------

The full build instructions are found in the file
`BuildInstructions <BuildInstructions.txt>`_


Execution
---------

Depending on your build platform, launch the executable called ``pyrogenesis``.
Custom flags and options can be found in the `readme
<binaries/system/readme.txt>`_

Result screenshots
------------------

.. figure:: /screenshots/screen_ours_1.jpg
    :align: center
    :width: 500pt

    Our implementation

.. figure:: /screenshots/screen_theirs_1.jpg
    :align: center
    :width: 500pt

    Their implementation

.. figure:: /screenshots/screen_ours_5.jpg
    :align: center
    :width: 500pt

    Our implementation

.. figure:: /screenshots/screen_theirs_5.jpg
    :align: center
    :width: 500pt

    Their implementation
