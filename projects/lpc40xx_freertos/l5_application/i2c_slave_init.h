#include <stdio.h>

volatile uint8_t slave_memory[256];

void i2c1__slave_init(uint8_t slave_address_to_respond_to);