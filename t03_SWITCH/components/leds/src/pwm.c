#include "leds.h"

void leds_pwm_ch_init(led_num_e pwm_pin, ledc_channel_t channel) {
    /*
     * Prepare and set configuration of timers
     * that will be used by LED Controller
     */
    ledc_timer_config_t ledc_timer = {
        .duty_resolution = LEDC_TIMER_13_BIT, // resolution of PWM duty
        .freq_hz = 5000,                      // frequency of PWM signal
        .speed_mode = LEDC_HIGH_SPEED_MODE,   // timer mode
        .timer_num = LEDC_TIMER_0,            // timer index
        .clk_cfg = LEDC_AUTO_CLK,             // Auto select the source clock
    };
    // Set configuration of timer0 for low speed channel
    ESP_ERROR_CHECK_WITHOUT_ABORT(ledc_timer_config(&ledc_timer));

    /*
     * Prepare individual configuration
     * for the channel of LED Controller
     */
    ledc_channel_config_t ledc_channel = {
            .channel    = channel,       //controller's channel number
            .duty       = 0,                    //output duty cycle, set initially to 0
            .gpio_num   = pwm_pin,            //GPIO number where LED is connected to
            .speed_mode = LEDC_HIGH_SPEED_MODE, //speed mode, either high or low
            .hpoint     = 0,                    
            .timer_sel  = LEDC_TIMER_0          //timer servicing selected channel
    };

    // Set LED Controller with previously prepared configuration
    ESP_ERROR_CHECK_WITHOUT_ABORT(ledc_channel_config(&ledc_channel));

    // Initialize fade service. You need first to install a default fade function, then you can use fade APIs.
    ESP_ERROR_CHECK_WITHOUT_ABORT(ledc_fade_func_install(0));
}

void leds_pwm_ch_deinit(ledc_channel_t channel) {
    ESP_ERROR_CHECK_WITHOUT_ABORT(ledc_stop(LEDC_HIGH_SPEED_MODE, channel, 0));
    ledc_fade_func_uninstall();
}

void leds_pwm_fade(ledc_channel_t channel, uint32_t duty, uint32_t set_time, bool wait) {
    ESP_ERROR_CHECK_WITHOUT_ABORT(ledc_set_fade_with_time(LEDC_HIGH_SPEED_MODE, channel, duty, set_time));
    ESP_ERROR_CHECK_WITHOUT_ABORT(ledc_fade_start(LEDC_HIGH_SPEED_MODE, channel, wait));
}

void leds_set_duty(ledc_channel_t channel, uint32_t duty) {
    ESP_ERROR_CHECK_WITHOUT_ABORT(ledc_set_duty(LEDC_HIGH_SPEED_MODE, channel, duty));
    ESP_ERROR_CHECK_WITHOUT_ABORT(ledc_update_duty(LEDC_HIGH_SPEED_MODE, channel));
}
