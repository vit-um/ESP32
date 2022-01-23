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
#define LED3_GPIO 33
#define SW1_GPIO 39
#define SW2_GPIO 18

static void turn_off_all(void) {
    printf("Turning off all LEDs\n");
    gpio_set_level(LED1_GPIO, 0);
    gpio_set_level(LED2_GPIO, 0);
    gpio_set_level(LED3_GPIO, 0);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
} 

static void turn_on_all(void) { 
        /* Light on (output high) */
        printf("Turning on LED1\n");
        gpio_set_level(LED1_GPIO, 1);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        /* Turn on (output high) */
        printf("Turning on LED2\n");
        gpio_set_level(LED2_GPIO, 1);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        printf("Turning on LED3\n");
        gpio_set_level(LED3_GPIO, 1);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
}

static void wave(void) { 
    while (1) {
        printf("Working wave mode, reboot plz\n");
        gpio_set_level(LED1_GPIO, 1);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        gpio_set_level(LED1_GPIO, 0);
        gpio_set_level(LED2_GPIO, 1);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        gpio_set_level(LED2_GPIO, 0);
        gpio_set_level(LED3_GPIO, 1);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        gpio_set_level(LED3_GPIO, 0);
    }        
}


void app_main(void)
{
    /* Configure the IOMUX register for pad GPIO (some pads are
       muxed to GPIO on reset already, but some default to other
       functions and need to be switched to GPIO.
    */
    gpio_reset_pin(LED1_GPIO);
    gpio_reset_pin(LED2_GPIO);
    gpio_reset_pin(LED3_GPIO);
    gpio_reset_pin(SW1_GPIO);
    gpio_reset_pin(SW2_GPIO);  
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(LED1_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED2_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED3_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_direction(SW1_GPIO, GPIO_MODE_INPUT);
    gpio_set_direction(SW2_GPIO, GPIO_MODE_INPUT);

    int mode = 0;
    while(1) {
        if (mode) 
            wave();
        else
            turn_on_all();
        printf("Press SW1: Turn on all LEDs\nDon`t press SW2!!!\n");
        while (1) {
            if (gpio_get_level(SW1_GPIO) == 0) {
                mode = 0;
                break;
            }
            if (gpio_get_level(SW2_GPIO) == 0) {
                mode = 1;
                break;
            }
            vTaskDelay(1000 / portTICK_PERIOD_MS / 5);
        }
        turn_off_all();    
    }
}


