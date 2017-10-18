//
//  benchmark.cpp
//  Performance benchmark program
//
//  Created by software team in Prolific on 10/11/2017.
//  Copyright (c) 2017 Prolific Corp.. All rights reserved.
//
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include "libusb.h"
#include "pl2701.h"
#if !defined(_WIN32)
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#if defined(__linux__)
#include <time.h>
#elif defined(__APPLE__)
#include <mach/mach_time.h>
#endif
#endif

static int gProgRole = PROGRAM_ROLE_UNKNOWN;     // The role which this program play
static bool gkilled = false;                     // Be true to kill the thread and then exit this program
static int g_bulk_interface_no = 0;              // The interface No. you wish to claim
#define DATA_TRANSFER_SIZE    (512 * 2 * 1024)   // 1 MB bytes for each transfer round
#define TRANSFER_ROUND_NO                   50   // The number of rounds sender will transmit

/*
   The called function when the process gets the specified signal(s)
 */
static void signal_handler(int signum)
{
    gkilled = true;
}

/*
   The receiver thread for receiving data
 */
void* receiver_task(void *param)
{
    int return_code;   // Return value of each Prolific API
	unsigned char *recvBuf = (unsigned char *)malloc(sizeof(char) * DATA_TRANSFER_SIZE);   // Buffer for receiving data
    int transferred_size = 0;   // Received data in bytes
	unsigned int transferred_total_size = 0; // Total received data in bytes
    struct libusb_device_handle **dev_handle = (struct libusb_device_handle **)param;   /* Structure of
		 the opened PL2701 device handle */

    printf("Flush Endpoint IN FIFO first\n");
    return_code = libusb_bulk_transfer(
                      *dev_handle,
		              BULK_INTERFACE5_EP1_IN_ADDR,
                      recvBuf,
		              BULK_EP1_FIFO_SIZE,
                      &transferred_size,
		              BULK_USB3_TIMEOUT
                  );

    printf("Start to receive the data\n");
    // This tread continuously receive data from the sender
    while (!gkilled) {
		transferred_size = 0;

        // Perform a USB bulk transfer
        return_code = libusb_bulk_transfer(
                          *dev_handle,
			              BULK_INTERFACE5_EP1_IN_ADDR,
                          recvBuf,
                          DATA_TRANSFER_SIZE,
                          &transferred_size,
			              BULK_USB3_TIMEOUT
                      );

		transferred_total_size += transferred_size;

        if (return_code == LIBUSB_ERROR_TIMEOUT) {
            // libusb keep return LIBUSB_ERROR_TIMEOUT if no data were be received, and this thread
            // keep trying to receive any one
            continue;
        } else if (return_code != LIBUSB_SUCCESS) {
            // Error occurs except LIBUSB_ERROR_TIMEOUT
            printf("Fail receiving data from bulk, return_code = %d\n", return_code);
            break;
        }
		//printf("Total received size = %u\n", transferred_total_size); // Don't show this message while doing benchmark or it'll impact the result significantly.
    }

	free(recvBuf);
    return NULL;
}

/*
   The sender thread for sending data
 */
