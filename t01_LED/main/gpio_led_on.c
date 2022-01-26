#include "leds.h"

#define SW1_GPIO 39
#define SW2_GPIO 18
#define DELAY_TIME 100

static void turn_off_all(void) {
    printf("Turning off all LEDs\n");
    gpio_set_level(LED_1_GPIO, 0);
    gpio_set_level(LED_2_GPIO, 0);
    gpio_set_level(LED_3_GPIO, 0);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
} 

static void sw_init(void) {
   /* Configure the IOMUX register for pad GPIO (some pads are
       muxed to GPIO on reset already, but some default to other
       functions and need to be switched to GPIO.
    */
    gpio_reset_pin(SW1_GPIO);
    gpio_reset_pin(SW2_GPIO);  
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(SW1_GPIO, GPIO_MODE_INPUT);
    gpio_set_direction(SW2_GPIO, GPIO_MODE_INPUT);
}

static void turn_on_all(void) { 
        /* Light on (output high) */
        printf("Turning on LED1\n");
        gpio_set_level(LED_1_GPIO, 1);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        /* Turn on (output high) */
        printf("Turning on LED2\n");
        gpio_set_level(LED_2_GPIO, 1);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        printf("Turning on LED3\n");
        gpio_set_level(LED_3_GPIO, 1);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
}

static void wave(void) { 
    printf("Works in loop mode, restart plz\n");
    while(1) {
        leds_turn_on(LED_1);
        vTaskDelay(DELAY_TIME / portTICK_PERIOD_MS);
        leds_turn_on(LED_2);
        vTaskDelay(DELAY_TIME / portTICK_PERIOD_MS);
        leds_turn_on(LED_3);
        vTaskDelay(DELAY_TIME / portTICK_PERIOD_MS);
        leds_turn_on(LED_BUILTIN);
        vTaskDelay(DELAY_TIME / portTICK_PERIOD_MS);

        leds_turn_off(LED_1);
        vTaskDelay(DELAY_TIME / portTICK_PERIOD_MS);
        leds_turn_off(LED_2);
        vTaskDelay(DELAY_TIME / portTICK_PERIOD_MS);
        leds_turn_off(LED_3);
        vTaskDelay(DELAY_TIME / portTICK_PERIOD_MS);
        leds_turn_off(LED_BUILTIN);
        vTaskDelay(DELAY_TIME / portTICK_PERIOD_MS);
    }      
}


void app_main(void) {
    int mode = 2;

    leds_init();
    sw_init();

    printf("Press SW1: Turn on all LEDs\nDon`t press SW2!!!\n");

    while(1) {
        if (mode == 1) 
            wave();
        if (mode == 0)
            turn_on_all();
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


