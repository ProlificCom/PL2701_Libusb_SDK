1. Introduction
Prolific PL2701 SDK directly utilized libusb to provide the interface of device control and data transfer to the upper software, so the customers or end users can easily develop their USB host-to-host applications on all popular platforms.
2. Directory tree
There are four directories under SDK, and each of them is listed below:
libusb\ - It includes whole libusb v1.0.21 source gotten from the libusb source on GitHub.
MSVC\ - It includes Visual Studio solution and projects. The built libraries and executable files will be generated and stored under this directory.
Source\ - It includes all sample codes which cross Windows, Linux, and MAC OS X operating systems. The three sample programs are as follows:
    listdev: To list PL25A1 device and show its information
    transmit: To transmit data from sender to receiver
    benchmark:To do performance benchmark for the sender side
Xcode\ - It includes MAC Xcode workspace and projects.