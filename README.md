MSP430 Software UART
=============

This is a basic software UART application for TI MSP430x2xx.
The toolchain uses GCC/make/splint. This project is suitable to use with the Launchpad as a base for other projects.


Quick start
-------

    sudo apt-get install git make splint gcc-msp430 mspdebug screen git
    git clone https://github.com/LarsH/msp430-uart.git
    cd msp430-uart
    make program
    screen /dev/ttyACM0 9600

The screen should not be detached from, but killed, when done with the session.
It is killed with the key combination

    Ctrl-a k

Choosing the target MCU
-------

The build environment reads the cpu type from the file config.mcu. It is initialized automatically, but be sure to change it if you change MCU! It is enough to just delete config.mcu and let the Makefile recreate it.

Dependencies
-------

The project uses the msp430 port of GCC for compiling, Splint for static code analysis and GNU Make to manage the build environment. To communicate with the serial port a terminal program is needed, the program screen works fine.

### Debian / Ubuntu

    apt-get install make splint gcc-msp430 mspdebug screen

### OS X
Installing instructions for gcc-msp430 and mspdebug:

<http://processors.wiki.ti.com/index.php/MSP430_LaunchPad_Mac_OS_X>

However, the scripts for building mspgcc4 uses wget so either build and install wget or update the do-[gdb,gcc,libc,binutils].sh scripts to use curl instead of wget.

Installing instructions for splint:

<http://www.tillett.info/2010/09/21/installing-splint-on-mac-osx/>

