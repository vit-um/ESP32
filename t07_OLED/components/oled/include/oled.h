#include <string.h>
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "esp_err.h"
#include "esp_log.h"
#include "sh1106_commands.h"
#include "font6x8.h"


/* --- Pinout --- */
#define OLED_EN_PIN             32
#define OLED_I2C_SCL_PIN        22
#define OLED_I2C_SDA_PIN        21

/* --- Defines --- */
#define OLED_I2C_NUM            I2C_NUM_0
#define OLED_I2C_SPEED          400000
#define OLED_I2C_ADRESS         0x3C
#define SCREEN_HEIGHT           64
#define SCREEN_WIDTH            128


/* --- Enums --- */


/* --- Typedefs --- */


/*--- Public functions declarations --- */
void oled_init(void);
void oled_turn_on(void);

void oled_clear(void);
void oled_clear_page(uint8_t page);

void oled_set_cursor(uint8_t page, uint8_t column);
void oled_set_offset(uint8_t offset);

void oled_putchar(char c);
void oled_puts(char *str);
