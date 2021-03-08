#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"

#include "board_io.h"
#include "common_macros.h"
#include "periodic_scheduler.h"
#include "sj2_cli.h"

// 'static' to make these functions 'private' to this file

<<<<<<< Updated upstream
// flash: python nxp-programmer/flash.py

int main(void) {
  create_blinky_tasks();
  create_uart_task();

  // If you have the ESP32 wifi module soldered on the board, you can try uncommenting this code
  // See esp32/README.md for more details
  // uart3_init();                                                                     // Also include:  uart3_init.h
  // xTaskCreate(esp32_tcp_hello_world_task, "uart3", 1000, NULL, PRIORITY_LOW, NULL); // Include esp32_task.h

  puts("Starting RTOS,  why hello there world");
  // LAB 2

  xTaskCreate(task_one, "aaaaaa", 1000, NULL, PRIORITY_LOW, NULL);
  xTaskCreate(task_two, "bbbbbb", 1000, NULL, PRIORITY_HIGH, NULL);

  vTaskStartScheduler(); // This function never returns unless RTOS scheduler runs out of memory and fails

  return 0;
}

static void create_blinky_tasks(void) {
  /**
   * Use '#if (1)' if you wish to observe how two tasks can blink LEDs
   * Use '#if (0)' if you wish to use the 'periodic_scheduler.h' that will spawn 4 periodic tasks, one for each LED
   */
#if (1)
  // These variables should not go out of scope because the 'blink_task' will reference this memory
  static gpio_s led0, led1;

  led0 = board_io__get_led0();
  led1 = board_io__get_led1();

  xTaskCreate(blink_task, "led0", configMINIMAL_STACK_SIZE, (void *)&led0, PRIORITY_LOW, NULL);
  xTaskCreate(blink_task, "led1", configMINIMAL_STACK_SIZE, (void *)&led1, PRIORITY_LOW, NULL);
#else
  const bool run_1000hz = true;
  const size_t stack_size_bytes = 2048 / sizeof(void *); // RTOS stack size is in terms of 32-bits for ARM M4 32-bit CPU
  periodic_scheduler__initialize(stack_size_bytes, !run_1000hz); // Assuming we do not need the high rate 1000Hz task
  UNUSED(blink_task);
#endif
=======
#include "FreeRTOS.h"
#include "adc.h"
#include "gpio_isr.h"
#include "lpc40xx.h"
#include "lpc_peripherals.h"
#include "pwm1.h"
#include "semphr.h"
#include "task.h"

static QueueHandle_t adc_to_pwm_task_queue;

// flash: python nxp-programmer/flash.py

void pwm_task(void *p) {        // basically PWM is the percent of the cycle that it is on
  pwm1__init_single_edge(1000); // sets frequency

  // Locate a GPIO pin that a PWM channel will control
  // NOTE You can use gpio__construct_with_function() API from gpio.h
  // LPC_IOCON->P2_0 &= ~(1 << 7); // this works because 8:7 are always 0 (This is for me, not for lab)
  gpio__construct_with_function(2, 0, GPIO__FUNCTION_1);

  // We only need to set PWM configuration once, and the HW will drive
  // the GPIO at 1000Hz, and control set its duty cycle to 50%

  // Continue to vary the duty cycle in the loop
  uint8_t percent = 0;
  int adc_reading = 0;
  while (1) {
    if (xQueueReceive(adc_to_pwm_task_queue, &adc_reading, 100)) { // 100 is same as wait 100 for signal
      percent = ((double)adc_reading / 4096) * 100;                //(adc_voltage / 3.3) = (adc_reading / 4095)
      pwm1__set_duty_cycle(PWM1__2_0, percent);
    }
  }
>>>>>>> Stashed changes
}

void adc_task(void *p) { // analog to digital converter
  adc__initialize();

  // TODO This is the function you need to add to adc.h
  // You can configure burst mode for just the channel you are using
  adc__enable_burst_mode();

  // Configure a pin, such as P1.31 with FUNC 011 to route this pin as ADC channel 5
  // You can use gpio__construct_with_function() API from gpio.h
  LPC_IOCON->P1_31 &= ~(1 << 7);                          // this sets it in analog mode
  gpio__construct_with_function(1, 31, GPIO__FUNCTION_3); // ADC[5]
  int adc_reading = 0; // Note that this 'adc_reading' is not the same variable as the one from adc_task
  while (1) {
    // Get the ADC reading using a new routine you created to read an ADC burst reading
    // TODO: You need to write the implementation of this function
    // const uint16_t adc_value = adc__get_channel_reading_with_burst_mode(ADC__CHANNEL_5);
    // fprintf(stderr, "%d\n", adc_value);
    adc_reading = adc__get_channel_reading_with_burst_mode(ADC__CHANNEL_5); // you need to constantly get reading
    xQueueSend(adc_to_pwm_task_queue, &adc_reading, 0);
    vTaskDelay(100);
  }
}

void main(void) {
  adc_to_pwm_task_queue = xQueueCreate(1, sizeof(int));
  xTaskCreate(pwm_task, "LED", 2048 / sizeof(void *), NULL, PRIORITY_LOW, NULL);
  xTaskCreate(adc_task, "potentiometer", 2048 / sizeof(void *), NULL, PRIORITY_LOW, NULL);
  vTaskStartScheduler();
}
