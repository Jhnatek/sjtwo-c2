#include "ssp2_lab.h"
#include "clock.h"
#include "lpc40xx.h"
#include "lpc_peripherals.h"
// for historic reason, ssp is spi

static void ssp2_init_pins(void) {
  // Mosi0-1 and Miso0-1 are in MCU
  // SCK1 is in expansion port J6, SCK0 is in Expansion port J5
  // SCK2 = p1.0, MOSI2 = p1.1, MISO2 = p1.4, CS (for flash) = P1.10
  LPC_IOCON->P1_0 &= ~0b111;
  LPC_IOCON->P1_1 &= ~0b111;
  LPC_IOCON->P1_4 &= ~0b111;
  LPC_IOCON->P1_10 &= ~0b111; // just to ensure all bits are 0
  LPC_IOCON->P1_0 |= 0b100;   // sk2
  LPC_IOCON->P1_1 |= 0b100;   // mosi2
  LPC_IOCON->P1_4 |= 0b100;   // miso2
  // set all to 4, because thats to use their respective flash (table 84)
}

static void ssp2_init_peripheral(void) {
  // in PCONP register, sert bit PCSSP0 to enable SSP0 and bit PCSSP1 to enable SSP1
  // pcssp2 is bit 20, an dwill power on ssp2, active high
  LPC_SC->PCONP |= (1 << 20); // power on peripheral, why SC?
}

static void ssp2_init_reg(void) {
  LPC_SSP2->CR0 = (0b111 << 0) | (0b00 << 4);
  LPC_SSP2->CR1 = (0b1 << 1);
}

static void ssp2_init_cpsr(uint32_t clock_freq) {
  uint8_t spr = 2;
  uint32_t cpu_clock = 96;
  while (clock_freq < (cpu_clock / spr)) {
    spr += 2;
  }

  LPC_SSP2->CPSR = spr;
  // max frequency is 104MHz, but 24 is about max on copper
}

void ssp2__init(uint32_t clock_freq) { // 256 kb of memory in flash
  // a) Power on Peripheral
  ssp2_init_pins();
  ssp2_init_peripheral();
  // b) Setup control registers CR0 and CR1
  ssp2_init_reg();
  // c) Setup prescalar register to be <= max_clock_mhz
  ssp2_init_cpsr(clock_freq);
  // SPI clock rate equation: PCLK/(CPSDVSR * [SCR+1])
  // pretty much its just pclk/cpsdvsr
  LPC_GPIO1->DIR |= (1 << 10);
  LPC_GPIO1->SET = (1 << 10);
}

uint8_t ssp2_lab__exchange_byte(uint8_t data_out) { // data out should be 0x9F

  LPC_SSP2->DR = data_out; // generate clocks to send byte
  // Configure the Data register(DR) to send and receive data by checking the SPI peripheral status register
  while (LPC_SSP2->SR & (1 << 4)) {
    ; // wait to complete the transfer
  }
  return (LPC_SSP2->DR); // same bi directional register
}