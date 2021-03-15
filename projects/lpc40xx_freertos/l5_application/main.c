#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"

#include "board_io.h"
#include "common_macros.h"
#include "periodic_scheduler.h"
#include "sj2_cli.h"

#include "uart_lab.h"

void uart_read_task(void *p) {
  while (1) {
    char *test;
    uart_lab__polled_get(UART_3, &test); // port 01
    printf("Test back: %c\n ", test);
    vTaskDelay(500);
  }
}

void uart_write_task(void *p) {
  while (1) {
    char test = "t";
    uart_lab__polled_put(UART_3, test); // port 00
    printf("Test sent: %c\n ", test);
    vTaskDelay(500);
  }
}

void main(void) {
  // TODO: Use uart_lab__init() function and initialize UART2 or UART3 (your choice)
  // TODO: Pin Configure IO pins to perform UART2/UART3 function
  uart_lab__init(UART_3, 96, 38400);
  xTaskCreate(uart_read_task, "test1", 2048 / sizeof(void *), NULL, PRIORITY_LOW, NULL);
  xTaskCreate(uart_write_task, "test2", 2048 / sizeof(void *), NULL, PRIORITY_LOW, NULL);

  vTaskStartScheduler();
}