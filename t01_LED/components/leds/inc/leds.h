#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"

/* --- LEDs pinout --- */
#define LED_1_GPIO           27
#define LED_2_GPIO           26
#define LED_3_GPIO           33
#define LED_BUILTIN_GPIO     2


/* --- Enums --- */
typedef enum led_state {
    LED_OFF = 0,
    LED_ON 
}            led_state_e;

typedef enum led_num {
    LED_1 = LED_1_GPIO,
    LED_2 = LED_2_GPIO,
    LED_3 = LED_3_GPIO,
    LED_BUILTIN = LED_BUILTIN_GPIO
}            led_num_e;


/*--- Public functions declarations --- */
void leds_init(void);
void leds_turn_on(led_num_e led_pin);
void leds_turn_off(led_num_e led_pin);
