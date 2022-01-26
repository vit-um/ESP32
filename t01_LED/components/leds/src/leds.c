#include "leds.h"

/* --- Static variables --- */
static const char *TAG = "LEDS";


/* --- Public functions definitions --- */

void leds_init(void) {
    gpio_config_t leds_config;
    leds_config.intr_type = GPIO_INTR_DISABLE;
    leds_config.mode = GPIO_MODE_OUTPUT;
    leds_config.pin_bit_mask = (1ULL << LED_1_GPIO) |
                               (1ULL << LED_2_GPIO) |
                               (1ULL << LED_3_GPIO) |
                               (1ULL << LED_BUILTIN_GPIO);
    leds_config.pull_down_en = GPIO_PULLDOWN_ENABLE;
    leds_config.pull_up_en = GPIO_PULLUP_DISABLE;

    esp_err_t res = gpio_config(&leds_config);
    ESP_LOGV(TAG, "leds_init: %s", esp_err_to_name(res));
}

void leds_turn_on(led_num_e led_pin) {
    esp_err_t res = gpio_set_level(led_pin, LED_ON);
    ESP_LOGV(TAG, "leds_turn_on %d: %s", led_pin, esp_err_to_name(res));
}

void leds_turn_off(led_num_e led_pin) {
    esp_err_t res = gpio_set_level(led_pin, LED_OFF);
    ESP_LOGV(TAG, "leds_turn_off %d: %s", led_pin, esp_err_to_name(res));
}
