#include "dht11.h"


/* --- Defines --- */
#define DHT11_DURATION_ONE          70
#define DHT11_DURATION_ZERO         28
#define DHT11_DURATION_START        85
#define DHT11_DURATION_NULL         50
#define DHT11_DURATON_THRESHOLD     10


/* --- Static variables --- */
static const char *TAG = "DHT11";
static const rmt_item32_t start_seq[] = {
    {{{20000, 0, 1, 1}}}
};


/* --- Static functions declarations --- */
static void dht11_gpio_config(void);
static int dht11_decode(rmt_item32_t *items, dht11_data_t *data, uint32_t length);
static bool dht11_is_start(uint32_t level, uint32_t duration);
static bool dht11_is_start_bit(uint32_t level, uint32_t duration);
static bool dht11_is_one(uint32_t level, uint32_t duration);
static bool dht11_is_zero(uint32_t level, uint32_t duration);
static int dht11_checksum_validation(dht11_data_t *data);


/* --- Public functions definitions --- */
void dht11_init(void) {
    dht11_gpio_config();

    rmt_config_t tx_config = {
        .channel = RMT_TX_CHANNEL,
        .clk_div = 80,              // 1 us tick
        .flags = 0,
        .gpio_num = 0,
        .mem_block_num = 1,
        .rmt_mode = RMT_MODE_TX,
        .tx_config.carrier_en = false,
        .tx_config.idle_level = RMT_IDLE_LEVEL_HIGH,
        .tx_config.idle_output_en = true,
        .tx_config.loop_en = false
    };
    ESP_ERROR_CHECK_WITHOUT_ABORT(rmt_config(&tx_config));
    ESP_ERROR_CHECK_WITHOUT_ABORT(rmt_driver_install(tx_config.channel, 0, 0));

        rmt_config_t rx_config = {
        .channel = RMT_RX_CHANNEL,
        .clk_div = 80,              // 1 us tick
        .flags = 0,
        .gpio_num = 0,
        .mem_block_num = 1,
        .rmt_mode = RMT_MODE_RX,
        .rx_config.idle_threshold = 5000,
    };
    ESP_ERROR_CHECK_WITHOUT_ABORT(rmt_config(&rx_config));
    ESP_ERROR_CHECK_WITHOUT_ABORT(rmt_driver_install(rx_config.channel, 45 * 4 * 2, 0));
}

int dht11_get_data(dht11_data_t *data) {
    rmt_item32_t *items = NULL;
    RingbufHandle_t rb = NULL;
    size_t length = 0;
    int status = 0;

    ESP_ERROR_CHECK_WITHOUT_ABORT(rmt_get_ringbuf_handle(RMT_RX_CHANNEL, &rb));

    memset(data, 0, sizeof(dht11_data_t));

    //send start sequence
    rmt_set_pin(RMT_TX_CHANNEL, RMT_MODE_TX, DHT11_DATA_GPIO);
    ESP_ERROR_CHECK_WITHOUT_ABORT(rmt_write_items(RMT_TX_CHANNEL, start_seq, sizeof(start_seq) / sizeof(start_seq[0]), true));

    //receieve data
    rmt_set_pin(RMT_RX_CHANNEL, RMT_MODE_RX, DHT11_DATA_GPIO);
    ESP_ERROR_CHECK_WITHOUT_ABORT(rmt_rx_start(RMT_RX_CHANNEL, true));
    items = (rmt_item32_t *)xRingbufferReceive(rb, &length, 10);

    if (items == NULL || length == 0) {
        ESP_LOGE(TAG, "Sensor does not respond!\n");
        status = -1;
    } else {
        dht11_decode(items, data, length / 4);
        vRingbufferReturnItem(rb, (void *)items);
        if (dht11_checksum_validation(data)) {
            status = -2;
        }
    }

    ESP_ERROR_CHECK_WITHOUT_ABORT(rmt_rx_stop(RMT_RX_CHANNEL));
    return status;
}

