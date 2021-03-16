#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "board_io.h"
#include "common_macros.h"
#include "periodic_scheduler.h"
#include "queue.h"
#include "sj2_cli.h"

#include "uart_lab.h"
// python nxp-programmer/flash.py

void uart_read_task(void *p) {
  char *test;
  while (1) {
    if (uart_lab__get_char_from_queue(&test, portMAX_DELAY)) { // took out & from test
      fprintf(stderr, "Test back: %c\n ", test);               // port 28
      vTaskDelay(500);
    } else {
      fprintf(stderr, "error in read task\n");
    }
  }
}

void uart_write_task(void *p) {
  // char test[5] = "test-"; // dont use double quotes!!!
  char test = 't';
  while (1) {
    for (int i = 0; i < 5; i++) {
      uart_lab__polled_put(UART_2, test); // port 29
      printf("Test sent: %c\n ", test);
    }
    vTaskDelay(500);
  }
}

void main(void) {
  uart_lab__init(UART_2, 96, 38400);
  uart__enable_receive_interrupt(UART_2);
  xTaskCreate(uart_read_task, "test1", 2048 / sizeof(void *), NULL, PRIORITY_LOW, NULL);
  xTaskCreate(uart_write_task, "test2", 2048 / sizeof(void *), NULL, PRIORITY_LOW, NULL);

  vTaskStartScheduler();
}

#if (0)
part 1 void uart_read_task(void *p) {
  while (1) {
    char *test;
    uart_lab__polled_get(UART_2, &test); // port 28
    printf("Test back: %c\n ", test);
    vTaskDelay(500);
  }
}

void uart_write_task(void *p) {
  while (1) {
    char test = 't';                    // dont use double quotes!!!
    uart_lab__polled_put(UART_2, test); // port 29
    printf("Test sent: %c\n ", test);
    vTaskDelay(500);
  }
}

void main(void) {
  // TODO: Use uart_lab__init() function and initialize UART2 or UART3 (your choice)
  // TODO: Pin Configure IO pins to perform UART2/UART3 function
  uart_lab__init(UART_2, 96, 38400);
  xTaskCreate(uart_read_task, "test1", 2048 / sizeof(void *), NULL, PRIORITY_LOW, NULL);
  xTaskCreate(uart_write_task, "test2", 2048 / sizeof(void *), NULL, PRIORITY_LOW, NULL);

  vTaskStartScheduler();
}
#endif