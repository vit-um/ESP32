#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "sdkconfig.h"


#define LED1_GPIO 27
#define LED2_GPIO 26

#define SW1_GPIO 39
#define SW2_GPIO 18

static void turn_off_all(void) {
    printf("Turning off all LEDs\n");
    gpio_set_level(LED1_GPIO, 0);
    gpio_set_level(LED2_GPIO, 0);
    vTaskDelay(100 / portTICK_PERIOD_MS);
} 

static void toggle_led(uint32_t led) {
    if (gpio_get_level(led) == 1)
        gpio_set_level(led, 0);
    else 
        gpio_set_level(led, 1);
    printf("LED%d has been switched: level - %d\n", 28 - led, gpio_get_level(led));
    /*loop for switch boun off */
    while ((gpio_get_level(SW1_GPIO) == 0) || (gpio_get_level(SW2_GPIO) == 0)) 
        vTaskDelay(1 / portTICK_PERIOD_MS);
}


void app_main(void) {
    gpio_reset_pin(LED1_GPIO);
    gpio_reset_pin(LED2_GPIO);
    gpio_reset_pin(SW1_GPIO);
    gpio_reset_pin(SW2_GPIO);  
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(LED1_GPIO, GPIO_MODE_INPUT_OUTPUT);
    gpio_set_direction(LED2_GPIO, GPIO_MODE_INPUT_OUTPUT);
    gpio_set_direction(SW1_GPIO, GPIO_MODE_INPUT);
    gpio_set_direction(SW2_GPIO, GPIO_MODE_INPUT);

    turn_off_all();
    printf("Press SW1: Toggle LED1\nPress SW2: Toggle LED2\n");
    while(1) {
        if (gpio_get_level(SW1_GPIO) == 0)
                toggle_led(LED1_GPIO);
        if (gpio_get_level(SW2_GPIO) == 0) 
                toggle_led(LED2_GPIO);
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}
