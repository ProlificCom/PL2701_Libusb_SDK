//
//  listdev.cpp
//  A sample program to list PL2701 device and show its information
//
//  Created by software team in Prolific on 10/05/2017.
//  Copyright (c) 2017 Prolific Corp.. All rights reserved.
//
#include <stdio.h>
#include <string.h>

#include "libusb.h"
#include "pl2701.h"

/*
   Main routine
 */
int main(void)
{
    libusb_device **devices;   // Structures representing all USB device detected on the system
    int return_code = 0;   // Return value from each libusb API
    ssize_t no_of_devices;   // The number of devices in the device list
    int no_of_ports = 0;   // The number of elements filled

    libusb_device *device;   // Structure representing one USB device detected on the system
    int i = 0, j = 0, k = 0;
    uint8_t path[8];   // The array that should contain the port numbers
    struct libusb_device_descriptor device_descriptor;   // structure represents the standard USB device descriptor
    struct libusb_config_descriptor *config_desc = NULL;   // Structure represents the standard USB configuration descriptor
    struct libusb_interface_descriptor interface_descriptor;   // A structure represents the standard USB interface descriptor
    struct libusb_endpoint_descriptor endpoint;   // A structure represents the standard USB endpoint descriptor
    struct libusb_device_handle *dev_handle = NULL;   // Structure represents the handles on a USB device

    // Initialize libusb
    return_code = libusb_init(NULL);
    if (return_code < LIBUSB_SUCCESS)
        return return_code;

    // Returns a list of USB devices currently attached to the system
    no_of_devices = libusb_get_device_list(NULL, &devices);
    if (no_of_devices < 1)
        return (int)no_of_devices;

    printf("Found the folowing devices\n");
    while((device = devices[i++]) != NULL) {
        return_code = libusb_get_device_descriptor(device, &device_descriptor);
        if(return_code < 0) {
            fprintf(stderr, "Failed to get device descriptor");
            return return_code;
        }

        printf("%04x:%04x (bus %d, device %d)",
               device_descriptor.idVendor, device_descriptor.idProduct,
               libusb_get_bus_number(device), libusb_get_device_address(device));

        // Get the list of all port numbers from root for the specified device
        no_of_ports = libusb_get_port_numbers(device, path, sizeof(path));
        if (no_of_ports > 0) {
            printf(" path: %d", path[0]);
            for (k = 1; k < no_of_ports; k++)
                printf(".%d", path[k]);
        }
        printf("\n");

        // Find PL2701 USB device
        if((PROLIFIC_VID == device_descriptor.idVendor) && (PL2701_PID == device_descriptor.idProduct)) {
            // open the target device
            DEBUG("(%s, %s(), L%d)\n", __FILE__, __FUNCTION__, __LINE__);
            printf("--------------------------------------------------------------\n");
            printf("    PL2701 USB device information\n");

            // Open a device and obtain a device handle
            return_code = libusb_open(device, &dev_handle);
            if(return_code < 0) {
                DEBUG("(%s, %s(), L%d) libusb_open fails\n", __FILE__, __FUNCTION__, __LINE__);
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

            // Find all interfaces in PL2701
            for(j = 0; j < config_desc->bNumInterfaces; j++) {
                memcpy(&interface_descriptor, &(config_desc->interface[j].altsetting[0]),
                       sizeof(struct libusb_interface_descriptor));
                DEBUG("(%s, %s(), L%d)\n", __FILE__, __FUNCTION__, __LINE__);
                printf("    No. of endpoints = %d\n", interface_descriptor.bNumEndpoints);

                // Find all endpoints in PL2701
                for(k = 0; k < interface_descriptor.bNumEndpoints; k++) {
                    memcpy(&endpoint, &interface_descriptor.endpoint[k], sizeof(struct libusb_endpoint_descriptor));
                    DEBUG("(%s, %s(), L%d) endpoint.bmAttributes = %x\n", __FILE__, __FUNCTION__, __LINE__,
                          endpoint.bmAttributes);

                    // Check whether it's control endpoint
                    if(endpoint.bmAttributes == LIBUSB_TRANSFER_TYPE_CONTROL)
                    {
                        DEBUG("(%s, %s(), L%d) \n", __FILE__, __FUNCTION__, __LINE__);
                        printf("    Found control endpoint No. %d\n", k);
                    }

                    // Check while it's bulk endpoint
                    if(endpoint.bmAttributes == LIBUSB_TRANSFER_TYPE_BULK)
                    {
                        DEBUG("(%s, %s(), L%d) \n", __FILE__, __FUNCTION__, __LINE__);
                        printf("    Found bulk endpoint No. %d", k);

                        // Check whether it's bulk in
                        if((endpoint.bEndpointAddress & LIBUSB_ENDPOINT_DIR_MASK)
                                == LIBUSB_ENDPOINT_IN) {
                            DEBUG("(%s, %s(), L%d)\n", __FILE__, __FUNCTION__, __LINE__);
                            printf(" at address = 0x%02x\n", endpoint.bEndpointAddress);

                            if(endpoint.bEndpointAddress != BULK_INTERFACE1_EP1_IN_ADDR &&
							   endpoint.bEndpointAddress != BULK_INTERFACE2_EP1_IN_ADDR &&
							   endpoint.bEndpointAddress != BULK_INTERFACE5_EP1_IN_ADDR &&
							   endpoint.bEndpointAddress != BULK_INTERFACE5_EP2_IN_ADDR) {
                                DEBUG("(%s, %s(), L%d)\n", __FILE__, __FUNCTION__, __LINE__);
                                break;
                            }
                        }

                        // Check whether it's bulk out
                        if((endpoint.bEndpointAddress & LIBUSB_ENDPOINT_DIR_MASK)
                                == LIBUSB_ENDPOINT_OUT) {
                            DEBUG("(%s, %s(), L%d)\n", __FILE__, __FUNCTION__, __LINE__);
                            printf(" at address = 0x%02x\n", endpoint.bEndpointAddress);

                            if(endpoint.bEndpointAddress != BULK_INTERFACE1_EP1_OUT_ADDR &&
							   endpoint.bEndpointAddress != BULK_INTERFACE2_EP1_OUT_ADDR &&
							   endpoint.bEndpointAddress != BULK_INTERFACE5_EP1_OUT_ADDR &&
							   endpoint.bEndpointAddress != BULK_INTERFACE5_EP2_OUT_ADDR) {
                                DEBUG("(%s, %s(), L%d)\n", __FILE__, __FUNCTION__, __LINE__);
                                break;
                            }
                        }
                    }
                    // Check while it's interrup endpoint
                    if(endpoint.bmAttributes == LIBUSB_TRANSFER_TYPE_INTERRUPT)
                    {
                        DEBUG("(%s, %s(), L%d)\n", __FILE__, __FUNCTION__, __LINE__);
                        printf("    Found interrupt endpoint No. %d at address = 0x%02x\n", k,
                               endpoint.bEndpointAddress);
                    }
                }
            }

            printf("--------------------------------------------------------------\n");
        }
    }

    // Frees a list of devices previously discovered using libusb_get_device_list()
    libusb_free_device_list(devices, 1);

    libusb_exit(NULL);
    return LIBUSB_SUCCESS;
}