void dht11_turn_on(void) {
    ESP_ERROR_CHECK_WITHOUT_ABORT(gpio_set_level(DHT11_EN_GPIO, 1));
    vTaskDelay(1500 / portTICK_PERIOD_MS);
}

void dht11_turn_off(void) {
    ESP_ERROR_CHECK_WITHOUT_ABORT(gpio_set_level(DHT11_EN_GPIO, 0));
}


/* --- Static functions definitions --- */
static void dht11_gpio_config(void) {
    gpio_config_t en_pin_config;

    en_pin_config.intr_type = GPIO_INTR_DISABLE;
    en_pin_config.mode = GPIO_MODE_OUTPUT;
    en_pin_config.pin_bit_mask = (1ULL << DHT11_EN_GPIO);
    en_pin_config.pull_down_en = GPIO_PULLDOWN_DISABLE;
    en_pin_config.pull_up_en = GPIO_PULLUP_DISABLE;

    ESP_ERROR_CHECK_WITHOUT_ABORT(gpio_config(&en_pin_config));
}

static int dht11_decode(rmt_item32_t *items, dht11_data_t *data, uint32_t length) {
    bool start = false;
    bool start_bit = false;
    dht11_fields_e field = 0;
    uint8_t bit_pos = 7;

    for (int num = 0; num < length; num++) {
        //divide into pulses
        for(int i = 0; i < 2; i++) {
            uint32_t level = (i == 0 ? items->level0 : items->level1);
            uint32_t duration = (i == 0 ? items->duration0 : items->duration1);
            if (start) {
                //decode
                if (start_bit) {
                    //decode
                    if (field >= DHT11_FIELD_MAX) {
                        ESP_LOGE(TAG, "Decoder: Data overflow!\n");
                        return -2;
                    }
                    uint8_t bit;
                    if(dht11_is_one(level, duration)) {
                        bit = 1;
                    } else if (dht11_is_zero(level, duration)) {
                        bit = 0;
                    } else {
                        continue;
                    }
                    data->data[field] |= (bit << bit_pos);
                    if (bit_pos == 0) {
                        field++;
                        bit_pos = 7;
                    } else {
                        bit_pos--;
                    }
                    start_bit = false;
                } else {
                    //search for start bit
                    start_bit = dht11_is_start_bit(level, duration);
                }
            } else {
                //search for start
                start = dht11_is_start(level, duration);
            }
        }
        items++;

    }
    if (!start) {
        ESP_LOGE(TAG, "Decoder: Failed to find start sequence!\n");
        return -1;
    }
    return 0;
}

static int dht11_checksum_validation(dht11_data_t *data) {
    uint8_t checksum = 0;
    for (dht11_fields_e field_p = 0; field_p < DHT11_FIELD_CHECKSUM; field_p++) {
        checksum += data->data[field_p];
        }

    if (checksum != data->data[DHT11_FIELD_CHECKSUM]) {
        ESP_LOGE(TAG, "Checksum does not match!\n");
        return -1;
    } else {
        ESP_LOGV(TAG, "Checksum ok.\n");
        return 0;
    }
}

static bool dht11_is_start(uint32_t level, uint32_t duration) {
    if (level == 1 && abs(DHT11_DURATION_START - duration) < DHT11_DURATON_THRESHOLD) {
        return true;
    } else {
        return false;
    }
}

static bool dht11_is_start_bit(uint32_t level, uint32_t duration) {
    if (level == 0 && abs(DHT11_DURATION_NULL- duration) < DHT11_DURATON_THRESHOLD) {
        return true;
    } else {
        return false;
    }
}

static bool dht11_is_one(uint32_t level, uint32_t duration) {
    if (level == 1 && abs(DHT11_DURATION_ONE - duration) < DHT11_DURATON_THRESHOLD) {
        return true;
    } else {
        return false;
    }
}

static bool dht11_is_zero(uint32_t level, uint32_t duration) {
    if (level == 1 && abs(DHT11_DURATION_ZERO- duration) < DHT11_DURATON_THRESHOLD) {
        return true;
    } else {
        return false;
    }
}
