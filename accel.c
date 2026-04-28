/*
 * accel.c
 *
 *  Created on: Apr 24, 2026
 *      Author: Brian
 */

#include "em_usart.h"
#include "em_gpio.h"
#include "em_cmu.h"
#include <stdio.h>
#include "DCMA.h"

// USART1 SPI configuration for ICM20648 on Thunderboard
#define SPI_PORT              gpioPortA
#define SPI_CLK_PIN           5   // PA05
#define SPI_TX_PIN            7   // PA07 (MOSI)
#define SPI_RX_PIN            6   // PA06 (MISO)
#define SPI_CS_PORT           gpioPortB
#define SPI_CS_PIN            2   // PB02

// ICM20648 SPI Commands
#define ICM20648_CHIP_ID      0xE1
#define ICM20648_WHO_AM_I     0x00

// Simple SPI read/write
uint8_t spi_transfer(uint8_t byte) {
    USART1->TXDATA = byte;
    while (!(USART1->STATUS & USART_STATUS_RXDATAV));
    return USART1->RXDATA;
}

uint8_t icm20648_read_register(uint8_t reg) {
    // Set read bit (0x80) and send register address
    GPIO_PinOutClear(SPI_CS_PORT, SPI_CS_PIN);  // CS low
    spi_transfer(reg | 0x80);  // Read command
    uint8_t value = spi_transfer(0x00);  // Read data byte
    GPIO_PinOutSet(SPI_CS_PORT, SPI_CS_PIN);   // CS high
    return value;
}

void init_icm20648_spi(void) {
    char debug1[] = "SPI: Enabling clocks\r\n";
    printc(debug1, sizeof(debug1) - 1);

    // 1. Enable clocks
    CMU_ClockEnable(cmuClock_GPIO, true);
    CMU_ClockEnable(cmuClock_USART1, true);

    char debug2[] = "SPI: Configuring GPIO\r\n";
    printc(debug2, sizeof(debug2) - 1);

    // 2. Configure GPIO pins for USART1 SPI mode
    GPIO_PinModeSet(SPI_PORT, SPI_CLK_PIN, gpioModePushPull, 0);   // CLK PA05
    GPIO_PinModeSet(SPI_PORT, SPI_TX_PIN, gpioModePushPull, 1);    // MOSI PA07
    GPIO_PinModeSet(SPI_PORT, SPI_RX_PIN, gpioModeInputPull, 1);   // MISO PA06 (input)
    GPIO_PinModeSet(SPI_CS_PORT, SPI_CS_PIN, gpioModePushPull, 1); // CS PB02 (inactive high)

    char debug3[] = "SPI: Setting up USART1 sync\r\n";
    printc(debug3, sizeof(debug3) - 1);

    // 3. Configure USART1 as SPI master
    USART_InitSync_TypeDef spiInit = USART_INITSYNC_DEFAULT;
    spiInit.baudrate = 1000000;  // 1 MHz for ICM20648
    spiInit.databits = usartDatabits8;
    spiInit.clockMode = usartClockMode0; // CPOL=0, CPHA=0
    spiInit.master = true;
    spiInit.autoCsEnable = false;

    USART_InitSync(USART1, &spiInit);

    char debug4[] = "SPI: Setting up routes\r\n";
    printc(debug4, sizeof(debug4) - 1);

    // 4. Route USART1 to the configured pins
    GPIO->USARTROUTE[1].TXROUTE = (SPI_PORT << _GPIO_USART_TXROUTE_PORT_SHIFT)
                                 | (SPI_TX_PIN << _GPIO_USART_TXROUTE_PIN_SHIFT);
    GPIO->USARTROUTE[1].RXROUTE = (SPI_PORT << _GPIO_USART_RXROUTE_PORT_SHIFT)
                                 | (SPI_RX_PIN << _GPIO_USART_RXROUTE_PIN_SHIFT);
    GPIO->USARTROUTE[1].CLKROUTE = (SPI_PORT << _GPIO_USART_CLKROUTE_PORT_SHIFT)
                                  | (SPI_CLK_PIN << _GPIO_USART_CLKROUTE_PIN_SHIFT);
    GPIO->USARTROUTE[1].ROUTEEN = GPIO_USART_ROUTEEN_TXPEN
                                 | GPIO_USART_ROUTEEN_RXPEN
                                 | GPIO_USART_ROUTEEN_CLKPEN;

    char debug5[] = "SPI: Testing chip ID\r\n";
    printc(debug5, sizeof(debug5) - 1);

    // 5. Test reading chip ID
    uint8_t chip_id = icm20648_read_register(ICM20648_WHO_AM_I);

    char chip_msg[50];
    int len = sprintf(chip_msg, "ICM20648 Chip ID: 0x%02X\r\n", chip_id);
    printc(chip_msg, (uint32_t)len);

    if (chip_id == ICM20648_CHIP_ID) {
        char success[] = "Sensor found!\r\n";
        printc(success, sizeof(success) - 1);
    } else {
        char fail[] = "Sensor NOT found\r\n";
        printc(fail, sizeof(fail) - 1);
    }
}

void print_accel_data_official(void) {
    sl_status_t status;
    float accel[3];  // X, Y, Z in 'g' units
    char buffer[200];
    int len;

    // Read accelerometer data
    status = sl_icm20648_accel_read_data(accel);

    if (status == SL_STATUS_OK) {
        // Format acceleration data (convert to millig for better precision)
        len = sprintf(buffer, "Accel X: %d mg, Y: %d mg, Z: %d mg\r\n",
                      (int)(accel[0] * 1000),
                      (int)(accel[1] * 1000),
                      (int)(accel[2] * 1000));

        printc(buffer, (uint32_t)len);
    } else {
        char err[] = "Error: Sensor read failed\r\n";
        printc(err, sizeof(err));
    }
}

