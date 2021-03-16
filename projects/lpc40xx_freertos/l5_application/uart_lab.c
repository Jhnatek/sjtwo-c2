#pragma once
#include "uart_lab.h"
#include "FreeRTOS.h"
#include "lpc40xx.h"
#include "lpc_peripherals.h"
#include "queue.h"
#include "task.h"
#include <stdio.h>

static QueueHandle_t your_uart_rx_queue;

// part 0
static void uart_lab__baud_init(uart_number_e uart, uint32_t peripheral_clock, uint32_t baud_rate) {
  uint32_t temp_clock = (peripheral_clock * 1000 * 1000); // some reason it doesnt like temp_clock, but it works
  const uint16_t sixteen_bit = temp_clock / (16 * baud_rate);
  if (uart == UART_2) {
    LPC_UART2->LCR |= (1 << 7);                 // dlab
    LPC_UART2->FDR = (0 << 0) | (1 << 4);       // divval|mulval
    LPC_UART2->DLL = (sixteen_bit & 0xFF);      // lower 8 bit values
    LPC_UART2->DLM = (sixteen_bit >> 8) & 0xFF; // upper 8 bit values
    LPC_UART2->LCR &= ~(1 << 7);                // disable dlab (after this you dont use this, only thr and rbr)

  } else if (uart == UART_3) {
    LPC_UART3->LCR |= (1 << 7);                 // dlab
    LPC_UART3->FDR = (0 << 0) | (1 << 4);       // divval|mulval
    LPC_UART3->DLL = (sixteen_bit & 0xFF);      // lower 8 bit values
    LPC_UART3->DLM = (sixteen_bit >> 8) & 0xFF; // upper 8 bit values
    LPC_UART3->LCR &= ~(1 << 7);                // disable dlab (after this you dont use this, only thr and rbr)i
  } else {
    fprintf(stderr, "Error: not UART 2 or 3 (baud funct)\n");
  }
}

static void uart_lab__iocon_init(uart_number_e uart) {
  if (uart == UART_2) {
    LPC_IOCON->P2_8 &= ~(0b111); // clear
    LPC_IOCON->P2_9 &= ~(0b111); // clear
    LPC_IOCON->P2_8 |= (0b010);  // U2_TXD in
    LPC_IOCON->P2_9 |= (0b010);  // U2_RXD out

  } else if (uart == UART_3) {
    LPC_IOCON->P0_0 &= ~(0b111); // clear
    LPC_IOCON->P0_1 &= ~(0b111); // clear
    LPC_IOCON->P0_0 |= (0b010);  // U3_TXD in
    LPC_IOCON->P0_1 |= (0b010);  // U3_RXD out
  } else {
    fprintf(stderr, "Error: not UART 2 or 3 iocon\n");
  }
}

void uart_lab__init(uart_number_e uart, uint32_t peripheral_clock, uint32_t baud_rate) {

  if (uart == UART_2) {
    LPC_SC->PCONP |= (1 << 24);   // uart 2 pconp (only real difference)
    LPC_UART2->LCR |= (0x3 << 0); // select length to be 8 bit using WLS
    uart_lab__iocon_init(uart);
    uart_lab__baud_init(uart, peripheral_clock, baud_rate);

  } else if (uart == UART_3) {
    LPC_SC->PCONP |= (1 << 25);   // uart 3 pconp (only real difference)
    LPC_UART3->LCR |= (0x3 << 0); // select length to be 8 bit using WLS
    uart_lab__iocon_init(uart);
    uart_lab__baud_init(uart, peripheral_clock, baud_rate);
  } else {
    fprintf(stderr, "Error: not UART 2 or 3 \n");
  }
}

