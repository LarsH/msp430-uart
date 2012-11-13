MSP430 Software UART
=============

This is a basic software UART application for TI MSP430x2xx.
The toolchain uses GCC/make/splint. This project is suitable to use with the Launchpad as a base for other projects.


Quick start
-------

    sudo apt-get install git make splint gcc-msp430
    git clone https://github.com/LarsH/msp430-uart.git
    cd msp430-uart
    make program
    screen /dev/ttyACM0 9600   

The screen should not be detached from, but killed, when done with the session.
It is killed with the key combination

    Ctrl-a k

Dependencies
-------

The project uses the msp430 port of GCC for compiling, Splint for static code analysis and GNU Make to manage the build environment.

### Debian / Ubuntu

    apt-get install make splint gcc-msp430

### OS X
Installing instructions for gcc-msp430 and mspdebug:

<http://processors.wiki.ti.com/index.php/MSP430_LaunchPad_Mac_OS_X>

Installing instructions for splint:

<http://www.tillett.info/2010/09/21/installing-splint-on-mac-osx/>

