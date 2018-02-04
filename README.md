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
The commands above also make the samples in the subdirectories of `samples/`.

mGPlus also provides some configuration options to customize the features.
For more information, please run

    $ ./configure --help

## Copying

    Copyright (C) 2008~2018, Beijing FMSoft Technologies Co., Ltd.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    Or,

    As this program is a library, any link to this program must follow
    GNU General Public License version 3 (GPLv3). If you cannot accept
    GPLv3, you need to be licensed from FMSoft.

    If you have got a commercial license of this program, please use it
    under the terms and conditions of the commercial license.

    For more information about the commercial license, please refer to
    <http://www.minigui.com/en/about/licensing-policy/>.


## AGG Copying

Note that mPlus uses AGG (Anti-Grain Geometry) V2.5 as the software 
vectorial graphics engien. Following is the license of AGG V2.5:

    Anti-Grain Geometry (AGG) - Version 2.5
    A high quality rendering engine for C++
    Copyright (C) 2002-2006 Maxim Shemanarev
    Contact: mcseem@antigrain.com
            mcseemagg@yahoo.com
            http://antigrain.com
 
    AGG is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 2
    of the License, or (at your option) any later version.
 
    AGG is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
 
    You should have received a copy of the GNU General Public License
    along with AGG; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, 
    MA 02110-1301, USA.

