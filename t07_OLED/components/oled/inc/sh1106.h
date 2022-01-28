#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "driver/timer.h"
#include "esp_log.h"
#include "driver/i2c.h"
#include "esp_err.h"

#include "sh1106_commands.h"
#include "font6x8.h"
#include "font8x8_basic.h"

/* --- Pinout --- */
#define EN_OLED        32
#define SCL_PIN        22
#define SDA_PIN        21


/* --- Defines --- */
#define OLED_I2C_SPEED          400000
#define OLED_I2C_ADRESS         0x3C
#define SCREEN_HEIGHT           64
#define SCREEN_WIDTH            132  //128

#define tag "SH1106"

/*--- Public functions declarations --- */
void oled_init(void);
void sh1106_display_clear(void *ignore);

void oled_set_cursor(uint8_t page, uint8_t column);
void oled_set_offset(uint8_t offset);

void oled_putchar(char c);
void oled_puts(char *str);

void send_display_text(const void *arg_text);
void sh1106_display_text2(const void *arg_text);
void sh1106_display_pattern(void *ignore);
