#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <stdio.h>
#include "pico/stdlib.h"

const int BTN_PIN_R = 28;
const int BTN_PIN_G = 26;

const int LED_PIN_R = 4;
const int LED_PIN_G = 6;

QueueHandle_t xQueueLedR;
QueueHandle_t xQueueLedG;

volatile int delay_R = 100;
volatile int delay_G = 100;

void btn_callback(uint gpio, uint32_t events) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    int newDelay;
    
    if ((gpio == BTN_PIN_R) && (events & GPIO_IRQ_EDGE_FALL)) {
        if (delay_R < 1000) {
            delay_R += 100;
        } else {
            delay_R = 100;
        }
        newDelay = delay_R;
        xQueueSendFromISR(xQueueLedR, &newDelay, &xHigherPriorityTaskWoken);
    }
    else if ((gpio == BTN_PIN_G) && (events & GPIO_IRQ_EDGE_FALL)) {
        if (delay_G < 1000) {
            delay_G += 100;
        } else {
            delay_G = 100;
        }
        newDelay = delay_G;
        xQueueSendFromISR(xQueueLedG, &newDelay, &xHigherPriorityTaskWoken);
    }
    
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void led_r_task(void *pvParameters) {
    gpio_init(LED_PIN_R);
    gpio_set_dir(LED_PIN_R, GPIO_OUT);
    
    int delay;
    while (true) {
        if (xQueueReceive(xQueueLedR, &delay, portMAX_DELAY) == pdPASS) {
            printf("LED R delay: %d ms\n", delay);
            gpio_put(LED_PIN_R, 1);
            vTaskDelay(pdMS_TO_TICKS(delay));
            gpio_put(LED_PIN_R, 0);
            vTaskDelay(pdMS_TO_TICKS(delay));
        }
    }
}

void led_g_task(void *pvParameters) {
    gpio_init(LED_PIN_G);
    gpio_set_dir(LED_PIN_G, GPIO_OUT);
    
    int delay;
    while (true) {
        if (xQueueReceive(xQueueLedG, &delay, portMAX_DELAY) == pdPASS) {
            printf("LED G delay: %d ms\n", delay);
            gpio_put(LED_PIN_G, 1);
            vTaskDelay(pdMS_TO_TICKS(delay));
            gpio_put(LED_PIN_G, 0);
            vTaskDelay(pdMS_TO_TICKS(delay));
        }
    }
}

int main() {
    stdio_init_all();
    printf("Iniciando FreeRTOS com controle dos LEDs...\n");

    xQueueLedR = xQueueCreate(32, sizeof(int));
    xQueueLedG = xQueueCreate(32, sizeof(int));

    gpio_init(BTN_PIN_R);
    gpio_set_dir(BTN_PIN_R, GPIO_IN);
    gpio_pull_up(BTN_PIN_R);
    gpio_set_irq_enabled_with_callback(BTN_PIN_R, GPIO_IRQ_EDGE_FALL, true, btn_callback);

    gpio_init(BTN_PIN_G);
    gpio_set_dir(BTN_PIN_G, GPIO_IN);
    gpio_pull_up(BTN_PIN_G);
    gpio_set_irq_enabled(BTN_PIN_G, GPIO_IRQ_EDGE_FALL, true);

    xTaskCreate(led_r_task, "LED_R_Task", 256, NULL, 1, NULL);
    xTaskCreate(led_g_task, "LED_G_Task", 256, NULL, 1, NULL);

    vTaskStartScheduler();

    while (true) {
    }
    
    return 0;
}