bool uart_lab__polled_get(uart_number_e uart, char *input_byte) {
  if (uart == UART_2) {
    bool checkempty = (LPC_UART2->LSR & (1 << 0));
    // if (checkempty) {
    //   *input_byte = LPC_UART2->RBR;
    // }
    while (!checkempty) {
      ;
    }
    *input_byte = LPC_UART2->RBR;
  }

  else if (uart == UART_3) {
    bool checkempty = (LPC_UART3->LSR & (1 << 0));
    // if (checkempty) {
    //   *input_byte = LPC_UART3->RBR;
    // }
    while (!checkempty) {
      ;
    }
    *input_byte = LPC_UART3->RBR;
  } else {
    fprintf(stderr, "Error: not UART 2 or 3 polled get\n");
  }
}

bool uart_lab__polled_put(uart_number_e uart, char output_byte) {
  if (uart == UART_2) {
    bool checkempty = (LPC_UART2->LSR & (1 << 5));
    // if (checkempty) {
    //   LPC_UART2->THR = output_byte;
    // }
    while (!checkempty) {
      ;
    }
    LPC_UART2->THR = output_byte;
  }

  else if (uart == UART_3) {
    bool checkempty = (LPC_UART3->LSR & (1 << 5));
    // if (checkempty) {
    //   LPC_UART3->THR = output_byte;
    // }
    while (!checkempty) {
      ;
    }
    LPC_UART3->THR = output_byte;
  } else {
    fprintf(stderr, "Error: not UART 2 or 3 polled put\n");
  }
}

// lab part 2
static void uart3_receive_interrupt(void) {
  bool checkIIR = (LPC_UART3->IIR & (0x2 << 0));
  bool check_LSR = (LPC_UART3->LSR & (0x1 << 0));
  if (checkIIR) {
    if (check_LSR) {
      const char byte = LPC_UART3->RBR;
      xQueueSendFromISR(your_uart_rx_queue, &byte, NULL);
    }
  }
}

static void uart2_receive_interrupt(void) {
  fprintf(stderr, "uart2 interrupt received \n");
  bool checkIIR = (!(LPC_UART2->IIR & (0x0 << 0)));
  bool check_LSR = (LPC_UART2->LSR & (0x1 << 0));
  if (checkIIR) {
    fprintf(stderr, "uart2 interrupt received: checkIRR passed \n");
    if (LPC_UART2->LSR & (0x1 << 0)) {
      fprintf(stderr, "uart2 interrupt received: check_lsr passed \n");
      const char byte = LPC_UART2->RBR;
      xQueueSendFromISR(your_uart_rx_queue, &byte, NULL);
    }
  }
}

void uart__enable_receive_interrupt(uart_number_e uart_number) {
  if (uart_number == UART_2) {
    NVIC_EnableIRQ(UART2_IRQn);
    lpc_peripheral__enable_interrupt(LPC_PERIPHERAL__UART2, uart2_receive_interrupt, NULL);
    // LPC_UART2->IER |= (0b11);
    LPC_UART2->IER |= (1 << 0); // RBR interrupt enable
    LPC_UART2->IER |= (1 << 1); // THR interrupt enable
  }

  else if (uart_number == UART_3) {
    NVIC_EnableIRQ(UART3_IRQn);
    lpc_peripheral__enable_interrupt(LPC_PERIPHERAL__UART3, uart3_receive_interrupt, NULL);
    LPC_UART2->IER |= (0b11);
    // LPC_UART3->IER |= (1 << 0); // RBR interrupt enable
    // LPC_UART3->IER |= (1 << 1); // THR interrupt enable
  } else {
    fprintf(stderr, "Error: not UART 2 or 3 receive interrupt\n");
  }
  // TODO: Create your RX queue
  your_uart_rx_queue = xQueueCreate(5, sizeof(char));
}

// Public function to get a char from the queue (this function should work without modification)
// TODO: Declare this at the header file
bool uart_lab__get_char_from_queue(char *input_byte, uint32_t timeout) {
  return xQueueReceive(your_uart_rx_queue, input_byte, timeout);
}