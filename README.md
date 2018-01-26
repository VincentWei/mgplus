# mGPlus

mGPlus - A MiniGUI component which provides support for advanced 2D graphics 
functions like path, gradient, anti-aliase stretch, and color combination.

This is the mainline release of mGPlus V1.4.x for MiniGUI V3.2.x or later.

## Prerequisites

    * MiniGUI: v3.2.0 or later
    * Freetype (optional): v6.2.0 or later

## Building

mGPlus uses GNU autoconf/automake scripts to configure and build the project.

Run

    $ ./configure; make; sudo make install

to configure, make, and install the headers and the libraries (libmgplus).

mGPlus also provides some configuration options to customize the features.
For more information, please run

    $ ./configure --help

After installed mGPlus successfully, you can change to samples/ to make
the samples:

    $ cd samples/
    $ make

## Copying

Copyright (C) 2008 ~ 2018, Beijing FMSoft Technologies Co., Ltd.

