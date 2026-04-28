/*
 * accel.c
 *
 *  Created on: Apr 24, 2026
 *      Author: Brian
 */

#include "sl_icm20648.h" // Official component header
#include <stdio.h>
#include "DCMA.h"

void print_accel_data_official(void) {
    sl_status_t status;
    float accel[3]; // Buffer for X, Y, Z in 'g' units
    char buffer[200];
    int len;
    sl_icm20648_init();
    for(volatile int i=0; i<1000000; i++);
    // 1. Read the acceleration (The component handles SPI and math)
    status = sl_icm20648_accel_read_data(accel);

    init_usart_ldma_output();

    if (status == SL_STATUS_OK) {
        // 2. Format the floats.
        // Note: If %f doesn't work in your console, use (int)(accel[0]*1000) for mg
        len = sprintf(buffer, "Accel X: %",
                      (int)(accel[0]*1000));

        // 3. Send to your terminal via LDMA
        int test = 0;
        test = test +1;
        printc(buffer, (uint32_t)len);
    } else {
        // Error handling
        char err[] = "Error: Sensor read failed\r\n";
        printc(err, sizeof(err));
    }
}

