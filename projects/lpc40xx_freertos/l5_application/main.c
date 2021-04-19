#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"

#include "adc.h" //use this when we implement sensor
// use adc__initialize, and then plug in component to an adc port, then use
// adc__get_adc_value(adc_channel_e channel_num); to get it as i think a uint16_t
#include "acceleration.h"
#include "board_io.h"
#include "common_macros.h"
#include "event_groups.h"
#include "ff.h" //directory for file api (theres a lot of carp here)
#include "periodic_scheduler.h"
#include "queue.h"
#include "sj2_cli.h"
#include <string.h>

#include "i2c.h"
#include "i2c_slave_init.h"
#include "lpc40xx.h"

static void create_uart_task();
static void uart_task(void *params);

// I2c__2 --> i2c__1

static void i2c_task(void *p) {
  LPC_GPIO2->DIR |= (1 << 3);
  while (1) {
    printf("In main i2c task\n");
    if (slave_memory[0]) {
      LPC_GPIO2->CLR |= (1 << 3); // ON
    } else {
      LPC_GPIO2->SET |= (1 << 3); // OFF
    }
    vTaskDelay(1000);
  }
}

int main(void) {
  create_uart_task();
  i2c1__slave_init(0x80);
  if (i2c__detect(I2C__2, 0x80))
    printf("I2C__1 Slave Detected!\n");
  else
    printf("I2C__1 Slave Fails To Detect!\n");
  xTaskCreate(i2c_task, "i2c_task", 1024, NULL, 1, NULL);
  vTaskStartScheduler();
}
static void create_uart_task(void) {
  // It is advised to either run the uart_task, or the SJ2 command-line (CLI), but not both
  // Change '#if (0)' to '#if (1)' and vice versa to try it out
#if (0)
  // printf() takes more stack space, size this tasks' stack higher
  xTaskCreate(uart_task, "uart", (512U * 8) / sizeof(void *), NULL, PRIORITY_LOW, NULL);
#else
  sj2_cli__init();
  UNUSED(uart_task); // uart_task is un-used in if we are doing cli init()
#endif
}
static void uart_task(void *params) {
  TickType_t previous_tick = 0;
  TickType_t ticks = 0;

  while (true) {
    // This loop will repeat at precise task delay, even if the logic below takes variable amount of ticks
    vTaskDelayUntil(&previous_tick, 2000);

    /* Calls to fprintf(stderr, ...) uses polled UART driver, so this entire output will be fully
     * sent out before this function returns. See system_calls.c for actual implementation.
     *
     * Use this style print for:
     *  - Interrupts because you cannot use printf() inside an ISR
     *    This is because regular printf() leads down to xQueueSend() that might block
     *    but you cannot block inside an ISR hence the system might crash
     *  - During debugging in case system crashes before all output of printf() is sent
     */
    ticks = xTaskGetTickCount();
    fprintf(stderr, "%u: This is a polled version of printf used for debugging ... finished in", (unsigned)ticks);
    fprintf(stderr, " %lu ticks\n", (xTaskGetTickCount() - ticks));

    /* This deposits data to an outgoing queue and doesn't block the CPU
     * Data will be sent later, but this function would return earlier
     */
    ticks = xTaskGetTickCount();
    printf("This is a more efficient printf ... finished in");
    printf(" %lu ticks\n\n", (xTaskGetTickCount() - ticks));
  }
}
