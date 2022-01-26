#pragma once

#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/dac.h"

/* --- LEDs pinout --- */
#define LED1_GPIO           27
#define LED2_GPIO           26
#define LED3_GPIO           33
#define LED_BUILTIN_GPIO     2
#define TAG "LEDS"

/* --- Enums --- */
typedef enum led_state {
    LED_OFF = 0,
    LED_ON 
}           led_state_e;

typedef enum led_num {
    LED_1 = LED1_GPIO,
    LED_2 = LED2_GPIO,
    LED_3 = LED3_GPIO,
    LED_BUILTIN = LED_BUILTIN_GPIO
}            led_num_e;


/*--- Public functions declarations --- */
void leds_init(void);
void leds_turn_on(led_num_e led_pin);
void leds_turn_off(led_num_e led_pin);
void leds_toggle(led_num_e led_pin);

void leds_pwm_ch_init(led_num_e pwm_pin, ledc_channel_t channel);
void leds_pwm_ch_deinit(ledc_channel_t channel);
void leds_pwm_fade(ledc_channel_t channel, uint32_t duty, uint32_t set_time, bool wait);
void leds_set_duty(ledc_channel_t channel, uint32_t duty);