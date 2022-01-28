#include <string.h>
#include "driver/gpio.h"
#include "driver/rmt.h"
#include "esp_err.h"
#include "esp_log.h"


/* --- Pinout --- */
#define DHT11_DATA_GPIO         4
#define DHT11_EN_GPIO           2

/* --- Defines --- */
#define RMT_TX_CHANNEL          RMT_CHANNEL_0
#define RMT_RX_CHANNEL          RMT_CHANNEL_1


/* --- Enums --- */
typedef enum dht11_fields {
    DHT11_FIELD_RH_INT = 0,
    DHT11_FIELD_RH_DEC,
    DHT11_FIELD_T_INT,
    DHT11_FIELD_T_DEC,
    DHT11_FIELD_CHECKSUM,
    DHT11_FIELD_MAX
}            dht11_fields_e;


/* --- Typedefs --- */
typedef union dht11_data {
    struct {
       uint8_t rh_int;
       uint8_t rh_dec;
       uint8_t temp_int;
       uint8_t temp_dec;
       uint8_t checksum;
    };
    uint8_t data[DHT11_FIELD_MAX];
}              dht11_data_t;


/*--- Public functions declarations --- */
void dht11_init(void);
void dht11_turn_on(void);
void dht11_turn_off(void);
int dht11_get_data(dht11_data_t *data);
