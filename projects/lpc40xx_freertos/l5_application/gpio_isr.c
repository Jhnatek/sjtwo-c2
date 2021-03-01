// @file gpio_isr.c
#include "gpio_isr.h"
#include "FreeRTOS.h"
#include "lpc40xx.h"
#include <stdio.h>

// Note: You may want another separate array for falling vs. rising edge callbacks
static function_pointer_t gpio0_callbacks[64];
// static function_pointer_t gpio1_callbacks[64];  not really needed
static function_pointer_t gpio2_callbacks[64];

void clear_pin_interrupt(const int port_that_generated_interrupt, const int pin_that_generated_interrupt);

void gpiO__attach_interrupt(uint32_t port, uint32_t pin, gpio_interrupt_e interrupt_type, function_pointer_t callback) {
  // 1) Store the callback based on the pin at gpio0_callbacks
  // 2) Configure GPIO 0 pin for rising or falling edge
  switch (port) {
  case 0:
    if (interrupt_type == GPIO_INTR__RISING_EDGE) {
      gpio0_callbacks[pin] = callback;
      LPC_GPIOINT->IO0IntEnR |= (1 << pin);
      // note that since im just using 3 arrays, falling edge will have to be +32
    }
    if (interrupt_type == GPIO_INTR__FALLING_EDGE) {
      gpio0_callbacks[pin + 32] = callback;
      LPC_GPIOINT->IO0IntEnF |= (1 << pin);
      // note that since im just using 3 arrays, falling edge will have to be +32
    }

    break;

  case 2:
    if (interrupt_type == GPIO_INTR__RISING_EDGE) {
      gpio2_callbacks[pin] = callback;
      LPC_GPIOINT->IO2IntEnR |= (1 << pin);
      // note that since im just using 3 arrays, falling edge will have to be +32
    }
    if (interrupt_type == GPIO_INTR__FALLING_EDGE) {
      gpio2_callbacks[pin + 32] = callback;
      LPC_GPIOINT->IO2IntEnF |= (1 << pin);
      // note that since im just using 3 arrays, falling edge will have to be +32
    }

    break;

    break;
  default:
    fprintf(stderr, "____GPIO attach interrupt port failure____\n");
    break;
  }
}

// We wrote some of the implementation for you
void gpiO__interrupt_dispatcher(void) {
  // Check which pin generated the interrupt
  // Invoke the user registered callback, and then clear the interrupt
  for (int pin = 0; pin < 32; pin++) {
    if (LPC_GPIOINT->IO0IntStatR & (1 << pin)) {
      gpio0_callbacks[pin]();
      clear_pin_interrupt(0, pin);
    }
    if (LPC_GPIOINT->IO0IntStatF & (1 << pin)) {
      gpio0_callbacks[pin + 32]();
      clear_pin_interrupt(0, pin);
    }
    if (LPC_GPIOINT->IO2IntStatR & (1 << pin)) {
      gpio2_callbacks[pin]();
      clear_pin_interrupt(0, pin);
    }
    if (LPC_GPIOINT->IO2IntStatF & (1 << pin)) {
      gpio2_callbacks[pin + 32]();
      clear_pin_interrupt(0, pin);
    }
  }
}

void clear_pin_interrupt(const int port_that_generated_interrupt, const int pin_that_generated_interrupt) {
  switch (port_that_generated_interrupt) {
  case 0:
    LPC_GPIOINT->IO0IntClr |= (1 << pin_that_generated_interrupt);
    break;
  case 2:
    LPC_GPIOINT->IO2IntClr |= (1 << pin_that_generated_interrupt);
    break;
  default:
    fprintf(stderr, "___Clear pin interrupt failure with port___\n");
    break;
  }
}