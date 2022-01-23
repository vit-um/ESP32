#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"        // for vTaskDelay
#include "driver/gpio.h"
#include <esp_err.h>
#include <string.h>
#include <esp_log.h>

#define DHT_DATA_GPIO 4
#define DHT_POW_GPIO 2

// DHT timer precision in microseconds
#define DHT_TIMER_INTERVAL 1
#define DHT_DATA_BITS 40
#define DHT_DATA_BYTES (DHT_DATA_BITS / 8)

/*
 *  Note:
 *  A suitable pull-up resistor should be connected to the selected GPIO line
 *
 *  __           ______          _______                              ___________________________
 *    \    A    /      \   C    /       \   DHT duration_data_low    /                           \
 *     \_______/   B    \______/    D    \__________________________/   DHT duration_data_high    \__
 *
 *
 *  Initializing communications with the DHT requires four 'phases' as follows:
 *
 *  Phase A - MCU pulls signal low for at least 18000 us
 *  Phase B - MCU allows signal to float back up and waits 20-40us for DHT to pull it low
 *  Phase C - DHT pulls signal low for ~80us
 *  Phase D - DHT lets signal float back up for ~80us
 *
 *  After this, the DHT transmits its first bit by holding the signal low for 50us
 *  and then letting it float back high for a period of time that depends on the data bit.
 *  duration_data_high is shorter than 50us for a logic '0' and longer than 50us for logic '1'.
 *
 *  There are a total of 40 data bits transmitted sequentially. These bits are read into a byte array
 *  of length 5.  The first and third bytes are humidity (%) and temperature (C), respectively.  Bytes 2 and 4
 *  are zero-filled and the fifth is a checksum such that:
 *
 *  byte_5 == (byte_1 + byte_2 + byte_3 + byte_4) & 0xFF
 *
 */

static const char *TAG = "dht11";

static portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

#define PORT_ENTER_CRITICAL() portENTER_CRITICAL(&mux)
#define PORT_EXIT_CRITICAL() portEXIT_CRITICAL(&mux)

#define CHECK_ARG(VAL) do { if (!(VAL)) return ESP_ERR_INVALID_ARG; } while (0)

#define CHECK_LOGE(x, msg, ...) do { \
        esp_err_t __; \
        if ((__ = x) != ESP_OK) { \
            PORT_EXIT_CRITICAL(); \
            ESP_LOGE(TAG, msg, ## __VA_ARGS__); \
            return __; \
        } \
    } while (0)

/**
 * Wait specified time for pin to go to a specified state.
 * If timeout is reached and pin doesn't go to a requested state
 * false is returned.
 * The elapsed time is returned in pointer 'duration' if it is not NULL.
 */
static esp_err_t dht_await_pin_state(uint32_t timeout, int expected_pin_state,
                                     uint32_t *duration) {
    /* XXX dht_await_pin_state() should save pin direction and restore
     * the direction before return. however, the SDK does not provide
     * gpio_get_direction().
     */
    gpio_set_direction(DHT_DATA_GPIO, GPIO_MODE_INPUT);
    for (uint32_t i = 0; i < timeout; i += DHT_TIMER_INTERVAL) {
        // need to wait at least a single interval to prevent reading a jitter
        ets_delay_us(DHT_TIMER_INTERVAL);
        if (gpio_get_level(DHT_DATA_GPIO) == expected_pin_state) {
            if (duration)
                *duration = i;
            return ESP_OK;
        }
    }

    return ESP_ERR_TIMEOUT;
}

/**
 * Request data from DHT and read raw bit stream.
 * The function call should be protected from task switching.
 * Return false if error occurred.
 */

static inline esp_err_t dht_fetch_data(uint8_t data[DHT_DATA_BYTES]) {
    uint32_t low_duration;
    uint32_t high_duration;

    // Phase 'A' pulling signal low to initiate read sequence
    gpio_set_direction(DHT_DATA_GPIO, GPIO_MODE_OUTPUT_OD);
    gpio_set_level(DHT_DATA_GPIO, 0);
    ets_delay_us(20000);
    gpio_set_level(DHT_DATA_GPIO, 1);

    // Step through Phase 'B', 40us
    CHECK_LOGE(dht_await_pin_state(40, 0, NULL),
            "Initialization error, problem in phase 'B'");
    // Step through Phase 'C', 82us
    CHECK_LOGE(dht_await_pin_state(82, 1, NULL),
            "Initialization error, problem in phase 'C'");
    // Step through Phase 'D', 82us
    CHECK_LOGE(dht_await_pin_state(82, 0, NULL),
            "Initialization error, problem in phase 'D'");

    // Read in each of the 40 bits of data...
    for (int i = 0; i < DHT_DATA_BITS; i++) {
        CHECK_LOGE(dht_await_pin_state(52, 1, &low_duration),
                "LOW bit timeout");
        CHECK_LOGE(dht_await_pin_state(72, 0, &high_duration),
                "HIGH bit timeout");

        uint8_t b = i / 8;
        uint8_t m = i % 8;
        if (!m)
            data[b] = 0;

        data[b] |= (high_duration > low_duration) << (7 - m);
    }

    return ESP_OK;
}


esp_err_t dht_read_data(int16_t *humidity, int16_t *temperature) {
    CHECK_ARG(humidity || temperature);

    uint8_t data[DHT_DATA_BYTES] = { 0 };

    gpio_set_direction(DHT_DATA_GPIO, GPIO_MODE_OUTPUT_OD);
    gpio_set_level(DHT_DATA_GPIO, 1);

    PORT_ENTER_CRITICAL();
    esp_err_t result = dht_fetch_data(data);
    if (result == ESP_OK)
        PORT_EXIT_CRITICAL();

    /* restore GPIO direction because, after calling dht_fetch_data(), the
     * GPIO direction mode changes */
    gpio_set_direction(DHT_DATA_GPIO, GPIO_MODE_OUTPUT_OD);
    gpio_set_level(DHT_DATA_GPIO, 1);

    if (result != ESP_OK)
        return result;

    if (data[4] != ((data[0] + data[1] + data[2] + data[3]) & 0xFF)) {
        ESP_LOGE(TAG, "Checksum failed, invalid data received from sensor");
        return ESP_ERR_INVALID_CRC;
    }

    if (humidity)
        *humidity = data[0];
    if (temperature)
        *temperature = data[2];

    ESP_LOGD(TAG, "Sensor data: humidity=%d, temp=%d", *humidity, *temperature);

    return ESP_OK;
}

void app_main(void) {
    int16_t hum;
    int16_t temp;

    // power on for DHT
    gpio_set_direction(DHT_POW_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(DHT_POW_GPIO, 1);
    vTaskDelay(2000 / portTICK_PERIOD_MS);
 
    while(1) {
        dht_read_data(&hum, &temp);
        printf("...\nTemperature: %d C\nHumidity: %d %%\n", temp, hum);
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}
