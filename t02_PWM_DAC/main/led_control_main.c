#include <stdio.h>
#include <driver/dac.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "esp_err.h"


#define LED1_GPIO         (27)   // LED1 controlled by PWM
#define LED2_GPIO         (26)   // LED2 controlled by DAC
#define LED1_DUTY         (8191) // set max = (2 ** LEDC_TIMER_13_BIT - 1)
#define LED1_FADE_TIME    (5000) // time of fade cycle, mc

static uint8_t count = 135;
static uint8_t next = 136;

static void dac_cycle(void) {
    if (count < next) {
        count++;
        next++;
    }
    else if (count > next) {
        count--;
        next--;
    }
    if (count == 255) { 
        next = 253;
        count = 254;
        printf("LED2: Achieved maximum voltage on the LED = %-3.2fV.\n", 3.3 * count / 255);
    }
    if (count == 135) {
        next = 137;
        count = 136;
        printf("LED2: Achieved minimum voltage on the LED = %-3.2fV.\n", 3.3 * count / 255);
    }
    dac_output_voltage(DAC_CHANNEL_2, count);
}


void app_main(void) {
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
    ledc_timer_config(&ledc_timer);

    /*
     * Prepare individual configuration
     * for the channel of LED Controller
     */
    ledc_channel_config_t ledc_channel = {
            .channel    = LEDC_CHANNEL_0,       //controller's channel number
            .duty       = 0,                    //output duty cycle, set initially to 0
            .gpio_num   = LED1_GPIO,            //GPIO number where LED is connected to
            .speed_mode = LEDC_HIGH_SPEED_MODE, //speed mode, either high or low
            .hpoint     = 0,                    
            .timer_sel  = LEDC_TIMER_0          //timer servicing selected channel
    };

    // Set LED Controller with previously prepared configuration
    ledc_channel_config(&ledc_channel);

    // Initialize fade service. You need first to install a default fade function, then you can use fade APIs.
    ledc_fade_func_install(0);
    
    // Select DAC output and enable it
    dac_output_enable(DAC_CHANNEL_2);

    int i = LED1_FADE_TIME * 2 / 100;
    while (1) {
        i--;
        dac_cycle(); // DAC control LED2
        vTaskDelay(100 / portTICK_PERIOD_MS);
        if (i == LED1_FADE_TIME / 100) {
            printf("LED1: Began fade up to duty %d\n", LED1_DUTY);
            ledc_set_fade_with_time(ledc_channel.speed_mode,
                                ledc_channel.channel, LED1_DUTY, LED1_FADE_TIME);
            ledc_fade_start(ledc_channel.speed_mode,
                                ledc_channel.channel, LEDC_FADE_NO_WAIT);
        }
        if (i == 0) {
            printf("LED1: Began fade down to duty 8\n");
            ledc_set_fade_with_time(ledc_channel.speed_mode,
                            ledc_channel.channel, 8, LED1_FADE_TIME);
            ledc_fade_start(ledc_channel.speed_mode,
                            ledc_channel.channel, LEDC_FADE_NO_WAIT);
            i = LED1_FADE_TIME * 2 / 100;
        }
    }
}
