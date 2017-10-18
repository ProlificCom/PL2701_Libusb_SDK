Introduction
    This software package includes the following components:
    1. listdev
        A sample program to list PL2701 device and show its information
    2. transmit
        A simplified program to transmit data from sender to receiver
    3. benchmark
        A program to measure the performance between two ends
    
Test/Build Environment
    Ubuntu 14.04 x64 and Ubuntu 16.04 x64

Installation/Build
    Follow the below steps to install the latest libusb and build all sample programs:
    1. Install all necessary packages – libudev-dev
        a. Download the latest package lists from the repositories by typing “sudo apt-get update” in the command line
        b. Install “libudev-dev” by typing “sudo apt-get install libudev-dev” in the command line
    2. Build and install the libusb downloaded from https://github.com/libusb/libusb
        a. Extract libusb-1.0.21.zip to the directory “libusb”
        b. Change directory to “libusb”
        c. Run “./configure”to configure the build environment
        d. Run “make” to build the library
        e. Run “sudo make install” to install it on the system
    3. Build the sample programs
        a. Change directory to “Source”
        b. Run “make” to build all programs, and then “listdev”, “transmit”, and “benchmark” will be created
    
Execution
listdev
    Perform the below steps to run the sample program “listdev”:
    1. Connect both sides of PL2701 USB cable to two PCs running Ubuntu OS.
    2. On any Ubuntu system, execute the following command
        sudo ./listdev
    After that, the program lists found PL2701 device and show its information.
transmit
    Perform the below steps to run the sample program “transmit”:
    1. Connect both sides of PL2701 USB cable to two PCs running Ubuntu OS. One is called Ubuntu A, and another is called Ubuntu B.
    2. On Ubuntu A, execute the following command
        sudo ./transmit recv
    3. On Ubuntu B, execute the following command
        sudo ./transmit send
    After that, the sender program transmits data to the receiver program.
benchmark
    Perform the below steps to run the sample program “benchmark”:
    1. Connect both sides of PL2701 USB cable to two PCs running Ubuntu OS. One is called Ubuntu A, and another is called Ubuntu B.
    2. On Ubuntu A, execute the following command
        sudo ./benchmark recv
    3. On Ubuntu B, execute the following command
        sudo ./benchmark send
    After that, the sender program transmits data to the receiver program and then gets the performance information.
