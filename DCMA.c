#include "em_device.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "em_usart.h"
#include "em_ldma.h"

#define TX_PORT     gpioPortA
#define TX_PIN      5
#define LDMA_CH     0

// Thunderboard BG22 VCOM Enable pin is PA00
#define VCOM_ENABLE_PORT gpioPortA
#define VCOM_ENABLE_PIN  0

static LDMA_TransferCfg_t ldmaTxConfig;
static LDMA_Descriptor_t ldmaTxDescriptor;



/**
 * Initialize USART0 and LDMA hardware
 */
void init_usart_ldma_output(void) {
  // 1. Enable Clocks
  CMU_ClockEnable(cmuClock_GPIO, true);
  CMU_ClockEnable(cmuClock_LDMA, true);
  CMU_ClockEnable(cmuClock_USART0, true);

  // 2. Enable VCOM Bridge (CRITICAL for Thunderboard)
  GPIO_PinModeSet(VCOM_ENABLE_PORT, VCOM_ENABLE_PIN, gpioModePushPull, 1);

  // 3. Configure UART TX pin
  GPIO_PinModeSet(TX_PORT, TX_PIN, gpioModePushPull, 1);

  // 4. Initialize USART0
  USART_InitAsync_TypeDef usartInit = USART_INITASYNC_DEFAULT;
  usartInit.baudrate = 115200; // Standard for VCOM
  USART_InitAsync(USART0, &usartInit);

  // 5. Correct Routing for Series 2 (BG22)
  // USARTROUTE is not an array on this specific peripheral for BG22
  GPIO->USARTROUTE[0].TXROUTE = (TX_PORT << _GPIO_USART_TXROUTE_PORT_SHIFT)
                                | (TX_PIN << _GPIO_USART_TXROUTE_PIN_SHIFT);
  GPIO->USARTROUTE[0].ROUTEEN = GPIO_USART_ROUTEEN_TXPEN;

  // 6. Initialize LDMA & NVIC
  LDMA_Init_t ldmaInit = LDMA_INIT_DEFAULT;
  LDMA_Init(&ldmaInit);

  NVIC_ClearPendingIRQ(LDMA_IRQn);
  NVIC_EnableIRQ(LDMA_IRQn);
}

/**
 * Start a DMA transfer from memory to USART
 */
void printc(void *buffer, uint32_t length) {
  // Wait for any active transfer to finish


     // Correct Trigger for Series 2
     ldmaTxConfig = (LDMA_TransferCfg_t)LDMA_TRANSFER_CFG_PERIPHERAL(ldmaPeripheralSignal_USART0_TXBL);

     ldmaTxDescriptor = (LDMA_Descriptor_t)LDMA_DESCRIPTOR_SINGLE_M2P_BYTE(
                           buffer,
                           (uint32_t)&(USART0->TXDATA),
                           length);

     ldmaTxDescriptor.xfer.doneIfs = 1;

     LDMA_StartTransfer(LDMA_CH, &ldmaTxConfig, &ldmaTxDescriptor);

     for(volatile long i = 0; i < 1000000; i++);
}

void LDMA_IRQHandler(void) {
  uint32_t pending = LDMA_IntGetEnabled();

    if (pending & LDMA_IF_ERROR) {
      LDMA->IF_CLR = LDMA_IF_ERROR;
    }

    uint32_t ch_mask = (1 << LDMA_CH);
    if (pending & ch_mask) {
      LDMA->IF_CLR = ch_mask;
      // Optional: Wait for hardware shift register to empty
      while (!(USART0->STATUS & USART_STATUS_TXC));
    }
  }

