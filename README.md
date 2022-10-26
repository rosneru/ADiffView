# README

## About

ADiffView is a file compare tool for AmigaOS3.0+. It compares two ASCII
text files and graphically displays the differences.

The diff engine uses my own implementation of Eugene Myers' diff
algorithm. In addition, the algorithm has been optimized for
non-recursive processing, as described on [Matthias Hertel's
website](www.mathertel.de). See the file LICENSE-3RD-PARTY for more
information.

## Development environment

ADiffView can be build under Linux with *cmake* and [Bebbos gcc 6.5
toolchain](https://github.com/bebbo/amiga-gcc) or on an Amiga with 
*StormC4*.

## Build with Linux
### Dependencies
The project is created with Debian buster on Windows 10 (WSL) with the 
following packages installed:

 - build-essentials
 - cmake
 - git
 - [ Bebbos gcc 6.5 toolchain](https://github.com/bebbo/amiga-gcc) 
which is expected to be installed in /opt

### Build
The Makefile to build this project must be created with cmake.

In the root directory of the project, enter:

    mkdir build
    cd build
    cmake ..

Once cmake has finished without errors, the project can be build:
    
    make -j4

### VSCode integration

On a Linux machine open the ADiffView directory in VSCode.

The VSCode default build task (see tasks.json) was set up to use the
manually created Makefile above.

From within VSCode building can be started with **Ctrl + Shift + b**.

Then the Amiga 68k binary ADiffView_gcc is rebuild according to the
changes made to the source code. This ia a stripped release build for
68k Amigas.

### Build and run the unit tests
There are some unit tests in directory `src/diffengine/test_boost/`.

To build them the *boost framework* and *qmake* must be installed on the
developer machine.

To prepare and build the unit tests, change to the project root
directory and enter:

    mkdir build-tests
    cd build-tests
    qmake ../src/diffengine/test_boost/DiffEngine_test_boost.pro
    make -j4

Now the tests are build initially. To rebuild and run the tests after
code changes, press *F5* in VSCode

## Build on Amiga
A project file for StormC4 is included so that the project can also be
built on the Amiga.

StormC4 contains a working source-level debugger with variable display.
This was an important tool during development.

### Dependencies

- [StormC4](https://www.amiga-shop.net/en/Amiga-Software/Amiga-Tools/StormC-v4::145.html)
- [STL implementation for StormC4](http://aminet.net/package/dev/c/amigastlport)

### Build and run

To build with StormC4 click on the button *Open project* and select the
file `ADiffView.¶`

Press *F8* to start building.

To run ADiffView after a successful build click on the *Run* icon or
press *F9*.
