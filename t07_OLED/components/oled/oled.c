#include "oled.h"


/* --- Defines --- */


/* --- Static variables --- */


/* --- Static functions declarations --- */
static void oled_gpio_init(void);
static void oled_i2c_init(void);
static void oled_send_cmd(uint8_t cmd);
static void oled_send_cmd_double(uint8_t cmd, uint8_t data);


/* --- Public functions definitions --- */
void oled_init(void) {
    oled_gpio_init();
    oled_i2c_init();
    oled_turn_on();

    //display itialization sequence
    oled_send_cmd(SH1106_DISPLAYOFF);
    //hardware
    oled_send_cmd_double(SH1106_SETMULTIPLEX, 63);
    oled_send_cmd_double(SH1106_SETDISPLAYCLOCKDIV, 0x80);
    oled_send_cmd_double(SH1106_SETCOMPINS, SH1106_SETCOMPINS_ALTERNATIVE); //hardware dependant
    oled_send_cmd_double(SH1106_SETVCOMDETECT, 0x40);
    oled_send_cmd_double(SH1106_SETPRECHARGE, 0xF1);
    oled_send_cmd_double(SH1106_DC_CMD, SH1106_DC_ON);
    //adressing
    oled_send_cmd(SH1106_SETSTARTLINE | 0);
    oled_send_cmd(SH1106_PAGE_ADDRESS | 0);
    oled_send_cmd(SH1106_SETHIGHCOLUMN | 0);
    oled_send_cmd(SH1106_SETLOWCOLUMN | 0);
    oled_send_cmd_double(SH1106_SETDISPLAYOFFSET, 0);
    //positioning
    oled_send_cmd(SH1106_SEGREMAP_REVERSE);                                 //aka mirror
    oled_send_cmd(SH1106_COMSCANDEC);                                       //aka flip display

    oled_send_cmd_double(SH1106_SETCONTRAST, 170);

    oled_send_cmd(SH1106_DISPLAYON);

    oled_clear();
    oled_set_cursor(0, 0);
}

void oled_turn_on(void) {
    ESP_ERROR_CHECK_WITHOUT_ABORT(gpio_set_level(OLED_EN_PIN, 1));
    vTaskDelay(1000 / portTICK_PERIOD_MS);
}

void oled_clear(void) {
    for (uint8_t i = 0; i < SCREEN_HEIGHT / 8; i++) {
        oled_clear_page(i);
    }
}

void oled_clear_page(uint8_t page) {
    uint8_t packet[] = {0x40, 0x00};
    oled_send_cmd(SH1106_PAGE_ADDRESS | page);
    for (int j = 0; j < SCREEN_WIDTH; j++) {
        ESP_ERROR_CHECK_WITHOUT_ABORT(i2c_master_write_to_device(OLED_I2C_NUM, OLED_I2C_ADRESS, packet, sizeof(packet), 10));
    }
}

void oled_set_cursor(uint8_t page, uint8_t column) {
    uint8_t higher_bits = 0x0F & (column >> 4);
    uint8_t lower_bits = 0x0F & column;

    oled_send_cmd(SH1106_PAGE_ADDRESS | page);
    oled_send_cmd(SH1106_SETHIGHCOLUMN | higher_bits);
    oled_send_cmd(SH1106_SETLOWCOLUMN | lower_bits);
}

void oled_putchar(char c) {
    uint8_t font_offset = c - 32;
    uint8_t letter[] = {0x40,
                        font6x8[6 * font_offset + 0],
                        font6x8[6 * font_offset + 1],
                        font6x8[6 * font_offset + 2],
                        font6x8[6 * font_offset + 3],
                        font6x8[6 * font_offset + 4],
                        font6x8[6 * font_offset + 5]
                        };
    ESP_ERROR_CHECK_WITHOUT_ABORT(i2c_master_write_to_device(OLED_I2C_NUM, OLED_I2C_ADRESS, letter, sizeof(letter), 10));
}

void oled_puts(char *str) {
    for (uint8_t i = 0; i < strlen(str); i++) {
        oled_putchar(str[i]);
    }
}

void oled_set_offset(uint8_t offset) {
    oled_send_cmd_double(SH1106_SETDISPLAYOFFSET, offset);
}


/* --- Static functions definitions --- */
static void oled_gpio_init(void) {
    gpio_config_t en_pin_config;

    en_pin_config.intr_type = GPIO_INTR_DISABLE;
    en_pin_config.mode = GPIO_MODE_OUTPUT;
    en_pin_config.pin_bit_mask = (1ULL << OLED_EN_PIN);
    en_pin_config.pull_down_en = GPIO_PULLDOWN_DISABLE;
    en_pin_config.pull_up_en = GPIO_PULLUP_DISABLE;

    ESP_ERROR_CHECK_WITHOUT_ABORT(gpio_config(&en_pin_config));
}

static void oled_i2c_init(void) {
    i2c_config_t oled_i2c_config = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = OLED_I2C_SDA_PIN,
        .scl_io_num = OLED_I2C_SCL_PIN,
        .sda_pullup_en = GPIO_PULLUP_DISABLE,
        .scl_pullup_en = GPIO_PULLUP_DISABLE,
        .master.clk_speed = OLED_I2C_SPEED,
    };

    ESP_ERROR_CHECK_WITHOUT_ABORT(i2c_param_config(OLED_I2C_NUM, &oled_i2c_config));
    ESP_ERROR_CHECK_WITHOUT_ABORT(i2c_driver_install(OLED_I2C_NUM, oled_i2c_config.mode, 0, 0, 0));
}

static void oled_send_cmd(uint8_t cmd) {
    uint8_t packet[] = {0x00, cmd};
    ESP_ERROR_CHECK_WITHOUT_ABORT(i2c_master_write_to_device(OLED_I2C_NUM, OLED_I2C_ADRESS, packet, sizeof(packet), 10));
}

static void oled_send_cmd_double(uint8_t cmd, uint8_t data) {
    uint8_t packet[] = {0x00, cmd, data};
    ESP_ERROR_CHECK_WITHOUT_ABORT(i2c_master_write_to_device(OLED_I2C_NUM, OLED_I2C_ADRESS, packet, sizeof(packet), 10));
}
