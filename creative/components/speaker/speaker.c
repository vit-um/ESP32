#include "speaker.h"


/* --- Defines --- */


/* --- Static variables --- */


/* --- Static functions declarations --- */
static void speaker_gpio_init(void);

/* --- Public functions definitions --- */
void speaker_init(void) {
    speaker_gpio_init();

    ESP_ERROR_CHECK_WITHOUT_ABORT(dac_output_enable(DAC_CHANNEL_1));

}

void speaker_turn_on(void) {
    gpio_set_level(SPEAKER_EN_PIN, 1);
}

void speaker_turn_off(void) {
    gpio_set_level(SPEAKER_EN_PIN, 0);
}

void speaker_play_tone(uint32_t freq, dac_cw_scale_t ampl, uint32_t time) {
    dac_cw_config_t tone_config = {
        .en_ch = DAC_CHANNEL_1,
        .freq = freq,
        .offset = 0,
        .phase = 0,
        .scale = ampl
    };
    ESP_ERROR_CHECK_WITHOUT_ABORT(dac_cw_generator_config(&tone_config));

    dac_cw_generator_enable();
    vTaskDelay(time / portTICK_PERIOD_MS);
    dac_cw_generator_disable();
}


/* --- Static functions definitions --- */
static void speaker_gpio_init(void) {
    gpio_config_t gpio_en_config;

    gpio_en_config.intr_type = GPIO_INTR_DISABLE;
    gpio_en_config.mode = GPIO_MODE_OUTPUT;
    gpio_en_config.pin_bit_mask = (1ULL << SPEAKER_EN_PIN);
    gpio_en_config.pull_down_en = GPIO_PULLDOWN_DISABLE;
    gpio_en_config.pull_up_en = GPIO_PULLUP_DISABLE;

    ESP_ERROR_CHECK_WITHOUT_ABORT(gpio_config(&gpio_en_config));
}
