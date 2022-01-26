#include "leds.h"
#include "buttons.h"

static void turn_off_all(void) {
    printf("Turning off all LEDs\n");
    gpio_set_level(LED1_GPIO, 0);
    gpio_set_level(LED2_GPIO, 0);
    vTaskDelay(100 / portTICK_PERIOD_MS);
} 

void app_main(void) {
    leds_init();
    buttons_init();
 
    turn_off_all();
    printf("Press SW1: Toggle LED1\nPress SW2: Toggle LED2\n");

    while(1) {
        if (buttons_get_event(BUTTON_1)) {
            leds_toggle(LED_1);  
            buttons_set_event(BUTTON_1, 0);
        }
        if (buttons_get_event(BUTTON_2)) {
            leds_toggle(LED_2); 
            buttons_set_event(BUTTON_2, 0);
        }

        vTaskDelay(200 / portTICK_PERIOD_MS);
    }

}
