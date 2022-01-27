#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "driver/dac.h"
#include "driver/timer.h"
#include "esp_log.h"


/*  The timer ISR has an execution time of 5.5 micro-seconds(us).
    Therefore, a timer period less than 5.5 us will cause trigger the interrupt watchdog.
    7 us is a safe interval that will not trigger the watchdog. No need to customize it.
*/

#define TIMER_INTR_US          7     // Execution time of each ISR interval in micro-seconds
#define TIMER_DIVIDER          16
#define POINT_ARR_LEN          200   // Length of points array
#define AMP_DAC                255   // Amplitude of DAC voltage. If it's more than 256 will causes dac_output_voltage() output 0.
#define VDD                    3300  // VDD is 3.3V, 3300mV
#define CONST_PERIOD_2_PI      6.2832
#define SEC_TO_MICRO_SEC(x)    ((x) / 1000 / 1000)    // Convert second to micro-second
#define UNUSED_PARAM           __attribute__((unused)) // A const period parameter which equals 2 * pai, used to calculate raw dac output value.
#define TIMER_TICKS            (TIMER_BASE_CLK / TIMER_DIVIDER)     // TIMER_BASE_CLK = APB_CLK = 80MHz
#define ALARM_VAL_US           SEC_TO_MICRO_SEC(TIMER_INTR_US * TIMER_TICKS)     // Alarm value in micro-seconds
#define OUTPUT_POINT_NUM       (int)(1000000 / (TIMER_INTR_US * FREQ) + 0.5)     // The number of output wave points.

#define DAC_CHAN               0  // DAC_CHANNEL_1 (GPIO25) by default
#define FREQ                   1000        // 3000 Hz by default
#define EN_AMP                 5   //GPIO5

#define SW1_GPIO 39
#define SW2_GPIO 18

_Static_assert(OUTPUT_POINT_NUM <= POINT_ARR_LEN, "The CONFIG_EXAMPLE_WAVE_FREQUENCY is too low and using too long buffer.");

enum form {
    SINE,
    TRIANGLE,
    SAWTOOTH,
    SQUARE,
} waveform = 0;

static int raw_val[POINT_ARR_LEN];                     // Used to store raw values
static int volt_val[POINT_ARR_LEN];                    // Used to store voltage values(in mV)
static const char *TAG = "wave_gen";

static int g_index = 0;

/* Timer interrupt service routine */
static void IRAM_ATTR timer0_ISR(void *ptr) {
    timer_group_clr_intr_status_in_isr(TIMER_GROUP_0, TIMER_0);
    timer_group_enable_alarm_in_isr(TIMER_GROUP_0, TIMER_0);

    int *head = (int*)ptr;

    /* DAC output ISR has an execution time of 4.4 us*/
    if (g_index >= OUTPUT_POINT_NUM) g_index = 0;
    dac_output_voltage(DAC_CHAN, *(head + g_index));
    g_index++;
}

/* Timer group0 TIMER_0 initialization */
static void dac_timer_init(int timer_idx, bool auto_reload) {
    timer_config_t config = {
        .divider = TIMER_DIVIDER,
        .counter_dir = TIMER_COUNT_UP,
        .counter_en = TIMER_PAUSE,
        .alarm_en = TIMER_ALARM_EN,
        .intr_type = TIMER_INTR_LEVEL,
        .auto_reload = auto_reload,
    };

    ESP_ERROR_CHECK(timer_init(TIMER_GROUP_0, timer_idx, &config));
    ESP_ERROR_CHECK(timer_set_counter_value(TIMER_GROUP_0, timer_idx, 0x00000000ULL));
    ESP_ERROR_CHECK(timer_set_alarm_value(TIMER_GROUP_0, timer_idx, ALARM_VAL_US));
    ESP_ERROR_CHECK(timer_enable_intr(TIMER_GROUP_0, TIMER_0));
    /* Register an ISR handler */
    timer_isr_register(TIMER_GROUP_0, timer_idx, timer0_ISR, (void *)raw_val, ESP_INTR_FLAG_IRAM, NULL);
}

static void prepare_data(int pnt_num, enum form waveform) {
    timer_pause(TIMER_GROUP_0, TIMER_0);
    for (int i = 0; i < pnt_num; i ++) {
        switch (waveform) {
        case 0:
            raw_val[i] = (int)((sin( i * CONST_PERIOD_2_PI / pnt_num) + 1) * (double)(AMP_DAC)/2 + 0.5);
            break;
        case 1:
            raw_val[i] = (i > (pnt_num/2)) ? (2 * AMP_DAC * (pnt_num - i) / pnt_num) : (2 * AMP_DAC * i / pnt_num);
            break;
        case 2:
            raw_val[i] = (i == pnt_num) ? 0 : (i * AMP_DAC / pnt_num);
            break;
        case 3:
            break;
            raw_val[i] = (i < (pnt_num/2)) ? AMP_DAC : 0;
        default:
            break;
        }
        volt_val[i] = (int)(VDD * raw_val[i] / (float)AMP_DAC);
    }
    timer_start(TIMER_GROUP_0, TIMER_0);
}

static void log_info(enum form waveform) {
    switch (waveform) {
    case 0:
        ESP_LOGI(TAG, "Waveform: SINE");
        break;
    case 1:
        ESP_LOGI(TAG, "Waveform: TRIANGLE");
        break;
    case 2:
        ESP_LOGI(TAG, "Waveform: SAWTOOTH");
        break;
    case 3:
        ESP_LOGI(TAG, "Waveform: SQUARE");
        break;    
    default:
        break;
    };
    ESP_LOGI(TAG, "Frequency(Hz): %d", FREQ);
    ESP_LOGI(TAG, "Output points num: %d\n", OUTPUT_POINT_NUM);
}

static void power_on_amp(void){
    gpio_reset_pin(EN_AMP);  
    gpio_set_direction(EN_AMP, GPIO_MODE_OUTPUT);
    printf("Power on the sound amplifier\n");
    gpio_set_level(EN_AMP, 1);
}

void app_main(void) {
    power_on_amp();
    dac_timer_init(TIMER_0, 1);
    ESP_ERROR_CHECK(dac_output_enable(DAC_CHAN));
    log_info(waveform);
    prepare_data(OUTPUT_POINT_NUM, waveform);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(SW1_GPIO, GPIO_MODE_INPUT);
    gpio_set_direction(SW2_GPIO, GPIO_MODE_INPUT);
    printf("Press SW1: Mute\nPress SW2: Change Waveform\n");
    bool mute = true;
    while(1) {
        if (gpio_get_level(SW1_GPIO) == 0) {
            if (mute) {
                mute = false;
                printf("\n\033[31mMute\033[0\n");
            }
            else {
                mute = true;
                printf("\n\033[31mUnmute\033[0\n");
            }
            gpio_set_level(EN_AMP, mute);
        }
        if (gpio_get_level(SW2_GPIO) == 0) {
            if (waveform == SQUARE)
                waveform = SINE;
            else
                waveform++;
            log_info(waveform);
            prepare_data(OUTPUT_POINT_NUM, waveform);        
        }
        while ((gpio_get_level(SW1_GPIO) == 0) || (gpio_get_level(SW2_GPIO) == 0)) 
            vTaskDelay(1 / portTICK_PERIOD_MS);
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}


/* version by pchernushe with speaker module 
#include "speaker.h"


void app_main(void) {

    printf("Task 06. Speaker\n");

    speaker_init();
    speaker_turn_on();

    while(1) {

        speaker_play_tone(300, DAC_CW_SCALE_1, 400);
        speaker_play_tone(250, DAC_CW_SCALE_1, 300);

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
} */
