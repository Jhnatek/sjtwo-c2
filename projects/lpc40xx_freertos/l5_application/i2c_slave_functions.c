#include "i2c_slave_functions.h"

static uint8_t slave_memory[256];

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