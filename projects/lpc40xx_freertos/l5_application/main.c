#include "FreeRTOS.h"
#include "task.h"

#include "gpio.h"
#include "semphr.h"
#include "ssp2_lab.h"

// python nxp-programmer/flash.py
const uint32_t spi_clock_mhz = 24;
SemaphoreHandle_t spitex;

void adesto_cs(void) { LPC_GPIO1->CLR = (1 << 10); }

void adesto_ds(void) { LPC_GPIO1->SET = (1 << 10); }

// TODO: Study the Adesto flash 'Manufacturer and Device ID' section
typedef struct {
  uint8_t manufacturer_id;
  uint8_t device_id_1;
  uint8_t device_id_2;
  uint8_t extended_device_id;
} adesto_flash_id_s;

// TODO: Implement the code to read Adesto flash memory signature
// TODO: Create struct of type 'adesto_flash_id_s' and return it
adesto_flash_id_s adesto_read_signature(void) {
  uint8_t random_data = 0xFF;
  adesto_flash_id_s data = {0};
  adesto_cs();
  (void)ssp2_lab__exchange_byte(0x9F); // switched this and line 27....this was problem
  {
    data.manufacturer_id = ssp2_lab__exchange_byte(random_data);
    data.device_id_1 = ssp2_lab__exchange_byte(random_data);
    data.device_id_2 = ssp2_lab__exchange_byte(random_data);
    data.extended_device_id = ssp2_lab__exchange_byte(random_data);
  }
  adesto_ds();
  return data;
}

void spi_task(void *p) {
  ssp2__init(spi_clock_mhz);

  while (1) {
    adesto_flash_id_s id = adesto_read_signature();
    // TODO: printf the members of the 'adesto_flash_id_s' struct
    printf("Manufacturer ID: 0x%02X Device ID1: 0x%02X Device ID2: 0x%02X Extended Device ID: 0x%02X \n",
           id.extended_device_id, id.device_id_1, id.device_id_2, id.extended_device_id);
    vTaskDelay(500);
  }
}

void spi_id_verification_task(void *p) {
  ssp2__init(spi_clock_mhz);
  while (1) {
    if (xSemaphoreTake(spitex, portMAX_DELAY)) {
      adesto_flash_id_s id = adesto_read_signature();

      // When we read a manufacturer ID we do not expect, we will kill this task
      if (0x1F != id.manufacturer_id) {
        fprintf(stderr, "Manufacturer ID read failure\n");
        vTaskSuspend(NULL); // Kill this task
      } else {
        printf("Manufacturer ID: 0x%02X Device ID1: 0x%02X Device ID2: 0x%02X Extended Device ID: 0x%02X \n",
               id.extended_device_id, id.device_id_1, id.device_id_2, id.extended_device_id);
      }
      xSemaphoreGive(spitex);
    }
    vTaskDelay(500);
  }
}

void main(void) {
  spitex = xSemaphoreCreateMutex();
  xTaskCreate(spi_id_verification_task, "SSP1", 2048 / sizeof(void *), NULL, PRIORITY_LOW, NULL);
  xTaskCreate(spi_id_verification_task, "SSP2", 2048 / sizeof(void *), NULL, PRIORITY_LOW, NULL);
  vTaskStartScheduler();
}
