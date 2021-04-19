#include "i2c_slave_init.h"
#include "gpio.h"
#include "i2c.h"
#include "i2c_slave_functions.h"
#include "lpc40xx.h"
#include <stdio.h>

void i2c2__master_init() {
  i2c_2_peripheral_init();
  i2c_2_pin_init();
}

void i2c_1_peripheral_init(void) { // from peripherals_init
  const uint32_t i2c_speed_hz = UINT32_C(400) * 1000;
  i2c__initialize(I2C__1, i2c_speed_hz, clock__get_peripheral_clock_hz());

  /*for (unsigned slave_address = 2; slave_address <= 254; slave_address += 2) {
    if (i2c__detect(I2C__1, slave_address)) {
      printf("I2C slave detected at address: 0x%02X\n", slave_address);
    }
  }*/
}
/*
void i2c_2_peripheral_init(void) { // from peripherals_init
  const uint32_t i2c_speed_hz = UINT32_C(400) * 1000;
  i2c__initialize(I2C__2, i2c_speed_hz, clock__get_peripheral_clock_hz());
}*/

void i2c_1_pin_init(void) {
  LPC_IOCON->P0_0 |= (1 << 10);
  LPC_IOCON->P0_1 |= (1 << 10);
  gpio__construct_with_function(0, 0, GPIO__FUNCTION_3); // i2c1_SDA
  gpio__construct_with_function(0, 1, GPIO__FUNCTION_3); // i2c1_SCL
}
/*
void i2c_2_pin_init(void) {
  gpio__construct_with_function(2, 30, GPIO__FUNCTION_2); // i2c2_SDA
  gpio__construct_with_function(2, 31, GPIO__FUNCTION_2); // i2c2_SCL
}*/

void set_i2c1_as_slave(void) {
  LPC_I2C1->CONSET = 0x44; // slave mode
}

void i2c1__slave_init(uint8_t slave_address_to_respond_to) {
  i2c_1_peripheral_init();
  i2c_1_pin_init();
  LPC_I2C1->ADR0 = slave_address_to_respond_to;
  set_i2c1_as_slave();
}

bool i2c_slave_callback__read_memory(uint8_t memory_index, uint8_t *memory) {
  if (memory_index < 256) {
    *memory = slave_memory[memory_index];
    return true;
  }
  return false;
}

// as long as memory index within range, we can write in that specific location
bool i2c_slave_callback__write_memory(uint8_t memory_index, uint8_t memory_value) {
  if (memory_index < 256) {
    slave_memory[memory_index] = memory_value;
    return true;
  }
  return false;
}