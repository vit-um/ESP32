#include <stdio.h>
#include <math.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_rom_sys.h"

#include "speaker.h"
#include "accelerometer.h"
#include "buttons.h"
#include "dht11.h"

#include "screen.h"


/* --- Static functions declarations --- */
static void board_init(void);
static void sensor_handler(void);
static void orient_handler(void);
static void buttons_handler(void);


void app_main(void) {

    printf("Creative task\n");

    board_init();

    uint8_t ctr = 0;
    while(1) {
        //get data not faster than every 2 seconds
        if (ctr >= 20) {
            ctr = 0;
            sensor_handler();
        }

        orient_handler();
        buttons_handler();

        screen_update();
        vTaskDelay(100 / portTICK_PERIOD_MS);
        ctr++;
    }
}

/* --- Static functions definitions --- */
static void board_init(void) {
    speaker_init();
    speaker_turn_on();

    buttons_init();

    dht11_init();
    dht11_turn_on();

    acc_init();
    acc_turn_on();
    acc_set_bandwidth(ADXL345_BW_12_5_HZ, 0);
    acc_set_range(ADXL345_RANGE_4G);
    acc_start_measure();

    screen_init();
    screen_update();
}

static void sensor_handler(void) {
    dht11_data_t sensor_data;
    dht11_get_data(&sensor_data);
    screen_set_data(sensor_data.temp_int, sensor_data.rh_int);
}

static void orient_handler(void) {
    acc_data_t orient_data;
    acc_get_values(&orient_data);

    //if y < - 0,7 - board is not flipped
    // if y > 0.7 - board is flipped
    float vector = sqrt(powf(orient_data.x, 2) + powf(orient_data.y, 2) + powf(orient_data.z, 2));
    bool stable = (vector > 0.9 && vector < 1.1) ? true : false;

    if (stable) {
        if (orient_data.y > 0.7) {
            screen_set_orientation(SCREEN_DOWN);
        } else if (orient_data.y < -0.7) {
            screen_set_orientation(SCREEN_UP);
        }
    }
}

static void buttons_handler(void) {
    if (buttons_get_event(BUTTON_1)) {
        screen_set_page(SCREEN_TEMPERATURE);
        speaker_play_tone(440, DAC_CW_SCALE_1, 200);
        buttons_set_event(BUTTON_1, 0);
    }
    if (buttons_get_event(BUTTON_2)) {
        screen_set_page(SCREEN_HUMIDITY);
        speaker_play_tone(440, DAC_CW_SCALE_1, 200);
        buttons_set_event(BUTTON_2, 0);
    }
}