void* sender_task(void *param)
{
    int i;   // Index for a loop
    int return_code;   // Return value of each Prolific API
	unsigned char *sendBuf = (unsigned char *)malloc(sizeof(char) * DATA_TRANSFER_SIZE);   // Buffer for sending data
    int transferred_size = 0;   // Sent data in bytes
	unsigned int transferred_total_size = 0; // Total sent data in bytes
    struct libusb_device_handle **dev_handle = (struct libusb_device_handle **)param;   /* Structure of
		 the opened PL2701 device handle */
#if defined(_WIN32)		 
	DWORD64 start_time;
	DWORD64 end_time;
#elif defined(__linux__)
    struct timespec start_time, end_time, diff_time;
    double elapsed_time;
#elif defined(__APPLE__)
	uint64_t start_time;
	uint64_t end_time;
	uint64_t diff_time;
	mach_timebase_info_data_t info;
#endif
    // Create the data pattern "sendBuf" to be sent to the receiver
    for (i = 0; i < DATA_TRANSFER_SIZE; i++) {
        sendBuf[i] = i % 0x100;
    }

    printf("Start to send the data\n");
#if defined(_WIN32)	
	start_time = GetTickCount();
#elif defined(__linux__)
    clock_gettime(CLOCK_REALTIME, &start_time);
#elif defined(__APPLE__)
	mach_timebase_info(&info);
	start_time = mach_absolute_time();
#endif

    // This thread send test pattern TRANSFER_ROUND_NO times
    for (i = 0; i < TRANSFER_ROUND_NO && !gkilled; i++) {
        transferred_size = 0;

        // Send the data to the sender
        return_code = libusb_bulk_transfer (
                          *dev_handle,
			              BULK_INTERFACE5_EP1_OUT_ADDR,
                          sendBuf,
                          DATA_TRANSFER_SIZE,
                          &transferred_size,
						  BULK_USB3_TIMEOUT
                      );
		transferred_total_size += transferred_size;

        if (return_code != LIBUSB_SUCCESS) {
            printf("Failed writing data to bulk, return_code = %d\n", return_code);
            break;
        }
    }

	// Show benchmark results
#if defined(_WIN32)	
	end_time = GetTickCount();
	
	DOUBLE performance = ((DOUBLE)DATA_TRANSFER_SIZE * TRANSFER_ROUND_NO * 1000) / (end_time - start_time) / 1024;
	printf("Transferred total size = %u bytes\n", transferred_total_size);
	printf("TickCount = %lld ms\n", end_time - start_time);
	printf("Performance = %.2lf KB/s\n", performance);
#elif defined(__linux__)
    clock_gettime(CLOCK_REALTIME, &end_time);

	if (end_time.tv_nsec - start_time.tv_nsec < 0)
    {
		diff_time.tv_sec = end_time.tv_sec - start_time.tv_sec - 1;
		diff_time.tv_nsec = 1000000000L + end_time.tv_nsec - start_time.tv_nsec;
    }
    else
    {
        diff_time.tv_sec = end_time.tv_sec - start_time.tv_sec;
        diff_time.tv_nsec = end_time.tv_nsec - start_time.tv_nsec;
    }
    elapsed_time = (double)diff_time.tv_sec + (double)(diff_time.tv_nsec) / 1000000000L;
    // printf("DiffTime = %ld %ld\n", diffTime.tv_sec, diffTime.tv_nsec);
    printf("Transferred %u bytes\n", transferred_total_size);
    printf("Elapsed Time = %.2lf sec\n", elapsed_time);
    printf("Performance = %.2lf KB/s\n", (double)transferred_total_size / elapsed_time / 1024);
#elif defined(__APPLE__)
	end_time = mach_absolute_time();
	diff_time = end_time - start_time;
	diff_time *= info.numer;
	diff_time /= info.denom;
	printf("Transferred %u bytes\n", transferred_total_size);
	printf("Elapsed Time = %.2lf sec\n", (double)diff_time / 1000000000);
	printf("Performance = %.2lf KB/s\n", (double)transferred_total_size / ((double)diff_time / 1000000000) / 1024);
#endif

	free(sendBuf);
    return NULL;
}

/*
   Main routine
 */
