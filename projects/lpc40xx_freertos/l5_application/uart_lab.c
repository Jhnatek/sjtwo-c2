#pragma once
#include "uart_lab.h"
#include "FreeRTOS.h"
#include "lpc40xx.h"
#include "lpc_peripherals.h"
#include "queue.h"
#include "task.h"
#include <stdio.h>

static void uart_lab__baud_init(uart_number_e uart, uint32_t peripheral_clock, uint32_t baud_rate) {
  uint32_t temp_clock = (peripheral_clock * 1000 * 1000); // some reason it doesnt like temp_clock, but it works
  const uint16_t sixteen_bit = temp_clock / (16 * baud_rate);
  if (uart == UART_2) {
    LPC_UART2->FDR = (0 << 0) | (1 << 4);       // divval|mulval
    LPC_UART2->DLL = (sixteen_bit & 0xFF);      // lower 8 bit values
    LPC_UART2->DLM = (sixteen_bit >> 8) & 0xFF; // upper 8 bit values

  } else if (uart == UART_3) {
    LPC_UART3->FDR = (0 << 0) | (1 << 4);       // divval|mulval
    LPC_UART3->DLL = (sixteen_bit & 0xFF);      // lower 8 bit values
    LPC_UART3->DLM = (sixteen_bit >> 8) & 0xFF; // upper 8 bit values
  } else {
    fprintf(stderr, "Error: not UART 1 or 2 (baud funct)\n");
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
    fprintf(stderr, "Error: not UART 1 or 2 iocon\n");
  }
}

void uart_lab__init(uart_number_e uart, uint32_t peripheral_clock, uint32_t baud_rate) {

  if (uart == UART_2) {
    LPC_SC->PCONP |= (1 << 24);   // uart 2 pconp (only real difference)
    LPC_UART2->LCR |= (1 << 7);   // dlab
    LPC_UART2->LCR |= (0x3 << 0); // select length to be 8 bit using WLS
    uart_lab__iocon_init(uart);
    uart_lab__baud_init(uart, peripheral_clock, baud_rate);

    LPC_UART2->LCR &= ~(1 << 7); // disable dlab (after this you dont use this, only thr and rbr)

  } else if (uart == UART_3) {
    LPC_SC->PCONP |= (1 << 25);   // uart 3 pconp (only real difference)
    LPC_UART3->LCR |= (1 << 7);   // dlab
    LPC_UART3->LCR |= (0x3 << 0); // select length to be 8 bit using WLS
    uart_lab__iocon_init(uart);
    uart_lab__baud_init(uart, peripheral_clock, baud_rate);

    LPC_UART3->LCR &= ~(1 << 7); // disable dlab (after this you dont use this, only thr and rbr)
  } else {
    fprintf(stderr, "Error: not UART 1 or 2 \n");
  }
}

bool uart_lab__polled_get(uart_number_e uart, char *input_byte) {
  if (uart == UART_2) {
    bool checkempty = (LPC_UART2->LSR & (1 << 5));
    if (checkempty) {
      *input_byte = LPC_UART2->RBR;
    }
  }

  if (uart == UART_3) {
    bool checkempty = (LPC_UART3->LSR & (1 << 5));
    if (checkempty) {
      *input_byte = LPC_UART3->RBR;
    }
  }
}

bool uart_lab__polled_put(uart_number_e uart, char output_byte) {
  if (uart == UART_2) {
    bool checkempty = (LPC_UART2->LSR & (1 << 5));
    if (checkempty) {
      LPC_UART2->THR = output_byte;
    }
  }

  if (uart == UART_3) {
    bool checkempty = (LPC_UART3->LSR & (1 << 5));
    if (checkempty) {
      LPC_UART3->THR = output_byte;
    }
  }
}