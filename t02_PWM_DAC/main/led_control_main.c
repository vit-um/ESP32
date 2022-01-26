#include <driver/dac.h>
#include "driver/ledc.h"

#include "leds.h"


/* LED1 controlled by PWM  LED2 controlled by DAC */
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

    leds_pwm_ch_init(LED_1, LEDC_CHANNEL_0);
    dac_output_enable(DAC_CHANNEL_2); // Select DAC output and enable it

    int i = LED1_FADE_TIME * 2 / 100;
    while (1) {
        i--;
        dac_cycle(); // DAC control LED2
        vTaskDelay(100 / portTICK_PERIOD_MS);
        if (i == LED1_FADE_TIME / 100) {
            printf("LED1: Began fade up to duty %d\n", LED1_DUTY);
            leds_pwm_fade(LEDC_CHANNEL_0, LED1_DUTY, LED1_FADE_TIME, LEDC_FADE_NO_WAIT);

            // ledc_set_fade_with_time(ledc_channel.speed_mode,
            //                     ledc_channel.channel, LED1_DUTY, LED1_FADE_TIME);
            // ledc_fade_start(ledc_channel.speed_mode,
            //                     ledc_channel.channel, LEDC_FADE_NO_WAIT);
        }
        if (i == 0) {
            printf("LED1: Began fade down to duty 8\n");
            leds_pwm_fade(LEDC_CHANNEL_0, 8, LED1_FADE_TIME, LEDC_FADE_NO_WAIT);

            // ledc_set_fade_with_time(ledc_channel.speed_mode,
            //                 ledc_channel.channel, 8, LED1_FADE_TIME);
            // ledc_fade_start(ledc_channel.speed_mode,
            //                 ledc_channel.channel, LEDC_FADE_NO_WAIT);
            i = LED1_FADE_TIME * 2 / 100;
        }
    }
}