int main(int argc, char* argv[])
{
    bool found_PL2701_device = false;   // TRUE means PL2701 was found
    int i = 0;   // Indexes for the loops
    libusb_device **devices;   // Structures representing all USB device detected on the system
    libusb_device *device;   // Structure representing one USB device detected on the system
    int return_code = 0;   // Return value from each libusb API
    ssize_t no_of_devices;   // The number of devices in the device list
#if !defined(_WIN32)
    pthread_t thread;   // For thread creation
#endif
    struct libusb_device_descriptor device_descriptor;   // structure represents the standard USB device descriptor
    struct libusb_config_descriptor *config_desc = NULL;   // Structure represents the standard USB configuration descriptor
    struct libusb_device_handle *dev_handle = NULL;   /* Structure represents the handles on a USB device */

    // Check command parameters and Initialize the transmission
    if (argc != 2) {
        printf("Usage:\n\t%s [send/recv]\n", argv[0]);
		return PL_ERROR_WRONG_PARA;
    } else if (strcmp(argv[1], "send") == 0) {
        gProgRole = PROGRAM_ROLE_SENDER;
    } else if (strcmp(argv[1], "recv") == 0) {
        gProgRole = PROGRAM_ROLE_RECEIVER;
    } else {
        printf("Usage:\n\t%s [send/recv]\n", argv[0]);
        return PL_ERROR_WRONG_PARA;
    }

    // Register the signals to exit the program
    signal(SIGINT, signal_handler);
#if !defined(_WIN32)
    signal(SIGKILL, signal_handler);
    signal(SIGQUIT, signal_handler);
    signal(SIGSTOP, signal_handler);
#endif
    signal(SIGILL, signal_handler);

    // Initialize libusb
    return_code = libusb_init(NULL);
    if (return_code < LIBUSB_SUCCESS)
        return return_code;

    // Returns a list of USB devices currently attached to the system
    no_of_devices = libusb_get_device_list(NULL, &devices);
    if (no_of_devices < 1)
        return PL_ERROR_WRONG_DEVICE_NO;

    // printf("Found the folowing devices\n");
    while((device = devices[i++]) != NULL) {
        return_code = libusb_get_device_descriptor(device, &device_descriptor);
        if(return_code < 0) {
            fprintf(stderr, "Failed to get device descriptor");
            return return_code;
        }

        // Find PL2701 USB device
        if((PROLIFIC_VID == device_descriptor.idVendor) && (PL2701_PID == device_descriptor.idProduct)) {
            found_PL2701_device = true;
            DEBUG("(%s, %s(), L%d)\n", __FILE__, __FUNCTION__, __LINE__);
            printf("Found PL2701 USB device!\n");

            // Open a device and obtain a device handle
            return_code = libusb_open(device, &dev_handle);
            if(return_code < 0) {
                DEBUG("(%s, %s(), L%d)\n", __FILE__, __FUNCTION__, __LINE__);
                libusb_close(dev_handle);

                return return_code;
            }

#if defined(__APPLE__)
            // Get a USB configuration descriptor based on its index
            return_code = libusb_get_config_descriptor(device, 0, &config_desc);
#else
            // Get the USB configuration descriptor for the currently active configuration
            return_code = libusb_get_active_config_descriptor(device, &config_desc);
#endif
            if(return_code != 0) {
                // Close a device handle
                libusb_close(dev_handle);
                continue;
            }
            DEBUG("(%s, %s(), L%d) config_desc->bNumInterfaces = %d\n",
                  __FILE__, __FUNCTION__, __LINE__, config_desc->bNumInterfaces);
        }
    }

    // Check whether any PL2701 USB device was found
    if(!found_PL2701_device) {
        printf("No PL2701 USB device was found!\n");
        return PL_ERROR_NO_DEVICE;
    }

#if !defined(_WIN32)
    // Check whether a kernel driver is attached to interface #0. If so, it need to be detached
    if(libusb_kernel_driver_active(dev_handle, g_bulk_interface_no))
    {
        return_code = libusb_detach_kernel_driver(dev_handle, g_bulk_interface_no);
        DEBUG("(%s, %s(), L%d) return_code = %d\n", __FILE__, __FUNCTION__, __LINE__, return_code);
    }
#endif

    // Claim an interface on a given device handle
    return_code = libusb_claim_interface(dev_handle, g_bulk_interface_no);
    DEBUG("(%s, %s(), L%d) return_code = %d\n", __FILE__, __FUNCTION__, __LINE__, return_code);

	// Check local and remote device statuses
	DEV_STATUS dev_status;
	memset((void *)&dev_status, 0xFF, sizeof(DEV_STATUS));
	do {
		// Get device statuses from vendor command
		return_code = VENDOR_SPECIFIC_REQ_GET_STATUS(dev_handle, (unsigned char *)&dev_status);   
		if (return_code < DEVICE_STATUS_LEN) {
			printf("Fail to get PL2701 USB device status!\n");
			return PL_ERROR_WRONG_STATUS;
		}
		printf("Local device status: %s, %s, %s\n", dev_status.localSuspend ? "Suspend" : "Active", 
			dev_status.localAttached ? "Attached" : "Unplug", dev_status.localSpeed ? "Super speed" : "High speed");
		printf("Remote device status: %s, %s, %s\n", dev_status.remoteSuspend ? "Suspend" : "Active",
			dev_status.remoteAttached ? "Attached" : "Unplug", dev_status.remoteSpeed ? "Super speed" : "High speed");
		// Break the loop when the remote device was attached
		if (dev_status.remoteAttached)
			break;
		else {
#if !defined(_WIN32)
			usleep(SLEEP_TIME);
#else
			Sleep(SLEEP_TIME);
#endif
			continue;
		}

	} while (!gkilled);

#if defined(_WIN32)
	// Run sender or receiver for data transfer
	if (gProgRole == PROGRAM_ROLE_RECEIVER) {
		// Run receiver's function
		receiver_task((void*)&dev_handle);
	}
	else if (gProgRole == PROGRAM_ROLE_SENDER) {
		// Run sender's function
		sender_task((void*)&dev_handle);
	}
#else
    // Create one sender and one receiver for data transfer
    if (gProgRole == PROGRAM_ROLE_RECEIVER) {
        // Create a thread to run sender's task.
        return_code = pthread_create(&thread, NULL, receiver_task, (void*) &dev_handle);
        if (return_code != 0) {
            printf("Unable to create sender thread.\n");
            return -1;
        }

        pthread_join(thread, NULL);

    } else if (gProgRole == PROGRAM_ROLE_SENDER) {
        // Create a thread to run receiver's task.
        return_code = pthread_create(&thread, NULL, sender_task, (void*) &dev_handle);
        if (return_code != 0) {
            printf("Unable to create receiver thread.\n");
            return -1;
        }

        pthread_join(thread, NULL);
    }
#endif
    // Frees a list of devices previously discovered using libusb_get_device_list()
    libusb_free_device_list(devices, 1);

    // Exit libusb
    libusb_exit(NULL);
    return PL_ERROR_SUCCESS;
}
