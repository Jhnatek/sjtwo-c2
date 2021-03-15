#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

void ssp2__init(uint32_t clock_freq);

uint8_t ssp2_lab__exchange_byte(uint8_t data_out);

void flash_activate(void);

void flash_deactivate(void);