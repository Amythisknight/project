/***************************************************************************//**
 * @file
 * @brief Top level application functions
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/
#include "sl_icm20648.h"
#include "sl_status.h"
#include "i2cspm.h"
#include "accel.h"
#include "DCMA.h"
  char myData[] = "Hello LDMA!\r\n";
/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  uint8_t chip_id = 0;
    sl_status_t status;

    // 1. Initialize your Terminal (LDMA + EUART0 or USART1)
    // Ensure this function no longer uses USART0 if the component is using it!
    init_usart_ldma_output();
    printc(myData, sizeof(myData));

    // 2. Wake up the sensor (The component starts in sleep mode)
    sl_icm20648_init();

    // 3. Verify the Sensor Handshake
    status = sl_icm20648_get_device_id(&chip_id);
    init_usart_ldma_output();

    if (status == SL_STATUS_OK && chip_id == 0xE1) {
      char success[] = "Sensor Initialized: ICM20648 found (0xE1)\r\n";
      printc(success, sizeof(success) - 1);
    } else {
      char error[50];
      int len = sprintf(error, "Sensor Error! ID: 0x%02X\r\n", chip_id);
      printc(error, len);
    }
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{

  i2cspm_app_process_action();
  print_accel_data_official();
  for(volatile int i=0; i<1000000; i++);
}
