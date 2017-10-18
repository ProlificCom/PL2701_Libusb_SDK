//
//  libpl2701.h
//  Library for accessing Prolific 2701 USB devices
// 
//  Created by software team in Prolific on 03/13/2017.
//  Copyright (c) 2017 Prolific Corp.. All rights reserved.
//

#ifndef LIBPL2701_H
#define LIBPL2701_H

#define VERSION                         0.92   // Sample code version

#define DEBUG_ENABLE                       0   // True for showing debug messages
#if DEBUG_ENABLE
#define DEBUG        printf
#else
#define	DEBUG(...)
#endif

#define PROLIFIC_VID                  0x067B   // Prolific Vender ID
#define PL2701_PID                    0x2701   // Prolific Product ID
#define BULK_USB3_TIMEOUT               1000   // Bulk transfer Timeout in millisecond 
#define SLEEP_TIME                       500   // Default sleep time in millisecond (Windows) or microsecond (MAC or Linux)

#define BULK_USB2_EP0_IN_ADDR           0x81   // Interrupt endpoint  
/* Endpoints for Inteface 1 */
#define BULK_INTERFACE1_EP1_OUT_ADDR    0x02   // Bulk endpoint 1 Out Address
#define BULK_INTERFACE1_EP1_IN_ADDR     0x83   // Bulk endpoint 1 In Address

/* Endpoints for Inteface 2 */
#define BULK_INTERFACE2_EP1_OUT_ADDR    0x04   // Bulk endpoint 1 Out Address
#define BULK_INTERFACE2_EP1_IN_ADDR     0x85   // Bulk endpoint 1 In Address

/* Endpoints for Inteface 5 */
#define BULK_INTERFACE5_EP1_OUT_ADDR    0x08   // Bulk endpoint 1 Out Address
#define BULK_INTERFACE5_EP1_IN_ADDR     0x89   // Bulk endpoint 1 In Address
#define BULK_INTERFACE5_EP2_OUT_ADDR    0x0A   // Bulk endpoint 2 Out Address
#define BULK_INTERFACE5_EP2_IN_ADDR     0x8B   // Bulk endpoint 2 In Address
#define BULK_EP1_FIFO_SIZE              2048   // FIFO size in PL2701 USB device is dependent on the firmware branch the customer got

// Error codes
#define PL_ERROR_SUCCESS                    0   // No error
#define PL_ERROR_WRONG_PARA              -100   // Wrong input parameter format
#define PL_ERROR_NO_DEVICE               -101   // No PL2701 USB device was found
#define PL_ERROR_WRONG_STATUS            -102   // Fail to get PL2701 USB device status
#define PL_ERROR_WRONG_DEVICE_NO         -103   // Number of devices are wrong

// Sender and receiver's definitions
#define PROGRAM_ROLE_UNKNOWN                0
#define PROGRAM_ROLE_SENDER                 1   // The sender of this sample program
#define PROGRAM_ROLE_RECEIVER               2   // The receiver of this sample program

// Vendor-specific Requests
#define DEVICE_STATUS_LEN                   2   // The byte length of return data from GET_STATUS
// Device status
typedef	struct _DEV_STATUS
{
	unsigned int pad : 1;
	unsigned int localAttached : 1;   // Local device was attached when it's 1
	unsigned int localSpeed : 1;      // Local device is running at super speed (USB 3.0) when it's 1 
	unsigned int localSuspend : 1;    // Local device was suspend when it's 1
	unsigned int pad1 : 5;
	unsigned int remoteAttached : 1;  // Remote device was attached when it's 1
	unsigned int remoteSpeed : 1;     // Remote device is running at super speed (USB 3.0) when it's 1 
	unsigned int remoteSuspend : 1;   // Remote device was suspend when it's 1
	unsigned int pad2 : 4;		      
} DEV_STATUS;
/*
   Get Device Statuses
   DEV_STATUS is a 2-byte array. Byte 0 is for local device status and byte 1 is for remote device status.
   For each byte, the following bits indicate:
   Bit0: suspend bit, 1 represent suspend
   Bit1: unplug bit, 1 represent unplug and 0 represent attached
*/
#define VENDOR_SPECIFIC_REQ_GET_STATUS(DEV_HANDLE, DEV_STATUS)   \
	libusb_control_transfer(DEV_HANDLE, 0xC0, 0xF1, 0, 0, DEV_STATUS, DEVICE_STATUS_LEN, 500)   

/*
   Manually switch to specific speed mode
   wIndex = 0x0200 means switching to high speed mode
   wIndex = 0x0300 means switching to super speed mode
*/
#define VENDOR_SPECIFIC_SWITCH_TO_HIGH_SPEED(DEV_HANDLE)   \
	libusb_control_transfer(dev_handle, 0x40, 0xF2, 0x7B06, 0x0200, NULL, 0, 500);
#endif