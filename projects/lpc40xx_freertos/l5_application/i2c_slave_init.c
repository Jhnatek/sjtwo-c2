#include "i2c_slave_init.h"
#include "FreeRTOS.h"
#include "gpio.h"
#include "i2c.h"
#include "i2c_slave_functions.h"
#include "lpc40xx.h"
#include <stdbool.h>
#include <stdio.h>

static void i2c_1_peripheral_init(void) { // from peripherals_init
  const uint32_t i2c_speed_hz = UINT32_C(400) * 1000;
  i2c__initialize(I2C__1, i2c_speed_hz, clock__get_peripheral_clock_hz());

  for (unsigned slave_address = 2; slave_address <= 254; slave_address += 2) {
    if (i2c__detect(I2C__1, slave_address)) {
      printf("I2C slave detected at address: 0x%02X\n", slave_address);
    }
  }
}

static void i2c_2_peripheral_init(void) { // from peripherals_init
  const uint32_t i2c_speed_hz = UINT32_C(400) * 1000;
  i2c__initialize(I2C__2, i2c_speed_hz, clock__get_peripheral_clock_hz());
}

static void i2c_1_set_as_slave(void) {
  LPC_I2C1->CONSET = 0x44; // slave mode
}

static void i2c_1_pin_init(void) {
  LPC_IOCON->P0_19 |= (1 << 10);
  LPC_IOCON->P0_20 |= (1 << 10);
  gpio__construct_with_function(0, 19, GPIO__FUNCTION_3); // i2c1_SDA
  gpio__construct_with_function(0, 20, GPIO__FUNCTION_3); // i2c1_SCL
}

static void i2c_2_pin_init(void) {
  LPC_IOCON->P2_30 |= (1 << 10); //compare with your main
  LPC_IOCON->P2_31 |= (1 << 10);
  gpio__construct_with_function(2, 30, GPIO__FUNCTION_2); // i2c2_SDA
  gpio__construct_with_function(2, 31, GPIO__FUNCTION_2); // i2c2_SCL
}

void i2c1__slave_init(uint8_t slave_address_to_respond_to) {
  i2c_1_peripheral_init();
  i2c_1_pin_init();
  LPC_I2C1->ADR0 = slave_address_to_respond_to;
  i2c_1_set_as_slave();
}
void i2c2__master_init(void) {
  i2c_2_peripheral_init();
  i2c_2_pin_init();
}
