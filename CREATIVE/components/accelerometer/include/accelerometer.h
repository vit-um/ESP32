#include <string.h>
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "esp_log.h"
#include "adxl345_commands.h"

/* --- Pinout --- */
#define ACC_MOSI_PIN            13
#define ACC_MISO_PIN            12
#define ACC_CLK_PIN             14
#define ACC_CS_PIN              15
#define ACC_INT1_PIN            34
#define ACC_INT2_PIN            35
#define ACC_EN_PIN              23


/* --- Defines --- */
#define ACC_SPI_NUM             SPI2_HOST
#define ACC_SPI_SPEED           400000
#define ACC_G_PER_BIT           0.004

/* --- Enums --- */


/* --- Typedefs --- */
typedef struct acc_data {
    float x;
    float y;
    float z;
}              acc_data_t;


/*--- Public functions declarations --- */
void acc_init(void);
void acc_turn_on(void);
void acc_get_values(acc_data_t *converted);
//commands
void acc_set_bandwidth(uint8_t bandwidth, bool low_power);
void acc_set_range(uint8_t range);
void acc_start_measure(void);
void acc_stop_measure(void);
