#include "accelerometer.h"

/*
TODO:   work with interrupts
        define gpio interrupts pins
        interrupt handler
*/
/* --- Defines --- */


/* --- Static variables --- */
static spi_device_handle_t acc_spi;


/* --- Static functions declarations --- */
static void acc_gpio_init(void);
static void acc_register_write(uint8_t address, uint8_t data);
static void acc_convert(uint8_t *data, acc_data_t *converted);


/* --- Public functions definitions --- */
void acc_init(void) {
    acc_gpio_init();

    spi_bus_config_t buscfg = {
        .miso_io_num = ACC_MISO_PIN,
        .mosi_io_num = ACC_MOSI_PIN,
        .sclk_io_num = ACC_CLK_PIN,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
    };
    ESP_ERROR_CHECK_WITHOUT_ABORT(spi_bus_initialize(ACC_SPI_NUM, &buscfg, SPI_DMA_DISABLED));

    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = ACC_SPI_SPEED,
        .mode = 3,
        .spics_io_num = ACC_CS_PIN,
        .queue_size = 5         //to investigate
    };
    ESP_ERROR_CHECK_WITHOUT_ABORT(spi_bus_add_device(ACC_SPI_NUM, &devcfg, &acc_spi));
}

void acc_turn_on(void) {
    ESP_ERROR_CHECK_WITHOUT_ABORT(gpio_set_level(ACC_EN_PIN, 1));
    vTaskDelay(500 / portTICK_PERIOD_MS);
}

void acc_set_bandwidth(uint8_t bandwidth, bool low_power) {
    if (low_power) {
        bandwidth |= ADXL345_BW_LOW_POWER;
    }
    acc_register_write(ADXL345_BW_RATE, bandwidth);
}

void acc_set_range(uint8_t range) {
    acc_register_write(ADXL345_DATA_FORMAT, range | (1 << 3));  //full resolution
}

void acc_start_measure(void) {
    acc_register_write(ADXL345_POWER_CTL, (1 << 3));
}

void acc_stop_measure(void) {
    acc_register_write(ADXL345_POWER_CTL, (0 << 3));
}

void acc_get_values(acc_data_t *converted) {
    uint8_t tx_buff[7];
    uint8_t rx_buff[7];
    memset(tx_buff, 0, 7);
    memset(rx_buff, 0, 7);
    memset(converted, 0, sizeof(acc_data_t));

    tx_buff[0] = ADXL345_DATAX0 | ADXL345_READ | (1 << 6);

    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    t.length = sizeof(tx_buff) * 8;
    t.tx_buffer = tx_buff;
    t.rx_buffer = rx_buff;
    ESP_ERROR_CHECK_WITHOUT_ABORT(spi_device_polling_transmit(acc_spi, &t));

    acc_convert(rx_buff, converted);
}


/* --- Static functions definitions --- */
static void acc_gpio_init(void) {
    gpio_config_t en_pin_config;

    en_pin_config.intr_type = GPIO_INTR_DISABLE;
    en_pin_config.mode = GPIO_MODE_OUTPUT;
    en_pin_config.pin_bit_mask = (1ULL << ACC_EN_PIN);
    en_pin_config.pull_down_en = GPIO_PULLDOWN_DISABLE;
    en_pin_config.pull_up_en = GPIO_PULLUP_DISABLE;

    ESP_ERROR_CHECK_WITHOUT_ABORT(gpio_config(&en_pin_config));
}

static void acc_register_write(uint8_t address, uint8_t data) {
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    uint8_t cmd[] = {address | ADXL345_WRITE, data};
    t.length = sizeof(cmd) * 8;
    t.tx_buffer = cmd;
    ESP_ERROR_CHECK_WITHOUT_ABORT(spi_device_polling_transmit(acc_spi, &t));
}

//rewrite
// static void acc_register_read(uint8_t address, uint8_t *data, uint8_t len) {
//     spi_transaction_t t;
//     memset(&t, 0, sizeof(t));
//     uint8_t cmd[] = {address | ADXL345_READ, data};
//     t.length = sizeof(cmd) * len;
//     t.tx_buffer = cmd;
//     ESP_ERROR_CHECK_WITHOUT_ABORT(spi_device_polling_transmit(acc_spi, &t));
// }

static void acc_convert(uint8_t *data, acc_data_t *converted) {
    int16_t x = data[1] | (data[2] << 8);
    int16_t y = data[3] | (data[4] << 8);
    int16_t z = data[5] | (data[6] << 8);

    converted->x = x * ACC_G_PER_BIT;
    converted->y = y * ACC_G_PER_BIT;
    converted->z = z * ACC_G_PER_BIT;
}
