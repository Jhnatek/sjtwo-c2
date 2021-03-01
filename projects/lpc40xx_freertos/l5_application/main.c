#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"

#include "board_io.h"
#include "common_macros.h"
#include "periodic_scheduler.h"
#include "sj2_cli.h"

// 'static' to make these functions 'private' to this file
static void create_blinky_tasks(void);
static void create_uart_task(void);
static void blink_task(void *params);
static void uart_task(void *params);

// lab 2
static void task_one(void *task_parameter) {
  while (true) {
    // Read existing main.c regarding when we should use fprintf(stderr...) in place of printf()
    // For this lab, we will use fprintf(stderr, ...)
    fprintf(stderr, "AAAAAAAAAAAA");

    // Sleep for 100ms
    vTaskDelay(100);
  }
}

static void task_two(void *task_parameter) {
  while (true) {
    fprintf(stderr, "bbbbbbbbbbbb");
    vTaskDelay(100);
  }
}

#include "gpio_isr.h"
#include "lpc40xx.h"
#include "lpc_peripherals.h"
#include "semphr.h"

static SemaphoreHandle_t switch_pressed_signal;

// lab 4 (lab 3 in different branch) make sure to use semaphore as flag/signal in lab
// for lab he said use 54 for gpio interrupt (interrupt_vector_table.c)
// use data sheet 8.5 to choose interrupt rising edge for ports
typedef void (*void_pointer_t)(void);
// void_pointer_t function_callbacks[3];

// void register_callback(uint8_t index, void_pointer_t function) { function_callbacks[index] = function; }

// void make_callback(uint8_t index) { function_callbacks[index](); }

// lab 4 part 1
/*void gpio_interrupt(void) {
  fprintf(stderr, "ISR Entry");
  xSemaphoreGiveFromISR(switch_pressed_signal, NULL);
  clear_gpio_interrupt();
} */
void clear_gpio_interrupt(void) {
  // LPC_GPIOINT->IO0IntClr |= (1 << 29);
  gpiO__interrupt_dispatcher();
  xSemaphoreGiveFromISR(switch_pressed_signal, NULL);
  fprintf(stderr, "----clear interrupt---- \n");
}

void configure_your_gpio_interrupt(void) {
  LPC_GPIO0->DIR &= ~(1 << 29);
  LPC_GPIOINT->IO0IntEnR = (1 << 29);
}

void sleep_on_sem_task(void *pvParameters) {
  LPC_IOCON->P1_26 = ~(0b111);
  LPC_GPIO1->DIR |= (1 << 26);
  while (1) {
    // Wait forever until a the semaphore is sent/given
    if (xSemaphoreTake(switch_pressed_signal, portMAX_DELAY)) {
      printf("     Semaphore taken\n-----------------------\n");
      // blink led
      LPC_GPIO1->PIN |= (1 << 26);
      vTaskDelay(500);
      LPC_GPIO1->PIN &= ~(1 << 26);
      vTaskDelay(500);
      LPC_GPIO1->PIN |= (1 << 26);
      vTaskDelay(500);
      LPC_GPIO1->PIN &= ~(1 << 26);
      vTaskDelay(500);
    }
  }
}
void switch2(void) { fprintf(stderr, "-----------------------\n  switch 2 was pressed\n"); }
void switch3(void) { fprintf(stderr, "-----------------------\n  switch 3 was pressed\n"); }

// flash: python nxp-programmer/flash.py

int main(void) {
  // create_blinky_tasks();
  // create_uart_task();

  // If you have the ESP32 wifi module soldered on the board, you can try uncommenting this code
  // See esp32/README.md for more details
  // uart3_init();                                                                     // Also include:  uart3_init.h
  // xTaskCreate(esp32_tcp_hello_world_task, "uart3", 1000, NULL, PRIORITY_LOW, NULL); // Include esp32_task.h
  puts("Starting RTOS,  why hello there world\n");

  switch_pressed_signal = xSemaphoreCreateBinary();
  void_pointer_t switch_pins[] = {switch2, switch3};
  gpiO__attach_interrupt(0, 30, GPIO_INTR__RISING_EDGE, switch_pins[0]);
  gpiO__attach_interrupt(0, 29, GPIO_INTR__FALLING_EDGE, switch_pins[1]);
  NVIC_EnableIRQ(GPIO_IRQn);
  lpc_peripheral__enable_interrupt(LPC_PERIPHERAL__GPIO, clear_gpio_interrupt, NULL);
  xTaskCreate(sleep_on_sem_task, "sem", (2048) / sizeof(void *), NULL, PRIORITY_LOW, NULL);
  vTaskStartScheduler(); // This function never returns unless RTOS scheduler runs out of memory and fails
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

static void blink_task(void *params) {
  const gpio_s led = *((gpio_s *)params); // Parameter was input while calling xTaskCreate()

  // Warning: This task starts with very minimal stack, so do not use printf() API here to avoid stack overflow
  while (true) {
    gpio__toggle(led);
    vTaskDelay(500);
  }
}

// This sends periodic messages over printf() which uses system_calls.c to send them to UART0
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
