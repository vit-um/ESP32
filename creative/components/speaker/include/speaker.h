#include "driver/dac.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_rom_sys.h"


/* --- Pinout --- */
#define SPEAKER_EN_PIN          5
#define SPEAKER_DAC_PIN         25


/* --- Defines --- */


/* --- Enums --- */


/* --- Typedefs --- */


/*--- Public functions declarations --- */
void speaker_init(void);
void speaker_turn_on(void);
void speaker_turn_off(void);
void speaker_play_tone(uint32_t freq, dac_cw_scale_t ampl, uint32_t time);
